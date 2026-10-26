
#include "stdafx.h"
#include <camera2/CameraType.h>
#include <camera2/stock/RelativeFlyCamera.h>
#include "CameraInt.h"

namespace Cam2
{

C_RelativeFlyCamera::C_RelativeFlyCamera()
{
	SetType(E_RelativeFlyCamera);
	SetName("RelativeFly");
}

void C_RelativeFlyCamera::OnUpdate()
{
	RelativePlace(*this, m_relativePosition, m_relativeOrient);

	C_FlyCamera::OnUpdate();

	CalcRelative();
}

void C_RelativeFlyCamera::CalcRelative()
{
	Maths::C_Quaternion obo = Object().orient;
	
	m_relativeOrient = !obo * Orient();
	m_relativeOrient.Normalise();
	
	Maths::HVec3 rel = Position() - Object().position;
	m_relativePosition = (Maths::HVec3)( !obo * Maths::C_Quaternion(rel, Maths::HVecW(0.f)) * obo );
}

void C_RelativeFlyCamera::CopyFrom(const C_Camera& other)
{
	C_Camera::CopyFrom(other);
	CalcRelative();
}

// Virtual function:
void C_RelativeFlyCamera::KeyNow()
{
	AddKeyFrame(true, m_relativePosition, m_relativeOrient);
}


} // End of namespace Cam2
