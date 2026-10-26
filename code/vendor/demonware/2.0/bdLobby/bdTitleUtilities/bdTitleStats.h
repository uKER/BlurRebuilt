// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The result set obtained following a call to bdTitleUtilities::getTitleStats.

#ifndef BD_TITLE_STATS_H
#define BD_TITLE_STATS_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdTitleStats : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdTitleStats();

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// The current number of advertised game sessions
		bdUInt32 m_numOnlineSessions;

		/// The total number of registered users
		bdUInt32 m_numTotalUsers;

		/// The number of users that are currently online
		bdUInt32 m_numOnlineUsers;
};

#endif // BD_TITLE_STATS_H
