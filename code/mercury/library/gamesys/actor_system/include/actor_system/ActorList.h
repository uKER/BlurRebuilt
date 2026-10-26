#ifndef ACTORLIST_H
#define ACTORLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/Handle.h>
#include <actor_system/HandleDeclare.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <system/Debug/I_DebugRender.h>
#include <actor_system/HandleManager.h>

#include <Btl/Containers/vector.h>

namespace Sys
{
	class C_ContextualStream;
	class I_ContextProvider;
}


// define DEFER_ACTOR_MESSAGES to have all actor messages deferred into message queues.
// warning: this changes the order of execution of messages from what we're used to.
//#define DEFER_ACTOR_MESSAGES


namespace Actor
{

#ifndef CFG_SPEED

/// Info about an actor.
struct C_ActorInfo
{
	C_ActorHandle	m_Handle;		///< handle to this actor
	C_ActorTypeId	m_TypeId;		///< type of this actor
	char			m_Name [56];	///< name of this actor (may be truncated)
};

typedef Btl::Vector<C_ActorInfo> C_ActorInfos; ///< Info about a collection of actors.

#endif // CFG_SPEED


/// Manages the list of actors and allows message sending and broadcasting.
class C_ActorList
{
	friend class C_DeferredActorMessageQueue;

public:
	C_ActorList(const U32 maxNoActors, const U32 noMsgQueues, C_ActorFactory* pFactory, const Sys::I_ContextProvider* pDefaultContextProvider = NULL);
	virtual ~C_ActorList();

	Util::C_StaggerManager&					GetStaggerManager(void)	{return *m_pStaggerManager;}
	Messaging::C_MessageSubscriberSystem&	GetSubscriberSystem(void) {return *m_pMessageSubscriberSystem;}
	C_ActorFactory*							GetActorFactory() const	{return m_pActorFactory;}
	const Sys::I_ContextProvider*			GetDefaultContextProvider() const {return m_pDefaultContextProvider;}
	U32										GetCurrentFrameNo() const { return m_CurrentFrameNo;}


	void							Reset();
	void							UpdateSystem(const Maths::C_TimeS& timeStep, const U32 frameNo);
	virtual void					UpdateSystemGlobal();
	virtual void					UpdateSystemNonGlobal(const Maths::C_TimeS& timeStep, const U32 frameNo);
	void							DebugRender(DbUtil::I_DebugRenderPtr debugRender) const;
	void							ProcessMessageQueue(const S32 msgQueue);
#ifdef DEFER_ACTOR_MESSAGES
	void							FlushCurrentMessageQueue();
	void							SetCurrentMessageQueue(S32 msgQueue);
#endif

	// new factory methods
	bool							CreatePreLoad(const I_ActorCreationList& list, const C_ActorCreationConfig& config, C_ActorCreationStatus* io_pStatus = NULL);
	bool							IsWaitingForLoad(const I_ActorCreationList& list) const;
	bool							CreatePostLoad(const I_ActorCreationList& list, const C_ActorCreationConfig& config, C_ActorCreationStatus* io_pStatus = NULL);
	C_ActorHandle					ConstructActor(const C_ActorTypeId actorType);
	bool							InitialiseActor(const C_ActorHandle& handle, const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate);
	bool							EnslaveActor(const C_ActorHandle& handle, const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate);
	bool							PrimeActor(const C_ActorHandle& handle);
	bool							CreateActor(const C_ActorHandle& handle);
	C_ActorHandle					Spawn(const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate);
	bool							DeleteActor(C_Actor* pActor);
	bool							ActorExists(C_ActorHandle handle) const;
	const C_Actor*					GetActorPtr(C_ActorHandle handle) const;

	// Actor messaging
	inline E_ActorMessageSendResult	SendMessageToActor(const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, S32 msgQueue = -1);
	inline E_ActorMessageSendResult	SendMessageToActor(const C_ActorHandle dest, Messaging::TMsgType msgType, S32 msgQueue = -1);
	E_ActorMessageSendResult		SendMessageToActorComponent(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, S32 msgQueue = -1);
	inline E_ActorMessageSendResult	SendMessageToActorComponent(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, Messaging::TMsgType msgType, S32 msgQueue = -1);
	void							BroadcastMessage(const Messaging::C_MessageBase* pMessage, U32 mask = 0xffffffff);
	inline void						BroadcastMessage(Messaging::TMsgType type, U32 mask = 0xffffffff);

	// Delayed actor messaging by a number of frames
	inline void						DelayMessageToActorByFrames(const U32 numFrames, const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, const S32 msgQueue = -1);
	inline void						DelayMessageToActorByFrames(const U32 numFrames, const C_ActorHandle dest, const Messaging::TMsgType msgType, const S32 msgQueue = -1);
	void							DelayMessageToActorComponentByFrames(const U32 numFrames, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, const S32 msgQueue = -1);
	inline void						DelayMessageToActorComponentByFrames(const U32 numFrames, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::TMsgType msgType, const S32 msgQueue = -1);
	void							DelayMessageBroadcastByFrames(const U32 numFrames, const Messaging::C_MessageBase* pMessage, const U32 mask = 0xffffffff);
	inline void						DelayMessageBroadcastByFrames(const U32 numFrames, const Messaging::TMsgType msgType, const U32 mask = 0xffffffff);

