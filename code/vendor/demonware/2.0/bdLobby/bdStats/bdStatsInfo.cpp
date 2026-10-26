// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdStats/bdStatsInfo.h>


#define BD_LOG_LEVEL "statsInfo"

bdStatsInfo::bdStatsInfo()
: m_leaderboardID(0),
  m_entityID(0),
  m_writeType(bdStats::STAT_WRITE_ADD),
  m_rating(0),
  m_rank(0),
  m_secondsSinceUpdate(0)
{
	bdMemset(m_entityName, 0, BD_MAX_USERNAME_LENGTH+1);
}



bdStatsInfo::~bdStatsInfo()
{
}


void bdStatsInfo::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt32(m_leaderboardID);
	buffer.writeUInt64(m_entityID);
	buffer.writeUByte8(static_cast<bdUByte8>(m_writeType));
	buffer.writeInt64(m_rating);
}

bdBool bdStatsInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = buffer->readUInt64(m_entityID);
	ok = ok && buffer->readInt64(m_rating);
	ok = ok && buffer->readUInt64(m_rank);
	ok = ok && buffer->readString(m_entityName, BD_MAX_USERNAME_LENGTH);
	ok = ok && buffer->readUInt32(m_secondsSinceUpdate);
	if (!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Deserialization failed");
	}
	return ok;
}

bdUInt bdStatsInfo::sizeOf()
{
	return sizeof(bdStatsInfo);
}

