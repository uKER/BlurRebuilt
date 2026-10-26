#ifndef AI_AMAX_H
#define AI_AMAX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageHandler.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>
#include <air.h>
#include <string.h>
#include <system/serialise.h>
#include <btl/containers/vector.h>
#include <system/scheduler/jobqueue.h>
//#include <AI_AmaxData.h>
#include <Mode/Destruction/iModeDestruction.h>
#include <Mode/FanDemand/iModeFanDemand.h>


// forward decs
namespace RacingPhysics
{
	class Vehicle;
	class Car;
}

namespace Game
{
	namespace Actors
	{
		class C_VehicleActor;
		class C_VehicleUpdateAIControlsMsg;
	}

	class C_ImpactMsg;
	class C_LevelState;
}

namespace DbUtil { class I_DebugRender; }

struct AIXt;
struct AISteeringXt;

namespace airacingtech
{
	class IPerkSystem;
	class IModeDestruction;
}

namespace AIAmax
{

	class AILoadDriveraMsg;
	class AIAmaxVehicleData;
	class AIPerkLinkMessage;
	
	class AmaxAIR : public Messaging::C_MessageHandler
	{
		static const int MIN_MP_RANK = 0;
		static const int MAX_MP_RANK = 49;

		static const float MAX_TRACK_SPEED;
		static const float MIN_TRACK_SPEED;
	public:

		static AmaxAIR& Instance() {return m_Instance;}

		AmaxAIR();
		
		// Safely shut down the ai....
		void ShutDown();

		// Handle game side messages
		virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		void LoadAIGameplayBalancing();
		bool LoadAICityData(const char *fileName);
		bool LoadAIEventData(const char *fileName);
		
		void Initialise(Messaging::C_MessageSubscriberSystem *pMessageSubscriberSystem);//, const char *fileName);

		void Update(bool updateSystem);
		void UpdateSystem(bool updateSystem);
		void PreUpdateExternal(int i);
		void PostUpdateExternal(int i);
		void ResetAfterHOG(int index);
	
		void AITakeControl(int p_Index);
		void AIRelinquishControl(int p_Index);
		void SetIsLocalPlayer(int p_Index, bool p_IsLocalPlayer);

		// debug draw
		DbUtil::I_DebugRender* DebugRenderer();
		void DebugDraw();
		void DebugDrawTrackData();
		void DebugDrawSpline(const airacingtech::AITrackDataRoutes *pRoute, int line,  Maths::RGBA colour);
		void DebugDrawWidth(const airacingtech::AITrackDataRoutes *pRoute, int line1, int line2,  Maths::RGBA colour);
		void DebugDrawSegment(const airacingtech::RouteInfo *pRouteInfo,  Maths::RGBA colour);
		void DebugDrawSegment();
		void DebugDrawSplineFollower();
		void DebugDrawVolume(airacingtech::AITrackDataRoutes *pRoute, Maths::RGBA colour);
		void DebugTest();

		/////////////////////////////////////////////////////////////////////////////////////////////////
		// Interface and functions for other people...
		//

		// this returns the last valid postion and direction of the track.....needs more work or a different solution..
		inline bool GetValidVehiclePosDir(int i, Maths::HVec3 &position, Maths::HVec3 &direction, float &speed)
		{
			bool status = false;
			
			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(i);
				if (pIData)
				{
					airacingtech::ISplineTracker *pSplineTracker = pIData->GetSplineTracker();
					if (pSplineTracker)
					{
						pSplineTracker->GetValidPosDir(position, direction, speed);

						if (speed >= 0)
						{
							status =  true;
						}	
					}
				}
			}
			
			return status;
		}
		
		// [Blur2PC] the loaded track data without the AllOK() gate: race logic checks the splits
		// before InitRace() sets m_safeToSimulate, so GetTrackDataSystem() returned NULL there.
		inline airacingtech::ITrackDataAI *GetLoadedTrackDataSystem() const { return m_DataInitilaised ? m_pITrackDataSystem : NULL; }

