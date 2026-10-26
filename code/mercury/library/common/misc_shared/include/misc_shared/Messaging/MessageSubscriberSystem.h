#ifndef MESSAGESUBSCRIBERSYSTEM_H
#define MESSAGESUBSCRIBERSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageSubscriberList.h>
#include <misc_shared/Messaging/MessagingDeclare.h>

#include <btl/allocators/pool.h>
#include <btl/containers/vector.h>


namespace Messaging
{

struct C_MessageSubscriberListType;

/**
@brief Manages the subscriber lists and allows broadcasting to the subscribers.

Subscriptions can be added and removed, allowing messages handlers to receive
broadcasts of the specified message type.
*/
class C_MessageSubscriberSystem
{
public:
	C_MessageSubscriberSystem();
	~C_MessageSubscriberSystem();

	C_MessageSubscriber* AddSubscriber(TMsgType messageType, C_MessageHandler* pHandler);
	bool RemoveSubscriber(C_MessageSubscriber* pSubscriber);

	/// sends the message to all enabled subscribers
	void BroadcastMessage(const C_MessageBase* pMessage, U32 mask = 0xffffffff);

	/// sends the simple message to all enabled subscribers
	void BroadcastMessage(const TMsgType messageType, U32 mask = 0xffffffff);

#ifndef CFG_SPEED
	void DumpSubscribers(const bool showDetails = false) const;
	void SetStats(C_MessageStats* pStats) {m_pStats = pStats;}
#endif

private:
	typedef Btl::Vector<C_MessageSubscriberListType>	C_MessageSubscriberLists;
	typedef Btl::DynamicPool<C_MessageSubscriber>		C_MessageSubscriberPool;

	C_MessageSubscriberLists	m_MessageSubscriberLists;	///< lists of subscribers to each message type
	C_MessageSubscriberPool*	m_pSubscriberPool;			///< provides storage for subscriber objects
#ifndef CFG_SPEED
	C_MessageStats*				m_pStats;					///< weak ptr, gathers stats from broadcasts
#endif

	C_MessageSubscriberSystem(const C_MessageSubscriberSystem&); // can not be copied
	C_MessageSubscriberSystem& operator = (const C_MessageSubscriberSystem&); // can not be copied

};

} // namespace Messaging


#endif // MESSAGESUBSCRIBERSYSTEM_H
