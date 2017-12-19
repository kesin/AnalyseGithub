#include "AudioRender.h"
#include "voip/voip_manager.h"
#include <assert.h>

CAudioRender::CAudioRender()
	: m_bufferSize(0)
	, m_wait(nullptr)
{
	m_bufferSize = BUFFER_FRAME_CNT * AUDIO_FRAME_SIZE;
	for (int i = 0; i < FRAGMENT_NUM; i++) {
		m_pBuffer[i] = (uint8_t*)malloc(m_bufferSize * sizeof(uint8_t));
	}
}

CAudioRender::~CAudioRender()
{
	for (int i = 0; i < FRAGMENT_NUM; i++) {
		free(m_pBuffer[i]);
	}
}

bool CAudioRender::InitThread(void * initInfo)
{
	m_ai = *(AudioInfo*)initInfo;
	return true;
}

bool CAudioRender::StartThread()
{
	unsigned threadID;
	m_handle = (HANDLE)_beginthreadex(NULL,   // security  
		0,      // stack size  
		CAudioRender::ThreadStaticEntryPoint,
		this,   // arg list  
		0,		// start thread immediately
		&threadID);
	if (m_handle == nullptr) {
		printf("create video render thread failed!\n");
		return false;
	}

	printf("video render thread start successfully!\n");
	m_threadID = GetThreadId(m_handle);
	return true;
}

bool CAudioRender::EndThread()
{
	m_isRendering = false;
	WaitForSingleObject(m_handle, INFINITE);
	//WAVEHDR waveHDR[FRAGMENT_NUM];
	//MMRESULT mmr;
	//for (int i = 0; i < FRAGMENT_NUM; i++) {
	//	waveOutUnprepareHeader(m_hwo, &waveHDR[i], sizeof(WAVEHDR));
	//}
	//mmr = waveOutClose(m_hwo);
	//if (mmr != MMSYSERR_NOERROR) {
	//	printf("wave close failed!\n");
	//	assert(0);
	//}
	DWORD exitCode;
	GetExitCodeThread(m_handle, &exitCode);
	printf("audio render thread exited with code %u\n", exitCode);
	CloseHandle(m_handle);
	CloseHandle(m_wait);
	return true;
}

unsigned CAudioRender::ThreadStaticEntryPoint(void * pThis)
{
	CAudioRender * pRender = (CAudioRender*)pThis;
	pRender->RenderAudio();
	return 0;
}

bool CAudioRender::RenderAudio()
{
	WAVEFORMATEX waveFmt;
	waveFmt.wFormatTag = (m_ai.fmt == AUDIO_FMT_PCM) ? WAVE_FORMAT_PCM : WAVE_INVALIDFORMAT;
	waveFmt.nChannels = m_ai.channels;
	waveFmt.nSamplesPerSec = m_ai.samplePerSec;
	waveFmt.wBitsPerSample = m_ai.bitPerSample;
	waveFmt.nBlockAlign = m_ai.channels*m_ai.bitPerSample / 8;
	waveFmt.nAvgBytesPerSec = waveFmt.nBlockAlign*waveFmt.nSamplesPerSec;
	waveFmt.cbSize = 0;
	m_wait = CreateEvent(NULL, 0, 0, NULL);
	MMRESULT mmr = waveOutOpen(&m_hwo, WAVE_MAPPER, &waveFmt, (DWORD_PTR)m_wait, 0, CALLBACK_EVENT);
	if (mmr != MMSYSERR_NOERROR) {
		printf("audio play device open failed!\n");
		return false;
	}

	WAVEHDR wHdr[FRAGMENT_NUM];
	for (int i = 0; i < FRAGMENT_NUM; i++) {
		wHdr[i].dwBufferLength = m_bufferSize;
		wHdr[i].dwBytesRecorded = 0;
		wHdr[i].dwUser = 0;
		wHdr[i].dwFlags = 0;
		wHdr[i].dwLoops = 0;
		wHdr[i].lpNext = NULL;
		wHdr[i].reserved = 0;
		wHdr[i].lpData = (LPSTR)m_pBuffer[i];
		waveOutPrepareHeader(m_hwo, &wHdr[i], sizeof(WAVEHDR));
	}
	if (!(wHdr[0].dwFlags & WHDR_PREPARED) || !(wHdr[1].dwFlags & WHDR_PREPARED)){
		printf("error!\n");
		return false;
    }

	m_isRendering = true;
	int index = 0;
	GetAudioData(index);
	while (m_isRendering) {
		GetAudioData(1-index);
		mmr = waveOutWrite(m_hwo, &wHdr[index], sizeof(WAVEHDR));
		if (mmr != MMSYSERR_NOERROR) {
			printf("wave out failed!\n");
			assert(0);
		}
		WaitForSingleObject(m_wait, INFINITE);
		index = 1- index;
	}
	WaitForSingleObject(m_wait, INFINITE);

	for (int i = 0; i < FRAGMENT_NUM; i++) {
		waveOutUnprepareHeader(m_hwo, &wHdr[i], sizeof(WAVEHDR));
	}
	mmr = waveOutClose(m_hwo);
	if (mmr != MMSYSERR_NOERROR) {
		printf("wave close failed!\n");
		assert(0);
	}
	return true;
}

void CAudioRender::GetAudioData(int index)
{
	for (int i = 0; i < BUFFER_FRAME_CNT; i++) {
		std::shared_ptr<VOIP::RawSound> buf = VOIP::TYVoipManager::getInstance().getVoipAudioData(AUDIO_FRAME_SIZE);
		if (buf != NULL) {
			memcpy(m_pBuffer[index] + i*AUDIO_FRAME_SIZE, buf->getSoundData(), AUDIO_FRAME_SIZE);
		}
		else {
			memset(m_pBuffer[index], 0, m_bufferSize);
		}
	}
}
