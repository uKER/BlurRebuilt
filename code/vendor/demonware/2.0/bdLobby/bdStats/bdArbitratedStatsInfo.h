// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_ARBITRATED_STATS_INFO_H
#define BD_ARBITRATED_STATS_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

/// Wraps a single user's stats on an arbitrated leaderboard. 
/// This class should be derived from and its serialize & sizeof functions
/// should be overridden so that title-specific statistics can be transmitted
/// to the MatchMaking+ service.
class bdArbitratedStatsInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdArbitratedStatsInfo();

		/// Destructor.
		virtual ~bdArbitratedStatsInfo();

		/// Serialize the stats for transmission to the MatchMaking+ service.
		/// \param param[out]	This is returned with the stats written to it.
		virtual void serialize(bdByteBuffer &buffer) const;
	
		virtual bdUInt sizeOf();

		/// The entity (i.e. either team or user) id
		bdEntityID m_entityID;

		/// Entity's rating, 64 bit
		bdInt64 m_rating;
		
};

#endif // BD_ARBITRATED_STATS_INFO_H
