// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCrypto/bdHMacSHA1.h>

#define BD_LOG_LEVEL "hmacsha1"

#if defined BD_PLATFORM_WIN32 || defined BD_PLATFORM_UNIX || \
	defined BD_PLATFORM_PS2 ||defined BD_PLATFORM_PSP || \
	defined BD_PLATFORM_PS3 || defined BD_PLATFORM_WII

bdHMacSHA1::bdHMacSHA1(const bdUByte8 *const key, const bdUInt keySize)
{
	if (register_hash(&sha1_desc) == -1) 
	{
		BD_ERR(BD_LOG_LEVEL,"Error registering SHA1");
	}

	const bdInt idx = find_hash("sha1");
	const bdInt status = hmac_init(&m_state, idx, key, keySize);
	
	if (status != CRYPT_OK) 
	{
		BD_ERR(BD_LOG_LEVEL,"Error setting up hmac: %s", error_to_string(status));
	}
}

bdHMacSHA1::~bdHMacSHA1()
{
}

bdBool bdHMacSHA1::process(const bdUByte8 *const data, const bdUInt length)
{
	const bdInt status = hmac_process(&m_state, data, length);

	if(status != CRYPT_OK) 
	{
		BD_ERR(BD_LOG_LEVEL,"Error processing hmac: %s", error_to_string(status));
		return false;
	}
	return true;
}

bdBool bdHMacSHA1::getData(bdUByte8 *dst, bdUInt &length)
{
	unsigned long tmpLength = length;
	const bdInt status = hmac_done(&m_state, dst, &tmpLength);
	if (status != CRYPT_OK) 
	{
		BD_ERR(BD_LOG_LEVEL,"Error getting hmac done data : %s", error_to_string(status));
		return false;
	}
	length = tmpLength;
	return true;
}

#else


bdHMacSHA1::bdHMacSHA1(const bdUByte8 *const, const bdUInt)
{
}

bdHMacSHA1::~bdHMacSHA1()
{
}

bdBool bdHMacSHA1::process(const bdUByte8 *const, const bdUInt)
{
	return true;
}

bdBool bdHMacSHA1::getData(bdUByte8 *dst, bdUInt &length)
{
	bdMemset(dst, 0, length);

	return true;
}

#endif
