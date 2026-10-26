// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdBitOperations.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP)

#include <math.h>

#endif

bdUInt BD_CALL bdBitOperations::nextPowerOf2(const bdUInt v)
{	
	bdUInt v2 = v;

	// smear the most significant bit down through v2
	v2 |= v2 >> 1;
	v2 |= v2 >> 2;
	v2 |= v2 >> 4;
	v2 |= v2 >> 8;
	v2 |= v2 >> 16;

	// mask out everything but the most significant bit
	v2 &= ~(v2 >> 1);

	if (v2 != v)
	{
		v2 <<= 1;
	}
	return v2;
}

bdUInt BD_CALL bdBitOperations::highBitNumber(bdUInt v)
{
	BD_ASSERT (v != 0, "bdBitOperations::highBitNumber, no bits set, so cannot find highest.");

	bdUInt i = (v & 0xffff0000) ? 16 : 0;

	if ((v >>= i) & 0xff00)
	{
		i |= 8;
		v >>= 8;
	}

	if (v & 0xf0)
	{
		i |= 4;
		v >>= 4;
	}

	if (v & 0xc)
	{
		i |= 2;
		v >>= 2;
	}

	return (i | (v >> 1));
}
