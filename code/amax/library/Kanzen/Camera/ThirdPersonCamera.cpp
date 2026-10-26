//=============================================================================
// C_ThirdPersonCamera.cpp
// 25/01/09
//=============================================================================
//=============================================================================
// Includes
//=============================================================================
#include "stdafx.h"
#include "Kanzen_PCH.h"
#include "ThirdPersonCamera.h"
#include "camera2/CameraType.h"

#include <game/AmaxCam/Helper.h>
#include <game/AmaxCam/PhysicsChecks.h>

#include "System/debug/I_DebugRender.h"
#include "core\maths\ValidateFloat.h"

#include "Utility.h"
//=============================================================================
// Defines
//=============================================================================
enum 
{
	TPCAMERA_DISTANCE_TRACKING_ENABLED			= (1<<0),
	TPCAMERA_PITCH_TRACKING_ENABLED				= (1<<1),
	TPCAMERA_YAW_TRACKING_ENABLED				= (1<<2),
	TPCAMERA_ROLL_TRACKING_ENABLED				= (1<<3),	
	TPCAMERA_DISTANCE_TRACKING_DELAYED_DISABLE	= (1<<4),
	TPCAMERA_PITCH_TRACKING_DELAYED_DISABLE		= (1<<5),
	TPCAMERA_YAW_TRACKING_DELAYED_DISABLE		= (1<<6),
	TPCAMERA_ROLL_TRACKING_DELAYED_DISABLE		= (1<<7),
	TPCAMERA_OVERRIDE_UP_AXIS					= (1<<8),
	TPCAMERA_DISTANCE_SOFT_MAX_ERROR			= (1<<9),
	TPCAMERA_PITCH_SOFT_MAX_ERROR				= (1<<10),
	TPCAMERA_YAW_SOFT_MAX_ERROR					= (1<<11),
	TPCAMERA_ROLL_SOFT_MAX_ERROR				= (1<<12),

};

//=============================================================================
// Data
//=============================================================================
#ifndef CFG_GOLD	

static Debug::Bool  OverrideDistance("Kanzen.Camera.ThirdPersonCamera.Overrides.OverrideDistance", false);
static Debug::Bool  OverridePitch("Kanzen.Camera.ThirdPersonCamera.Overrides.OverridePitch", false);
static Debug::Bool  OverrideYaw("Kanzen.Camera.ThirdPersonCamera.Overrides.OverrideYaw", false);
static Debug::Bool  OverrideRoll("Kanzen.Camera.ThirdPersonCamera.Overrides.OverrideRoll", false);
static Debug::Float OverrideDistanceValue("Kanzen.Camera.ThirdPersonCamera.Overrides.Distance", 5.0f, Debug::Range( 0.f, 100.f));
static Debug::Float OverridePitchValue("Kanzen.Camera.ThirdPersonCamera.Overrides.Pitch", 0.0f, Debug::Range( -360.0f, 360.0f));
static Debug::Float OverrideYawValue("Kanzen.Camera.ThirdPersonCamera.Overrides.Yaw", 5.0f, Debug::Range( -360.0f, 360.0f));
static Debug::Float OverrideRollValue("Kanzen.Camera.ThirdPersonCamera.Overrides.Roll", 5.0f, Debug::Range( -360.0f, 360.0f)); 
static Debug::Float MaxOrientChange("Kanzen.Camera.ThirdPersonCamera.Overrides.MaxOrientAngle", -1.0f, Debug::Range( -1.0f, 360.0f)); 


#endif
//=============================================================================
// Class Implementation
//=============================================================================
namespace Kanzen
{
	using namespace Maths;

	C_ThirdPersonCamera::C_ThirdPersonCamera()
	{
		SetType(Cam2::E_ChaseCamera2);
		SetName("Third");

		m_flags =	TPCAMERA_DISTANCE_TRACKING_ENABLED | TPCAMERA_PITCH_TRACKING_ENABLED | TPCAMERA_YAW_TRACKING_ENABLED | TPCAMERA_ROLL_TRACKING_ENABLED;

		m_pitch.min = DegToRad( 20.0f );
		m_pitch.max = DegToRad( 25.0f );
		m_distance.min = 2.5f;
		m_distance.max = 3.5f;
		m_distance.smoothing = 0.05f;
		m_yaw.smoothing = 0.0f;
		m_pitch.smoothing = 0.1f;
		m_roll.min = DegToRad( 0.0f );
		m_roll.max = DegToRad( 0.0f );
		m_rollCurrent = 0.0f;
		m_up = Constants< HVec3 >::UnitAxisPositiveY();
		m_lastValidUp=Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
		m_desiredUp = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
	}

	
	//-----------------------------------------------------------------------------
	//  SetupDefaults
	//-----------------------------------------------------------------------------
	void
	C_ThirdPersonCamera::Setup( 
		const HMat43& TargetTransform,
		const HVec3& TargetPos, 
		const HVec3& TargetAimPos,
		F32 Pitch, 
		F32 Yaw,
		F32 Roll,
		F32 Distance)
	{
		m_lastValidUp=Maths::Constants<Maths::HVec3>::UnitAxisNegativeZ();
		SetTargetTransform(TargetTransform);
		SetTargetPos(TargetPos);
		SetTargetAimPos(TargetAimPos);
		m_rollCurrent = Roll;
		m_lastValidUp=Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
		m_desiredUp = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
		SetPosition(CalculateNewPosition(Distance, Yaw, Pitch));
		CalculateLook();
		
	}

