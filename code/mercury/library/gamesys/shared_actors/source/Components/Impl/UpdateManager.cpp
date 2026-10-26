#include "SharedActorsPch.h"

#include <algorithm>

#include <misc_shared/Messaging/GameMessaging.h>

#include <actor_system/StaggerManager.h>
#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <actor_system/ActorMessages.h>

//#include "GameActors/GameActor.h"
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Impl/UpdateManager.h>
#include <shared_actors/Components/Creation/UpdateManagerCreate.h>
#include <shared_actors/Components/Messages/UpdateManagerMessages.h>

#if defined(SHARED_ACTORS_OPTIMIZE_OFF)
#pragma optimize( "", off ) 
#endif

// Debug Varibales
Debug::Float g_dbStagger2FrameRangeScale("ActorDebug.Components.StaggerUpdate.2FrameRangeScale", 0.5f, Debug::Range(0.f, 1.0f));
Debug::Float g_dbStagger4FrameRangeScale("ActorDebug.Components.StaggerUpdate.4FrameRangeScale", 0.6f, Debug::Range(0.f, 1.0f));
Debug::Float g_dbStagger8FrameRangeScale("ActorDebug.Components.StaggerUpdate.8FrameRangeScale", 0.7f, Debug::Range(0.f, 1.0f));
Debug::Float g_dbStagger16FrameRangeScale("ActorDebug.Components.StaggerUpdate.16FrameRangeScale", 0.8f, Debug::Range(0.f, 1.0f));
Debug::Float g_dbStagger32FrameRangeScale("ActorDebug.Components.StaggerUpdate.32FrameRangeScale", 0.9f, Debug::Range(0.f, 1.0f));
Debug::Int g_dbUpdateWarmupFrames("ActorDebug.Components.UpdateControl.WarmupFrames", 2);
Debug::Bool g_dbUpdateSeedDebugRender("ActorDebug.Components.UpdateSeed.DebugRender.Enable", false);

U32 GetStaggerLevelFromDistance(F32 distance, F32 range)
{
	// handle update stagger
	F32 fStaggerLevel( 0.0f );

	//                            test									if test >= 0	if test < 0
	fStaggerLevel = Maths::Select_GE_Zero( distance - (range * g_dbStagger2FrameRangeScale), 1.0f , fStaggerLevel);
	fStaggerLevel = Maths::Select_GE_Zero( distance - (range * g_dbStagger4FrameRangeScale), 2.0f , fStaggerLevel);
	fStaggerLevel = Maths::Select_GE_Zero( distance - (range * g_dbStagger8FrameRangeScale), 3.0f , fStaggerLevel);
	fStaggerLevel = Maths::Select_GE_Zero( distance - (range * g_dbStagger16FrameRangeScale), 4.0f , fStaggerLevel);
	fStaggerLevel = Maths::Select_GE_Zero( distance - (range * g_dbStagger32FrameRangeScale), 5.0f , fStaggerLevel);

	return (U32)fStaggerLevel;
}

namespace Components
{

C_UpdateManagerComponent::C_UpdateManagerComponent():m_WarmUpFrames(0)
{

}

// concrete update manager base class
C_UpdateManagerComponent::~C_UpdateManagerComponent()
{
	// make sure actor is updating again
	//SendMessageToActor(E_GameMsg_UpdateEnable);
}

// from Component::I_GameComponent
void	C_UpdateManagerComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_UpdateManagerComponent::SerialiseForSave(stream);
	stream << m_DistFromSeed;
}

void	C_UpdateManagerComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_UpdateManagerComponent::DeserialiseForSave(stream);
	stream >> m_DistFromSeed;
}

void	C_UpdateManagerComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_UpdateManagerComponent::SerialiseForSync(stream);
	stream << m_DistFromSeed;
}

void	C_UpdateManagerComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_UpdateManagerComponent::DeserialiseForSync(stream);
	stream >> m_DistFromSeed;
}


// Active Range Update Component
// -----------------------------
// Switches actor update on/off based on range from the seed
C_ActiveRangeUpdateComponent::~C_ActiveRangeUpdateComponent()
{
	if(m_SeedCalcStaggerGroup!=-1)
	{
		Util::C_StaggerManager &staggerManager = GetActorList()->GetStaggerManager();
		staggerManager.FreeGroup(m_SeedCalcStaggerGroup);
	}
}

bool C_ActiveRangeUpdateComponent::InitialiseImpl()
{
	if(!C_UpdateManagerComponent::InitialiseImpl())
		return false;

	const C_ActiveRangeUpdateComponentCreator *pActiveRangeCreator = GetCreationData();
	m_ActiveRange = pActiveRangeCreator->m_ActiveRange;
	m_bOutOfRange = pActiveRangeCreator->m_StartAsleep;
	return true;
}

