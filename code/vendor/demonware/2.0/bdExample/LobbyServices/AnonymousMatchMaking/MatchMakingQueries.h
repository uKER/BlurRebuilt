
// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_MATCHMAKING_QUERIES_H
#define BD_MATCHMAKING_QUERIES_H

#include <bdLobby/bdMatchMaking/bdSessionParams.h>


enum QueryId
{
	FIND_SESSIONS_BY_GAME_TYPE = 1
};


/// Encapsulates a matchmaking query.
class FindSessionsByGameType : public bdSessionParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		FindSessionsByGameType();

		virtual ~FindSessionsByGameType();

		virtual void serialize(bdByteBuffer &buffer)const;

		virtual bdUInt sizeOf();

		bdUInt32 m_gameType;
};

#endif // BD_MATCHMAKING_QUERIES_H
