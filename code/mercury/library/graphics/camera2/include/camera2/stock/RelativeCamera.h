
#ifndef CAMERA2_RELATIVECAMERA_H
#define CAMERA2_RELATIVECAMERA_H

#include <camera2/Camera.h>

namespace Cam2
{

using namespace Maths;

class C_RelativeCamera : public C_Camera
{
public:
	C_RelativeCamera();
	void OnUpdate() OVERRIDE;

	HVec3 RelativePosition() { return m_relativePosition; }
	void SetRelativePosition(const HVec3& pos) { m_relativePosition = pos; }

	C_Quaternion RelativeOrient() { return m_relativeOrient; }
	void SetRelativeOrient(const C_Quaternion& orient) { m_relativeOrient = orient; }
	void SetRelativeOrient(F32 pitch, F32 yaw, F32 roll); // Euler angles helper function
	void CalcRelative();

	virtual void CopyFrom(const C_Camera& other);

protected:
	HVec3 m_relativePosition;
	C_Quaternion m_relativeOrient;
};

} // End of namespace Cam2

#endif
