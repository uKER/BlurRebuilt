#ifndef MESSAGESUBSCRIBERLIST_H
#define MESSAGESUBSCRIBERLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessagingDeclare.h>


namespace Messaging
{

/// A link on a double-ended list.
template <typename T>
struct C_ListLink
{
	T*	m_Prev;
	T*	m_Next;
};

/// A double-ended list.
template <typename T>
struct C_ListContainer
{
	T*	m_First;
	T*	m_Last;

	C_ListContainer() : m_First(NULL), m_Last(NULL) {}
};

typedef C_ListLink<C_MessageSubscriber> C_MessageSubscriberLink; ///< link in list of message subscribers
typedef C_ListContainer<C_MessageSubscriber> C_MessageSubscriberList; ///< list of message subscribers

} // namespace Messaging


#endif // MESSAGESUBSCRIBERLIST_H
