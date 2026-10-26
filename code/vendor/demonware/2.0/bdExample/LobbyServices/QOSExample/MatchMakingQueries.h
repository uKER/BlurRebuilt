// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef MATCHMAKING_QUERIES_H
#define MATCHMAKING_QUERIES_H

#include <bdLobby/bdMatchMaking/bdSessionParams.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

enum SearchId
{
	FINDSESSIONSBYGAMETYPE = 1
};

/// Encapsulates a matchmaking query.
class FindSessionsByGameType : public bdSessionParams
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		FindSessionsByGameType (bdUInt gameType);

		virtual void serialize(bdByteBuffer &buffer)const;

		virtual bdUInt sizeOf();

	protected:

		bdUInt m_gameType;
};

#endif // MATCHMAKING_QUERIES_H
