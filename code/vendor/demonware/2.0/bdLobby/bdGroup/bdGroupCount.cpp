// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdGroup/bdGroupCount.h>

bdGroupCount::bdGroupCount()
:m_groupID(0),
 m_groupCount(0)
{
}

void bdGroupCount::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt32(m_groupID);
	buffer.writeUInt32(m_groupCount);
}

bdBool bdGroupCount::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_groupID);
	ok = ok && buffer->readUInt32(m_groupCount);
	return ok;
}
		
bdUInt bdGroupCount::sizeOf()
{
	return sizeof(bdGroupCount);
}



