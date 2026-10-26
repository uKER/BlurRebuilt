#include "ActorSystemPch.h"

#include <actor_system/ActorMessaging.h>

#include <actor_system/ActorSys.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace Actor
{

/**
Makes a copy of this message.
@param pMessage message to send.
*/
void C_ActorMessageStoreBase::SetMessage(const Messaging::C_MessageBase* pMessage)
{
	if (pMessage != GetMessage())
	{
		Messaging::C_DefaultMessageAllocator messageAllocator;
		messageAllocator.Destroy(m_pMessage);

		if (pMessage != NULL)
			m_pMessage = messageAllocator.Copy(pMessage);
		else
			m_pMessage = NULL;
	}
}
	
void C_ActorMessageStoreBase::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Dest;
	stream << m_ComponentOrMask;
	Messaging::C_MessageSerialiser::Serialise(stream, m_pMessage);
}

void C_ActorMessageStoreBase::Deserialise(Sys::C_ContextualStream& stream)
{
	Messaging::C_DefaultMessageAllocator messageAllocator;
	messageAllocator.Destroy(m_pMessage);

	stream >> m_Dest;
	stream >> m_ComponentOrMask;
	m_pMessage = Messaging::C_MessageSerialiser::Deserialise(stream, messageAllocator);
}





C_DeferredActorMessageQueue::C_DeferredActorMessageQueue(C_DeferredActorMessagePool& messagePool) : 
	m_MessagePool(messagePool) 
{
	// intentionally blank
}

C_DeferredActorMessageQueue::~C_DeferredActorMessageQueue
(
)
{
	// TODO: [Oscar] are we sure it's ok to ignore these messages, or should we enforce that our owner processes all messages before killing up?
	Clear();
}

/**
@brief Queues a message to send to an actor or component.
@param dest actor to send to.
@param component type identifier of component interface to send to, or -1 for actor.
@param pMessage message to send.
@returns true if the message was queued.
*/
const bool C_DeferredActorMessageQueue::PushMessage(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase *pMessage)
{
	if ((static_cast<U32>(dest) != 0) && (pMessage != NULL))
	{
		C_DeferredActorMessageStore* pStore = m_MessagePool.Allocate();
		pStore->Init(dest,component,pMessage);
		m_MessageFIFO.PushBack(*pStore);
		return true;
	}

	return false;
}

/**
@brief Queues a message to broadcast to all actors.
@param mask bitmask to restrict message handling.
@param pMessage message to send.
@returns true if the message was queued.
*/
const bool C_DeferredActorMessageQueue::BroadcastMessage(const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	if (pMessage != NULL)
	{
		C_DeferredActorMessageStore* pStore = m_MessagePool.Allocate();
		pStore->Init(mask, pMessage);
		m_MessageFIFO.PushBack(*pStore);
		return true;
	}

	return false;
}

/**
@brief Dispatches each queued message in the same order they were added.
@param pActorList actor list to dispatch message with, must not be NULL.
@returns the number of messages dispatched.
*/
const U32 C_DeferredActorMessageQueue::ProcessMessages(C_ActorList* pActorList)
{
	BIZ_ASSERT(pActorList != NULL);

	U32 noMessages;
	for (noMessages = 0; !m_MessageFIFO.Empty(); ++noMessages)
	{
		C_DeferredActorMessageStore* pStore = &m_MessageFIFO.Front();
		m_MessageFIFO.PopFront();

		if(pStore->IsBroadcast())
			pActorList->BroadcastMessageNow(pStore->GetBroadcastMask(), pStore->GetMessage());
		else
			pActorList->SendMessageNow(pStore->GetDestActor(), pStore->GetDestComponent(), pStore->GetMessage());

		m_MessagePool.Free(pStore);
	}

	return noMessages;
}

void C_DeferredActorMessageQueue::Clear()
{
	while (!m_MessageFIFO.Empty())
	{
		C_DeferredActorMessageStore* pStore = &m_MessageFIFO.Front();
		m_MessageFIFO.PopFront();
		m_MessagePool.Free(pStore);
	}
}
	
