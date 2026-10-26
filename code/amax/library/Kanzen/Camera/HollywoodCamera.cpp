
#include "stdafx.h"
#include "HollywoodCamera.h"
#include "camera2/CameraType.h"

#include <game/AmaxCam/Helper.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <game/AmaxCam/PhysicsChecks.h>

#include "System/debug/I_DebugRender.h"
#include "Utility.h"

namespace Kanzen
{

	using namespace Maths;

	static Debug::Bool  Override      ("Kanzen.Camera.HollywoodCamera.Override", false);
	static Debug::Float CustomDistance("Kanzen.Camera.HollywoodCamera.Distance", 5.0f, Debug::Range( 0.f, 8.f));
	static Debug::Float CustomHeight  ("Kanzen.Camera.HollywoodCamera.Height", 1.5f, Debug::Range( 0.f, 2.f));
	static Debug::Float CustomFov     ("Kanzen.Camera.HollywoodCamera.Fov", 60.f, Debug::Range( 1.f, 90.f));
	static Debug::Float CustomTilt    ("Kanzen.Camera.HollywoodCamera.Tilt", -8.0f, Debug::Range( -20.f, 20.f));
	static Debug::Float CustomBank    ("Kanzen.Camera.HollywoodCamera.Bank", 0.f, Debug::Range( 0.f, 1.f));
	static Debug::Float BehindSpring  ("Kanzen.Camera.HollywoodCamera.BehindSpring", 3.f, Debug::Range( 0.f, 20.f));
	static Debug::Float AngularLimit  ("Kanzen.Camera.HollywoodCamera.AngularLimit", 120.f, Debug::Range( 0.f, 360.f));
	static Debug::Float AngularFade   ("Kanzen.Camera.HollywoodCamera.AngularFade", 40.f, Debug::Range( 0.f, 180.f));

	static Debug::Float DebugHollywoodLimitDegrees  ("Kanzen.Camera.HollywoodCamera.HollywoodLimitDegrees", 60.f, Debug::Range( 0.f, 180.f));

	C_HollywoodCamera::C_HollywoodCamera()
	{
		SetType(Cam2::E_ChaseCamera2);
		SetName("Holly");
		m_lookAngle = 0.f;
		m_physics = 0;
		m_SafeHeight = 0.6f;
		VehicleReset();
	}

	void C_HollywoodCamera::VehicleReset()
	{
		m_trailPos=HVec3(0.f, 0.f, 0.f);
		SetOffset(HVec3(), 0.f);
		Reset();
	}



	// Get the look quaternion for looking in a direction
	C_Quaternion MyLookQuat(const HVec3& direction, HVec3 up)
	{
		HVec3 toz = direction;
		toz.Normalise();
		HVec3 tox = up ^ toz;
		tox.Normalise();
		HVec3 toy = toz ^ tox;
		toy.Normalise();

		HMat33 look;
		look.AX(tox);
		look.AY(toy);
		look.AZ(toz);

		return C_Quaternion(look);
	}

	void C_HollywoodCamera::OnUpdate()
	{
		const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);
		if (vehicleActor == 0)
			return;

		Cam2::C_CamObj object = AmaxCam::VehicleActorToCameraObject(vehicleActor);
		SetObject(object);

		SetPhysics(vehicleActor->GetPhysicsChecks());

		Cam2::C_ChaseParam param = CurrentParam();

		HVec3 opos = Object().position;

		// Get follow position:
		HVec3 follow = m_trailPos - opos;
		F32 scale = follow.Length();
		if (scale > 1e-5f)
			scale = param.distance / scale;
		follow *= scale;

		// Get travel position:
		HVec3 travel = m_trailPos - m_prevObject.position;
		scale = travel.Length();
		if (scale > 1e-5f)
			scale = param.distance / scale;
		travel *= scale;

		// Get behind position:
		

		C_Quaternion oQuat=Object().orient;

		float test = oQuat.Imag().X()*oQuat.Imag().Y() + oQuat.Imag().Z()*oQuat.Real().W();
		float attitude;

		if (test > 0.499f) // singularity at north pole
			attitude = Maths::PiBy2;
		else
		if (test < -0.499f) // singularity at south pole
			attitude = -Maths::PiBy2;
		else
			attitude = asin(2.f*test);

