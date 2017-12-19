#pragma once
#include <Windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include "opencv2/opencv.hpp"
#include "IThread.h"
#include "Definitions.h"


class CVideoRender : public IThread {
public:

	CVideoRender();
	virtual ~CVideoRender();

	virtual bool InitThread(void * initInfo);
	virtual bool StartThread();
	virtual bool EndThread();

private:
	static unsigned __stdcall ThreadStaticEntryPoint(void * pThis);
	int ThreadMessageLoop();
	bool RenderOneFrame(WPARAM wParam, LPARAM lParam);
};