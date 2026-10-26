// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdSession/bdSessionListener.h>

bdSessionListener::bdSessionListener()
{
}

bdSessionListener::~bdSessionListener()
{
}

void bdSessionListener::onSessionJoinAccepted()
{
}

void bdSessionListener::onSessionJoinRefused(const bdBitBufferRef)
{
}

void bdSessionListener::onSessionConnectFail() 
{
}

void bdSessionListener::onSessionConnectSuccess() 
{
}

void bdSessionListener::onSessionConnect(const bdConnectionRef) 
{
}

void bdSessionListener::onSessionDisconnect(const bdConnectionRef) 
{
}

void bdSessionListener::onSessionStatusChange(const bdSession::bdSessionStatus,
										      const bdSession::bdSessionStatus)
{
}

void bdSessionListener::onSessionRoleUpdate(const bdSession::bdSessionRole)
{
}

void bdSessionListener::onForceLeave()
{
}
