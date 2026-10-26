#ifndef ANIMATION_DECOMPRESSIONVMX_INL
#define ANIMATION_DECOMPRESSIONVMX_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{

	/// Helper function to load a packed 4D 32-bit value.
	FORCE_INLINE __vector4 Load_10_10_9_3( const void* src )
	{
		static const __vector4i kShiftConstants = { 22, 12, 3, 0 };
		static const __vector4i kMaskConstants = { ( 1 << 10) - 1, ( 1 << 9 ) - 1, ( 1 << 3 ) - 1, 0 };
		
		//BIZ_ASSERT( ( (U32) src & 0x3 ) == 0 );
		__vector4 v = __lvlx( src, 0 );
		
		// 12345678 12345678 12345678 12345678
		// ZZZZZZZZ ZZYYYYYY YYYYXXXX XXXXXWWW
		__vector4 shiftConstants = *(const __vector4*) &kShiftConstants;
		__vector4 maskConstants = *(const __vector4*) &kMaskConstants;
		
		__vector4 shift22 = __vspltw( shiftConstants, 0 );
		__vector4 shift12 = __vspltw( shiftConstants, 1 );
		__vector4 shift3 = __vspltw( shiftConstants, 2 );
		__vector4 mask10 = __vspltw( maskConstants, 0 );
		__vector4 mask9 = __vspltw( maskConstants, 1 );
		__vector4 mask3 = __vspltw( maskConstants, 2 );	
		
		__vector4 vz = __vand( __vsrw( v, shift22 ), mask10 );
		__vector4 vy = __vand( __vsrw( v, shift12 ), mask10 );
		__vector4 vx = __vand( __vsrw( v, shift3 ), mask9 );
		__vector4 vw = __vand( v, mask3 );
		
		// merge.
		vx = __vrlimi( vx, vy, 0x4, 3 );	// insert y.
		vx = __vrlimi( vx, vz, 0x2, 2 );	// insert z.
		vx = __vrlimi( vx, vw, 0x1, 1 );	// insert w.
		
		// convert to float.
		__vector4 result = __vcfux( vx, 0 );
		
		return result;
	}

	/// Decompress a smallest-3 compressed quaterion using VMX.
	FORCE_INLINE  __vector4 DecompressQuaternionSmallest3_VMX( __vector4 quat, __vector4 scale, __vector4 offset )
	{
		XMVECTOR data = quat;
		XMVECTOR one = XMVectorSplatOne();

		XMVECTOR largestIndexAndSign = XMConvertVectorFloatToUInt( XMVectorSplatW(data), 0 );
		XMVECTOR smallestComponents	= XMVectorMultiplyAdd( data, scale, offset );
		
		XMVECTOR largestComponentSq = XMVectorSubtract(one, XMVector3Dot(smallestComponents, smallestComponents));
		XMVECTOR largestComponent   = XMVectorSqrtEst(largestComponentSq);
		//largestComponent = XMVectorSelect( -largestComponent, largestComponent, XMVectorEqualInt( signBit, oneInt ) );
		
		// Generate basic permute constant without any loads from memory.
		XMVECTOR perm0   = __lvsr(NULL, 0);	// 0x10111213 14151617 18191a1b 1c1d1e1f
		XMVECTOR permXYZ = __lvsl(NULL, 0); // 0x00010203 04050607 08090a0b 0c0d0e0f
		
		// Rotate and shift to get 0x10111213 00010203 04050607 08090a0b
		perm0			 = XMVectorInsert(perm0, permXYZ, 3, 0, 1, 1, 1);
		
		// Generate the other 3 different permute constants.
		XMVECTOR perm1  = XMVectorSwizzle(perm0, 1, 0, 2, 3);
		XMVECTOR perm2  = XMVectorSwizzle(perm1, 0, 2, 1, 3);
		XMVECTOR perm3  = XMVectorSwizzle(perm2, 0, 1, 3, 2);

		// Pick the right one.
		XMVECTOR oneInt    = XMVectorSplatConstantInt(0x01);
		XMVECTOR twoInt	   = XMVectorSplatConstantInt(0x02);
		XMVECTOR fourInt = XMVectorSplatConstantInt(0x04);
		XMVECTOR lowIndex  = XMVectorAndInt(largestIndexAndSign, twoInt);
		XMVECTOR highIndex = XMVectorAndInt(largestIndexAndSign, fourInt);
	    
		XMVECTOR lowPerm   = XMVectorSelect(perm0, perm1, XMVectorEqualInt(lowIndex, twoInt));
		XMVECTOR highPerm  = XMVectorSelect(perm2, perm3, XMVectorEqualInt(lowIndex, twoInt));
		XMVECTOR finalPerm = XMVectorSelect(lowPerm, highPerm, XMVectorEqualInt(highIndex, fourInt));
		
		// Fix sign of largest component.
		XMVECTOR sign = XMVectorAndInt( largestIndexAndSign, oneInt );
		largestComponent = XMVectorSelect( largestComponent, -largestComponent, XMVectorEqualInt( sign, oneInt ) );

		// Do the permute.
		XMVECTOR result = XMVectorPermute(smallestComponents, largestComponent, finalPerm);
		
		return result;
	}

	FORCE_INLINE void DecompressQuaternion32( U32 quat32, Maths::Quaternion& outQuat )
	{
		static const __vector4 kUnpackConstants = { 2.f * 0.7071067811865f / 1022.f, 2.f * 0.7071067811865f / 510.f, -0.7071067811865f, 1.f };
		
		// (x, y, z, permute, sign) = 10:10:9:2:1
		__vector4 packedQuat = Load_10_10_9_3( &quat32 );
		
		__vector4 constants = *(__vector4*)&kUnpackConstants;
		__vector4 scale = __vspltw( constants, 0 );
		scale = __vrlimi( scale, constants, 8, 1 );
		//scale = XMVectorInsert( scale, constants, 1, 1, 0, 0, 0 ); 
		__vector4 offset = __vspltw( constants, 2 );
		//XMVECTOR offset = XMVectorSplatZ( constants );
			
		__vector4 result = DecompressQuaternionSmallest3_VMX( packedQuat, scale, offset );

		outQuat.m_Q.m_V = result;
	}
	
	// Load a 48-bit packed 4D vector.
	FORCE_INLINE __vector4 Load_15_15_15_3( const void* src )
	{
		static const __vector4i kConstants = { 17, 16, ( 1 << 15 ) - 1, ( 1 << 3 ) - 1 };
		
		const __vector4 v = __loadunalignedvector( src );
		
		// |                X                | |                Y                |
		// 12345678 12345678 12345678 12345678 12345678 12345678 12345678 12345678
		// ZZZZZZZZZZZZZZZZY YYYYYYYY YYYYYYXX XXXXXXXXXXXXXXWWW 00000000000000000
		
		const __vector4 constants = * (const __vector4*) &kConstants;
		const __vector4 shift17 = __vspltb( constants, 3 );
		const __vector4 shift16 = __vspltb( constants, 7 );
		const __vector4 mask15 = __vspltw( constants, 2 );
		const __vector4 mask3 = __vspltw( constants, 3 );
		const __vector4 shift2 = __vspltisb( 2 );
		const __vector4 shift13 = __vspltisb( 13 );
		const __vector4 vz = __vsr( __vsro( v, shift17), shift17 );
		const __vector4 vy = __vand( __vsr( v, shift2 ), mask15 );
		__vector4 vx = __vand( __vsl( __vslo( v, shift13 ), shift13 ), mask15 );
		const __vector4 vw = __vand( __vslo( v, shift16 ), mask3 );
		
		// merge.
		vx = __vrlimi( vx, vy, 0x4, 3 );	// insert y.
		vx = __vrlimi( vx, vz, 0x2, 2 );	// insert z.
		vx = __vrlimi( vx, vw, 0x1, 1 );	// insert w.
		
		// convert to float.
		__vector4 result = __vcfux( vx, 0 );
		
		return result;
	}
	

	FORCE_INLINE void DecompressQuaternion48( const U16 quat48[3], Maths::Quaternion& outQuat )
	{
		static const __vector4 kUnpackConstants = { 2.f * 0.7071067811865f / 32766.f, -0.7071067811865f, 1.f, 1.f };
		
		// (x, y, z, permute, sign) = 15:15:15:2:1
		const __vector4 packedQuat = Load_15_15_15_3( quat48 );

		__vector4 constants = *(const __vector4*)&kUnpackConstants;
		__vector4 scale = __vspltw( constants, 0 );
		__vector4 offset = __vspltw( constants, 1 );
			
		__vector4 result = DecompressQuaternionSmallest3_VMX( packedQuat, scale, offset );

		outQuat.m_Q.m_V = result;
	}


	FORCE_INLINE __vector4 LoadVec3U8( const void* src )
	{
		// src may be misaligned so need to do two 16-byte loads and combine result.
		__vector4 v0 = __lvlx( src, 0 );
		__vector4 v1 = __lvrx( src, 3 );
		v1 = __vsldoi( v1, v1, 13 );
		__vector4 v = __vor( v0, v1 );
		__vector4 zero = __vzero();
		v = __vmrghb( zero, v );
		v = __vmrghh( zero, v );
		__vector4 result = __vcfux( v, 0 );
		return result;	
	}

	FORCE_INLINE __vector4 LoadVec3U16( const void* src )
	{
		// src may be misaligned so need to do two 16-byte loads and combine result.
		__vector4 v0 = __lvlx( src, 0 );
		__vector4 v1 = __lvrx( src, 6 );
		v1 = __vsldoi( v1, v1, 10 );
		__vector4 v = __vor( v0, v1 );
		__vector4 zero = __vzero();
		
		//v = __vmrghb( zero, v );
		v = __vmrghh( zero, v );
		__vector4 result = __vcfux( v, 0 );
		return result;	
	}

	FORCE_INLINE __vector4 LoadVec3( const void* src )
	{
		__vector4 vr = __lvrx( src, 12 );
		__vector4 vl = __lvlx( src, 0 );
		vr = __vsldoi( vr, vr, 1 << 2 );
		__vector4 result = __vor( vl, vr );
		return result;
	}

	FORCE_INLINE void StoreVec3( __vector4 x, void* dst )
	{
		__vector4 vx = __vspltw( x, 0 );
		__vector4 vy = __vspltw( x, 1 );
		__vector4 vz = __vspltw( x, 2 );
		__stvewx( vx, dst, 0);
		__stvewx( vy, dst, 4);
		__stvewx( vz, dst, 8);
	}

	inline void DecompressVector3U8( const U8 vec3U8[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 )
	{
		static const __vector4 kInvRangeConstants = { 1.f / 254.f, 1.f / 254.f, 1.f / 254.f, 0.f };
		
		__vector4 v = LoadVec3U8( vec3U8 );
		__vector4 vOffset = LoadVec3( offset );
		__vector4 vScale = LoadVec3( scale ) * kInvRangeConstants;
		__vector4 result = __vmaddfp( v, vScale, vOffset );
		
		outVec3.m_V.m_V = result;
	}

	inline void DecompressVector3U16( const U16 vec3U16[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 )
	{
		static const __vector4 kInvRangeConstants = { 1.f / 65534.f, 1.f / 65534.f, 1.f / 65534.f, 0.f };
		
		__vector4 v = LoadVec3U16( vec3U16 );
		__vector4 vOffset = LoadVec3( offset );
		__vector4 vScale = LoadVec3( scale ) * kInvRangeConstants;
		__vector4 result = __vmaddfp( v, vScale, vOffset );

		outVec3.m_V.m_V = result;
	}

}

#endif
