// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PS3 PSP-INFRA WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "MatchMakingQueries.h"


FindSessionsByGameType::FindSessionsByGameType(	bdUInt gameType)
: m_gameType(gameType)
{
}

void FindSessionsByGameType::serialize(bdByteBuffer& buffer)const
{

	buffer.writeUInt32(m_gameType);
}

bdUInt FindSessionsByGameType::sizeOf()
{
	return sizeof(FindSessionsByGameType);
}

