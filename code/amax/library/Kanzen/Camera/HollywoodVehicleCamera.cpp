//=============================================================================
// HollywoodVehicleCamera.cpp
// 25/01/09
//=============================================================================
//=============================================================================
// Includes
//=============================================================================
#include "stdafx.h"
#include "Kanzen_PCH.h"
#include "HollywoodVehicleCamera.h"
#include "camera2/CameraType.h"
#include <game/AmaxCam/CameraSystem.h>
#include <game/AmaxCam/Helper.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <game/AmaxCam/PhysicsChecks.h>
#include <game/AmaxCam/CameraDollyZoomSpecialFX.h>
#include <game/AmaxCam/CameraNoiseSpecialFX.h>
#include <GameLogic\GameData\GameVehicleData.h>
#include "System/debug/I_DebugRender.h"
#include "racingphysics/physics/geometry.h"
#include "racingphysics/vehicles/car/car.h"
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <vehicle\VehicleModel.h>
#include <Render/View/ViewManager.h>
#include "vehicle/input.h"
#include "Utility.h"
#include "Util/Graph.h"
#include "Game/GameDef/Polyflags.h"
#include <Game/Game.h>
#include "system/misc/Random.h"


#define HOLLYWOOD_CAMERA_GRAPHS 0
//=============================================================================
// Class Implementation
//=============================================================================

Util::C_Random g_HollyWoodcameraRandom;

#ifndef CFG_GOLD
extern Debug::Bool g_debugUiHUDShowCameraHelpers;
#endif
namespace Kanzen
{
	using namespace Maths;

	//=========================================================================
	// Debug Variables
	//=========================================================================
#ifndef CFG_GOLD
	static Debug::Bool DebugSimulateAirbourneTimeEnable("Kanzen.Camera.HollywoodVehicleCamera.Simulate.AirbourneTimeEnable", false);
	static Debug::Float DebugSimulateAirbourneTime("Kanzen.Camera.HollywoodVehicleCamera.Simulate.AirbourneTime", 0.0f, Debug::Range( 0.f, 100.f));
	static Debug::Bool DebugSimulateLinearVelocityEnable("Kanzen.Camera.HollywoodVehicleCamera.Simulate.LinearVelocityEnable", false);
	static Debug::Vector DebugSimulateLinearVelocity("Kanzen.Camera.HollywoodVehicleCamera.Simulate.LinearVelocity",  0.0f, 0.0f, 0.0f);

	static Debug::Bool OverrideTransform("Kanzen.Camera.HollywoodVehicleCamera.OverrideTransform.Override", false);
	static Debug::Float OverrideTransformPitch("Kanzen.Camera.HollywoodVehicleCamera.OverrideTransform.Pitch", 0.0f, Debug::Range(-180.0f, 180.f));
	static Debug::Float OverrideTransformYaw("Kanzen.Camera.HollywoodVehicleCamera.OverrideTransform.Yaw", 0.0f, Debug::Range(-180.0f, 180.f));
	static Debug::Float OverrideTransformRoll("Kanzen.Camera.HollywoodVehicleCamera.OverrideTransform.Roll", 0.0f, Debug::Range(-180.0f, 180.f));
	static Debug::Bool OverrideTransformRender("Kanzen.Camera.HollywoodVehicleCamera.OverrideTransform.DebugDraw", false);
	static Debug::Float AntiPendulumDesiredOverride("Kanzen.Camera.HollywoodVehicleCamera.OverrideTransform.AntiPendulumDesired", -1.0f, Debug::Range(-1.0f, 1.0f));
	
	
#endif

	Debug::Bool DebugOverrideTransX("Kanzen.Camera.HollywoodVehicleCamera.TransX.Override", false);
	Debug::Float DebugOverrideTransXValue("Kanzen.Camera.HollywoodVehicleCamera.TransX.OverrideValue", 0.0f, Debug::Range(-2.0f, 2.0f));
	Debug::Bool DebugOverrideLookSide("Kanzen.Camera.HollywoodVehicleCamera.LookSide.Override", false);
	Debug::Float DebugOverrideLookSideValue("Kanzen.Camera.HollywoodVehicleCamera.LookSide.OverrideValue", 0.0f, Debug::Range(-3.0f, 3.0f));

	Debug::Float DebugMaxFovScaleDiff("Kanzen.Camera.HollywoodVehicleCamera.FovScale.MaxFovScake", 0.3f, Debug::Range(0.0f, 1.0f));
	Debug::Float DebugMaxFovScaleScale("Kanzen.Camera.HollywoodVehicleCamera.FovScale.Scale", 0.15f, Debug::Range(0.0f, 3.0f));


	Debug::Float s_cameraDisplacementCorrection("Kanzen.Camera.HollywoodVehicleCamera.DollyZoom.CorrectionFactor", 2.0f, Debug::Range(-10.0f, 10.0f));
	
	static Debug::Bool HollywoodCamUseEffects("Kanzen.Camera.HollywoodVehicleCamera.UseEffects",true);
	static Debug::Float HollywoodCamViewDirOffsetOverride("Kanzen.Camera.HollywoodVehicleCamera.ViewDir.Override",-1.0f,Debug::Range(0.0f,1.0f));



	//=====================================================================
	// Internal Data
	//=====================================================================

	static F32 s_antiPendulumSmoothUp = 0.25f;
	static F32 s_antiPendulumSmoothDown = 1.00f;
	static F32 s_ReversingVelocityThreshold = -7.5f;
	static F32 s_ReversingTimeThreshold = 1.0f;
	static F32 s_ReversingSideVelocityThreshold = 3.0f;
	static F32 s_vehiclePitchMin = Maths::DegToRad(45.0f);
	static F32 s_minAirbourneTimeForPitchDeviation = 0.15f;
	static F32 s_LinVelZToExitBurnout = 3.0f;
	static F32 s_distanceAccelerationThreshold = 0.1f;
	static F32 s_distanceDecelerationThreshold = -0.1f;
	static F32 s_minAirbourneTimeBeforeResettingDistance = 0.3f;
	static F32 s_antiPendulumTimeout = 1.0f;
	static F32 s_antiPendulumWheelWithoutContactTime = 0.2f;
	static F32 s_antiPendulumUpDotThreshold = 0.820f;
	static F32 s_antiPendulumMinAirbourneTime = 0.5f;
	
#if HOLLYWOOD_CAMERA_GRAPHS

	static Graph s_WheelSlipGraph = { 100.0f, 100.0f, 200.0f, 150.0f, 3.0f, -3.0f };
	static GraphData s_TransXTarget;
	static GraphData s_TransXCurrent;
	static GraphData s_BackWheelSlipData;
	static GraphData s_FrontWheelSlipData;
	static GraphData s_AllWheelSlipData;
	static GraphData s_YawVelocityData;

	C_HollywoodVehicleCamera* s_MainHollywoodCamera = NULL;
#endif
	//=====================================================================
	// Class Implementation
	//=====================================================================
	//---------------------------------------------------------------------
	// C_HollywoodVehicleCamera::hollywoodDataCache::Reset
	//---------------------------------------------------------------------
	void
	C_HollywoodVehicleCamera::hollywoodDataCache::Reset()
	{
		m_actor = NULL;
		m_physics = NULL;
		m_input = NULL;
		m_linVel = Maths::HVec3();
		m_linVelLS = Maths::HVec3();
		m_angVel = Maths::HVec3();
		m_angVelLS = Maths::HVec3();
		m_linVelMag = Maths::Constants<F32>::Zero();
		m_angVelMag = Maths::Constants<F32>::Zero();
		m_airbourneTime = Maths::Constants<F32>::Zero();
	
		
	}
	
	//---------------------------------------------------------------------
	// C_HollywoodVehicleCamera::hollywoodPersistantDataCache::Reset
	//---------------------------------------------------------------------
	void
	C_HollywoodVehicleCamera::hollywoodPersistantDataCache::Reset()
	{
		for(int i=0;i<NUM_WHEELS;i++)
		{
			m_wheelWithoutContactTime[i] = Maths::Constants<F32>::Zero();
		}
		m_dampedThrottle = 0.0f;
		m_antiPendulumTimer = -1.0f;
		m_landedTimer = -1.0f;
		m_reversing = false;
		m_reversingTimer= 0.0f;
		m_jTurning = false;
		m_highSpeed = false;
		m_groundTime = Maths::Constants<F32>::Zero();
		m_fixedFovWidth = 0.0f;
		
	}

	//---------------------------------------------------------------------
	// Constructor/Destructor
	//---------------------------------------------------------------------
	C_HollywoodVehicleCamera::C_HollywoodVehicleCamera()
	{
		//m_defaultParameters = ExtractDefaultParamsFromDebugVars();
		//m_currentParameters = m_defaultParameters;

		SetType(Cam2::E_ChaseCamera2);
		SetName("Holly");
		m_lookAngle = 0.f;
		m_lookX = 0.0f;
		m_lookY = 0.0f;
		m_physics = 0;
		m_SafeHeight = 1.0f;
		VehicleReset();
		BlankParams();
		m_debugIndex = -1;
		m_Raytrace = true;
		m_tracking = true;
		m_Wrecked = false;
	
	}


	//---------------------------------------------------------------------
	// SetHandle
	// update our tracking actor handle and conditionally reset the camera
	//---------------------------------------------------------------------
	void 
	C_HollywoodVehicleCamera::SetHandle(const Actor::C_ActorHandle& handle)
	{
		bool reset = m_Handle != handle;
		m_Handle = handle;

		if( reset )
		{
			VehicleReset();
		}
	}

	//---------------------------------------------------------------------
	// VehicleReset
	// The vehicle has been reset so re-initialise the camera to it's default
	// state.
	//---------------------------------------------------------------------
	void 
	C_HollywoodVehicleCamera::VehicleReset()
	{
		const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);

		if (vehicleActor)
			m_currentParameters = ExtractParamsFromSourcery();
		const HollywoodVehicleParams& params = GetParams();

