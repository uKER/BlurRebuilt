#include "stdafx.h"


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include "SpawnerActor.h"
#include <misc_shared\Messaging\GameMessaging.h>


//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::SpawnerCreationData,		"Spawner")
	XT_BASE(Actors::C_PosActorCreate)
/*
	XT_FIELD(m_DynamicObjectDef,				"DynamicObjType")
	XT_FIELD(m_OnPlayerCollision,				"OnPlayerCollision")
	XT_FIELD(m_pSorceryMovementComponentCreate,	"MovementComponent")
*/
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void Actors::SpawnerCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

//	if (m_pSorceryMovementComponentCreate != NULL)
//		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate, &m_pMovementCreator);
}




//=========================================================================
//					Spawner Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

SpawnerActor::SpawnerActor()
{
}

//=========================================================================

SpawnerActor::~SpawnerActor()
{
}

//=========================================================================

bool SpawnerActor::InitialiseImpl()					// Called first
{
	//S32		random_entry;

	if (!C_GameActor::InitialiseImpl())
		return(false);

	const SpawnerCreationData *pCreate = GetCreationData();
	m_WorldMatrix = pCreate->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool SpawnerActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	AddSubscriber(GameMessaging::E_GameMsg_Update);

	return(true);
}

//=========================================================================

void SpawnerActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
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


bool SpawnerActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
			Update();
			break;

/*
		case ActorMsg::E_GameActorMsg_Disable:					// MESSAGE TEST TEST TEST TEST
			KillThis();
			break;

		case ActorMsg::E_GameActorMsg_SetTransform:
		{
			const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
			m_WorldMatrix = msg->Transform().Matrix();
			break;
		}

		case ActorMsg::E_GameActorMsg_AnimateTransform:
		{
			const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
			m_WorldMatrix = msg->Transform().Matrix();
			break;
		}
*/
		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

void SpawnerActor::Update()					// Update logic called each frame
{
//	if(m_pModelInstance != NULL)
//		m_pModelInstance->SetElementMatrix(-1, m_WorldMatrix);
}

//=========================================================================

void SpawnerActor::KillThis()					// Cleans up any children etc before killing itself
{
	Kill();
}



}// End namespace Actors



//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");




