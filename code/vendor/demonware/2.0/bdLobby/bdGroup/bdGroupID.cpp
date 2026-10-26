// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdGroup/bdGroupID.h>

bdGroupID::bdGroupID()
:m_groupID(0)
{
}

void bdGroupID::serialize(bdByteBuffer &buffer) const
{
	buffer.writeUInt32(m_groupID);
}

bdBool bdGroupID::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_groupID);
	return ok;
}

bdUInt bdGroupID::sizeOf()
{
	return sizeof(bdGroupID);
}



