#include "stdafx.h"


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <core/xtstdx.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/RaceLogicCore/Data/RLDataStructures.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/Messaging/GameMessages.h>

#include <GameLogic/ActorStudioActors/TriggerRegionActor.h>

#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <shared_actors/SharedActorUtils.h>

#include "Ui/HudDataFinder.h"

#include <actor_studio_msg_game/ActorStudioMessaging.h>

Debug::Logger &g_TriggerRegionActorLog( Debug::GetLogger("amax.actors.triggerregion") );


//#pragma optimize("", off)


//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::TriggerRegionCreationData,		"TriggerRegion")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(box,								"Box")

	XT_FIELD(m_TriggerType,						"Type")
	XT_FIELD(m_RefNumber,						"RefNumber")

	XT_FIELD(m_DestroyWhenTriggered,			"DestroyWhenTriggered")

	XT_FIELD(m_PlayerControlled,				"PlayerControlled")
	XT_FIELD(m_AiControlled,					"AiControlled")
	XT_FIELD(m_MinLap,							"MinLap")
	XT_FIELD(m_MaxLap,							"MaxLap")

	XT_FIELD(m_OnTriggered,						"OnTriggered")
	XT_FIELD(m_OnExited,						"OnExited")

	XT_FIELD(m_VehicleMessage,					"VehicleMessage")
	XT_FIELD(m_VehicleMessageOnExit,			"VehicleMessageOnExit")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void Actors::TriggerRegionCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);
}




//=========================================================================
//					TriggerRegion Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

TriggerRegionActor::TriggerRegionActor()
{
	m_Active = false;
	m_pTriggerObject = NULL;
	m_Selected = false;
}

//=========================================================================

TriggerRegionActor::~TriggerRegionActor()
{
}

//=========================================================================

bool TriggerRegionActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	const TriggerRegionCreationData *pCreate = GetCreationData();
	m_WorldMatrix = pCreate->GetCreationMatrix();

	return(true);
}


//=========================================================================


bool TriggerRegionActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	::Actors::C_ActorUtils::Instance().DestroyTrigger(m_pTriggerObject);		// Just to make sure
	
	m_pTriggerObject = ::Actors::C_ActorUtils::Instance().CreateTrigger(GetHandle(), m_WorldMatrix, GetCreationData()->box);

	if (!m_pTriggerObject)
		LOGWARN( Resource::g_Log, "Trigger region not created");

	AddSubscriber(Game::E_VehicleMessage_CheckTriggerRegion);
	AddSubscriber(Game::E_GameMsg_PostCreateInit);

	// PR - tell the UI about this
	UI::C_HudDataFinder::Instance().AddTriggerRegionActor( GetHandle() );

	return(true);
}



//=========================================================================


void TriggerRegionActor::KillImpl()
{
	::Actors::C_ActorUtils::Instance().DestroyTrigger(m_pTriggerObject);
	m_pTriggerObject = NULL;

	C_GameActor::KillImpl();
}

//=========================================================================


void TriggerRegionActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
			break;
		
		case Resource::E_Notify_DataError:
			break;
		
		case Resource::E_Notify_NoData:
			break;
	}
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool TriggerRegionActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	//PERF_EVENT("TriggerRegionActor::MessageHandler");
	switch(pMessage->Type())
	{
		case ActorMsg::E_GameActorMsg_Activate:
			m_Active = true;
			break;

		case ActorMsg::E_GameActorMsg_Deactivate:
			m_Active = false;
			break;

		case Game::E_GameMsg_PostCreateInit:
			RemoveSubscriber(Game::E_GameMsg_PostCreateInit);
			m_Active = true;
			break;

		case Game::E_VehicleMessage_CheckTriggerRegion:
		{
			const Game::Actors::C_VehicleCheckTriggerRegionMsg* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleCheckTriggerRegionMsg>(pMessage);
			const Game::Actors::RaceLogicCore* pRLC						= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			const Actor::C_ActorList* pActorList						= pRLC->GetGameWorld()->GetActorList();
			const Game::Actors::C_VehicleActor* pVehicleActor			= Actor::ActorCast_Safe<Game::Actors::C_VehicleActor>( pActorList->GetActorPtr(pMsg->mVehicleActorHandle) );
					
			CheckRegion(pMsg->mVehicleActorHandle, pVehicleActor->GetVehicleIndex(), pVehicleActor->GetMatrix().AW(), pMsg->mEventType);
			break;
		}

		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		{
			if (m_pTriggerObject)
			{
				U8 data[1024 * sizeof(void*)];
				SharedPhysics::C_SafeStack<true> physicsStack(data, 1024);
				m_pTriggerObject->SetMatrix(m_WorldMatrix, physicsStack);
			}
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		{
			m_Selected = true;
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			m_Selected = false;
		}
		break;

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

void TriggerRegionActor::CheckRegion(Actor::C_ActorHandle vehicle_handle, S8 index, Maths::HVec3 pos, S32 event_type)
{
	if (!m_Active)
		return;

	if (event_type == Gravity::C_TriggerEvent::E_Enter_Event)
	{
		ActorMsg::C_VehicleEnteredTriggerMsg	entry_msg;				// Entered a trigger region
		entry_msg.m_TriggerType	= GetCreationData()->m_TriggerType;
		entry_msg.m_RefNumber	= GetCreationData()->m_RefNumber;
		SendMessageToActor(vehicle_handle, &entry_msg);

		if (CheckTriggered(vehicle_handle))
		{
			ActorMsg::C_VehicleActivatedTriggerMsg		activate_msg;				// Activated the trigger
			activate_msg.m_TriggerType	= GetCreationData()->m_TriggerType;
			activate_msg.m_RefNumber	= GetCreationData()->m_RefNumber;
			//SendMessageToActor(vehicle_handle, &activate_msg);

			GetCreationData()->m_OnTriggered.Send(this);					// Notify other actors that this object has been triggered

			if(GetCreationData()->m_VehicleMessage)
				SendMessageToActor(vehicle_handle, GetCreationData()->m_VehicleMessage->GetMessage(this));		// Send any specified messages to the vehicle thats triggered the trigger :)

			if (GetCreationData()->m_DestroyWhenTriggered)
				KillThis();
		}
	}
	else if (event_type == Gravity::C_TriggerEvent::E_Exit_Event)
	{
		ActorMsg::C_VehicleExitedTriggerMsg		exit_msg;				// Exited a trigger region
		exit_msg.m_TriggerType	= GetCreationData()->m_TriggerType;
		exit_msg.m_RefNumber	= GetCreationData()->m_RefNumber;
		SendMessageToActor(vehicle_handle, &exit_msg);

		if(GetCreationData()->m_VehicleMessageOnExit)
			SendMessageToActor(vehicle_handle, GetCreationData()->m_VehicleMessageOnExit->GetMessage(this));		// Send any specified messages to the vehicle thats triggered the trigger :)

		GetCreationData()->m_OnExited.Send(this);						// Notify other actors that a vehicle has exited this trigger
	}
}


//=========================================================================

bool TriggerRegionActor::CheckTriggered(Actor::C_ActorHandle vehicle_handle)
{
	// All this just to get a bit of info about the car!!!!!!!!!!!!!!!
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	if (!pRLC)
	{
		LOGFATAL(g_TriggerRegionActorLog, "TriggerRegionActor::CheckTriggered() - pRLC == NULL");
		return false;
	}

	const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(vehicle_handle);
	if (!pActor)
	{
		LOGFATAL(g_TriggerRegionActorLog, "TriggerRegionActor::CheckTriggered() - pActor == NULL");
		return false;
	}

	Game::Actors::C_VehicleActor *pVehicleActor = (Game::Actors::C_VehicleActor *)pActor;
	class RacingPhysics::Car *pCar = pVehicleActor->GetVehiclePhysics();
	if (!pCar)
	{
		LOGFATAL(g_TriggerRegionActorLog, "TriggerRegionActor::CheckTriggered() - pCar == NULL");
		return false;
	}
	class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);

	if ((GetCreationData()->m_PlayerControlled) || (GetCreationData()->m_AiControlled))		// Ignore control types if both off
	{
		if ((pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER) && (!GetCreationData()->m_PlayerControlled))
			return(false);

		if ((pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI) && (!GetCreationData()->m_AiControlled))
			return(false);
	}

	if ((GetCreationData()->m_MinLap != 0) && (GetCreationData()->m_MaxLap != 0))			// Ignore lap restrictions if both set to zero
	{
		const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(pVehicleActor->GetVehicleIndex());
		U8 lap = pLogicActor->GetLapInfo().mCurrentLap;
		if ((lap < GetCreationData()->m_MinLap) || (lap > GetCreationData()->m_MaxLap))
			return(false);
	}

	return(true);
}

//=========================================================================

void TriggerRegionActor::KillThis()					// Cleans up any children etc before killing itself
{
//	DestroyTrigger();

	::Actors::C_ActorUtils::Instance().DestroyTrigger(m_pTriggerObject);
	m_pTriggerObject = NULL;

	Kill();
}

void TriggerRegionActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	debugRenderPtr->Draw3dMatrix(m_WorldMatrix, 0.5f);
	if(m_Selected)
		debugRenderPtr->Draw3dText(Maths::HVec3(m_WorldMatrix.Translation()), "Selected");
}


}// End namespace Actors


//#pragma optimize("", on)



