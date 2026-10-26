#ifndef _DATA_AI_I_H
#define _DATA_AI_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif


#include <ComponentSystem/componentlib.h>


namespace airacingtech
{
	// forward decs
	class IVehicleState;

	class IPIDControllerAI;
	class IControlState;
	class ISplineFollowingAI;
	class IAccelBrakeController;
	//class ISliding;
	class IAvoidance;
	//class IBios;
	//class ITest;
	class IRecoveryAI;
	class IParamsAI;
	//class INavigationTrackerAI;
	class ISplineTracker;
	class ISplineFollower;
	class ISpeedTracker;
	class IVehicleLogic;
	class IDebugAI;
	class IRubberbandingAI;
	class IIntroOutroSplineAI;
	class IGameBrain;
	

	class CollisionInfo
	{
	public:
		enum COLLISION_TYPE
		{
			COLLISION_TYPE_VEHICLE_TO_VEHICLE = 0,
			COLLISION_TYPE_VEHICLE_TO_ENVIRONMENT,
			COLLISION_TYPE_VEHICLE_TO_OBJECTS,
			COLLISION_TYPE_NONE
		};


		CollisionInfo() : m_collisionType(COLLISION_TYPE_NONE) , m_TimeStamp(-1.0f), m_VehicleIndexA(-1), m_VehicleIndexB(-1) {};
		~CollisionInfo(){};

		
		COLLISION_TYPE	m_collisionType;
		float			m_TimeStamp;
		Maths::HVec3	m_localContactPointA;
		Maths::HVec3	m_localContactPointB;
		int				m_VehicleIndexA;
		int				m_VehicleIndexB;

	};


	class RecordedCollisions
	{
	public:

		RecordedCollisions(){};
		~RecordedCollisions(){};

		inline CollisionInfo* GetCollision(CollisionInfo::COLLISION_TYPE type)
		{
			CollisionInfo *pCollision = 0;
			switch(type)
			{
				case CollisionInfo::COLLISION_TYPE_VEHICLE_TO_VEHICLE:
					{
						pCollision = &m_vehicleCollision;
					}
					break;

				case CollisionInfo::COLLISION_TYPE_VEHICLE_TO_ENVIRONMENT:
					{
						pCollision = &m_environmentCollision;
					}
					break;

				case CollisionInfo::COLLISION_TYPE_VEHICLE_TO_OBJECTS:
					{
						pCollision = &m_ObjectCollision;
					}
					break;

				default:
					break;
			}

			return pCollision;
		}

		CollisionInfo	m_environmentCollision;
		CollisionInfo	m_vehicleCollision;
		CollisionInfo	m_ObjectCollision;

	
	};
	


	class DataState
	{
	public:

		enum SteerState
		{
			NEUTRAL = 0,
			UNDER_STEER_SAME,
			OVER_STEER_SAME,
			OVER_STEER_FRONT,
			OVER_STEER_REAR
		};

		enum GameState
		{
			INTRO,
			GAME,
			OUTRO
		};

		// override for the perk catching
		struct OverridePerks
		{
			bool			m_setOverride;
			float			m_offset;
			float			m_dist;
			float			m_secs;
		};


		DataState()
		{
			m_trackSpeed = 0.0f;
			m_overrideSpeed = 0.0f;
			//m_pSliding = 0;
			m_pIVehicleState = 0;
			m_pIPIDController = 0;
			m_pIControlState = 0;
			//m_pISplineFollowingAI = 0;
			m_pIAccelBrakeController = 0;
			m_pIParams = 0;
			m_pISplineTracker = 0;
			m_pISplineFollower = 0;
			m_pISpeedTracker = 0;
			m_pVehicleLogic = 0;
			m_pIDebugAI = 0;
			m_pIGameBrainAI = 0;

			m_pIAvoidance = 0;
			//m_pIBios = 0;
			//m_pITests = 0;
			m_pIRecovery = 0;
			//m_pINavTracker = 0;	
			m_SteerState = NEUTRAL;
			m_rearWheelSlip = 0.0f;
			m_frontWheelSlip = 0.0f;
			m_DiffTrackSpeed = 0.0f;
			m_slideHandBrake = false;
			m_TryAndSlide = false;
			m_rearWheelSlideFactorLast = 0.0f;
			m_InTheTryAndSlideZone = false;
			m_pressureSpeedDist = 0.0f;
			
			m_index = -1;
			m_alignedAngleDirection = 0.0f;
			m_distToSpline = 0.0f;

			m_futureLeftDist = 0.0f;
			m_futureRightDist = 0.0f;

			m_LeftDist = 0.0f;
			m_RightDist = 0.0f;
			m_DangerSlidingToRightEdge = 0.0f;
			m_DangerSlidingToLeftEdge = 0.0f;
			m_SlideSteeringAngle = 0.0f;
			m_SteeringAngle = 0.0f;
			m_speedCoeff = 1.0f;
			m_brakingCoeff = 1.0f;
			m_endRace = false;
			m_startRace = false;
			m_spinOut = false;
			//m_rubberBandingBoost = 0.0f;
			m_trackSpeedPreprocessed = 0.0f;

			m_pIRubberBanding = 0;
			m_pIntroOutroSplineAI = 0;
			m_gameState = GAME;
			m_startIntro = false;
			m_startOutro = false;



			// bad
			m_usingNitro = false;
			m_hasNitro = false;
			m_fireNitro = false;

			m_overidePerkSteering.m_setOverride = false;
			m_overidePerkSteering.m_offset = 9999.9f;
			m_overidePerkSteering.m_dist = 9999.9f;
			m_overidePerkSteering.m_secs = -1.0f;
			m_DoNotUseInPositionCalculation = false;
			m_usingCoolDownNitroExtended = false;

			m_aiInControl = true;
			m_localPlayer = false;
		}
		~DataState(){}

		
		IIntroOutroSplineAI		*m_pIntroOutroSplineAI;
		IRubberbandingAI		*m_pIRubberBanding; 
		IVehicleState			*m_pIVehicleState;

