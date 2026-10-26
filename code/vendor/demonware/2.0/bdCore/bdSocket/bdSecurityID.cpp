// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdSecurityID.h>

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

#define BD_LOG_LEVEL "bdCore/bdSocket/bdSecurityID"

BD_COMPILE_ASSERT(sizeof(bdSecurityID) == BD_SECURITY_ID_LENGTH, The_security_id_length_has_been_unsafely_altered);

bdSecurityID::bdSecurityID()
{
	bdMemset(ab,1, BD_SECURITY_ID_LENGTH);
}

bdSecurityID::bdSecurityID( const bdSecurityID &other )
{
	bdMemcpy(ab, other.ab, BD_SECURITY_ID_LENGTH);
}

bdBool bdSecurityID::operator == (const bdSecurityID &other) const
{
	return bdMemcmp(ab, other.ab, BD_SECURITY_ID_LENGTH) == 0;
}

bdBool bdSecurityID::operator != (const bdSecurityID &other) const
{
	const bdBool equal = (*this == other);
	return !equal;
}


