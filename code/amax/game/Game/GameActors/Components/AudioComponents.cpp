


#include "stdafx.h"

#ifndef NOT_USING_AUDIO_COMPONENETS

#include "GameComponents.h"
#include "AudioComponents.h"

//#include "actor_system/Actor.h"
//#include "GameApp.h"
//
#include "Game/GameActors/Components/Messages/AudioMessages.h"
#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/SystemSubsystem.h"
//#include <shared_actors/Components/Messages/MovementMessages.h>
//#include "GameActors/ActorMessages/WeaponMessages.h"
//
//#include "NightCam/CameraSystem.h"
//
#include "AudioMessageHandlers/BaseMessageHandler.h"
#include <Game\Messaging\GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameApp.h>
#include <Game/Game.h>

static Debug::Logger &g_Logger = Debug::GetLogger("audio.ComponentLogger");
//Debug::Int g_MaxDynmaicObjectInstances("Audio.Subsystem.DynamicObjects.MaxDynamicObjectInstances", 30);

Debug::Bool gbUsePool("Audio.Subsystem.DynamicObjects.UsePool", true);

Debug::Bool g_bAudioResetMaxDynObjCount("Audio.Subsystem.DynamicObjects.ResetMaxDynObjCount", false);
Debug::Bool g_bAudioPrintMaxDynObjCount("Audio.Subsystem.DynamicObjects.PrintMaxDynObjCount", false);

Debug::Bool g_bLoadDefault("Audio.Subsystem.LoadSubsystem.LoadDefault", true, Debug::Persist());
Debug::Bool g_bLoadVehicles("Audio.Subsystem.LoadSubsystem.LoadVehicles", true, Debug::Persist());
Debug::Bool g_bLoadDynObj("Audio.Subsystem.LoadSubsystem.LoadDynamicObjects", true, Debug::Persist());

Debug::Bool g_bLoadPositionalPickups("Audio.Subsystem.LoadSubsystem.LoadPositionalPickups", true, Debug::Persist());

//Debug::Bool g_bLoadMusic("Audio.Subsystem.LoadSubsystem.LoadMusic", true, Debug::Persist());
//Debug::Bool g_bLoadVO("Audio.Subsystem.DontLoadVO", true);
Debug::Bool g_bLoadWheels("Audio.Subsystem.LoadSubsystem.LoadWheels", true, Debug::Persist());

C_DynamicEntityList C_DynamicEntityList::m_sInstance;

void ActiveDynamicEntity::Clear()
{
	hActor = Actor::C_ActorHandle(); // clear actor handle
	hEntity.Zero();
	fTimeCreated = FLT_MAX;
	bToDestroy = false;
	callbackState = 0;
	fDistance = 0.f;
}

ActiveDynamicEntity* C_DynamicEntityList::Lookup(AudioEntityHandle& hEntity)
{
	for(int i = 0; i < C_DynamicEntityList::MAX_NUM_DYNOBJ_INSTANCES;i++)
		if (hEntity == mEntities[i].hEntity)
			return &mEntities[i];

	return NULL;
}

// get a free slot. if one does not exist then get the oldest slot
ActiveDynamicEntity* C_DynamicEntityList::GetFreeEntitySlot()
{
	/*printf("create entity. pool :");
	for (int i=0; i<MAX_NUM_DYNOBJ_INSTANCES; i++)
		printf("0x%x 0x%x, ", mEntities[i].hEntity.ArrayHandle(), mEntities[i].hActor.GetIndex());*/

	S32 sSlotToUse = 0;
		
	// see if we've got a free slot
	for (; sSlotToUse<MAX_NUM_DYNOBJ_INSTANCES; sSlotToUse++)
	{
		if (!mEntities[sSlotToUse].hEntity.IsValid())
			break;
	}
		
	if (sSlotToUse == C_DynamicEntityList::MAX_NUM_DYNOBJ_INSTANCES)
	{
		F32 fCurrOldest = FLT_MAX;
		sSlotToUse = -1;

		for (int i=0; i<C_DynamicEntityList::MAX_NUM_DYNOBJ_INSTANCES; i++)
		{
			if(mEntities[i].hEntity.IsValid())
			{
				if (mEntities[i].fTimeCreated < fCurrOldest)
				{
					fCurrOldest = mEntities[i].fTimeCreated;
					sSlotToUse = i;
				}
			}
		}

		//if (sSlotToUse != -1)
		//	printf("all slots were used. oldest slot is slot %d.", sSlotToUse);

		if (sSlotToUse == -1)
		{
			LOGWARN(Game::g_Log, "Could not add dynamic entity to active list. could not find slot");
			return NULL;
		}
	}
	else
		m_DynamicObjectCount++;

	//printf("create in slot %d\n", sSlotToUse);

	return &mEntities[sSlotToUse];

}