		IPIDControllerAI		*m_pIPIDController;
		IControlState			*m_pIControlState;
		//ISplineFollowingAI		*m_pISplineFollowingAI;
		IAccelBrakeController	*m_pIAccelBrakeController;
		ISplineFollower			*m_pISplineFollower;
		ISpeedTracker			*m_pISpeedTracker;

		//ISliding			*m_pSliding;

		IAvoidance			*m_pIAvoidance;

		//IBios				*m_pIBios;
		//ITest				*m_pITests;
		IRecoveryAI			*m_pIRecovery;
		IParamsAI			*m_pIParams;

		//INavigationTrackerAI *m_pINavTracker;
		ISplineTracker		*m_pISplineTracker;
		IVehicleLogic		*m_pVehicleLogic;

		IDebugAI			*m_pIDebugAI;
		IGameBrain			*m_pIGameBrainAI;
		

		float	m_trackSpeedPreprocessed;
		float	m_trackSpeed;
		float	m_overrideSpeed;

		bool	m_slideHandBrake;
		SteerState m_SteerState;	

		float	m_currentCurvature;
		float	m_rearWheelSlip;
		float	m_frontWheelSlip;
		float	m_DiffTrackSpeed;

		float m_slip;
		float m_slipN;
		//float m_speedmps; 
		//float m_speedmph; 
		float m_SteerAngle;
		//float m_speedfrds;
		float m_rearWheelSlideFactor;
		float m_rearWheelSlideFactorLast;
		float m_frontWheelSlideFactor;
		float m_angVelY; 
		float m_angVelYLast;
		float m_diffAngVel;
		float m_pressureSpeedDist;
		Maths::HVec3 m_angVelV ;
		Maths::HVec3 m_angVelVLast;
		bool	m_TryAndSlide;
		Maths::HVec3 m_width;		// width a look ahead distance ahead.....
		Maths::HVec3 m_widthCurrent; // the width at the current sample point
		float		m_alignedAngleDirection;

		bool		m_InTheTryAndSlideZone;

		int			m_RacePos;
		
		int			m_index;

		//CollisionInfo	m_CollisionInfo;
		//CollisionInfo	m_CollisionInfoPrev;


		float	m_distToSpline;
		float	m_futureLeftDist;
		float	m_futureRightDist;
		float	m_LeftDist;
		float	m_RightDist;
		float	m_DangerSlidingToRightEdge;
		float	m_DangerSlidingToLeftEdge;
		float	m_SlideSteeringAngle;
		float	m_SteeringAngle;

		float	m_speedCoeff;
		float	m_brakingCoeff;

		int		m_widthLookahead;
		float   m_headingXZ;
		float	m_headingTrackXZ;
		bool	m_endRace;
		bool	m_startRace;


		bool	m_spinOut;
		bool	m_startIntro;
		bool	m_startOutro;
		bool	m_endIntro; // is this needed
		bool	m_endOutro; // is this needed

		float	m_rubberBandingBoost;

		int		m_gameState;

		bool	m_usingNitro;
		bool	m_usingCoolDownNitroExtended;
		bool	m_hasNitro;
		bool	m_fireNitro;
		bool	m_DoNotUseInPositionCalculation;
		bool	m_aiInControl;
		bool	m_localPlayer;

		RecordedCollisions	m_Collisions;

		

		OverridePerks			m_overidePerkSteering;
		
		

	};


	BEGIN_INTERFACE_DEC(IData)
	
		virtual IVehicleState *GetVehicleState() const = 0;
		virtual IVehicleLogic *GetVehicleLogic() const = 0;
		virtual IPIDControllerAI  *GetPIDController() const  = 0;
		virtual IControlState *GetControlState() const  = 0;
		//virtual ISplineFollowingAI *GetSplineFollowing() const  = 0;
		virtual IAccelBrakeController *GetAccelBrakeController() const = 0;
		//virtual ISliding *GetSliding() const = 0;
		virtual DataState &GetState() = 0;
		virtual IAvoidance *GetAvoidance() const = 0;
		//virtual ITest *GetTests() const = 0;
		virtual IRecoveryAI *GetRecovery() const  = 0;
		virtual IParamsAI *GetParams() const = 0;

		//virtual INavigationTrackerAI *GetNAvigationTracker() const = 0;
		virtual ISplineTracker *GetSplineTracker() const = 0;
		virtual ISplineFollower *GetSplineFollower() const = 0;
		virtual ISpeedTracker *GetSpeedTracker() const = 0;
		virtual IDebugAI *GetDebugAI() const = 0;
		virtual IRubberbandingAI *GetRubberBanding() const = 0;
		virtual IIntroOutroSplineAI *GetIntroOutroSplineAI() const = 0;
		virtual IGameBrain *GetGameBrainAI() const = 0;

		virtual bool IsInIntro() const = 0;
		virtual bool IsInGame() const = 0;
		virtual bool IsInOutro() const = 0;

		virtual const bool IsPlayer() const = 0;

		virtual void SetOverrideSteerPerk(bool Override, float offset, float dist, float secs) = 0;

	
	END_INTERFACE_DEC
}

#endif //_DATA_AI_I_H
