#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "../Types/FP32.h"

template<int prime0, int prime1, int prime2>
class C_PerlinNoise
{
public:
	C_PerlinNoise() {};
	~C_PerlinNoise() {};

	F32 SoftNoise1D(FP32 x);
	F32 SoftNoise1D(S32 x);

	F32 Noise1D(S32 x);
	F32 Noise1D(S32 x, S32 offset);
	F32 Noise2D(S32 x, S32 y);
	F32 Noise2D(S32 x, S32 y, S32 offset);
	F32 Noise3D(S32 x, S32 y, S32 z);
	F32 Noise3D(S32 x, S32 y, S32 z, S32 offset);

public:
	F32 Interpolate(F32 a, F32 b, F32 t);
};

template<int prime0, int prime1, int prime2>
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Noise1D(S32 x)
{
	x = (x << 13) ^ x;
	x = x * (x * x * prime0 + prime1) + prime2;
	return ((x & 0x7fffffff) / 1073741823.5f) - 1.f;
}

template<int prime0, int prime1, int prime2>
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Noise1D(S32 x, S32 offset)
{
	return Noise1D(x + offset);
}

template<int prime0, int prime1, int prime2>
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Noise2D(S32 x, S32 y)
{
	return Noise1D(x + y * 57);
}

template<int prime0, int prime1, int prime2>
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Noise2D(S32 x, S32 y, S32 offset)
{
	return Noise1D(x + y * 57 + offset);
}

template<int prime0, int prime1, int prime2>
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Noise3D(S32 x, S32 y, S32 z)
{
	return Noise1D(x + y * 57 + z * 149);
}

template<int prime0, int prime1, int prime2>
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Noise3D(S32 x, S32 y, S32 z, S32 offset)
{
	return Noise1D(x + y * 57 + z * 149 + offset);
}

template<int prime0, int prime1, int prime2>
// Interpolate between a and b where t is the amount (0 to 1)
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::Interpolate(F32 a, F32 b, F32 t)
{
	t = (1.f - cos(t * Maths::Pi)) * 0.5f; // Change t to a curve
	return a * (1.f - t) + b * t;
}

template<int prime0, int prime1, int prime2>
// interpolate between consecutive noise values
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::SoftNoise1D(FP32 x)
{
	S32 i = x;
	F32 a = Noise1D(i);
	F32 b = Noise1D(i + 1);
	F32 t = x.Frac();
	return Interpolate(a, b, t);
}

template<int prime0, int prime1, int prime2>
// interpolate between consecutive noise values
INLINE F32 C_PerlinNoise<prime0, prime1, prime2>::SoftNoise1D(S32 x)
{
	F32 a = Noise1D((x>>16));
	F32 b = Noise1D((x>>16) + 1);
	F32 t = (x & 0xffff)/65536.f;
	return Interpolate(a, b, t);
}

// some default primes
typedef C_PerlinNoise<19379, 819233, 1266122899>	C_PerlinNoiseDefault;

#endif // PERLINNOISE_H
