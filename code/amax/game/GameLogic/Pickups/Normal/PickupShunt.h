#ifndef PICKUP_SHUNT_H
#define PICKUP_SHUNT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupProjectileBase.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>
#include <utilityai/utilityai.h>
#include <AI/AI_Amax.h>


class airacingtech::AITrackDataRoutes;

namespace Game
{


	// This is based upon a simple 2d car, written many years ago.....
	class AbstractProjectile
	{
		public:
			AbstractProjectile(){};
			~AbstractProjectile(){};

			void Reset()
			{
				Init();
			}

			void Init()
			{
				m_positionWorld = Maths::HVec3(0.0f,0.0f,0.0f);
				m_velocityWorld = Maths::HVec3(0.0f,0.0f,0.0f);
				m_maxSpeed = 150.0f;
				m_useGroundHeight = false;
				m_minSpeed = 40.0f;
				m_targetSpeed = 0.0f;
				m_MaximumTurnRate = Maths::DegToRad(45.0f);
				m_Acceleration = 0.25f; // 1.0f
				m_CurrentLogicIndex = -1;
				m_Deceleration = 3.0f;
			}


			inline Maths::HVec3 ClipVectorToLength (const float maxLength, const Maths::HVec3 v) const
			{
				const float maxLengthSquared = maxLength * maxLength;
				const float vecLengthSquared = v.LengthSquared();   
				if (vecLengthSquared <= maxLengthSquared)
					return v;
				else
					return  v * (maxLength / Maths::Sqrt(vecLengthSquared));
			}

			inline void SetTargetSpeed(float tsp)
			{ 
				// this is set outside the main update loop at 1.0f/30.0fhz
				const float timeStep = 1.0f / 30.0f;

				float currentSpeed = m_velocityWorld.Length();
				m_targetSpeed = Maths::Max(tsp, m_minSpeed);
				
				if (m_targetSpeed > currentSpeed)
				{
					currentSpeed += m_Acceleration;						
					m_targetSpeed = Maths::Min(currentSpeed, m_targetSpeed);
				}
				else if (m_targetSpeed < currentSpeed)
				{
					currentSpeed -= m_Deceleration;						
					m_targetSpeed = Maths::Max(currentSpeed, m_targetSpeed);
				}


				//clamp the speed...
				m_targetSpeed = Maths::Clamped(m_targetSpeed, m_minSpeed, m_maxSpeed);
			}

			inline void SetMaxTurnRate(float m){m_MaximumTurnRate = m;}

			inline void Simulate(float timeStep)
			{
				RotateToFacePosition(timeStep);
				m_velocityWorld *= m_targetSpeed;
				
				m_velocityWorld = ClipVectorToLength(m_targetSpeed, m_velocityWorld);
				m_velocityWorld = ClipVectorToLength(m_maxSpeed, m_velocityWorld);
				m_positionWorld += (m_velocityWorld * timeStep);
			}
			
			inline void RotateToFacePosition(float timeStep)
			{
				Maths::HVec3 toTarget = m_targetPos - m_positionWorld;
				toTarget.Normalise();

				Maths::HVec3 head = m_velocityWorld;
				head.Normalise();

				float angle = airacingtech::AngleBetween(head, toTarget);
				float maxAngle = Maths::DegToRad(m_MaximumTurnRate) * timeStep;

				if (angle > 0.00001) 
				{
					if (angle > maxAngle) 
						angle = maxAngle;

					int side = -airacingtech::Vector2DSign(head, toTarget);
					m_velocityWorld = airacingtech::RotateXZ((float)(angle * side), head);
				}
			}

			inline void Update(float frameTime)
			{
				const float timeStep = 1.0f / 60.0f;//  1.0f / 60.0f;//30.0f;   //60Hz  // 100Hz
				float accumStep = timeStep;
				
				while(accumStep < frameTime)
				{
					Simulate(timeStep);
					accumStep += timeStep;
				} 

				// Simulate remaining time
				accumStep -= timeStep;
				float remainingTime = frameTime - accumStep;
				if (remainingTime > 0.0f)
				{
					Simulate(remainingTime);
				}
			}

			inline void SetPosition(Maths::HVec3 position) {m_positionWorld = position;}
			inline Maths::HVec3 GetPosition() {return m_positionWorld;}

			inline Maths::HVec3 GetVelocity(){return m_velocityWorld;}
			inline void SetVelocity(Maths::HVec3 v){m_velocityWorld = v;}

			inline void SetMaxSpeed(float sp){m_maxSpeed = sp;}
			
			Maths::HVec3	m_positionWorld;		
			Maths::HVec3	m_velocityWorld;		
			Maths::HVec3	m_targetPos;

