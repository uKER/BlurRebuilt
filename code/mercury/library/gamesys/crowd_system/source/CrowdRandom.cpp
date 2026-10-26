#include "CrowdSystemPch.h"

#include <crowd_system/CrowdRandom.h>

#include <system/misc/random.h>

namespace AnimatedCrowd
{
C_Random C_Random::s_Instance;

C_Random::C_Random()
	: LUTIndexF32(0),
	LUTIndexU32(0)
{
	memset(RandomF32LUT, 0, sizeof(F32) * CROWD_RANDOM_LUT_SIZE);
	memset(RandomU32LUT, 0, sizeof(U32) * CROWD_RANDOM_LUT_SIZE);
}

void C_Random::Initialise(const U32 seed)
{
	LUTIndexF32 = 0;
	LUTIndexU32 = 0;

	Util::C_Random random;
	random.Seed(seed);

	for (U32 i = 0, count = CROWD_RANDOM_LUT_SIZE; i < count; ++i)
		RandomF32LUT[i] = random.FRand();
	for (U32 i = 0, count = CROWD_RANDOM_LUT_SIZE; i < count; ++i)
		RandomU32LUT[i] = random.Rand();
}

}; // namespace AnimatedCrowd
