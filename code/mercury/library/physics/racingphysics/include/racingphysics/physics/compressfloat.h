#pragma once

#ifndef COMPRESS_FLOAT_H
#define COMPRESS_FLOAT_H

// Convert floating point to fixed point with a given number of bits.

namespace RacingPhysics
{

template<typename type, int bits> inline type CompressFloat(float f, float min, float max)
{
	RP_ASSERT(f >= min && f <= max); 
	RP_ASSERT(bits <= sizeof(type) * 8);
	RP_ASSERT(bits <= 32); // U64 is possible but requires more precision (even with double)

	U32 umax = ~((~U32(0) << (bits - 1)) << 1) - 1; // subtract 1 so that the midpoint will compress exactly
	double d = (((f - min) * ((double) umax)) / (max - min));
	return (type) Maths::Floor((float)(d + 0.5));
}

template<typename type, int bits> inline float UncompressFloat(type t, float min, float max)
{
	U32 mask = ~((~U32(0) << (bits - 1)) << 1);
	U32 umax = mask - 1;
	return (float) (((double) (t & mask)) * (max - min) / (double) umax) + min;
}

template<typename type, int bits> inline type CompressFloat(float f, float range)
{
	return CompressFloat<type, bits>(f, -range, range);
}

template<typename type, int bits> inline float UncompressFloat(type t, float range)
{
	return UncompressFloat<type, bits>(t, -range, range);
}

}

#endif
