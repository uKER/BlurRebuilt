#include "stdafx.h"
#include <animation/skeleton/WindGenerator.h>
#include <animation/skeleton/WindParams.h>

namespace Animation
{

C_WindGenerator::C_WindGenerator():
	m_time(0)
{
}

void C_WindGenerator::ResetTime()
{
	SetTime(0);
}

void C_WindGenerator::SetTime(U32 time)
{
	m_time = time;
}

U32 C_WindGenerator::GetTime() const
{
	return m_time;
}

void C_WindGenerator::Update(const C_WindParams& windParams, F32 deltaTime)
{
	m_time += static_cast<S32>(windParams.GetWindVariationRate() * deltaTime * 65536.f);
	m_windConstants = windParams.GetWindConstants();
	m_windDirection = Maths::Normalise(windParams.GetWindDirection());
}

Maths::Vector3 C_WindGenerator::GenerateWindForce(U32 seed) const
{
	Maths::Vec4F32 constants = m_windConstants;
	Maths::Normal3 windDirection = m_windDirection;
	Maths::VF32 windDirectionVariation = Maths::X(constants);
	Maths::VF32 windStrength = Maths::Y(constants);
	Maths::VF32 windStrengthVariation = Maths::Z(constants);
	
	// Generate two octaves of noise.
	U32 x = m_time + (seed << 17);
	U32 y = x + 0xd194912 * seed;
	U32 z = y + 0xaa19301 * seed;

	Maths::Vector3 slowNoise = SmoothNoise3d(x, y, z);
	Maths::Vector3 fastNoise = SmoothNoise3d(0x7777 + x * 2, 0x9999 + y * 2, 0xcccc + z * 2);

	Maths::VF32 kQuarter = Maths::Const<Maths::VF32>::Quarter();
	Maths::VF32 kOne = Maths::Const<Maths::VF32>::One();
	Maths::VF32 kZero = Maths::Const<Maths::VF32>::Zero();

	// Mix together to produce direction noise.
	Maths::Vector3 noise = slowNoise * (kOne - kQuarter) + fastNoise * kQuarter;
	
	U32 w = m_time + 0x123819a * seed;
	Maths::VF32 strengthNoise = Maths::MakeVF32(SmoothNoise1d(w));
	
	Maths::VF32 localWindStrength = (kOne + strengthNoise * windStrengthVariation) *  windStrength;
	localWindStrength = Maths::Max(localWindStrength, kZero);
	
	Maths::Normal3 localWindDirection = Maths::Normalise(windDirection + windDirectionVariation * noise);
	Maths::Vector3 windForce = localWindStrength * localWindDirection;

	return windForce;
}

F32 C_WindGenerator::SmoothNoise1d(U32 x)
{
	U32 t = x >> 16;
	U32 n1 = Noise(t) & 0xffff;
	U32 n2 = Noise(t + 1) & 0xffff;

	U32 dx = x & 0xffff;

	// convert to float...

	const F32 kTwoOver65535 = 2.f / 65535.f;
	const F32 kOneOver65536 = 1.f / 65536.f;

	F32 fn1 = (n1 * kTwoOver65535) - 1.f;
	F32 fn2 = (n2 * kTwoOver65535) - 1.f;
	F32 fdx = dx * kOneOver65536;

	// Use smooth "S-shaped" polynomial: 3x^2 - 2x^3
	F32 fdx2 = fdx * fdx;
	F32 lerpFactor = 3.f * fdx2 - 2.f * fdx2 * fdx;

	return fn1 + (fn2 - fn1) * lerpFactor;
}

Maths::Vector3 C_WindGenerator::SmoothNoise3d(U32 x, U32 y, U32 z)
{
	// TODO: implement a SIMD version
	return Maths::MakeVector3(SmoothNoise1d(x), SmoothNoise1d(y), SmoothNoise1d(z));
}


} // end Animation
