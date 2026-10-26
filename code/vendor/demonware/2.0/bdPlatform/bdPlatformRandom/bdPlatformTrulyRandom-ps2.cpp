// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2 PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformRandom/bdPlatformTrulyRandom.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

static bdInt32 s_seed = 0;

bdInt32 getRandomInt()
{
	if(!s_seed)
	{
		s_seed = static_cast<bdUInt32>(bdPlatformTiming::getHiResTimeStamp());
	}
	
	bdInt32 t = 16807*(s_seed % 127773) - 2836 * (s_seed / 127773);

	if (t <= 0)
	{
		t += 2147483647;
	}

	s_seed = t;

	return s_seed;
}


void BD_CALL bdGetRandomUChar8(bdUByte8 in[], 
							   const bdUInt32 length)
{
	bdInt32 random;

	for(bdUInt32 i = 0; i < length; i++)
	{
		random = getRandomInt();
		in[i] = static_cast<bdUByte8>(random);
	}
}

