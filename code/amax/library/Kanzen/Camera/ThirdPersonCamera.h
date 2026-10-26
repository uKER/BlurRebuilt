//=============================================================================
// C_ThirdPersonCamera.h
// 25/01/09
//=============================================================================
#ifndef _THIRDPERSONCAMERA_H
#define _THIRDPERSONCAMERA_H
//=============================================================================
// Includes
//=============================================================================
#include <camera2/Camera.h>
#include <camera2/stock/ChaseParam.h>
#include <actor_system/ActorDeclare.h>

//=============================================================================
// Defines
//=============================================================================
#define		    TPCAMERA_UPDATE_TRANSFORM		(1<<0)
//=============================================================================
// Class Defintion
//=============================================================================

namespace Kanzen
{
	class C_ThirdPersonCamera : public Cam2::C_Camera
	{
	public:
		
		struct SIdealData
		{
			SIdealData()	{ Reset(); }
			void Reset();	 
			F32	min;
			F32	max;
			F32 smoothing;
			F32	speed;
			F32	velocity;
			F32 maxError;
		};
		
		typedef Cam2::C_Camera inherited;
	
	    

			C_ThirdPersonCamera();
				
			void		OnUpdate() OVERRIDE;
			void		Setup(const Maths::HMat43& TargetTransform,
							  const Maths::HVec3& TargetPos, 
							  const Maths::HVec3& TargetAimPos,
							  F32 Pitch, 
							  F32 Yaw,
							  F32 Roll,
							  F32 Distance);
			void		SetTargetTransform(const Maths::HMat43& target);	// The world matrix of the target, eg the players matrix 	
			void		SetTargetPos(const Maths::HVec3& targetPos);		// The target space base that all position offsets are calculated from. eg the players head
			void		SetTargetAimPos(const Maths::HVec3& targetAimPos);	// The target space aim position, eg a position in front of the head
			void		SetDistanceTrackingEnabled(bool enabled,bool immediate=true);
			void		SetYawTrackingEnabled(bool enabled,bool immediate=true);
			void		SetPitchTrackingEnabled(bool enabled,bool immediate=true);
			void		SetRollTrackingEnabled(bool enabled,bool immediate=true);
			void		SetIdealDistanceRange(F32 distMin, F32 distMax);
			void		SetIdealDistance(F32 dist);
			void		SetIdealDistanceDamping(F32 damping);
			void		SetIdealDistanceMaxSpeed(F32 speed);
			void		SetIdealDistanceMaxError(F32 error);
			void		SetIdealDistanceMaxErrorSoften(bool soften);
			void		SetIdealYawRange(F32 distMin, F32 distMax);
			void		SetIdealYaw(F32 yaw);
			void		SetIdealYawDamping(F32 damping);
			void		SetIdealYawMaxSpeed(F32 speed);
			void		SetIdealYawMaxError(F32 error);
			void		SetIdealYawMaxErrorSoften(bool soften);
			void		SetIdealPitchRange(F32 distMin, F32 distMax);
			void		SetIdealPitch(F32 pitch);
			void		SetIdealPitchDamping(F32 damping);
			void		SetIdealPitchMaxSpeed(F32 speed);
			void		SetIdealPitchMaxError(F32 error);
			void		SetIdealPitchMaxErrorSoften(bool soften);
			void		SetIdealRollRange(F32 yawMin, F32 yawMax);
			void		SetIdealRoll(F32 roll);
			void		SetIdealRollDamping(F32 damping);
			void		SetIdealRollMaxSpeed(F32 speed);
			void		SetIdealRollMaxError(F32 error);
			void		SetIdealRollMaxErrorSoften(bool soften);
			void		SetIdeals(F32 pitch, F32 yaw, F32 roll,F32 distance, F32 tollerence = 0.0f);
			void		SetUpAxis(const Maths::HVec3& up);
			void		SetUpAxisAlwaysFromTransform();

			F32			CalculateCurrentRoll() const;
			F32			CalculateCurrentPitch() const;
			F32			CalculateCurrentYaw() const;
			F32			CalculateCurrentDistance() const;
			
			void		CalculateLook();
			void		ResetParams();
			
			F32			GetRoll();
			void		GetTargetTransform(Maths::HMat43& target);
			void		GetTargetPos(Maths::HVec3& pos);
			void		GetTargetAimPos(Maths::HVec3& pos);
			void		GetRollData(SIdealData& rollData);
			void		GetPitchData(SIdealData& rollData);
			void		GetYawData(SIdealData& rollData);
			void		GetDistanceData(SIdealData& rollData);

	virtual void		DebugRender(Maths::RGBA colour = Maths::RGBA(255, 255, 255, 255), bool renderFrustum = false) const;

	protected:
		
			F32			CalculateCurrentRoll(const Maths::HMat33& targetSpaceCameraMtx ) const;
			F32			CalculateCurrentPitch(const Maths::HVec3& targetSpaceCameraPosition ) const;
			F32			CalculateCurrentYaw(const Maths::HVec3& targetSpaceCameraPosition ) const;
			F32			CalculateCurrentDistance(const Maths::HVec3& targetSpaceCameraPosition ) const;

			F32			CalculateCurrentRollError() const;
			F32			CalculateCurrentPitchError() const;
			F32			CalculateCurrentYawError() const;
			F32			CalculateCurrentDistanceError() const;

			F32			 CalculateClosenessToUpVectorFactor(const Maths::HVec3& lookDir) const;
			Maths::HVec3 CalculateUpVector(const Maths::HVec3& lookDir) const;
			Maths::HVec3 CalculateNewPosition(F32 distance, F32 yaw, F32 pitch) const;
			Maths::C_Quaternion 
				        CalculateLookQuaternion(const Maths::HVec3& direction, Maths::HVec3 up, F32 roll ) const;
			
		
	
			F32				m_rollCurrent;
			Maths::HMat43	m_target;
			Maths::HVec3	m_targetPos;
			Maths::HVec3	m_targetAimPos;
			Maths::HVec3	m_targetSpaceCameraPosition;
			Maths::HVec3	m_up;
			Maths::HVec3	m_desiredUp;
			Maths::HVec3	m_lastValidUp;
			SIdealData		m_roll;
			SIdealData		m_pitch;
			SIdealData		m_yaw;
			SIdealData		m_distance;
			U16				m_flags;
	};
}

#endif //_THIRDPERSONCAMERA_H



