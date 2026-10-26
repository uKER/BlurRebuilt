// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdRandom.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#define BD_LOG_LEVEL "random"

const bdInt bdRandom::a = 16807;
const bdInt bdRandom::m = BD_MAX_RANDOM;
const bdInt bdRandom::q = 127773;
const bdInt bdRandom::r = 2836;

bdRandom::bdRandom()
{
	setSeed(static_cast<bdUInt>(bdPlatformTiming::getHiResTimeStamp()));
}

bdRandom::bdRandom(const bdUInt seed)
{
	setSeed(seed);
}

bdRandom::~bdRandom()
{
}

bdUInt bdRandom::nextUInt()
{
	bdInt t = a*(m_val%q) - r*(m_val/q);
	if (t <= 0)
	{
		t += m;
	}
	m_val = static_cast<bdUInt>(t);

	return m_val;
}

void bdRandom::nextUBytes(bdUByte8 in[], 
						  const bdInt length)
{
	bdUInt random;

	for(bdInt i = 0; i < length; i++)
	{
		random = nextUInt();
		in[i] = static_cast<bdUByte8>(random);
	}
}

void bdRandom::setSeed(const bdUInt seed)
{
	m_val = seed;

	if(!seed)
	{
		BD_WARN(BD_LOG_LEVEL,"Shouldn't use 0 for seed. 12,195,257 used instead.");
		m_val = 12195257;
	}
}

bdUInt bdRandom::getSeed() const
{
	return m_val;
}