bool C_ActiveRangeUpdateComponent::CreateImpl()
{
	if(!C_UpdateManagerComponent::CreateImpl())
		return false;

	m_WarmUpFrames = g_dbUpdateWarmupFrames;

	AddSubscriber(ActorMsg::E_GameActorMsg_UpdateSeedPosition);
	if(m_bOutOfRange == false && m_SeedCalcStaggerGroup == -1)
		SetSeedCalcStaggerLevel(Util::E_StaggerLevel_4Frame);

	return true;
}

bool C_ActiveRangeUpdateComponent::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_GameActorMsg_UpdateSeedPosition:
			{
				const ActorMsg::C_UpdateSeedPositionMsg *updateSeedPosMsg =  Messaging::MessageCast<ActorMsg::C_UpdateSeedPositionMsg>(pMessage);
				m_DistFromSeed = updateSeedPosMsg->GetPosition().Distance(GetActor()->GetPosition());

				bool inRange = true;

				if(m_WarmUpFrames == 0)
					inRange = (m_DistFromSeed < m_ActiveRange);
				else
					m_WarmUpFrames--;
	
				if(inRange)
				{
					// start update
					if(m_bOutOfRange == true)
					{
						SendMessageToActor(GameMessaging::E_GameMsg_UpdateEnable);
						m_bOutOfRange = false;
					}
				}
				else
				{
					// stop update
					if(m_bOutOfRange == false)
					{
						SendMessageToActor(GameMessaging::E_GameMsg_UpdateDisable);
						m_bOutOfRange = true;
					}
				}

				if(m_SeedCalcStaggerGroup == -1)
					SetSeedCalcStaggerLevel(Util::E_StaggerLevel_4Frame);
			}
			break;
		default:
			return C_UpdateManagerComponent::OnMessage(pMessage);
	}
	return true;
}

// set the stagger level for the seed distance calculator
void C_ActiveRangeUpdateComponent::SetSeedCalcStaggerLevel(Util::E_StaggerLevel level)
{
	Util::C_StaggerManager &staggerManager = GetActorList()->GetStaggerManager();
	staggerManager.FreeGroup(m_SeedCalcStaggerGroup);
	m_SeedCalcStaggerGroup = staggerManager.AllocGroup(level);
	U32 newMask = staggerManager.GetGroupMask(m_SeedCalcStaggerGroup);

	// 	Actor::C_SetSubscriberMaskMsg subscriberMaskMsg(ActorMsg::E_GameActorMsg_UpdateSeedPosition, newMask);
	// 
	// 	SendMessageToActor(&subscriberMaskMsg);

	SetSubscriberMask(ActorMsg::E_GameActorMsg_UpdateSeedPosition, newMask);
}

void	C_ActiveRangeUpdateComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_ActiveRange;
	stream << m_bOutOfRange;
	//stream << m_SeedCalcStaggerGroup;
}

void	C_ActiveRangeUpdateComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_ActiveRange;
	stream >> m_bOutOfRange;
	//stream >> m_SeedCalcStaggerGroup;
}

// from Component::I_GameComponent
void	C_ActiveRangeUpdateComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_UpdateManagerComponent::SerialiseForSave(stream);
	Serialise(stream);
}

void	C_ActiveRangeUpdateComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_UpdateManagerComponent::DeserialiseForSave(stream);
	Deserialise(stream);
}

void	C_ActiveRangeUpdateComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_UpdateManagerComponent::SerialiseForSync(stream);
	Serialise(stream);
}
void	C_ActiveRangeUpdateComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_UpdateManagerComponent::DeserialiseForSync(stream);
	Deserialise(stream);
}

// Stagger Update Component
// ------------------------

// Staggers actor update based on range from the seed
C_StaggerUpdateComponent::~C_StaggerUpdateComponent()
{
	GetActorList()->GetStaggerManager().FreeGroup(m_UpdateStaggerGroup);
}

// Create the component from setup parameters
bool C_StaggerUpdateComponent::CreateImpl()
{
	if(!C_ActiveRangeUpdateComponent::CreateImpl())
		return false;

	/*const C_StaggerUpdateComponentCreator *pStaggerCreator = GetCreationData();
	m_ActiveRange = pStaggerCreator->m_ActiveRange;

	AddSubscriber(ActorMsg::E_GameActorMsg_UpdateSeedPosition);*/
	return true;
}



// set the stagger level for the update subscriber
void C_StaggerUpdateComponent::SetUpdateStaggerLevel(Util::E_StaggerLevel level)
{
	Actor::C_ActorList *pActorList = GetActorList();
	pActorList->GetStaggerManager().FreeGroup(m_UpdateStaggerGroup);
	m_UpdateStaggerGroup = pActorList->GetStaggerManager().AllocGroup(level);
	U32 newMask = pActorList->GetStaggerManager().GetGroupMask(m_UpdateStaggerGroup);

	const Actor::C_SetUpdateMaskMsg message(newMask);
	SendMessageToActor(&message);
}


