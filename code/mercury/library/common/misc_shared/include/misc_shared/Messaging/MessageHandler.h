#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageSubscriberList.h>
#include <misc_shared/Messaging/MessagingDeclare.h>


namespace Debug
{
	class Logger;
}


namespace Messaging
{

/**
@brief All objects that wish to handle & subscribe to messages must be derived 
from this type.

In order to handle messages, you must derive from C_MessageHandler and 
implement the MessageHandler() method.

If you wish to receive broadcasts from your subscriber system then you should 
subscribe to the relevant message types using AddSubscriber(). You can 
configure these subscriptions using the EnableSubscriber() and 
SetSubscriberMask() methods.
*/
class C_MessageHandler
{
public:
	C_MessageHandler();
	virtual ~C_MessageHandler();

	/// @brief Invoked when a message is dispatched to this handler.
	/// User should implement this method to inspect pMessage->Type() and 
	/// perform	appropriate response.
	/// @param pMessage message to handle, will not be NULL.
	/// @returns true if the message was handled.
	virtual bool MessageHandler(const C_MessageBase* pMessage) = 0;

	bool AddSubscriber(TMsgType messageType); ///< subscribe this handler to broadcasts of this message type
	bool RemoveSubscriber(TMsgType messageType); ///< remove this handlers subscription from broadcasts of this message type
	bool RemoveAllSubscribers(void); ///< remove all broadcast subscriptions

	bool EnableSubscriber(TMsgType messageType, bool bEnable = true); ///< enable/disable our subscription to this message type
	bool SetSubscriberMask(TMsgType messageType, U32 mask); ///< sets the bitmask used to filter this message type
	void SetSubscriberSystem(C_MessageSubscriberSystem* pSubscriberSys);
	C_MessageSubscriberSystem& GetSubscriberSystem(void) {BIZ_ASSERT(m_pMessageSubscriberSystem); return *m_pMessageSubscriberSystem;}	

	bool IsSubscribedTo(TMsgType messageType) const; ///< returns if we are subscribed to this message type

#ifndef CFG_SPEED
	void DebugDumpSubscribers(Debug::Logger& log, const char* prefix) const;
	virtual void DebugDumpMessageHandler(Debug::Logger& log, const char* prefix) const;
#endif

private:
	C_MessageHandler(const C_MessageHandler&); // can not be copied
	C_MessageHandler& operator = (const C_MessageHandler&); // can not be copied

	C_MessageSubscriberList		m_Subscribers;				///< list of subscribers
	C_MessageSubscriberSystem*	m_pMessageSubscriberSystem;	///< message subscriber system the handler belongs to, could we use a singleton instead?

	C_MessageSubscriber* GetSubscriber(TMsgType messageType); ///< gets our subscriber for this message type

};

} // namespace Messaging


#endif // MESSAGEHANDLER_H
