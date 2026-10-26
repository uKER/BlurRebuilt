#ifndef ANIMATION_DECOMPRESSIONGENERIC_INL
#define ANIMATION_DECOMPRESSIONGENERIC_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <core/maths/hvec/storage.h>

namespace Animation
{
	static const F32 kRootTwo = 1.414213562373f;
	static const F32 kOneOverRootTwo = 1.f / kRootTwo;
	static const F32 kOneOverRootTwoApprox = 0.707107f;

	inline void DecompressQuaternion32( U32 quat32, Maths::Quaternion& outQuat )
	{
		static const U32 kMask10 = ( 1 << 10 ) - 1;
		static const U32 kMask9 = ( 1 << 9 ) - 1;

	
		U32 largestSign = quat32 & 1;
		U32 largestIndex = ( quat32 >> 1 ) & 0x3;
		S32 iC0 = ( quat32 >> 3  ) & kMask9;
		S32 iC1 = ( quat32 >> 12 ) & kMask10;
		S32 iC2 = ( quat32 >> 22 ) & kMask10;
			
		static const F32 fRange0 = 510.f;
		static const F32 fRange1 = 1022.f;
		static const F32 fRange2 = 1022.f;
		
		F32 fC0 = kOneOverRootTwo * ( 2.f * ( iC0 / fRange0 ) - 1.f );
		F32 fC1 = kOneOverRootTwo * ( 2.f * ( iC1 / fRange1 ) - 1.f );
		F32 fC2 = kOneOverRootTwo * ( 2.f * ( iC2 / fRange2 ) - 1.f );

		Maths::StorageVec4F32 q;
		
		static const U32 permutes[4][3] =
		{
			{ 1, 2, 3 },
			{ 0, 2, 3 },
			{ 0, 1, 3 },
			{ 0, 1, 2 }
		};
		
		q.m_Data.v[ permutes[ largestIndex ][ 0 ] ] = fC0;
		q.m_Data.v[ permutes[ largestIndex ][ 1 ] ] = fC1;
		q.m_Data.v[ permutes[ largestIndex ][ 2 ] ] = fC2;

		F32 largestSquared = 1.f - ( fC0 * fC0 + fC1 * fC1 + fC2 * fC2 );

		BIZ_ASSERT( largestSquared >= 0.f && largestSquared <= 1.f );

		F32 largestValue = Maths::Sqrt( largestSquared );
		q.m_Data.v[ largestIndex ] = largestSign ? -largestValue : largestValue;

		outQuat = Maths::MakeQuaternion( Maths::Load<Maths::Vec4F32>( &q.m_Data ) );
	}
		
	inline void DecompressQuaternion48( const U16 quat48[3], Maths::Quaternion& outQuat )
	{
		U64 src = static_cast<U64>( quat48[ 2 ] )
			| ( static_cast<U64>( quat48[ 1 ] ) << 16 )
			| ( static_cast<U64>( quat48[ 0 ] ) << 32 );

		//static const U64 mask = kMask15;
		static const U32 kMask15 = ( 1 << 15 ) - 1;

		U32 largestSign = static_cast<U32>( src & 1 );
		U32 largestIndex = static_cast<U32>( ( src >> 1 ) & 3 );
		
		
		
		S32 iC0 = static_cast<U32>( ( src >> 3 ) & kMask15 );
		S32 iC1 = static_cast<U32>( ( src >> 18 ) & kMask15 );
		S32 iC2 = static_cast<U32>( ( src >> 33 ) & kMask15 );
			
		static const F32 fRange = 32766.f;
		
		F32 fC0 = kOneOverRootTwo * ( 2.f * ( iC0 / fRange ) - 1.f );
		F32 fC1 = kOneOverRootTwo * ( 2.f * ( iC1 / fRange ) - 1.f );
		F32 fC2 = kOneOverRootTwo * ( 2.f * ( iC2 / fRange ) - 1.f );

		Maths::StorageVec4F32 q;
		
		static const U32 permutes[4][3] =
		{
			{ 1, 2, 3 },
			{ 0, 2, 3 },
			{ 0, 1, 3 },
			{ 0, 1, 2 }
		};
		
		q.m_Data.v[ permutes[ largestIndex ][ 0 ] ] = fC0;
		q.m_Data.v[ permutes[ largestIndex ][ 1 ] ] = fC1;
		q.m_Data.v[ permutes[ largestIndex ][ 2 ] ] = fC2;

		F32 largestSquared = 1.f - ( fC0 * fC0 + fC1 * fC1 + fC2 * fC2 );

		BIZ_ASSERT( largestSquared >= 0.f && largestSquared <= 1.f );

		F32 largestValue = Maths::Sqrt( largestSquared );
		q.m_Data.v[ largestIndex ] = largestSign ? -largestValue : largestValue;

		outQuat = Maths::MakeQuaternion( Maths::Load<Maths::Vec4F32>( &q.m_Data ) );		
		
	}
	
	inline void DecompressVector3U8( const U8 vec3U8[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 )
	{
		static const F32 fRange = 254.f;
		F32 x = offset[ 0 ] + scale[ 0 ] * ( vec3U8[ 0 ] / fRange );
		F32 y = offset[ 1 ] + scale[ 1 ] * ( vec3U8[ 1 ] / fRange );
		F32 z = offset[ 2 ] + scale[ 2 ] * ( vec3U8[ 2 ] / fRange );
		outVec3 = Maths::MakeVector3( x, y, z );
	}

	inline void DecompressVector3U16( const U16 vec3U16[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], Maths::Vector3& outVec3 )
	{
		static const F32 fRange = 65534.f;
		F32 x = offset[ 0 ] + scale[ 0 ] * ( vec3U16[ 0 ] / fRange );
		F32 y = offset[ 1 ] + scale[ 1 ] * ( vec3U16[ 1 ] / fRange );
		F32 z = offset[ 2 ] + scale[ 2 ] * ( vec3U16[ 2 ] / fRange );
		outVec3 = Maths::MakeVector3( x, y, z );
	}
}

#endif
