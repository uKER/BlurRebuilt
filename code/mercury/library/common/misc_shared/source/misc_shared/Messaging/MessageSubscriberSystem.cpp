#include "MiscSharedPch.h"
#include <misc_shared/Messaging/MessageSubscriberSystem.h>

#include <algorithm>

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <misc_shared/Messaging/MessageStats.h>
#include <misc_shared/Messaging/MessageSubscriber.h>
#include <misc_shared/Messaging/MessageSubscriberList.inl>
#include <system/debuglog.h>


namespace Messaging
{

struct C_MessageSubscriberListType
{
	TMsgType					m_Type;
	C_MessageSubscriberList*	m_pList;

	C_MessageSubscriberListType() : m_Type(0), m_pList(NULL) {}
	C_MessageSubscriberListType(const TMsgType msgType, C_MessageSubscriberList* pList) : m_Type(msgType), m_pList(pList) {BIZ_ASSERT(pList != NULL);}
};

inline bool operator < (const C_MessageSubscriberListType& a, const C_MessageSubscriberListType& b) {return a.m_Type < b.m_Type;}
inline bool operator < (const TMsgType& a, const C_MessageSubscriberListType& b) {return a < b.m_Type;}
inline bool operator < (const C_MessageSubscriberListType& a, const TMsgType& b) {return a.m_Type < b;}





C_MessageSubscriberSystem::C_MessageSubscriberSystem()
	: m_MessageSubscriberLists()
	, m_pSubscriberPool(new C_MessageSubscriberPool)
#ifndef CFG_SPEED
	, m_pStats(NULL)
#endif
{
	// intentionally blank
}

C_MessageSubscriberSystem::~C_MessageSubscriberSystem()
{
#ifndef CFG_SPEED
	for (C_MessageSubscriberLists::ConstIterator it = m_MessageSubscriberLists.Begin(); it != m_MessageSubscriberLists.End(); ++it)
	{
		if (!C_SystemListUtils::IsEmpty(*it->m_pList))
		{
			LOGERROR(Messaging::GetDefaultLog(), "System still subscribed to message: %s (0x%8x)", Messaging::FindDebugName(it->m_Type), it->m_Type);
		}
	}
#endif
	
	// all subscribers should have been removed by this point
	BIZ_ASSERT(m_pSubscriberPool->Allocated() == 0);

	// clean up all the subscriber lists
	for (C_MessageSubscriberLists::Iterator it = m_MessageSubscriberLists.Begin(); it != m_MessageSubscriberLists.End(); ++it)
	{
		delete it->m_pList;
	}

	// free memory
	delete m_pSubscriberPool;
}

/**
Sends the message to all enabled subscribers to this message type that pass 
the specified mask. For more information on masks, see C_MessageSubscriber.
@param pMessage message to send, must not be NULL.
@param mask bitmask to filter subscribers with.
*/
void C_MessageSubscriberSystem::BroadcastMessage(const C_MessageBase* pMessage, U32 mask)
{
	U32 numHandlers = 0;

	C_MessageSubscriberLists::ConstIterator it = std::lower_bound(m_MessageSubscriberLists.Begin(), m_MessageSubscriberLists.End(), pMessage->Type());

	if ((it != m_MessageSubscriberLists.End()) && (it->m_Type == pMessage->Type()))
	{
		C_MessageSubscriber* pCurrent = it->m_pList->m_First;

		while (pCurrent != NULL)
		{
			C_MessageSubscriber* pNext = C_SystemLinkPolicy::Get(*pCurrent).m_Next;

			if (pNext != NULL)
			{
				DCBT(0, pNext);
			}

			if (pCurrent->m_bEnabled && ((pCurrent->m_Mask & mask) != 0))
			{
				pCurrent->m_pHandler->MessageHandler(pMessage);
				++numHandlers;
			}

			pCurrent = pNext;
		}
	}

#ifndef CFG_SPEED
	if (m_pStats != NULL)
	{
		m_pStats->AccumulateBroadcast(pMessage->Type(), numHandlers);
	}
#endif
}

/**
Sends the message to all enabled subscribers to this message type that pass 
the specified mask. For more information on masks, see C_MessageSubscriber.
@param messageType type identifier for simple message.
@param mask bitmask to filter subscribers with.
*/
void C_MessageSubscriberSystem::BroadcastMessage(const TMsgType messageType, U32 mask)
{
	const C_MessageBase message(messageType, sizeof(C_MessageBase));
	BroadcastMessage(&message, mask);
}

/**
Creates a subscription for the message handler to the message type.
Caller can configure the returned subscription using the Enable() and 
SetMask() methods.
@param messageType message type to subscribe to.
@param pHandler must not be NULL.
@returns subscription object, should be relinquished with RemoveSubscriber().
*/
C_MessageSubscriber* C_MessageSubscriberSystem::AddSubscriber(TMsgType messageType, C_MessageHandler* pHandler)
{
	BIZ_ASSERT(pHandler != NULL);

	// we maintain one list of subscribers for each message type
	C_MessageSubscriberList* pList = NULL;
	{
		C_MessageSubscriberLists::Iterator it = std::lower_bound(m_MessageSubscriberLists.Begin(), m_MessageSubscriberLists.End(), messageType);

		if ((it != m_MessageSubscriberLists.End()) && (it->m_Type == messageType))
		{
			pList = it->m_pList;
		}
		else
		{
			pList = new C_MessageSubscriberList;
			const C_MessageSubscriberListType item(messageType, pList);

			if (it == m_MessageSubscriberLists.End())
			{
				m_MessageSubscriberLists.PushBack(item);
			}
			else
			{
				m_MessageSubscriberLists.InsertBefore(item, it);
			}
		}
	}

	// add a subscription to this list
	C_MessageSubscriber* pSubscriber = m_pSubscriberPool->Allocate();

	pSubscriber->m_bEnabled = true;
	pSubscriber->m_Mask = 0xffffffff;
	pSubscriber->m_pHandler = pHandler;
	pSubscriber->m_Type = messageType;
	
	C_SystemListUtils::PushBack(*pList, *pSubscriber);
	return pSubscriber;
}

/**
Destroys a subscription created by AddSubscriber().
@param pSubscriber must not be NULL.
@returns true if the subscription was removed.
*/
bool C_MessageSubscriberSystem::RemoveSubscriber(C_MessageSubscriber* pSubscriber)
{
	BIZ_ASSERT(pSubscriber != NULL);

	// we maintain one list of subscribers for each message type
	C_MessageSubscriberLists::Iterator it = std::lower_bound(m_MessageSubscriberLists.Begin(), m_MessageSubscriberLists.End(), pSubscriber->m_Type);

	if ((it != m_MessageSubscriberLists.End()) && (it->m_Type == pSubscriber->m_Type))
	{
		// remove the subscriber
		C_SystemListUtils::Remove(*it->m_pList, *pSubscriber);
		m_pSubscriberPool->Free(pSubscriber);

		// check if we need to remove the list type?
		//if (C_SystemListUtils::IsEmpty(*it->m_pList))
		//{
		//	delete it->m_pList;
		//	m_MessageSubscriberLists.Erase(it);
		//}

		return true;
	}
	else
	{
		return false;
	}
}

#ifndef CFG_SPEED

void C_MessageSubscriberSystem::DumpSubscribers(const bool showDetails) const
{
	Debug::Logger& log = Messaging::GetDefaultLog();

	for (C_MessageSubscriberLists::ConstIterator it = m_MessageSubscriberLists.Begin(); it != m_MessageSubscriberLists.End(); ++it)
	{
		const U32 count = C_SystemListUtils::CalcCount(*it->m_pList);

		LOGINFO(log, "%10d  0x%8x    %s", count, it->m_Type, Messaging::FindDebugName(it->m_Type));

		if (showDetails)
		{
			for (const C_MessageSubscriber* pSubscriber = it->m_pList->m_First; pSubscriber != NULL; pSubscriber = C_SystemLinkPolicy::Get(*pSubscriber).m_Next)
			{
				pSubscriber->m_pHandler->DebugDumpMessageHandler(log, "                ");
			}
		}
	}
}

#endif // ndef CFG_SPEED

} // namespace Messaging