		m_tracking = true;
		m_burningout = false;
		m_burningoutTimer = 0.0f;
		m_lostControl = false;
		m_Wrecked = false;
		m_wreckedIdealPitch= 0.0f;
		m_WreckedTimer = 0.0f;
		m_crashedTransformSmoothData.Reset();
		m_antiPendulumTransformSmoothData.Reset();
		m_lookSideSmoothData.Reset();
		m_lookAngleSmoothData.Reset();
		m_MaxDistanceSmoothData.Reset();
		m_MaxDistanceSmoothData.targetValue = params.IdealDistanceNormal * 2.0f;
		m_MaxDistanceSmoothData.currentValue = m_MaxDistanceSmoothData.targetValue;
		m_lookUpDownSmoothData.Reset();
		m_yawDampingSmoothData.Reset();
		m_yawDampingSmoothData.currentValue = params.YawDampingNormal;
		m_rollDampingSmoothData.Reset();
		m_rollDampingSmoothData.currentValue = 0.0f;
		m_distanceSmoothData.Reset();
		m_distanceSmoothData.currentValue = params.IdealDistanceNormal;
		m_distanceSmoothSmoothData.Reset();
		m_distanceSmoothSmoothData.currentValue = params.IdealDistanceSmoothing;
		m_pitchDampingSmoothData.Reset();
		m_pitchDampingSmoothData.currentValue = params.PitchDampingNormal;
		m_ThrottleSmoothData.Reset();
		m_FovSmoothData.Reset();
		m_FovSmoothData.currentValue = params.NormalFOV;
		m_cameraTransformXOffset.Reset();
		m_persistantData.Reset();
		m_persistantDataLast.Reset();
		m_lookBackQuick = false;
		m_lookSide = false;
		m_lookDeadZone = 0.0f;

		
		if( vehicleActor )
		{
			HMat43 vehicleTransform;
			vehicleTransform = vehicleActor->GetRawMatrix();	

			
			SetFov(m_FovSmoothData.currentValue);
			SetDistanceTrackingEnabled(true);
			SetPitchTrackingEnabled(true);
			SetYawTrackingEnabled(true);
			SetRollTrackingEnabled(true);
			SetIdealDistance(m_distanceSmoothData.currentValue);
			SetIdealDistanceDamping(0.0f);
			SetIdealDistanceMaxSpeed(-1.0f);
			SetIdealYaw(0.0f);
			SetIdealYawDamping(m_yawDampingSmoothData.currentValue);
			SetIdealYawMaxSpeed(params.YawMaxSpeedNormal<0.0f? -1.0f:params.YawMaxSpeedNormal);
			SetIdealYawMaxError(params.YawMaxErrorNormal<0.0f? -1.0f:params.YawMaxErrorNormal);
			SetIdealPitch(params.IdealPitch);
			SetIdealPitchDamping(m_pitchDampingSmoothData.currentValue);
			SetIdealPitchMaxSpeed(params.PitchMaxSpeedNormal<0.0f? -1.0f:params.PitchMaxSpeedNormal);
			SetIdealPitchMaxError(params.PitchMaxErrorNormal<0.0f? -1.0f:params.PitchMaxErrorNormal);
			SetIdealRoll(DegToRad(0.0f));
			SetIdealRollDamping(m_rollDampingSmoothData.currentValue);
			SetIdealRollMaxSpeed(-1.0f);
			SetUpAxis(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
			Setup( vehicleTransform,
				   Maths::HVec3(params.IdealBase.vx, params.IdealBase.vy, params.IdealBase.vz),
				   Maths::HVec3(params.LookAtPos.vx, params.LookAtPos.vy, params.LookAtPos.vz),
				   params.IdealPitch,
				   DegToRad(0.f),
				   DegToRad(0.f),
				   params.IdealDistanceNormal);

			m_unmodifiedPosition = Position();
			m_unmodifiedRotation = Orient();
			
		}

		ResetParams();
		
	}

	
	//---------------------------------------------------------------------
	// OnUpdate
	// Set the parameters of the thirdperson camera based on the current
	// state of the vehicle.
	//---------------------------------------------------------------------
	void C_HollywoodVehicleCamera::OnUpdate()
	{
		PERF_EVENT("HollyCam.OnUpdate");

#if HOLLYWOOD_CAMERA_GRAPHS
		if(!s_MainHollywoodCamera)
		{
			s_MainHollywoodCamera = this;
		}
#endif

		const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);
		if(vehicleActor)
		{
			int i;
			F32 timeElapsed = Period();

			Maths::HVec3 lastFrameFinalPos = Position();
			Maths::C_Quaternion lastFrameFinalOrient = Orient();

			// Before we do anything we should return the camera
			// position/orientation values to the ones before
			// we modified them last frame. This ensures consistency
			// in the ThirdPersonCamera tracking functionality.
			if(m_tracking)
			{
				SetPosition(m_unmodifiedPosition);
				SetOrient(m_unmodifiedRotation);
			}
			
			const Game::Actors::C_VehicleActorCreate *pVehicleCreate = (const Game::Actors::C_VehicleActorCreate *)(vehicleActor->GetCreationData().GetData());
			S32 viewportIndex = -1;
			if(pVehicleCreate)
			{
				viewportIndex = pVehicleCreate->m_pRLC->GetViewportIndex(vehicleActor->GetVehicleIndex());
			}
			
			//---------------------------------------------------------------
			// Standstill noise
#ifndef CFG_GOLD
			if(g_debugUiHUDShowCameraHelpers)
			{
				SetShakeXYZ(0.0f,0.0f,0.0f,0.5f);
			}
			else
#endif
			{
				SetShakeXYZ(1.0f,1.0f,0.0f,0.5f);
				
			}
			//---------------------------------------------------------------
			// Noise
			if(HollywoodCamUseEffects)
			{
				// Query the SFX system for any noise
				const AmaxCam::C_CameraNoiseSpecialFX::ActiveNoiseList& NoiseList = AmaxCam::C_CameraNoiseSpecialFX::GetActiveNoiseList();
				for(AmaxCam::C_CameraNoiseSpecialFX::ActiveNoiseList::ConstIterator pIterator = NoiseList.Begin(); pIterator != NoiseList.End(); ++pIterator)
				{
					if( pIterator->m_ViewportID == viewportIndex )
					{
						AddOffset(pIterator->m_Position,pIterator->m_Orientation, 0.0f);
					}
				}
			}

			

			SetOffset(m_EffectOffset);
			
			Maths::HMat33 cameraRotation(m_unmodifiedRotation);

			// Get our parameter data
			m_currentParameters = ExtractParamsFromSourcery();
			HollywoodVehicleParams params = GetParams();
			
			// Because the 'params' struct can be dynamically 
			// modified make sure we pass on all changes to the camera
			
			// These next few functions are called here as the values
			// are set directly from the parameters. There is no
			// additional logic required. 
			SetTargetPos(Maths::HVec3(params.IdealBase.vx, params.IdealBase.vy, params.IdealBase.vz));
			SetIdealPitchMaxSpeed(params.PitchMaxSpeedNormal<0.0f? -1.0f:params.PitchMaxSpeedNormal);
			SetIdealYawMaxSpeed(params.YawMaxSpeedNormal<0.0f? -1.0f:params.YawMaxSpeedNormal);
			
			// Update the tracking object
			Cam2::C_CamObj object = AmaxCam::VehicleActorToCameraObject(vehicleActor);
			SetObject(object);

			

			// Update our parameters
			
			
			// State update
			RacingPhysics::Car* physics = vehicleActor->GetVehiclePhysics();
			BIZ_ASSERT(physics);
			BIZ_ASSERT(physics->GetNumWheels() <= NUM_WHEELS);
			
			Vehicle::C_ModelInput* input = vehicleActor->GetVehicleInput();

			hollywoodDataCache data;
			//---------------------------------------------------------------
			// Calculate and cache commonly useful data
		
			data.m_actor = vehicleActor;
			data.m_physics = physics;
			data.m_input = input;
			data.m_angVel = physics->GetAngularVelocity();
			data.m_linVel = physics->GetVelocity();

			HMat33 inverseVehicleTransform(vehicleActor->GetRawMatrix().InverseRotTrans());
			
			data.m_angVelLS = data.m_angVel * inverseVehicleTransform;
			data.m_linVelLS = data.m_linVel * inverseVehicleTransform;
			data.m_angVelMag = data.m_angVel.Length();
			data.m_linVelMag = data.m_linVel.Length();
			data.m_linAccel = input->m_vAcceleration;
			data.m_linAccelLS = data.m_linAccel * inverseVehicleTransform;
			data.m_airbourneTime = Maths::Constants<F32>::Max();
			bool anyOnGround = false;
			for(i=0;i<physics->GetNumWheels();i++)
			{
				anyOnGround = physics->WheelOnGround(i)?true:anyOnGround;
				m_persistantData.m_wheelWithoutContactTime[i] = physics->WheelOnGround(i) ? 0.0f:m_persistantData.m_wheelWithoutContactTime[i] + timeElapsed;
				data.m_airbourneTime = Maths::Min(data.m_airbourneTime,m_persistantData.m_wheelWithoutContactTime[i]);	
			}


			static F32 s_maxOnGroundTime = 10.0f;
			m_persistantData.m_groundTime = anyOnGround?m_persistantData.m_groundTime+timeElapsed:0.0f;
			m_persistantData.m_groundTime = Maths::Clamped<F32>(m_persistantData.m_groundTime,0.0f,s_maxOnGroundTime);

			data.m_vehicleTransform = vehicleActor->GetRawMatrix();

			static F32 throttleSmoothing = 1.5f;
			m_ThrottleSmoothData.targetValue = input->m_fBrake > 0.1f ?-input->m_fBrake:input->m_fThrottle;
			m_ThrottleSmoothData.targetValue = Maths::Clamped<F32>(m_ThrottleSmoothData.targetValue,-1.0f,1.0f);
			m_ThrottleSmoothData.currentValue = SmoothingFunction(	m_ThrottleSmoothData.currentValue, 
																	m_ThrottleSmoothData.targetValue, 
																	m_ThrottleSmoothData.velocity,
																	throttleSmoothing,
																	timeElapsed);
			m_persistantData.m_dampedThrottle = m_ThrottleSmoothData.currentValue;


		


#ifndef CFG_GOLD
			if(OverrideTransform)
			{
				static bool rph = true;
				C_MatrixRotateRPH eulersrph(Maths::DegToRad(OverrideTransformRoll.Value()),
											Maths::DegToRad(OverrideTransformPitch.Value()),
											Maths::DegToRad(OverrideTransformYaw.Value()));

				C_MatrixRotatePHR eulersphr(Maths::DegToRad(OverrideTransformPitch.Value()),
											Maths::DegToRad(OverrideTransformYaw.Value()),
											Maths::DegToRad(OverrideTransformRoll.Value()));
				;
				data.m_vehicleTransform = HMat43(rph?Maths::HMat33(eulersrph):Maths::HMat33(eulersphr),data.m_vehicleTransform.AW());
			}

			if(OverrideTransformRender)
			{
				DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
				debugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);
				debugRender->Draw3dBoxLit(Maths::HVec3(-1.0f,-0.75f,-2.0f),Maths::HVec3(1.0f,0.75f,2.0f),data.m_vehicleTransform,Maths::RGBA(30,50,180,170));
			}
#endif			

			data.m_vehicleUpDotWorldUp = DotProduct(data.m_vehicleTransform.AY(), Constants<Maths::HVec3>::UnitAxisPositiveY());
			
			C_MatrixRotateRPH rot = HMat33(data.m_vehicleTransform).GetRotationRPH();
			data.m_vehicleRoll = CalculateRollOfTransform(HMat33(data.m_vehicleTransform));
			data.m_vehiclePitch = fabs(rot.Pitch().m_AngleR);
			
			if(data.m_vehicleUpDotWorldUp<0.0f)
			{
				data.m_vehiclePitch = Maths::PiBy2 + (Maths::PiBy2-Maths::Clamped<F32>(data.m_vehiclePitch,0.0f,Maths::PiBy2));
			}

			// Wheel Slips
			data.m_FrontWheelsSlip = (input->m_Wheels[0].m_fSlipAngle * (input->m_Wheels[0].m_bOnGround?1.0f:0.0f)) +
									 (input->m_Wheels[1].m_fSlipAngle * (input->m_Wheels[1].m_bOnGround?1.0f:0.0f));
			data.m_BackWheelSlip   = (input->m_Wheels[2].m_fSlipAngle * (input->m_Wheels[2].m_bOnGround?1.0f:0.0f)) +
									 (input->m_Wheels[3].m_fSlipAngle * (input->m_Wheels[3].m_bOnGround?1.0f:0.0f));

#if HOLLYWOOD_CAMERA_GRAPHS
			if(this==s_MainHollywoodCamera)
			{
				s_BackWheelSlipData.SetValue(data.m_BackWheelSlip);
				s_FrontWheelSlipData.SetValue(data.m_FrontWheelsSlip);
				s_AllWheelSlipData.SetValue(data.m_BackWheelSlip+data.m_FrontWheelsSlip);
				
				GraphRenderer renderer;
				renderer.RenderAxis(s_WheelSlipGraph);
				renderer.RenderData(s_WheelSlipGraph,s_BackWheelSlipData,Maths::RGBA(255,0,0,255));
				renderer.RenderData(s_WheelSlipGraph,s_TransXTarget,Maths::RGBA(0,255,0,255));
				renderer.RenderData(s_WheelSlipGraph,s_TransXCurrent,Maths::RGBA(0,0,255,255));
				//renderer.RenderData(s_WheelSlipGraph,s_YawVelocityData,Maths::RGBA(255,0,255,255));
			}
#endif
			//--------------------------------------------------------------
			// Simulate overrides - Allows us to set up the conditions to 
			// test any particular parameters.
#ifndef CFG_GOLD
			if(DebugSimulateAirbourneTimeEnable)
			{
				data.m_airbourneTime = DebugSimulateAirbourneTime;
			}
			if(DebugSimulateLinearVelocityEnable)
			{
				data.m_linVel.X(DebugSimulateLinearVelocity.Value().vx);
				data.m_linVel.Y(DebugSimulateLinearVelocity.Value().vy);
				data.m_linVel.Z(DebugSimulateLinearVelocity.Value().vz);
			}
#endif //CFG_GOLD

			//---------------------------------------------------------------
			// Wrecked
			if( !m_Wrecked )
			{
				SetRollTrackingEnabled(true);
				if(data.m_actor->GetLostControl())
				{	
					static F32 s_wreckedIdealPitchMin = -.15f;
					static F32 s_wreckedIdealPitchMax = 0.10f;
					m_Wrecked = true;
					m_wreckedIdealPitch = g_HollyWoodcameraRandom.FRand(s_wreckedIdealPitchMin, s_wreckedIdealPitchMax);
					m_WreckedCamPos = lastFrameFinalPos;
				}
			}
			else
			{
				SetRollTrackingEnabled(false,false);
			}

