#include "VoipManager.h"
#include "voip/voip_manager.h"
#include "voip/voip_define.h"
#include <Windows.h>

CVoipCallback::CVoipCallback()
	: m_renderThread(0)
	, m_isInited(false)
{
}

CVoipCallback::~CVoipCallback()
{
}
void CVoipCallback::onVideoFrame(bool isLocal, std::shared_ptr<VOIP::RawImage> image, uint32_t userId)
{
	if (!m_isInited) {
		return;
	}
	VideoInfo * vi = new VideoInfo();
	VOIP::RawImageParameter param = image->getRawImageParameter();
	vi->usrID = userId;
	vi->width = param.width;
	vi->height = param.height;
	vi->isLocal = isLocal;
	vi->rotation = param.position ? VOIP::VIDEO_ROTATION_0 : param.rotation;
	vi->fmt = PIX_FMT_YUV420;
	vi->buffer = (uint8_t*)malloc(param.width * param.height * 3 / 2);
	memcpy(vi->buffer, image->getImageData(), param.width*param.height * 3 / 2);
	if (!PostThreadMessage(m_renderThread, MSG_RENDER_VFRAME, (WPARAM)vi, NULL)) {
		free(vi->buffer);
		delete vi;
	}
}

void CVoipCallback::InitCallback(DWORD threadID)
{
	m_renderThread = threadID;
	m_isInited = true;
}

CVoipManager::CVoipManager()
	: m_clientNetwork(nullptr)
	, m_voip(nullptr)
	, m_voipCallback(nullptr)
	, m_selfID(-1)
	, m_sessionID(-1)
	, m_audioCap(nullptr)
	, m_videoCap(nullptr)
	, m_audioRender(nullptr)
	, m_videoRender(nullptr)
{
	m_clientNetwork = ClientNetwork::GetInstance();
	VOIP::TYVoip::Parameter param{ ".", 5, false,30 };
	m_voipCallback = new CVoipCallback();
	m_voip = VOIP::TYVoip::createInstance(param, m_voipCallback);
	m_audioCap = IThread::CreateInstance(IID_AudioCap);
	m_videoCap = IThread::CreateInstance(IID_VideoCap);
	m_audioRender = IThread::CreateInstance(IID_AudioRender);
	m_videoRender = IThread::CreateInstance(IID_VideoRender);
	
	//int channels;	int samplePerSec; int bitPerSample; AudioFmt fmt;	AudioCallback callback;
	m_ac = { 1, 16000, 16, AUDIO_FMT_PCM, AudioCapCallback };
	//int width; int height; int interval; PixelFmt fmt; VideoCallback callback;
	m_vc = { 640, 480, 30, PIX_FMT_YUV420, VideoCapCallback };
	//int channels; int samplePerSec; int bitPerSample; AudioFmt fmt;
	m_ai = { 1, 16000, 16, AUDIO_FMT_PCM };
}

CVoipManager::~CVoipManager()
{
	if (m_voipCallback) {
		delete m_voipCallback;
	}
	if (m_audioCap) {
		IThread::DestroyInstance(&m_audioCap);
	}
	if (m_videoCap) {
		IThread::DestroyInstance(&m_videoCap);
	}
	if (m_audioRender) {
		IThread::DestroyInstance(&m_audioRender);
	}
	if (m_videoRender) {
		IThread::DestroyInstance(&m_videoRender);
	}
}

bool CVoipManager::LoginServer(const char * serverIP, uint32_t port, uint32_t usrID, uint32_t sessionID)
{
	int res = m_clientNetwork->Login(serverIP, port, usrID, sessionID);

	if (res == 0) {
		return false;
	}
	else {
		return true;
	}
}

void CVoipManager::AddNode(uint32_t nodeID)
{
	m_clientNetwork->AddGroupClientNode(nodeID);
	m_nodeList.push_back(nodeID);
	printf("add node %d\n", nodeID);
}

void CVoipManager::RemoveNode(uint32_t nodeID)
{
	m_clientNetwork->RemoveGroupClientNode(nodeID);
	m_voip->removeGroupClientNode(nodeID);
	m_nodeList.remove(nodeID);
	printf("add node %d\n", nodeID);
}

void CVoipManager::StartVoipTransmit()
{
	m_voip->startGroupCallByCenter("sui bian tian", m_selfID, m_clientNetwork);
	Sleep(10);
	m_voip->startAudio(VOIP::TYVoip::AudioParameter(16000, false));
	m_voip->startVideo(VOIP::TYVoip::VideoParameter(512, 1024 * 16 / 9, false));
	Sleep(10);
	// av capture thread start
	m_audioCap->InitThread(&m_ac);
	m_audioCap->StartThread();
	m_videoCap->InitThread(&m_vc);
	m_videoCap->StartThread();

	// av render thread start
	m_audioRender->InitThread(&m_ai);
	m_audioRender->StartThread();
	m_videoRender->InitThread(nullptr);
	m_videoRender->StartThread();
	m_voipCallback->InitCallback(m_videoRender->m_threadID);
}

void CVoipManager::EndVoipTransmit()
{
	// clear all node list
	m_voip->stopCall();
	Sleep(100);
	m_clientNetwork->ClearGroupClientList();
	m_clientNetwork->Logout();
	m_audioCap->EndThread();
	m_videoCap->EndThread();
	m_audioRender->EndThread();
	m_videoRender->EndThread();
}


void VideoCapCallback(uint8_t * vData, int width, int height, PixelFmt fmt)
{
	if (fmt != PIX_FMT_YUV420) {
		printf("wrong video fmt!\n");
		return;
	}
	VOIP::TYVoipManager::getInstance().setVoipVideoData(
		vData,
		width * height * 3 / 2,
		width,
		height,
		VOIP::VIDEO_ROTATION_0,
		false);
}

DWORD AudioCapCallback(uint8_t * aData, size_t size, AudioFmt fmt)
{
	if (fmt != AUDIO_FMT_PCM) {
		printf("wrong aduio fmt!\n");
		return 0;
	}
	//printf("````````````audio call back`````````````\n");
	VOIP::TYVoipManager::getInstance().setVoipAudioData(
		aData,
	    size,
		16000);
	return 0;
}