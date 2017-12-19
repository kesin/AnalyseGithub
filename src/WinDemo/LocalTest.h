#pragma once

#include "IThread.h"
#include "Definitions.h"


void LocalTest(void);

void VideoCap(uint8_t * vData, int width, int height, PixelFmt fmt);

DWORD AudioCap(uint8_t * aData, size_t size, AudioFmt fmt);