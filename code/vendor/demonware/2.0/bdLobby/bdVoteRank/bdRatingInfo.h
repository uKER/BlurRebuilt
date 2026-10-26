// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_RATING_INFO_H
#define BD_RATING_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

#define BD_VOTE_RANK_MAX_RATING (10u)

/// Class used for specifying an entityIDs rating.
/// Used in bdVoteRank::submitRating().
class bdRatingInfo : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdRatingInfo();
		bdRatingInfo(bdEntityID entityID, bdUByte8 rating);
		
		~bdRatingInfo();

		/// Serialize the stats for transmission to the MatchMaking+ service.
		/// \param param[out]	This is returned with the stats written to it.
		virtual void serialize(bdByteBuffer &buffer) const;
		
		// n.b. this class is not used for reading results, but deserialize must be implemented for correct inheritance.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// Entity ID
		bdEntityID m_entityID;

		/// Rating 0-10
		bdUByte8 m_rating;
};

#endif // BD_RATING_INFO_H
