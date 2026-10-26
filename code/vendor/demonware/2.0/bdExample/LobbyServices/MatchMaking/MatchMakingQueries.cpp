// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "MatchMakingQueries.h"

#define BD_LOG_LEVEL "MM Queries"


FindSessionsByGameType::FindSessionsByGameType()
{
}

FindSessionsByGameType::~FindSessionsByGameType()
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