void C_DynamicEntityList::RemoveFromList(AudioEntityHandle& hEntity)
{
	ActiveDynamicEntity* pSlot = Lookup(hEntity);

	if (pSlot)
	{
		//printf(". releasing 0x%x.", pSlot->hEntity.ArrayHandle());

		pSlot->Clear();

		m_DynamicObjectCount--;

		/*printf("remove entity from list. pool :");
		for (int j=0; j<MAX_NUM_DYNOBJ_INSTANCES; j++)
			printf("0x%x 0x%x, ", mEntities[j].hEntity.ArrayHandle(), mEntities[j].hActor.GetIndex());*/
	
		return;
	}
	
	LOGWARN(Game::g_Log, "Could not remove dynamic entity 0x%x from active list", hEntity.ArrayHandle().GetValue());
}

void C_DynamicEntityList::SetCallbackState(AudioEntityHandle& hEntity, int state)
{
	ActiveDynamicEntity* pSlot = Lookup(hEntity);

	if (pSlot)
	{
		pSlot->callbackState = state;
	}
}

void C_DynamicEntityList::SetDistance(AudioEntityHandle& hEntity, F32 fDistance)
{
	#ifndef CFG_GOLD
	ActiveDynamicEntity* pSlot = Lookup(hEntity);

	if (pSlot)
	{
		pSlot->fDistance = fDistance;
	}
	#endif
}

