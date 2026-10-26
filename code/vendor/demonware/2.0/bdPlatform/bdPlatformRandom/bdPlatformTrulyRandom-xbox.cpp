// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformRandom/bdPlatformTrulyRandom.h>
#include <winsockx.h>

void BD_CALL bdGetRandomUChar8(bdUByte8 in[], 
							   const bdUInt32 length)
{
	XNetRandom(in, length);
}

