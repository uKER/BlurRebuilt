// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdPacket/bdCookie.h>
#include <bdCore/bdCrypto/bdHMacSHA1.h>

#define BD_LOG_LEVEL	"bdConnection/packet"

bdUByte8 bdCookie::m_secret[BD_COOKIE_SIGNED_HASH_SIZE];
bdBool bdCookie::m_secretInitialized = false;

bdCookie::bdCookie() :
m_localTag(0),
m_peerTag(0),
m_localTieTag(0),
m_peerTieTag(0)
{
}

bdCookie::bdCookie(bdUInt32 localTag,
				   bdUInt32 peerTag,
				   bdUInt32 localTieTag,
				   bdUInt32 peerTieTag)
: m_localTag(localTag),
  m_peerTag(peerTag),
  m_localTieTag(localTieTag),
  m_peerTieTag(peerTieTag)
{
	if(!m_secretInitialized)
	{
		// xxx: secret should probably combine with destination addr.
		bdTrulyRandom::getInstance()->getRandomUByte8(m_secret, BD_COOKIE_SIGNED_HASH_SIZE);
		m_secretInitialized = true;
	}
}

bdCookie::~bdCookie()
{
}

bdUInt32 bdCookie::getLocalTag() const
{
	return m_localTag;
}		

bdUInt32 bdCookie::getPeerTag() const
{
	return m_peerTag;
}

bdUInt32 bdCookie::getLocalTieTag() const
{
	return m_localTieTag;
}

bdUInt32 bdCookie::getPeerTieTag() const
{
	return m_peerTieTag;
}

bdUInt bdCookie::serialize(bdUByte8 *data, 
						   const bdUInt size) const
{
	// WARNING: This code must always return the serialized size of the cookie.
	// Even when data == BD_NULL and size == 0, be careful when editing.

	// skip the hash.
	bdUInt offset = BD_COOKIE_SIGNED_HASH_SIZE;
	bdBool ok = true;

	// xxx: do time
	bdUInt32 timeTag = 0;
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, timeTag);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, m_localTag);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, m_peerTag);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, m_peerTieTag);
	ok = ok && bdBytePacker::appendBasicType(data, size, offset, offset, m_localTieTag);

	if (data != BD_NULL)
	{
		// hash and sign the cookie
		bdHMacSHA1 hmac(m_secret, BD_COOKIE_SIGNED_HASH_SIZE);
		hmac.process(data + BD_COOKIE_SIGNED_HASH_SIZE, offset - BD_COOKIE_SIGNED_HASH_SIZE);
		bdUInt hmacSize = BD_COOKIE_SIGNED_HASH_SIZE;
		hmac.getData(data, hmacSize);
	}

	return offset;
}

bdBool bdCookie::deserialize(const bdUByte8 *const data, 
							 const bdUInt size, 
							 bdUInt &offset)
{
	bdBool ok = false;

	if (size - offset > BD_COOKIE_SIGNED_HASH_SIZE)
	{
		bdUInt bytesRead = offset;
		const bdUByte8 *const cookieData = data + offset;

		// verify the cookie hmac
		bdUByte8 tmpHMac[BD_COOKIE_SIGNED_HASH_SIZE];
		bdHMacSHA1 hmac(m_secret, BD_COOKIE_SIGNED_HASH_SIZE);
		hmac.process(cookieData + BD_COOKIE_SIGNED_HASH_SIZE, getSerializedSize() - BD_COOKIE_SIGNED_HASH_SIZE);
		bdUInt hmacSize = BD_COOKIE_SIGNED_HASH_SIZE;
		hmac.getData(tmpHMac, hmacSize);

		if (bdMemcmp(cookieData, tmpHMac, BD_COOKIE_SIGNED_HASH_SIZE) == 0)
		{
			bytesRead += BD_COOKIE_SIGNED_HASH_SIZE;

			// xxx: do time
			bdUInt32 timeTag = 0;
			ok = bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, timeTag);
			ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_localTag);
			ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_peerTag);
			ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_peerTieTag);
			ok = ok && bdBytePacker::removeBasicType(data, size, bytesRead, bytesRead, m_localTieTag);

			if(ok)
			{
				offset = bytesRead;
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "cookie failed HMac test.");
		}
	}

	return ok;
}

bdUInt bdCookie::getSerializedSize() const
{
	const bdUInt serializedSize = serialize(BD_NULL, 0);
	return serializedSize;
}


