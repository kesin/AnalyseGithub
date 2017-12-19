#include "VideoRender.h"

CVideoRender::CVideoRender()
{
}

CVideoRender::~CVideoRender()
{
}

bool CVideoRender::InitThread(void * initInfo)
{
	return true;
}

bool CVideoRender::StartThread()
{
	unsigned threadID;
	m_handle = (HANDLE)_beginthreadex(NULL,   // security  
		0,      // stack size  
		CVideoRender::ThreadStaticEntryPoint,
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

bool CVideoRender::EndThread()
{
	cv::destroyAllWindows();
	PostThreadMessage(m_threadID, WM_QUIT, NULL, NULL);
	WaitForSingleObject(m_handle, INFINITE);
	DWORD exitCode;
	GetExitCodeThread(m_handle, &exitCode);
	printf("video render thread exited with code %u\n", exitCode);
	CloseHandle(m_handle);
	return true;
}

unsigned CVideoRender::ThreadStaticEntryPoint(void * pThis)
{
	CVideoRender * pRender = (CVideoRender*)pThis;
	pRender->ThreadMessageLoop();
	return 0;
}

int CVideoRender::ThreadMessageLoop()
{
	BOOL ret;
	MSG msg;
	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (ret == -1) {
			DWORD err = GetLastError();
			printf("something terriblly wrong,  errno = %lu!\n", err);
			assert(0);
		}
		switch (msg.message)
		{
		case MSG_RENDER_VFRAME:
			RenderOneFrame(msg.wParam, msg.lParam);
			break;
		default:
			//printf("unknown message!\n");
			break;
		}
	}
	return 0;
}

bool CVideoRender::RenderOneFrame(WPARAM wParam, LPARAM lParam)
{
	VideoInfo *vi = (VideoInfo*)wParam;
	if (vi->fmt != PIX_FMT_YUV420) {
		printf("wrong pixel fmt!\n");
		return false;
	}
	char winName[50];
	if (vi->isLocal) {
		sprintf(winName, "local video");
	}
	else {
		sprintf(winName, "usrID is %lu\n", vi->usrID);
	}
	cv::Mat yuvFrame{ vi->height * 3 / 2, vi->width, CV_8UC1, vi->buffer };
	cv::Mat rgbFrame;
	cv::cvtColor(yuvFrame, rgbFrame, CV_YUV2RGB_I420);
	cv::imshow(winName, rgbFrame);
	cvWaitKey(1);
	free(vi->buffer);
	delete vi;
	return true;
}
