#pragma once
#include <stdint.h>

enum PixelFmt
{
	PIX_FMT_YUV420 = 0,
	PIX_FMT_YUY2,
	PIX_FMT_RGBA
};
enum AudioFmt
{
	AUDIO_FMT_PCM
};

typedef void(*VideoCallback)(uint8_t * vData, int width, int height, PixelFmt fmt);
typedef DWORD(*AudioCallback)(uint8_t * aData, size_t size, AudioFmt fmt);

struct VideoConfig
{
	int width;
	int height;
	int interval;
	PixelFmt fmt;
	VideoCallback callback;
};

struct AudioConfig
{
	int channels;
	int samplePerSec;
	int bitPerSample;
	AudioFmt fmt;
	AudioCallback callback;
};

struct VideoInfo
{
	int width;
	int height;
	UINT32 usrID;
	bool isLocal;
	int rotation;
	PixelFmt fmt;
	uint8_t * buffer;
};

struct AudioInfo
{
	int channels;
	int samplePerSec;
	int bitPerSample;
	AudioFmt fmt;
};

enum { MSG_RENDER_VFRAME = WM_USER + 10 };