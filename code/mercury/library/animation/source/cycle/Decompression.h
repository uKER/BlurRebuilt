#ifndef ANIMATION_DECOMPRESSION_H
#define ANIMATION_DECOMPRESSION_H

#ifdef _MSC_VER
#pragma once
#endif

#define ANIMATION_SIMD	(1)

namespace Animation
{
	/// Decompress a 32-bit quaternion.
	void DecompressQuaternion32( U32 quat32, Maths::Quaternion& outQuat );

	/// Decompress a 48-bit quaternion.
	void DecompressQuaternion48( const U16 quat48[3], Maths::Quaternion& outQuat );

	/// Decompress a vector3 from 3 U8s.
	void DecompressVector3U8( const U8 vec3U8[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 );
	
	/// Decompress a vector3 from 3 U16s.
	void DecompressVector3U16( const U16 vec3U16[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 );
}


#if ANIMATION_SIMD && PLATFORM_SPU
#include "DecompressionSpu.inl"
#else
#include "DecompressionGeneric.inl"
#endif

#endif
