//=============================================================================
// C_HollywoodVehicleCamera.h
// 25/01/09
//=============================================================================
#ifndef _HOLLYWOODVEHICLECAMERA_H
#define _HOLLYWOODVEHICLECAMERA_H

//=============================================================================
// Includes
//=============================================================================
#include <camera2/Camera.h>
#include <camera2/IPhysics.h>
#include <camera2/stock/ChaseParam.h>
#include <actor_system/ActorDeclare.h>
#include "ThirdPersonCamera.h"
#include "Utility.h"
#include <Xt/XtGame.h>

namespace Game { namespace Actors { class C_VehicleActor; } }

//=====================================================================
// Forward Declarations
//=====================================================================
namespace RacingPhysics
{
	class Car;
}
namespace Vehicle
{
	class C_ModelInput;
}
//=====================================================================
// Structs
//=====================================================================
namespace Kanzen
{
	
	struct HollywoodVehicleParams
	{
		F32 NormalFOV;

		F32 IdealDistanceNormal;
		F32 IdealDistanceDampingNormal;
		F32 IdealDistanceAtFullAcceleration;
		F32 IdealDistanceAtFullDeceleration;
		F32 IdealDistanceAirbourne;
		F32 IdealDistanceSmoothing;
		F32 IdealDistanceSmoothingAccelerating;
		F32 IdealDistanceSmoothingDecelerating;
		F32	IdealDistanceSmoothingAirbourne;
		F32 MaxAcceleration;
		F32 MaxDeceleration;

		F32 IdealPitchAirbourneUp;
		F32 IdealPitchAirbourneDown;
		
		F32 IdealPitch;
		XtVec3 IdealBase;
		
		F32 PitchDampingNormal;
		F32 PitchDampingAirbourneUp;
		F32 PitchDampingAirbourneDown;
		F32 MaxUpwardsVelocity;
		F32 MaxDownwardsVelocity;
		
		F32 YawDampingNormal;
		F32 YawDampingSmoothValue;
		F32 CameraCarRollDiffMax;
		F32 LostControlRollSmoothing;
		F32 UprightRollSmoothing;
		F32 BarrelRollSmoothing;
		F32 PitchMaxSpeedNormal;
		F32 PitchMaxErrorNormal;
		F32 YawMaxSpeedNormal;
		F32 YawMaxErrorNormal;

		F32 YawRollFwdsVelThreshold;
		F32 YawRollMinYaw;
		F32 YawRollMaxYaw;
		F32 YawRollMax;

		F32 LookSideMax;
		F32 LookSideUpDownMultiplier;
		F32 ForwardsVelocityUpDownMultiplier;
		F32 LookAtSmoothingValue;
		F32 LookAtMaxAngVel;
		F32 LookAtMinAngVel;
		XtVec3 LookAtPos;

		F32 WheelNoContactThreshold;
		F32 UpsideDownDotProductThreshold;
		F32 RegainedControlAngVelThreshold;
		F32 RegainedControlThrottleThreshold;
		F32 TargetTransformSmooth;
 
		F32 RotationalUpperThreshold;
		F32 RotationalLowerThreshold;
		F32 BurnoutTimerThreshold;

		F32 HighSpeedLowerVelThreshold;
		F32 HighSpeedUpperVelThreshold;
		F32 HighSpeedFovMultiplier;
		
	};



	//=============================================================================
	// Class Definition
	//=============================================================================

	class C_HollywoodVehicleCamera : public C_ThirdPersonCamera
	{
		static const int NUM_WHEELS = 4;

			typedef  C_ThirdPersonCamera inherited;

	public:
									C_HollywoodVehicleCamera();
		