	// Delayed actor messaging by a number of seconds
	inline void						DelayMessageToActorBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage, const S32 msgQueue = -1);
	inline void						DelayMessageToActorBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Messaging::TMsgType msgType, const S32 msgQueue = -1);
	void							DelayMessageToActorComponentBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, const S32 msgQueue = -1);
	inline void						DelayMessageToActorComponentBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::TMsgType msgType, const S32 msgQueue = -1);
	void							DelayMessageBroadcastBySeconds(const F32 numSeconds, const Messaging::C_MessageBase* pMessage, const U32 mask = 0xffffffff);
	inline void						DelayMessageBroadcastBySeconds(const F32 numSeconds, const Messaging::TMsgType msgType, const U32 mask = 0xffffffff);

	// Generic component message types
	const Messaging::C_MessageTypeListVariable&	GetComponentGenericMessageTypeList() const {return *m_pGenericComponentMessageTypeList;}
	void							AddComponentGenericMessageType(const Messaging::TMsgType msgType);
	void							AddComponentGenericMessageTypes(const Messaging::I_MessageTypeList& list);

	// components
	const I_ActorComponent*			QueryInterface(C_ActorHandle actor, Component::C_ComponentInterfaceId interfaceID) const;

	void							AddKilledActor(C_Actor* pActor); // TODO: [Oscar] should this be publically accessible? maybe C_Actor needs another way to tell us it's been killed

	typedef Util::I_HandleVisitor<C_Actor>			I_ActorListVisitor;			///< visitor interface for non-const actors
	typedef Util::I_HandleVisitor<const C_Actor>	I_ConstActorListVisitor;	///< visitor interface for const actors

	bool							SerialiseForSave(Sys::C_ContextualStream& stream) const;
	bool							DeserialiseForLoad(C_ActorCreationListMultiple& creationList, Sys::C_ContextualStream& stream);

#ifndef CFG_SPEED
	void							GatherActorInfo(C_ActorInfos& dest) const;
#endif

#ifndef CFG_GOLD
	static void						DebugDumpActors(void* eLevel);
	static void						DebugDumpComponents(void* eLevel);
	static void						DebugDumpSubscribers(void* eLevel);
#endif

protected:
	Btl::Vector<C_ActorHandle>*			m_pKilledActorsList;		///< list for actors to be killed

	C_Actor*						GetActorRWPtr(C_ActorHandle handle);
	bool							VisitActors(I_ActorListVisitor& visitor);
	bool							VisitActors(I_ConstActorListVisitor& visitor) const;

	void							MarkForDeletion(C_Actor& actor);

private:
	Util::C_HandleManager<C_Actor>*			m_pHandleManager;			///< manager for actor handles
	C_ActorFactory*							m_pActorFactory;			///< factory object for creating and destroying actors
	const Sys::I_ContextProvider*			m_pDefaultContextProvider;	///< default context provider
	C_GroupHandleList*						m_pActorGroups;				///< every actor is in one group
	//C_GroupHandleList*						m_pActiveActorGroups;		///< some actors are in one active group
	Util::C_StaggerManager*					m_pStaggerManager;			///< used to stagger actor updates
	Messaging::C_MessageTypeListVariable*	m_pGenericComponentMessageTypeList; ///< messages types that are forwarded to all components
	Messaging::C_MessageSubscriberSystem*	m_pMessageSubscriberSystem;	///< handles message subscriptions for actors and components
	C_DeferredActorMessages*				m_pDeferredMessages;		///< actor deferred message queues
	C_DelayedActorMessages*					m_pDelayedMessages;			///< actor delayed message queues
#ifdef DEFER_ACTOR_MESSAGES
	S32										m_CurrentMessageQueue;		///< which queue should Immediate messages be pushed onto
	bool									m_IsFlushingCurrentQueue;	///< are we flushing the current message queue
#endif
#ifndef CFG_SPEED
	Messaging::C_MessageStats*				m_pMessageStats;			///< strong ptr, gathers stats on messages going through this list
#endif
#ifndef CFG_GOLD
	static C_ActorList*						s_pLastActorList;
#endif
	U32										m_CurrentFrameNo;

	bool							BindActorCreationData(C_Actor* pActor, const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate);

	E_ActorMessageSendResult		SendMessageNow(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase *pMessage);
	void							BroadcastMessageNow(const U32 mask, const Messaging::C_MessageBase* pMessage);

};

} // namespace Actor


#define ACTORLIST_INL
#include "ActorList.inl"
#undef ACTORLIST_INL

#endif
