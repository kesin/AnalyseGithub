#pragma once
#include "voip/voip_context.h"
#include "client_network/network_module.h"
#include "IThread.h"
#include "Definitions.h"
#include <stdint.h>
#include <atomic>
#include <list>

class CVoipCallback :public VOIP::TYVoip::Callback {
public:
	CVoipCallback();
	virtual ~CVoipCallback();
	void onVideoFrame(bool isLocal, std::shared_ptr<VOIP::RawImage> image, uint32_t userId);
	void InitCallback(DWORD threadID);
private:
	std::atomic_bool m_isInited;
	DWORD m_renderThread;
};

class CVoipManager {
public:
	CVoipManager();
	virtual ~CVoipManager();

	bool LoginServer(const char* serverIP, uint32_t port, uint32_t usrID, uint32_t sessionID);
	void AddNode(uint32_t nodeID);
	void RemoveNode(uint32_t nodeID);
	void StartVoipTransmit();
	void EndVoipTransmit();
private:
	ClientNetwork * m_clientNetwork;
	VOIP::TYVoip * m_voip;
	CVoipCallback * m_voipCallback;
	uint32_t m_selfID, m_sessionID;
	IThread *m_audioCap, *m_videoCap, *m_audioRender, *m_videoRender;
	AudioConfig m_ac;
	VideoConfig m_vc;
	AudioInfo m_ai;
	VideoInfo m_vi;
	std::list<UINT32> m_nodeList;
};

void VideoCapCallback(uint8_t * vData, int width, int height, PixelFmt fmt);

DWORD AudioCapCallback(uint8_t * aData, size_t size, AudioFmt fmt);