// Handle stagger based update messages
bool C_StaggerUpdateComponent::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_UpdateSeedPosition:
		{
			const ActorMsg::C_UpdateSeedPositionMsg *updateSeedPosMsg =  Messaging::MessageCast<ActorMsg::C_UpdateSeedPositionMsg>(pMessage);
			m_DistFromSeed = updateSeedPosMsg->GetPosition().Distance(GetActor()->GetPosition());

			bool inRange = true;

			if(m_WarmUpFrames == 0)
				inRange = (m_DistFromSeed < m_ActiveRange);
			else
				m_WarmUpFrames--;

			if(inRange)
			{
				SetUpdateStaggerLevel((Util::E_StaggerLevel)GetStaggerLevelFromDistance(m_DistFromSeed,m_ActiveRange));

				// start update
				if(m_bOutOfRange == true)
				{
					SendMessageToActor(GameMessaging::E_GameMsg_UpdateEnable);
					m_bOutOfRange = false;
				}

			}
			else
			{
				// stop update
				if(m_bOutOfRange == false)
				{
					SendMessageToActor(GameMessaging::E_GameMsg_UpdateDisable);
					m_bOutOfRange = true;
				}
			}

			if(m_SeedCalcStaggerGroup == -1)
				SetSeedCalcStaggerLevel(Util::E_StaggerLevel_4Frame);
		}
		break;
	default:
		return C_UpdateManagerComponent::OnMessage(pMessage);
	}
	return true;
}

// from Component::I_GameComponent
void C_StaggerUpdateComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_ActiveRangeUpdateComponent::SerialiseForSave(stream);
	//stream << m_UpdateStaggerGroup;
}

void C_StaggerUpdateComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_ActiveRangeUpdateComponent::DeserialiseForSave(stream);
	//stream >> m_UpdateStaggerGroup;
}

void C_StaggerUpdateComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_ActiveRangeUpdateComponent::SerialiseForSync(stream);
	//stream << m_UpdateStaggerGroup;
}

void C_StaggerUpdateComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_ActiveRangeUpdateComponent::DeserialiseForSync(stream);
	//stream >> m_UpdateStaggerGroup;
}

// Counter Update Component
// ------------------------

// Updates based on how many objects of the same group ID are between it and the seed

bool C_CounterUpdateComponent::InitialiseImpl()
{
	const C_CounterUpdateComponentCreator *pCounterComponentCreator = GetCreationData();

	m_UpdateOrderLimit = pCounterComponentCreator->m_UpdateOrderLimit;
	m_GroupId = pCounterComponentCreator->m_GroupID;
	return true;
}

bool C_CounterUpdateComponent::CreateImpl()
{
	m_WarmUpFrames = g_dbUpdateWarmupFrames;
	AddSubscriber(ActorMsg::E_GameActorMsg_SeedRequestPosition);
	return true;
}

// Handle counter based update manager messages
bool C_CounterUpdateComponent::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
		// service request from seed for position
		case ActorMsg::E_GameActorMsg_SeedRequestPosition:
		{
			const ActorMsg::C_SeedRequestPositionMsg* pSeedRequestPositionMsg = Messaging::MessageCast<ActorMsg::C_SeedRequestPositionMsg>(pMessage);
			const ActorMsg::C_CountedActorPositionMsg message(m_GroupId, GetActor()->GetPosition(), GetActorHandle());
			SendMessageToActor(pSeedRequestPositionMsg->m_SeedActor, &message);
			m_DistFromSeed = GetActorList()->GetActorPtr(pSeedRequestPositionMsg->m_SeedActor)->GetPosition().Distance(GetActor()->GetPosition());
		}
		break;

		// received counted actor position
		case ActorMsg::E_GameActorMsg_CountedActorOrder:
		{
			const ActorMsg::C_CountedActorOrderMsg* pOrderMsg = Messaging::MessageCast<ActorMsg::C_CountedActorOrderMsg>(pMessage);

			// Base update on order
			bool bOutOfRange = false;

			if(m_WarmUpFrames == 0)
				bOutOfRange = (pOrderMsg->m_Order >= m_UpdateOrderLimit);
			else
				m_WarmUpFrames--;

			if(bOutOfRange!=m_bOutOfRange)
			{
				SendMessageToActor(bOutOfRange ? GameMessaging::E_GameMsg_UpdateDisable : GameMessaging::E_GameMsg_UpdateEnable);
				m_bOutOfRange = bOutOfRange;
			}
		}
		break;

	default:
		return C_UpdateManagerComponent::OnMessage(pMessage);
	}
	return true;
}