		C_Quaternion bankRemove(HVec3(0.f, 0.f, 1.f), -attitude);
		C_Quaternion bankingRemoved=bankRemove*oQuat;	

		HMat33 omat(bankingRemoved);
		HVec3 behind = HVec3(0.f, 0.f, -param.distance) * omat;
		scale = behind.Length();
		if (scale > 1e-5f)
			scale = param.distance / scale;
		behind *= scale;

		// Find the drive position, which is partway between 'behind' and 'travel'
		HVec3 drive = travel + (behind - travel) * param.behindSpring * Period();

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

		m_trailPos = opos + trailRel;

		// Do look around:
		C_Quaternion look =
			C_Quaternion( omat.AY() * sin(-m_lookAngle * 0.5f), HVecW(cos(-m_lookAngle * 0.5f)));
		m_lookOrient = m_lookOrient.ShortestLerp(look, LerpValue(Period(), 0.02));
		m_lookOrient.Normalise();

		HVec3 lookPos = opos + trailRel * HMat33(m_lookOrient);
	
		HVec3 up(omat.AY());

		C_Quaternion orient=MyLookQuat(opos - lookPos, up);

		// Hollywoodism
		C_Quaternion finalOrient=Object().orient;
		C_Quaternion diff=orient*(!finalOrient);

		float angleDiff=fabs(acos(diff.CosTheta()));

		float hollywoodLimit=Maths::DegToRad(DebugHollywoodLimitDegrees.Value());

		if (angleDiff>hollywoodLimit)
		{
			finalOrient=Object().orient.ShortestSlerp(orient, 1.0f-hollywoodLimit/angleDiff);
		}

		test = finalOrient.Imag().X()*finalOrient.Imag().Y() + finalOrient.Imag().Z()*finalOrient.Real().W();

		if (test > 0.499f) // singularity at north pole
			attitude = Maths::PiBy2;
		else
		if (test < -0.499f) // singularity at south pole
			attitude = -Maths::PiBy2;
		else
			attitude = asin(2.f*test);

		bankRemove=C_Quaternion(HVec3(0.f, 0.f, 1.f), -attitude);
		bankingRemoved=bankRemove*finalOrient;

		SetOrient(bankingRemoved);

		HVec3 high = up * (F32)param.height;
		HVec3 want = lookPos + high + m_Offset * omat;

		HVec3 safe = Object().position + up * m_SafeHeight; //HVec3(0.f, m_SafeHeight, 0.f);
		F32 trace = 1.f;
		if (m_physics)
			trace = m_physics->RayTrace(safe, want, 0.2f);
		SetPosition(safe + (want - safe) * trace);	
		// Tilt down to look at the vehicle:
		F32 tilt = DegToRad(param.tilt) * 0.5f;
		SetOrient( Orient() * C_Quaternion(sin(tilt), 0.f, 0.f, cos(tilt)) );

		SetFov(DegToRad(param.fov));

		m_prevObject = Object();

		SetPhysics(0); // blank physics checks pointer, just in case
	}


	Cam2::C_ChaseParam C_HollywoodCamera::CurrentParam()
	{
		Cam2::C_ChaseParam param = m_param;

		if (Override)
		{
			param.distance = CustomDistance;
			param.height   = CustomHeight;
			param.fov      = CustomFov;
			param.tilt     = CustomTilt;
			param.bank     = CustomBank;
			param.behindSpring = BehindSpring;
			param.angularLimit = AngularLimit;
			param.angularFade = AngularFade;
		}

		return param;
	}

	F32 C_HollywoodCamera::GetAngularSpeed()
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

	void C_HollywoodCamera::SetLookAround(F32 angle)
	{
		m_lookAngle = angle;
	}

	// Set lookaround from stick values:
	void C_HollywoodCamera::SetLookAround(F32 sx, F32 sy)
	{
		F32 mag = sx * sx + sy * sy; 
		if (mag >= 0.2f * 0.2f)
			m_lookAngle = atan2(sx, sy); // Right stick being used to look around
		else
			m_lookAngle = 0.f; // Not pressed - look straight ahead:
	}

	void C_HollywoodCamera::SetOffset(const Maths::HVec3& offset, F32 fov)
	{
		SetOffset(offset);

		Cam2::C_ChaseParam param;
		param.fov = 53.f + fov;

		SetParam(param);
	}

} // End of namespace Kanzen
