
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/SimpleChaseCamera.h"

namespace Cam2
{

C_SimpleChaseCamera::C_SimpleChaseCamera()
{
	SetType(E_SimpleChaseCamera);
	SetName("SimpleChase");
	m_lookAngle = 0.f;
	SetParam(C_ChaseParam()); // Set defaults
	m_reset = true;
	m_physics = 0;
}

void C_SimpleChaseCamera::OnUpdate()
{
	C_ChaseParam param ;
	
	F32 distance = m_param.distance;
	F32 height = m_param.height;
	F32 fov = Maths::DegToRad(m_param.fov);
	F32 tilt = Maths::DegToRad(m_param.tilt);
	F32 lag = m_param.lag;
	F32 bank = m_param.bank;
	
	F32 period = Period();
	
	// Either jump to the vehicle orientation (for the first frame), or lerp to it:
	if (m_reset)
		m_chaseOrient = Object().orient;
	else
		m_chaseOrient = m_chaseOrient.ShortestLerp(Object().orient, LerpValue(period, lag));
	m_reset = false;

	m_chaseOrient.Normalise();

	Maths::C_Quaternion look =
		Maths::C_Quaternion( Maths::HVec3(0.f, 1.f, 0.f) * sin(-m_lookAngle * 0.5f), Maths::HVecW(cos(-m_lookAngle * 0.5f)));

	m_lookOrient = m_lookOrient.ShortestLerp(look, LerpValue(period, 0.02));
	m_lookOrient.Normalise();
	
	SetOrient( m_chaseOrient * m_lookOrient );

	Maths::HMat33 mat = Orient();
	// Level up the banking level and right:
	F32 roll = atan2(-mat.AX().Y(), mat.AY().Y()) * (1.f - bank);
	SetOrient( Orient() * Maths::C_Quaternion(Maths::HVec3(0.f, 0.f, sin(roll * -0.5f)), Maths::HVecW(cos(roll * -0.5f))));

	mat = Orient();
	Maths::HMat33 omat = Object().orient;
	Maths::HVec3 above  = Object().position + omat.AY();
	Maths::HVec3 behind = Object().position + omat.AY() * height + mat.AZ() * -distance;

	F32 trace = 1.f;
	if (m_physics)
		trace = m_physics->RayTrace(above, behind, 0.5f);

	SetPosition(above + (behind - above) * trace);
	SetOrient( Orient() * Maths::C_Quaternion(Maths::HVec3(sin(tilt * 0.5f), 0.f, 0.f), Maths::HVecW(cos(tilt * 0.5f))));
	SetFov(fov);
}

void C_SimpleChaseCamera::SetLookAround(F32 angle)
{
	m_lookAngle = angle;
}

// Set lookaround from stick values:
void C_SimpleChaseCamera::SetLookAround(F32 sx, F32 sy)
{
	F32 mag = sx * sx + sy * sy; 
	if (mag >= 0.2f * 0.2f)
		m_lookAngle = atan2(sx, sy); // Right stick being used to look around
	else
		m_lookAngle = 0; // Not pressed - look straight ahead:
}

} // End of namespace Cam2
