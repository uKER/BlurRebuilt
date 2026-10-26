// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "MatchMakingQueries.h"

#define BD_LOG_LEVEL "MM Queries"


FindSessions::FindSessions()
{
}

FindSessions::~FindSessions()
{
}

void FindSessions::serialize(bdByteBuffer& buffer)const
{
	buffer.writeUInt32(m_gameType);
}

bdUInt FindSessions::sizeOf()
{
	return sizeof(FindSessions);
}