void C_DeferredActorMessageQueue::Serialise(Sys::C_ContextualStream& stream) const
{
	const U32 numMessages = m_MessageFIFO.Size();
	stream << numMessages;

	for (C_DeferredActorMessageList::ConstIterator it = m_MessageFIFO.Begin(); it != m_MessageFIFO.End(); ++it)
		stream << *it;
}

void C_DeferredActorMessageQueue::Deserialise(Sys::C_ContextualStream& stream)
{
	Clear();

	U32 numMessages = 0;
	stream >> numMessages;

	for (U32 i = 0; i < numMessages; ++i)
	{
		C_DeferredActorMessageStore* pStore = m_MessagePool.Allocate();
		stream >> *pStore;
		m_MessageFIFO.PushBack(*pStore);
	}
}





/**
@param noMessageQueues number of message queues needed.
*/
C_DeferredActorMessages::C_DeferredActorMessages(const U32 noMessageQueues)
{
	Init(noMessageQueues);
}

C_DeferredActorMessages::~C_DeferredActorMessages()
{
	Clear();
}

void C_DeferredActorMessages::Init(const U32 noMessageQueues)
{
	Clear();

	m_MessageQueues.Resize(noMessageQueues);

	for (U32 msgQueueNo = 0; msgQueueNo < noMessageQueues; ++msgQueueNo)
		m_MessageQueues[msgQueueNo] = new C_DeferredActorMessageQueue(m_MessagePool);
}

void C_DeferredActorMessages::Clear()
{
	for (S32 i = 0; i < m_MessageQueues.Size(); ++i)
		delete m_MessageQueues[i];

	m_MessageQueues.Resize(0);
}

void C_DeferredActorMessages::Serialise(Sys::C_ContextualStream& stream) const
{
	const U32 numQueues = m_MessageQueues.Size();
	stream << numQueues;

	for (Btl::Vector<C_DeferredActorMessageQueue*>::ConstIterator it = m_MessageQueues.Begin(); it != m_MessageQueues.End(); ++it)
		stream << **it;
}

void C_DeferredActorMessages::Deserialise(Sys::C_ContextualStream& stream)
{
	U32 numQueues = 9;
	stream >> numQueues;

	Init(numQueues);

	for (U32 i = 0; i < numQueues; ++i)
		stream >> *m_MessageQueues[i];
}





	
void C_DelayedActorMessageStore::Serialise(Sys::C_ContextualStream& stream) const
{
	C_ActorMessageStoreBase::Serialise(stream);
	stream << m_TimeStamp;
	stream << m_MessageQueue;
}

void C_DelayedActorMessageStore::Deserialise(Sys::C_ContextualStream& stream)
{
	C_ActorMessageStoreBase::Deserialise(stream);
	stream >> m_TimeStamp;
	stream >> m_MessageQueue;
}





/**
@param messagePool allocates message storage.
*/
C_DelayedActorMessageQueue::C_DelayedActorMessageQueue(C_DelayedActorMessagePool& messagePool) :
	m_TimeStamp(0), 
	m_MessagePool(messagePool)
{
	// intentionally blank
}

C_DelayedActorMessageQueue::~C_DelayedActorMessageQueue()
{
	// TODO: [Oscar] are we sure it's ok to ignore these messages, or should we enforce that our owner processes all messages before killing up?
	Clear();
}

/**
@brief Delays the message for the specified number of units.
@param delay number of units to delay for.
@param messageQueue message queue to send message to.
@param dest actor to send message to.
@param component component interface type identifier to send message to, -1 for actor.
@param pMessage message to send.
@returns true if the message was queued.
*/
const bool C_DelayedActorMessageQueue::PushMessage(const U32 delay, const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage
)
{
	if ((static_cast<U32>(dest) != 0) && (pMessage != NULL))
	{
		C_DelayedActorMessageStore* pStore = m_MessagePool.Allocate();
		pStore->Init(dest, component, pMessage, m_TimeStamp + delay, messageQueue);
		m_MessageList.PushBack(*pStore);
		return true;
	}

	return false;
}

/**
@brief Delays the message for the specified number of units.
@param delay number of units to delay for.
@param mask bitmask to filter message with.
@param pMessage message to send.
@returns true if the message was queued.
*/
const bool C_DelayedActorMessageQueue::BroadcastMessage(const U32 delay, const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	if (pMessage != NULL)
	{
		C_DelayedActorMessageStore* pStore = m_MessagePool.Allocate();
		pStore->Init(mask, pMessage, m_TimeStamp + delay);
		m_MessageList.PushBack(*pStore);
		return true;
	}

	return false;
}

