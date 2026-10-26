// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdMatchMaking/bdPerformanceValue.h>


bdPerformanceValue::bdPerformanceValue()
: m_entityID(0),
  m_performanceValue(0)
{
}

bdPerformanceValue::~bdPerformanceValue()
{
}


void bdPerformanceValue::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt64(m_entityID);
	buffer.writeInt64(m_performanceValue);
}

bdBool bdPerformanceValue::deserialize(bdByteBufferRef buffer)
{

	bdUInt64 entityID = 0;
	bdBool ok = buffer->readUInt64(entityID);
	m_entityID = entityID;

	bdInt64 performanceValue = 0;	
	ok = ok && buffer->readInt64(performanceValue);
	m_performanceValue = performanceValue;
	if (!ok)
	{
		BD_ERR("err", "Deserialization failed");
	}
	return ok;
}

bdUInt bdPerformanceValue::sizeOf()
{
	return sizeof(bdPerformanceValue);
}

