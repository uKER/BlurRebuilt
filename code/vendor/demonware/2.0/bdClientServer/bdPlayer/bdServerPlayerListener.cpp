// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdPlayer/bdServerPlayerListener.h>
#include <bdConnection/bdConnection.h>

bdServerPlayerListener::~bdServerPlayerListener()
{
}

bdBool bdServerPlayerListener::onServerPlayerJoinRequest(bdServer *const,
														 bdConnectionRef)
{
	return true;
}

void bdServerPlayerListener::onServerSetupPlayer(bdServer *const,
												 bdPlayerRef)
{
}

void bdServerPlayerListener::onServerPlayerJoin(bdServer *const,
												bdPlayerRef)
{
}

void bdServerPlayerListener::onServerPlayerLeave(bdServer *const,
												 bdPlayerRef)
{
}
