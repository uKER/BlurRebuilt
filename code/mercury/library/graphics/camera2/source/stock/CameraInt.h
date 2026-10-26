
// CameraInt.h - Internal shared functions in the camera library 

namespace Cam2
{
// RelativeCamera.cpp:
void RelativePlace(C_Camera& cam, const Maths::HVec3& repos, const Maths::C_Quaternion& reori);

// ScriptCamera.h
Maths::C_Quaternion EulerToQuat(F32 pitch, F32 yaw, F32 roll);


} // End of namespace Cam2
