#include "stdafx.h"
#include <data/cdata.h>
#include <controlsstate/icontrolstate.h>
#include <pidcontrollerai/ipidcontrollerai.h>
#include <vehiclestate/ivehiclestate.h>
#include <vehiclelogic/ivehiclelogic.h>
#include <accelbrakecontroller/iaccelbrakecontroller.h>
#include <trackdata/itrackdata.h>
#include <avoidance/iavoidance.h>
#include <paramsai/iparamsai.h>
#include <splinefollower/isplinefollower.h>
#include <splinetracker/isplinetracker.h>
#include <speedtracker/ispeedtracker.h>
#include <debugTools/idebugai.h>
#include <recovery/irecoveryai.h>
#include <rubberbanding/irubberbandingai.h>
#include <driverai/igamebrain.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	CData::CData() 
	{
	}

	CData::~CData()
	{

	}

	void CData::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_DATA_AI, CID_DATA_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_DATA_AI, CData::CreateMe, CData::DestroyMe, CHash("DataAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DATA_AI, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DATA_AI, MT_RACE_POSITION);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DATA_AI, MT_COLLISION);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DATA_AI, MT_SET_GAMESTATE_INTRO);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DATA_AI, MT_SET_GAMESTATE_GAME);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DATA_AI, MT_SET_GAMESTATE_OUTRO);
		
	}

	IComponent *CData::CreateMe()
	{
		return new CData;
	}

	bool CData::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CData::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CData::Deinit(void)
	{

	}

	EMessageResult CData::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{
		case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				CachedInterfaces();
			}
			break;

		case MT_SET_GAMESTATE_INTRO:
			{
				m_state.m_gameState = DataState::INTRO;
			}
			break;

		case MT_SET_GAMESTATE_GAME:
			{
				m_state.m_gameState = DataState::GAME;
			}
			break;

		case MT_SET_GAMESTATE_OUTRO:
			{
				m_state.m_gameState = DataState::OUTRO;
			}
			break;



		/*case MT_COLLISION:
			{
				CollisionInfo *pCollisionInfo = static_cast<CollisionInfo*>(msg.mpData);
				if (pCollisionInfo)
				{
					m_state.m_CollisionInfo = *pCollisionInfo;
				}
				msgRet =  MR_TRUE;
			}
			break;

		*/

		case MT_RACE_POSITION:
			{
				m_state.m_RacePos = *(static_cast<CObjectId*>(msg.mpData));
				msgRet =  MR_TRUE;
			}
			break;

		default:
			break;
		}

		return msgRet;
	}

	void CData::CachedInterfaces()
	{
		// cache all the interfaces needed for the ai...
		CObjectId id = GetObjectId();
		m_state.m_pIVehicleState = static_cast<IVehicleState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_STATE));
		m_state.m_pIPIDController = static_cast<IPIDControllerAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PID_CONTROLLER));
		m_state.m_pIControlState = static_cast<IControlState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_CONTROL_STATE));
		//m_state.m_pISplineFollowingAI = static_cast<ISplineFollowingAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPLINEFOLLOWING_AI));
		m_state.m_pIAccelBrakeController = static_cast<IAccelBrakeController *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_ACCEL_BRAKE_CONTROLLER_AI));
		//m_state.m_pSliding = static_cast<ISliding *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SLIDING));	
		m_state.m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));
		//m_state.m_pIBios = static_cast<IBios *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_BIOS_AI));
		//m_state.m_pITests = static_cast<ITest *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_TEST_AI));
		m_state.m_pIRecovery = static_cast<IRecoveryAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_RECOVERY_AI));
		m_state.m_pIParams = static_cast<IParamsAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PARAMS_AI));
		//m_state.m_pINavTracker = static_cast<INavigationTrackerAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_NAVIGATION_TRACKER_AI));
		m_state.m_pISplineTracker =  static_cast<ISplineTracker *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPLINE_TRACKER));
		m_state.m_pISplineFollower =  static_cast<ISplineFollower *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPLINE_FOLLOWER));
		m_state.m_pISpeedTracker = static_cast<ISpeedTracker *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPEED_TRACKER));
		m_state.m_pVehicleLogic = static_cast<IVehicleLogic *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_LOGIC));
		
		m_state.m_pIDebugAI = static_cast<IDebugAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DEBUG_AI));
		m_state.m_pIRubberBanding = static_cast<IRubberbandingAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_RUBBERBANDING_AI));

		m_state.m_pIntroOutroSplineAI = static_cast<IIntroOutroSplineAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_INTRO_OUTRO_SPLINE_AI));
		m_state.m_pIGameBrainAI = static_cast<IGameBrain *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_GAMEBRAIN));


	}


	EComponentTypeId CData::GetComponentTypeId(void)
	{
		return CID_DATA_AI;
	}


	const bool CData::IsPlayer() const 
	{
		bool eval = false;

		IVehicleState *pIVehicleState = GetVehicleState();

		if (pIVehicleState && pIVehicleState->GetState()->GetPlayer())
		{
			eval = true;
		}

		return eval;
	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