				void				OnUpdate() OVERRIDE;

				
				void				SetLookAround(F32 angle);
				void				SetLookAround(F32 sx, F32 sy);
				void				SetPhysics(const Cam2::I_Physics* physics) { m_physics = physics; }
				void				SetOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov);
				void				AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov);
				void				SetOffset(const Maths::HVec3& offset) { m_Offset = offset; }
				void				VehicleReset();
				void				SetHandle(const Actor::C_ActorHandle& handle);
			
				F32					GetLookAngle() { return m_lookAngle; }
				const Cam2::I_Physics* 
									GetPhysics() { return m_physics; }
				const Maths::C_Quaternion& 
									GetLookOrient() { return m_lookOrient; }
			
				bool				GetLostControl() { return m_lostControl; }
				bool				GetBurningOut() { return m_burningout; }

				void				SetRaytrace(bool raytrace) { m_Raytrace = raytrace; }

				S32					m_debugIndex;
		
	protected:

		
		
		

		class hollywoodDataCache
		{
		public:
			void		Reset();
			const Game::Actors::C_VehicleActor* m_actor;
			const RacingPhysics::Car* m_physics;
			const Vehicle::C_ModelInput* m_input;
			Maths::HVec3 m_linVel;
			Maths::HVec3 m_linVelLS;
			Maths::HVec3 m_angVel;
			Maths::HVec3 m_angVelLS;
			Maths::HVec3 m_linAccel;
			Maths::HVec3 m_linAccelLS;
			F32 m_linVelMag;
			F32 m_angVelMag;
			F32 m_airbourneTime;
			F32 m_vehicleUpDotWorldUp;
			Maths::HMat43 m_vehicleTransform;
			F32 m_vehicleRoll;
			F32 m_vehiclePitch;
			F32 m_FrontWheelsSlip;
			F32 m_BackWheelSlip;
		};

		class hollywoodPersistantDataCache
		{
		public:
			void		Reset();
			F32 m_wheelWithoutContactTime[ NUM_WHEELS ];
			F32 m_landedTimer;
			F32 m_antiPendulumTimer;
			F32 m_landedLinVelDrama;
			F32 m_groundTime;
			F32 m_dampedThrottle;
			F32 m_fixedFovWidth;
			bool m_highSpeed;
			bool m_reversing;
			bool m_jTurning;
			F32 m_reversingTimer;

			SmoothingData m_cameraTransformXOffset;
	
		
		};
				void				DebugRender(Maths::RGBA colour, bool renderFrustum) const;
				void				BlankParams();
				F32					CalculateDesiredYaw(const hollywoodDataCache& data) const;
				F32					CalculateDesiredYawTollerance(const hollywoodDataCache& data) const;
				F32					CalculateDesiredYawMaxError(const hollywoodDataCache& data);
				F32					CalculateDesiredLookUpDown(const hollywoodDataCache& data) const;
				F32					CalculateDesiredLookSide(const hollywoodDataCache& data) const;
				F32					CalculateDesiredYawDamping(const hollywoodDataCache& data) const;
				F32					CalculateDesiredYawDampingSmooth(const hollywoodDataCache& data) const;
				F32					CalculateDesiredRoll(const hollywoodDataCache& data) const;
				F32					CalculateDesiredRollDamping(const hollywoodDataCache& data) const;
				const Maths::HVec3  CalculateDesiredUpAxis(const hollywoodDataCache& data) const;
				F32					CalculateDesiredPitch(const hollywoodDataCache& data) const;
				F32					CalculateDesiredPitchDamping(const hollywoodDataCache& data) const;
				F32					CalculateDesiredPitchErrorMax(const hollywoodDataCache& data) const;
				F32					CalculateDesiredTransXOffset(const hollywoodDataCache& data) const;
				F32					CalculateDesiredTransXOffsetSmooth(const hollywoodDataCache& data) const;
				F32					CalculateDesiredDistance(const hollywoodDataCache& data) const;
				F32					CalculateDesiredDistanceDamping(const hollywoodDataCache& data) const;
				F32					CalculateDesiredDistanceSmooth(const hollywoodDataCache& data) const;
				F32					CalculateDesiredDistanceMaxSpeed(const hollywoodDataCache& data) const;
				F32					CalculateDesiredFov(const hollywoodDataCache& data) const;
				F32					CalculateDesiredAntiPendulum(const hollywoodDataCache& data) const;
				F32					CalculateDesiredAntiPendulumSmooth(const hollywoodDataCache& data) const;
				F32					CalculateDesiredLookAroundAngle(const hollywoodDataCache& data) const;
				F32					CalculateDesiredLookAroundSmoothing(const hollywoodDataCache& data) const;

				bool				CalculateVehicleScreenSpaceBB( const Maths::HMat44& viewProject, const Game::Actors::C_VehicleActor*, Maths::HVec3& screenSpaceMin, Maths::HVec3& screenSpaceMax ) const;
				Maths::HMat44		CalculateViewProject( int viewIndex, const Maths::Mat43& cameraMtx, F32 fov	);
				Maths::HVec3		WorldSpaceToScreenPos( const Maths::HVec3& worldPos, const Maths::HMat44& viewProject, bool& isBehind ) const;
				Maths::HVec3		ScreenPosToWorldDir( const Maths::HVec3& screenPos, const Maths::HMat43& cameraMtx, F32 fovY, F32 aspectRatio ) const;
				Maths::HVec3		ScreenPosToViewDir( const Maths::HVec3& screenPos, F32 fovY, F32 aspectRatio ) const;

				bool				ShouldDoHits(const hollywoodDataCache& data) const;
				Maths::C_Quaternion CalculateOverlayQuaternion(const hollywoodDataCache& data) const;
				Maths::HVec3		CalculateOverlayPosition(const hollywoodDataCache& data) const;
	
				void				UpdateLookAround(const hollywoodDataCache& data, F32 timeElapsed);

			   HollywoodVehicleParams
									ExtractParamsFromSourcery();
					const HollywoodVehicleParams& 
									GetParams() const;


				HollywoodVehicleParams m_currentParameters;
			

				F32					m_lookDeadZone;
				bool				m_lookSide;
				bool				m_lookBackQuick;
				F32					m_lookX;
				F32					m_lookY;
				F32					m_lookAngle;
				Maths::C_Quaternion	m_lookOrient;
		
				Maths::HVec3 		m_EffectOffset;
				Maths::HVec3 		m_EffectOffsetRot;
				F32 				m_EffectFov;
				
				F32					m_WreckedTimer;
				F32					m_wreckedIdealPitch;
				bool				m_Wrecked;
				Maths::HVec3		m_WreckedCamPos;


				const Cam2::I_Physics* m_physics;
		F32 m_SafeHeight;
		Maths::HVec3 m_Offset;
		
		Actor::C_ActorHandle m_Handle;

		Cam2::C_ChaseParam CurrentParam();

		hollywoodDataCache m_dataLast;
		hollywoodPersistantDataCache m_persistantData;
		hollywoodPersistantDataCache m_persistantDataLast;

		bool m_tracking;
		bool m_burningout;
		F32 m_burningoutTimer;
		bool m_lostControl;

		SmoothingData m_antiPendulumTransformSmoothData;
		SmoothingData m_crashedTransformSmoothData;
		SmoothingData m_lookSideSmoothData;
		SmoothingData m_lookUpDownSmoothData;
		SmoothingData m_yawDampingSmoothData;
		SmoothingData m_rollDampingSmoothData;
		SmoothingData m_distanceSmoothData;
		SmoothingData m_distanceSmoothSmoothData;
		SmoothingData m_pitchDampingSmoothData;
		SmoothingData m_ThrottleSmoothData;
		SmoothingData m_FovSmoothData;
		SmoothingData m_cameraTransformXOffset;
		SmoothingData m_lookAngleSmoothData;
		SmoothingData m_MaxDistanceSmoothData;
		
		Maths::HVec3 m_unmodifiedPosition;
		Maths::C_Quaternion m_unmodifiedRotation;
		bool m_Raytrace;
	};

} // End of namespace Kanzen

#endif  //_HOLLYWOODVEHICLECAMERA_H
