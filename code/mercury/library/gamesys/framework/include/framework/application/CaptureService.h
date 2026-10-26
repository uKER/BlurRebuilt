#ifndef FRAMEWORK_CAPTURE_SERVICE_H
#define FRAMEWORK_CAPTURE_SERVICE_H

#include <camera2/capture/CaptureService.h>

namespace Framework
{

class C_CaptureService : public Cam2::C_CaptureService
{
	void SetFps(F32 fps);
	F32 GetFps();
};

}

#endif
