#ifndef MESSAGEMULTIHANDLER_4DB7A743_4BD0_4D3C_A3B2_A63B4F597CD6_H
#define MESSAGEMULTIHANDLER_4DB7A743_4BD0_4D3C_A3B2_A63B4F597CD6_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/list.h>
#include <misc_shared/messaging/MessageHandler.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace Messaging
{

/// Forwards messages to another handler.
class C_MessageForwardingHandler : public C_MessageHandler
{
public:
	explicit C_MessageForwardingHandler(C_MessageHandler* pMessageHandler);
	virtual ~C_MessageForwardingHandler();

	virtual bool MessageHandler(const C_MessageBase* pMessage); // from C_MessageHandler

private:
	C_MessageHandler* m_pMessageHandler; ///< handler to forward message on to

};

/// Subscribes to multiple subscriber systems.
class C_MessageMultiHandler : public C_MessageHandler
{
public:
	C_MessageMultiHandler();
	virtual ~C_MessageMultiHandler();

	// overload base class non-virtual functions... won't function correct if called via base class!
	bool AddSubscriber(const TMsgType messageType);
	bool RemoveSubscriber(const TMsgType messageType);
	bool RemoveAllSubscribers();
	bool EnableSubscriber(const TMsgType messageType, const bool bEnable = true);
	bool SetSubscriberMask(const TMsgType messageType, const U32 mask);
	void SetSubscriberSystem(C_MessageSubscriberSystem* pSubscriberSys);
	void RemoveCurrentSubscriberSystem();
	C_MessageSubscriberSystem& GetSubscriberSystem();

private:
	typedef Btl::List<C_MessageForwardingHandler*> C_Forwarders;

	C_Forwarders				m_Forwarders;			///< strong references, one forwarder per subscriber system.
	C_MessageForwardingHandler*	m_pCurrentForwarder;	///< weak reference, forwarder for current subscriber system.

};

} // namespace Messaging


#endif // MESSAGEMULTIHANDLER_4DB7A743_4BD0_4D3C_A3B2_A63B4F597CD6_H
