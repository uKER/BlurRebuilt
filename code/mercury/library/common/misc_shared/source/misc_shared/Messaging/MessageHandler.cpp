#include "MiscSharedPch.h"

#include <misc_shared/Messaging/MessageHandler.h>

#include <misc_shared/Messaging/MessageDebug.h>
#include <misc_shared/Messaging/MessageSubscriber.h>
#include <misc_shared/Messaging/MessageSubscriberList.inl>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>


namespace Messaging
{

C_MessageHandler::C_MessageHandler() : 
	m_Subscribers(), 
	m_pMessageSubscriberSystem(NULL)
{
}

C_MessageHandler::~C_MessageHandler()
{
	RemoveAllSubscribers();
}

/**
@param messageType type identifier for the message we want to subscribe to.
@returns true if the subscription was added or was already present.
*/
bool C_MessageHandler::AddSubscriber(TMsgType messageType)
{
	if (GetSubscriber(messageType) == NULL)
	{
		C_HandlerListUtils::PushBack(m_Subscribers, *GetSubscriberSystem().AddSubscriber(messageType, this));
	}

	return true;
}

/**
@param messageType type identifier for message we want to unsubscribe from.
@returns true fif the subscription existed before removal.
*/
bool C_MessageHandler::RemoveSubscriber(TMsgType messageType)
{
	C_MessageSubscriber* pSubscriber = GetSubscriber(messageType);

	if (pSubscriber == NULL)
	{
		return false;
	}
	else
	{
		C_HandlerListUtils::Remove(m_Subscribers, *pSubscriber);
		GetSubscriberSystem().RemoveSubscriber(pSubscriber);
		return true;
	}
}	

/**
@returns true if all subscribers were removed.
*/
bool C_MessageHandler::RemoveAllSubscribers(void)
{
	C_MessageSubscriber* pData = m_Subscribers.m_First;

	while (pData != NULL)
	{
		C_MessageSubscriber* pNext = C_HandlerLinkPolicy::Get(*pData).m_Next;
		GetSubscriberSystem().RemoveSubscriber(pData);
		pData = pNext;
	}

	C_HandlerListUtils::Clear(m_Subscribers);
	return true;
}	

/**
@param messageType type identifier of the message we want the subscriber for.
@returns NULL if we're not subscribed to this message type.
*/
C_MessageSubscriber* C_MessageHandler::GetSubscriber(TMsgType messageType)
{
	for (C_MessageSubscriber* pData = m_Subscribers.m_First; pData != NULL; pData = C_HandlerLinkPolicy::Get(*pData).m_Next)
	{
		if (pData->m_Type == messageType)
		{
			return pData;
		}
	}

	return NULL;
}

/**
@param messageType type identifier of the message we want the check for.
@returns false if we're not subscribed to this message type.
*/
bool C_MessageHandler::IsSubscribedTo(TMsgType messageType) const
{
	for (C_MessageSubscriber* pData = m_Subscribers.m_First; pData != NULL; pData = C_HandlerLinkPolicy::Get(*pData).m_Next)
	{
		if (pData->m_Type == messageType)
		{
			return true;
		}
	}

	return false;
}

/**
We will not received broadcasts from our subscriber system when our subscriber
is disabled. This is a convenient way to temporarily block messages, but for
long term situations it is better to remove the subscription and add it again
later.
@param messageType type identifier of the message to enable/disable 
subsription to.
@param bEnable true to enable, false to disable.
@returns true if we are subscribed to this message type.
*/
// [MarkC] This function is being phased out
bool C_MessageHandler::EnableSubscriber(TMsgType messageType, bool bEnable)
{
	C_MessageSubscriber* pSubscriber = GetSubscriber(messageType);
	
	if (pSubscriber != NULL)
	{
		pSubscriber->m_bEnabled = bEnable;
		return true;
	}
	else
	{
		return false;
	}
}

/**
Sets a bitmask that will be compared to the broadcast bitmask suppied by our
subscriber system. We will not receive the broadcast if there are no matching
bits in the masks. This allows the user to categorise groups of message 
handlers and to target specific groups with each broadcast.
@param messageType type identifier of the message to set a mask for.
@param mask bitmask.
@returns true if we are subscribed to this message type.
*/
bool C_MessageHandler::SetSubscriberMask(TMsgType messageType, U32 mask)
{
	C_MessageSubscriber* pSubscriber = GetSubscriber(messageType);

	if (pSubscriber != NULL)
	{
		pSubscriber->m_Mask = mask;
		return true;
	}
	else
	{
		return false;
	}
}

/**
@param pSubscriberSys message system we want to subscribe to.
*/
void C_MessageHandler::SetSubscriberSystem(C_MessageSubscriberSystem* pSubscriberSys)
{
	// Check if we've already subscribed to another message system and have messages attached to that
	// We're only allowed to be subscribed to one at a time with C_MessageHandler (did you mean to use C_MessageMultiHandler)?
	BIZ_ASSERT((m_pMessageSubscriberSystem == pSubscriberSys) || C_HandlerListUtils::IsEmpty(m_Subscribers));

	m_pMessageSubscriberSystem = pSubscriberSys;
}

#ifndef CFG_SPEED

void C_MessageHandler::DebugDumpSubscribers(Debug::Logger& log, const char* prefix) const
{
	for (const C_MessageSubscriber* pSubscriber = m_Subscribers.m_First; pSubscriber != NULL; pSubscriber = C_HandlerLinkPolicy::Get(*pSubscriber).m_Next)
	{
		LOGINFO(log, "%s0x%8x    %s", prefix, pSubscriber->m_Type, FindDebugName(pSubscriber->m_Type));
	}
}

void C_MessageHandler::DebugDumpMessageHandler(Debug::Logger& log, const char* prefix) const
{
	LOGINFO(log, "%s<unknown>", prefix);
}

#endif

} // namespace Messaging
