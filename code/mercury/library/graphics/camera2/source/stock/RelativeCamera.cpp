
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/RelativeCamera.h"
#include "camera2/stock/RelativeFlyCamera.h"
#include "CameraInt.h"

namespace Cam2
{

C_RelativeCamera::C_RelativeCamera()
{
	SetType(E_RelativeCamera);
	SetName("Relative");
}

void C_RelativeCamera::OnUpdate()
{
	RelativePlace(*this, m_relativePosition, m_relativeOrient);
}

void C_RelativeCamera::SetRelativeOrient(F32 pitch, F32 yaw, F32 roll)
{
	m_relativeOrient = EulerToQuat(pitch, yaw, roll);
}

void C_RelativeCamera::CalcRelative()
{
	Maths::C_Quaternion obo = Object().orient;
	
	m_relativeOrient = !obo * Orient();
	m_relativeOrient.Normalise();
	
	Maths::HVec3 rel = Position() - Object().position;
	m_relativePosition = (Maths::HVec3)( !obo * Maths::C_Quaternion(rel, Maths::HVecW(0.f)) * obo );
}

void C_RelativeCamera::CopyFrom(const C_Camera& other)
{
	C_Camera::CopyFrom(other);
	CalcRelative();
}

void RelativePlace(C_Camera& cam, const Maths::HVec3& relativePosition, const Maths::C_Quaternion& relativeOrient)
{
	// Get relative vector in world space:
	Maths::C_Quaternion obo = cam.Object().orient;
	Maths::HVec3 relative = (Maths::HVec3)(obo * Maths::C_Quaternion(relativePosition, Maths::HVecW(0.f)) * !obo);

	cam.SetPosition(cam.Object().position + relative);
	cam.SetOrient(obo * relativeOrient);
}


} // End of namespace Cam2
