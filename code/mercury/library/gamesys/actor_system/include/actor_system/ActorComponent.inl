#ifndef ACTORCOMPONENT_INL
#error ActorComponent.inl should only be included from ActorComponent.h
#endif

#include <misc_shared/Messaging/MessageBase.h>


namespace Actor
{

/**
@brief Sends the message to the actor owning this component.
@param pMessage message to send, must not be NULL.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult I_ActorComponent::SendMessageToActor(const Messaging::C_MessageBase* pMessage, S32 msgQueue) const
{
	return SendMessageToActor(m_ActorHandle, pMessage, msgQueue);
}

inline E_ActorMessageSendResult I_ActorComponent::SendMessageToActor(const C_ActorHandle dest, Messaging::TMsgType msgType, S32 msgQueue) const
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToActor(dest, &message, msgQueue);
}

/**
@brief Sends the message to the actor owning this component.
@param msgType type identifier for message, must not need additional data.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult I_ActorComponent::SendMessageToActor(Messaging::TMsgType msgType, S32 msgQueue) const
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToActor(&message, msgQueue);
}

/**
@brief Sends the message to another component of the actor owning this one.
@param component type identifier of component interface to send message to.
@param msgType type identifier of message, must not need additional data.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult I_ActorComponent::SendMessageToComponent(const S32 component, Messaging::TMsgType msgType, S32 msgQueue) const
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToComponent(component, &message, msgQueue);
}

} // namespace Actor
