// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef LEADERBOARD_ENTRY_H
#define LEADERBOARD_ENTRY_H

#include <bdLobby/bdStats/bdStatsInfo.h>
		 
#define LEADERBOARD_ID (1u)

/// This class derives from bdStatsInfo to add custom data attributes.
/// It also overrides the base class serialize/deserialize functions so that the
/// custom attributes are transmitted and received correctly
/// See the stats tutorial for more info.

class LeaderboardEntry : public bdStatsInfo
{
	public:
	
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		LeaderboardEntry();		

		virtual ~LeaderboardEntry();
						
		void setKills(const bdInt64 kills);	
		
		bdInt64 getKills() const;		

		void setDeaths(const bdInt32 deaths);

		bdUInt32 getDeaths() const;

		void setAssists(const bdInt32 assists);

		bdUInt32 getAssists() const;

		void setStarted(const bdInt32 started);

		bdUInt32 getStarted() const;

		void setCompleted(const bdInt32 completed);

		bdUInt32 getCompleted() const;

		void setKillRate(const bdFloat64 killRate);

		bdFloat64 getKillRate() const;

		/// Serialize all matchmaking info into a buffer.
		/// This is used to transmit this information to the service.
		/// \param param[out]	The buffer into which the member variables are 
		///						serialized.
		virtual void serialize(bdByteBuffer &buffer) const;

		/// Deserialize matchmaking info from a buffer.
		/// This is used to decode matchmaking results received from the service.
		/// \param param[in]	The buffer from which the matchmaking attributes
		///						should be deserialized.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

	protected:

		bdInt64 m_kills;
		bdInt32 m_deaths;
		bdInt32 m_assists;
		bdInt32 m_started;
		bdInt32 m_completed;
		bdFloat64 m_killRate;		
};

void BD_CALL printInfo(const LeaderboardEntry& sessionData,
					   bdNChar8 *buf,
					   const bdUInt length);


#endif // LEADERBOARD_ENTRY_H