	//-----------------------------------------------------------------------------
	//  CalculateClosenessToUpVectorFactor
	//-----------------------------------------------------------------------------
	F32
	C_ThirdPersonCamera::CalculateClosenessToUpVectorFactor(const Maths::HVec3& lookDir) const
	{
		Maths::HVec3 lookDirNrm = Normalise(lookDir);

		Maths::HVec3 desiredUp;
		if((m_flags & TPCAMERA_OVERRIDE_UP_AXIS)==0)
		{
			desiredUp = m_target.AY();
		}
		else
		{
			desiredUp = m_desiredUp;
		}

		F32 closeToYAxisFactor = DotProduct(lookDirNrm,desiredUp);
		closeToYAxisFactor = fabs(closeToYAxisFactor);

		static F32 closeToYAxisMin = Maths::Cos(Maths::DegToRad(25.0f));
		// blended
		closeToYAxisFactor = (closeToYAxisFactor - closeToYAxisMin)/(1.0f-closeToYAxisMin);
		closeToYAxisFactor = Maths::Clamped<F32>(closeToYAxisFactor,0.0f,1.0f);

		// switched
		//closeToYAxisFactor = closeToYAxisFactor > closeToYAxisMin ? 1.0f:0.0f;
		return closeToYAxisFactor;
	}

	//-----------------------------------------------------------------------------
	//  CalculateUpVector
	//-----------------------------------------------------------------------------
	Maths::HVec3
	C_ThirdPersonCamera::CalculateUpVector(const Maths::HVec3& lookDir) const
	{
		F32 factor = CalculateClosenessToUpVectorFactor(lookDir);

		Maths::HVec3 desiredUp;
		Maths::HVec3 up;
		if((m_flags & TPCAMERA_OVERRIDE_UP_AXIS)==0)
		{
			desiredUp = m_target.AY();
		}
		else
		{
			desiredUp = m_desiredUp;
		}
		up=m_desiredUp.Lerp(m_target.AY(),factor);
		/*if(factor<0.01f)
		{
			up=desiredUp;
		}
		else
		{
			up=m_lastValidUp;
		}*/
		
		return up;
	}
	//-----------------------------------------------------------------------------
	//  Calculate Look
	//-----------------------------------------------------------------------------
	void		
	C_ThirdPersonCamera::CalculateLook()
	{
		HVec3 PosWS = Position();
		HVec3 LookPosWS = m_targetAimPos * m_target;
		HVec3 LookDirWS = LookPosWS - PosWS;
		
		Maths::HVec3 up = CalculateUpVector(LookDirWS);
		C_Quaternion desiredOrient = CalculateLookQuaternion( LookDirWS, up, m_rollCurrent );
		
		F32 factor = CalculateClosenessToUpVectorFactor(LookDirWS);
		if(factor<0.01f)
		{
			Maths::HMat33 cameraMat(desiredOrient);
			m_lastValidUp=cameraMat.AY();
		}
		SetOrient(desiredOrient);
	}

	//-----------------------------------------------------------------------------
	//  Sets/Gets
	//-----------------------------------------------------------------------------
	void		
	C_ThirdPersonCamera::SetTargetTransform( const HMat43& target )		// The world matrix of the target, eg the players matrix 
	{			
		m_target = target;
	}

	void
	C_ThirdPersonCamera::SetTargetPos( const HVec3& targetPos )
	{
		m_targetPos = targetPos;  
	}

	void
	C_ThirdPersonCamera::SetTargetAimPos( const HVec3& targetAimPos )
	{
		m_targetAimPos = targetAimPos;
	}

