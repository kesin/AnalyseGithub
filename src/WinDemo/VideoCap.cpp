#include "VideoCap.h"
#include "opencv2/imgproc/imgproc.hpp"

CVideoCap::CVideoCap()
	: m_vCap(nullptr)
	, m_isCapping(false)
{
	m_vCap = new cv::VideoCapture(0);
}

CVideoCap::~CVideoCap()
{
	if (m_vCap) {
		delete m_vCap;
	}
}

bool CVideoCap::InitThread(void * initInfo)
{
	VideoConfig *vc = (VideoConfig*)initInfo;
	if (!m_vCap->isOpened()) {
		printf("Open Camera failed!\n");
		m_isInited = false;
		return false;
	}
	m_vcnf.width = vc->width;
	m_vcnf.height = vc->height;
	m_vcnf.interval = vc->interval;
	m_vcnf.fmt = vc->fmt;
	m_vcnf.callback = vc->callback;
	m_vCap->set(CV_CAP_PROP_FRAME_WIDTH, m_vcnf.width);
	m_vCap->set(CV_CAP_PROP_FRAME_HEIGHT, m_vcnf.height);
	m_vCap->set(CV_CAP_PROP_FORMAT, CV_8UC3);
	m_isInited = true;
	m_isCapping = true;
	return true;
}

bool CVideoCap::StartThread()
{
	unsigned threadID;
	m_handle = (HANDLE)_beginthreadex(NULL,
		0,
		CVideoCap::ThreadStaticEntryPoint,
		this,
		0,
		&threadID);
	if (m_handle == 0) {
		printf("create video capture thread failed!\n");
		return false;
	}
	printf("video capture thread start successfully!\n");
	m_threadID = GetThreadId(m_handle);
	return true;
}

bool CVideoCap::EndThread()
{
	m_isCapping = false;
	WaitForSingleObject(m_handle, INFINITE);
	DWORD exitCode;
	GetExitCodeThread(m_handle, &exitCode);
	printf("video capture thread exited with code %u\n", exitCode);
	CloseHandle(m_handle);
	return true;
}

unsigned CVideoCap::ThreadStaticEntryPoint(void * pThis)
{
	CVideoCap * pVCap = (CVideoCap*)pThis;
	pVCap->CaptureVideoData();
	return 0;
}

bool CVideoCap::CaptureVideoData()
{
	if (!m_isInited) {
		printf("thread inits failed, exit thread!\n");
		return false;
	}
	while (m_isCapping) {
		CapOneFrame();
		Sleep(m_vcnf.interval);
	}
	Sleep(300);
	return true;
}

bool CVideoCap::CapOneFrame()
{
	cv::Mat frame, yuvFrame;
	(*m_vCap) >> frame;
	cvtColor(frame, yuvFrame, CV_RGB2YUV_I420);
	m_vcnf.callback(yuvFrame.data, m_vcnf.width, m_vcnf.height, m_vcnf.fmt);
	return true;
}