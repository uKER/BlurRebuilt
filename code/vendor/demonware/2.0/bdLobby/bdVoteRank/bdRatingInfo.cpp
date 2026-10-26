// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdVoteRank/bdRatingInfo.h>
#define BD_LOG_LEVEL "rating info"

bdRatingInfo::bdRatingInfo()
: m_entityID(0),
  m_rating(0)
{
}

bdRatingInfo::bdRatingInfo(bdEntityID entityID, bdUByte8 rating)
: m_entityID(entityID),
  m_rating(rating)
{
	if (rating > BD_VOTE_RANK_MAX_RATING)
    {
        BD_WARN(BD_LOG_LEVEL, "bdRatingInfo.rating must be between 0 and 10 inclusive (%d found)", rating);
    }
    m_rating = BD_MIN(m_rating, BD_VOTE_RANK_MAX_RATING);
}

bdRatingInfo::~bdRatingInfo()
{
}

void bdRatingInfo::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt64(m_entityID);
	buffer.writeUByte8(m_rating);
}

bdBool bdRatingInfo::deserialize(bdByteBufferRef buffer)
{
	// this class is not used for reading results
	return true;
}

bdUInt bdRatingInfo::sizeOf()
{
	return sizeof(bdRatingInfo);
}


