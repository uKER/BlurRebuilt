// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdGame.h>

#include <bdPeer/bdPlayer/bdPlayer.h>
#include <bdPeer/bdCommand/bdCommand.h>

bdGame::bdGame() 
{
}

bdGame::~bdGame() 
{
}

void bdGame::tick()
{
}

void bdGame::processCommand(const bdPlayerRef,
							const bdCommandRef)
{
}

void bdGame::onRoundJoined()
{
}

void bdGame::onRoundEnd()
{
}

bdBool bdGame::allowLateJoins() const
{
	return false;
}

bdBool bdGame::allowStateBasedMidRoundJoins() const
{
	return false;
}

void bdGame::serialize(bdBitBufferRef)
{
}

bdBool bdGame::deserialize(bdBitBufferRef)
{
	return true;
}

bdBool bdGame::getGameStateHash(bdUInt32&) const
{
	return false;
}
