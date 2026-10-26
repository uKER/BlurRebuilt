// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PSP-ADHOC PS3 WII XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "hashtiger192"

bdHashTiger192::bdHashTiger192()
{
	if (register_hash(&tiger_desc) == -1)
	{
		BD_ERR(BD_LOG_LEVEL,"Unable to register hash.");
	}
}

bdHashTiger192::~bdHashTiger192()
{
}

bdBool bdHashTiger192::hash(const bdUByte8 *data, const bdUInt dataSize, bdUByte8 *result, bdUInt &resultSize)
{
	if(resultSize > BD_TIGER_HASH_SIZE)
	{
		// Indicate to users that supplied buffer is excessively large.
		BD_WARN(BD_LOG_LEVEL,"Tiger hash only requires result buffer of 24 bytes. Remaineder of buffer will not be used");
	}

	unsigned long resultSz = resultSize;
	const bdInt error = hash_memory(find_hash("tiger"), data, dataSize, result, &resultSz);
	if(CRYPT_BUFFER_OVERFLOW == error)
	{
		// This is how libtomcrypt behaves.
		BD_ERR(BD_LOG_LEVEL,"Unable to create tiger hash of less than 24 bytes in length!.");
		return false;
	}
	else if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL,"Unable to create hash.");
		return false;
	}
	resultSize = resultSz;
	return true;
}

