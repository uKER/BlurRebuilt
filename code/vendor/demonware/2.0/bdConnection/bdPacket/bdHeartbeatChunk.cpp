// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdHeartbeatChunk.h>

bdHeartbeatChunk::bdHeartbeatChunk()
: bdChunk(BD_CT_HEARTBEAT),
  m_flags(BD_CA_NO_FLAGS)
{
}

/// Destructor.
bdHeartbeatChunk::~bdHeartbeatChunk()
{
}

bdHeartbeatChunk::bdHeartbeatFlags bdHeartbeatChunk::getFlags() const
{
	return m_flags;
}

bdUInt bdHeartbeatChunk::serialize(bdUByte8 *data, 
								   const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = 0;
	bdBool ok = true;

	offset = bdChunk::serialize(data, size);

	const bdUByte8 flags = static_cast<bdUByte8>(m_flags);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, flags);

	bdUInt16 length = 0;
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, length);

	return offset;
}

bdBool bdHeartbeatChunk::deserialize(const bdUByte8 *const data, 
									 const bdUInt size,
									 bdUInt &offset)
{
	bdBool ok = false;
	bdUInt bytesRead = offset;

	ok = bdChunk::deserialize(data, size, bytesRead);	

	bdUByte8 flags = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, flags);
	m_flags = static_cast<bdHeartbeatFlags>(flags);

	bdUInt16 length = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, length);

	if(ok)
	{
		offset = bytesRead;
	}

	return ok;
}
