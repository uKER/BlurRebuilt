// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdPlayer/bdClientPlayerListener.h>

bdClientPlayerListener::~bdClientPlayerListener()
{
}

void bdClientPlayerListener::onClientPlayerJoin(bdPlayerRef)
{
}

void bdClientPlayerListener::onClientPlayerLeave(const bdPlayerId)
{
}

void bdClientPlayerListener::onClientJoinReply(bdPlayerRef,
											   const bdBool)
{
}

void bdClientPlayerListener::onClientSetControl(bdPlayerRef, bdReplicaRef)
{
}
