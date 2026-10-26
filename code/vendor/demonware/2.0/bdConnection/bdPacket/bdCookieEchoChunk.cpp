// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdCookieEchoChunk.h>
#include <bdConnection/bdPacket/bdCookie.h>

#define BD_LOG_LEVEL	"bdConnection/packet"

bdCookieEchoChunk::bdCookieEchoChunk()
: bdChunk(BD_CT_COOKIE_ECHO),
  m_flags(BD_CE_NO_FLAGS)
{
}

bdCookieEchoChunk::bdCookieEchoChunk(bdByteBufferRef cookieBuffer)
: bdChunk(BD_CT_COOKIE_ECHO),
  m_flags(BD_CE_NO_FLAGS),
  m_cookieBuffer(cookieBuffer)
{
}

/// Destructor.
bdCookieEchoChunk::~bdCookieEchoChunk()
{
}

bdBool bdCookieEchoChunk::getCookie(bdCookieRef &cookie)
{
	if(m_cookie)
	{
		cookie = m_cookie;
	}

	return m_cookie.notNull();
}

bdCookieEchoChunk::bdCookieEchoFlags bdCookieEchoChunk::getFlags() const
{
	return m_flags;
}

bdUInt bdCookieEchoChunk::serialize(bdUByte8 *data, 
									const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the chunk.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	bdUInt offset = 0;
	bdBool ok = true;

	if(m_cookie.notNull() || m_cookieBuffer.notNull())
	{
		offset = bdChunk::serialize(data, size);

		bdUByte8 flags;
		flags = static_cast<bdUByte8>(m_flags);
		ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, flags);
	
		if(m_cookieBuffer.notNull())
		{
			const bdUInt16 length = static_cast<bdUInt16>(m_cookieBuffer->getSize());
			ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, length);

			ok = ok && bdBytePacker::appendBuffer(data, size, offset, offset,
							m_cookieBuffer->getData(), length);
		}
		else
		{
			//xxx needed?
			bdUInt oldOffset = offset;
			offset += 2;

			// write cookie - be careful data could be BD_NULL
			bdUByte8 *const cookieDest = data?data+offset:BD_NULL;
			const bdUInt16 length = static_cast<bdUInt16>(m_cookie->serialize(cookieDest, size - offset));
			offset += length;

			ok = ok && bdBytePacker::appendBasicType(data, size, oldOffset, oldOffset, length);
		}
	}

	return offset;
}

bdBool bdCookieEchoChunk::deserialize(const bdUByte8 *const data, 
									  const bdUInt size,
									  bdUInt &offset)
{
	bdBool ok = true;
	bdUInt bytesRead = offset;

	ok = ok && bdChunk::deserialize(data, size, bytesRead);

	bdUByte8 flags = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, flags);
	m_flags = static_cast<bdCookieEchoFlags>(flags);

	bdUInt16 length = 0;
	ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, length);

	m_cookie = new bdCookie;
	const bdUInt cookieOffset = bytesRead;
	if (ok && m_cookie->deserialize(data, size, bytesRead))
	{
		if ((bytesRead - cookieOffset) != length)
		{
			BD_WARN(BD_LOG_LEVEL, "Invalid cookie echo.");
			ok = false;
		}
	}

	if(ok)
	{
		offset = bytesRead;
	}

	return ok;
}

