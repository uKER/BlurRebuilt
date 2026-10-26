#ifndef MESSAGESUBSCRIBER_H
#define MESSAGESUBSCRIBER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageSubscriberList.h>
#include <misc_shared/Messaging/MessagingDeclare.h>


namespace Messaging
{

/**
@brief Message Handlers create these if they want to receive broadcasts of the 
message type.

Message broadcasts are received by message handlers if:
- the handler has a C_MessageSubscriber for the message type identifier.
- the subscriber is Enabled().
- the bitmask supplied to the broadcast has at least one bit matching the 
bitmask submitted to the subscriber via SetMask(), by default all bits are
set for the subscriber.

It is the responsibility of the C_MessageSubscriberSystem to ensure that all
of these conditions are checked, and to broadcast the message to the handler
if it is satisfied that the subscription applies.
*/
struct C_MessageSubscriber
{
	C_MessageSubscriberLink	m_SubscriberSystemLink;	///< link for list held by subscriber system
	bool					m_bEnabled;				///< enable message receiving
	U32						m_Mask;					///< subscriber mask
	C_MessageHandler*		m_pHandler;				///< handler that owns the subscriber
	TMsgType				m_Type;					///< message type we subscribe to
	C_MessageSubscriberLink	m_MessageHandlerLink;	///< link for list held by message handler
};

} // namespace Messaging


#endif // MESSAGESUBSCRIBER_H
