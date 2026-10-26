#ifndef ACTOR_SYSTEM_ACTOR_PERF_H
#define ACTOR_SYSTEM_ACTOR_PERF_H


#include <actor_system/ActorDeclare.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/UID/TypeTable.h>
#include <system/Perf/PerfEvent.h>


#define ACTOR_SYSTEM_PERF_EVENTS (!(CFG_GOLD))


namespace Messaging
{
	class C_DebugMessageType;
}


#if ACTOR_SYSTEM_PERF_EVENTS

namespace Actor
{

typedef U32 C_MessagePerfEventId;

inline C_MessagePerfEventId MakeMessagePerfEventId(const Messaging::TMsgType msgType, const Actor::C_ActorTypeId actorType, const Component::C_ComponentInterfaceId interfaceType = -1)
{
	return static_cast<C_MessagePerfEventId>(msgType) ^ static_cast<C_MessagePerfEventId>(actorType) ^ static_cast<C_MessagePerfEventId>(interfaceType);
}

struct C_MessagePerfEventName
{
	Messaging::TMsgType					m_MessageType;
	Actor::C_ActorTypeId				m_ActorType;
	Component::C_ComponentInterfaceId	m_InterfaceType;
	const char*							m_PerfEventName;
};

class C_MessagePerfEvents
{
public:
	C_MessagePerfEvents();
	~C_MessagePerfEvents();

	static void GlobalUpdate();

	static C_MessagePerfEvents* Instance() {return s_pInstance;}

	inline bool HasMessageType(const Messaging::TMsgType msgType) const
	{
		for (U32 i = 0; i < kMessageTableCapacity; ++i)
			if (m_MessageTable[i] == msgType)
				return true;

		return false;
	}

	const char* MakeMessagePerfEventName(const Messaging::TMsgType msgType, const C_Actor* pActor, const I_ActorComponent* pComponent = NULL);
	
private:
	typedef UID::C_TypeTableHashPolicyDefault<C_MessagePerfEventId, 256> C_HashPolicy;
	typedef UID::C_TypeTable<C_MessagePerfEventId, C_MessagePerfEventName, C_HashPolicy> C_TypeTable;

	static const U32 kMessageTableCapacity = 4;
	static const U32 kStringTableCapacity = 4096;
	
	static C_MessagePerfEvents*		s_pInstance;

	Messaging::TMsgType				m_MessageTable [kMessageTableCapacity];
	U32								m_MessageTableLRU;
	U32								m_StringTableSize;
	Messaging::C_DebugMessageType*	m_pDebugMessageType;
	C_TypeTable						m_TypeTable;
	char							m_StringTable [kStringTableCapacity];

};

} // namespace Actor

#ifdef ENABLE_PIX
	#define ACTOR_PERF_EVENT_BEGIN_IMPL(x_Name) \
		Perf::BeginNamedEvent(x_Name); \
		PIXBeginNamedEvent(0, x_Name);
	#define ACTOR_PERF_EVENT_END_IMPL() \
		Perf::EndNamedEvent(); \
		PIXEndNamedEvent();
#else
	#define ACTOR_PERF_EVENT_BEGIN_IMPL(x_Name) Perf::BeginNamedEvent(x_Name);
	#define ACTOR_PERF_EVENT_END_IMPL() Perf::EndNamedEvent();
#endif

#define ACTOR_PERF_EVENT_BEGIN(x_MsgType, x_pActor) \
	const char* perfEventName = NULL; \
	if ((C_MessagePerfEvents::Instance() != NULL) && C_MessagePerfEvents::Instance()->HasMessageType(x_MsgType)) \
	{ \
		perfEventName = C_MessagePerfEvents::Instance()->MakeMessagePerfEventName(x_MsgType, x_pActor); \
		if (perfEventName != NULL) \
		{ \
			ACTOR_PERF_EVENT_BEGIN_IMPL(perfEventName); \
		} \
	}

#define ACTOR_PERF_EVENT_END() \
	if (perfEventName != NULL) \
	{ \
		ACTOR_PERF_EVENT_END_IMPL(); \
	}

#define COMPONENT_PERF_EVENT_BEGIN(x_MsgType, x_pComponent) \
	const char* perfEventName = NULL; \
	if ((C_MessagePerfEvents::Instance() != NULL) && C_MessagePerfEvents::Instance()->HasMessageType(x_MsgType)) \
	{ \
		perfEventName = C_MessagePerfEvents::Instance()->MakeMessagePerfEventName(x_MsgType, (x_pComponent)->GetActor(), x_pComponent); \
		if (perfEventName != NULL) \
		{ \
			ACTOR_PERF_EVENT_BEGIN_IMPL(perfEventName); \
		} \
	}

#define COMPONENT_PERF_EVENT_END() \
	if (perfEventName != NULL) \
	{ \
		ACTOR_PERF_EVENT_END_IMPL(); \
	}

#else

#define ACTOR_PERF_EVENT_BEGIN(x_MsgType, x_pActor)
#define ACTOR_PERF_EVENT_END()

#define COMPONENT_PERF_EVENT_BEGIN(x_MsgType, x_pComponent)
#define COMPONENT_PERF_EVENT_END()

#endif // ACTOR_SYSTEM_PERF_EVENTS


#endif // ACTOR_SYSTEM_ACTOR_PERF_H
