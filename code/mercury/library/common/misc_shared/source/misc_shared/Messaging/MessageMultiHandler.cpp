#include "MiscSharedPch.h"

#include <misc_shared/messaging/MessageMultiHandler.h>


namespace Messaging
{

C_MessageForwardingHandler::C_MessageForwardingHandler(C_MessageHandler* pMessageHandler) :
	C_MessageHandler(), 
	m_pMessageHandler(pMessageHandler)
{
	assert(pMessageHandler != NULL);
}

C_MessageForwardingHandler::~C_MessageForwardingHandler()
{
}

bool C_MessageForwardingHandler::MessageHandler(const C_MessageBase* pMessage)
{
	return m_pMessageHandler->MessageHandler(pMessage);
}

C_MessageMultiHandler::C_MessageMultiHandler() :
	C_MessageHandler(), 
	m_Forwarders(), 
	m_pCurrentForwarder(NULL)
{
}

C_MessageMultiHandler::~C_MessageMultiHandler()
{
	for (C_Forwarders::Iterator it = m_Forwarders.Begin(); it != m_Forwarders.End(); ++it)
		delete *it;
}

bool C_MessageMultiHandler::AddSubscriber(const TMsgType messageType)
{
	if (m_pCurrentForwarder != NULL)
		return m_pCurrentForwarder->AddSubscriber(messageType);

	return false;
}

bool C_MessageMultiHandler::RemoveSubscriber(const TMsgType messageType)
{
	if (m_pCurrentForwarder != NULL)
		return m_pCurrentForwarder->RemoveSubscriber(messageType);

	return false;
}

bool C_MessageMultiHandler::RemoveAllSubscribers()
{
	for (C_Forwarders::Iterator it = m_Forwarders.Begin(); it != m_Forwarders.End(); ++it)
		(*it)->RemoveAllSubscribers();

	return true;
}

bool C_MessageMultiHandler::EnableSubscriber(const TMsgType messageType, const bool bEnable)
{
	if (m_pCurrentForwarder != NULL)
		return m_pCurrentForwarder->EnableSubscriber(messageType, bEnable);

	return false;
}

bool C_MessageMultiHandler::SetSubscriberMask(const TMsgType messageType, const U32 mask)
{
	if (m_pCurrentForwarder != NULL)
		return m_pCurrentForwarder->SetSubscriberMask(messageType, mask);

	return false;
}

void C_MessageMultiHandler::SetSubscriberSystem(C_MessageSubscriberSystem* pSubscriberSys)
{
	// check if we've already got a forwarder for this subscriber system
	for (C_Forwarders::ConstIterator it = m_Forwarders.Begin(); it != m_Forwarders.End(); ++it)
	{
		if (&(*it)->GetSubscriberSystem() == pSubscriberSys)
		{
			m_pCurrentForwarder = *it;
			return;
		}
	}

	// add new forwarder to this subscriber system
	m_pCurrentForwarder = new C_MessageForwardingHandler(this);
	m_pCurrentForwarder->SetSubscriberSystem(pSubscriberSys);
	m_Forwarders.PushBack(m_pCurrentForwarder);
}

void C_MessageMultiHandler::RemoveCurrentSubscriberSystem()
{
	BIZ_ASSERT_MSG(m_pCurrentForwarder, "m_pCurrentForwarder needs to be set");

	// check if we've already got a forwarder for this subscriber system
	for (C_Forwarders::Iterator it = m_Forwarders.Begin(); it != m_Forwarders.End(); ++it)
	{
		if (*it == m_pCurrentForwarder)
		{
			delete *it;
			m_Forwarders.Erase(it);
			m_pCurrentForwarder = NULL;
			return;
		}
	}
}

C_MessageSubscriberSystem& C_MessageMultiHandler::GetSubscriberSystem()
{
	assert(m_pCurrentForwarder != NULL);
	return m_pCurrentForwarder->GetSubscriberSystem();
}

} // namespace Messaging
