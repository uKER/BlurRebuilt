#include "SharedActorsPch.h"

#include <shared_actors/components/interfaces/I_Trigger.h>

#include <actor_system/ActorList.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <shared_actors/components/messages/PhysicsMessages.h>
#include <shared_actors/components/messages/TrackingMessages.h>
#include <shared_actors/components/messages/TriggerMessages.h>
#include <shared_actors/SharedActorMessages.h>


namespace Components
{



/**
@param flags E_Flag bitfield.
*/
I_VolumeTrigger::I_VolumeTrigger(const U32 flags) : 
	Actor::I_ActorComponent(), 
	m_TrackedActors(), 
	m_Flags(flags)
{
	// intentionally blank
}

I_VolumeTrigger::~I_VolumeTrigger()
{
	// intentionally blank
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(I_VolumeTrigger)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_Activate, ActorMsg::E_GameActorMsg_Deactivate)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_TrackingMsg_Begin, ActorMsg::E_TrackingMsg_End)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(E_PhysicsTriggerNotification_Enter, E_PhysicsTriggerNotification_Exit)
ACTOR_COMPONENT_MESSAGE_TYPES_END(I_VolumeTrigger, Actor::I_ActorComponent)

bool I_VolumeTrigger::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case ActorMsg::E_TrackingMsg_Position:
		{
			const ActorMsg::C_TrackingPositionMsg *pActorPositionMsg = Messaging::MessageCast<ActorMsg::C_TrackingPositionMsg>(pMessage);
			OnActorPositionNotify(pActorPositionMsg->GetActorHandle(), pActorPositionMsg->GetValue());
		}
		break;

	case ActorMsg::E_TrackingMsg_Lost:
		OnActorLostNotify();
		break;

	case ActorMsg::E_GameActorMsg_Activate:
		SetFlag(E_Flag_Active);
		break;

	case ActorMsg::E_GameActorMsg_Deactivate:
		ClearFlag(E_Flag_Active);
		break;

	case E_PhysicsTriggerNotification_Enter:
		//if(TestFlag(E_Flag_ItemInTrigger) == false)
		{
			if(TestFlag(E_Flag_Active))
				PhysicsNotifyEnter(Messaging::MessageCast<C_PhysicsTriggerEnterMsg>(pMessage)->GetActorHandle());

			SetFlag(E_Flag_ItemInTrigger);
		}
		break;

	case E_PhysicsTriggerNotification_Exit:
		//if(TestFlag(E_Flag_ItemInTrigger) == true)
		{
			if(TestFlag(E_Flag_Active))
				PhysicsNotifyExit(Messaging::MessageCast<C_PhysicsTriggerExitMsg>(pMessage)->GetActorHandle());

			ClearFlag(E_Flag_ItemInTrigger);
		}
		break;

	default:
		return Actor::I_ActorComponent::OnMessage(pMessage);
	}

	return true;
}

void I_VolumeTrigger::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Flags;
}

void I_VolumeTrigger::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Flags;
}

void I_VolumeTrigger::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Actor::I_ActorComponent::SerialiseForSave(stream);

	// write tracked actors
	{
		const U32 numTrackedActors = m_TrackedActors.Size();
		stream << numTrackedActors;

		for (C_TrackedActors::ConstIterator it = m_TrackedActors.Begin(); it != m_TrackedActors.End(); ++it)
		{
			stream << it->GetKey();
			stream << it->GetValue().m_Position;
			stream << it->GetValue().m_InTrigger;
		}
	}

	Serialise(stream);
}

void I_VolumeTrigger::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Actor::I_ActorComponent::DeserialiseForSave(stream);

	// read tracked actors
	{
		U32 numTrackedActors = 0;
		stream >> numTrackedActors;

		for (U32 i = 0; i < numTrackedActors; ++i)
		{
			Actor::C_ActorHandle key;
			stream >> key;

			C_TrackedState& value = m_TrackedActors[key];
			stream >> value.m_Position;
			stream >> value.m_InTrigger;
		}
	}

	Deserialise(stream);
}

void I_VolumeTrigger::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Actor::I_ActorComponent::SerialiseForSync(stream);

	// tracked actors not serialised

	Serialise(stream);
}

void I_VolumeTrigger::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Actor::I_ActorComponent::DeserialiseForSync(stream);

	// tracked actors not serialised

	Deserialise(stream);
}

/**
@param actorHandle actor being updated.
@param position updated position of actor.
*/
void I_VolumeTrigger::OnActorPositionNotify(const Actor::C_ActorHandle& actorHandle, const Maths::C_Vertex& position)
{
	// get the state
	C_TrackedState* pState = NULL;
	{
		C_TrackedActors::Iterator it = m_TrackedActors.Find(actorHandle);

		if (it == m_TrackedActors.End())
		{
			pState = &m_TrackedActors[actorHandle];
			pState->m_Position = position;
			pState->m_InTrigger = false;
		}
		else
		{
			pState = &it->GetValue();
		}
	}

	// update the state
	if (TestFlag(E_Flag_Active))
	{
		if (IsInside(position))
		{
			if (!pState->m_InTrigger)
			{
				NotifyEnter(actorHandle);
			}
		}
		else if (pState->m_InTrigger)
		{
			NotifyExit(actorHandle);
		}
		else if (TestFlag(E_Flag_HandleCrossing))
		{
			if (CheckLineIntersection(pState->m_Position, position))
			{
				const ActorMsg::C_TriggerCrossedMsg triggerCrossedMsg(actorHandle);
				SendMessageToActor(&triggerCrossedMsg);
			}
		}
	}
	else if (pState->m_InTrigger)
	{
		NotifyExit(actorHandle);
	}

	// record the new position
	pState->m_Position = position;
}

