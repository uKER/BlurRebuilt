
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/ChaseCamera2.h"

namespace Cam2
{

using namespace Maths;

static Debug::Bool  Override      ("Camera2.Chase2.Override", false);
static Debug::Float CustomDistance("Camera2.Chase2.Distance", 5.0f, Debug::Range( 0.f, 8.f));
static Debug::Float CustomHeight  ("Camera2.Chase2.Height", 1.5f, Debug::Range( 0.f, 2.f));
static Debug::Float CustomFov     ("Camera2.Chase2.Fov", 60.f, Debug::Range( 1.f, 90.f));
static Debug::Float CustomTilt    ("Camera2.Chase2.Tilt", -8.0f, Debug::Range( -20.f, 20.f));
static Debug::Float PivotZ        ("Camera2.Chase2.PivotZ", 0.f, Debug::Range(0.f, 1.f));
static Debug::Float BehindSpring  ("Camera2.Chase2.BehindSpring", 3.f, Debug::Range( 0.f, 20.f));
static Debug::Float AngularLimit  ("Camera2.Chase2.AngularLimit", 120.f, Debug::Range( 0.f, 360.f));
static Debug::Float AngularFade   ("Camera2.Chase2.AngularFade", 40.f, Debug::Range( 0.f, 180.f));
static Debug::Float g_LookAngle   ("Camera2.Chase2.LookAngle", 0.f, Debug::Range(-180.f, 180.f));
static Debug::Bool DeferredRaytrace("Camera2.Chase2.DeferredRaytrace", true);

C_ChaseCamera2::C_ChaseCamera2()
{
	SetType(E_ChaseCamera2);
	SetName("Chase2");
	SetParam(C_ChaseParam()); // Set defaults
	m_lookAngle = 0.f;
	Reset();
	m_physics = 0;
	m_RayHandle = -1;
}

void C_ChaseCamera2::OnUpdate()
{
	C_ChaseParam param = CurrentParam();

	HVec3 carOffset(0.f, 0.f, param.pivotZ);
	HVec3 pivot = Object().ToWorldSpace(carOffset);

	// Get follow position:
	HVec3 follow = m_trailPos - pivot;
	F32 scale = follow.Length();
	if (scale > 1e-5f)
		scale = param.distance / scale;
	follow *= scale;
	
	// Get travel position:
	HVec3 travel = m_trailPos - m_PrevPivot;
	scale = travel.Length();
	if (scale > 1e-5f)
		scale = param.distance / scale;
	travel *= scale;

	// Get behind position:
	HMat33 omat(Object().orient);
	HVec3 behind = HVec3(0.f, 0.f, -param.distance) * omat;
	scale = behind.Length();
	if (scale > 1e-5f)
		scale = param.distance / scale;
	behind *= scale;

	// Find the drive position, which is partway between 'behind' and 'travel'
	HVec3 drive = travel + (behind - travel) * param.behindSpring * Period();

	scale = drive.Length();
	if (scale > 1e-5f)
		scale = param.distance / scale;
	drive *= scale;

	// Find out whether we should use the drive position or the follow position:
	F32 angSpeed = RadToDeg(GetAngularSpeed());

	F32 driveFade = 1.f;
	if (param.angularFade > 1e-6f)
	{
		driveFade = (angSpeed - param.angularLimit) / param.angularFade;
		if (driveFade < 0.f)
			driveFade = 0.f;
		if (driveFade > 1.f)
			driveFade = 1.f;
	}
	
	HVec3 trailRel;
	if (m_reset)
	{
		trailRel = behind;
		m_reset = false;
	}
	else
	{
		trailRel = drive + (follow - drive) * driveFade;
	}
	
	m_trailPos = pivot + trailRel;

	// Do look around:
	F32 angle = m_lookAngle;
	if (Override)
		angle += DegToRad((F32)g_LookAngle);
	
	C_Quaternion look = C_Quaternion(0.0f, sin(-angle * 0.5f), 0.0f, cos(-angle * 0.5f));
	m_lookOrient = m_lookOrient.ShortestLerp(look, LerpValue(Period(), 0.02));
	m_lookOrient.Normalise();

	HVec3 lookPos = pivot + trailRel * HMat33(m_lookOrient);
	SetOrient(GetLookQuat(pivot - lookPos));

	HVec3 high = HVec3(0.f, (F32)param.height, 0.f);
	m_Want = lookPos + high + m_Offset * omat;

	m_Safe = Object().ToWorldSpace( HVec3(0.f, param.safeHeight, 0.f) );
	F32 trace = 1.f;
	m_RayHandle = -1;
	if (m_physics)
	{
		// If we are doing deferred raytraces, get a ray handle:
		if (DeferredRaytrace)
			m_RayHandle = m_physics->DeferredTrace(m_Safe, m_Want, 0.2f);

		// If we don't have a valid deferred handle for any reason, do an immediate raytrace:
		if (m_RayHandle < 0)
			trace = m_physics->RayTrace(m_Safe, m_Want, 0.2f);
	}
	
	if (m_RayHandle < 0)
		SetPosition(m_Safe + (m_Want - m_Safe) * trace);
	else
		SetPosition(m_Want);

	// Tilt down to look at the vehicle:
	F32 tilt = DegToRad(param.tilt) * 0.5f;
	SetOrient( Orient() * C_Quaternion(sin(tilt), 0.f, 0.f, cos(tilt)) );

	SetFov(DegToRad(param.fov));

	m_prevObject = Object();
	m_PrevPivot = pivot;
}

void C_ChaseCamera2::Update2()
{
	if (m_RayHandle < 0)
		return;

	F32 trace = 1.f;
	
	if (m_RayHandle >= 0 && m_physics)
		trace = m_physics->DeferredResult(m_RayHandle);
	
	SetPosition(m_Safe + (m_Want - m_Safe) * trace);
}

C_ChaseParam C_ChaseCamera2::CurrentParam()
{
	C_ChaseParam param = m_param;
	
	if (Override)
	{
		param.distance = CustomDistance;
		param.height   = CustomHeight;
		param.fov      = CustomFov;
		param.tilt     = CustomTilt;
		param.bank     = 0.0f;
		param.pivotZ   = PivotZ;
		param.behindSpring = BehindSpring;
		param.angularLimit = AngularLimit;
		param.angularFade = AngularFade;
	}

	return param;
}

F32 C_ChaseCamera2::GetAngularSpeed()
{
	C_Quaternion rel = !m_prevObject.orient * Object().orient;
	F32 imgLen = rel.Imag().Length();

	F32 speed = 0.f;
	if (imgLen > 0.f && imgLen <= 1.f)
	{
		speed = asin(imgLen) * 2.f;
		if (Period() > 1e-6f)
			speed /= Period();
	}
	return speed;
}

void C_ChaseCamera2::SetLookAround(F32 angle)
{
	m_lookAngle = angle;
}

// Set lookaround from stick values:
void C_ChaseCamera2::SetLookAround(F32 sx, F32 sy)
{
	F32 mag = sx * sx + sy * sy; 
	if (mag >= 0.2f * 0.2f)
		m_lookAngle = atan2(sx, sy); // Right stick being used to look around
	else
		m_lookAngle = 0.f; // Not pressed - look straight ahead:
}

// --------------------------

C_ChaseParam::C_ChaseParam()
{
	distance = 5.f;
	height = 1.5f;
	fov = 60.f;
	tilt = -8.f;
	lag = 0.1f;
	bank = 0.f;
	pivotZ = 0.f;
	behindSpring = 3.f;
	angularLimit = 120.f;
	angularFade = 40.f;
	safeHeight = 0.7f;
}

} // End of namespace Cam2
