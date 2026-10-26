#include "stdafx.h"
#include <animation/tools/CompressedCurveUtils.h>
#include <animation/tools/Curve.h>
#include <System/Misc/Random.h>

namespace AnimationTools
{

namespace CompressedCurveUtils
{

Debug::Logger& gLog = Debug::GetLogger("animation");

// Empirically measured error constants (calculated by TestQuaternionCompression).
const F32 kMaxSingleAxisQuaternion16ErrorInDegrees = 0.2655f;	// average: 0.00783868 Use Smallest of W and axis to improve this.
// POSITIVE W: static const F32 kMaxQuaternion32ErrorInDegrees = 0.143f;	// average: 0.076086
const F32 kMaxQuaternion32ErrorInDegrees = 0.306f;	// average: 0.113955
const F32 kMaxQuaternion48ErrorInDegrees = 0.0690f;	// average: 0.00773703 

//
// CompressedCurveUtils
//

bool CompressedCurveUtils::BuildCompressedQuaternionCurve(
	const Maths::C_Quaternion* quatKeys,
	const U32* keyTimes,
	U32 numKeys,
	U32 durationInFrames,
	F32 maxRotationErrorInDegrees,
	C_Curve& curve
)
{
	E_KeyType keyType= E_KeyType_QuaternionF32;
	
	bool isVarying = numKeys > 1;
	
	// *** Excluding single axis formats for the time being to keep code complexity down.
#if 0
	if ( isVarying )
	{
		// Try and use a single axis format.
		F32 rotationThreshold = 1.f / 16384.f;
		U32 axisFlags = FindSignificantRotationAxes( quatKeys, numKeys, rotationThreshold );
		
		switch ( axisFlags )
		{
			case 0: keyType = E_KeyType_QuaternionX16; break; // Any format will do, but this is the smallest.
			case E_AxisFlag_X: keyType = E_KeyType_QuaternionX16; break;
			case E_AxisFlag_Y: keyType = E_KeyType_QuaternionY16; break;
			case E_AxisFlag_Z: keyType = E_KeyType_QuaternionZ16; break;
			default: break;
		}
	}
#endif	

	if ( keyType == E_KeyType_QuaternionF32 )
	{
		if ( isVarying )
		{
				// General rotation: choose most compact format within error limits.
			if ( maxRotationErrorInDegrees >= kMaxQuaternion32ErrorInDegrees )
			{
				keyType = E_KeyType_Quaternion32;
			}
			else if ( maxRotationErrorInDegrees >= kMaxQuaternion48ErrorInDegrees )
			{
				keyType = E_KeyType_Quaternion48;
			}
			else
			{
			#ifdef CFG_DEBUG
				LOGWARN(
					Debug::GetLogger( "animation" ),
					"Requested maxRotationError (%f) cannot be achieved with existing compressed curve formats (best possible is %f).\n",
					maxRotationErrorInDegrees,
					kMaxQuaternion48ErrorInDegrees
				);
			#endif
				keyType = E_KeyType_Quaternion48;
			}
		}
		else
		{
			// Use 48-bit quaternions for constant channels.
			keyType = E_KeyType_Quaternion48;
		}
	}

	
	bool ok = curve.CreateQuaternionCurve( keyType, numKeys, keyTimes, quatKeys );	
	
	return ok;
}

bool BuildCompressedVectorCurve(
	const F32* keys,
	const U32* keyTimes,
	U32 numKeys,
	U32 numDimensions,
	U32 durationInFrames,
	F32 maxError,
	bool useRelativeError,
	const F32* offset,
	const F32* scale,
	C_Curve& outCurve
)
{
	if ( numDimensions > 4 )
	{
		BIZ_ASSERT( false );
		return false;
	}
	
	if ( 0 == numDimensions )
	{
		BIZ_ASSERT( false );
		return false;
	}

	F32 threshold = 1.f / 32768.f; // TODO: pass zero threshold in to.
	
	// Calculate maximum extent of key data along any of its dimensions.
	F32 maxExtent = 0.f;
		
	for ( U32 dimIndex = 0; dimIndex < numDimensions; ++dimIndex )
	{
		F32 extent = Maths::Abs( scale[ dimIndex ] );
		if ( extent > maxExtent )
		{
			maxExtent = extent;
		}
	}
	
	F32 absMaxError = maxError;
	
	if ( useRelativeError )
	{
		absMaxError = maxError * maxExtent;
	}
	
	E_KeyType keyType = E_KeyType_Invalid;
	switch ( numDimensions )
	{
		case 1:
		{
			keyType = E_KeyType_ScalarF32;
			return outCurve.CreateScalarCurve( keyType, numKeys, keyTimes, keys );
			break;
		}
	
		case 3:
		{
			keyType = E_KeyType_Vector3F32;

			if ( numKeys >= 3 )
			{
				// Choose between U8 and U16 formats.
				if ( ( maxExtent / 254.f ) < absMaxError )
				{
					keyType = E_KeyType_Vector3U8;
				}
				else if ( ( maxExtent / 65534.f ) < absMaxError )
				{
					keyType = E_KeyType_Vector3U16;
				}
			}
			
			return outCurve.CreateVector3Curve( keyType, numKeys, keyTimes, (const MathsStorage::Float3*) keys, offset, scale );
			break;
		}
		
		default:
			break;
	}
	
	return false;
}

bool CompressedCurveUtils::BuildCompressedVector3Curve(
	const MathsStorage::Float3* vec3Keys,
	const U32* keyTimes,
	U32 numKeys,
	U32 durationInFrames,
	F32 maxError,
	bool useRelativeError,
	const F32* offset,
	const F32* scale,
	C_Curve& outCurve
)
{
	return BuildCompressedVectorCurve( vec3Keys->v, keyTimes, numKeys, 3, durationInFrames, maxError, useRelativeError, offset, scale, outCurve );
}

bool CompressedCurveUtils::BuildCompressedScalarCurve(
	const F32* scalarKeys,
	const U32* keyTimes,
	U32 numKeys,
	U32 durationInFrames,
	F32 maxError,
	bool useRelativeError,
	F32 offset,
	F32 scale,
	C_Curve& outCurve
)
{
	return BuildCompressedVectorCurve( scalarKeys, keyTimes, numKeys, 1, durationInFrames, maxError, useRelativeError, &offset, &scale, outCurve );
}


U32 CompressedCurveUtils::FindSignificantRotationAxes(
	const Maths::C_Quaternion* quats,
	U32 numQuats,
	F32 threshold
)
{
	BIZ_STATIC_ASSERT( E_AxisFlag_X == 1 );
	BIZ_STATIC_ASSERT( E_AxisFlag_Y == 2 );
	BIZ_STATIC_ASSERT( E_AxisFlag_Z == 4 );

	U32 flags = 0;
	for ( U32 i = 0; i < numQuats; ++i )
	{
		MathsStorage::Float4 q4;
		Maths::Store< Maths::C_Quaternion, MathsStorage::Float4 >( quats[i], &q4 );

		for ( U32 j = 0; j < 3; ++j )
		{
			if ( Maths::Abs( q4.v[ j ] ) >= threshold )
			{
				flags |= ( 1 << j );
			}
		}
	}
	return flags;
}

bool CompressedCurveUtils::RotationsEqualWithinTolerance(
	const Maths::C_Quaternion& rotationA,
	const Maths::C_Quaternion& rotationB,
	F32 maxRotationErrorInDegrees
)
{
	F32 maxError = 1.f - cosf( 0.5f * Maths::DegToRad( maxRotationErrorInDegrees ) );
	Maths::HF32 cosHalfAngle = rotationA | rotationB;
	Maths::HF32 error = Maths::Constants<Maths::HF32>::One() - Maths::Abs( cosHalfAngle );
	return static_cast<F32>( error ) <= maxError;
}
	
bool CompressedCurveUtils::VectorsEqualWithinTolerance(
	const F32* vectorA,
	const F32* vectorB,
	U32 numDimensions,
	F32 maxError
)
{
	if ( numDimensions <= 0 )
	{
		return false;
	}
	
	F32 l2Norm = 0.f;
	
	for ( U32 i = 0; i < numDimensions; ++i )
	{
		F32 delta = vectorB[ i ] - vectorA[ i ];
		l2Norm += delta * delta;
	}
	
	F32 maxErrorSquared = maxError * maxError;
	
	return ( l2Norm <= maxErrorSquared );
}

void CompressedCurveUtils::CompressSingleAxisQuaternion16( const Maths::C_Quaternion& quat, E_QuaternionAxis axis, U16& outQuat16 )
{
	MathsStorage::Float4 q;
	Maths::Store< Maths::C_Quaternion, MathsStorage::Float4 >( quat.Normalised(), &q );

	U32 wSign = 0;
	
	if ( q.w < 0.f )
	{
		wSign = 1;
		//q.v[ axis ] = -q.v[ axis ];
	}
	
	S32 iAxis = static_cast<S32>( ( 32766.f * ( q.v[ axis ] * 0.5f + 0.5f ) ) + 0.5f );

	BIZ_ASSERT( iAxis < 32767 && iAxis >= 0 );
	
	outQuat16 = static_cast<U16>( ( iAxis << 1 ) | wSign );
}

void CompressedCurveUtils::DecompressSingleAxisQuaternion16( U16 quat16, E_QuaternionAxis axis, Maths::C_Quaternion& outQuat )
{
	MathsStorage::Float4 q;

	S32 iAxis = quat16;

	q.x = 0.f;
	q.y = 0.f;
	q.z = 0.f;

	q.v[ axis ] = 2.f * ( ( iAxis >> 1 ) / 32766.f ) - 1.f;
	BIZ_ASSERT( (q.v[ axis ] * q.v[ axis ]) <= 1.f );
	q.w = Maths::Sqrt( 1.f - q.v[ axis ] * q.v[ axis ] );
	
	if ( iAxis & 1 )
	{
		q.w = -q.w;
	}

	outQuat = Maths::Load< Maths::C_Quaternion, MathsStorage::Float4 >( &q );
}

static const F32 kRootTwo = 1.414213562373f;
static const F32 kOneOverRootTwo = 1.f / kRootTwo;
static const F32 kOneOverRootTwoApprox = 0.707107f;

namespace
{
	void CompressQuaternionSmallest3(
		const Maths::C_Quaternion& quat,
		const U32 smallest3Ranges[ 3 ],
		U32 outSmallest3Components[ 3 ],
		U32& outLargestIndex,
		U32& outLargestComponentSign
	)
	{
		MathsStorage::Float4 q;
		Maths::Store< Maths::C_Quaternion, MathsStorage::Float4 >( quat.Normalised(), &q );
					
		// Find largest component.
		outLargestIndex = 0;
		for ( U32 i = 0; i < 4; ++i )
		{
			if ( Maths::Abs( q.v[ i ] ) > Maths::Abs( q.v[ outLargestIndex ] ) )
			{
				outLargestIndex = i;
			}
		}

		// Make sure largest component is positive.
		if ( q.v[ outLargestIndex ] < 0.f )
		{
			outLargestComponentSign = 1;
			/*
			q.x = -q.x;
			q.y = -q.y;
			q.z = -q.z;
			q.w = -q.w;
			*/
		}
		else
		{
			outLargestComponentSign = 0;
		}

		// Convert 3 smallest components into integers in the specified range.
		// Scale by sqrt(2), since the largest component can only be 1/sqrt(2)
		// give or take floating point representation errors.
		

		U32 componentIndex = 0;
		for ( U32 i = 0; i < 4; ++i )
		{
			if ( i != outLargestIndex )
			{
				const U32 range = smallest3Ranges[ componentIndex ];
				const F32 fRange = static_cast<F32>( range );
				
				BIZ_ASSERT( Maths::Abs( q.v[ i ] ) <= kOneOverRootTwoApprox );
				
				// Map into range 0-1.
				F32 scaledAndBiased = 0.5f + 0.5f * ( q.v[ i ] * kRootTwo );
				
				// Clamp for safety.
				scaledAndBiased = Maths::Clamped( scaledAndBiased, 0.f, 1.f );
				
				// Scale into integer range 0 - 1022.
				U32 iValue = static_cast< U32 >( fRange * scaledAndBiased + 0.5f );
				BIZ_ASSERT( iValue <= range );

				outSmallest3Components[ componentIndex ] = iValue;
				++componentIndex;
			}
		}
	}

