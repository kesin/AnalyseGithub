//
// Created @ 2017/12/13
//

#pragma once
#include <Windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <atomic>
#include "opencv2/opencv.hpp"
#include "Definitions.h"
#include "IThread.h"

// video capture thread 
class CVideoCap : public IThread {
public:
	CVideoCap();
	virtual ~CVideoCap();

	virtual bool InitThread(void * initInfo);
	virtual bool StartThread();
	virtual bool EndThread();

	static unsigned __stdcall ThreadStaticEntryPoint(void * pThis);

private:
	bool CapOneFrame();
	bool CaptureVideoData();

private:
	cv::VideoCapture * m_vCap;
	VideoConfig m_vcnf;
	std::atomic_bool m_isCapping;
};