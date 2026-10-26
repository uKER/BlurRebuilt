#ifndef ANIMATION_DECOMPRESSIONSPU_INL
#define ANIMATION_DECOMPRESSIONSPU_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{
	FORCE_INLINE qword LoadUnalignedQword( const void* src )
	{
		const qword srcAddr = si_from_ptr( src );
		const qword q1 = si_lqd( srcAddr, 0 );
		const qword q2 = si_lqd( srcAddr, 16 );
		
		const qword lshift = si_andi( srcAddr, 0xf );
		const qword rshift = si_ai( lshift, -16 );

		const qword r1 = si_shlqby( q1, lshift );
		const qword r2 = si_rotqmby( q2, rshift );
		
		return si_or( r1, r2 );
	}

	FORCE_INLINE qword Load_10_10_9_3( const void* src )
	{
		const qword splatX = si_ila( 0x10203 );
		
		qword v = LoadUnalignedQword( src );
		v = si_shufb( v, v, splatX );
				
		// 12345678 12345678 12345678 12345678
		// ZZZZZZZZ ZZYYYYYY YYYYXXXX XXXXXWWW
		
		const qword mask1023 = si_il( 1023 );
		
		const qword vz = si_and( si_rotmi( v, -22 ), mask1023 );
		const qword vy = si_and( si_rotmi( v, -12 ), mask1023 );
		const qword vx = si_andi( si_rotmi( v, -3 ), 511 );
		const qword vw = si_andi( v, 7 );
		
		qword result = si_selb( vx, vy,		si_fsmbi( 0x0f00 ) );
		result		 = si_selb( result, vz, si_fsmbi( 0x00f0 ) );
		result		 = si_selb( result, vw, si_fsmbi( 0x000f ) );
		
		// convert to float.
		result = si_cuflt( result, 0 );
		
		return result;			
	}

	FORCE_INLINE qword DecompressQuaternionSmallest3( qword quat, qword scale, qword offset )
	{
		// Unpack smallest components.
		const qword one = si_ilhu( 0x3f80 );
		const qword splatX = si_ila( 0x00010203 );
		const qword smallestComponents = si_fma( quat, scale, offset );
		
		// Extract largest index (bits 1-2) and sign (bit 0).
		const qword wInX = si_shlqbyi( quat, 12 );
		const qword w = si_shufb( wInX, wInX, splatX );
		const qword largestIndexAndSign = si_cfltu( w, 0 );
		
		// Calculate largest component as sqrt( 1 - x2 - y2 - z2 )
		const qword smallestMul = si_fm( smallestComponents, smallestComponents );
		qword smallestSum = si_fa( smallestMul, si_shlqbyi( smallestMul, 4 ) );
		smallestSum = si_fa( smallestSum, si_shlqbyi( smallestMul, 8 ) );
		qword largestComponentSq = si_fs( one, smallestSum );
		largestComponentSq = si_shufb( largestComponentSq, largestComponentSq, splatX );
		
		qword largestComponentRecip = si_frsqest( largestComponentSq );
		largestComponentRecip = si_fi( largestComponentSq, largestComponentRecip );
		const qword largestComponent = si_fm( largestComponentSq, largestComponentRecip );

		static const vector unsigned int kPermuteTable[ 4 ] = 
		{
			{ 0x10111213, 0x00010203, 0x04050607, 0x08090a0b },
			{ 0x00010203, 0x10111213, 0x04050607, 0x08090a0b },
			{ 0x00010203, 0x04050607, 0x10111213, 0x08090a0b },
			{ 0x00010203, 0x04050607, 0x08090a0b, 0x10111213 }
		};
		
		// Figure out how to permute the smallest 3 + largest components into the right places.
		const qword permuteTableAddr = si_from_ptr( &kPermuteTable );
		const qword largestIndexTimes16 = si_andi( si_shli( largestIndexAndSign, 3 ), ~0xf );
		const qword finalPerm = si_lqx( permuteTableAddr, largestIndexTimes16 );
		
		// Set sign of largest component.
		const qword sign = si_shli( largestIndexAndSign, 31 );
		const qword signedLargestComponent = si_or( largestComponent, sign );
		const qword result = si_shufb( smallestComponents, signedLargestComponent, finalPerm );
		
		return result;
	}

	inline void DecompressQuaternion32( U32 quat32, Maths::Quaternion& outQuat )
	{
		static const vector float kUnpackConstants = { 2.f * 0.7071067811865f / 1022.f, 2.f * 0.7071067811865f / 510.f, -0.7071067811865f, 1.f };
		const qword packedQuat = Load_10_10_9_3( &quat32 );
		
		const qword constants = (qword) kUnpackConstants;
		const qword splatX = si_ila( 0x10203 );
		
		// Create scale vector.
		const qword yzScale = si_shufb( constants, constants, splatX );
		const qword xScalePref = si_shlqbyi( constants, 4 );
		const qword xScale = si_shufb( xScalePref, xScalePref, splatX );
		const qword merge = si_fsmbi( 0xf000 );
		const qword scale = si_selb( yzScale, xScale, merge );
		
		// Create offset vector.
		const qword offsetInX = si_shlqbyi( constants, 8 );
		const qword offset = si_shufb( offsetInX, offsetInX, splatX );
		
		const qword result = DecompressQuaternionSmallest3( packedQuat, scale, offset );
		
		outQuat.m_Q.m_V = static_cast< Maths::V128 >( result );
	}
	
	FORCE_INLINE qword Load_15_15_15_3( const void* src )
	{
		const qword v = LoadUnalignedQword( src );
		
		// |                X                | |                Y                |
		// 12345678 12345678 12345678 12345678 12345678 12345678 12345678 12345678
		// ZZZZZZZZZZZZZZZZY YYYYYYYY YYYYYYXX XXXXXXXXXXXXXXWWW 00000000000000000
		
		// Shift right by 17 bits to align Z in LSB of word 0.
		const qword vz = si_rotqmbii( si_rotqmbyi( v, -2 ), -1 );
		
		// Shift right by 2 bits to align Y in LSB of word 0.
		const qword mask15 = si_il( 32767 );
		const qword vy = si_and( si_rotqmbii( v, -2 ), mask15 );
		
		// Shift left by 13 bits to align X in LSB of word 0.
		const qword vx = si_and( si_shlqbii( si_shlqbyi( v, 1 ), 5 ), mask15 );
		
		// Shift left by 16 bits to align W in LSB of word 0.
		const __vector4 vw = si_andi( si_shlqbyi( v, 2 ), 7 );
		
		// Combine into single quadword.
		qword result = vx;
		result = si_selb( result, si_rotqmbyi( vy, -4 ), si_fsmbi( 0x0f00 ) );
		result = si_selb( result, si_rotqmbyi( vz, -8 ), si_fsmbi( 0x00f0 ) );
		result = si_selb( result, si_rotqmbyi( vw, -12 ), si_fsmbi( 0x000f ) );
		
		// convert to float.
		result = si_cuflt( result, 0 );
		
		return result;	
	}
		
	inline void DecompressQuaternion48( const U16 quat48[3], Maths::Quaternion& outQuat )
	{
		static const vector float kUnpackConstants = { 2.f * 0.7071067811865f / 32766.f, -0.7071067811865f, 1.f, 1.f };

		const qword packedQuat = Load_15_15_15_3( quat48 );
		
		const qword constants = * (const qword*) &kUnpackConstants;
		const qword splatX = si_ila( 0x00010203 );
		
		// Create scale vector.
		const qword scale = si_shufb( constants, constants, splatX );
		
		// Create offset vector.
		const qword offsetInX = si_shlqbyi( constants, 4 );
		const qword offset = si_shufb( offsetInX, offsetInX, splatX );
		
		const qword result = DecompressQuaternionSmallest3( packedQuat, scale, offset );
		
		outQuat.m_Q.m_V = static_cast< Maths::V128 >( result );
	}
	
	FORCE_INLINE void DecompressVector3U8( const U8 vec3U8[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 )
	{
		qword v = LoadUnalignedQword( vec3U8 );
		const qword splatX = si_ila( 0x00010203 );
		v = si_shufb( v, v, splatX );
		
		const qword zero = si_ila( 0 );
		
#if 0
		// Generate mask...
		const qword maskUpper = si_ilhu( 0x1010 );
		const qword maskLower = si_iohl( maskUpeer, 0x1000 );
		// splat one, rotate, add, rotate, add, rotate, add.
#endif		
		static const vector unsigned int bytesToWords = { 0x10101000, 0x10101001, 0x10101002, 0x10101003 };
				
		// Unpack 3 bytes into words.
		qword vec3 = si_shufb( v, zero, (qword) bytesToWords );
		
		// Convert vector to floating point.
		vec3 = si_cuflt( vec3, 0 );
		
		const qword vOffset = LoadUnalignedQword( offset );
		const qword vScale = LoadUnalignedQword( scale );
				
		qword oneOverRange = si_from_float( 1.f / 254.f );
		oneOverRange = si_shufb( oneOverRange, oneOverRange, splatX );
		
		const qword unitVec3 = si_fm( vec3, oneOverRange );
		
		const qword result = si_fma( vScale, unitVec3, vOffset );		
		
		outVec3.m_V.m_V = static_cast< Maths::V128 >( result );
	}

	FORCE_INLINE void DecompressVector3U16( const U16 vec3U16[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 )
	{
		static const vector unsigned int halfWordsToWords = { 0x10100001, 0x10100203, 0x10100405, 0x10101010 };

		const qword zero = si_ila( 0 );
		qword v = LoadUnalignedQword( vec3U16 );
		qword vec3 = si_shufb( v, zero, (qword) halfWordsToWords );
		vec3 = si_cuflt( vec3, 0 );
		
		const qword vOffset = LoadUnalignedQword( offset );
		const qword vScale = LoadUnalignedQword( scale );
				
		const qword splatX = si_ila( 0x00010203 );
		qword oneOverRange = si_from_float( 1.f / 65534.f );
		oneOverRange = si_shufb( oneOverRange, oneOverRange, splatX );
		
		const qword unitVec3 = si_fm( vec3, oneOverRange );
		
		const qword result = si_fma( vScale, unitVec3, vOffset );		
		
		outVec3.m_V.m_V = static_cast< Maths::V128 >( result );
	}
}

#endif
