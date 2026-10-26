#include "stdafx.h"


#include <Game/Game.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <Game/Messaging/GameFlowMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <game_resource/LevelResource.h>


#include "MessageSenderActor.h"


//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::MessageSenderCreationData,		"MessageSender")
	XT_BASE(Actors::C_GameActorCreate)
/*
	XT_FIELD(m_DynamicObjectDef,				"DynamicObjType")
	XT_FIELD(m_OnPlayerCollision,				"OnPlayerCollision")
	XT_FIELD(m_pSorceryMovementComponentCreate,	"MovementComponent")
*/
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void Actors::MessageSenderCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
}




//=========================================================================
//					MessageSender Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

MessageSenderActor::MessageSenderActor()
{
}

//=========================================================================

MessageSenderActor::~MessageSenderActor()
{
}

//=========================================================================

bool MessageSenderActor::InitialiseImpl()					// Called first
{
	//S32		random_entry;

	if (!C_GameActor::InitialiseImpl())
		return(false);

	const MessageSenderCreationData *pCreate = GetCreationData();
	m_WorldMatrix = pCreate->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool MessageSenderActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	AddSubscriber(GameMessaging::E_GameMsg_Update);
	AddSubscriber(Game::E_GameflowMessage_RaceFinished);
	AddSubscriber(Game::E_GameflowMessage_VehicleFinished);
	AddSubscriber(ActorMsg::E_DynamicObjMsg_EnableLevelRoute);
	AddSubscriber(ActorMsg::E_DynamicObjMsg_DisableLevelRoute);

	return(true);
}

//=========================================================================

void MessageSenderActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
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


bool MessageSenderActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	if ((pMessage->Type() == Game::E_GameflowMessage_RaceFinished) || (pMessage->Type() == Game::E_GameflowMessage_VehicleFinished))				// Simple way to notify actors that the race has finished (using an actor studio message)
	{
		C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_RaceEnded );
		return(true);
	}

	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_OutputDebug:
		{
			const ActorMsg::C_OutputDebugMsg* pMsg = Messaging::MessageCast<ActorMsg::C_OutputDebugMsg>(pMessage);
			LOGINFO( Resource::g_Log, "Debug Message String -> %s", pMsg->m_String.Chars());
			return(true);
		}

		case ActorMsg::E_DynamicObjMsg_EnableLevelRoute:
		{
			const ActorMsg::C_EnableLevelRouteMsg* pMsg = Messaging::MessageCast<ActorMsg::C_EnableLevelRouteMsg>(pMessage);
			C_ActorUtils::Instance().levelinstance->EnableRoute( pMsg->m_RouteNumber );
			return(true);
		}

		case ActorMsg::E_DynamicObjMsg_DisableLevelRoute:
		{
			const ActorMsg::C_DisableLevelRouteMsg* pMsg = Messaging::MessageCast<ActorMsg::C_DisableLevelRouteMsg>(pMessage);
			C_ActorUtils::Instance().levelinstance->DisableRoute( pMsg->m_RouteNumber );
			return(true);
		}
	}
	
	if (pMessage->Type() != GameMessaging::E_GameMsg_Update)
	{
		C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );
		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(pMessage);		// Pass on this message to rest of game logic
	}

	return(true);
}

//=========================================================================
/*
void MessageSenderActor::Update()					// Update logic called each frame
{
}

//=========================================================================

void MessageSenderActor::KillThis()					// Cleans up any children etc before killing itself
{
	Kill();
}
*/


}// End namespace Actors



//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");




