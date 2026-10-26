// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdInitAckChunk.h>
#include <bdConnection/bdPacket/bdInitChunk.h>
#include <bdConnection/bdPacket/bdCookie.h>

bdInitAckChunk::bdInitAckChunk()
: bdChunk(BD_CT_INIT_ACK),
  m_initTag(0),
  m_flags(BD_IC_NO_FLAGS),
  m_windowCredit(0),
  m_peerTag(0)
{
}

bdInitAckChunk::bdInitAckChunk(bdUInt32 initTag, 
							   bdCookieRef cookie,
							   const bdWord windowCredit,
							   bdUInt32 peerTag)
: bdChunk(BD_CT_INIT_ACK),
  m_initTag(initTag),
  m_flags(BD_IC_NO_FLAGS),
  m_cookie(cookie),
  m_windowCredit(windowCredit),
  m_peerTag(peerTag)
{
}

bdInitAckChunk::~bdInitAckChunk()
{
}

bdUInt32 bdInitAckChunk::getInitTag() const
{
	return m_initTag;
}

bdBool bdInitAckChunk::getCookie(bdByteBufferRef &cookie)
{
	if(m_cookieBuffer)
	{
		cookie = m_cookieBuffer;
	}

	return m_cookieBuffer.notNull();
}

bdWord bdInitAckChunk::getWindowCredit() const
{
	return m_windowCredit;
}

bdUInt bdInitAckChunk::serialize(bdUByte8 *data, 
								 const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = 0;
	bdBool ok = true;

	if(m_cookie.notNull())
	{
		offset = bdChunk::serialize(data, size);

		const bdUByte8 flags = static_cast<bdUByte8>(m_flags);
		ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, flags);

		bdUInt oldOffset = offset; // write length later
		offset += sizeof(bdUInt16); // bdUInt16 length16

		ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, m_initTag);

		// write cookie - be careful data could be BD_NULL
		bdUByte8 *const cookieDest = data?data+offset:BD_NULL;
		const bdUInt cookieLength = m_cookie->serialize(cookieDest, size - offset);
		const bdUInt16 length16 = static_cast<bdUInt16>(cookieLength);
		BD_ASSERT(cookieLength == length16, "Cookie is too big to bit in 16 bits.");
		offset += length16;

		// write length
		ok = ok && bdBytePacker::appendBasicType(data, size, oldOffset, oldOffset, length16);
	}

	return offset;
}

bdBool bdInitAckChunk::deserialize(const bdUByte8 *const data, 
								   const bdUInt size,
								   bdUInt &offset)
{
	bdBool ok = true;
	bdUInt bytesRead = offset;

	if(size - offset > BD_CHUNK_MIN_SIZE)
	{
		
		ok = bdChunk::deserialize(data, size, bytesRead);
	
		bdUByte8 flags = 0;
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, flags);
		m_flags = static_cast<bdInitAckChunkFlags>(flags);

		bdUInt16 length = 0;
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, length);
		ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_initTag);

		// read out cookie
		m_cookieBuffer = new bdByteBuffer(length);
		ok = ok && bdBytePacker::removeBuffer(data, size, bytesRead, bytesRead,
						m_cookieBuffer->getData(), length);
	}

	if(ok)
	{
		offset = bytesRead;
	}

	return ok;
}

bdUInt32 bdInitAckChunk::getPeerTag() const
{
	return m_peerTag;
}

