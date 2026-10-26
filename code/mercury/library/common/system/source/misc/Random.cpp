#ifdef PLATFORM_SPU
#include "StdAfx.h"
#else
#include "SystemPch.h"
#endif
#include <core/core.h>
#include <core/Maths.h>
#include <System/Misc/Random.h>

using namespace Maths;

namespace Util
{

// Random Number State
// Makoto Matsumoto and Takuji Nishimura,
#define M 397
#define N RAND_N
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

#define SYSTEM_RAND_MAX 4294967295UL

#ifdef PLATFORM_PC
static U32 g_Value;
#endif

C_Random::C_Random()
{
    m_Value = 0;
	mti = RAND_N+1;
	Seed(5489UL);
}

C_Random::~C_Random()
{
}

void C_Random::UpdateSeed()
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	m_Value = y;
}


void C_Random::Seed(const U32 seed)
{
	mt[0]= seed & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] =
		(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

const U32 C_Random::Rand()
{
	UpdateSeed();
	return m_Value;
}

const U32 C_Random::Rand(const U32 range)
{
    if(range == 0)
        return 0;

	UpdateSeed();
	return m_Value % range;
}

const S32 C_Random::SignedRand()
{
	UpdateSeed();
	return m_Value - (SYSTEM_RAND_MAX/2);
}

const S32 C_Random::SignedRand(const U32 range)
{
    if(range == 0)
        return 0;

	UpdateSeed();
	return (m_Value - 32767) % range;
}

const F32 C_Random::FRand()
{
	UpdateSeed();
#ifdef PLATFORM_PC
	g_Value = m_Value;
#endif
	return (F32)m_Value * (const F32)(1.0f / (float)SYSTEM_RAND_MAX);
}

const F32 C_Random::FRand(const F32 range)
{
    return FRand() * range;
}

const F32 C_Random::FRand(const F32 min, const F32 max)
{
    const F32 fRand = FRand();
    return min * fRand + max * (1.0f - fRand);
}

const F32 C_Random::SignedFRand()
{
    UpdateSeed();
	return (F32)m_Value * (const F32)(2.0f / (float)SYSTEM_RAND_MAX) - 1.0f;
}

const F32 C_Random::SignedFRand(const F32 range)
{
    UpdateSeed();
	return ((F32)m_Value * (const F32)(2.0f / (float)SYSTEM_RAND_MAX) - 1.0f) * range;
}

const F32 C_Random::AngRand()
{
	UpdateSeed();
	return (F32)m_Value * (const F32)((2.0f * Maths::Pi) / (float)SYSTEM_RAND_MAX);
}

const F32 C_Random::SignedAngRand()
{
	UpdateSeed();
	return (F32)m_Value * (const F32)((2.0f * Maths::Pi) / (float)SYSTEM_RAND_MAX) - Maths::Pi;
}

const HVec2 C_Random::RectRand()
{
    return HVec2(FRand(), FRand());
}

const HVec2 C_Random::RectRand(const F32 range)
{
	const F32 x = FRand();
	const F32 y = FRand();
    return HVec2(x, y) * range;
}

const HVec2 C_Random::SignedRectRand()
{
	const F32 x = SignedFRand();
	const F32 y = SignedFRand();
    return HVec2(x, y);
}

const HVec2 C_Random::SignedRectRand(const F32 range)
{
	const F32 x = SignedFRand();
	const F32 y = SignedFRand();
    return HVec2(x, y) * range;
}

const HVec3 C_Random::CubeRand()
{
    return HVec3(FRand(), FRand(), FRand());
}

const HVec3 C_Random::CubeRand(const F32 range)
{
	const F32 x = FRand();
	const F32 y = FRand();
	const F32 z = FRand();
    return HVec3(x, y, z) * range;
}

const HVec3 C_Random::SignedCubeRand()
{
	const F32 x = SignedFRand();
	const F32 y = SignedFRand();
	const F32 z = SignedFRand();
    return HVec3(x, y, z);
}

const HVec3 C_Random::SignedCubeRand( const F32 range )
{
	const F32 x = SignedFRand();
	const F32 y = SignedFRand();
	const F32 z = SignedFRand();
    return HVec3(x, y, z) * range;
}

const HVec3 C_Random::SphereRand()
{
	float u1 = FRand();
	float u2 = FRand();
	float z = 1.f - 2.f * u1;
	float r = sqrtf(Maths::Max(0.f, 1.f - z*z));
	float phi = float(2.f * M_PI * u2);
	float x = r * cosf(phi);
	float y = r * sinf(phi);
	return HVec3(x, y, z);
}

}; // namespace Util


#undef M
#undef N