	void		
	C_ThirdPersonCamera::SetDistanceTrackingEnabled( bool enabled, bool immediate )
	{
		if(enabled||immediate)
		{
			if(!enabled)
			{
				m_distance.velocity = 0.0f;
			}

			m_flags = enabled ? m_flags | TPCAMERA_DISTANCE_TRACKING_ENABLED : 
								m_flags & ~TPCAMERA_DISTANCE_TRACKING_ENABLED; 

			m_flags &= ~TPCAMERA_DISTANCE_TRACKING_DELAYED_DISABLE;
		}
		else
		{
			m_flags |= TPCAMERA_DISTANCE_TRACKING_DELAYED_DISABLE;	
		}
	}

	void		
	C_ThirdPersonCamera::SetYawTrackingEnabled( bool enabled, bool immediate )
	{
		if(enabled||immediate)
		{
			if(!enabled)
			{
				m_yaw.velocity = 0.0f;
			}
		
			m_flags = enabled ? m_flags | TPCAMERA_YAW_TRACKING_ENABLED : 
								m_flags & ~TPCAMERA_YAW_TRACKING_ENABLED;

			m_flags &= ~TPCAMERA_YAW_TRACKING_DELAYED_DISABLE;
		}
		else
		{
			m_flags |= TPCAMERA_YAW_TRACKING_DELAYED_DISABLE;	
		}
	}

	void		
	C_ThirdPersonCamera::SetPitchTrackingEnabled( bool enabled, bool immediate )
	{
		if(enabled||immediate)
		{
			if(!enabled)
			{
				m_pitch.velocity = 0.0f;
			}

			m_flags = enabled ? m_flags | TPCAMERA_PITCH_TRACKING_ENABLED : 
								m_flags & ~TPCAMERA_PITCH_TRACKING_ENABLED; 

			m_flags &= ~TPCAMERA_PITCH_TRACKING_DELAYED_DISABLE;
		}
		else
		{
			m_flags |= TPCAMERA_PITCH_TRACKING_DELAYED_DISABLE;		
		}
	}

	void		
	C_ThirdPersonCamera::SetRollTrackingEnabled( bool enabled, bool immediate )
	{
		if(enabled||immediate)
		{
			if(!enabled)
			{
				m_roll.velocity = 0.0f;
			}

			m_flags = enabled ? m_flags | TPCAMERA_ROLL_TRACKING_ENABLED : 
								m_flags & ~TPCAMERA_ROLL_TRACKING_ENABLED; 

			m_flags &= ~TPCAMERA_ROLL_TRACKING_DELAYED_DISABLE;
		}
		else
		{
			m_flags |= TPCAMERA_ROLL_TRACKING_DELAYED_DISABLE;
		}
	}	

	void		
	C_ThirdPersonCamera::SetIdealDistanceRange( F32 distMin, F32 distMax )
	{
		m_distance.min = distMin;
		m_distance.max = distMax;
	}

	void		
	C_ThirdPersonCamera::SetIdealDistance( F32 dist )
	{
		m_distance.min = dist;
		m_distance.max = dist;
	}

	void		
	C_ThirdPersonCamera::SetIdealDistanceDamping( F32 damping )
	{
		m_distance.smoothing = damping;
	}

	void		
	C_ThirdPersonCamera::SetIdealDistanceMaxSpeed( F32 speed )
	{
		m_distance.speed = speed;
	}

	void		
	C_ThirdPersonCamera::SetIdealDistanceMaxError( F32 error )
	{
		m_distance.maxError = error;
	}

	void		
	C_ThirdPersonCamera::SetIdealDistanceMaxErrorSoften( bool soften )
	{
		m_flags = soften==false? ( m_flags & (~TPCAMERA_DISTANCE_SOFT_MAX_ERROR)):(m_flags | TPCAMERA_DISTANCE_SOFT_MAX_ERROR);
	}

	void		
	C_ThirdPersonCamera::SetIdealYawRange( F32 distMin, F32 distMax )
	{
		m_yaw.min = distMin;
		m_yaw.max = distMax;
	}

	void		
	C_ThirdPersonCamera::SetIdealYaw( F32 dist )
	{
		m_yaw.min = dist;
		m_yaw.max = dist;
	}

	void		
	C_ThirdPersonCamera::SetIdealYawDamping( F32 damping )
	{
		m_yaw.smoothing = damping;
	}

	void		
	C_ThirdPersonCamera::SetIdealYawMaxSpeed( F32 speed )
	{
		m_yaw.speed = speed;
	}

	void		
	C_ThirdPersonCamera::SetIdealYawMaxError( F32 error)
	{
		m_yaw.maxError = error;
	}