			//---------------------------------------------------------------
			// Wrecked	
			if( m_Wrecked )
			{
				m_WreckedTimer += timeElapsed;
				SetRollTrackingEnabled(false,false);
			}
			else
			{
				SetRollTrackingEnabled(true);
			}
			//---------------------------------------------------------------
			// Reversing
			if( m_persistantData.m_reversing )
			{
				static F32 s_breakOutOfReverseZVelocity = 2.0f;
				static F32 s_breakOutOfReverseYAngularVelocity = 0.3f;
				static F32 s_angularVelocityForJTurn = 1.0f;
				bool jturning = fabs(data.m_angVelLS.Y()) > s_angularVelocityForJTurn;
				bool accelerating = (input->m_fThrottle > 0.1f && !physics->Reverse());
				bool travellingForwards = data.m_linVelLS.Z() > s_breakOutOfReverseZVelocity ?true:false;
				bool spinning = fabs(data.m_angVelLS.Y()) > s_breakOutOfReverseYAngularVelocity?true:false;

				if(jturning && !m_persistantData.m_jTurning)
				{
					m_persistantData.m_jTurning = true;
				}
			
				// We have two different sets of exit conditions from the 
				// reversing state. Dependant on whether or not a j-turn
				// maneuver was performed.
				if(m_persistantData.m_jTurning)
				{
					if(!spinning)
					{
						SetYawTrackingEnabled(false,false);
						SetIdealYawMaxErrorSoften(true);
						if(accelerating)
						{
							SetYawTrackingEnabled(true);
							m_persistantData.m_reversing = false;
							m_persistantData.m_jTurning = false;
						}
						else if( physics->Reverse() && data.m_linVelLS.Z() < s_ReversingVelocityThreshold)
						{
							SetYawTrackingEnabled(true);
							m_persistantData.m_jTurning = false;
						}
					}
				}
				else
				{
					if(accelerating || travellingForwards)
					{
						SetIdealYawMaxErrorSoften(true);
						m_persistantData.m_reversing = false;
						m_persistantData.m_jTurning = false;
					}
				}
			}
			else
			{
				bool reversing = physics->Reverse() && 
								 input->m_fThrottle > 0.1f &&
								 data.m_linVelLS.Z() < s_ReversingVelocityThreshold && 
								 !m_lostControl;
				if(reversing)
				{
					m_persistantData.m_reversingTimer += timeElapsed;
					if( m_persistantData.m_reversingTimer > s_ReversingTimeThreshold)
					{
						m_persistantData.m_reversing = true;
						m_persistantData.m_jTurning = false;
						m_persistantData.m_reversingTimer = 0.0f;
					}
					
				}
				else
				{
					m_persistantData.m_reversingTimer = 0.0f;
				}
			}

		
			//---------------------------------------------------------------
			// Crash detection

			// Update our wheel states. We keep track of how long each wheel 
			// has been airbourne and use the results in the logic that follows			
			int numWheelsWithoutContact = 0;
			int numWheelsWithContact;
			
			for(i=0;i<physics->GetNumWheels();i++)
			{
				if( m_persistantData.m_wheelWithoutContactTime[i] >= params.WheelNoContactThreshold )
				{
					numWheelsWithoutContact++;
				}
			}
			// Not actually wheels in contact with the ground. More the number
			// of wheels that haven't been out of contact longer than
			// WheelNoContactThreshold
			numWheelsWithContact = physics->GetNumWheels() - numWheelsWithoutContact;

			bool fakePhysicsActive = vehicleActor->GetFakePhysics()->IsActive();
			if( !m_lostControl )
			{
				// ----------------------------------------------------------
				// The conditional checks for losing control

				// 1) Have we turned on our side or gone upside down,
				// without any of our wheels being in contact
				if( (numWheelsWithContact == 0 && data.m_vehicleUpDotWorldUp < params.UpsideDownDotProductThreshold ) || 
					fakePhysicsActive)
				{
					m_lostControl = true;
					SetYawTrackingEnabled(false,false);
					
					SetIdealYawMaxErrorSoften(true);
					m_crashedTransformSmoothData.targetValue = 1.0f;
					m_rollDampingSmoothData.currentValue = params.LostControlRollSmoothing;
					m_persistantData.m_reversing = false;
					m_persistantData.m_jTurning = false;
					m_burningout = false;
					m_burningoutTimer = 0.0f;
				}
			}
			else
			{
				// ----------------------------------------------------------
				// The conditional checks for regaining control
				RacingPhysics::Vector3R angVel = physics->GetAngularVelocity();
				angVel.Y(0.0f);
				if( angVel.Length() < params.RegainedControlAngVelThreshold && 
					numWheelsWithoutContact <= 1 &&
					input->m_fThrottle > params.RegainedControlThrottleThreshold && 
					!fakePhysicsActive)
				{
					m_lostControl = false;

					SetYawTrackingEnabled(true);
					SetIdealYawMaxErrorSoften(true);
					SetIdealPitchMaxErrorSoften(true);

					m_crashedTransformSmoothData.targetValue = 0.0f;
					m_yawDampingSmoothData.currentValue = 1.0f;
				}
			}

			


			//---------------------------------------------------------------
			// Burnout		
			static bool burnoutEnabled = true;
			if( !m_burningout )
			{
				m_burningoutTimer += timeElapsed;
				if( m_burningoutTimer > params.BurnoutTimerThreshold && !m_lostControl && params.YawMaxErrorNormal<0.0f && burnoutEnabled)
				{
					if( fabs( data.m_angVelLS.Y()) > params.RotationalUpperThreshold && !m_persistantData.m_reversing)
					{
						// When we start to burnout, leave the
						// camera where it is. It looks cool to see the side
						// of the vehicle and it also allows us to see the
						// car come to a complete standstill. We will track 
						// back to the rear of the vehicle when it starts  
						// driving forwards again.

						m_burningout = true;
						
						SetYawTrackingEnabled(false,false);
					}
				}
			}
			else
			{
				static F32 linVelForZeroDampingMag = 1.0f;
				static F32 angVelForZeroDampingMag = 1.0f;

				
				bool drivingAgain = (input->m_fThrottle > 0.1f) && (fabs(data.m_linVelLS.Z()) > s_LinVelZToExitBurnout);
				bool stoppedSpinning = fabs( data.m_angVelLS.Y()) < params.RotationalLowerThreshold;
				
				
				if( (drivingAgain && stoppedSpinning) )
				{
					m_burningout = false;
					m_burningoutTimer = 0.0f;
					SetYawTrackingEnabled(true);
				}
			}
				
			
			//---------------------------------------------------------------
			// Calculate Offsets
			F32 cameraTransXOffsetSmooth = CalculateDesiredTransXOffsetSmooth(data);
			m_cameraTransformXOffset.targetValue = CalculateDesiredTransXOffset(data);
			m_cameraTransformXOffset.currentValue = SmoothingFunction(	m_cameraTransformXOffset.currentValue, 
																		m_cameraTransformXOffset.targetValue, 
																		m_cameraTransformXOffset.velocity,
																		cameraTransXOffsetSmooth,
																		timeElapsed);

#if HOLLYWOOD_CAMERA_GRAPHS
			if(this==s_MainHollywoodCamera)
			{
				s_TransXTarget.SetValue(m_cameraTransformXOffset.targetValue);
				s_TransXCurrent.SetValue(m_cameraTransformXOffset.currentValue);
			}
#endif
			//---------------------------------------------------------------
			// Calculate our basis transform 
			Maths::HMat43 crashedVehicleTransform(Maths::UNDEFINED);
			Maths::HMat43 normalDrivingTransform(data.m_vehicleTransform);
			Maths::HMat43 antiPendulumTransform(Maths::UNDEFINED);
			Maths::HMat43 targetTransform(Maths::UNDEFINED);
			
			
	
			m_crashedTransformSmoothData.currentValue = SmoothingFunction(		m_crashedTransformSmoothData.currentValue, 
																				m_crashedTransformSmoothData.targetValue, 
																				m_crashedTransformSmoothData.velocity,
																				params.TargetTransformSmooth,
																				timeElapsed);
			m_crashedTransformSmoothData.currentValue = Maths::Clamped( m_crashedTransformSmoothData.currentValue, 0.0f, 1.0f );
	

			F32 desiredAntiPendulum = CalculateDesiredAntiPendulum(data);
			F32 desiredAntiPendulumSmooth = CalculateDesiredAntiPendulumSmooth(data);
			// We apply a timer to ensure that we don't oscillate 
			// between anti-pendulum being on and off
			if(desiredAntiPendulum>0.01f)
			{
				m_persistantData.m_antiPendulumTimer = s_antiPendulumTimeout;
			}
			if(m_persistantData.m_antiPendulumTimer>0.0f)
			{
				m_persistantData.m_antiPendulumTimer-=timeElapsed;
				desiredAntiPendulum = 1.0f;
			}
			m_antiPendulumTransformSmoothData.targetValue = desiredAntiPendulum;
			m_antiPendulumTransformSmoothData.currentValue = SmoothingFunction(	m_antiPendulumTransformSmoothData.currentValue, 
																				m_antiPendulumTransformSmoothData.targetValue, 
																				m_antiPendulumTransformSmoothData.velocity,
																				desiredAntiPendulumSmooth,
																				timeElapsed);
			m_antiPendulumTransformSmoothData.currentValue = Maths::Clamped( m_antiPendulumTransformSmoothData.currentValue, 0.0f, 1.0f );

