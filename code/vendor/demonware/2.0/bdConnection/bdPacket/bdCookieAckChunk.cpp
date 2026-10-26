// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdCookieAckChunk.h>

bdCookieAckChunk::bdCookieAckChunk()
: bdChunk(BD_CT_COOKIE_ACK),
  m_flags(BD_CA_NO_FLAGS)
{
}

/// Destructor.
bdCookieAckChunk::~bdCookieAckChunk()
{
}

bdCookieAckChunk::bdCookieAckFlags bdCookieAckChunk::getFlags() const
{
	return m_flags;
}

bdUInt bdCookieAckChunk::serialize(bdUByte8 *data, 
								   const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = 0;
	bdBool ok = true;

	offset = bdChunk::serialize(data, size);

	const bdUByte8 flags = static_cast<bdUByte8>(m_flags);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, flags);

	// XXX: Why write 0 into the buffer?
	bdUInt16 length = 0;
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, length);

	return offset;
}

bdBool bdCookieAckChunk::deserialize(const bdUByte8 *const data, 
									  const bdUInt size,
									  bdUInt &offset)
{
	bdBool ok = true;
	bdUInt bytesRead = offset;

	ok = ok && bdChunk::deserialize(data, size, bytesRead);

	bdUByte8 flags = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, flags);
	m_flags = static_cast<bdCookieAckFlags>(flags);

	// XXX: length is ignored, why bother writing/reading it?
	bdUInt16 length = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, length);

	if(ok)
	{
		offset = bytesRead;
	}

	return ok;
}

