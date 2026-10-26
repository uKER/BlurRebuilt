#include "stdafx.h"
#include <Animation/Tools/KeyReductionUtils.h>

namespace AnimationTools
{

namespace
{
	// Outline for keyframe format.
	template <typename TKey>
	struct C_KeyFormat
	{
		// Linearly interpolates key1 towards key2 by the specified fraction.
		static TKey Interpolate( const TKey& key1, const TKey& key2, F32 fraction );

		// Returns the error between key1 and key2.
		static F32 GetError( const TKey& key1, const TKey& key2 );
	};

	// Maths::C_Quaternion key format.
	template <>
	struct C_KeyFormat<Maths::C_Quaternion>
	{
		typedef Maths::C_Quaternion TKey;

		static Maths::C_Quaternion Interpolate( const Maths::C_Quaternion& q1, const Maths::C_Quaternion& q2, F32 fraction )
		{
			#ifdef CFG_DEBUG
			BIZ_ASSERT( fraction >= 0.f && fraction <= 1.f );
			#endif
			return q1.ShortestLerpNormalised( q2, Maths::HF32( fraction ) );
		}

		static F32 GetError( const Maths::C_Quaternion& q1, const Maths::C_Quaternion& q2 )
		{
			// Use "angle" on unit hypersphere. 
			Maths::HF32 cosHalfAngle = q1 | q2;
			return static_cast<F32>( Maths::Constants< Maths::HF32 >::One() - Maths::Abs( cosHalfAngle ) );
		}
	};

	// MathsStorage::Float3 key format.
	template <>
	struct C_KeyFormat< MathsStorage::Float3 >
	{
		typedef MathsStorage::Float3 TKey;

		static MathsStorage::Float3 Interpolate(
			const MathsStorage::Float3& v1,
			const MathsStorage::Float3& v2,
			F32 fraction )
		{
			return MathsStorage::Float3(
				v1.x + (v2.x - v1.x) * fraction,
				v1.y + (v2.y - v1.y) * fraction,
				v1.z + (v2.z - v1.z) * fraction
			);
		}

		static F32 GetError(
			const MathsStorage::Float3& v1,
			const MathsStorage::Float3& v2
		)
		{
			return Maths::Sqrt(
				Maths::Square( v2.x - v1.x ) +
				Maths::Square( v2.y - v1.y ) +
				Maths::Square( v2.z - v1.z )
				);
		}
	};

	// F32 key format.
	template <>
	struct C_KeyFormat< F32 >
	{
		typedef F32 TKey;

		static F32 Interpolate( F32 f1, F32 f2, F32 fraction )
		{
			return f1 + ( f2 - f1 ) * fraction;
		}

		static F32 GetError( F32 f1, F32 f2 )
		{
			return Maths::Abs( f2 - f1 );
		}
	};

	// Gets the maximum linear interpolation error if all keys from startKey + 1 to
	// endKey - 1 are removed. The search will finish early if maxError is exceeded.
	template < typename TKeyFormat >
	F32 GetLinearInterpolationError(
		const typename TKeyFormat::TKey* startKey,
		const typename TKeyFormat::TKey* endKey,
		F32 maxError )
	{
		S32 segmentSize = endKey - startKey;

		if ( segmentSize < 2 )
		{
			return 0.f;
		}

		F32 maxSegmentError = 0.f;
		#ifdef CFG_DEBUG
		BIZ_ASSERT( TKeyFormat::GetError( *startKey, TKeyFormat::Interpolate( *startKey, *endKey, 0.f ) ) < 0.001f );
		BIZ_ASSERT( TKeyFormat::GetError( *endKey, TKeyFormat::Interpolate( *startKey, *endKey, 1.f ) ) < 0.001f );
		#endif

		for ( S32 i = 1; i < segmentSize; ++i )
		{
			F32 interpolateFraction = static_cast<F32>(i) / segmentSize;

			TKeyFormat::TKey interpolatedKey = TKeyFormat::Interpolate( *startKey, *endKey, interpolateFraction );

			maxSegmentError = Maths::Max(
				maxSegmentError,
				TKeyFormat::GetError( interpolatedKey, startKey[ i ] )
			);

			if ( maxSegmentError > maxError )
			{
				// Maximum error exceeded: no point continuing.
				break;
			}
		}

		return maxSegmentError;
	}
	
	// Generic constant curve detector.
	template < typename TKeyFormat >
	bool IsConstantCurve(
		const typename TKeyFormat::TKey* keys,
		S32 numFrames,
		F32 maxError
	)
	{
		if ( numFrames <= 1 )
		{
			return true;
		}
		
		for ( S32 i = 1; i < numFrames; ++i )
		{
			if ( TKeyFormat::GetError( keys[0], keys[i] ) > maxError )
			{
				return false;
			}
		}
		return true;
	}

