#ifndef ACTORMESSAGING_H
#define ACTORMESSAGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/Messaging/MessagingDeclare.h>

#include <Btl/Containers/list.h>
#include <Btl/Containers/vector.h>

namespace Sys { class C_ContextualStream; }


namespace Actor
{

/**
@brief Base class for deferred actor messages.

Stores details of the message destination along with a copy of the message
contents. The destination may be all actors or a single actor (possibly even
a single component within that actor). Use IsBroadcast() to determine whether
the destination should be obtained with GetBroadcastMask() or with 
GetDestActor() and GetDestComponent(). 
*/
class C_ActorMessageStoreBase
{
public:
	inline C_ActorMessageStoreBase();
	inline C_ActorMessageStoreBase(const C_ActorMessageStoreBase& copy);
	inline C_ActorMessageStoreBase(const C_ActorHandle& dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage);
	inline C_ActorMessageStoreBase(const U32 mask, const Messaging::C_MessageBase* pMessage);
	inline ~C_ActorMessageStoreBase();
	inline C_ActorMessageStoreBase& operator = (const C_ActorMessageStoreBase& copy);

	inline void Init(const C_ActorHandle &dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase *pMessage);
	inline void Init(const U32 mask, const Messaging::C_MessageBase* pMessage);

	inline const bool								IsBroadcast() const;
	inline const U32								GetBroadcastMask() const;
	inline const C_ActorHandle&						GetDestActor() const;
	inline const Component::C_ComponentInterfaceId	GetDestComponent() const;
	inline const Messaging::C_MessageBase*			GetMessage() const;
	
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

private:
	C_ActorHandle				m_Dest;				///< if as U32 is zero then this is a broadcast, otherwise specifies the actor to receive the message
	S32							m_ComponentOrMask;	///< if IsBroadcast() then this is the broadcast mask, otherwise it is the component id
	Messaging::C_MessageBase*	m_pMessage;			///< strong reference

	void SetMessage(const Messaging::C_MessageBase* pMessage); ///< copies the message data

};


/**
@brief Stores messages that are deferred until flushed.
*/
class C_DeferredActorMessageStore : public Btl::ListElementSingle<C_DeferredActorMessageStore>, public C_ActorMessageStoreBase 
{
public:
	// auto: C_DeferredActorMessageStore();
	// auto: C_DeferredActorMessageStore(const C_DeferredActorMessageStore&);
	// auto: ~C_DeferredActorMessageStore();
	// auto: C_DeferredActorMessageStore& operator = (const C_DeferredActorMessageStore&);
};

/**
@brief Queues message in first-in-first-out order.

Messages can be inserted into the queue using PushMessage() for messages aimed
at a single actor or component, and using BroadcastMessage() for messages
aimed at all actors. The queue can be emptied using ProcessMessage().
*/
class C_DeferredActorMessageQueue
{
public:
	C_DeferredActorMessageQueue(C_DeferredActorMessagePool& messagePool);
	~C_DeferredActorMessageQueue();

	const U32	Size() const {return m_MessageFIFO.Size();} ///< @returns the number of messages in the queue

	const bool	PushMessage(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage);
	const bool	BroadcastMessage(const U32 mask, const Messaging::C_MessageBase* pMessage);
	const U32	ProcessMessages(C_ActorList* pActorList);
	
	void		Serialise(Sys::C_ContextualStream& stream) const;
	void		Deserialise(Sys::C_ContextualStream& stream);

private:
	C_DeferredActorMessageList		m_MessageFIFO; ///< stores messages
	C_DeferredActorMessagePool&		m_MessagePool; ///< allocates messages

	C_DeferredActorMessageQueue(const C_DeferredActorMessageQueue&);				// can not copy
	C_DeferredActorMessageQueue& operator = (const C_DeferredActorMessageQueue&);	// can not copy

	void		Clear(); ///< empties the queue

};

/**
@brief Deferred actor messaging.

Provides a number of message queues that can be built up using PushMessage()
for messages targetted at specific actors or components, or using
BroadcastMessage() for messages targetted at all actors. These queues are then
flushed using ProcessMessages().
*/
class C_DeferredActorMessages
{
public:
	explicit C_DeferredActorMessages(const U32 noMessageQueues);
	~C_DeferredActorMessages();

	const U32			GetNumQueues() const {return static_cast<U32>(m_MessageQueues.Size());}

