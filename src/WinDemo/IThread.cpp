#include "IThread.h"
#include "VideoCap.h"
#include "VideoRender.h"
#include "AudioCap.h"
#include "AudioRender.h"
#include <assert.h>

IThread * IThread::CreateInstance(int iid)
{
	switch (iid)
	{
	case IID_VideoRender:
		return new CVideoRender();
	case IID_VideoCap:
		return new CVideoCap();
		break;
	case IID_AudioRender:
		return new CAudioRender();
		break;
	case IID_AudioCap:
		return new CAudioCap();
		break;
	default:
		return nullptr;
		break;
	}
}

bool IThread::DestroyInstance(IThread ** pInstance)
{
	if ((*pInstance) != nullptr) {
		delete (*pInstance);
		(*pInstance) = nullptr;
	}
	return true;
}
