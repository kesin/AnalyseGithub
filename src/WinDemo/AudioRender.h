#pragma once
#include <Windows.h>
#include <process.h>
#include "IThread.h"
#include "Definitions.h"
#include <atomic>

class CAudioRender : public IThread {
public:
	enum { FRAGMENT_NUM = 2 };
	enum { BUFFER_FRAME_CNT = 8, AUDIO_FRAME_SIZE = 320 };
	CAudioRender();
	virtual ~CAudioRender();

	virtual bool InitThread(void * initInfo);
	virtual bool StartThread();
	virtual bool EndThread();

private:
	static unsigned __stdcall ThreadStaticEntryPoint(void * pThis);
	bool RenderAudio();
	void GetAudioData(int index);
private:
	HWAVEOUT m_hwo;
	AudioInfo m_ai;
	std::atomic_bool m_isRendering;
	int m_bufferSize;
	uint8_t * m_pBuffer[FRAGMENT_NUM];
	HANDLE m_wait;
};