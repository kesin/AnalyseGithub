#include "LocalTest.h"

#include <assert.h>
#include <stdio.h>

DWORD id = 0;

void LocalTest(void)
{
	int cx = 640, cy = 480, interval = 30;
	UINT32 usrID = 0;
	PixelFmt pixFmt = PIX_FMT_YUV420;
	// video capture thread
	IThread * videoCap = IThread::CreateInstance(IID_VideoCap);
	VideoConfig vc {cx, cy, interval, pixFmt, VideoCap};
	videoCap->InitThread(&vc);
	if (!videoCap->StartThread()) {
		assert(0);
	}
	// video render thread
	IThread * videoRender = IThread::CreateInstance(IID_VideoRender);
	VideoInfo vi { cx, cy, usrID, pixFmt};
	videoRender->InitThread(nullptr);
	if (!videoRender->StartThread()) {
		assert(0);
	}
	id = videoRender->m_threadID;
	Sleep(5000);
	videoCap->EndThread();
	IThread::DestroyInstance(&videoCap);
	Sleep(1000);
	videoRender->EndThread();
	IThread::DestroyInstance(&videoRender);

	// audio capture thread;
	int channels = 1, samplePerSec = 44100, bitPerSample = 16;
	AudioFmt fmt = AUDIO_FMT_PCM;

	// audio capture thread
	IThread * audioCap = IThread::CreateInstance(IID_AudioCap);
	AudioConfig ac{channels, samplePerSec, bitPerSample, fmt, AudioCap};
	audioCap->InitThread(&ac);
	if (!audioCap->StartThread()) {
		assert(0);
	}
	Sleep(2000);
	audioCap->EndThread();
	IThread::DestroyInstance(&audioCap);

	// audio render thread
	IThread *audioRender = IThread::CreateInstance(IID_AudioRender);
	AudioInfo ai{ channels, samplePerSec, bitPerSample, fmt};
	audioRender->InitThread(&ai);
	if (!audioRender->StartThread()) {
		assert(0);
	}
	Sleep(3000);
	audioRender->EndThread();
	IThread::DestroyInstance(&audioRender);
}


void VideoCap(uint8_t * vData, int width, int height, PixelFmt fmt)
{
	if (id == 0) {
		return;
	}
	VideoInfo * vi = new VideoInfo();
	vi->usrID = 0;
	vi->width = width;
	vi->height = height;
	vi->isLocal = true;
	vi->rotation = 0;
	vi->fmt = fmt;
	vi->buffer = (uint8_t*)malloc(width * height * 3 / 2);
	memcpy(vi->buffer, vData, width * height * 3 / 2*sizeof(uint8_t));
	if (!PostThreadMessage(id, MSG_RENDER_VFRAME, (WPARAM)vi, NULL)) {
		printf("post msg failed!\n");
		free(vi->buffer);
		delete vi;
	}
}

FILE * fo = fopen("audioOut.pcm", "wb");

DWORD AudioCap(uint8_t * aData, size_t size, AudioFmt fmt)
{
	static int i = 0;
	//printf("audio cap %d\n", i++);
	fwrite(aData, sizeof(uint8_t), size, fo);
	return 0;
}
