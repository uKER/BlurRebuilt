// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdStats/bdArbitratedStatsInfo.h>


bdArbitratedStatsInfo::bdArbitratedStatsInfo()
: m_entityID(0),
  m_rating(0)
{
}

bdArbitratedStatsInfo::~bdArbitratedStatsInfo()
{
}

void bdArbitratedStatsInfo::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt64(m_entityID);
	buffer.writeInt64(m_rating);
}

bdUInt bdArbitratedStatsInfo::sizeOf()
{
	return sizeof(bdArbitratedStatsInfo);
}

