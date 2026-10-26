#include "SharedActorsPch.h"

#include <shared_actors/components/impl/TrackingImpl.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <shared_actors/components/messages/TrackingMessages.h>
#include <shared_actors/SharedActorMessages.h>


namespace Components
{

C_SpecificActorTracking::C_SpecificActorTracking() : 
	I_ActorTracking(), 
	m_SpecificActor(), 
	m_IsTracking(false)
{
	// intentionally blank
}

C_SpecificActorTracking::~C_SpecificActorTracking()
{
	// intentionally blank
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_SpecificActorTracking)
	ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE(ActorMsg::E_TrackingMsg_Actor)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_Enable, ActorMsg::E_GameActorMsg_Disable)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_SpecificActorTracking, I_ActorTracking)

bool C_SpecificActorTracking::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case ActorMsg::E_TrackingMsg_Actor:
		SetSpecificActor(Messaging::MessageCast<ActorMsg::C_TrackingActorMsg>(pMessage)->GetActorHandle());
		return true;

	case GameMessaging::E_GameMsg_Update:
		OnUpdate();
		return true;

	case ActorMsg::E_GameActorMsg_Enable:
		if (m_IsTracking)
		{
			AddSubscriber(GameMessaging::E_GameMsg_Update);
		}
		return true;

	case ActorMsg::E_GameActorMsg_Disable:
		if (m_IsTracking)
		{
			RemoveSubscriber(GameMessaging::E_GameMsg_Update);
		}
		return true;

	default:
		return I_ActorTracking::OnMessage(pMessage);
	}
}
	
void C_SpecificActorTracking::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_SpecificActor;
	stream << m_IsTracking;
}

void C_SpecificActorTracking::Deserialise(Sys::C_ContextualStream& stream)
{
	Actor::C_ActorHandle specificActor;
	bool isTracking = false;

	stream >> specificActor;
	stream >> isTracking;

	if (isTracking)
		SetSpecificActor(specificActor);
}

void C_SpecificActorTracking::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_ActorTracking::SerialiseForSave(stream);
	Serialise(stream);
}

void C_SpecificActorTracking::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_ActorTracking::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_SpecificActorTracking::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_ActorTracking::SerialiseForSync(stream);
	Serialise(stream);
}

void C_SpecificActorTracking::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_ActorTracking::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_SpecificActorTracking::SetSpecificActor(const Actor::C_ActorHandle& actorHandle)
{
	m_SpecificActor = actorHandle;

	if (!m_IsTracking)
	{
		m_IsTracking = true;
		AddSubscriber(GameMessaging::E_GameMsg_Update);
	}
}

void C_SpecificActorTracking::OnUpdate()
{
	const Actor::C_Actor* pActor = GetActorList()->GetActorPtr(m_SpecificActor);

	if (pActor != NULL)
	{
		NotifyActorPosition(m_SpecificActor, pActor->GetPosition());
	}
	else	// Actor not there so don't track
	{
		m_IsTracking = false;
		RemoveSubscriber(GameMessaging::E_GameMsg_Update);

		// Handle entity leaving trigger
		NotifyActorLost();
	}
}

} // namespace Components
