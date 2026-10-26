// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdSecurityKey.h>

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

#define BD_LOG_LEVEL "bdCore/bdSocket/bdSecurityKey"

BD_COMPILE_ASSERT(sizeof(bdSecurityKey) == BD_SECURITY_KEY_LENGTH, The_security_key_length_has_been_unsafely_altered);

bdSecurityKey::bdSecurityKey()
{
	bdMemset(ab,1, BD_SECURITY_KEY_LENGTH);
}

bdSecurityKey::bdSecurityKey( const bdSecurityKey &other )
{
	bdMemcpy(ab, other.ab, BD_SECURITY_KEY_LENGTH);
}

bdBool bdSecurityKey::operator == (const bdSecurityKey &other) const
{
 	return bdMemcmp(ab, other.ab, BD_SECURITY_KEY_LENGTH) == 0;
}


