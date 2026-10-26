// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdPacket/bdDTLSCookieEcho.h>
#include <bdCore/bdCrypto/bdECCKey.h>

#define BD_LOG_LEVEL "dtls cookie echo"

bdDTLSCookieEcho::bdDTLSCookieEcho()
{
}

// This constructor can fail, it might be better to put all the logic 
// into an init function...
bdDTLSCookieEcho::bdDTLSCookieEcho(bdUInt16 vtag,
								   const bdDTLSInitAck &cookie,
								   bdCommonAddrRef localCommonAddr,
								   bdECCKey *const ECCKey)
: bdDTLSHeader(BD_DTLS_COOKIE_ECHO, vtag, 0),
m_cookie(cookie)
{
	bdSecurityID secID;
	cookie.getSecID(secID);

	bdUByte8 serializedCommonAddr[BD_COMMON_ADDR_SERIALIZED_SIZE];
	localCommonAddr->serialize(serializedCommonAddr);

	bdUByte8 key[BD_ECC_EXPORTED_KEY_SIZE];
	bdUInt keyLen = BD_ECC_EXPORTED_KEY_SIZE;
	const bdBool exportResult = ECCKey->exportKey(key, keyLen);
	if((false == exportResult) || (keyLen != BD_ECC_EXPORTED_KEY_SIZE))
	{
		BD_WARN(BD_LOG_LEVEL, "problem with key");
	}

	bdMemcpy(m_ca, serializedCommonAddr, BD_COMMON_ADDR_SERIALIZED_SIZE);
	bdMemcpy(m_secID, secID.ab, BD_SECURITY_ID_LENGTH);
	bdMemcpy(m_ECCKey, key, BD_ECC_EXPORTED_KEY_SIZE);
}

bdDTLSCookieEcho::~bdDTLSCookieEcho()
{
}

bdBool bdDTLSCookieEcho::serialize(void *data,
								   const bdUInt size,
								   const bdUInt offset,
								   bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::serialize(data, size, newOffset, newOffset);
	status = status && m_cookie.serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset,
			newOffset, m_ca, BD_COMMON_ADDR_SERIALIZED_SIZE);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset,
			newOffset, m_secID, BD_SECURITY_ID_LENGTH);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset,
			newOffset, m_ECCKey, BD_ECC_EXPORTED_KEY_SIZE);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdDTLSCookieEcho::deserialize(const void* data,
									 const bdUInt size,
									 const bdUInt offset,
									 bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
	status = status && m_cookie.deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset,
			newOffset, m_ca, BD_COMMON_ADDR_SERIALIZED_SIZE);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset,
			newOffset, m_secID, BD_SECURITY_ID_LENGTH);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset,
			newOffset, m_ECCKey, BD_ECC_EXPORTED_KEY_SIZE);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

const bdDTLSInitAck& bdDTLSCookieEcho::getCookie() const
{
	return m_cookie;
}

const bdUByte8* bdDTLSCookieEcho::getCa() const
{
	return m_ca;
}

const bdUByte8*  bdDTLSCookieEcho::getSecID() const
{
	return m_secID;
}

const bdUByte8*  bdDTLSCookieEcho::getECCKey() const
{
	return m_ECCKey;
}


