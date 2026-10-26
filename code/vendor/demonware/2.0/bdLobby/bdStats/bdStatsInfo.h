// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_STATS_INFO_H
#define BD_STATS_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdStats/bdStats.h>

/// Wraps a single user's stats on a single leaderboard. 
/// This class should be derived from and its serialize/deserialize functions
/// should be overridden so that title-specific statistics can be transmitted
/// to the MatchMaking+ service.
class bdStatsInfo : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdStatsInfo();

		/// Destructor.
		virtual ~bdStatsInfo();

		/// Serialize the stats for transmission to the MatchMaking+ service.
		/// \param param[out]	This is returned with the stats written to it.
		virtual void serialize(bdByteBuffer &buffer) const;

		/// Deserialize the stats from a buffer.
		/// \param param[in]	This buffer contains stats transmitted by the 
		///						MatchMaking+ service.
		/// \return 
		///		- True, the buffer was de-serialized correctly.
		///		- False, there was an error de-serializing the buffer.
		virtual bdBool deserialize(bdByteBufferRef buffer);
	
		virtual bdUInt sizeOf();

		/// The leaderboard ID
		bdLeaderBoardID m_leaderboardID;

		/// The entity (i.e. either team or user) id
		bdEntityID m_entityID;

		/// Specifies how the rating is accumulated on the server
		bdStats::bdWriteType m_writeType;

		/// Entity's rating, 64 bit
		bdInt64 m_rating;
		
		/// Entity's rank (read only)
		bdUInt64 m_rank;

		/// Entity's name (read only)
		bdNChar8 m_entityName[BD_MAX_USERNAME_LENGTH+1];

		bdUInt32 m_secondsSinceUpdate;
};

#endif // BD_STATS_INFO_H
