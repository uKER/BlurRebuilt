
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
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include "CustomMatchMakingInfo.h"

#define MAX_FIND_SESSIONS_BY_GAME_TYPE_RESULTS (25u)

enum SearchId
{
	FIND_SESSIONS_BY_GAME_TYPE = 1
};

/// Encapsulates and handles matchmaking results for a query by session ID.
class FindSessionByIDResult : public bdTaskResult
{
	public :

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		FindSessionByIDResult();

		virtual ~FindSessionByIDResult();

		virtual bdBool handleResult(const bdUInt numResults, 
									bdBitBufferRef buffer);

		CustomMatchMakingInfo m_customMMInfo;
};

/// Encapsulates a matchmaking query.
class FindSessionsByGameType : public bdSessionParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		FindSessionsByGameType();

		virtual ~FindSessionsByGameType();

		virtual void serialize(bdByteBuffer &buffer)const;


		bdInt m_gameType;
};


#endif // BD_MATCHMAKING_QUERIES_H
