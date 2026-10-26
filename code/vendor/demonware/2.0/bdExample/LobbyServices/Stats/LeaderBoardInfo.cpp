// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "LeaderBoardInfo.h"

LeaderBoardInfo::LeaderBoardInfo()
: m_kills(0),    
  m_deaths(0),     
  m_started(0),   
  m_completed(0)
{
}

LeaderBoardInfo::~LeaderBoardInfo()
{
}

bdUInt LeaderBoardInfo::sizeOf()
{
	return sizeof(LeaderBoardInfo);
}


				 
void LeaderBoardInfo::serialize(bdByteBuffer &buffer)const
{
	// Call up to the base class to serialize the standard statistics info.
	bdStatsInfo::serialize(buffer);

	// Serialize the custom attributes.
	buffer.writeInt64(m_kills);
	buffer.writeInt32(m_deaths);
	buffer.writeInt32(m_started);
	buffer.writeInt32(m_completed);
}

bdBool LeaderBoardInfo::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdStatsInfo::deserialize(buffer);
	ok = ok && buffer->readInt64(m_kills);	
	ok = ok && buffer->readInt32(m_deaths);	
	ok = ok && buffer->readInt32(m_started);	
	ok = ok && buffer->readInt32(m_completed);
	return ok;
}

void BD_CALL printInfo(const LeaderBoardInfo& sessionData,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	bdUInt bytesLeft = length; 
	const bdInt required = bdSnprintf(buf, bytesLeft, "%s Rank:BD_UINT_64_FORMAT Kills:BD_INT_64_FORMAT Deaths:%u", 
		sessionData.m_entityName, sessionData.m_rank, sessionData.m_kills, sessionData.m_deaths);

	bytesLeft -= BD_CLAMP(static_cast<bdUInt>(required), 0, bytesLeft);

	bdSnprintf(buf, bytesLeft, " Started:%u Completed:%u ", 
		sessionData.m_started, sessionData.m_completed);

}

LeaderBoardInfo2::LeaderBoardInfo2()
{
}

void LeaderBoardInfo2::serialize(bdByteBuffer &buffer) const
{
	// Call up to the base class to serialize the standard statistics info.
	bdStatsInfo::serialize(buffer);

	// Serialize the custom attributes.
	buffer.writeInt64(m_kills);
	buffer.writeInt32(m_deaths);

}

bdBool LeaderBoardInfo2::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdStatsInfo::deserialize(buffer);
	ok = ok && buffer->readInt64(m_kills);	
	ok = ok && buffer->readInt32(m_deaths);	
	return ok;
}

bdUInt LeaderBoardInfo2::sizeOf()
{
	return sizeof(LeaderBoardInfo2);
}