void I_VolumeTrigger::OnActorLostNotify()
{
	Actor::C_ActorList* pActorList = GetActorList();

	C_TrackedActors::Iterator it = m_TrackedActors.Begin();
	while (it != m_TrackedActors.End())
	{
		if (!pActorList->ActorExists(it->GetKey()))
		{
			// check if we need to send an exit message
			if (it->GetValue().m_InTrigger)
			{
				NotifyExit(it->GetKey());
			}

			// remove the entry
			C_TrackedActors::Iterator next = it;
			++next;
			m_TrackedActors.Erase(it);
			it = next;
		}
		else
		{
			// actor is still present, skip to next one
			++it;
		}
	}
}

/**
@param actorHandle actor entering the trigger.
*/
void I_VolumeTrigger::NotifyEnter(const Actor::C_ActorHandle& actorHandle)
{
	BIZ_ASSERT(m_TrackedActors.Find(actorHandle) != m_TrackedActors.End());
	C_TrackedState& state = m_TrackedActors[actorHandle];
	BIZ_ASSERT(!state.m_InTrigger);

	// send the notification if it was requested
	if (TestFlag(E_Flag_HandleEnterExit))
	{
		// don't allow one shot triggers to handle 2 OnEnter's
		if (!(TestFlag(E_Flag_OneShot) && TestFlag(E_Flag_EnterHandled)))
		{
			const ActorMsg::C_TriggerEnterMsg message(actorHandle);
			SendMessageToActor(&message);
			SetFlag(E_Flag_EnterHandled);
		}
	}

	// clear the trigger state
	state.m_InTrigger = true;
}

/**
@param actorHandle actor exiting the trigger.
*/
void I_VolumeTrigger::NotifyExit(const Actor::C_ActorHandle& actorHandle)
{
	BIZ_ASSERT(m_TrackedActors.Find(actorHandle) != m_TrackedActors.End());
	C_TrackedState& state = m_TrackedActors[actorHandle];
	BIZ_ASSERT(state.m_InTrigger);

	// send the notification if it was requested
	if (TestFlag(E_Flag_HandleEnterExit))
	{
		const ActorMsg::C_TriggerExitMsg message(actorHandle);
		SendMessageToActor(&message);
	}

	// clear the trigger state
	state.m_InTrigger = false;

	// one shot triggers become inactive after the first exit
	if (TestFlag(E_Flag_OneShot))
	{
		ClearFlag(E_Flag_Active);
		ClearFlag(E_Flag_EnterHandled);	// reset enter handled state
	}
}

/**
@param actorHandle actor crossing the trigger.
*/
void I_VolumeTrigger::NotifyCrossed(const Actor::C_ActorHandle& actorHandle)
{
	// send the notification if it was requested
	if (TestFlag(E_Flag_HandleCrossing))
	{
		const ActorMsg::C_TriggerCrossedMsg message(actorHandle);
		SendMessageToActor(&message);
	}

	// one shot triggers become inactive after the first crossing
	if (TestFlag(E_Flag_OneShot))
	{
		SendMessageToComponent(E_TriggerIF_Volume, ActorMsg::E_GameActorMsg_Deactivate);
		//ClearFlag(E_Flag_Active);
	}

}

void I_VolumeTrigger::PhysicsNotifyEnter(const Actor::C_ActorHandle& actorHandle)
{
	// send the notification if it was requested
	if (TestFlag(E_Flag_HandleEnterExit))
	{
		// don't allow one shot triggers to handle 2 OnEnter's
		if (!(TestFlag(E_Flag_OneShot) && TestFlag(E_Flag_EnterHandled)))
		{
			const ActorMsg::C_TriggerEnterMsg message(actorHandle);
			SendMessageToActor(&message);
			SetFlag(E_Flag_EnterHandled);
		}
	}
}

void I_VolumeTrigger::PhysicsNotifyExit(const Actor::C_ActorHandle& actorHandle)
{
	// send the notification if it was requested
	if (TestFlag(E_Flag_HandleEnterExit))
	{
		const ActorMsg::C_TriggerExitMsg message(actorHandle);
		SendMessageToActor(&message);
	}

	// one shot triggers become inactive after the first exit
	if (TestFlag(E_Flag_OneShot))
	{
		SendMessageToComponent(E_TriggerIF_Volume, ActorMsg::E_GameActorMsg_Deactivate);
		//ClearFlag(E_Flag_Active);
		ClearFlag(E_Flag_EnterHandled);	// reset enter handled state
	}
}


} // namespace Components