namespace Components
{


I_AudioEntity::~I_AudioEntity()
{
	
	//if(!m_hEntityInst.IsValid())
	//	return;
	
	//Handler can be present without any entity instances 
	const BaseMessageHandler* pHandler = GetEntityHandler();

	//Audio::InterfaceController::Get().ReleaseEntity(m_hEntityInst);
	if(m_hEntityInst.IsValid())
	{
		//printf("~I_AudioEntity\n");

		ReleaseEntityInstance(true);
	
		m_hEntityInst = NULL;
	}

	if (pHandler)
	{
		delete pHandler;
		pHandler = NULL;
	}
       
	// free the resources
	const I_AudioEntity_CreateInfo* pCreateData = GetCreationData();

	// release the audio resources. dont release dynamic object resources as they are persistent
	if (pCreateData && pCreateData->GetXtEntityDef() && pCreateData->GetXtEntityDef()->Group() != Game::C_GameAudioLayer::eDynamicObjSubsystem)
		pCreateData->GetXtEntityDef()->ReleaseEntityType();


}

bool I_AudioEntity::PrimeImpl()
{
	// load the resources
	const I_AudioEntity_CreateInfo* pCreateData = GetCreationData();

	if (pCreateData && pCreateData->GetXtEntityDef())
	{
		Game::CacheEntityResources(pCreateData->GetXtEntityDef(), GetActor()->GetActorName(), true);
	
		// add dynamic object entities to the list of dynamic objects active in the level.
		// this is needed to increment ref counts for the audio resources to prevent
		// resources getting freed when the last actor of a specific type is destroyed.
		// if this happens we get a memory leak.
		if(pCreateData->GetXtEntityDef()->Group() == Game::C_GameAudioLayer::eDynamicObjSubsystem)
			gAudioDynObjLevelEntities.Add(pCreateData->GetXtEntityDef());
	}

	return true;
}

// Create Method
bool I_AudioEntity::CreateImpl()
{
	// due to the fact we get actor updates AFTER the E_GameflowMessage_StopInGameAudio message we need this logic
	// here to prevent creation of entity instances after that point, otherwise we get a memory leak as when the 
	// entity types are released there are still active instances.
	if (C_GameApp::GetInstance()->GetGameAudioLayer()->FineToCreateEntityInstances() == false)
		return true;

	// add subscribers here.
	//AddSubscriber(GameMessaging::E_GameMsg_Update);
	AddSubscriber(ActorMsg::E_AudioMsg_TriggerActionFromStrHash);
	AddSubscriber(ActorMsg::E_AudioMsg_SetParamFromStrHash);
	AddSubscriber(ActorMsg::E_AudioMsg_SetParameter);
	AddSubscriber(ActorMsg::E_AudioMsg_Event);
	
	const I_AudioEntity_CreateInfo* pCreationData = GetCreationData();

	// create audio entity instance
	/*XtHandle<Audio::XtEntityProxy>*/ m_hXtEntityDef = pCreationData->GetXtEntityDef();
	if(pCreationData && m_hXtEntityDef != NULL)
	{
		m_hEntityDefinition = m_hXtEntityDef->GetEntityDefHnd();

		//BIZ_ASSERT( GetEntityHandler() == NULL );
		
		//just in case
		if(GetEntityHandler() != NULL)
			DebugBreak();

		//m_hEntityInst = Audio::InterfaceController::Get().CreateEntity(hXtEntityDef->GetEntityTypeHnd());
		//m_hEntityInst = CreateEntityInstance();

		//if (m_hEntityInst.IsValid())
		{
			#ifdef GAME_AUDIO_BUILD
			LOGINFO(g_Logger, "Actor Audio Component created entity instance for '%s' ok", hXtEntityDef->Name());
			#endif			

			switch(m_hXtEntityDef->Group())			
			{

				case Game::C_GameAudioLayer::eWheelSubsystem:
				{
					if(!g_bLoadWheels)
						break;

					AddSubscriber(GameMessaging::E_GameMsg_Update);

					CreateEntityInstance();

					if(!m_hEntityInst.IsValid())
						return true;

					SetHandler( new WheelMessageHandler( m_hEntityDefinition, this ) );

					////msg.currentInstance;
					//ActorMsg::C_AudioConnectEntityMsg msg;
					//msg.m_EntityInstanceHandle = m_hEntityInst;

					//SendMessageToComponent(E_AudioEntity_Spawned, &msg);

				}					
				break;

				case Game::C_GameAudioLayer::eUISubsystem:
				{
					/*if(!g_bLoadUI)
						break;*/

					AddSubscriber(GameMessaging::E_GameMsg_Update);

					CreateEntityInstance();

					if(!m_hEntityInst.IsValid())
						return true;

					SetHandler( new UIMessageHandler( m_hEntityDefinition, this ) );

				}
				break;

				case Game::C_GameAudioLayer::eMusicSubsystem:
				{
				//	if(!g_bLoadMusic)
					//	break;

					AddSubscriber(GameMessaging::E_GameMsg_Update);

					CreateEntityInstance();

					if(!m_hEntityInst.IsValid())
						return true;

					SetHandler( new MusicMessageHandler( m_hEntityDefinition, this ) );

				}
				break;

				case  Game::C_GameAudioLayer::eDialogueSubsystem:
				{
					AddSubscriber(GameMessaging::E_GameMsg_Update);

					//CreateEntityInstance();

					//if(!m_hEntityInst.IsValid())
					//	return true;

					//SetHandler( new VOMessageHandler( m_hEntityDefinition, this ) );
				}
				break;

				case Game::C_GameAudioLayer::eDynamicObjSubsystem:
				{
					if(!g_bLoadDynObj)
						break;
					//don't create entity right now
				//	CreateEntityInstance();
					SetHandler( new DynamicObjMessageHandler( m_hEntityDefinition, this ) );
				}
				break;

				case Game::C_GameAudioLayer::ePickupSubsystem:
				{
					if(!g_bLoadPositionalPickups)
						break;

					AddSubscriber(GameMessaging::E_GameMsg_Update);

						//don't create entity right now, do it dynamically
					//CreateEntityInstance();

					//if(!m_hEntityInst.IsValid())
				//		return true;

					SetHandler( new PickupPositionMessageHandler( m_hEntityDefinition, this ) );
				}
				break;

				case Game::C_GameAudioLayer::eDefaultSubsystem:
				{
					if(!g_bLoadDefault)
						break;

					AddSubscriber(GameMessaging::E_GameMsg_Update);

					CreateEntityInstance();

					if(!m_hEntityInst.IsValid())
						return true;

					SetHandler( new WorldObjectMessageHandler( m_hEntityDefinition, this ) );
				}
				break;
			}
		}
		//else
		{
			//LOGWARN(g_Logger, "Actor Audio Component failed to create entity instance from '%s' def", m_hXtEntityDef->Name());
			//return false;
		}
	}
	else
	{
		//LOGWARN(Game::g_Log, "Actor Audio Component failed to create entity instance");
		//return false;
	}

	return true;
}

void I_AudioEntity::KillImpl()
{
	return;
	Audio::InterfaceController::Get().ReleaseEntity(m_hEntityInst);
	m_hEntityInst = NULL;
	return;
}


//create the Entity Instance
AudioEntityHandle	I_AudioEntity::CreateEntityInstance()
{
	// due to the fact we get actor updates AFTER the E_GameflowMessage_StopInGameAudio message we need this logic
	// here to prevent creation of entity instances after that point, otherwise we get a memory leak as when the 
	// entity types are released there are still active instances.
	if (C_GameApp::GetInstance()->GetGameAudioLayer()->FineToCreateEntityInstances() == false)
		return AudioEntityHandle();
	
	#ifndef CFG_GOLD
	if(m_hEntityInst.IsValid())
		DebugBreak();

	//debug debug debug
	static S32 maxDynObjCount = 0;

	if(g_bAudioResetMaxDynObjCount)
	{
		maxDynObjCount = 0;
		g_bAudioResetMaxDynObjCount = false;

	}
	#endif

	//limit dynamic objects
	if(m_hXtEntityDef->Group() == Game::C_GameAudioLayer::eDynamicObjSubsystem)
	{
		if (gbUsePool)
		{
			ActiveDynamicEntity* pSlot = C_DynamicEntityList::Get().GetFreeEntitySlot();

			if (pSlot)
			{
				if (pSlot->hEntity.IsValid())
				{
					if (pSlot->bToDestroy)
					{
						// if we got back a slot which is marked to destroy we need to release the entity first. otherwise when the callback comes in to the subsystem it wont be able to free it
						Audio::InterfaceController::Get().ReleaseEntity(pSlot->hEntity);
					}
					else
					{
						// if slot has an entity in it then tell the actor which the entity belongs to that we want it to be released
						GetActor()->SendMessageToActor(pSlot->hActor, ActorMsg::E_AudioMsg_ReleaseEntity);
					}

					pSlot->Clear();
				}	

				Audio::InterfaceController::Get().CreateEntity(m_hXtEntityDef->GetEntityTypeHnd(), m_hEntityInst);

				pSlot->hEntity = m_hEntityInst;
				pSlot->fTimeCreated = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

				pSlot->hActor = GetActorHandle();
			}
		}
		else
		{
			Audio::InterfaceController::Get().CreateEntity(m_hXtEntityDef->GetEntityTypeHnd(), m_hEntityInst);
		}
	}
	else //create for all other objects
		Audio::InterfaceController::Get().CreateEntity(m_hXtEntityDef->GetEntityTypeHnd(), m_hEntityInst);

	#ifndef CFG_GOLD
	// temp. until we have connect to system. we now have connect to system but this is useful for debugging
	Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_hEntityDefinition);
	if (pDef && pDef->LookupInstanceComponent(Audio::StringHash("OutputToRenderer")) != 0xffffffff)
		Audio::InterfaceController::Get().SetMaster(m_hEntityInst, "OutputToRenderer");
	#endif