		inline airacingtech::ITrackDataAI *GetTrackDataSystem() const 
		{
			if ( AllOK() )
			{
				return m_pITrackDataSystem;
			}
			else
			{
				return 0;
			}
		}

			
		inline airacingtech::AIEventBalancingData *GetEventBalancingData(int index)
		{
			airacingtech::AIEventBalancingData *pEventData = 0;
			
			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					airacingtech::IRubberbandingAI *pIRubberBanding = pIData->GetRubberBanding();
					if (pIRubberBanding)
					{
						pEventData = pIRubberBanding->GetEventBalancingData();
					}
				}
			}

			return pEventData;
		}
		

		inline airacingtech::AIEventSegmentData *GetEventBalancingSegmentData(int index)
		{
			airacingtech::AIEventSegmentData *pSegmentData = 0;

			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					airacingtech::IRubberbandingAI *pIRubberBanding = pIData->GetRubberBanding();
					if (pIRubberBanding)
					{
						pSegmentData = pIRubberBanding->GetEventBalancingSegmentData();
					}
				}
			}

			return pSegmentData;
		}



		inline airacingtech::ISplineTracker *GetAISplineTracker(int index) const
		{
			airacingtech::ISplineTracker *pSplineTracker = 0;
			
			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					pSplineTracker = pIData->GetSplineTracker();
				}
			}

			return pSplineTracker;
		}

		inline float TrackLength() const 
		{
			float trackLength = -1.0f;
			if ( AllOK() )
			{
				airacingtech::AITrackData *pAITrackData = m_pITrackDataSystem->GetTrackData();
				if (pAITrackData && pAITrackData->m_TrackLogicData.m_pLogicRoute)
				{
					trackLength = pAITrackData->m_TrackLogicData.m_noSamples * pAITrackData->m_TrackLogicData.m_stepSize;
				}
			}

			return trackLength;
		}

		inline float DistanceFromStartOfSpline(float pos) // Note this is not the start line.
		{
			float dist = 0.0f;

			dist = pos * TrackLength();
			/*if ( AllOK() )
			{
				airacingtech::AITrackData *pAITrackData = m_pITrackDataSystem->GetTrackData();
				if (pAITrackData && pAITrackData->m_TrackLogicData.m_pLogicRoute)
				{
					dist = pos * pAITrackData->m_TrackLogicData.m_noSamples * pAITrackData->m_TrackLogicData.m_stepSize;
				}
			}*/

			return dist;
		}


		inline float DistanceInfrontBehindLogicCalculationParamatised(float pos1, float pos2)
		{
			float dist = 0.0f;

			float pos = airacingtech::DistanceInfrontBehindF(pos2,pos1);

			dist = DistanceFromStartOfSpline(pos);

			return dist;
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		inline bool AllOK() const
		{
			bool ok = false;
			if (m_pITrackDataSystem && m_DataInitilaised == true && m_SystemsInitialised == true && m_pISystemAI && m_safeToSimulate)
			{
				ok = true;
			}

			return ok;
		}


		inline float ApproxDistanceThroughRace(int index)
		{
			float dist = 0.0f;

			if ( AllOK() )
			{
				airacingtech::AITrackData *pAITrackData = m_pITrackDataSystem->GetTrackData();
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData && pAITrackData)
				{
					airacingtech::IVehicleLogic *pVehicleLogic = pIData->GetVehicleLogic();
					if (pVehicleLogic)
					{
						float combinedPosition = pVehicleLogic->GetLogicState()->GetLogicPositionCombined();
						//combinedPosition -= 1.0f;
						//combinedPosition = Maths::Max(combinedPosition, 0.0f);
						combinedPosition *= pAITrackData->m_TrackLogicData.m_noSamples * pAITrackData->m_TrackLogicData.m_stepSize;
						dist = combinedPosition;
					}
				}
			}

			return dist;
		}

		inline void StoreResetLogicState(int index)
		{
			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					if (pIData->GetVehicleLogic())
					{
						pIData->GetVehicleLogic()->StoreResetLogicState();
					}
				}
			}
		}

		inline void ResetResetLogicState(int index)
		{
			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					if (pIData->GetVehicleLogic())
					{
						pIData->GetVehicleLogic()->ResetResetLogicState();
					}
				}
			}
		}

		
		inline airacingtech::LogicState *GetResetLogicState(int index)
		{
			airacingtech::LogicState *pState = 0;

			if ( AllOK() )
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					if (pIData->GetVehicleLogic())
					{
						pState = pIData->GetVehicleLogic()->GetResetLogicState();
					}
				}
			}

			return pState;
		}


		inline float GetDistanceToCentreOfCurrentRoute(int index)
		{
			float dist = 0.0f;

			if (index >=0 && index < m_cars.Size())
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(index);
				if (pIData)
				{
					airacingtech::ISplineTracker *pSplineTracker = pIData->GetSplineTracker();
					if (pSplineTracker)
					{
						const airacingtech::RouteInfo *pRouteInfo = pSplineTracker->GetCurrentRoute();
						if (pRouteInfo)
						{
							dist = pRouteInfo->m_offsetCentre * pRouteInfo->m_width;
						}
					}
				}
			}
			

			return dist;
		}

		



		void SteeringImpluseForceTorqueAI(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput, Maths::HVec3 &AItorque, Maths::HVec3 &AIlocalforce);
		
		void SteeringImpluseForceTorqueAIandPlayer(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput, Maths::HVec3 &AItorque, Maths::HVec3 &AIlocalforce);
		void BrakeForceAIandPlayer(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput, Maths::HVec3 &AItorque, Maths::HVec3 &AIlocalforce);

		void AIGameLogic();
		void AIGameLogicRecovery(int i);
		void AIRubberBanding(int i);

		void HandleStartVehiclesAtSpeed(float percentage);

		void BroadCastTrackLogicMessages(int i);
		void UpdatePerks();

		void SetTimeStep(float timeStep) { m_TimeStep = timeStep; }
		
		inline bool IsDataInitialised() const {return m_DataInitilaised;}


		void FireFireMessages();


	private:

		Scheduler::GroupHandle		m_Group;
		Scheduler::PolicyHandle		m_AIPolicy;
		
		// create load functions...
		static AmaxAIR					m_Instance;
		bool							m_DataInitilaised;
		bool							m_SystemsInitialised;
		bool							m_enableAI;
		bool							m_safeToSimulate;
		bool							m_trackLoaded;
		bool							m_driversLoaded;
		bool							m_handBrakeOn;

		float							m_TimeStep;

		airacingtech::CObjectManager	*m_pObjectManager;

		// ISystemAI is the cached interface pointer of CObjectId....below should be the same object
		airacingtech::CObjectId			m_IDSystemAI;
		airacingtech::ISystemAI			*m_pISystemAI;
		airacingtech::IGameServerTime	*m_pIGameServerTime;
		airacingtech::ITrackDataAI		*m_pITrackDataSystem;
		airacingtech::IPerkSystem		*m_pIPerkSystem;
		airacingtech::IModeDestruction	*m_pIModeDestruction;
		airacingtech::IModeFanDemand	*m_pIModeFanDemand;
	
		
	
		void RegisterComponents();
		void InitialiseSystems();
		void InitAISystem();
		void InitTrackDataSystem();
		void InitServerTime();
		void InitRaceDirector();
		void InitPerkSystem();
		void InitDestructionSystem();
		void InitFanDemandSystem();

		
		inline bool IsSystemsInitialised() const {return m_SystemsInitialised;}


		bool LoadAITrackData(const char *fileName);
		bool LoadEventData(const char *fileName);
		
		
		bool LoadVersionSix(Sys::BinaryDeserialiser<Ut::LittleEndianToNative> &ar, airacingtech::AITrackData *pAIData, airacingtech::ITrackDataAI *pITrackData);


		void LoadAIDriver(const AILoadDriveraMsg* pDriver);
		void CopyDriverData(airacingtech::DriverData &driver, AIXt *pAiData, float difficulty);
		void CopySteeringData(airacingtech::SteeringControllerInitialiseParams &nodeSteering, AISteeringXt *pAISteering);
		void CopyPIDData(airacingtech::PIDControllerInitialiseParams &nodePID, AISteeringXt *pAISteering);
		void FillInVehicleStructure(const RacingPhysics::Vehicle *pCar, airacingtech::VehicleState *pVehState);
		void FillInAnyRaceLogic(Game::Actors::C_VehicleActor *pActor, int iteration, airacingtech::IData *pIData);	
		void PostUpdate();
		void PostUpdateControlsOnly();
		void PreUpdate();
		void PostInitialise();
		void InitRace();
		void ReLoadSorceryAIData();
		void AddInAirForcesAndTorques(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput);
		void HandleCollision(Game::C_ImpactMsg* pMsg);
		void HandleCreatePerk(AIPerkLinkMessage *pMsg);

		inline int FindIndexFromActor(const Game::Actors::C_VehicleActor *pFindVehicleActor)
		{
			int index = -1;
			for(int i=0; i<m_cars.Size(); i++)
			{
				AIAmaxVehicleData *pData = m_cars[i];
				if (pData->m_pVehicleActor == pFindVehicleActor)
				{
					index = i;
					break;
				}
			}

			return index;
		}

	

	
		struct AIAmaxVehicleData
		{
			S32									m_Event;			// Rubber banding and Difficulty.....
			S32									m_AISteering;		// AI steering template
			S32									m_AIIndex ;			// AI behaviour template
			const Game::Actors::C_VehicleActor	*m_pVehicleActor;	// pointer to the vehicle actor, holds the rigid body, racing physics and stuff
		};

		Btl::Vector<AIAmaxVehicleData *>	m_cars;

		bool	m_RollingStartSpeedEnabled_HACK;
		bool	m_startRaceDisablePickupsBeingAdded;

	};






} //namespace AIAmax

#endif // AI_AMAX_H
