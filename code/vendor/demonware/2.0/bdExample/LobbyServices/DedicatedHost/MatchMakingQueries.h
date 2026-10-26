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
	FINDSESSIONS_QUERY_ID = 1
};

/// Encapsulates a matchmaking query.
class FindSessions : public bdSessionParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		FindSessions();

		virtual ~FindSessions();

		virtual void serialize(bdByteBuffer &buffer)const;

		virtual bdUInt sizeOf();

		/// Game type is a standard matchmaking attribute
		bdUInt m_gameType;
};

#endif // BD_MATCHMAKING_QUERIES_H