	// connect to system entity
	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &m_hEntityInst);

	return m_hEntityInst;
}

//create the Entity Instance
bool I_AudioEntity::ReleaseEntityInstance(bool bReleaseForever)
{
	if(m_hEntityInst.m_u16SubsystemId == Game::C_GameAudioLayer::eDynamicObjSubsystem)
	{
		DynamicObjMessageHandler* pHandler = static_cast<DynamicObjMessageHandler*>(GetEntityHandler());

		// if m_hEntityInst is not valid then it's already been released
		if (m_hEntityInst.IsValid())
		{
			ActiveDynamicEntity* pSlot = C_DynamicEntityList::Get().Lookup(m_hEntityInst);
			
			// if slot is not found then it could have been freed in dynamicobjsubsystem on the endLevel message.
			if (pSlot)
			{
				// if entity is still playing then mark for delete but dont release the entity. it will get released by the DynamicObjectSubsystem
				if (bReleaseForever && pHandler->GetCallbackState() == DynamicObjMessageHandler::eWaitingForCallback)
				{
					//printf("0x%x. releasing but still playing. setting to destroy\n", m_hEntityInst.ArrayHandle());
					pSlot->bToDestroy = true;

					#ifndef CFG_GOLD
					C_DynamicEntityList::Get().SetDistance(m_hEntityInst, -1.f);
					#endif
				}
				else
				{
					// otherwise release entity and remove from active list
					C_DynamicEntityList::Get().RemoveFromList(m_hEntityInst);
					Audio::InterfaceController::Get().ReleaseEntity(m_hEntityInst);
					m_hEntityInst = NULL;
				}
			}
		}
	}	
	else
	{
		//currentDynamicObjectCount--;
		Audio::InterfaceController::Get().ReleaseEntity(m_hEntityInst);
		m_hEntityInst = NULL;
	}
	return false;
}


