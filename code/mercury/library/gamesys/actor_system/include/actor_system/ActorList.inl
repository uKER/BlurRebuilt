#ifndef ACTORLIST_INL
#error ActorList.inl should only be included from ActorList.h
#endif

#include <misc_shared/Messaging/MessageBase.h>
#include <actor_system/ActorSys.h>

namespace Actor
{
	
/**
@brief Sends a message to the specified actor.
@param dest actor to receive the message.
@param pMessage message to send.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult C_ActorList::SendMessageToActor(const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, S32 msgQueue)
{
	return SendMessageToActorComponent(dest, -1, pMessage, msgQueue);
}

/**
@brief Sends a message to the specified actor.
@param dest actor to receive the message.
@param msgType type of message, must not carry additional data.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult C_ActorList::SendMessageToActor(const C_ActorHandle dest, Messaging::TMsgType msgType, S32 msgQueue)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToActor(dest, &message, msgQueue);
}

/**
@brief Sends a message to a specific component of an actor.
@param dest actor to receive the message.
@param component component interface type identifier within actor.
@param msgType type of message, must not carry additional data.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult C_ActorList::SendMessageToActorComponent(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, Messaging::TMsgType msgType, S32 msgQueue)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToActorComponent(dest, component, &message, msgQueue);
}

/**
@brief Sends a message to all actors subscribed to the message type.
@param type type of message, must not carry additional data.
@param mask bitmask to filter the message with.
*/
inline void C_ActorList::BroadcastMessage(Messaging::TMsgType type, U32 mask)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(type));
	BroadcastMessage(&message, mask);
}

/**
@brief Delays sending the message for a number of frames.
@param numFrames number of frames to delay message for.
@param dest actor to receive the message.
@param pMessage message to send.
@param msgQueue user-defined message queue index.
*/
inline void C_ActorList::DelayMessageToActorByFrames(const U32 numFrames, const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, const S32 msgQueue)
{
	DelayMessageToActorComponentByFrames(numFrames, dest, -1, pMessage, msgQueue);
}

/**
@brief Delays sending the message for a number of frames.
@param numFrames number of frames to delay message for.
@param dest actor to receive the message.
@param msgType type of message, must not carry additional data.
@param msgQueue user-defined message queue index.
*/
inline void C_ActorList::DelayMessageToActorByFrames(const U32 numFrames, const C_ActorHandle dest, const Messaging::TMsgType msgType, const S32 msgQueue)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	DelayMessageToActorByFrames(numFrames, dest, &message, msgQueue);
}

/**
@brief Delays sending the message for a number of frames.
@param numFrames number of frames to delay message for.
@param dest actor to receive the message.
@param component component interface type identifier within actor.
@param msgType type of message, must not carry additional data.
@param msgQueue user-defined message queue index.
*/
inline void C_ActorList::DelayMessageToActorComponentByFrames(const U32 numFrames, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::TMsgType msgType, const S32 msgQueue)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	DelayMessageToActorComponentByFrames(numFrames, dest, component, &message, msgQueue);
}

/**
@brief Delays sending the message for a number of frames.
@param numFrames number of frames to delay message for.
@param msgType type of message, must not carry additional data.
@param mask bitmask to filter the message with.
*/
inline void C_ActorList::DelayMessageBroadcastByFrames(const U32 numFrames, const Messaging::TMsgType msgType, const U32 mask)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	DelayMessageBroadcastByFrames(numFrames, &message, mask);
}

/**
@brief Delays sending the message for a number of seconds.
@param numSeconds number of seconds to delay message for.
@param dest actor to receive the message.
@param pMessage message to send.
@param msgQueue user-defined message queue index.
*/
inline void C_ActorList::DelayMessageToActorBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, const S32 msgQueue)
{
	DelayMessageToActorComponentBySeconds(numSeconds, dest, -1, pMessage, msgQueue);
}

/**
@brief Delays sending the message for a number of seconds.
@param numSeconds number of seconds to delay message for.
@param dest actor to receive the message.
@param msgType type of message, must not carry additional data.
@param msgQueue user-defined message queue index.
*/
inline void C_ActorList::DelayMessageToActorBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Messaging::TMsgType msgType, const S32 msgQueue)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	DelayMessageToActorBySeconds(numSeconds, dest, &message, msgQueue);
}

/**
@brief Delays sending the message for a number of seconds.
@param numSeconds number of seconds to delay message for.
@param dest actor to receive the message.
@param component component interface type identifier within actor.
@param msgType type of message, must not carry additional data.
@param msgQueue user-defined message queue index.
*/
inline void C_ActorList::DelayMessageToActorComponentBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::TMsgType msgType, const S32 msgQueue)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	DelayMessageToActorComponentBySeconds(numSeconds, dest, component, &message, msgQueue);
}

/**
@brief Delays sending the message for a number of seconds.
This does not go through the subscribers so all actors must handle it.
@param numSeconds number of seconds to delay message for.
@param msgType type of message, must not carry additional data.
@param mask bitmask to filter the message with.
*/
inline void C_ActorList::DelayMessageBroadcastBySeconds(const F32 numSeconds, const Messaging::TMsgType msgType, const U32 mask)
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	DelayMessageBroadcastBySeconds(numSeconds, &message, mask);
}

/**
@brief Gets the actor referred to by this handle.
@param handle handle to use.
@returns actor referred to by handle, or NULL if the handle is not valid.
*/
FORCE_INLINE const C_Actor* C_ActorList::GetActorPtr(C_ActorHandle handle) const 
{
	return m_pHandleManager->GetItemPtr(handle);
} 

} // namespace Actor
