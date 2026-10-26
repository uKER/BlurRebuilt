#include "stdafx.h"


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>


#include <core/xtstdx.h>


#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>

#include <GameLogic/GameData/GameEventData.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <shared_actors/SharedActorUtils.h>

#include "PhysicsModifierActor.h"




//#pragma optimize("", off)


//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::PhysicsModifierCreationData,	"PhysicsModifier")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(box,								"Box")
	XT_FIELD(m_Deactivate,						"Deactivate")
	XT_FIELD(m_Force,							"Force")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void Actors::PhysicsModifierCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);
}




//=========================================================================
//					PhysicsModifier Actor Core Routines
//=========================================================================




namespace Actors
{


//=========================================================================

PhysicsModifierActor::PhysicsModifierActor()
{
	m_Active = false;
	m_pTriggerObject = NULL;
}

//=========================================================================

PhysicsModifierActor::~PhysicsModifierActor()
{
}

//=========================================================================

bool PhysicsModifierActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	const PhysicsModifierCreationData *pCreate = GetCreationData();
	m_WorldMatrix = pCreate->GetCreationMatrix();
	return(true);
}


//=========================================================================

bool PhysicsModifierActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	::Actors::C_ActorUtils::Instance().DestroyTrigger(m_pTriggerObject);		// Just to make sure
	m_pTriggerObject = NULL;
	m_pTriggerObject = ::Actors::C_ActorUtils::Instance().CreateTrigger(GetHandle(), m_WorldMatrix, GetCreationData()->box);
	if (!m_pTriggerObject)
		LOGWARN( Resource::g_Log, "Physics Trigger region not created");

	AddSubscriber(Game::E_GameMsg_PostCreateInit);
	AddSubscriber(GameMessaging::E_GameMsg_Update);
	return(true);
}

//=========================================================================

void PhysicsModifierActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
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


bool PhysicsModifierActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{//
	//PERF_EVENT("PhysicsModifierActor::MessageHandler");
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
			if (!GetCreationData()->m_Deactivate)
				m_Active = true;
			break;


		case GameMessaging::E_GameMsg_Update:
		{
			U32		a, numactors;
			Actor::C_ActorList* actorList		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
			Gravity::C_TriggerSystem* system = m_pTriggerObject->GetTriggerSystem();
			if (system)
			{
				Gravity::C_TriggerCollider* collider = system->GetActorsInTrigger(m_pTriggerObject, numactors);
				if (collider)
					for(a=0;a<numactors;a++)
					{
						Actor::C_ActorHandle gameactor = SharedActorUtils::HandleFromUserData( collider[a].m_pTriggerActor->GetUserData() );
						const Actor::C_Actor* pActor		= actorList->GetActorPtr(gameactor);
						if (pActor && pActor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle)
						{
							Game::Actors::C_VehicleActor *pVehicleActor = (Game::Actors::C_VehicleActor *)pActor;
							pVehicleActor->GetCollisionBody()->GetBody().ApplyImpulseWorldSpace(m_WorldMatrix.AZ() * GetCreationData()->m_Force, pVehicleActor->GetMatrix().Translation());
						}
					}
			}
			break;
		}


/*
		case GameMessaging::E_GameMsg_Update:
			Update();
			break;

		case Game::E_VehicleMessage_VehicleWorldPosition:
		{
			if (m_Active)
			{
				const Game::Actors::C_VehicleWorldPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleWorldPositionMsg>(pMessage);
				localpos = pMsg->mWorldPosition * inverse_world_matrix;
				if ((localpos.X() >= 0.0f) && (localpos.Y() >= 0.0f) && (localpos.Z() >= 0.0f) && (localpos.X() <= extents.X()) && (localpos.Y() <= extents.Y()) && (localpos.Z() <= extents.Z()))			// If within the region
				{
					const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(pMsg->mActorHandle);
					Game::Actors::C_VehicleActor *pVehicleActor = (Game::Actors::C_VehicleActor *)pActor;
					pVehicleActor->GetCollisionBody()->GetBody().ApplyImpulseWorldSpace(m_WorldMatrix.AZ() * GetCreationData()->m_Force, pVehicleActor->GetMatrix().Translation());
				}
			}

			break;
		}
*/

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

//=========================================================================

void PhysicsModifierActor::KillThis()					// Cleans up any children etc before killing itself
{

	::Actors::C_ActorUtils::Instance().DestroyTrigger(m_pTriggerObject);
	m_pTriggerObject = NULL;

	Kill();
}

//=========================================================================








}// End namespace Actors


//#pragma optimize("", on)






