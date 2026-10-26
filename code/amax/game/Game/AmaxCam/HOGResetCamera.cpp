
#include "stdafx.h"
#include "HOGResetCamera.h"
#include "Helper.h"
#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <../library/kanzen/camera/Utility.h>


static Debug::Float g_DebugHogCameraOrientShakeFrequency("Camera2.HogReset.Orient.Frequency", 2.7f, Debug::Range(0.0f, 20.0f));
static Debug::Float g_DebugHogCameraOrientShakeXScale("Camera2.HogReset.Orient.XScale", 2.7f, Debug::Range(0.0f, 20.0f));
static Debug::Float g_DebugHogCameraOrientShakeYScale("Camera2.HogReset.Orient.YScale", 2.7f, Debug::Range(0.0f, 20.0f));
static Debug::Float g_DebugHogCameraOrientShakeZScale("Camera2.HogReset.Orient.ZScale", 0.0f, Debug::Range(0.0f, 20.0f));

static Debug::Float g_DebugHogCameraVelocityDamping("Camera2.HogReset.Position.Damping", 0.70f, Debug::Range(0.0f, 1.0f));

static Debug::Float g_DebugHogCameraOrientDamping("Camera2.HogReset.Orient.Damping", 0.90f, Debug::Range(0.0f, 1.0f));

static Debug::Float g_DebugHogCameraShakeDamping("Camera2.HogReset.Shake.Damping", 0.96f, Debug::Range(0.0f, 1.0f));

namespace AmaxCam
{

	using namespace Game::Actors;
	using namespace Maths;

	C_HOGResetCamera::C_HOGResetCamera()
	{
		SetName("HOGReset");
	}

	C_HOGResetCamera::~C_HOGResetCamera()
	{
	}

	void C_HOGResetCamera::Start(const Actor::C_ActorHandle& handle, const Cam2::C_Camera& startFrom)
	{
		const C_VehicleActor* vehicleActor = GetVehicleActor(handle);
		if(vehicleActor)
			mVehicleIndex = vehicleActor->GetVehicleIndex();
		else
			mVehicleIndex = -1;

		CopyFrom(startFrom);
		//Update(0.f);
		mTimer = 0.0f;
		mOrientBlend = 0.0f;
		mShakeBlend = 0.0f;
		SetInitialVelocity(startFrom.Velocity());
		mStartingOrient = Orient();
	}

	void C_HOGResetCamera::GetObject()
	{
		if(mVehicleIndex != -1)
		{
			const C_VehicleActor* vehicleActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetVehicleActor(mVehicleIndex);

			if(vehicleActor)
			{
				Cam2::C_CamObj object = VehicleActorToCameraObject(vehicleActor);
				SetObject(object);
			}
		}
	}


	void C_HOGResetCamera::OnUpdate()
	{	
		GetObject();

		mTimer+= Period();
			
		SetShake(Maths::HVec3(	g_DebugHogCameraOrientShakeXScale.Value() * mShakeBlend,
								g_DebugHogCameraOrientShakeYScale.Value() * mShakeBlend,
								g_DebugHogCameraOrientShakeZScale.Value()) * mShakeBlend,
								g_DebugHogCameraOrientShakeFrequency.Value());


		// Update the velocity.
		// We just want to quickly ramp it down to zero
		F32 velocityMag = mVelocity.Length();
		F32 velocityDesired = 0.0f;

		if(velocityMag>0.001f)
		{
			F32 velocityNew = velocityMag + ((velocityDesired - velocityMag) * Kanzen::DampingAdjust( g_DebugHogCameraVelocityDamping, Period() ));
			mVelocity *= velocityNew/velocityMag;
		}
		SetPosition( Position() + (mVelocity * Period()));

		mOrientBlend += (1.0f - mOrientBlend) * Kanzen::DampingAdjust( g_DebugHogCameraOrientDamping, Period() );
		mShakeBlend += (1.0f - mShakeBlend) * Kanzen::DampingAdjust( g_DebugHogCameraShakeDamping, Period() );

		Maths::C_Quaternion desiredOrient = mStartingOrient;
		if(mCameraTracking)
		{
			desiredOrient = GetLookQuat((Object().ToWorldSpace(HVec3(0.f, 2.f, 2.f)))- Position());
		}
		Maths::C_Quaternion finalOrient = mStartingOrient.ShortestSlerp(desiredOrient,mOrientBlend);
		SetOrient(finalOrient);
	}

} // End of namespace
