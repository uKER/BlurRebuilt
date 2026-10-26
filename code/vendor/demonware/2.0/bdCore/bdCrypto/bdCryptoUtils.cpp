// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PS2 PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCrypto/bdCryptoUtils.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdCore/bdCrypto/bdCypher3Des.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

#include <bdCore/bdUtilities/bdTrulyRandom.h>

bdUInt32 BD_CALL bdCryptoUtils::getNewIVSeed()
{
	return bdTrulyRandom::getInstance()->getRandomUInt();
}

void BD_CALL bdCryptoUtils::calculateInitialVector(const bdUInt32 seed, bdUByte8* iv)
{
	bdHashTiger192 tigerHash;
	bdUInt IVSize = BD_TIGER_HASH_SIZE;

	const bdUInt seedBufferSize = sizeof(seed);
	bdUByte8 seedBuffer[seedBufferSize];
	bdUInt tmp = 0;
	BD_ASSERT(bdBytePacker::appendBasicType(seedBuffer, seedBufferSize, 0, tmp, seed),
			"Failed to serialize ivseed.");
	BD_ASSERT(tigerHash.hash(seedBuffer, seedBufferSize, iv, IVSize	),
			  "Hash function failed.");
}

void BD_CALL bdCryptoUtils::encrypt(const void *key, const void *initialVector, const void *source, void *dest, const bdUInt size)
{
	// TODO add a warning if the data id not correctly sized... must be a multiple of 8!!
	bdCypher3Des cypher;
	bdUInt s = size;
	// encrypt
	cypher.init(reinterpret_cast<const bdUByte8*>(key), BD_ENCRYPTION_KEY_SIZE);
	BD_ASSERT(cypher.encrypt(reinterpret_cast<const bdUByte8*>(initialVector),
					reinterpret_cast<const bdUByte8*>(source),
					reinterpret_cast<bdUByte8*>(dest),
					s),
				"encryption failed");
}

void BD_CALL bdCryptoUtils::decrypt(const void *key, const void *initialVector, const void *source, void *dest, const bdUInt size)
{
	bdCypher3Des cypher;
	bdUInt s = size;
	// encrypt
	cypher.init(reinterpret_cast<const bdUByte8*>(key), BD_ENCRYPTION_KEY_SIZE);
	BD_ASSERT(cypher.decrypt(reinterpret_cast<const bdUByte8*>(initialVector),
					reinterpret_cast<const bdUByte8*>(source),
					reinterpret_cast<bdUByte8*>(dest),
					s),
				"decryption failed");
}

#include <stdlib.h>
extern "C" void BD_CALL libTomCryptQsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
	::qsort(base, nmemb, size, compar);
}

#if defined __PPU__

extern "C" void* BD_CALL libTomCryptMemset(void *s, int c, size_t n)
{
	return bdMemset(s, c, n);
}

extern "C" void* BD_CALL libTomCryptMemcpy(void *dest, const void *src, size_t n)
{
	return bdMemcpy(dest, src, n);
}

extern "C" int BD_CALL libTomCryptMemcmp(const void *s1, const void *s2, size_t n)
{
	return bdMemcmp(s1, s2, n);
}

#endif // __PPU__