void C_CounterUpdateComponent::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_UpdateOrderLimit;
	stream << m_GroupId;
}

void C_CounterUpdateComponent::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_UpdateOrderLimit;
	stream >> m_GroupId;
}

// from Component::I_GameComponent
void C_CounterUpdateComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_UpdateManagerComponent::SerialiseForSave(stream);
	Serialise(stream);
}

void C_CounterUpdateComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_UpdateManagerComponent::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_CounterUpdateComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_UpdateManagerComponent::SerialiseForSync(stream);
	Serialise(stream);
}

void C_CounterUpdateComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_UpdateManagerComponent::DeserialiseForSync(stream);
	Deserialise(stream);
}
// Update Seed Component
// ---------------------

C_UpdateSeedSingularComponent * C_UpdateSeedSingularComponent::m_pSingleInstance = NULL;

// Updates based on how many objects of the same group ID are between it and the seed

C_UpdateSeedSingularComponent::C_UpdateSeedSingularComponent() :
	m_pCountedItemListMap(new C_CountedItemListMap),m_FrameCounter(0)
{
	// intentionally blank
}

C_UpdateSeedSingularComponent::~C_UpdateSeedSingularComponent()
{
	delete m_pCountedItemListMap;
}

// Create the component
bool C_UpdateSeedSingularComponent::CreateImpl()
{
	AddSubscriber(GameMessaging::E_GameMsg_Update);
	AddSubscriber(ActorMsg::E_GameActorMsg_Update2ndPass);
	return true;
}

// Handler all update seed related messages
bool C_UpdateSeedSingularComponent::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		{
			// Broadcast position
			const ActorMsg::C_UpdateSeedPositionMsg positionMsg(GetActor()->GetPosition());
			GetActorList()->BroadcastMessage(&positionMsg, 1<< (++m_FrameCounter & 31));
			// broadcast position request
			const ActorMsg::C_SeedRequestPositionMsg requestMsg(GetActorHandle());
			GetActorList()->BroadcastMessage(&requestMsg);
		}
		break;

		// receive a counted actors position
	case ActorMsg::E_GameActorMsg_CountedActorPosition:
		{
			const ActorMsg::C_CountedActorPositionMsg *countedActorPosMsg =  Messaging::MessageCast<ActorMsg::C_CountedActorPositionMsg>(pMessage);
			//F32 activeDistance = countedActorPosMsg->GetPosition().Distance(GetActor()->GetPosition());
			F32 activeDistance = GetActorList()->GetActorPtr(countedActorPosMsg->m_ActorHandle)->GetPosition().Distance(GetActor()->GetPosition());
			Actor::C_ActorDistanceItem::C_List &list = (*m_pCountedItemListMap)[countedActorPosMsg->m_GroupId];

			// insert item into list
			list.PushBack(Actor::C_ActorDistanceItem(activeDistance, countedActorPosMsg->m_ActorHandle));
		}
		break;
		// sort and send the update order
	case ActorMsg::E_GameActorMsg_Update2ndPass:
		{
			C_CountedItemListMap::Iterator it = m_pCountedItemListMap->Begin();
			C_CountedItemListMap::Iterator end = m_pCountedItemListMap->End();

			// sort list & send out order information if needed
			while(it != end)
			{
				Actor::C_ActorDistanceItem::C_List &list = it->GetValue();

				std::sort(list.Begin(),list.End());

				for(S32 actorIndex = 0; actorIndex < list.Size(); actorIndex++)
				{
					const ActorMsg::C_CountedActorOrderMsg message(actorIndex);
					SendMessageToActor(list[actorIndex].m_ActorHandle, &message);
				}

				list.Clear();	// empty the list for next time

				++it;
			}

		}
		break;

	default:
		return I_UpdateSeedComponent::OnMessage(pMessage);
	}
	return true;
}

#ifndef CFG_GOLD
void C_UpdateSeedSingularComponent::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if(g_dbUpdateSeedDebugRender == false)
		return;

	Maths::HVec2 pos(50,50);
	U32 groupNo=0;

	debugRenderPtr->Draw2dText(pos, Util::C_String("UpdateSeedDebug"));
	pos += Maths::HVec2(0,20);

	for(C_CountedItemListMap::Iterator it = m_pCountedItemListMap->Begin(); it != m_pCountedItemListMap->End();++it)
	{
		Actor::C_ActorDistanceItem::C_List &list = it->GetValue();

		debugRenderPtr->Draw2dText(pos, Util::C_String().Printf("Group %d size = %d",groupNo,list.Size()));	// TODO: list will always be empty!
		pos += Maths::HVec2(0,20);
		groupNo++;
	}

}
#endif

}

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize( "", on ) 
#endif


