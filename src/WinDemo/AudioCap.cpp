#include "AudioCap.h"
#include <mmsystem.h>
#include <stdio.h>
#include <assert.h>

CAudioCap::CAudioCap()
	: m_devCnt(0)
	, m_hwi(nullptr)
	, m_isCapping(false)
	, m_bufferSize(0)
{
	m_devCnt = waveInGetNumDevs();
	printf("total audio input device num is : %d\n", m_devCnt);	
	for (int i = 0; i < m_devCnt; i++) {
		WAVEINCAPS wic;
		waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS));
		WAVE_FORMAT_1M08;
		printf("[Capture] DeviceNum:[%d], DevicePID:[%d], DeviceName：%s\n", i, wic.wPid, wic.szPname);
	}
}

CAudioCap::~CAudioCap()
{
}

bool CAudioCap::InitThread(void * initInfo)
{
	if (m_devCnt == 0) {
		printf("have no audio input devices, please check!\n");
		return false;
	}
	m_ac = *(AudioConfig*)initInfo;
	m_bufferSize = m_ac.samplePerSec * m_ac.bitPerSample / 8 * 10 / 1000;  //SamplesPerSec * byte per sample * duration time
	return true;
}

bool CAudioCap::StartThread()
{
	unsigned threadID;
	m_handle = (HANDLE)_beginthreadex(NULL,
		0,
		CAudioCap::ThreadStaticEntryPoint,
		this,
		0,
		&threadID);
	if (m_handle == 0) {
		printf("create audio capture thread failed!\n");
		return false;
	}
	printf("audio capture thread start successfully!\n");
	m_threadID = GetThreadId(m_handle);
	return true;
}

bool CAudioCap::EndThread()
{
	m_isCapping = false;
	WaitForSingleObject(m_handle, INFINITE);
	DWORD exitCode;
	GetExitCodeThread(m_handle, &exitCode);
	printf("audio capture thread exited with code %u\n", exitCode);
	CloseHandle(m_handle);
	return true;
}

DWORD CAudioCap::MicCallback(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	CAudioCap * pCap = (CAudioCap*)dwInstance;
	switch (uMsg)
	{
	case WIM_OPEN:
		printf("audio capture device opened \n");
		break;
	case WIM_DATA:
		pCap->m_ac.callback((uint8_t*)((PWAVEHDR)dwParam1)->lpData, ((PWAVEHDR)dwParam1)->dwBytesRecorded, pCap->m_ac.fmt);
		if (pCap->m_isCapping) {
			waveInAddBuffer(pCap->m_hwi, (PWAVEHDR)dwParam1, sizeof(WAVEHDR));
		}
		break;
	case WIM_CLOSE:
		printf("audio capture device closed\n");
		break;
	default:
		break;
	}
	return 0;
}

unsigned CAudioCap::ThreadStaticEntryPoint(void * pThis)
{
	CAudioCap *pACap = (CAudioCap*)pThis;
	pACap->CapAudioData();
	return 0;
}

bool CAudioCap::CapAudioData()
{
	WAVEFORMATEX waveForm;
	memset(&waveForm, 0, sizeof(WAVEFORMAT));
	waveForm.wFormatTag = (m_ac.fmt == AUDIO_FMT_PCM) ? WAVE_FORMAT_PCM : WAVE_INVALIDFORMAT;
	waveForm.nChannels = m_ac.channels;
	waveForm.nSamplesPerSec = m_ac.samplePerSec;
	waveForm.wBitsPerSample = m_ac.bitPerSample;
	waveForm.nBlockAlign = waveForm.nChannels * waveForm.wBitsPerSample / 8;
	waveForm.nAvgBytesPerSec = waveForm.nBlockAlign*waveForm.nSamplesPerSec;
	waveForm.cbSize = 0;
	MMRESULT mmr = waveInOpen(&m_hwi, WAVE_MAPPER, &waveForm, (DWORD)(MicCallback), DWORD(this), CALLBACK_FUNCTION);
	if (mmr != MMSYSERR_NOERROR) {
		m_isInited = false;
		m_isCapping = false;
		return false;
	}
	printf("open audio device successfully!\n");
	m_isInited = true;
	m_isCapping = true;
	
	WAVEHDR wHdr[FRAGMENT_NUM];
	for (int i = 0; i < FRAGMENT_NUM; i++) {
		wHdr[i].dwBufferLength = m_bufferSize;
		wHdr[i].dwBytesRecorded = 0;
		wHdr[i].dwUser = 0;
		wHdr[i].dwFlags = 0;
		wHdr[i].dwLoops = 0;
		wHdr[i].lpNext = NULL;
		wHdr[i].reserved = 0;
		m_pBuffer[i] = (uint8_t*)malloc(m_bufferSize * sizeof(uint8_t));
		wHdr[i].lpData = (LPSTR)m_pBuffer[i];
		waveInPrepareHeader(m_hwi, &wHdr[i], sizeof(WAVEHDR));
		waveInAddBuffer(m_hwi, &wHdr[i], sizeof(WAVEHDR));
	}
	mmr = waveInStart(m_hwi);
	if (mmr != MMSYSERR_NOERROR) {
		printf("waveInStart failed!\n");
		assert(0);
	}
	while (m_isCapping) {
		Sleep(100);
	}
	Sleep(300); // 暂停当前线程等待VoIP里面的音频数据处理结束再退出，后面要改
	mmr = waveInStop(m_hwi);
	if (mmr != MMSYSERR_NOERROR) {
		printf("stop wave capture failed!\n");
		assert(0);
	}
	mmr = waveInReset(m_hwi);
	if (mmr != MMSYSERR_NOERROR) {
		printf("close wave in device failed!\n");
		assert(0);
	}
	WAVEHDR waveHDR;
	for (int i = 0; i < FRAGMENT_NUM; i++) {
		waveInUnprepareHeader(m_hwi, &waveHDR, sizeof(WAVEHDR));
		free(m_pBuffer[i]);
		m_pBuffer[i] = NULL;
	}
	mmr = waveInClose(m_hwi);
	if (mmr != MMSYSERR_NOERROR) {
		printf("close wave in device failed!\n");
		assert(0);
	}
	return true;
}
