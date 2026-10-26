// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII 
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdPacket/bdDTLSCookieAck.h>

bdDTLSCookieAck::bdDTLSCookieAck()
{
}

bdDTLSCookieAck::~bdDTLSCookieAck()
{
}

bdDTLSCookieAck::bdDTLSCookieAck(bdUInt16 vtag, 
								 const bdUByte8 key[BD_ECC_EXPORTED_KEY_SIZE],
								 const bdSecurityID secID)

: bdDTLSHeader(BD_DTLS_COOKIE_ACK, vtag, 0),
  m_secID(secID)
{
	bdMemcpy(m_ECCKey, key, BD_ECC_EXPORTED_KEY_SIZE);
}

bdBool bdDTLSCookieAck::serialize(void *data,
								  const bdUInt size,
								  const bdUInt offset,
								  bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset,
			newOffset, m_ECCKey, BD_ECC_EXPORTED_KEY_SIZE);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset, newOffset, m_secID.ab, sizeof(m_secID.ab));

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdDTLSCookieAck::deserialize(const void* data,
									const bdUInt size,
									const bdUInt offset,
									bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset,
			newOffset, m_ECCKey, BD_ECC_EXPORTED_KEY_SIZE);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset, newOffset, m_secID.ab, sizeof(m_secID.ab));


	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

const bdUByte8 *bdDTLSCookieAck::getECCKey() const
{
	return m_ECCKey;
}

void bdDTLSCookieAck::getSecID(bdSecurityID& secID) const
{
	secID = m_secID;
}


