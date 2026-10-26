
#ifndef CAMERA2_RELATIVEFLYCAMERA_H
#define CAMERA2_RELATIVEFLYCAMERA_H

#include <camera2/stock/FlyCamera.h>
#include <camera2/stock/RelativeCamera.h>

#pragma warning (disable:4481) // nonstandard extension used: override specifier 'override'

namespace Cam2
{

class C_RelativeFlyCamera : public C_FlyCamera
{
public:
	C_RelativeFlyCamera();
	void OnUpdate() OVERRIDE;

	Maths::HVec3 RelativePosition() { return m_relativePosition; }
	void SetRelativePosition(const Maths::HVec3& pos) { m_relativePosition = pos; }

	Maths::C_Quaternion RelativeOrient() { return m_relativeOrient; }
	void SetRelativeOrient(const Maths::C_Quaternion& orient) { m_relativeOrient = orient; }

	void CalcRelative();
	virtual void CopyFrom(const C_Camera& other);
	void KeyNow() OVERRIDE;

protected:
	Maths::HVec3 m_relativePosition;
	Maths::C_Quaternion m_relativeOrient;
};

} // End of namespace Cam2

#endif