// Message Handlers
bool I_AudioEntity::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
	//PERF_EVENT("I_AudioEntity::MessageHandler");
	//if (!GetEntityHnd().IsValid())
	//	return false;

	BaseMessageHandlerInfo info;
	info.pMessage = pMessage;
	info.pHandler = this;

	//m_pCom->GetEntityHnd() = GetEntityHnd();	//	removeme
	info.pActor = GetActor();	//	removeme
	info.hEntityDef = GetEntityDefHnd();

	if(GetEntityHandler())
	{
		//if (!GetEntityHandler()->m_pCom->GetEntityHnd().IsValid())
		//{
		//	RemoveSubscriber(GameMessaging::E_GameMsg_Update);
		//	return true;
		//}
		bool bSuccess = GetEntityHandler()->ProcessMessage(info);

		if (bSuccess)
			return true;
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

C_AudioEntity_Spawned::~C_AudioEntity_Spawned()
{
}

C_AudioEntity_AdditionalSpawned::~C_AudioEntity_AdditionalSpawned()
{

	
	//Handler can be present without any entity instances 
	//const BaseMessageHandler* pHandler = GetEntityHandler();

	//Audio::InterfaceController::Get().ReleaseEntity(m_hEntityInst);
	if(m_hSharedEntityInst.IsValid())
	{
		//printf("~I_AudioEntity\n");

		ReleaseEntityInstance(true);
	
		m_hSharedEntityInst= NULL;
	}
       
	// free the resources
	const C_AudioEntity_AdditionalSpawnedCreateInfo* pCreateData = GetCreationData();

	if(pCreateData->GetXtSharedEntityDef())
		pCreateData->GetXtSharedEntityDef()->ReleaseEntityType();
}

bool C_AudioEntity_AdditionalSpawned::PrimeImpl()
{
	//call the baseClass first to do the first entity
	I_AudioEntity::PrimeImpl();


	// load the resources
	const C_AudioEntity_AdditionalSpawnedCreateInfo* pCreateData = GetCreationData();

	if (pCreateData && pCreateData->GetXtSharedEntityDef())
	{
		Game::CacheEntityResources(pCreateData->GetXtSharedEntityDef(), GetActor()->GetActorName(), true);
	}

	return true;
}

void C_AudioEntity_AdditionalSpawned::SetHandler(BaseMessageHandler* pHandler)
{
	m_pHandler = pHandler; 
}

// Create Method
bool C_AudioEntity_AdditionalSpawned::CreateImpl()
{
	//NOTE
	//This function handles the special additional entity caser and handles both entities within it

	// due to the fact we get actor updates AFTER the E_GameflowMessage_StopInGameAudio message we need this logic
	// here to prevent creation of entity instances after that point, otherwise we get a memory leak as when the 
	// entity types are released there are still active instances.
	if (C_GameApp::GetInstance()->GetGameAudioLayer()->FineToCreateEntityInstances() == false)
		return true;

	// add subscribers here.
	//AddSubscriber(GameMessaging::E_GameMsg_Update);
	AddSubscriber(ActorMsg::E_AudioMsg_TriggerActionFromStrHash);
	AddSubscriber(ActorMsg::E_AudioMsg_SetParamFromStrHash);
	AddSubscriber(ActorMsg::E_AudioMsg_SetParameter);
	AddSubscriber(ActorMsg::E_AudioMsg_Event);
	
	const C_AudioEntity_AdditionalSpawnedCreateInfo* pCreationData = GetCreationData();

	// create audio entity instance
	/*XtHandle<Audio::XtEntityProxy>*/ m_hXtEntityDef = pCreationData->GetXtEntityDef();

	//[Cian] set the shared instance, since the additionalEntity type has a shared entity, we need to set this up also
	m_hXtSharedEntityDef = pCreationData->GetXtSharedEntityDef();

	if(pCreationData && m_hXtEntityDef != NULL)
	{
		m_hEntityDefinition = m_hXtEntityDef->GetEntityDefHnd();

		if(m_hXtSharedEntityDef != NULL)
			m_hSharedEntityDefinition = m_hXtSharedEntityDef->GetEntityDefHnd();

		//BIZ_ASSERT( GetEntityHandler() == NULL );
		
		//just in case
		if(GetEntityHandler() != NULL)
			DebugBreak();

		//m_hEntityInst = Audio::InterfaceController::Get().CreateEntity(hXtEntityDef->GetEntityTypeHnd());
		//m_hEntityInst = CreateEntityInstance();

		//if (m_hEntityInst.IsValid())
		{
			#ifdef GAME_AUDIO_BUILD
			LOGINFO(g_Logger, "Actor Audio Component created entity instance for '%s' ok", hXtEntityDef->Name());
			#endif			

			switch(m_hXtEntityDef->Group())			
			{
				case Game::C_GameAudioLayer::eVehicleSubsystem:
				{
					if(!g_bLoadVehicles)
						break;

					AddSubscriber(GameMessaging::E_GameMsg_Update);

					CreateEntityInstance();

					if(!m_hEntityInst.IsValid())
						return true;

					SetHandler( new VehicleMessageHandler( m_hEntityDefinition, this ) );

					//msg.currentInstance;
					ActorMsg::C_AudioConnectEntityMsg msg;
					msg.m_EntityInstanceHandle = m_hEntityInst;

					//connect car through wheel
					SendMessageToComponent(E_AudioEntity_Spawned, &msg);
					
					//if there is no shared instance, break.
					if(!m_hSharedEntityInst.IsValid())
						break;
					
					ActorMsg::C_AudioConnectEntityMsg msgShared;
					msgShared.m_EntityInstanceHandle = m_hSharedEntityInst;
									
					//connect shared through wheel
					SendMessageToComponent(E_AudioEntity_Spawned, &msgShared);

					//special case: connect car through shared entity object
					Audio::EntityDefinitionHandle hCarDef = Audio::InterfaceController::Get().GetDefinitionHandle(m_hEntityInst);

					Audio::EntityDefinition* pSharedDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_hSharedEntityDefinition);
				
					Audio::LocalFunctionHandle hFunc = Audio::InterfaceController::Get().LookupParameterisedFunctionById(hCarDef, "DoConnections", pSharedDef->GetId());
					// only try to connect if "DoConnections" func exists
					if (hFunc != 0xffffffff)
					{
						Audio::InterfaceController::Get().RunFunctionInParameterisedEntityInstance( m_hEntityInst,m_hSharedEntityInst  , hFunc);
					} 

				}					
				break;

		
			}
		}

	}

	return true;
}


