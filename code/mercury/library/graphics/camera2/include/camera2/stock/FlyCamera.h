
#ifndef CAMERA2_FLYCAMERA_H
#define CAMERA2_FLYCAMERA_H

#include <camera2/Camera.h>
#include <camera2/CameraInput.h>

namespace Cam2
{

using namespace Maths;

class C_FlyCamera : public C_Camera
{
public:
	C_FlyCamera();
	void SetInput(const CamInput& input);
	void OnUpdate() OVERRIDE;
	void CheckKeys();
	virtual void KeyNow();
	void AddKeyFrame(bool relative, const HVec3& pos, const C_Quaternion& orient);
	float GameSpeed() const;

protected:
	CamInput m_input;
	void MoveAxis(F32& sx, F32& sy, F32& sz);
	void LookAxis(F32& sx, F32& sy);
	
private:
	S32 m_Debounce;
	HVec3 m_Velocity;
	F32 m_RotateA, m_RotateB, m_RotateC, m_Zoom;
};

void QuatToEuler(const C_Quaternion& q, F32& pitch, F32& yaw, F32& roll);
extern Debug::Float KeyFramePeriod;

} // End of namespace Cam2

#endif
