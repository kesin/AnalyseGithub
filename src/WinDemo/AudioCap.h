#pragma once
#include "IThread.h"
#include "Definitions.h"
#include <atomic>

class CAudioCap : public IThread {
public:
	enum{ FRAGMENT_NUM = 2 };
	CAudioCap();
	virtual ~CAudioCap();
	virtual bool InitThread(void * initInfo);
	virtual bool StartThread();
	virtual bool EndThread();

private:
	static DWORD CALLBACK MicCallback(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	static unsigned __stdcall ThreadStaticEntryPoint(void * pThis);
	bool CapAudioData();
private:
	HWAVEIN m_hwi;
	int m_devCnt;
	int m_index;
	AudioConfig m_ac;
	std::atomic_bool m_isCapping;
	uint8_t * m_pBuffer[FRAGMENT_NUM];
	size_t m_bufferSize;
};