	void DecompressQuaternionSmallest3(
		U32 largestIndex,
		U32 largestSign,
		const U32 smallest3Ranges[ 3 ],
		U32 smallest3[ 3 ],
		Maths::C_Quaternion& outQuat
	)
	{
		const F32 fRange0 = static_cast<F32>( smallest3Ranges[ 0 ] );
		const F32 fRange1 = static_cast<F32>( smallest3Ranges[ 1 ] );
		const F32 fRange2 = static_cast<F32>( smallest3Ranges[ 2 ] );

		S32 iC0 = smallest3[ 0 ];
		S32 iC1 = smallest3[ 1 ];
		S32 iC2 = smallest3[ 2 ];
		
		F32 fC0 = kOneOverRootTwo * ( 2.f * ( iC0 / fRange0 ) - 1.f );
		F32 fC1 = kOneOverRootTwo * ( 2.f * ( iC1 / fRange1 ) - 1.f );
		F32 fC2 = kOneOverRootTwo * ( 2.f * ( iC2 / fRange2 ) - 1.f );

		MathsStorage::Float4 q;

		switch ( largestIndex )
		{
			case 0:
				q.y = fC0;
				q.z = fC1;
				q.w = fC2;
				break;
			case 1:
				q.x = fC0;
				q.z = fC1;
				q.w = fC2;
				break;
			case 2:
				q.x = fC0;
				q.y = fC1;
				q.w = fC2;
				break;
			case 3:
				q.x = fC0;
				q.y = fC1;
				q.z = fC2;
				break;
			default: BIZ_ASSERT( false ); break;
		}

		F32 largestSquared = 1.f - ( fC0 * fC0 + fC1 * fC1 + fC2 * fC2 );

		if ( largestSquared < 0.f || largestSquared > 1.f )
		{
			volatile int breakme = 1;
		}

		BIZ_ASSERT( largestSquared >= 0.f && largestSquared <= 1.f );

		q.v[ largestIndex ] = Maths::Sqrt( largestSquared );
		
		if ( largestSign )
		{
			q.v[ largestIndex ] = -q.v[ largestIndex ];
		}

		outQuat = Maths::Load< Maths::C_Quaternion, MathsStorage::Float4 >( &q );
	}

}

namespace
{