	inline const bool	PushMessage(const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage);
	inline const bool	BroadcastMessage(const U32 messageQueue, const U32 mask, const Messaging::C_MessageBase* pMessage);
	inline const U32	ProcessMessages(const U32 messageQueue, C_ActorList* pActorList);
	
	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

private:
	Btl::Vector<C_DeferredActorMessageQueue*>	m_MessageQueues;	///< stores messages
	C_DeferredActorMessagePool					m_MessagePool;		///< allocates messages

	C_DeferredActorMessages(const C_DeferredActorMessages&);				// can not copy
	C_DeferredActorMessages& operator = (const C_DeferredActorMessages&);	// can not copy

	void				Init(const U32 noMessageQueues);
	void				Clear();

};


/**
@brief Stores messages that are delayed until a specific time.
*/
class C_DelayedActorMessageStore : public Btl::ListElementDouble<C_DelayedActorMessageStore>, public C_ActorMessageStoreBase
{
public:
	inline C_DelayedActorMessageStore();
	// auto: C_DelayedActorMessageStore(const C_DelayedActorMessageStore&);
	// auto: ~C_DelayedActorMessageStore();
	// auto: C_DelayedActorMessageStore& operator = (const C_DelayedActorMessageStore&);

	inline void			Init(const C_ActorHandle &dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase *pMessage, const U32 timeStamp, const U32 messageQueue);
	inline void			Init(const U32 mask, const Messaging::C_MessageBase* pMessage, const U32 timeStamp);

	inline const U32	GetTimeStamp() const {return m_TimeStamp;}
	inline void			SetTimeStamp(const U32 timeStamp) {m_TimeStamp = timeStamp;}

	inline const U32	GetMessageQueue() const {return m_MessageQueue;}
	
	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

private:
	U32				m_TimeStamp;		///< when should the message be sent
	U32				m_MessageQueue;		///< which queue is the message for

};

/**
@brief Queues messages along with a time stamp, allowing them to be delayed.

Messages are added to the queue with an associated time stamp and message
queue index. The Update() method checks for messages that have passed the
delay time and sends them using the specified message queue.
*/
class C_DelayedActorMessageQueue
{
public:
	explicit C_DelayedActorMessageQueue(C_DelayedActorMessagePool& messagePool);
	~C_DelayedActorMessageQueue();

	const bool	PushMessage(const U32 delay, const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage);
	const bool	BroadcastMessage(const U32 delay, const U32 mask, const Messaging::C_MessageBase* pMessage);

	void		Update(const U32 delta, C_ActorList* pActorList);
	
	void		Serialise(Sys::C_ContextualStream& stream) const;
	void		Deserialise(Sys::C_ContextualStream& stream);

private:
	U32								m_TimeStamp;		///< base value for time stamps
	C_DelayedActorMessageList		m_MessageList;		///< stores messages
	C_DelayedActorMessagePool&		m_MessagePool;		///< allocated messages

	C_DelayedActorMessageQueue(const C_DelayedActorMessageQueue&);				// can not copy
	C_DelayedActorMessageQueue& operator = (const C_DelayedActorMessageQueue&); // can not copy

	void		Clear();

};

/**
@brief Delays actor messages for a specified time.

Allows actor messages to be sent after a certain amount of time, or a number
of frames, has passed.
*/
class C_DelayedActorMessages
{
public:
	explicit C_DelayedActorMessages();
	~C_DelayedActorMessages();

	inline const bool	DelayMessageByFrames(const U32 numFrames, const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage);
	inline const bool	DelayBroadcastByFrames(const U32 numFrames, const U32 mask, const Messaging::C_MessageBase* pMessage);

	inline const bool	DelayMessageBySeconds(const F32 numSeconds, const U32 messageQueue, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage);
	inline const bool	DelayBroadcastBySeconds(const F32 numSeconds, const U32 mask, const Messaging::C_MessageBase* pMessage);

	void				Update(const F32 timeStep, C_ActorList* pActorList);
	
	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

private:
	C_DelayedActorMessagePool		m_MessagePool;		///< allocates message storage
	C_DelayedActorMessageQueue		m_FrameQueue;		///< messages delayed by a number of frames
	C_DelayedActorMessageQueue		m_TimedQueue;		///< messages delayed by a number of milliseconds

	C_DelayedActorMessages(const C_DelayedActorMessages&);				// can not copy
	C_DelayedActorMessages& operator = (const C_DelayedActorMessages&); // can not copy

	inline static const U32 SecondsToMilliseconds(const F32 seconds);

};

} // namespace Actor


#define ACTORMESSAGING_INL
#include "ActorMessaging.inl"
#undef ACTORMESSAGING_INL

#endif
