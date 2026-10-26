// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "bdVoteRankStatsInfo.h"

bdVoteRankStatsInfo::bdVoteRankStatsInfo()
: m_fileOwnerID(0),
  m_totalVotes(0),    
  m_avgVoteValue(0)
{
	bdMemset(m_fileOwnerName, 0, BD_MAX_USERNAME_LENGTH+1);
}

bdVoteRankStatsInfo::~bdVoteRankStatsInfo()
{
}

bdUInt bdVoteRankStatsInfo::sizeOf()
{
	return sizeof(bdVoteRankStatsInfo);
}
		 
void bdVoteRankStatsInfo::serialize(bdByteBuffer &buffer)const
{
	// Call up to the base class to serialize the standard statistics info.
	bdStatsInfo::serialize(buffer);

	// Serialize the custom attributes.
	buffer.writeUInt64(m_fileOwnerID);
	buffer.writeString(m_fileOwnerName, BD_MAX_USERNAME_LENGTH+1);
	buffer.writeUInt32(m_totalVotes);
	buffer.writeUInt32(m_avgVoteValue);
}

bdBool bdVoteRankStatsInfo::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdStatsInfo::deserialize(buffer);
	ok = ok && buffer->readUInt64(m_fileOwnerID);	
	ok = ok && buffer->readString(m_fileOwnerName, BD_MAX_USERNAME_LENGTH+1);
	ok = ok && buffer->readUInt32(m_totalVotes);
	ok = ok && buffer->readUInt32(m_avgVoteValue);
	return ok;
}