	static const U32 kMask15 = ( 1 << 15 ) - 1;
	static const U32 kMask10 = ( 1 << 10 ) - 1;
	static const U32 kMask9 = ( 1 << 9 ) - 1;

	static const U32 kQuaternion32Ranges[ 3 ] =
	{
		510,
		1022,
		1022
	};
	
	static const U32 kQuaternion48Ranges[ 3 ] =
	{
		32766,
		32766,
		32766
	};
	
}

void CompressedCurveUtils::CompressQuaternion32( const Maths::C_Quaternion& quat, U32& outQuat32 )
{
	U32 smallest3[ 3 ];
	U32 largestIndex = 0;
	
	// 2:10:10:10 = no space for sign! sign important to detect rotations greater than 180 degrees etc.
	
	U32 largestSign = 0;
	
	CompressQuaternionSmallest3( quat, kQuaternion32Ranges, smallest3, largestIndex, largestSign );

	static const U32 mask10 = (1 << 10) - 1;
	static const U32 mask9 = (1 << 9) - 1;

	outQuat32 = 
		( ( smallest3[ 2 ] & kMask10 ) << 22 ) |
		( ( smallest3[ 1 ] & kMask10 ) << 12 ) |
		( ( smallest3[ 0 ] & kMask9 ) << 3 ) |
		( ( largestIndex & 3 ) << 1 ) |
		( largestSign & 1 );
}

void CompressedCurveUtils::DecompressQuaternion32( U32 quat32, Maths::C_Quaternion& outQuat )
{
	U32 largestIndex = ( quat32 >> 1 ) & 0x3;

	U32 smallest3[ 3 ] =
	{
		( quat32 >> 3  ) & kMask9,
		( quat32 >> 12 ) & kMask10,
		( quat32 >> 22 ) & kMask10
	};

	const U32 largestSign = quat32 & 1;
	
	DecompressQuaternionSmallest3( largestIndex, largestSign, kQuaternion32Ranges, smallest3, outQuat );
}

void CompressedCurveUtils::CompressQuaternion48( const Maths::C_Quaternion& quat, U16 outQuat48[3] )
{
	U32 smallest3[ 3 ];
	U32 largestIndex = 0;
	U32 largestSign = 0;

	// 15 bits each.
	
	CompressQuaternionSmallest3( quat, kQuaternion48Ranges, smallest3, largestIndex, largestSign );

	U64 result = 
		  ( static_cast<U64>( smallest3[ 0 ] & kMask15 ) << 3 )
		| ( static_cast<U64>( smallest3[ 1 ] & kMask15 ) << 18 )
		| ( static_cast<U64>( smallest3[ 2 ] & kMask15 ) << 33 )
		| ( static_cast<U64>( largestIndex & 3 ) << 1 )
		| ( static_cast<U64>( largestSign & 1 ) );

	outQuat48[0] = static_cast<U16>( ( result >> 32 ) & 0xffff );
	outQuat48[1] = static_cast<U16>( ( result >> 16 ) & 0xffff );
	outQuat48[2] = static_cast<U16>( result & 0xffff );
}

void CompressedCurveUtils::DecompressQuaternion48( const U16 quat48[3], Maths::C_Quaternion& outQuat )
{
	U64 src = static_cast<U64>( quat48[ 2 ] )
		| ( static_cast<U64>( quat48[ 1 ] ) << 16 )
		| ( static_cast<U64>( quat48[ 0 ] ) << 32 );

	static const U64 mask = kMask15;

	U32 largestSign = static_cast<U32>( src & 1 );
	U32 largestIndex = static_cast<U32>( ( src >> 1 ) & 3 );
	U32 smallest3[ 3 ] =
	{
		static_cast<U32>( ( src >> 3 ) & mask ),
		static_cast<U32>( ( src >> 18 ) & mask ),
		static_cast<U32>( ( src >> 33 ) & mask )
	};

	DecompressQuaternionSmallest3( largestIndex, largestSign, kQuaternion48Ranges, smallest3, outQuat );
}

void CompressedCurveUtils::CompressVector3U8( const F32 vector3[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], U8 outVec3U8[ 3 ] )
{
	static const U32 range = 254;
	static const F32 fRange = static_cast< F32 >( range );
	
	U32 iX = static_cast< U32 >( fRange * ( (vector3[ 0 ] - offset[ 0 ] ) / scale[ 0 ] ) + 0.5f );
	U32 iY = static_cast< U32 >( fRange * ( (vector3[ 1 ] - offset[ 1 ] ) / scale[ 1 ] ) + 0.5f );
	U32 iZ = static_cast< U32 >( fRange * ( (vector3[ 2 ] - offset[ 2 ] ) / scale[ 2 ] ) + 0.5f );

	BIZ_ASSERT( iX <= range );
	BIZ_ASSERT( iY <= range );
	BIZ_ASSERT( iZ <= range );

	outVec3U8[ 0 ] = iX;
	outVec3U8[ 1 ] = iY;
	outVec3U8[ 2 ] = iZ;
}

void CompressedCurveUtils::DecompressVector3U8( const U8 vec3U8[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], F32 outVec3[ 3 ] )
{
	static const F32 fRange = 254.f;
	outVec3[ 0 ] = offset[ 0 ] + scale[ 0 ] * ( vec3U8[ 0 ] / fRange );
	outVec3[ 1 ] = offset[ 1 ] + scale[ 1 ] * ( vec3U8[ 1 ] / fRange );
	outVec3[ 2 ] = offset[ 2 ] + scale[ 2 ] * ( vec3U8[ 2 ] / fRange );
}

void CompressedCurveUtils::CompressVector3U16( const F32 vector3[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], U16 outVec3U16[ 3 ] )
{
	static const U32 range = 65534;
	static const F32 fRange = static_cast< F32 >( range );
	
	U32 iX = static_cast< U32 >( fRange * ( (vector3[ 0 ] - offset[ 0 ] ) / scale[ 0 ] ) + 0.5f );
	U32 iY = static_cast< U32 >( fRange * ( (vector3[ 1 ] - offset[ 1 ] ) / scale[ 1 ] ) + 0.5f );
	U32 iZ = static_cast< U32 >( fRange * ( (vector3[ 2 ] - offset[ 2 ] ) / scale[ 2 ] ) + 0.5f );

	BIZ_ASSERT( iX <= range );
	BIZ_ASSERT( iY <= range );
	BIZ_ASSERT( iZ <= range );

	outVec3U16[ 0 ] = iX;
	outVec3U16[ 1 ] = iY;
	outVec3U16[ 2 ] = iZ;
}

void CompressedCurveUtils::DecompressVector3U16( const U16 vec3U16[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], F32 outVec3[ 3 ] )
{
	static const F32 fRange = 65534.f;
	outVec3[ 0 ] = offset[ 0 ] + scale[ 0 ] * ( vec3U16[ 0 ] / fRange );
	outVec3[ 1 ] = offset[ 1 ] + scale[ 1 ] * ( vec3U16[ 1 ] / fRange );
	outVec3[ 2 ] = offset[ 2 ] + scale[ 2 ] * ( vec3U16[ 2 ] / fRange );
}


template < typename T >	
U32 BinarySearch( T x, const T* values, U32 count )
{
	U32 left = 0;
	U32 right = count;

	// Find first keyTime > iTime.
	while ( left < right )
	{
		U32 midIndex = left + ( ( right - left ) / 2 );
		T midKeyTime = values[ midIndex ];

		if ( midKeyTime > x )
		{
			right = midIndex;
		}
		else
		{
			left = midIndex + 1;
		}
	}
	
	return right;
}



void CompressedCurveUtils::FindKeyIndexForTime(
	const U32* keyTimes,
	U32 numKeys,
	F32 time,
	U32& outNextKeyIndex,
	F32& outInterpolationFraction
)
{
	U32 timeInteger = static_cast< U32 >( time );

	const U32 nextKeyIndex = BinarySearch< U32 >( timeInteger, keyTimes, numKeys );
	
	outNextKeyIndex = nextKeyIndex;
	
	if ( nextKeyIndex < 1 )
	{
		outInterpolationFraction = 0.f;
		return;
	}
	
	if ( nextKeyIndex < numKeys )
	{
		const U32 prevTime = keyTimes[ nextKeyIndex - 1 ];
		const U32 nextTime = keyTimes[ nextKeyIndex ];
		outInterpolationFraction = ( time - prevTime ) / static_cast< F32 >( nextTime - prevTime );
		return;
	}

	outInterpolationFraction = 1.f;

	return;
}

} // namespace CompressedCurveUtils

} // namespace Animation
