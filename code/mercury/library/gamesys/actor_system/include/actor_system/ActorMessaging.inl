#ifndef ACTORMESSAGING_INL
#error ActorMessaging.inl should only be included from ActorMessaging.h
#endif

#include <limits>


namespace Actor
{

inline C_ActorMessageStoreBase::C_ActorMessageStoreBase() :
	m_Dest(), m_ComponentOrMask(0), m_pMessage(NULL)
{
	// intentionally blank
}

inline C_ActorMessageStoreBase::C_ActorMessageStoreBase(const C_ActorMessageStoreBase& copy) :
	m_Dest(copy.m_Dest), m_ComponentOrMask(copy.m_ComponentOrMask), m_pMessage(NULL)
{
	SetMessage(copy.GetMessage());
}

/**
@brief Stores a message for a single actor.
@param dest actor to send to, must not be the NULL handle.
@param component -1 to send to actor, or index to send to a specific component.
@param pMessage message to send.
*/
inline C_ActorMessageStoreBase::C_ActorMessageStoreBase(const C_ActorHandle& dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage) :
	m_Dest(dest), m_ComponentOrMask(static_cast<U32>(component)), m_pMessage(NULL)
{
	SetMessage(pMessage);
	BIZ_ASSERT(!IsBroadcast());
}

/**
@brief Stores a message for broadcast to all actors.
@param mask bitmask to limit message responses.
@param pMessage message to send.
*/
inline C_ActorMessageStoreBase::C_ActorMessageStoreBase(const U32 mask, const Messaging::C_MessageBase* pMessage) :
	m_Dest(), m_ComponentOrMask(mask), m_pMessage(NULL)
{
	SetMessage(pMessage);
	BIZ_ASSERT(IsBroadcast());
}

inline C_ActorMessageStoreBase::~C_ActorMessageStoreBase()
{
	if (m_pMessage != NULL)
		SetMessage(NULL);
}

inline C_ActorMessageStoreBase& C_ActorMessageStoreBase::operator = (const C_ActorMessageStoreBase& copy)
{
	m_Dest = copy.m_Dest;
	m_ComponentOrMask = copy.m_ComponentOrMask;
	SetMessage(copy.GetMessage());
	return *this;
}

/**
@brief Initialises the store with a message for a single actor.
@param dest actor to send to, must not be the NULL handle.
@param component -1 to send to actor, or index to send to a specific component.
@param pMessage message to send.
*/
inline void C_ActorMessageStoreBase::Init(const C_ActorHandle &dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase *pMessage)
{
	m_Dest = dest;
	m_ComponentOrMask = component;
	SetMessage(pMessage);
	BIZ_ASSERT(!IsBroadcast());
}

/**
@brief Initialises the store with a message for all actors.
@param mask bitmask to limit message responses.
@param pMessage message to send.
*/
inline void C_ActorMessageStoreBase::Init(const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	m_Dest = C_ActorHandle();
	m_ComponentOrMask = static_cast<S32>(mask);
	SetMessage(pMessage);
	BIZ_ASSERT(IsBroadcast());
}

/**
@returns true if this message is intended for all actors.
*/
inline const bool C_ActorMessageStoreBase::IsBroadcast() const 
{
	return (static_cast<U32>(m_Dest) == 0);
}

/**
@brief Only call this if IsBroadcast() returns true.
@returns bitmask to limit message responses.
*/
inline const U32 C_ActorMessageStoreBase::GetBroadcastMask() const
{
	BIZ_ASSERT(IsBroadcast());
	return static_cast<U32>(m_ComponentOrMask);
}

/**
@brief Only call this if IsBroadcast() returns false.
@returns actor to receive the message.
*/
inline const C_ActorHandle&	C_ActorMessageStoreBase::GetDestActor() const
{
	BIZ_ASSERT(!IsBroadcast());
	return m_Dest;
}

/**
@brief Only call this if IsBroadcast() returns false.
@returns index of component to receive the message, or -1 to send to actor.
*/
inline const Component::C_ComponentInterfaceId C_ActorMessageStoreBase::GetDestComponent() const
{
	BIZ_ASSERT(!IsBroadcast());
	return static_cast<Component::C_ComponentInterfaceId>(m_ComponentOrMask);
}

/**
@returns message to send.
*/
inline const Messaging::C_MessageBase* C_ActorMessageStoreBase::GetMessage() const 
{
	return m_pMessage;
}





/**
@brief Queues a message for an actor or component.
@param messageQueue index of the queue to insert the message into.
@param dest actor to receive the message.
@param component type identifier of component interface to receive the message, or -1 for actor.
@param pMessage message to send.
@returns true if the message was queued.
*/
inline const bool C_DeferredActorMessages::PushMessage(const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage)
{
	return m_MessageQueues[messageQueue]->PushMessage(dest, component, pMessage);
}

/**
@brief Queues a message for all actors.
@param messageQueue index of the queue to insert the message into.
@param mask bitmask to restrict which actors handle the message.
@param pMessage message to send.
@returns true if the message was queued.
*/
inline const bool C_DeferredActorMessages::BroadcastMessage(const U32 messageQueue, const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	return m_MessageQueues[messageQueue]->BroadcastMessage(mask, pMessage);
}

/**
@brief Processes all message in this queue.
@param messageQueue index of the queue to process.
@param pActorList dispatches the messages, must not be NULL.
@returns the number of messages dispatched.
*/
inline const U32 C_DeferredActorMessages::ProcessMessages(const U32 messageQueue, C_ActorList* pActorList)
{
	return m_MessageQueues[messageQueue]->ProcessMessages(pActorList);
}





C_DelayedActorMessageStore::C_DelayedActorMessageStore() :
	m_TimeStamp(0), m_MessageQueue(0)
{
	// intentionally blank
}

/**
@brief Initialises the store with a message for a single actor.
@param dest actor to send to, must not be the NULL handle.
@param component -1 to send to actor, or index to send to a specific component.
@param pMessage message to send.
@param timeStamp when should the message be sent.
@param messageQueue which queue should the message be sent to.
*/
inline void C_DelayedActorMessageStore::Init(const C_ActorHandle &dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase *pMessage, const U32 timeStamp, const U32 messageQueue)
{
	C_ActorMessageStoreBase::Init(dest, component, pMessage);
	m_TimeStamp = timeStamp;
	m_MessageQueue = messageQueue;
}

/**
@brief Initialises the store with a message for all actors.
@param mask bitmask to limit message responses.
@param pMessage message to send.
@param timeStamp when should the message be sent.
*/
inline void C_DelayedActorMessageStore::Init(const U32 mask, const Messaging::C_MessageBase* pMessage, const U32 timeStamp)
{
	C_ActorMessageStoreBase::Init(mask, pMessage);
	m_TimeStamp = timeStamp;
	m_MessageQueue = 0;
}





/**
@param seconds number of seconds.
@returns number of milliseconds.
*/
inline const U32 C_DelayedActorMessages::SecondsToMilliseconds(const F32 seconds)
{
	return static_cast<U32>((seconds * 1000.0f) + 0.5f);
}

/**
@brief Sends a message to the actor or component after a number of frames.
@param numFrames number of frames to delay for.
@param messageQueue queue to send message in.
@param dest actor to receive message.
@param component component interface type identifier to receive message, -1 for actor.
@param pMessage message to send.
@returns true if the message was queued.
*/
inline const bool C_DelayedActorMessages::DelayMessageByFrames(const U32 numFrames, const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage)
{
	return m_FrameQueue.PushMessage(numFrames, messageQueue, dest, component, pMessage);
}

/**
@brief Broadcasts a message after a number of frames.
@param numFrames number of frames to delay for.
@param mask bitmask to filter the message by.
@param pMessage message to send.
@returns true if the message was queued.
*/
inline const bool C_DelayedActorMessages::DelayBroadcastByFrames(const U32 numFrames, const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	return m_FrameQueue.BroadcastMessage(numFrames, mask, pMessage);
}

/**
@brief Sends a message to the actor or component after a number of seconds.
@param numSeconds number of frames to delay for.
@param messageQueue queue to send message in.
@param dest actor to receive message.
@param component component interface type identifier to receive message, -1 for actor.
@param pMessage message to send.
@returns true if the message was queued.
*/
inline const bool C_DelayedActorMessages::DelayMessageBySeconds(const F32 numSeconds, const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage)
{
	return m_TimedQueue.PushMessage(SecondsToMilliseconds(numSeconds), messageQueue, dest, component, pMessage);
}

/**
@brief Broadcasts a message after a number of seconds.
@param numSeconds number of frames to delay for.
@param mask bitmask to filter the message by.
@param pMessage message to send.
@returns true if the message was queued.
*/
inline const bool C_DelayedActorMessages::DelayBroadcastBySeconds(const F32 numSeconds, const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	return m_TimedQueue.BroadcastMessage(SecondsToMilliseconds(numSeconds), mask, pMessage);
}

} // namespace Actor