	void		
	C_ThirdPersonCamera::SetIdealYawMaxErrorSoften( bool soften )
	{
		m_flags = soften == false? ( m_flags & (~TPCAMERA_YAW_SOFT_MAX_ERROR)):(m_flags | TPCAMERA_YAW_SOFT_MAX_ERROR);
	}

	void		
	C_ThirdPersonCamera::SetIdealPitchRange( F32 distMin, F32 distMax )
	{
		m_pitch.min = distMin;
		m_pitch.max = distMax;
	}

	void		
	C_ThirdPersonCamera::SetIdealPitch( F32 dist )
	{
		m_pitch.min = dist;
		m_pitch.max = dist;
	}

	void		
	C_ThirdPersonCamera::SetIdealPitchDamping( F32 damping )
	{
		m_pitch.smoothing = damping;
	}

	void		
	C_ThirdPersonCamera::SetIdealPitchMaxSpeed( F32 speed )
	{
		m_pitch.speed = speed;
	}

	void		
	C_ThirdPersonCamera::SetIdealPitchMaxError( F32 error )
	{
		m_pitch.maxError = error;
	}

	void		
	C_ThirdPersonCamera::SetIdealPitchMaxErrorSoften( bool soften )
	{
		m_flags = soften == false? ( m_flags & (~TPCAMERA_PITCH_SOFT_MAX_ERROR)):(m_flags | TPCAMERA_PITCH_SOFT_MAX_ERROR);
	}


	void		
	C_ThirdPersonCamera::SetIdealRollRange( F32 rollMin, F32 rollMax )
	{
		m_roll.min = rollMin;
		m_roll.max = rollMax;
	}

	void		
	C_ThirdPersonCamera::SetIdealRoll( F32 roll )
	{
		m_roll.min = roll;
		m_roll.max = roll;
	}

	void		
	C_ThirdPersonCamera::SetIdealRollDamping( F32 damping )
	{
		m_roll.smoothing = damping;
	}

	void		
	C_ThirdPersonCamera::SetIdealRollMaxSpeed( F32 speed )
	{
		m_roll.speed = speed;
	}

	void		
	C_ThirdPersonCamera::SetIdealRollMaxError( F32 error )
	{
		m_roll.maxError = error;
	}

	void		
	C_ThirdPersonCamera::SetIdealRollMaxErrorSoften( bool soften )
	{
		m_flags = soften == false? ( m_flags & (~TPCAMERA_ROLL_SOFT_MAX_ERROR)):(m_flags | TPCAMERA_ROLL_SOFT_MAX_ERROR);
	}

	void		
	C_ThirdPersonCamera::SetIdeals( F32 pitch, F32 yaw, F32 roll, F32 distance, F32 tollerence )
	{
		m_pitch.min = pitch - tollerence;
		m_pitch.max = pitch + tollerence;

		m_yaw.min = yaw - tollerence;
		m_yaw.max = yaw + tollerence;

		m_distance.min = distance - tollerence;
		m_distance.max = distance + tollerence;

		m_roll.min = roll - tollerence;
		m_roll.max = roll + tollerence;
	}

	void		
	C_ThirdPersonCamera::SetUpAxis(const Maths::HVec3& up)
	{
		m_flags |= TPCAMERA_OVERRIDE_UP_AXIS;
		m_desiredUp = up;
	}
	
	void		
	C_ThirdPersonCamera::SetUpAxisAlwaysFromTransform()
	{
		m_flags &= ~TPCAMERA_OVERRIDE_UP_AXIS;
	}

	void
	C_ThirdPersonCamera::ResetParams()
	{
		m_pitch.velocity = 0.0f;
		m_yaw.velocity = 0.0f;
		m_roll.velocity = 0.0f;
		m_distance.velocity = 0.0f;
	}

	F32			
	C_ThirdPersonCamera::GetRoll()
	{
		return m_rollCurrent;
	}

	void		
	C_ThirdPersonCamera::GetTargetTransform(Maths::HMat43& target)
	{
		target = m_target;
	}

	void		
	C_ThirdPersonCamera::GetTargetPos(Maths::HVec3& pos)
	{
		pos = m_targetPos;
	}

	void		
	C_ThirdPersonCamera::GetTargetAimPos(Maths::HVec3& pos)
	{
		pos = m_targetAimPos;
	}

	
	void		
	C_ThirdPersonCamera::GetRollData(SIdealData& rollData)
	{
		rollData = m_roll;
	}

	void		
	C_ThirdPersonCamera::GetPitchData(SIdealData& pitchData)
	{
		pitchData = m_pitch;
	}

	void		
	C_ThirdPersonCamera::GetYawData(SIdealData& yawData)
	{
		yawData = m_yaw;
	}

