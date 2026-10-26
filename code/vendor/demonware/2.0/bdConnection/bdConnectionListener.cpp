// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdConnectionListener.h>
#include <bdConnection/bdConnection.h>

bdConnectionListener::bdConnectionListener()
{
}

bdConnectionListener::~bdConnectionListener()
{
}

void bdConnectionListener::onConnect(bdConnectionRef)
{
}

void bdConnectionListener::onConnectFailed(bdConnectionRef)
{
}

void bdConnectionListener::onDisconnect(bdConnectionRef)
{
}

void  bdConnectionListener::onReconnect(bdConnectionRef connection)
{
	onDisconnect(connection);
	onConnect(connection);
}