/**
@brief Sends any messages that have reached the delay time.
@param delta number of time units passed.
@param pActorList sends messages to actors.
*/
void C_DelayedActorMessageQueue::Update(const U32 delta, C_ActorList* pActorList)
{
	C_DelayedActorMessageList messagesToSend;

	// rip expired messages off the queue
	C_DelayedActorMessageList::Iterator itQueue = m_MessageList.Begin();
	while (itQueue != m_MessageList.End())
	{
		const U32 messageDelta = itQueue->GetTimeStamp() - m_TimeStamp;

		if (messageDelta <= delta)
		{
			C_DelayedActorMessageStore* pStore = &(*itQueue);

			// remove the message from the queue
			itQueue = m_MessageList.Erase(itQueue);

			// record the delta
			pStore->SetTimeStamp(messageDelta);

			// add the message to the send list in sorted order
			C_DelayedActorMessageList::Iterator itSend;

			for (itSend = messagesToSend.Begin(); itSend != messagesToSend.End(); ++itSend)
			{
				if (messageDelta < itSend->GetTimeStamp())
				{
					break;
				}
			}

			if (itSend != messagesToSend.End())
				messagesToSend.InsertBefore(*pStore, itSend);
			else
				messagesToSend.PushBack(*pStore);
		}
		else
		{
			++itQueue;
		}
	}

	// send the messages that have expired (they're already sorted)
	while (!messagesToSend.Empty())
	{
		C_DelayedActorMessageStore* pStore = &messagesToSend.Front();
		messagesToSend.PopFront();
		
		if (pStore->IsBroadcast())
			pActorList->BroadcastMessage(pStore->GetMessage(), pStore->GetBroadcastMask());
		else
			pActorList->SendMessageToActorComponent(pStore->GetDestActor(), pStore->GetDestComponent(), pStore->GetMessage(), pStore->GetMessageQueue());

		m_MessagePool.Free(pStore);
	}

	// update timestamp
	m_TimeStamp = m_TimeStamp + delta;
}

void C_DelayedActorMessageQueue::Clear()
{
	while (!m_MessageList.Empty())
	{
		C_DelayedActorMessageStore* pStore = &m_MessageList.Front();
		m_MessageList.PopFront();
		m_MessagePool.Free(pStore);
	}
}

void C_DelayedActorMessageQueue::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_TimeStamp;

	const U32 numMessages = m_MessageList.Size();
	stream << numMessages;

	for (C_DelayedActorMessageList::ConstIterator it = m_MessageList.Begin(); it != m_MessageList.End(); ++it)
		stream << *it;
}

void C_DelayedActorMessageQueue::Deserialise(Sys::C_ContextualStream& stream)
{
	Clear();

	stream >> m_TimeStamp;

	U32 numMessages = 0;
	stream >> numMessages;

	for (U32 i = 0; i < numMessages; ++i)
	{
		C_DelayedActorMessageStore* pStore = m_MessagePool.Allocate();
		stream >> *pStore;
		m_MessageList.PushBack(*pStore);
	}
}





C_DelayedActorMessages::C_DelayedActorMessages() :
	m_FrameQueue(m_MessagePool), 
	m_TimedQueue(m_MessagePool)
{
	// intentionally blank
}

C_DelayedActorMessages::~C_DelayedActorMessages()
{
	// intentionally blank
}

/**
@brief Sends any messages that have passed the delay period.
@param timeStep number of seconds passed.
@param pActorList sends messages to actors.
*/
void C_DelayedActorMessages::Update(const F32 timeStep, C_ActorList* pActorList)
{
	m_FrameQueue.Update(1, pActorList);
	m_TimedQueue.Update(SecondsToMilliseconds(timeStep), pActorList);
}

void C_DelayedActorMessages::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_FrameQueue;
	stream << m_TimedQueue;
}

void C_DelayedActorMessages::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_FrameQueue;
	stream >> m_TimedQueue;
}

} // namespace Actor
