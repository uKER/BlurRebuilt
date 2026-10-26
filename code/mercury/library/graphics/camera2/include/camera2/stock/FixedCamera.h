
#ifndef CAMERA2_FIXEDCAMERA_H
#define CAMERA2_FIXEDCAMERA_H

#include <camera2/Camera.h>
#include <camera2/CameraType.h>

namespace Cam2
{

class C_FixedCamera : public C_Camera
{
public:
	C_FixedCamera()
	{
		SetType(E_FixedCamera);
		SetName("Fixed");
	}
};

} // End of namespace Cam2

#endif
