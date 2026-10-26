#ifndef CROWD_RANDOM_H
#define CROWD_RANDOM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/threads/atomic.h>

namespace AnimatedCrowd
{

static const U32 CROWD_RANDOM_MAX = 4294967295UL;
static const U32 CROWD_RANDOM_LUT_SIZE = 256;

//! C_Random
class ALIGN16 C_Random
{
public:
	C_Random();

	void Initialise(const U32 seed);

	static C_Random& Instance()
	{
		return s_Instance;
	}

	inline F32 RandomF32()
	{
		/*U32 oldValue = 0;
		U32 newValue = 0;
		do
		{
			oldValue = LUTIndexF32;
			newValue = (oldValue + 1) % CROWD_RANDOM_LUT_SIZE;
		} while(!MT::AtomicCompareAndSwap((U32*)&LUTIndexF32, (U32)newValue, (U32)oldValue));
		return RandomF32LUT[newValue];*/
		
		return RandomF32LUT[LUTIndexF32++ % CROWD_RANDOM_LUT_SIZE];
	}

	inline F32 RandomF32(const F32 range)
	{
		return RandomF32() * range;
	}

	inline F32 RandomF32(const F32 min, const F32 max)
	{
		const F32 f = RandomF32();
		return min * f + max * (1.0f - f);
	}

	inline U32 RandomU32()
	{
		/*U32 oldValue = 0;
		U32 newValue = 0;
		do
		{
			oldValue = LUTIndexU32;
			newValue = (oldValue + 1) % CROWD_RANDOM_LUT_SIZE;
		} while(!MT::AtomicCompareAndSwap((U32*)&LUTIndexU32, (U32)newValue, (U32)oldValue));
		return RandomU32LUT[newValue];*/

		return RandomU32LUT[LUTIndexU32++ % CROWD_RANDOM_LUT_SIZE];
	}

	inline U32 RandomU32(const U32 range)
	{
		return (RandomU32() % range);
	}

	inline U32 RandomU32(const U32 min, const U32 max)
	{
		BIZ_ASSERT(min < max);
		return (min + (RandomU32() % (max - min)));
	}

	inline U32 RandomU32Seeded(const U32 seed)
	{
		return RandomU32LUT[seed % CROWD_RANDOM_LUT_SIZE];
	}

	inline U32 RandomU32Seeded(const U32 seed, const U32 range)
	{
		return (RandomU32Seeded(seed) % range);
	}

	inline U32 RandomU32Seeded(const U32 seed, const U32 min, const U32 max)
	{
		BIZ_ASSERT(min < max);
		return (min + (RandomU32Seeded(seed) % (max - min)));
	}

private:
	static AnimatedCrowd::C_Random	s_Instance;

	U32	LUTIndexF32;
	F32	RandomF32LUT[CROWD_RANDOM_LUT_SIZE];
	U32	LUTIndexU32;
	U32	RandomU32LUT[CROWD_RANDOM_LUT_SIZE];
};

}; // namespace AnimatedCrowd

#endif // CROWD_RANDOM_H