			bool		m_useGroundHeight;
			float		m_minSpeed;
			float		m_targetSpeed;
			float		m_maxSpeed;		
			float		m_MaximumTurnRate;
			float		m_Acceleration;
			int			m_CurrentLogicIndex;
			float		m_Deceleration;

	};

	class ShuntRouteInfo
	{
	public:
 
		ShuntRouteInfo() : m_Index(-1), m_pRoute(0) {};
		~ShuntRouteInfo(){};
		
		int								m_StartIndex;	// The start index of the spline when we fired.....
		int								m_Index;
		float							m_IndexPos;
		airacingtech::AITrackDataRoutes	*m_pRoute;

		void Reset()
		{
			m_Index = -1;
			m_pRoute = 0;
			m_IndexPos = -1.0f;
		}

	};


	class C_PickupShuntProjectile : public C_PickupProjectile
	{
	public:
		C_PickupShuntProjectile(U32 id, int firingIdx, int targetIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool			Update();

		virtual void			Explode(Maths::HVec3 pos, Maths::C_Normal normal);
		virtual void			Trigger(int idx);

		virtual bool			NinjaKillIfAnyHit() { return true; }
		void					SetHitPlayer() { m_HasHitPlayer = true; }

		virtual void			CollideWithPickup(int vehIdx);
		virtual void			NinjaKill(const Game::PickupAutopsyReport& autoReport);

		virtual void			DebugDraw();
		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		virtual void			SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);

		virtual bool			Rebound(Maths::C_Normal normal, Maths::HVec3 pos);

		virtual E_PICKUP_LOCK_ON	CheckVehicleTargetted(int vehIdx);

		enum SHUNT_STATE
		{
			SHUNT_STATE_STAGE_1 = 0,
			SHUNT_STATE_STAGE_2,
			SHUNT_STATE_STAGE_3,
			SHUNT_STATE_STAGE_4,
			SHUNT_STATE_STAGE_5,
			SHUNT_STATE_STAGE_6,
			SHUNT_STATE_STAGE_7,
			SHUNT_STATE_STAGE_8,
			SHUNT_STATE_STAGE_9,
			NUM_SHUNT_STATES
		};

		void UpdateAbstractProjectile();
		void UpdateCurrentRoute();
		void UpdateTargetRoute();
		void TrackAILogicSpline();
		void UpdateRoute(ShuntRouteInfo *pShuntRoute);
		void FindNearest(ShuntRouteInfo *pShuntRoute, Maths::HVec3 pos);

		void DebugDrawSegment(airacingtech::AITrackDataRoutes *pRoute,  int index, Maths::RGBA colour, float y);
		//void AlignProjectileToGroundFirstPart(Maths::HVec3 old, Maths::HVec3 pos);
		void FindATarget();
		//float AlignProjectileToGround(Maths::HVec3 pos);
		bool LineOfSight();
		void DebugCheckTarget();
		void PhotoMove(F32 frac) OVERRIDE;

	private:

		void ShuntState3();
		void SendShuntEvadedScoringMessageToVehicle(S32 vehicleIndex);

		void CalculateECMTurnScalar(const float currentDistToTargetSqrd);
		void CheckDisplayECMEvadeWarning(const float currentDistToTargetSqrd);
		void DisplayECMEvadeWarning(); 
		void CleanUpECMWarningsOnDeath();

	private:
		float					mDropFXTimer;
		RLShuntBalancingData*	mpShuntData;
		RLShuntSpecialFX*		mpShuntSFX;
		SHUNT_STATE				m_ProjectileState;
		bool					m_forwards;

		AbstractProjectile		m_shuntCar;
		float					m_killTime;
		float					m_stage1Timer;
		bool					m_collisionCheck;

		float					m_CurrentECMHomingTurnScalar;
		float					m_ECMActivationDistSqrd	;     // Schmidt trigger for ecm activation - val one for starting ECM effects
		float					m_ECMDeactivationDistSqrd;    // val two for testing when ecm effects already active
		bool					m_ECMEffectsActive;

		float					m_ECMWarningDistSqrd;		// Dist at which ecm wil trigger an 'Evade now ! ' message
		bool					m_ECMWarningGiven;			// Has UI 'Evade now! ' warning been displayed?

		bool					m_ECMWarningEnabled;		// Enable / disable evade now warning
		bool					m_IsSplitscreen;			// ECM enabled by default in splitscreen
		
		ShuntRouteInfo			m_LogicRoute;
		ShuntRouteInfo			m_TargetRoute;
		ShuntRouteInfo			m_CurrentRoute;
		float					m_coneAngle;

		int						m_targetIndex; // Dynamic Target......
		float					m_distanceInfront;

		bool					m_HasHitPlayer;

		bool					m_HasBeenLockedOn;

		bool					m_ClosestApproach;
		Maths::HVec3			m_PrevPos;

		int						m_Sequence;
	};
}

#endif //PICKUP_SHUNT_H
