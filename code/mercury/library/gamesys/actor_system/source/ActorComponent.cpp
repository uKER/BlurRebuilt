#include "ActorSystemPch.h"

#include <actor_system/ActorFactory.h>
#include <actor_system/ActorPerf.h>
#include <actor_system/ActorSys.h>
#include <actor_system/GameComponentFactory.h>
#include <misc_shared/messaging/GameMessaging.h>


namespace Actor
{

/**
@class I_ActorComponent
@ingroup actor_group_component
*/

I_ActorComponent::I_ActorComponent()
	: Component::I_GameComponent()
	, m_ActorHandle()
	, m_pActorList(NULL)
{
}

I_ActorComponent::~I_ActorComponent()
{
}

/**
@returns the actor containing this component.
*/
const C_Actor* I_ActorComponent::GetActor(void) const 
{
	const C_Actor* actor = m_pActorList->GetActorPtr(m_ActorHandle);
	BIZ_ASSERT(actor && DBSTR " NULL actor - failed to allocate handle?");
	return actor;
}

/**
@brief Gets an interface from our actor.
If our actor contains any component supporting this interface then we will
return that interface.
@param interfaceID component interface type identifier.
@returns NULL if the interface is not supported.
*/
const I_ActorComponent* I_ActorComponent::QueryInterface(S32 interfaceID) const
{
	return m_pActorList->QueryInterface(m_ActorHandle, interfaceID);
}

/**
@brief Sends the message to another actor.
@param dest actor to receive the message.
@param pMessage message to send, must not be NULL.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
E_ActorMessageSendResult I_ActorComponent::SendMessageToActor(const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, S32 msgQueue) const
{
	return m_pActorList->SendMessageToActor(dest, pMessage, msgQueue);
}

/**
@brief Sends the message to another component owner by our actor.
@param component component interface type identifier.
@param pMessage message to send, must not be NULL.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
E_ActorMessageSendResult I_ActorComponent::SendMessageToComponent(const S32 component, const Messaging::C_MessageBase* pMessage, S32 msgQueue) const
{
	return m_pActorList->SendMessageToActorComponent(m_ActorHandle, component, pMessage, msgQueue);
}

/**
Derived actor components may override this method and should forward any
unhandled messages to their direct base class.
*/
bool I_ActorComponent::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_DebugRenderSpecific:
#ifndef CFG_GOLD
		DebugRender(Messaging::MessageCast<GameMessaging::C_DebugRenderSpecificMsg>(pMessage)->GetValue());
#endif
		return true;

	case GameMessaging::E_GameMsg_DebugRenderOn:
		AddSubscriber(GameMessaging::E_GameMsg_DebugRenderSpecific);
		return true;

	case GameMessaging::E_GameMsg_DebugRenderOff:
		RemoveSubscriber(GameMessaging::E_GameMsg_DebugRenderSpecific);
		return true;

	default:
		return false;
	}
}

bool I_ActorComponent::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetActor()->GetMemoryTracker());
#endif

	COMPONENT_PERF_EVENT_BEGIN(pMessage->Type(), this);

	const bool bResult = OnMessage(pMessage);

	COMPONENT_PERF_EVENT_END();

	return bResult;
}

/**
@brief Requests the specified context.
@param contextId context to request.
@returns NULL if the context is not supported.
*/
const Sys::I_Anything* I_ActorComponent::GetContext(const Sys::C_AnythingUid contextId) const
{
	return GetActor()->GetContext(contextId);
}

void I_ActorComponent::SetPrimed()
{
	Component::I_GameComponent::SetPrimed();
	const C_ComponentPrimedMsg message(GetInterfaceID());
	SendMessageToActor(&message);
}

/**
@returns message type list that this component wants from actors, may be NULL.
*/
const Messaging::I_MessageTypeList*	I_ActorComponent::ClassActorMessageTypeList()
{
	return NULL;
}

/**
@returns message type list that this component wants from actors, may be NULL.
*/
const Messaging::I_MessageTypeList* I_ActorComponent::GetActorMessageTypeList() const
{
	return NULL;
}

#ifndef CFG_SPEED

void I_ActorComponent::DebugDumpMessageHandler(Debug::Logger& log, const char* prefix) const
{
	LOGINFO(log, "%sComponent: 0x%8x    %s    %s    %s", prefix, GetComponentTypeID(), Component::C_GameComponentFactory::GetInstance()->GetComponentIFName(GetInterfaceID()).Chars(), GetActorList()->GetActorFactory()->LookupName(GetActor()->GetActorTypeId()).Chars(), GetActor()->GetActorName());
}

#endif

} // namespace Actor
