
#ifndef CAMERA2_CHASEPARAMS_H
#define CAMERA2_CHASEPARAMS_H

namespace Cam2
{

class C_ChaseParam
{
public:
	C_ChaseParam();

	F32 distance, height, fov, tilt, lag, bank, pivotZ;
	F32 behindSpring, angularLimit, angularFade;
	F32 safeHeight;
};

} // End of namespace Cam2

#endif