	// Generic implementation of linear key reduction.
	template < typename TKeyFormat >
	S32 GetKeyReducedCurve(
		const typename TKeyFormat::TKey* keys,
		const AnimationTools::KeyReductionUtils::FlagVector& discontinuityFlags,
		S32 numFrames,
		F32 maxError,
		Btl::Vector< typename TKeyFormat::TKey >& outKeys,
		AnimationTools::KeyReductionUtils::KeyIndexVector& outKeyIndices
	)
	{
		outKeys.Clear();
		outKeys.Reserve( numFrames );
		outKeyIndices.Clear();
		outKeyIndices.Reserve( numFrames );
		
		BIZ_ASSERT(discontinuityFlags.Size() == numFrames);
		
		// Always keep first key.
		outKeys.PushBack( keys[0] );
		outKeyIndices.PushBack( 0 );
		
		#define CONSTANT_CHECK 1
		
	#ifdef CONSTANT_CHECK
		if ( IsConstantCurve<TKeyFormat>( keys, numFrames, maxError ) )
		{
			// Nothing more to do.
			return 1;
		}
		
	#else
		if ( numFrames == 1 )
		{
			// Nothing more to do.
			return 1;
		}
	#endif

		if ( numFrames == 2 )
		{
			volatile F32 error = TKeyFormat::GetError( keys[0], keys[1] );
			if ( error <= maxError )
			{
				#if defined(CONSTANT_CHECK)
				//assert(false); // should have been handled with IsConstantCurve() test
				#endif
				// Both keys are the same.
				return 1;
			}
			else
			{
				// Two different keys.
				outKeys.PushBack( keys[1] );
				outKeyIndices.PushBack( 1 );
				return 2;
			}
		}

		// Three or greater keys.
		BIZ_ASSERT( numFrames >= 3 );

		// This is the index of the last emitted key.
		S32 startKeyIndex = 0;
		U32 numSteps = 0;
		
		while ( startKeyIndex < numFrames - 1 )
		{
			S32 endKeyIndex = startKeyIndex + 1;
			
			bool foundStep = discontinuityFlags[endKeyIndex];
			
			// Find longest sequence of keys that can be represented as a linear interpolation within
			// the acceptable error bounds, maxError. Note that discontinuities don't interrupt this
			// process so that a sequence of stepped tangents with the same key value will still get key reduced.
			for ( S32 trialEndKeyIndex = startKeyIndex + 2; trialEndKeyIndex < numFrames; ++trialEndKeyIndex)
			{
				F32 maxSegmentError = GetLinearInterpolationError< TKeyFormat >(
					&keys[ startKeyIndex ],
					&keys[ trialEndKeyIndex ],
					maxError
				);

				if ( maxSegmentError <= maxError )
				{
					// Segment is within error bounds.
					endKeyIndex = trialEndKeyIndex;
					
					foundStep = false; // foundStep || discontinuityFlags[endKeyIndex];
				}
				else
				{
					// Segment exceeded maximum error - abandon search and use last acceptable.
					// It might actually be the case that we could get a segment _within_
					// our error threshold if we continued the search beyond this point.
					// Would get pretty expensive though.
					break;
				}
			}
			
			bool storeEndOfSegment = true;

			// If segment covered entire curve, check to see if we can just store a single key.
			if ( endKeyIndex == (numFrames - 1) && startKeyIndex == 0 )
			{
				if ( TKeyFormat::GetError(keys[0], keys[numFrames - 1]) <= maxError )
				{
					// Within our error limit - just store a single key.
					storeEndOfSegment = false;
				}
			}
			
			if ( storeEndOfSegment )
			{
				// Start key immediately switches to end key without interpolation.
				// To make this happen, emit (start_key, end_key_time), (end_key, end_key_time) key value pairs.
				if (foundStep)
				{
					// Emit (start_key, end_key_time)
					outKeys.PushBack(keys[startKeyIndex]);
					outKeyIndices.PushBack(endKeyIndex);
					++numSteps;
				}
			
				// Emit (end_key, end_key_time)
				outKeys.PushBack(keys[endKeyIndex]);
				outKeyIndices.PushBack(endKeyIndex);
			}
			startKeyIndex = endKeyIndex;
		}

		BIZ_ASSERT( outKeys.Size() == outKeyIndices.Size() );
		BIZ_ASSERT( (U32) outKeys.Size() <= numFrames + numSteps );

		return outKeys.Size();
	}
}

//
// KeyReductionUtils
//

S32 KeyReductionUtils::GetKeyReducedQuaternionCurve(
	const Maths::C_Quaternion* quaternionKeys,
	const FlagVector& discontinuityFlags,
	S32 numFrames,
	F32 maxRotationErrorInDegrees,
	QuaternionVector& outQuaternionKeys,
	KeyIndexVector& outKeyIndices
)
{
	F32 maxError = 1.f - cosf( 0.5f * Maths::DegToRad( maxRotationErrorInDegrees ) );

	return GetKeyReducedCurve< C_KeyFormat< Maths::C_Quaternion > >(
		quaternionKeys, 
		discontinuityFlags,
		numFrames,
		maxError,
		outQuaternionKeys,
		outKeyIndices
		);
}

S32 KeyReductionUtils::GetKeyReducedVector3Curve(
	const MathsStorage::Float3* vector3Keys,
	const FlagVector& discontinuityFlags,
	S32 numFrames,
	F32 maxError,
	Float3Vector& outVector3Keys,
	KeyIndexVector& outKeyIndices
)
{
	return GetKeyReducedCurve< C_KeyFormat< MathsStorage::Float3 > >(
		vector3Keys,
		discontinuityFlags,
		numFrames,
		maxError,
		outVector3Keys,
		outKeyIndices
	);
}

S32 KeyReductionUtils::GetKeyReducedScalarCurve(
	const F32* scalarKeys,
	const FlagVector& discontinuityFlags,
	S32 numFrames,
	F32 maxError,
	FloatVector& outScalarKeys,
	KeyIndexVector& outKeyIndices
)
{
	return GetKeyReducedCurve< C_KeyFormat< F32 > >(
		scalarKeys,
		discontinuityFlags,
		numFrames,
		maxError,
		outScalarKeys,
		outKeyIndices
	);
}

} // namespace Animation
