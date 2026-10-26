// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_VOTERANK_STATS_INFO_H
#define BD_VOTERANK_STATS_INFO_H

#include <bdLobby/bdStats/bdStatsInfo.h>

/// Results class. Use with bdStats API to deserialize bdVoteRank leaderboards.
class bdContentStreamingStatsInfo : public bdStatsInfo
{
	public:
	
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdContentStreamingStatsInfo();		

		virtual ~bdContentStreamingStatsInfo();

		virtual bdUInt sizeOf();
						
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

		/// The entity (i.e. either team or user) id
		bdEntityID m_fileOwnerID;

		/// Entity's name (read only)
		bdNChar8 m_fileOwnerName[BD_MAX_USERNAME_LENGTH+1];
};

#endif // BD_VOTERANK_STATS_INFO_H
