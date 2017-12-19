#pragma once
#include <Windows.h>
#include <process.h>

enum {IID_VideoRender = 0, IID_VideoCap, IID_AudioRender, IID_AudioCap};

class IThread {
public:
	IThread() : m_isInited(false), m_handle(nullptr), m_threadID(0) {};
	virtual ~IThread() {};

	virtual bool InitThread(void * initInfo) = 0;
	virtual bool StartThread() = 0;
	virtual bool EndThread() = 0;

	static IThread * CreateInstance(int iid);
	static bool DestroyInstance(IThread ** pInstance);
public:
	DWORD m_threadID;

protected:
	bool m_isInited;
	HANDLE m_handle;
};