			Maths::HVec3 flattenedForwards( data.m_vehicleTransform.AZ().X(), 0.0f, data.m_vehicleTransform.AZ().Z());
			if(flattenedForwards.LengthSquared() > 0.01f)
			{
				flattenedForwards.Normalised();
			}
			else
			{
				flattenedForwards =Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
			}
			crashedVehicleTransform = Maths::HMat33(CalculateLookQuaternion( flattenedForwards,  Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), 0.0f ));
			crashedVehicleTransform.AW(data.m_vehicleTransform.AW());

			antiPendulumTransform = RemoveRoll(data.m_vehicleTransform.Rotation());
			antiPendulumTransform.AW(data.m_vehicleTransform.AW());
			
			targetTransform = InterpolateTransform( normalDrivingTransform, antiPendulumTransform, m_antiPendulumTransformSmoothData.currentValue);
			targetTransform = InterpolateTransform( targetTransform, crashedVehicleTransform, m_crashedTransformSmoothData.currentValue );

			// Set the transform of the cameras target
			SetTargetTransform( targetTransform );
			

			//--------------------------------------------------------------
			// Distance Parameters
			static F32 s_distanceSmoothSmooth = 0.0f;
			m_distanceSmoothData.targetValue = CalculateDesiredDistance(data);
			F32 desiredDistanceDamping = CalculateDesiredDistanceDamping(data);
			F32 desiredDistanceMaxSpeed = CalculateDesiredDistanceMaxSpeed(data);
			m_distanceSmoothSmoothData.targetValue = CalculateDesiredDistanceSmooth(data);
			m_distanceSmoothSmoothData.currentValue = SmoothingFunction(	m_distanceSmoothSmoothData.currentValue, 
																			m_distanceSmoothSmoothData.targetValue, 
																			m_distanceSmoothSmoothData.velocity,
																			s_distanceSmoothSmooth,
																			timeElapsed);
		
			
			m_distanceSmoothData.currentValue = SmoothingFunction(	m_distanceSmoothData.currentValue, 
																	m_distanceSmoothData.targetValue, 
																	m_distanceSmoothData.velocity,
																	m_distanceSmoothSmoothData.currentValue,
																	timeElapsed);
			SetIdealDistanceMaxSpeed(desiredDistanceMaxSpeed);
			SetIdealDistanceDamping(desiredDistanceDamping);
			SetIdealDistance(m_distanceSmoothData.currentValue);

			//--------------------------------------------------------------
			// Roll Parameters
			
			static F32 rollDampingSmooth = 0.2f;
			F32 desiredRoll = CalculateDesiredRoll(data);
			const Maths::HVec3& upAxis = CalculateDesiredUpAxis(data);

			m_rollDampingSmoothData.targetValue = CalculateDesiredRollDamping(data);
			m_rollDampingSmoothData.currentValue = SmoothingFunction(m_rollDampingSmoothData.currentValue, 
																	 m_rollDampingSmoothData.targetValue, 
																	 m_rollDampingSmoothData.velocity,
																	 rollDampingSmooth,
																	 timeElapsed);
			//We don't smooth the desired roll in the Hollywood
			//cam. We let the ThirdPersonCamera handle this exclusively.
			SetIdealRoll(desiredRoll);
			SetIdealRollDamping(m_rollDampingSmoothData.currentValue);
			SetUpAxis(upAxis);

			//--------------------------------------------------------------
			// Pitch Parameters
			static F32 s_pitchDampSmooth = 0.1f;
			F32 desiredPitchError = CalculateDesiredPitchErrorMax(data);
			F32 desiredPitch = CalculateDesiredPitch(data);
			F32 desiredPitchDamping = CalculateDesiredPitchDamping(data);
			m_pitchDampingSmoothData.targetValue = desiredPitchDamping;
			m_pitchDampingSmoothData.currentValue = SmoothingFunction(	m_pitchDampingSmoothData.currentValue, 
																		m_pitchDampingSmoothData.targetValue, 
																		m_pitchDampingSmoothData.velocity,
																		s_pitchDampSmooth,
																		timeElapsed);
			SetIdealPitch(desiredPitch);
			SetIdealPitchDamping(m_pitchDampingSmoothData.currentValue);
			SetIdealPitchMaxError(desiredPitchError<0.0f? -1.0f:desiredPitchError);
			
			//---------------------------------------------------------------
			//Fov Parameters
			static F32 s_fovDamping = 0.8f;
			static F32 s_fovMax = Maths::DegToRad(120.0f);
			m_FovSmoothData.targetValue = CalculateDesiredFov(data);
			m_FovSmoothData.currentValue = SmoothingFunction(	m_FovSmoothData.currentValue, 
																m_FovSmoothData.targetValue, 
																m_FovSmoothData.velocity,
																s_fovDamping,
																timeElapsed);
			F32 finalFov = m_FovSmoothData.currentValue;
			F32 dollyZoomFov = 0.0f;
			if(HollywoodCamUseEffects)
			{
				finalFov += Maths::DegToRad(m_EffectFov);
				// Query the SFX system for any dollyzooms
				const AmaxCam::C_CameraDollyZoomSpecialFX::ActiveDollyZoomList& DollyZoomList = AmaxCam::C_CameraDollyZoomSpecialFX::GetActiveDollyZoomList();
				for(AmaxCam::C_CameraDollyZoomSpecialFX::ActiveDollyZoomList::ConstIterator pIterator = DollyZoomList.Begin(); pIterator != DollyZoomList.End(); ++pIterator)
				{
					if( pIterator->m_ViewportID == viewportIndex )
					{
						dollyZoomFov += Maths::DegToRad(pIterator->m_Fov);
					}
				}
			}
			
			
			F32 fovScale = AmaxCam::CameraSystem().FovScale();
			F32 preDollyZoomFov = finalFov;
			F32 postDollyZoomFov = finalFov + dollyZoomFov;

			preDollyZoomFov = Maths::Clamped<F32>(preDollyZoomFov,0.0f,s_fovMax) * fovScale;
			postDollyZoomFov = Maths::Clamped<F32>(postDollyZoomFov,0.0f,s_fovMax) * fovScale;

			SetFov(postDollyZoomFov);
			
			//---------------------------------------------------------------
			// Calculate a look at position

			HVec3 targetUpVector = Maths::HVec3(params.LookAtPos.vx, params.LookAtPos.vy, params.LookAtPos.vz);
					
			static F32 lookUpDownSmoothing = 0.2f;
			m_lookSideSmoothData.targetValue = CalculateDesiredLookSide(data);
			m_lookSideSmoothData.currentValue = SmoothingFunction(	m_lookSideSmoothData.currentValue, 
																	m_lookSideSmoothData.targetValue, 
																	m_lookSideSmoothData.velocity,
																	params.LookAtSmoothingValue,						
																	timeElapsed);
			m_lookUpDownSmoothData.targetValue = CalculateDesiredLookUpDown(data);
			m_lookUpDownSmoothData.currentValue = SmoothingFunction(m_lookUpDownSmoothData.currentValue, 
																	m_lookUpDownSmoothData.targetValue, 
																	m_lookUpDownSmoothData.velocity,
																	lookUpDownSmoothing,						
																	timeElapsed);
			
			
			targetUpVector.X( targetUpVector.X() + m_lookSideSmoothData.currentValue );
			targetUpVector.Y( targetUpVector.Y() + m_lookUpDownSmoothData.currentValue ); 
			SetTargetAimPos( targetUpVector );

			F32 yawDampingSmooth = CalculateDesiredYawDampingSmooth(data);
			m_yawDampingSmoothData.targetValue = CalculateDesiredYawDamping(data);
			m_yawDampingSmoothData.currentValue = SmoothingFunction(m_yawDampingSmoothData.currentValue, 
																	m_yawDampingSmoothData.targetValue, 
																	m_yawDampingSmoothData.velocity,
																	yawDampingSmooth,
																	timeElapsed);
			SetIdealYawDamping( m_yawDampingSmoothData.currentValue );

			F32 IdealYaw = CalculateDesiredYaw(data);
			F32 IdealYawTollerance = CalculateDesiredYawTollerance(data);
			F32 IdealYawMin = IdealYaw - IdealYawTollerance;
			F32 IdealYawMax = IdealYaw + IdealYawTollerance;
	
			IdealYawMin = IdealYawMin < -Maths::Pi?IdealYawMin+Maths::Pi2:IdealYawMin;
			IdealYawMax = IdealYawMax > Maths::Pi?IdealYawMax-Maths::Pi2:IdealYawMax;
			SetIdealYawRange(IdealYawMin,IdealYawMax);
			
			F32 IdealYawMaxError = CalculateDesiredYawMaxError(data);
			SetIdealYawMaxError(IdealYawMaxError);
			// Have we 'just' changed the ideal yaw?
			// If so, relax the max yaw error value
			// until it falls back into the new range.
			// Stops it snapping. 
			if(fabs(IdealYaw)>0.0f )
			{
				SetIdealYawMaxErrorSoften(true);
			}
			// Apply the third person tracking
			inherited::OnUpdate();

			
			// Now as we are going to monkey around with the resultant
			// camera position/orientation, we will cache the current
			// results so that we can return them to these before the
			// next update;
			m_unmodifiedPosition = Position();
			m_unmodifiedRotation = Orient();

			// Apply the post update overlays
			SetTargetAimPos(m_targetAimPos + Maths::Constants<Maths::HVec3>::UnitAxisPositiveX() * m_cameraTransformXOffset.currentValue);
			if(HollywoodCamUseEffects)
			{
				SetTargetAimPos(m_targetAimPos + m_Offset);
			}

			
			//---------------------------------------------------------------
			// Do the look-around	
			UpdateLookAround(data,timeElapsed);

			F32 desiredLookAngle = CalculateDesiredLookAroundAngle( data );
			F32 desiredLookSmoothing = CalculateDesiredLookAroundSmoothing( data );
			F32 lookAngleError = m_lookAngleSmoothData.currentValue - desiredLookAngle;
			if( fabs(lookAngleError) > Maths::Pi)
			{
				lookAngleError += lookAngleError < 0.0f ? Maths::Pi2 : -Maths::Pi2; 
			}
			m_lookAngleSmoothData.currentValue = SmoothingFunction(m_lookAngleSmoothData.currentValue, 
																   m_lookAngleSmoothData.currentValue - lookAngleError, 
																   m_lookAngleSmoothData.velocity,
																   desiredLookSmoothing,						
																   timeElapsed);
			if( fabs(m_lookAngleSmoothData.currentValue) > Maths::Pi)
			{
				m_lookAngleSmoothData.currentValue += m_lookAngleSmoothData.currentValue < 0.0f ? Maths::Pi2 : -Maths::Pi2; 
			}

			m_lookOrient = C_Quaternion( Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() * sin(m_lookAngleSmoothData.currentValue  * 0.5f), HVecW(cos(m_lookAngleSmoothData.currentValue  * 0.5f)));
			m_lookOrient.Normalise();
		
			// Transform the camera displacement into vehicle space, 
			// rotate by lookOrient and return to world space.
			HVec3 cameraPos;
			HVec3 cameraDisplacement;
			cameraPos = Position() * targetTransform.InverseRotTrans();
			if(HollywoodCamUseEffects)
			{
				cameraPos = cameraPos + m_Offset;
			}
			cameraPos += Maths::Constants<Maths::HVec3>::UnitAxisPositiveX() * m_cameraTransformXOffset.currentValue;
			cameraDisplacement = cameraPos - m_targetPos;
			if(HollywoodCamUseEffects)
			{
				
				F32 cameraDisplacementLength = cameraDisplacement.Length();
				F32 cameraDisplacementLengthCorrected = cameraDisplacementLength - s_cameraDisplacementCorrection;
				F32 fixedFovWidth = Maths::Tan(preDollyZoomFov/2.0f) * 2.0f * cameraDisplacementLengthCorrected;
				F32 newCameraDisplacementLength = fixedFovWidth/(2.0f*Maths::Tan(postDollyZoomFov/2.0f));
				newCameraDisplacementLength += s_cameraDisplacementCorrection;
				cameraDisplacement *= (newCameraDisplacementLength/cameraDisplacementLength);
			}
			cameraDisplacement = cameraDisplacement * m_lookOrient;
			cameraPos = cameraDisplacement + m_targetPos;
			cameraPos = cameraPos * targetTransform;
			SetPosition( cameraPos );

			

			Maths::HVec3 viewDirOffset = Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
			//---------------------------------------------------------------
			// Framing
			{

				Maths::HMat44 viewProject = CalculateViewProject(viewportIndex, Mat43().InverseRotTrans(), Fov());

				
				Maths::HVec3 screenSpaceMin = Maths::Constants<Maths::HVec3>::Max();
				Maths::HVec3 screenSpaceMax = Maths::Constants<Maths::HVec3>::Min();
				if(CalculateVehicleScreenSpaceBB(viewProject,vehicleActor,screenSpaceMin,screenSpaceMax))
				{
					//bool isBehind;
					Maths::HVec3 screenSpaceCentre;
					screenSpaceCentre.X( screenSpaceMin.X() + (screenSpaceMax.X()-screenSpaceMin.X())/2.0f );
					screenSpaceCentre.Y( screenSpaceMin.Y() + (screenSpaceMax.Y()-screenSpaceMin.Y())/2.0f );
					screenSpaceCentre.Z( 0.0f );

					viewDirOffset = ScreenPosToViewDir(screenSpaceCentre,Fov(),Display::Device()->GetAspectRatio());
					/*
					Maths::HVec3 worldDir = ScreenPosToWorldDir(screenSpaceCentre,Mat43(),Fov(),Display::Device()->GetAspectRatio());
					static F32 scale = 100.0f;
					Maths::HVec3 worldPos = Position() + (worldDir * scale);

					Maths::HVec3 screenSpaceCentre2 = WorldSpaceToScreenPos( worldPos, viewProject,isBehind);

					{
						Maths::HVec2 pixelSpaceMin;
						Maths::HVec2 pixelSpaceMax;
						Maths::HVec2 pixelSpaceCentre;
						Maths::HVec2 pixelSpaceCentre2;

						const Framework::C_View& rFrameworkView = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewportIndex);
						Maths::C_Rect viewExtents =  rFrameworkView.m_View.m_ViewSettings2D.m_ViewExtents;

						pixelSpaceMin.X( (((screenSpaceMin.X()+1.0f) /2.0f) * ( viewExtents.Max().X() - viewExtents.Min().X())) + viewExtents.Min().X());
						pixelSpaceMin.Y( ((1.0f-((screenSpaceMin.Y()+1.0f) /2.0f)) * ( viewExtents.Max().Y() - viewExtents.Min().Y())) + viewExtents.Min().Y());
						pixelSpaceMax.X( (((screenSpaceMax.X()+1.0f) /2.0f) * ( viewExtents.Max().X() - viewExtents.Min().X())) + viewExtents.Min().X());
						pixelSpaceMax.Y( ((1.0f-((screenSpaceMax.Y()+1.0f) /2.0f)) * ( viewExtents.Max().Y() - viewExtents.Min().Y())) + viewExtents.Min().Y());
						pixelSpaceCentre.X( (((screenSpaceCentre.X()+1.0f) /2.0f) * ( viewExtents.Max().X() - viewExtents.Min().X())) + viewExtents.Min().X());
						pixelSpaceCentre.Y( ((1.0f-((screenSpaceCentre.Y()+1.0f) /2.0f)) * ( viewExtents.Max().Y() - viewExtents.Min().Y())) + viewExtents.Min().Y());
						pixelSpaceCentre2.X( (((screenSpaceCentre2.X()+1.0f) /2.0f) * ( viewExtents.Max().X() - viewExtents.Min().X())) + viewExtents.Min().X());
						pixelSpaceCentre2.Y( ((1.0f-((screenSpaceCentre2.Y()+1.0f) /2.0f)) * ( viewExtents.Max().Y() - viewExtents.Min().Y())) + viewExtents.Min().Y());

						DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceMin.X(),pixelSpaceMin.Y()),Maths::HVec2(pixelSpaceMin.X(),pixelSpaceMax.Y()),Maths::RGBA(0,255,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceMin.X(),pixelSpaceMax.Y()),Maths::HVec2(pixelSpaceMax.X(),pixelSpaceMax.Y()),Maths::RGBA(0,255,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceMax.X(),pixelSpaceMax.Y()),Maths::HVec2(pixelSpaceMax.X(),pixelSpaceMin.Y()),Maths::RGBA(0,255,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceMax.X(),pixelSpaceMin.Y()),Maths::HVec2(pixelSpaceMin.X(),pixelSpaceMin.Y()),Maths::RGBA(0,255,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceCentre.X()-10.0f,pixelSpaceCentre.Y()),Maths::HVec2(pixelSpaceCentre.X()+10.0f,pixelSpaceCentre.Y()),Maths::RGBA(255,0,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceCentre.X(),pixelSpaceCentre.Y()-10.0f),Maths::HVec2(pixelSpaceCentre.X(),pixelSpaceCentre.Y()+10.0f),Maths::RGBA(255,0,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceCentre2.X()-10.0f,pixelSpaceCentre2.Y()),Maths::HVec2(pixelSpaceCentre2.X()+10.0f,pixelSpaceCentre2.Y()),Maths::RGBA(255,0,0,255));
						debugRender->Draw2dLine(Maths::HVec2(pixelSpaceCentre2.X(),pixelSpaceCentre2.Y()-10.0f),Maths::HVec2(pixelSpaceCentre2.X(),pixelSpaceCentre2.Y()+10.0f),Maths::RGBA(255,0,0,255));

					}
					*/
				}
			}


			F32 desiredViewDirOffsetAmount = 0.0f;
			if(HollywoodCamViewDirOffsetOverride>0.0f)
			{
				desiredViewDirOffsetAmount = HollywoodCamViewDirOffsetOverride;
			}
			Maths::C_Quaternion viewOriginal = CalculateLookQuaternion( Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), 0.0f );
			Maths::C_Quaternion viewDeviation = CalculateLookQuaternion( viewDirOffset, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), 0.0f );
			Maths::C_Quaternion viewResult = viewOriginal.Slerp(viewDeviation, desiredViewDirOffsetAmount);

		
			// As we have updated the camera's position, we need to recalculate
			// the orientation quaternion as well.
			CalculateLook();

			SetOrient(Orient() * viewResult);
			if(HollywoodCamUseEffects)
			{
				Maths::C_Quaternion xrot(Maths::HVec3(1.0f, 0.0f, 0.0f), DegToRad(m_EffectOffsetRot.X()));
				Maths::C_Quaternion yrot(Maths::HVec3(0.0f, 1.0f, 0.0f), DegToRad(m_EffectOffsetRot.Y()));
				Maths::C_Quaternion zrot(Maths::HVec3(0.0f, 0.0f, 1.0f), DegToRad(m_EffectOffsetRot.Z()));
				SetOrient(Orient() * xrot * yrot * zrot);
			}

			
			//---------------------------------------------------------------
			// Update the max distance clamping
			static F32 desiredMaxDistanceSmoothing = 2.0f;
			F32 prevMaxDistance = m_MaxDistanceSmoothData.currentValue;
			m_MaxDistanceSmoothData.targetValue = params.IdealDistanceNormal * 2.0f;
			m_MaxDistanceSmoothData.currentValue = SmoothingFunction(	m_MaxDistanceSmoothData.currentValue, 
																		m_MaxDistanceSmoothData.targetValue, 
																		m_MaxDistanceSmoothData.velocity,
																		desiredMaxDistanceSmoothing,						
																		timeElapsed);

			//---------------------------------------------------------------
			// Apply hits
			
			if(ShouldDoHits(data))
			{
				if (m_Raytrace)
					SetPhysics(vehicleActor->GetPhysicsChecks());

				Maths::HVec3 safePositionLS = m_targetPos;
				safePositionLS.Y(vehicleActor->GetCollisionBody()->GetLocalBoundingBox().extents.Y()+
								 vehicleActor->GetCollisionBody()->ElementOffset().Y() + 
								 vehicleActor->GetCollisionBody()->GetLocalBoundingBox().centre.Y());
				Maths::HVec3 safePositionWS = safePositionLS * m_target;
				Maths::HVec3 desiredPositionWS = Position();
				Maths::HVec3 desiredOffsetWS = Maths::HVec3(desiredPositionWS - safePositionWS);
				F32 desiredLength = desiredOffsetWS.Length();
				// Clamp this length to our current maximum
				F32 finalLength = Maths::Clamped<F32>(desiredLength,0.0f,m_MaxDistanceSmoothData.currentValue);
				desiredOffsetWS.Normalise();
				desiredOffsetWS *= finalLength;
				desiredPositionWS = safePositionWS + desiredOffsetWS;
		
				F32 trace = 1.0f;
				if (m_physics)
				{
					Collision::C_CollisionFilter ghostedCollisionFilter(0xffffffff, Game::PolyFlags::kCameraSolid ,vehicleActor->GetPhysicalGroupID(), 0, 0);
					ghostedCollisionFilter.SetBodyMask(ghostedCollisionFilter.GetBodyMask() &(~Game::PolyFlags::kbVehicle));
					ghostedCollisionFilter.SetBodyMask(ghostedCollisionFilter.GetBodyMask() &(~Game::PolyFlags::kbPerk));
					ghostedCollisionFilter.SetBodyMask(ghostedCollisionFilter.GetBodyMask() &(~Game::PolyFlags::kbOuterMine));
					ghostedCollisionFilter.SetBodyMask(ghostedCollisionFilter.GetBodyMask() & (~Game::PolyFlags::kbDynamicObject));


					Collision::C_CollisionFilter defaultCollisionFilter(0xffffffff, Game::PolyFlags::kCameraSolid , vehicleActor->GetPhysicalGroupID(), 0, 0);
					defaultCollisionFilter.SetBodyMask(defaultCollisionFilter.GetBodyMask() & (~Game::PolyFlags::kbPerk));
					defaultCollisionFilter.SetBodyMask(defaultCollisionFilter.GetBodyMask() & (~Game::PolyFlags::kbOuterMine));

					trace = m_physics->RayTrace(safePositionWS, desiredPositionWS, 0.35f, vehicleActor->IsCollisionEnabled()?&defaultCollisionFilter:&ghostedCollisionFilter );
				
				}
				
				F32 collisionDistance = finalLength * trace;
				// Clamp the distance of the camera from the target pos
				desiredOffsetWS = Maths::HVec3((desiredPositionWS - safePositionWS) * trace);

				SetPosition(safePositionWS + desiredOffsetWS);	
				
				// Did we hit something?
				if(trace<0.99f)
				{
					m_MaxDistanceSmoothData.currentValue = collisionDistance;
					if(collisionDistance < prevMaxDistance)
					{
						m_MaxDistanceSmoothData.velocity = 0.0f;
					}
					else if( timeElapsed > 0.01f )
					{
						m_MaxDistanceSmoothData.velocity = (collisionDistance - prevMaxDistance)/timeElapsed;
					}
				}
				
				

				if(m_Wrecked)
				{
					// If have lost control and there is an intersecting obstacle between the
					// target and the current camera position. Stop tracking!
					if(trace < 0.9999f)
					{
						static F32 s_wreckedTimeBeforeFreezingPos = 0.0f;
						if(m_WreckedTimer > s_wreckedTimeBeforeFreezingPos )
						{
							//stop tracking
							SetDistanceTrackingEnabled(false,true);
							SetPitchTrackingEnabled(false,true);
							SetYawTrackingEnabled(false,true);
							m_tracking = false;
							m_WreckedCamPos = Position();							
						}
					}
				}
			}

			if( !m_tracking )
			{
				static bool useWreckedCamPos = true;
				if( useWreckedCamPos )
				{
					SetPosition(m_WreckedCamPos);
				}
			}


			// blank physics checks pointer, just in case.
			SetPhysics(0); 

			BlankParams();
			// cache this frames data, for use next frame if desired.
			m_dataLast = data;
			m_persistantDataLast = m_persistantData;
		}
	}

	//---------------------------------------------------------------------
	// BlankParams
	//---------------------------------------------------------------------
	
	void C_HollywoodVehicleCamera::BlankParams()
	{
		m_EffectOffset = HVec3();
		m_EffectOffsetRot = HVec3();
		m_EffectFov = 0.f;
	}
	//---------------------------------------------------------------------
	// CalculateDesiredPitch 
	// What pitch would we ideally want the camera positioned at.
	//---------------------------------------------------------------------
	F32			
	C_HollywoodVehicleCamera::CalculateDesiredPitch(const hollywoodDataCache& data) const
	{
		
		const HollywoodVehicleParams& params = GetParams();
		F32 desiredPitch = params.IdealPitch;
		if( m_Wrecked )
		{
			return m_wreckedIdealPitch;
		}
		else if(!m_lostControl)
		{
			// If airbourne modify the pitch based on the current
			// world y velocity of the vehicle
			if(data.m_airbourneTime > s_minAirbourneTimeForPitchDeviation)
			{
				
				
				if(data.m_linVel.Y()<0.0f)
				{
					F32 clampedYVel = fabs(data.m_linVel.Y())/fabs(params.MaxDownwardsVelocity);
					clampedYVel = Maths::Clamped<F32>(clampedYVel,0.0f,1.0f);
					desiredPitch = params.IdealPitchAirbourneDown * clampedYVel;
				}
				else
				{
					F32 clampedYVel = fabs(data.m_linVel.Y())/fabs(params.MaxUpwardsVelocity);
					clampedYVel = Maths::Clamped<F32>(clampedYVel,0.0f,1.0f);
					desiredPitch = params.IdealPitchAirbourneUp * clampedYVel;
				}
			}
		}
		
		return desiredPitch;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredPitchDamping 
	//---------------------------------------------------------------------
	F32			
	C_HollywoodVehicleCamera::CalculateDesiredPitchDamping(const hollywoodDataCache& data) const
	{
		const HollywoodVehicleParams& params = GetParams();
		F32 desiredPitchDamping = params.PitchDampingNormal;
		if(!m_lostControl)
		{
			
			if(data.m_airbourneTime > s_minAirbourneTimeForPitchDeviation)
			{
				if(data.m_linVel.Y()<0.0f)
				{
					desiredPitchDamping = params.PitchDampingAirbourneDown;
				}
				else
				{
					desiredPitchDamping = params.PitchDampingAirbourneUp;
				}
			}
		}
		
		return desiredPitchDamping;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredPitchErrorMax 
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredPitchErrorMax(const hollywoodDataCache& data) const
	{
		const HollywoodVehicleParams& params = GetParams();

		F32 desiredPitchError = params.PitchMaxErrorNormal;
		if( m_Wrecked )
		{
			desiredPitchError = -1.0f;
		}
		else if(m_lostControl)
		{
			desiredPitchError = -1.0f;
		}
		else if(data.m_airbourneTime > s_minAirbourneTimeForPitchDeviation)
		{
			desiredPitchError = -1.0f;
		}
		else
		{
			static F32 s_maxPitchError = Maths::DegToRad(25.0f);
			static F32 s_pitchErrorDecayTime = 0.2f;
			desiredPitchError = desiredPitchError + ((s_maxPitchError - desiredPitchError) * (1.0f - Maths::Clamped<F32>((m_persistantData.m_groundTime/s_pitchErrorDecayTime),0.0f,1.0f)));
		}
		return desiredPitchError;
	}

	F32			
	C_HollywoodVehicleCamera::CalculateDesiredTransXOffsetSmooth(const hollywoodDataCache& data) const
	{
		F32 desiredXOffsetSmooth = 0.0f;

		if(m_persistantData.m_jTurning)
		{
			static F32 s_jTurningSmooth = 0.4f;
			desiredXOffsetSmooth = s_jTurningSmooth;
		}
		else
		{
			static F32 s_driftingSmooth = 0.8f;
			desiredXOffsetSmooth = s_driftingSmooth;
		}
		return desiredXOffsetSmooth;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredTransXOffset 
	// We offset the transform passed to the third person, shifting the position
	// of the target in screen space. This function calculates our desired 
	// offset in the cameras world 'x' direction
	//---------------------------------------------------------------------
	F32			
	C_HollywoodVehicleCamera::CalculateDesiredTransXOffset(const hollywoodDataCache& data) const
	{
		F32 desiredXOffset = 0.0f;
		if(m_persistantData.m_jTurning)
		{
			static F32 s_jTurnMultiplier = -2.0f;
			static F32 s_jTurnAngVelMin = 1.0f;
			static F32 s_jTurnAngVelMax = 4.0f;

			F32 normalisedAngularVel = ((Maths::Clamped<F32>(fabs(data.m_angVelLS.Y()),s_jTurnAngVelMin,s_jTurnAngVelMax)-s_jTurnAngVelMin)/(s_jTurnAngVelMax-s_jTurnAngVelMin));
			desiredXOffset = s_jTurnMultiplier * Maths::Sign(data.m_angVelLS.Y()) * normalisedAngularVel;
		}
		else
		{
			static F32 s_LookSideScale = 1.0f;
			static F32 s_LookSideSlipMin = 0.3f;
			static F32 s_LookSideSlipMax = 0.8f;

			F32 slipNormalised = (fabs(data.m_BackWheelSlip) - s_LookSideSlipMin) / (s_LookSideSlipMax - s_LookSideSlipMin);
			slipNormalised = Maths::Clamped<F32>(slipNormalised,0.0f,1.0f);
			slipNormalised *= data.m_BackWheelSlip < 0.0f ? -1.0f:1.0f;

			// WheelContactFactor
			// The time the wheels have been off the ground reduces the lookSide
			// amount applied.
			F32 wheelContactFactor = 0.0f;
			{
				int i;
				for(i=0;i<data.m_physics->GetNumWheels();i++)
				{
					wheelContactFactor += m_persistantData.m_wheelWithoutContactTime[i];
				}
				wheelContactFactor /= data.m_physics->GetNumWheels();	
				wheelContactFactor = Clamped<F32>( 0.5f - wheelContactFactor, 0.0f, 0.5f ) / 0.5f;
			}

			static F32 uprightDotForNoEffect = 0.70f;
			static F32 uprightDotForFullEffect = 0.85f;
			F32 uprightFactor = (data.m_vehicleUpDotWorldUp - uprightDotForNoEffect) / (uprightDotForFullEffect - uprightDotForNoEffect);
			uprightFactor = Maths::Clamped<F32>(uprightFactor,0.0f,1.0f);

			// Look around factor
			// When the user pushes the look-around stick to move the camera around
			// we dont want any lookside so use the look around angle as a scaling
			// factor
			F32 lookAroundFactor = 0.0f;
			{
				const F32 minLookAngle = 0.10f;
				const F32 maxLookAngle = 0.20f;
				lookAroundFactor = Clamped<F32>(( fabs(m_lookAngle) - minLookAngle )/(maxLookAngle - minLookAngle ),0.0f, 1.0f);
				lookAroundFactor = 1.0f - lookAroundFactor;
			}
			
			// Local z velocity factor
			// A high z velocity is usually a sign that we are burning
			// out. so use this to reduce the amount of lookside
			F32 zVelFactor = 0.0f;
			{
				const F32 minZVel = 16.0f;
				const F32 maxZVel = 30.0f;
				zVelFactor = (fabs(data.m_linVelLS.Z()) - minZVel) / (maxZVel - minZVel);
				zVelFactor = Clamped<F32>( zVelFactor, 0.0f, 1.0f );
			}
			// No side looking if we are reversing.
			if( data.m_linVelLS.Z() < 0.0f )
			{
				desiredXOffset = 0.0f;
			}
			else if( m_lostControl )
			{
				desiredXOffset = 0.0f;
			}
			else if( data.m_input->m_fThrottle < 0.25f )
			{
				desiredXOffset = 0.0f;
			}
			else
			{
				desiredXOffset = slipNormalised * s_LookSideScale;
				desiredXOffset *= uprightFactor;
				desiredXOffset *= wheelContactFactor;
				desiredXOffset *= lookAroundFactor;
				desiredXOffset *= zVelFactor;
			}

			if(DebugOverrideTransX)
			{
				desiredXOffset = DebugOverrideTransXValue;
			}
			
		}
		return desiredXOffset;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredDistance
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredDistance(const hollywoodDataCache& data) const
	{
	
		
		const HollywoodVehicleParams& params = GetParams();
		F32 desiredDistance = params.IdealDistanceNormal;

		
		F32 fovScale = AmaxCam::CameraSystem().FovScale();
		fovScale = Maths::Clamped<F32>(fovScale,0.0f,1.0f);
		
		F32 fovScaleNormalised = (1.0f - fovScale);
		fovScaleNormalised = Maths::Clamped<F32>(fovScaleNormalised,0.0f,DebugMaxFovScaleDiff);
		if( DebugMaxFovScaleDiff > 0.0f )
		{
			fovScaleNormalised *= (1.0f / DebugMaxFovScaleDiff);
		}
		else
		{
			fovScaleNormalised = 0.0f;
		}

		desiredDistance += (desiredDistance * fovScaleNormalised * DebugMaxFovScaleScale );

		if(m_persistantData.m_highSpeed)
		{
			F32 currentFov = Fov();
			desiredDistance = m_persistantData.m_fixedFovWidth/(2.0f*Maths::Tan(currentFov/2.0f));
		}
		else if(data.m_airbourneTime>s_minAirbourneTimeBeforeResettingDistance)
		{
			desiredDistance = params.IdealDistanceAirbourne;
		}
		else if(data.m_linVelLS.Z()>0.0f)
		{
			if(data.m_linAccelLS.Z() > s_distanceAccelerationThreshold )
			{
				F32 normalisedAcceleration = (data.m_linAccelLS.Z() - s_distanceAccelerationThreshold)/(params.MaxAcceleration-s_distanceAccelerationThreshold);
				normalisedAcceleration = Maths::Clamped<F32>(normalisedAcceleration,0.0f,1.0f);
				desiredDistance += (params.IdealDistanceAtFullAcceleration - params.IdealDistanceNormal) * normalisedAcceleration;
			}
			else if(data.m_linAccelLS.Z() < s_distanceDecelerationThreshold )
			{
				F32 normalisedDeceleration = (fabs(data.m_linAccelLS.Z()) - fabs(s_distanceDecelerationThreshold))/(fabs(params.MaxDeceleration)-fabs(s_distanceDecelerationThreshold));
				normalisedDeceleration = Maths::Clamped<F32>(normalisedDeceleration,0.0f,1.0f);
				desiredDistance += (params.IdealDistanceAtFullDeceleration - params.IdealDistanceNormal) * normalisedDeceleration;
			}
		}
		
		return desiredDistance;

	}

	//---------------------------------------------------------------------
	// CalculateDesiredDistanceMaxSpeed
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredDistanceMaxSpeed(const hollywoodDataCache& data) const
	{
		static F32 s_LostControlMaxSpeed = 34.0f;
		F32 desiredMaxSpeed = -1.0f;
		if( m_Wrecked )
		{
			return desiredMaxSpeed = s_LostControlMaxSpeed;
		}
		return desiredMaxSpeed;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredDistanceDamping 
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredDistanceDamping(const hollywoodDataCache& data) const
	{
		static F32 s_distanceDamping = 0.0f;
		static F32 s_distanceDampingLostControl = 0.25f;
		if( m_Wrecked )
		{
			return s_distanceDampingLostControl;
		}
		return s_distanceDamping;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredDistanceSmoothing
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredDistanceSmooth(const hollywoodDataCache& data) const
	{
		const HollywoodVehicleParams& params = GetParams();

		F32 desiredDistanceSmoothing = params.IdealDistanceSmoothing;
		if(m_persistantData.m_highSpeed)
		{
			desiredDistanceSmoothing = 0.0f;
		}
		else if(data.m_airbourneTime>s_minAirbourneTimeBeforeResettingDistance)
		{
			desiredDistanceSmoothing = params.IdealDistanceSmoothingAirbourne;
		}
		else if(data.m_linVelLS.Z()>0.0f)
		{
			if(data.m_linAccelLS.Z() > s_distanceAccelerationThreshold )
			{
				desiredDistanceSmoothing = params.IdealDistanceSmoothingAccelerating;
			}
			else if(data.m_linAccelLS.Z() < s_distanceDecelerationThreshold )
			{
				desiredDistanceSmoothing = params.IdealDistanceSmoothingDecelerating;
			}
		}
		return desiredDistanceSmoothing;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredFov
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredFov(const hollywoodDataCache& data) const
	{
		const HollywoodVehicleParams& params = GetParams();
	
		F32 desiredFov = params.NormalFOV;

		if(m_persistantData.m_highSpeed)
		{	
			desiredFov *= params.HighSpeedFovMultiplier;
		}

		return desiredFov;
	}
	
	//---------------------------------------------------------------------
	// CalculateDesiredAntiPendulum
	//   Returns a value between 0.0f (Vehicle Transform) and 
	//   1.0f (AntiPendulum Transform)
	//---------------------------------------------------------------------
	F32					
	C_HollywoodVehicleCamera::CalculateDesiredAntiPendulum(const hollywoodDataCache& data) const
	{
	

		F32 antiPendulumValue = 0.0f;

		int numWheelsWithoutContact = 0;

		for(int i=0;i<data.m_physics->GetNumWheels();i++)
		{
			if( m_persistantData.m_wheelWithoutContactTime[i] >= s_antiPendulumWheelWithoutContactTime )
			{
				numWheelsWithoutContact++;
			}
		}

		// If we are non-horizontal and have at least two wheels off the floor
		// play it safe and switch on the anti-pendulum mechanism.
		if(data.m_vehicleUpDotWorldUp<s_antiPendulumUpDotThreshold && numWheelsWithoutContact > 2)
		{
			antiPendulumValue = 1.0f;
		}
		// If we have just landed, switch on the anti-pendulum mechanism as this is the 
		// most likely time it will occur.
		if( data.m_airbourneTime < 0.1f && 
			m_dataLast.m_airbourneTime > s_antiPendulumMinAirbourneTime )
		{
			antiPendulumValue = 1.0f;
		}
		
#ifndef CFG_GOLD
		antiPendulumValue = AntiPendulumDesiredOverride < 0.0f ? antiPendulumValue:AntiPendulumDesiredOverride;
#endif
		return antiPendulumValue;
	}
	//---------------------------------------------------------------------
	// CalculateDesiredAntiPendulumSmooth
	//---------------------------------------------------------------------
	F32 
	C_HollywoodVehicleCamera::CalculateDesiredAntiPendulumSmooth(const hollywoodDataCache& data) const
	{
		F32 antiPendulumSmooth = m_antiPendulumTransformSmoothData.targetValue > 0.1f ? s_antiPendulumSmoothUp : s_antiPendulumSmoothDown;
		return antiPendulumSmooth;
	}
	
	//---------------------------------------------------------------------
	// CalculateDesiredRoll 
	// What roll would we ideally like the camera to have
	//---------------------------------------------------------------------
	F32 
	C_HollywoodVehicleCamera::CalculateDesiredRoll(const hollywoodDataCache& data) const 
	{
		F32 desiredCameraRoll = 0.0f;
		//As the car rolls around its axis the camera also tracks this to a degree
		
		HollywoodVehicleParams params = GetParams();

		
	
		F32 CameraCarRollDiffMaxRad = params.CameraCarRollDiffMax; 

		// Lost control of the car, return the camera to its upright state.
		if( m_lostControl )
		{
			desiredCameraRoll = 0.0f;
		}
		else 
		{
			//Calculate the vehicles roll. Where 0.0f represents 
			//being upright in worldspace (ignoring pitch & yaw)
			//and +/-PI would mean you are upside down
		
			
			
			if(data.m_vehiclePitch>s_vehiclePitchMin)
			{
				desiredCameraRoll = data.m_vehicleRoll;
			}
			else
			{
				// Calculate the desired roll based on the roll of the vehicle			
				if(data.m_vehicleUpDotWorldUp>Maths::Cos(CameraCarRollDiffMaxRad))
				{
					
					if( data.m_linVelLS.Z() > params.YawRollFwdsVelThreshold)
					{

						// Roll based on our current yaw error
						F32 currentYaw = CalculateCurrentYaw();
						F32 yawRollNormalised = (fabs(currentYaw) - params.YawRollMinYaw)/(params.YawRollMaxYaw-params.YawRollMinYaw);
						yawRollNormalised = Maths::Clamped<F32>(yawRollNormalised,0.0f,1.0f);
						desiredCameraRoll = yawRollNormalised * params.YawRollMax * (currentYaw<0.0f?-1.0f:1.0f);
					}
					else
					{
						desiredCameraRoll = 0.0f;
					}
				}
				else
				{
					desiredCameraRoll = data.m_vehicleRoll;
				}	
			}
		}
	
		return desiredCameraRoll;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredRollDamping 
	//---------------------------------------------------------------------
	F32 
	C_HollywoodVehicleCamera::CalculateDesiredRollDamping(const hollywoodDataCache& data) const 
	{
		F32 rollDamping = 0.0f;
		
		HollywoodVehicleParams params = GetParams();

		
		F32 CameraCarRollDiffMaxRad = params.CameraCarRollDiffMax; 

		// Lost control of the car, return the camera to its upright state.
		if( m_lostControl )
		{
			rollDamping = params.LostControlRollSmoothing;
		}
		else 
		{
			
			if(data.m_vehiclePitch>s_vehiclePitchMin)
			{
				rollDamping = 0.0f;
			}
			else
			{
				// We have different damping values for above and
				// below our yaw tracking theshold.
				if( fabs(data.m_vehicleRoll) < CameraCarRollDiffMaxRad)
				{
					rollDamping = params.UprightRollSmoothing;
				}
				else 
				{
					rollDamping = params.BarrelRollSmoothing;
				}
			}
		}
		
		return rollDamping;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredUpAxis 
	//---------------------------------------------------------------------
	const Maths::HVec3 
	C_HollywoodVehicleCamera::CalculateDesiredUpAxis(
		const hollywoodDataCache& data
	) const
	{

		//if(data.m_vehicleUpDotWorldUp>s_upAxisCosThreshold)
		//{
		//	Maths::HVec3 vehicleZAxis = data.m_vehicleTransform.AZ();
		//	Maths::HVec3 vehicleZAxisFlat(vehicleZAxis.X(), 0.0f, vehicleZAxis.Z());
		//	
		//	// This assert verifies that our flat axis is
		//	// always non-zero. The condition above should ensure
		//	// this but just incase I have missed something.
		//	BIZ_ASSERT(vehicleZAxisFlat.LengthSquared()>0.1f);

		//	return vehicleZAxisFlat;
		//}
		//static F32 multiplier = 1.0f;
		//HMat33 pitchMtx( C_MatrixRotatePHR( data.m_vehiclePitch * multiplier, 0.0f, 0.0f ));
		//DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
		//debugRender->Draw3dArrow(data.m_vehicleTransform.AW(),pitchMtx.AY(),4.0f,1.0f,Maths::RGBA(0,255,0,255));
		//return pitchMtx.AY();
		return Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
	}

	//---------------------------------------------------------------------
	// CalculateDesiredYawDamping 
	// What yaw damping factor would we ideally like right now.
	//---------------------------------------------------------------------
	F32 
	C_HollywoodVehicleCamera::CalculateDesiredYawDamping(const hollywoodDataCache& data) const
	{
		
		HollywoodVehicleParams params = GetParams();

		F32 desiredYawDamping = params.YawDampingNormal;
		
		// If we are reversing
		
		if( m_persistantData.m_reversing ) 
		{
			static F32 s_reversingYawDampingNormal = 0.15f;
			static F32 s_reversingYawDampingJTurn = 10.0f;
			
			if(m_persistantData.m_jTurning)
			{
				desiredYawDamping = s_reversingYawDampingJTurn;
			}
			else
			{
				desiredYawDamping = s_reversingYawDampingNormal;
			}
		}
		
		return desiredYawDamping;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredYawDampingSmooth 
	// Determine the factor which smooths out the differences between
	// the desired and the current yaw damping. 
	//---------------------------------------------------------------------
	F32 
	C_HollywoodVehicleCamera::CalculateDesiredYawDampingSmooth(const hollywoodDataCache& data) const
	{
		const HollywoodVehicleParams& params = GetParams();
		
		F32 dampingSmooth = params.YawDampingSmoothValue;
		if( m_persistantData.m_reversing == false && m_persistantDataLast.m_reversing == true)
		{
			// In reverse mode we have no damping on the ideal yaw so it literally
			// stays locked to the front of the car. When we leave this mode (and 
			// the ideal yaw becomes the back of the car we want to blend nicely 
			// to this so we dont blend out the damping smooth value we snap.
			dampingSmooth = 0.0f;
		}
		return dampingSmooth;
	}
	//---------------------------------------------------------------------
	// CalculateDesiredYaw 
	// The ideal angular camera position around the basis transforms y axis 
	//---------------------------------------------------------------------
	F32			
	C_HollywoodVehicleCamera::CalculateDesiredYaw(const hollywoodDataCache& data) const
	{
		F32 desiredYaw = 0.0f;
		if( m_persistantData.m_reversing ) 
		{
			desiredYaw = Maths::Pi;
		}
		return desiredYaw;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredYaw 
	// The ideal angular camera position around the basis transforms y axis 
	//---------------------------------------------------------------------
	F32			
	C_HollywoodVehicleCamera::CalculateDesiredYawTollerance(const hollywoodDataCache& data) const
	{
		static F32 tollerance = Maths::DegToRad(5.0f);
		return tollerance;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredYawMaxError 
	// The maximum yaw error allowed before we lock it 
	//---------------------------------------------------------------------
	F32 
	C_HollywoodVehicleCamera::CalculateDesiredYawMaxError(const hollywoodDataCache& data) 
	{
		F32 maxError = -1.0f;
		if( !m_persistantData.m_reversing )
		{
			HollywoodVehicleParams params = GetParams();
			maxError = params.YawMaxErrorNormal<0.0f? -1.0f:params.YawMaxErrorNormal;
		}
		return maxError;
	}
	//---------------------------------------------------------------------
	// CalculateDesiredLookUpDown 
	// Desired deviation of the lookat point in the local Y axis
	//---------------------------------------------------------------------
	F32
	C_HollywoodVehicleCamera::CalculateDesiredLookUpDown(const hollywoodDataCache& data) const
	{
	
		static F32 s_distanceVelocityThreshold = 0.2f;
		static F32 s_distanceVelocityMax = 50.0f;
		HollywoodVehicleParams params = GetParams();
		F32 desiredLookUpDown = 0.0f;
		F32 lookSideFactor = fabs(m_lookSideSmoothData.currentValue)/params.LookSideMax;
		lookSideFactor = Maths::Clamped<F32>(lookSideFactor,0.0f,1.0f);
		F32 desiredBasedOnLookSide = lookSideFactor * params.LookSideUpDownMultiplier;
		F32 desiredBasedOnVelocity;
	
		
		F32 normalisedVelocity = (data.m_linVelLS.Z() - s_distanceVelocityThreshold)/(s_distanceVelocityMax-s_distanceVelocityThreshold);
		normalisedVelocity = Maths::Clamped<F32>(normalisedVelocity,0.0f,1.0f);

		desiredBasedOnVelocity = normalisedVelocity * params.ForwardsVelocityUpDownMultiplier;
	
		desiredLookUpDown = desiredBasedOnLookSide;
		desiredLookUpDown *= desiredBasedOnVelocity;
		return desiredLookUpDown;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredLookSide 
	// Desired deviation of the lookat point in the local X axis
	//---------------------------------------------------------------------
	F32
	C_HollywoodVehicleCamera::CalculateDesiredLookSide(const hollywoodDataCache& data) const
	{
		
		F32 desiredLookSide = 0.0f;			

		const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);
		if(vehicleActor)
		{
			HollywoodVehicleParams params = GetParams();


			F32 angVelNormalised = fabs(data.m_angVel.Y()) - params.LookAtMinAngVel / ( params.LookAtMaxAngVel - params.LookAtMinAngVel );
			angVelNormalised = Clamped( angVelNormalised, 0.0f, 1.0f );
			angVelNormalised *= data.m_angVel.Y() < 0.0f ? -1.0f:1.0f;
			F32 angularVelocityComponent = angVelNormalised * params.LookSideMax;
			
#if HOLLYWOOD_CAMERA_GRAPHS
			if(this==s_MainHollywoodCamera)
			{
				s_YawVelocityData.SetValue(fabs(angVelNormalised));					
			}
#endif
			static F32 s_backWheelSlipScale = 1.0f;
			F32 transverseSlipComponent = Maths::Clamped<F32>(data.m_BackWheelSlip * s_backWheelSlipScale,0.0f,1.0f);
			

			// WheelContactFactor
			// The time the wheels have been off the ground reduces the lookSide
			// amount applied.
			F32 wheelContactFactor = 0.0f;
			{
				int i;
				for(i=0;i<data.m_physics->GetNumWheels();i++)
				{
					wheelContactFactor += m_persistantData.m_wheelWithoutContactTime[i];
				}
				wheelContactFactor /= data.m_physics->GetNumWheels();	
				wheelContactFactor = Clamped<F32>( 0.5f - wheelContactFactor, 0.0f, 0.5f ) / 0.5f;
			}

			// Look around factor
			// When the user pushes the look-around stick to move the camera around
			// we dont want any lookside so use the look around angle as a scaling
			// factor
			F32 lookAroundFactor = 0.0f;
			{
				const F32 minLookAngle = 0.10f;
				const F32 maxLookAngle = 0.20f;
				lookAroundFactor = Clamped<F32>(( fabs(m_lookAngle) - minLookAngle )/(maxLookAngle - minLookAngle ),0.0f, 1.0f);
				lookAroundFactor = 1.0f - lookAroundFactor;
			}
			// orientationFactor
			// The larger the angular difference between the camera z-axis
			// and the car z-axis axis the more the lookside is reduced.
			// This reduces lookslide when burning out or spinning.
			F32 orientationFactor = 0.0f;
			{
				HVec3 camToCar = Object().position - Position();
				camToCar.Normalise();
				F32 dot = DotProduct( vehicleActor->GetRawMatrix().AZ(), camToCar );
				const F32 minOrientationAngle = 0.5f;
				const F32 maxOrientationAngle = 0.707f;
				orientationFactor = Clamped<F32>( dot, minOrientationAngle, maxOrientationAngle );
				orientationFactor = (orientationFactor - minOrientationAngle) / (maxOrientationAngle - minOrientationAngle);
			}

			// Local X velocity factor
			// A high x velocity is usually a sign that we are burning
			// out. so use this to reduce the amount of lookside
			F32 zVelFactor = 0.0f;
			{
				const F32 minZVel = 5.0f;
				const F32 maxZVel = 9.0f;
				zVelFactor = (fabs(data.m_linVelLS.Z()) - minZVel) / (maxZVel - minZVel);
				zVelFactor = Clamped<F32>( zVelFactor, 0.0f, 1.0f );
			}
			// No side looking if we are reversing.
			if( data.m_linVelLS.Z() < 0.0f )
			{
				desiredLookSide = 0.0f;
			}
			else if( m_lostControl )
			{
				desiredLookSide = 0.0f;
			}
			else if( data.m_input->m_fThrottle < 0.25f )
			{
				desiredLookSide = 0.0f;
			}
			else
			{
				desiredLookSide = angularVelocityComponent;//Maths::Max<F32>(fabs(angularVelocityComponent),fabs(transverseSlipComponent)) * (angularVelocityComponent<0.0f?-1.0f:1.0f);
				desiredLookSide *= wheelContactFactor;
				desiredLookSide *= orientationFactor;
				desiredLookSide *= lookAroundFactor;
				desiredLookSide *= zVelFactor;
			}

			if(DebugOverrideLookSide)
			{
				desiredLookSide = DebugOverrideLookSideValue;
			}
		}

		return desiredLookSide;
	}

	//---------------------------------------------------------------------
	// WorldSpaceToScreenPos 
	//---------------------------------------------------------------------
	Maths::HMat44		
	C_HollywoodVehicleCamera::CalculateViewProject(
		int viewIndex,
		const Maths::Mat43& cameraMtx,
		F32 fov
	)
	{
		if(viewIndex >= Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount())
			return Maths::HMat44();

		Display::C_View view;
		const Framework::C_View& rFrameworkView = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewIndex);

		Maths::HMat44 projection;
		view.CalculateProjectionMatrix(rFrameworkView.m_ViewSetup.m_ViewSetup3D, projection);
		Maths::HMat44 viewScale;
		{


			F32 width = 0.f, height = 0.f;

			// Update the global aspect ratio:
			F32 AspectRatio = Display::Device()->GetAspectRatio();



			height = tanf(fov * 0.5f) * 1024.0f;
			width = height * rFrameworkView.m_ViewSetup.m_ViewSetup3D.m_WindowProportion * AspectRatio;

			F32 viewXMin = -width;
			F32 viewXMax = width;

			F32 viewYMin = -height;
			F32 viewYMax = height;

			F32 xs = 2.0f / (viewXMax - viewXMin);
			F32 ys = 2.0f / (viewYMax - viewYMin);

			F32 xo = -viewXMin * xs - 1.0f;
			F32 yo = -viewYMin * ys - 1.0f;

			viewScale.AX(Maths::HVec4( xs, 0.f, 0.f, 0.f));
			viewScale.AY(Maths::HVec4(0.f,  ys, 0.f, 0.f));
			viewScale.AZ(Maths::HVec4(0.f, 0.f, 1.f, 0.f));
			viewScale.AW(Maths::HVec4( xo,  yo, 0.f, 1.f));
		}

		// calculate the camera to device matrix
		Maths::HMat44 viewProject;
		viewProject = projection * viewScale;
		viewProject = Maths::HMat44(cameraMtx) * viewProject;

		return viewProject;
	}

	//---------------------------------------------------------------------
	// WorldSpaceToScreenPos 
	//---------------------------------------------------------------------
	Maths::HVec3		
	C_HollywoodVehicleCamera::WorldSpaceToScreenPos(
		const Maths::HVec3& worldPos, 
		const Maths::HMat44& viewProject,
		bool& isBehind
	) const
	{
		
		
		//view.CalculateWorldToWindow(rFrameworkView.m_ViewSetup, projection);
		Maths::HVec4 pos4d = Maths::HVec4(worldPos);
		pos4d.W(Maths::Constants<Maths::HF32>::One());
		const Maths::HVec4 transformedPos = pos4d * viewProject;

		Maths::HVec3 result;
		if(transformedPos.W() > 0.00001f)
		{
			result = Maths::HVec3(transformedPos) / (Maths::HF32)transformedPos.W();
			result.Z(Maths::Constants<Maths::HF32>::One());
			isBehind = false;
		}
		else
		{
			result = Maths::HVec3(0.0f, 0.0f, -1.0f);
			isBehind = true;
		}
		
		return result;
	}
	
	//---------------------------------------------------------------------
	// ScreenPosToWorldDir 
	//---------------------------------------------------------------------
	Maths::HVec3		
	C_HollywoodVehicleCamera::ScreenPosToWorldDir(
			const Maths::HVec3& screenPos, 
			const Maths::HMat43& cameraMtx,
			F32 fovY,
			F32 aspectRatio
	) const
	{
		return ScreenPosToViewDir(screenPos,fovY,aspectRatio) * Maths::HMat33(cameraMtx);
	}

	//---------------------------------------------------------------------
	// ScreenPosToViewDir 
	//---------------------------------------------------------------------
	Maths::HVec3		
		C_HollywoodVehicleCamera::ScreenPosToViewDir(
		const Maths::HVec3& screenPos, 
		F32 fovY,
		F32 aspectRatio
		) const
	{
		Maths::HVec3 result;
		F32 fovX = 2.0f * atan(tan(fovY/2.0f) * aspectRatio);
		result.X(tan(fovX/2.0f) * (F32)screenPos.X());	// [Blur2PC] explicit: '*' was ambiguous
		result.Y(tan(fovY/2.0f) * (F32)screenPos.Y());
		result.Z(1.0f);
		result.Normalise();
		return result;
	}
	//---------------------------------------------------------------------
	// CalculateVehicleScreenSpaceBB 
	//---------------------------------------------------------------------
	bool				
	C_HollywoodVehicleCamera::CalculateVehicleScreenSpaceBB( 
		const Maths::HMat44& viewProject,
		const Game::Actors::C_VehicleActor* actor, 
		Maths::HVec3& screenSpaceMin, 
		Maths::HVec3& screenSpaceMax 
	) const
	{
		Maths::HVec3 extentsPoints[8];
		bool isAnyBehind = false;

		screenSpaceMin = Maths::Constants<Maths::HVec3>::Max();
		screenSpaceMax = Maths::Constants<Maths::HVec3>::Min();
		
		Vehicle::C_VehicleModel* vehicleModel = actor->GetVehicleModel();
		const Maths::HVec3 extents = vehicleModel->GetModelInput()->m_vExtents;
		const Maths::HVec3 centre = vehicleModel->GetModelInput()->m_vCentre;

		const Maths::HVec3 extentsMinLS = (centre - extents);
		const Maths::HVec3 extentsMaxLS = (centre + extents);

		
		DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
		
		
		extentsPoints[0] = extentsMinLS;
		extentsPoints[1] = Maths::HVec3(extentsMinLS.X(), extentsMinLS.Y(), extentsMaxLS.Z());
		extentsPoints[2] = Maths::HVec3(extentsMinLS.X(), extentsMaxLS.Y(), extentsMinLS.Z());
		extentsPoints[3] = Maths::HVec3(extentsMinLS.X(), extentsMaxLS.Y(), extentsMaxLS.Z());
		extentsPoints[4] = Maths::HVec3(extentsMaxLS.X(), extentsMinLS.Y(), extentsMinLS.Z());
		extentsPoints[5] = Maths::HVec3(extentsMaxLS.X(), extentsMinLS.Y(), extentsMaxLS.Z());
		extentsPoints[6] = Maths::HVec3(extentsMaxLS.X(), extentsMaxLS.Y(), extentsMinLS.Z());
		extentsPoints[7] = extentsMaxLS;


		for(int i=0;i<8;i++)
		{
			bool isBehind;
			extentsPoints[i] = extentsPoints[i] * actor->GetMatrix();
			//debugRender->Draw3dCross(extentsPoints[i],1.0f,Maths::RGBA(255,0,0,255));
			extentsPoints[i] = WorldSpaceToScreenPos( extentsPoints[i], viewProject ,isBehind);

			isAnyBehind = isBehind?true:isAnyBehind;
		}

		if(!isAnyBehind)
		{
			for(int i=0;i<8;i++)
			{
				screenSpaceMin = screenSpaceMin.Min(extentsPoints[i]);
				screenSpaceMax = screenSpaceMax.Max(extentsPoints[i]);
			}
		}

		return !isAnyBehind;
	}


	//---------------------------------------------------------------------
	// SetLookAround 
	//---------------------------------------------------------------------
	void C_HollywoodVehicleCamera::SetLookAround(F32 angle)
	{
		m_lookAngle = angle;
	}

	//---------------------------------------------------------------------
	// SetLookAround 
	// From the control stick values
	//---------------------------------------------------------------------
	void C_HollywoodVehicleCamera::SetLookAround(F32 sx, F32 sy)
	{
		F32 mag = sx * sx + sy * sy; 
		if (mag >= 0.2f * 0.2f)
		{
			F32 lookAngle = atan2(sx, sy);
			
			m_lookAngle = lookAngle;
			m_lookX = sx;
			m_lookY = sy;
		}
		else
		{
			m_lookAngle = 0.f; // Not pressed - look straight ahead:
			m_lookX = 0.0f;
			m_lookY = 0.0f;
		}
	}

	//---------------------------------------------------------------------
	// SetOffset
	//---------------------------------------------------------------------

	void C_HollywoodVehicleCamera::SetOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov)
	{
		m_Offset = offset;
	}

	void C_HollywoodVehicleCamera::AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov)
	{
		if(m_tracking)
		{
			m_EffectOffset += offset;
			m_EffectOffsetRot += offsetrot;
			m_EffectFov += fov;
		}
	}

	bool
	C_HollywoodVehicleCamera::ShouldDoHits(const hollywoodDataCache& data) const
	{
		bool doHits = m_tracking;
		return doHits;
	}
	//---------------------------------------------------------------------
	// CalculateOverlayQuaternion 
	//---------------------------------------------------------------------
	Maths::C_Quaternion 
	C_HollywoodVehicleCamera::CalculateOverlayQuaternion(const hollywoodDataCache& data) const
	{
		Maths::C_Quaternion result;
		return result;
	}

	//---------------------------------------------------------------------
	// CalculateOverlayPosition 
	//---------------------------------------------------------------------
	Maths::HVec3		
	C_HollywoodVehicleCamera::CalculateOverlayPosition(const hollywoodDataCache& data) const
	{
		Maths::HVec3 result;
		return result;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredLookAroundAngle 
	//---------------------------------------------------------------------
	F32
	C_HollywoodVehicleCamera::CalculateDesiredLookAroundAngle(const hollywoodDataCache& data ) const
	{
		F32 desiredLookAngle = -m_lookAngle;

		if(!m_tracking)
		{
			desiredLookAngle = 0.0f;
		}

		return desiredLookAngle;
	}

	//---------------------------------------------------------------------
	// CalculateDesiredLookAroundSmoothing 
	//---------------------------------------------------------------------
	F32
	C_HollywoodVehicleCamera::CalculateDesiredLookAroundSmoothing(const hollywoodDataCache& data ) const
	{
		static F32 s_DefaultLookAroundSmoothing = 0.03f;
		F32 desiredLookAroundSmoothing = s_DefaultLookAroundSmoothing;

		if(m_lookBackQuick)
		{
			desiredLookAroundSmoothing = 0.0f;
		}
		return desiredLookAroundSmoothing;
	}

	//---------------------------------------------------------------------
	// UpdateLookAround 
	//---------------------------------------------------------------------
	void
	C_HollywoodVehicleCamera::UpdateLookAround(const hollywoodDataCache& data, F32 timeElapsed)
	{
		// Data
		static F32 s_quickLookDeadZone = 0.35f;
		static F32 s_quickLookXThreshold = 0.3f;
		static F32 s_quickLookOffAngleThreshold = 0.55f;
		static F32 s_quickLookDamping = 0.90f;
		static F32 s_quickLookMagThreshold = 0.3f;

		F32 LookMag = Maths::Sqrt((m_lookX * m_lookX) + (m_lookY * m_lookY));
		LookMag = Maths::Clamp<F32>(LookMag, 0.0f, 1.0f);

		// State management
		if(!m_lookSide)
		{
			if(fabs(m_lookX) > s_quickLookXThreshold)
			{
				m_lookSide = true;
			}
		}
		else
		{
			if( fabs(m_lookAngle)< s_quickLookDeadZone )
			{
				m_lookSide = false;
			}
		}

		if(!m_lookBackQuick)
		{
			if( m_lookY<0.0f && 
				fabs(m_lookAngle)>Maths::Pi-s_quickLookDeadZone &&
				!m_lookSide)
			{
				m_lookBackQuick = true;
			}
		}
		else
		{
			if( (fabs(m_lookAngle) < Maths::Pi - s_quickLookOffAngleThreshold) &&
				LookMag > s_quickLookMagThreshold )
			{
				m_lookBackQuick = false;
			}
		}

		// Variables updates
		F32 desiredDeadzone = m_lookBackQuick ? s_quickLookDeadZone:0.0f;
		m_lookDeadZone += (desiredDeadzone - m_lookDeadZone) * DampingAdjust(s_quickLookDamping,timeElapsed);
			
		F32 minAngleRange = m_lookDeadZone;
		F32 maxAngleRange = Maths::Pi - m_lookDeadZone;
		F32 remapped = (fabs(m_lookAngle) - minAngleRange) / (maxAngleRange-minAngleRange);
		remapped = Maths::Clamped<F32>(remapped,0.0f,1.0f) * Maths::Pi * (m_lookAngle<0.0f?-1.0f:1.0f);

		m_lookAngle = remapped;
	}

	//---------------------------------------------------------------------
	// ExtractParamsFromSourcery
	// Transfers the sorcery parameters into our custom structure.
	// Part of this process also converts values such as angles from degrees
	// to radians
	//---------------------------------------------------------------------
	HollywoodVehicleParams 
	C_HollywoodVehicleCamera::ExtractParamsFromSourcery() 
	{
		const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);

		HollywoodVehicleParams params;

		if (!vehicleActor)
		{
			BIZ_ASSERT_MSG(0,"C_HollywoodVehicleCamera::ExtractParamsFromSourcery() - no valid VehicleActor  SN added this assert to see if we should be handling this case");
			//params = ExtractParamsFromDebugVars();
		}
		else
		{			
			params = vehicleActor->GetHollywoodCameraParams(m_debugIndex);
			
			
			// Convert values as necessary
			params.IdealPitch = DegToRad(params.IdealPitch);
			params.IdealPitchAirbourneUp = DegToRad(params.IdealPitchAirbourneUp);
			params.IdealPitchAirbourneDown = DegToRad(params.IdealPitchAirbourneDown);
			params.CameraCarRollDiffMax = DegToRad(params.CameraCarRollDiffMax);
			params.LookAtMaxAngVel = DegToRad(params.LookAtMaxAngVel);
			params.LookAtMinAngVel = DegToRad(params.LookAtMinAngVel);
			params.UpsideDownDotProductThreshold = DegToRad(params.UpsideDownDotProductThreshold);
			params.UpsideDownDotProductThreshold = cosf(params.UpsideDownDotProductThreshold);
			params.RegainedControlAngVelThreshold = DegToRad(params.RegainedControlAngVelThreshold);
			params.RotationalUpperThreshold = DegToRad(params.RotationalUpperThreshold);
			params.RotationalLowerThreshold = DegToRad(params.RotationalLowerThreshold);
			params.PitchMaxErrorNormal = DegToRad(params.PitchMaxErrorNormal);
			params.NormalFOV = DegToRad(params.NormalFOV);
			params.YawRollMaxYaw = DegToRad(params.YawRollMaxYaw);
			params.YawRollMinYaw = DegToRad(params.YawRollMinYaw);
			params.YawRollMax = DegToRad(params.YawRollMax);
			params.YawMaxErrorNormal = DegToRad(params.YawMaxErrorNormal);
			
			
		}
		return params;
	}

	//---------------------------------------------------------------------
	// GetParams
	// Returns a structure containing all the variable camera parameters.
	// The source of these parameters is internally chosen based on the 
	// overrideDefaults debug variable.
	//---------------------------------------------------------------------
	const HollywoodVehicleParams&
	C_HollywoodVehicleCamera::GetParams() const
	{
		return m_currentParameters;
	}

	//-----------------------------------------------------------------------------
	//  DebugRender
	//-----------------------------------------------------------------------------
	void 
	C_HollywoodVehicleCamera::DebugRender(RGBA colour, bool renderFrustum) const
	{
#if !CFG_CONVERTER && !defined(CFG_GOLD)

		const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);
		if(vehicleActor)
		{
			Gravity::C_Body* body = vehicleActor->GetCollisionBody();

		
			HVec3 RaytracePosLS = m_targetPos;
			if (body)
			{
				RaytracePosLS.Y(body->GetLocalBoundingBox().extents.Y()+
								body->ElementOffset().Y() + 
								body->GetLocalBoundingBox().centre.Y());
			}
			
			HVec3 RaytracePosWS = RaytracePosLS * m_target;

			DbUtil::I_DebugRender* debugRender = Cam2::GetDebugRender();
			debugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);

			debugRender->Draw3dCross(RaytracePosWS,0.3f,Maths::RGBA(255,0,0,255));
		}
#endif
		C_ThirdPersonCamera::DebugRender(colour,renderFrustum);
	}


} // End of namespace Kanzen