	void		
	C_ThirdPersonCamera::GetDistanceData(SIdealData& distanceData)
	{
		distanceData = m_distance;
	}

	//-----------------------------------------------------------------------------
	//  DebugRender
	//-----------------------------------------------------------------------------
	void 
	C_ThirdPersonCamera::DebugRender(RGBA colour, bool renderFrustum) const
	{
		assert(this);
#if !CFG_CONVERTER && !defined(CFG_GOLD)
		
		HVec3 LookPosWS = m_targetAimPos * m_target;
		HVec3 TargetPosWS = m_targetPos * m_target;

		DbUtil::I_DebugRender* debugRender = Cam2::GetDebugRender();
		debugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);

		Maths::HMat43 targetPos = m_target;
		Maths::HMat43 targetAimPos = m_target;
		targetPos.AW(TargetPosWS);
		targetAimPos.AW(LookPosWS);
		debugRender->Draw3dMatrix(targetPos,0.3f);
		debugRender->Draw3dMatrix(targetAimPos,0.3f);
		debugRender->Draw3dLine(LookPosWS,Position(),Maths::RGBA(255,0,0,255));
		debugRender->Draw3dLine(TargetPosWS,Position(),Maths::RGBA(255,0,0,255));
		debugRender->Draw3dCircle(TargetPosWS,m_target.AX(),CalculateCurrentDistance(),Maths::RGBA(0,0,255,180));

#endif
		Cam2::C_Camera::DebugRender(colour,renderFrustum);
	}

	//-----------------------------------------------------------------------------
	//  Update
	//-----------------------------------------------------------------------------
	void		
	C_ThirdPersonCamera::OnUpdate()
	{
		F32 timeSlice = Period();
		static F32 s_delayedDisableDampingCoeef = 0.99f;
		static F32 s_delayedDisableCutoff = 0.05f;
		// Always keep the target space position of the camera up to date.
		HMat43 invTarget;
		invTarget = m_target.InverseRotTrans();
		m_targetSpaceCameraPosition =  Position() * invTarget;
		
		F32 distance = CalculateCurrentDistance(m_targetSpaceCameraPosition);
		F32 yaw = CalculateCurrentYaw(m_targetSpaceCameraPosition);
		F32 pitch = CalculateCurrentPitch(m_targetSpaceCameraPosition);
		F32 roll = CalculateCurrentRoll();

		if( m_flags & TPCAMERA_DISTANCE_TRACKING_ENABLED )
		{
			F32 error = CalculateCurrentDistanceError();		
			

			F32 maxSpeed = m_distance.speed > 0.0f ? m_distance.speed * timeSlice : FLT_MAX;
			m_distance.velocity = Maths::Clamped(m_distance.velocity,-maxSpeed,maxSpeed);
			
			if(m_flags&TPCAMERA_DISTANCE_TRACKING_DELAYED_DISABLE)
			{
				m_distance.velocity += -m_distance.velocity * DampingAdjust(s_delayedDisableDampingCoeef,timeSlice);
				distance += m_distance.velocity * timeSlice;
				if(fabs(m_distance.velocity)<s_delayedDisableCutoff)
				{
					SetDistanceTrackingEnabled(false,true);
				}
			}
			else
			{
			
				if( m_distance.maxError >= 0.0f )
				{
					if(m_distance.maxError < fabs(error))
					{
						if((m_flags&TPCAMERA_DISTANCE_SOFT_MAX_ERROR)==0)
						{
							distance -= ( fabs(error) - m_distance.maxError ) * (error < 0.0f ? -1.0f:+1.0f);
						}
						else
						{
							if( m_distance.velocity * error > 0.0f )
							{
								m_distance.velocity = 0.0f;
							}
						}
					}
					else
					{
						// We are within our error limit now so turn off soft lock
						m_flags &= ~TPCAMERA_DISTANCE_SOFT_MAX_ERROR;
					}
				}
				else
				{
					m_flags &= ~TPCAMERA_DISTANCE_SOFT_MAX_ERROR;
				}
				
				distance = SmoothingFunction( distance, distance - error, m_distance.velocity, m_distance.smoothing, timeSlice );
			}
		}

		if( m_flags & TPCAMERA_YAW_TRACKING_ENABLED )
		{
			F32 error = CalculateCurrentYawError();
		


			F32 maxSpeed = m_yaw.speed > 0.0f ? m_yaw.speed * timeSlice : FLT_MAX;
			m_yaw.velocity = Maths::Clamped(m_yaw.velocity,-maxSpeed,maxSpeed);
			
			if(m_flags&TPCAMERA_YAW_TRACKING_DELAYED_DISABLE)
			{
				m_yaw.velocity += -m_yaw.velocity * DampingAdjust(s_delayedDisableDampingCoeef,timeSlice);
				yaw += m_yaw.velocity * timeSlice;
				if(fabs(m_yaw.velocity)<s_delayedDisableCutoff)
				{
					SetYawTrackingEnabled(false,true);
				}
			}
			else
			{
				if( m_yaw.maxError >= 0.0f )
				{
					if(m_yaw.maxError < fabs(error))
					{
						if((m_flags&TPCAMERA_YAW_SOFT_MAX_ERROR)==0)
						{
							yaw -= ( fabs(error) - m_yaw.maxError ) * (error < 0.0f ? -1.0f:+1.0f);
						}
						else
						{
							if( m_yaw.velocity * error > 0.0f )
							{
								m_yaw.velocity = 0.0f;
							}
						}
					}
					else
					{
						// We are within our error limit now so turn off soft lock
						m_flags &= ~TPCAMERA_YAW_SOFT_MAX_ERROR;
					}
				}
				else
				{
					m_flags &= ~TPCAMERA_YAW_SOFT_MAX_ERROR;
				}

				yaw = SmoothingFunction( yaw, yaw - error, m_yaw.velocity, m_yaw.smoothing, timeSlice );
			}
	
			// Handle yaw wrapping properly 
			if( fabsf(yaw) > Pi )
			{
				yaw += yaw < 0.0f ? Pi2 : -Pi2; 
			}
			BIZ_ASSERT( fabsf(yaw) <= Pi );
		}

		if( m_flags & TPCAMERA_PITCH_TRACKING_ENABLED )
		{
			
			F32 error = CalculateCurrentPitchError();
			
			F32 maxSpeed = m_pitch.speed > 0.0f ? m_pitch.speed * timeSlice : FLT_MAX;
			m_pitch.velocity = Maths::Clamped(m_pitch.velocity,-maxSpeed,maxSpeed);
			
			if(m_flags&TPCAMERA_PITCH_TRACKING_DELAYED_DISABLE)
			{
				m_pitch.velocity += -m_pitch.velocity * DampingAdjust(s_delayedDisableDampingCoeef,timeSlice);
				pitch += m_pitch.velocity * timeSlice;
				if(fabs(m_pitch.velocity)<s_delayedDisableCutoff)
				{
					SetPitchTrackingEnabled(false,true);
				}
			}
			else
			{
				if( m_pitch.maxError >= 0.0f )
				{
					if(m_pitch.maxError < fabs(error))
					{
						if((m_flags&TPCAMERA_PITCH_SOFT_MAX_ERROR)==0)
						{
							pitch -= ( fabs(error) - m_pitch.maxError ) * (error < 0.0f ? -1.0f:+1.0f);
						}
						else
						{
							if( m_pitch.velocity * error > 0.0f )
							{
								m_pitch.velocity = 0.0f;
							}
						}
					}
					else
					{
						// We are within our error limit now so turn off soft lock
						m_flags &= ~TPCAMERA_PITCH_SOFT_MAX_ERROR;
					}
				}
				else
				{
					m_flags &= ~TPCAMERA_PITCH_SOFT_MAX_ERROR;
				}

				pitch = SmoothingFunction( pitch, pitch - error, m_pitch.velocity, m_pitch.smoothing, timeSlice );
			}
		}

		if( m_flags & TPCAMERA_ROLL_TRACKING_ENABLED )
		{
			F32 error = CalculateCurrentRollError();
			
			F32 maxSpeed = m_roll.speed > 0.0f ? m_roll.speed * timeSlice : FLT_MAX;
			m_roll.velocity = Maths::Clamped(m_roll.velocity,-maxSpeed,maxSpeed);
			
			if(m_flags&TPCAMERA_ROLL_TRACKING_DELAYED_DISABLE)
			{
				m_roll.velocity += -m_roll.velocity * DampingAdjust(s_delayedDisableDampingCoeef,timeSlice);
				roll += m_roll.velocity * timeSlice;
				if(fabs(m_roll.velocity)<s_delayedDisableCutoff)
				{
					SetRollTrackingEnabled(false,true);
				}
			}
			else
			{
				if( m_roll.maxError >= 0.0f )
				{
					if(m_roll.maxError < fabs(error))
					{
						if((m_flags&TPCAMERA_ROLL_SOFT_MAX_ERROR)==0)
						{
							roll -= ( fabs(error) - m_roll.maxError ) * (error < 0.0f ? -1.0f:+1.0f);
						}
						else
						{
							if( m_roll.velocity * error > 0.0f )
							{
								m_roll.velocity = 0.0f;
							}
						}
					}
					else
					{
						// We are within our error limit now so turn off soft lock
						m_flags &= ~TPCAMERA_ROLL_SOFT_MAX_ERROR;
					}
				}
				else
				{
					m_flags &= ~TPCAMERA_ROLL_SOFT_MAX_ERROR;
				}

				roll = SmoothingFunction( roll, roll - error, m_roll.velocity, m_roll.smoothing, timeSlice );
			}
		}
		
		m_rollCurrent = roll;
#ifndef CFG_GOLD		
		if(OverrideDistance)
		{
			distance = OverrideDistanceValue.Value();
		}
		if(OverridePitch)
		{
			pitch = Maths::DegToRad(OverridePitchValue.Value());
		}
		if(OverrideYaw)
		{
			yaw = Maths::DegToRad(OverrideYawValue.Value());
		}
		if(OverrideRoll)
		{
			m_rollCurrent = Maths::DegToRad(OverrideRollValue.Value());
		}
#endif
		
		SetPosition(CalculateNewPosition(distance, yaw, pitch));
		CalculateLook();
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentPitch
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentPitch() const
	{
		HVec3 targetSpaceCameraPosition = Position() * m_target.InverseRotTrans();
		return CalculateCurrentPitch( targetSpaceCameraPosition );
	}

	F32		
	C_ThirdPersonCamera::CalculateCurrentPitch( const HVec3& targetSpaceCameraPosition ) const
	{
	   
		if( targetSpaceCameraPosition.DistanceSquared(m_targetPos) > 0.0001f )
		{
			HVec3 camToTarget;
			HVec3 camToTargetFlat;
			camToTarget = m_targetPos - targetSpaceCameraPosition;
			camToTargetFlat = camToTarget;
			camToTargetFlat.Y(0.0f);
			camToTarget.Normalise();
			camToTargetFlat.Normalise();
			F32 dot = Clamped<F32>( DotProduct(camToTargetFlat, camToTarget ), -1.0f,1.0f);
			return camToTarget.Y() > 0.0f ? -acosf(dot): acosf(dot);
		}
		return 0.0f;
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentYaw
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentYaw() const
	{
		HVec3 targetSpaceCameraPosition = Position() * m_target.InverseRotTrans();
		return CalculateCurrentYaw( targetSpaceCameraPosition );
	}

	F32		
	C_ThirdPersonCamera::CalculateCurrentYaw( const HVec3& targetSpaceCameraPosition ) const
	{
		HVec3 normalised;
		if( targetSpaceCameraPosition.DistanceSquared(m_targetPos) > 0.0001f )
		{
			normalised = m_targetPos - targetSpaceCameraPosition;
			normalised.Y(0.0f);
			if( normalised.LengthSquared() > 0.0001f )
			{
				normalised.Normalise();
				normalised.Z( Clamped<F32>(normalised.Z(),-1.0f,1.0f));
				return normalised.X() > 0.0f ? acosf(normalised.Z()):-acosf(normalised.Z());
			}
		}
		return 0.0f;
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentRoll
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentRoll() const
	{
		Maths::HMat33 cameraMtx(Orient());
		Maths::HVec3 zeroRollUp = CalculateUpVector(cameraMtx.AZ());

		HVec3 side = zeroRollUp ^ cameraMtx.AZ();
		//BIZ_ASSERT(side.LengthSquared() > 0.01f);
		side.Normalise();
		HVec3 up = cameraMtx.AZ() ^ side; 
		up.Normalise();

		F32 dot = Maths::DotProduct( up, cameraMtx.AY() );
		F32 angle = acosf( Maths::Clamped<F32>( dot, -1.0f, 1.0f ));
		return Maths::DotProduct( side, cameraMtx.AY() ) < 0.0f? angle:-angle;

	}

	F32		
	C_ThirdPersonCamera::CalculateCurrentRoll(const Maths::HMat33& targetSpaceCameraMtx) const
	{
		BIZ_ASSERT(0);
		if( fabs( targetSpaceCameraMtx.AZ().Y()) < 0.99f )
		{
			Maths::HVec3 side = targetSpaceCameraMtx.AZ() ^ Constants<Maths::HVec3>::UnitAxisPositiveY();
			side.Normalise();
			
			Maths::HVec3 up = side ^ targetSpaceCameraMtx.AZ();
			up.Normalise();
			
			F32 dot = Maths::DotProduct( up, targetSpaceCameraMtx.AY() );
			F32 angle = acosf( Maths::Clamped<F32>( dot, -1.0f, 1.0f ));
			return Maths::DotProduct( side, targetSpaceCameraMtx.AY() ) < 0.0f? -angle:angle;
				
		}
		// The roll axis and the local up vector
		// are parallel so there is no roll as such. 
		return FLT_MAX;
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentDistance
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentDistance() const
	{
		HVec3 targetSpaceCameraPosition = Position() * m_target.InverseRotTrans();
		return CalculateCurrentDistance( targetSpaceCameraPosition );
	}

	F32		
	C_ThirdPersonCamera::CalculateCurrentDistance( const HVec3& targetSpaceCameraPosition ) const
	{
		return targetSpaceCameraPosition.Distance( m_targetPos );
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentPitchError
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentPitchError() const
	{
		F32 current = CalculateCurrentPitch();
		F32 error = 0.0f;
		if( current < m_pitch.min )
		{
			error = current - m_pitch.min;
		}
		else if( current > m_pitch.max )
		{
			error = current - m_pitch.max;
		}
		return error;
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentYawError
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentYawError() const
	{
		F32 current = CalculateCurrentYaw();
		F32 error = 0.0f;

		if(m_yaw.min<=m_yaw.max )
		{
			if( current < m_yaw.min )
			{
				error = current - m_yaw.min;
			}
			else if( current > m_yaw.max )
			{
				error = current - m_yaw.max;
			}
		}
		else
		{
			if( current > m_yaw.max &&
				current < m_yaw.min)
			{
				F32 errorMax = (current - m_yaw.max);
				F32 errorMin = (current - m_yaw.min);
				error = fabs(errorMax)>fabs(errorMin) ? errorMin:errorMax;
			}
		}
		// Handle yaw wrapping properly 
		if( fabsf(error) > Pi )
		{
			error += error < 0.0f ? Pi2 : -Pi2; 
		}
		
		return error;
	}

	//-----------------------------------------------------------------------------
	//  CalculateCurrentRollError
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentRollError() const
	{
		F32 current = CalculateCurrentRoll();
		F32 error = 0.0f;
		if( current < FLT_MAX )
		{
			if( current < m_roll.min )
			{
				error = current - m_roll.min;
			}
			else if( current > m_roll.max )
			{
				error = current - m_roll.max;
			}
			// Handle roll wrapping properly 
			if( fabsf(error) > Pi )
			{
				error += error < 0.0f ? Pi2 : -Pi2; 
			}
		}
		return error;
	}
	//-----------------------------------------------------------------------------
	//  CalculateCurrentDistanceError
	//-----------------------------------------------------------------------------
	F32		
	C_ThirdPersonCamera::CalculateCurrentDistanceError() const
	{
		F32 current = CalculateCurrentDistance();
		F32 error = 0.0f;
		if( current < m_distance.min)
		{
			error = current - m_distance.min;
		}
		else if( current > m_distance.max )
		{
			error = current - m_distance.max;
		}
		return error;
	}

	//-----------------------------------------------------------------------------
	//  CalculateNewPosition
	//-----------------------------------------------------------------------------
	HVec3		
	C_ThirdPersonCamera::CalculateNewPosition( F32 distance, F32 yaw, F32 pitch ) const
	{
		HVec3 position;
		HVec3 toCam( 0.0f, 0.0f, -distance );
		
		HMat33 pitchYaw( C_MatrixRotatePHR( pitch, yaw, 0.0f ));
		toCam = toCam * pitchYaw;
		
		position = m_targetPos + toCam;
		position = position * m_target;

		return position;   
	}

	//-----------------------------------------------------------------------------
	// Get the look quaternion for looking in a direction with a user specified up vector
	//-----------------------------------------------------------------------------
	C_Quaternion 
	C_ThirdPersonCamera::CalculateLookQuaternion(const HVec3& direction, HVec3 up, F32 Roll ) const
	{
			HVec3 toz = direction;
			toz.Normalise();
			HVec3 tox = up ^ toz;
			tox.Normalise();
			HVec3 toy = toz ^ tox;
			toy.Normalise();

			HMat33 lookMtx;
			lookMtx.AX(tox);
			lookMtx.AY(toy);
			lookMtx.AZ(toz);
			
			RotZ rollrot(Roll);
			HMat33 rollMtx( rollrot );
			HMat33 result = rollMtx*lookMtx;
		
			return C_Quaternion(result);
	}
	//-----------------------------------------------------------------------------
	//  SIdealData::Reset
	//-----------------------------------------------------------------------------
	void
	C_ThirdPersonCamera::SIdealData::Reset()
	{
		min = 0.0f;
		max = 0.0f;
		smoothing = 0.4f;
		speed = -1.0f;
		velocity = 0.0f;
		maxError = -1.0f;
	}

}
