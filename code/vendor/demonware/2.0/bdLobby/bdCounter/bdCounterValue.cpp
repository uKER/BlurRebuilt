// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdCounter/bdCounterValue.h>

bdCounterValue::bdCounterValue()
:m_counterID(0),
 m_counterValue(0)
{
}

void bdCounterValue::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt32(m_counterID);
	buffer.writeInt64(m_counterValue);
}

bdBool bdCounterValue::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_counterID);
	ok = ok && buffer->readInt64(m_counterValue);
	return ok;
}
		
bdUInt bdCounterValue::sizeOf()
{
	return sizeof(bdCounterValue);
}