//create the Entity Instance
AudioEntityHandle	C_AudioEntity_AdditionalSpawned::CreateEntityInstance()
{
	// due to the fact we get actor updates AFTER the E_GameflowMessage_StopInGameAudio message we need this logic
	// here to prevent creation of entity instances after that point, otherwise we get a memory leak as when the 
	// entity types are released there are still active instances.
	if (C_GameApp::GetInstance()->GetGameAudioLayer()->FineToCreateEntityInstances() == false)
		return AudioEntityHandle();

	//call the base class function to setup the regular entity
	m_hEntityInst = I_AudioEntity::CreateEntityInstance();

	//check if it has a shared entity, if not, return the normal entity Istance handle
	if(m_hXtSharedEntityDef == NULL)
		return m_hEntityInst;

	//now do the shared aspectif it exists
	Audio::InterfaceController::Get().CreateEntity(m_hXtSharedEntityDef->GetEntityTypeHnd(), m_hSharedEntityInst);

	#ifndef CFG_GOLD
	// temp. until we have connect to system. we now have connect to system but this is useful for debugging
	Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_hSharedEntityDefinition);
	if (pDef && pDef->LookupInstanceComponent(Audio::StringHash("OutputToRenderer")) != 0xffffffff)
		Audio::InterfaceController::Get().SetMaster(m_hEntityInst, "OutputToRenderer");
	#endif

	// connect to system entity
	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &m_hSharedEntityInst);

	return m_hEntityInst;
}

//create the Entity Instance
bool C_AudioEntity_AdditionalSpawned::ReleaseEntityInstance(bool bReleaseForever)
{
	
		//currentDynamicObjectCount--;
	Audio::InterfaceController::Get().ReleaseEntity(m_hSharedEntityInst);
	m_hSharedEntityInst = NULL;

	return false;
}


C_AudioEntity_ActorStudio::~C_AudioEntity_ActorStudio()
{
}

} // namespace Components


#endif //#ifndef NOT_USING_AUDIO_COMPONENETS
