#include "ActorSystemPch.h"
#include <actor_system/ActorPerf.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorComponent.h>
#include <actor_system/ActorFactory.h>
#include <actor_system/ActorList.h>
#include <actor_system/GameComponent.h>
#include <actor_system/GameComponentFactory.h>
#include <misc_shared/Messaging/MessageDebug.h>


#if ACTOR_SYSTEM_PERF_EVENTS

static Debug::Bool g_DebugActorPerfEnable("ActorDebug.Perf.Enable", false);
static Debug::Bool g_DebugActorPerfAddMessage("ActorDebug.Perf.AddMessage", false);
static Debug::Bool g_DebugActorPerfClearMessages("ActorDebug.Perf.ClearMessages", false);

static const char g_PerfEventNamePrefix [] = "Actor";

namespace Actor
{

C_MessagePerfEvents* C_MessagePerfEvents::s_pInstance = NULL;

C_MessagePerfEvents::C_MessagePerfEvents()
	: m_MessageTableLRU(0)
	, m_StringTableSize(0)
	, m_pDebugMessageType(Messaging::CreateDebugMessageType("ActorDebug.Perf.MessageType"))
{
	for (U32 i = 0; i < kMessageTableCapacity; ++i)
		m_MessageTable[i] = 0;
}

C_MessagePerfEvents::~C_MessagePerfEvents()
{
	Messaging::DestroyDebugMessageType(m_pDebugMessageType);
}

void C_MessagePerfEvents::GlobalUpdate()
{
	if (g_DebugActorPerfEnable)
	{
		if (s_pInstance == NULL)
		{
			s_pInstance = new C_MessagePerfEvents;
		}

		if (g_DebugActorPerfAddMessage)
		{
			s_pInstance->m_MessageTable[s_pInstance->m_MessageTableLRU] = Messaging::GetDebugMessageType(*s_pInstance->m_pDebugMessageType);
			s_pInstance->m_MessageTableLRU = (s_pInstance->m_MessageTableLRU + 1) % kMessageTableCapacity;

			g_DebugActorPerfAddMessage.SetValue(false);
		}

		if (g_DebugActorPerfClearMessages)
		{
			for (U32 i = 0; i < kMessageTableCapacity; ++i)
				s_pInstance->m_MessageTable[i] = 0;

			g_DebugActorPerfClearMessages.SetValue(false);
		}
	}
	else
	{
		SAFE_DELETE_PTR(s_pInstance);
	}
}

const char* C_MessagePerfEvents::MakeMessagePerfEventName(const Messaging::TMsgType msgType, const C_Actor* pActor, const I_ActorComponent* pComponent)
{
	BIZ_ASSERT(HasMessageType(msgType));
	BIZ_ASSERT(pActor != NULL);

	const Actor::C_ActorTypeId actorType = pActor->GetActorTypeId();
	const Component::C_ComponentInterfaceId interfaceType = (pComponent != NULL) ? pComponent->GetInterfaceID() : -1;
	const C_MessagePerfEventId perfEventId = static_cast<C_MessagePerfEventId>(msgType) ^ static_cast<C_MessagePerfEventId>(actorType) ^ static_cast<C_MessagePerfEventId>(interfaceType);

	// look for existing entry
	const C_TypeTable::ConstIterator findIt = m_TypeTable.Find(perfEventId);

	if (findIt != m_TypeTable.End())
	{
		return findIt.GetData().m_PerfEventName;
	}

	// add to string table
	char* perfEventName = NULL;
	{
		const char* messageName = Messaging::FindDebugName(msgType);
		const C_ActorTypeName actorName = pActor->GetActorList()->GetActorFactory()->LookupName(actorType);
		const Btl::String& interfaceName = Component::C_GameComponentFactory::GetInstance()->GetComponentIFName(interfaceType);

		const U32 messageNameLen = strlen(messageName);
		const U32 stringSize = lengthof(g_PerfEventNamePrefix) + messageNameLen + 1 + actorName.Length() + 1 + ((pComponent != NULL) ? interfaceName.Length() + 1 : 0);

		if ((m_StringTableSize + stringSize) <= kStringTableCapacity)
		{
			char* perfEventName = &m_StringTable[m_StringTableSize];
			m_StringTableSize += stringSize;

			if (pComponent != NULL)
			{
				sprintf_s(perfEventName, stringSize, "%s.%s.%s.%s", g_PerfEventNamePrefix, messageName, actorName.Chars(), interfaceName.Chars());
			}
			else
			{
				sprintf_s(perfEventName, stringSize, "%s.%s.%s", g_PerfEventNamePrefix, messageName, actorName.Chars());
			}

			// add to perf event table
			{
				C_TypeTable::InsertionResult result = m_TypeTable.Insert(perfEventId);
				BIZ_ASSERT(result.first);

				C_MessagePerfEventName& data = result.second.GetData();

				data.m_MessageType = msgType;
				data.m_ActorType = actorType;
				data.m_InterfaceType = interfaceType;
				data.m_PerfEventName = perfEventName;
			}
		}
	}

	// done
	return perfEventName;
}

} // namespace Actor

#endif // ACTOR_SYSTEM_PERF_EVENTS
