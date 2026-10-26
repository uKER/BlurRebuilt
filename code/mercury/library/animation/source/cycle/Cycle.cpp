#include "stdafx.h"
#include <animation/cycle/Cycle.h>
#include <animation/cycle/CycleInstance.h>
#include "CycleMapping.h"
#include "Decompression.h"
#include <animation/common/StringId.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/MemoryBlock.h>
#include <animation/common/Stats.h>
#include <animation/pose/Pose.h>
#include "pose/Pose.inl"
#include "pose/BlendFunctions.h"
#include "pose/BlendFunctions.inl"

#include <system/serialise.h>
#include <system/memory/MemoryTracker.h>
#include <system/Perf.h>
#include <system/scheduler.h>
#include <btl/utility/types.h>

namespace Animation
{

#ifdef PLATFORM_PS3
#define BRANCH_HINT_LIKELY( condition )		__builtin_expect( condition, 1 )
#define BRANCH_HINT_UNLIKELY( condition )	__builtin_expect( condition, 0 )
#else
#define BRANCH_HINT_LIKELY( condition )		condition
#define BRANCH_HINT_UNLIKELY( condition )	condition
#endif

//
// C_Cycle
//

#ifndef PLATFORM_SPU

C_Cycle::C_Cycle():
	m_playbackDuration( 0.f ),
	m_frameRate( 0.f ),
	m_numFrames( 0 ),
	m_numFields( 0 ),
	m_fieldTypes( 0 ),
	m_totalConstantCurves( 0 ),
	m_totalVaryingCurves( 0 ),
	m_channelInfo( 0 ),
	m_constantCurveDataSize( 0 ),
	m_constantCurveData( 0 ),
	m_numSegments( 0 ),
	m_totalSegmentDataSize( 0 ),
	m_segmentData( 0 ),
	m_segments( 0 ),
	m_mappings( NULL ),
	m_isPartiallyLoaded(0)
{
	for ( U32 i = 0; i < E_VaryingCurveType_Count; ++i )
	{
		m_numVaryingCurvesOfType[ i ] = 0;
	}
	for ( U32 i = 0; i < E_ConstantCurveType_Count; ++i )
	{
		m_numConstantCurvesOfType[ i ] = 0;
	}
	C_PoseLayout::GetTrajectoryPoseLayout()->AddRef();

	ANIMATION_STATS_UPDATE(numCycles++);
	ANIMATION_STATS_UPDATE(memoryUsedByCycles += sizeof(C_Cycle));
}

C_Cycle::~C_Cycle()
{
	Destroy();
	C_PoseLayout::GetTrajectoryPoseLayout()->DecRef();

	ANIMATION_STATS_UPDATE(numCycles--);
	ANIMATION_STATS_UPDATE(memoryUsedByCycles -= sizeof(C_Cycle));
}

void C_Cycle::Destroy()
{
	ANIMATION_STATS_UPDATE(memoryUsedByCycles -= GetMemoryUsage() - sizeof(C_Cycle));

	delete[] m_segments;
	m_segments = 0;
	
	delete[] m_segmentData;
	m_segmentData = 0;
	
	m_totalSegmentDataSize = 0;
	m_numSegments = 0;
	
	delete[] m_constantCurveData;
	m_constantCurveData = 0;
	
	m_constantCurveDataSize = 0;
	
	delete[] m_channelInfo;
	m_channelInfo = 0;
	
	m_totalVaryingCurves = 0;
	m_totalConstantCurves = 0;
	
	m_numFields = 0;
	m_fieldNames.Clear();
	
	delete[] m_fieldTypes;
	m_fieldTypes = 0;
	
	m_numFrames = 0;
	m_playbackDuration = 0.f;
	m_frameRate = 0;
	m_isPartiallyLoaded = 0;
	
	ClearMappingCache();
}

#endif

bool C_Cycle::IsValid() const
{
	return m_numFrames > 0;
}

bool C_Cycle::IsPartiallyLoaded() const
{
	return IsValid() && m_isPartiallyLoaded;
}

void C_Cycle::SetName( const CHAR8* name )
{
#if !defined(CFG_GOLD) && !defined(PLATFORM_SPU)
	// In gold builds, avoid storing name in string table to save memory.
	m_name = C_StringId( name );
#endif
}

#if !defined(PLATFORM_SPU)
C_JointTransform C_Cycle::EvaluateTrajectoryTransform( F32 time, E_LoopMode loopMode ) const
{
	const C_PoseLayout* poseLayout = C_PoseLayout::GetTrajectoryPoseLayout();
	
	// Temporary buffer 
	U8 scratchBuffer[64];
	
	// Verify buffer is large enough.
	BIZ_ASSERT(sizeof(scratchBuffer) >= poseLayout->GetPoseDataSize());
		
	Animation::C_Pose pose;
	pose.CreateInPlace(*poseLayout, scratchBuffer);
	
	pose.Clear();

	C_CycleInstance cycleInstance(this, &pose);
	
	// Set field mask to evaluate the trajectory transform and nothing else.
	Animation::C_PoseFieldMask fieldMask;
	fieldMask.DisableAll().Enable(Animation::E_PoseFieldType_TrajectoryTransform);
	
	Evaluate(Animation::E_BlendMode_Accumulate, time, cycleInstance, 0, 1.f, pose, loopMode, fieldMask);
	
	pose.Normalise(NULL);
	
	const C_JointTransform traj = pose.GetTrajectoryTransform();

	return traj;
}
#endif

U32 C_Cycle::GetMemoryUsage() const
{
	U32 used = sizeof(C_Cycle);
	
	// Size of m_fieldTypes.
	used += sizeof(U8) * m_numFields;
	// Size of field name array.
	used += m_fieldNames.Size() * sizeof(C_StringId);
	// Size of m_channelInfo array.
	used += sizeof(U16) * (m_totalConstantCurves + m_totalVaryingCurves);
	// Size of m_constantCurveData array.
	used += m_constantCurveDataSize;
	// Size of m_segmentData array
	used += m_totalSegmentDataSize;
	// Size of m_segments array.
	used += m_numSegments * sizeof(C_Segment);
	
	return used;
}


#ifndef PLATFORM_SPU

//#define kCycleMappingCacheMemorySubTag		"animation.cycle.mappingcache"

const C_CycleMapping* C_Cycle::GetMapping( const C_PoseLayout& poseLayout ) const
{
	// Search for matching mapping in cache.
	C_CycleMapping* mapping = m_mappings;
	while ( mapping  )
	{
		if ( mapping->GetPoseLayout() == &poseLayout )
		{
			break;
		}
		mapping = mapping->GetNextByCycle();
	}

	if ( mapping )
	{
		mapping->AddRef();
	}
	else
	{
		//MEMORY_TRACKER_PUSH_SUB( kCycleMappingCacheMemorySubTag );
	
		// Create new mapping (automatically added to appropriate lists)
		mapping = new ( GetNumFields() ) C_CycleMapping( *this, poseLayout );
		
		//MEMORY_TRACKER_POP_SUB( kCycleMappingCacheMemorySubTag );
	}
	
	return mapping;
}

void C_Cycle::ClearMappingCache() const
{
	C_CycleMapping* mapping = m_mappings;
	while ( mapping  )
	{
		C_CycleMapping* next = mapping->GetNextByCycle();
		mapping->Invalidate();
		mapping = next;
	}
	BIZ_ASSERT( m_mappings == 0 ); // should have been done automatically
	m_mappings = 0;
}

#endif

//
// Joint transform sampling and blending.
// 

U32 C_Cycle::FindSegmentIndex( const C_Segment* segments, F32 timeInFrames ) const
{
	const U32 iTime = static_cast< U32 >( timeInFrames );

	U32 left = 0;
	U32 right = m_numSegments - 1; // ignore incomplete final frame.

	// Find first keyTime > iTime.
	
	while ( left < right )
	{
		U32 midIndex = left + ( ( right - left ) / 2 );
		U32 midKeyTime = segments[ midIndex ].m_startFrame;

#if 1
		if ( midKeyTime > iTime )
		{
			right = midIndex;
		}
		else
		{
			left = midIndex + 1;
		}
#else				
		// Branchless "if (midKeyTime > iTime) right = midIndex; else left = midIndex;"
		const S32 mask = ( (S32) midKeyTime - (S32) iTime - 1 ) >> 31;
		right = midIndex + ( ( right - midIndex ) & mask );
		left  = left + ( 
		( midIndex - left ) & mask );
#endif
	}
	
	U32 segmentIndex = ( right > 0 ) ? right - 1 : 0;
	BIZ_ASSERT( segmentIndex < m_numSegments );
	
	return segmentIndex;
}

//
// Evaluation.
//


// Search for first value > x. Equivalent to std::upper_bound.
U32 BinarySearch( U32 x, const U8* values, U32 count )
{
	U32 left = 0;
	U32 right = count;
	
	// Find first keyTime > iTime.
	while ( left < right )
	{
		U32 midIndex = left + ( ( right - left ) / 2 );
		U32 midKeyTime = values[ midIndex ];

#if 1
		if ( midKeyTime > x )
		{
			right = midIndex;
		}
		else
		{
			// Skip past anything <= x
			left = midIndex + 1;
		}
#else				
		// Branchless "if (midKeyTime > iTime) right = midIndex; else left = midIndex;"
		const S32 mask = ( (S32) midKeyTime - (S32) x - 1 ) >> 31;
		right = midIndex + ( ( right - midIndex ) & mask );
		left  = left + ( ( midIndex + 1 - left ) & mask );
#endif
	}
	
	return right;
}

F32 FindKeys(
	const void* initKey,
	const void* midKeys,
	const void* finalKey,
	const U8* keyTimes,
	F32 timeInFrames,
	U32 timeAsInteger,
	U32 numSegmentFrames,
	F32 fNumSegmentFrames,
	U32 keySize,
	const void*& outPrevKey,
	const void*& outNextKey,
	U32& outNumKeys
)
{
	const U32 numKeys = keyTimes[ 0 ];
	outNumKeys = numKeys;
	
	// Initialise to end cases.
	F32 prevTime = 0;
	outPrevKey = initKey;
	F32 nextTime = fNumSegmentFrames;
	outNextKey = finalKey;
	
	if ( 0 != numKeys )
	{
		const U8* actualKeyTimes = keyTimes + 1;

		U32 nextKeyIndex = BinarySearch( timeAsInteger, actualKeyTimes, numKeys );
		
		// Previous key is initial.
		if ( nextKeyIndex > 0 )
		{
			outPrevKey = ( const U8* ) midKeys + ( nextKeyIndex - 1 ) * keySize;
			prevTime = static_cast<F32>( actualKeyTimes[ nextKeyIndex - 1 ] );
		}
		
		if ( nextKeyIndex < numKeys )
		{
			outNextKey = ( const U8* ) midKeys + nextKeyIndex * keySize;
			nextTime = static_cast<F32>( actualKeyTimes[ nextKeyIndex ] );
		}

		
	#ifdef CFG_DEBUG
		BIZ_ASSERT( nextTime <= fNumSegmentFrames );
		BIZ_ASSERT( prevTime <= fNumSegmentFrames );
		BIZ_ASSERT( nextTime >= prevTime );
	#endif

	}

#ifdef CFG_DEBUG
	BIZ_ASSERT( nextTime <= fNumSegmentFrames );
	BIZ_ASSERT( prevTime <= fNumSegmentFrames );
	BIZ_ASSERT( nextTime >= prevTime );
#endif
		
	const F32 fPrevTime = prevTime;
	const F32 fNextTime = nextTime;
	const F32 blend = ( timeInFrames - fPrevTime ) * Maths::Reciprocal( fNextTime - fPrevTime );
	return blend;
}

FORCE_INLINE S32 GetPoseFieldIndex( U32 channelInfo, const C_CycleMapping& mapping, E_PoseFieldElement& outFieldElement )
{
#ifdef CFG_DEBUG
	BIZ_ASSERT( (channelInfo & 0x3f) < E_PoseFieldElement_Count );
#endif

	const U32 cycleFieldIndex = channelInfo >> 6;
	outFieldElement = static_cast<E_PoseFieldElement>( channelInfo & 0x3f );
	return mapping.GetPoseFieldIndex( cycleFieldIndex );;
}

U32 PackFieldInfo( U32 fieldIndex, E_PoseFieldElement fieldElement )
{
	return (fieldIndex << 6 ) | (fieldElement & 0x3f);
}

namespace
{
	class C_CycleSegmentReader
	{
	public:
	
		C_CycleSegmentReader();

		C_CycleSegmentReader(
			const U8* data,
			const U32 size,
			U32 numFrames,
			F32 sampleTime,
			const U16* numVaryingCurvesOfType,
			U32 totalVaryingCurves,
			U32 totalKeysInSegment
		);
			
		F32 FindVec3F32Keys(
			const void*& outPrevKey,
			const void*& outNextKey
		);
		
		F32 FindQuat32Keys(
			const void*& outPrevKey,
			const void*& outNextKey
		);
		
		F32 FindScalarF32Keys(
			const void*& outPrevKey,
			const void*& outNextKey
		);

		F32 FindQuat48Keys(
			const void*& outPrevKey,
			const void*& outNextKey
		);

		F32 FindVec3U16Keys(
			const void*& outPrevKey,
			const void*& outNextKey
		);
		
		F32 FindVec3U8Keys(
			const void*& outPrevKey,
			const void*& outNextKey
		);

		const U8* GetKeyTimes() const	{ return m_keyTimes; }

	private:
		const U32 m_numFrames;
		const F32 m_fNumFrames;
		const U16* m_numKeysPerCurveType;
		const F32 m_timeInFrames;
		const U32 m_timeAsInteger;

		// Keys for initial frame.
		const F32*	m_initVec3F32Keys;
		const U32* m_initQuat32Keys;
		const F32* m_initScalarF32Keys;
		const U16* m_initQuat48Keys;
		const U16* m_initVec3U16Keys;
		const U8* m_initVec3U8Keys;
		
		const U8* m_keyTimes;
		
		// Current key pointers.
		const F32* m_vec3F32Keys;
		const U32* m_quat32Keys;
		const F32* m_scalarF32Keys;
		const U16* m_quat48Keys;
		const U16* m_vec3U16Keys;
		const U8* m_vec3U8Keys;
		
		// Keys for final frame.
		const F32*	m_finalVec3F32Keys;
		const U32* m_finalQuat32Keys;
		const F32* m_finalScalarF32Keys;
		const U16* m_finalQuat48Keys;
		const U16* m_finalVec3U16Keys;
		const U8* m_finalVec3U8Keys;
	};

	C_CycleSegmentReader::C_CycleSegmentReader(
		const U8* data,
		U32 size,
		U32 numFrames,
		F32 sampleTime,
		const U16* numVaryingCurvesOfType,
		U32 totalVaryingCurves,
		U32 totalKeysInSegment
	):
		m_numFrames( numFrames ),
		m_fNumFrames( static_cast<F32>( numFrames ) ),
		m_timeInFrames(sampleTime),
		m_timeAsInteger(static_cast<U32>(sampleTime))
	{
		C_MemoryBlockReader blockReader( data, size );

		// Decode the segment into arrays.
		m_initVec3F32Keys = blockReader.Read< F32 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3F32 ] * 3 );
		m_initQuat32Keys = blockReader.Read< U32 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion32 ] );
		m_initScalarF32Keys = blockReader.Read< F32 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingScalarF32 ] );
		m_initQuat48Keys = blockReader.Read< U16 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion48 ] * 3 );
		m_initVec3U16Keys = blockReader.Read< U16 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U16 ] * 3 );
		m_initVec3U8Keys = blockReader.Read< U8 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U8 ] * 3 );
		
		// no. keys for each curve type.
		m_numKeysPerCurveType = blockReader.Read< U16 >( E_VaryingCurveType_Count );

		m_keyTimes = blockReader.Read< U8 >( totalVaryingCurves + totalKeysInSegment );
		
		m_vec3F32Keys = blockReader.Read< F32 >( m_numKeysPerCurveType[ E_VaryingCurveType_VaryingVector3F32 ] * 3 );
		m_quat32Keys = blockReader.Read< U32 >( m_numKeysPerCurveType[ E_VaryingCurveType_VaryingQuaternion32 ] );
		m_scalarF32Keys = blockReader.Read< F32 >( m_numKeysPerCurveType[ E_VaryingCurveType_VaryingScalarF32 ] );
		m_quat48Keys = blockReader.Read< U16 >( m_numKeysPerCurveType[ E_VaryingCurveType_VaryingQuaternion48 ] * 3 );
		m_vec3U16Keys = blockReader.Read< U16 >( m_numKeysPerCurveType[ E_VaryingCurveType_VaryingVector3U16 ] * 3 );
		m_vec3U8Keys = blockReader.Read< U8 >( m_numKeysPerCurveType[ E_VaryingCurveType_VaryingVector3U8 ] * 3 );
		
		blockReader.Align( 16 );

		m_finalVec3F32Keys = blockReader.Read< F32 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3F32 ] * 3 );
		m_finalQuat32Keys = blockReader.Read< U32 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion32 ] );
		m_finalScalarF32Keys = blockReader.Read< F32 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingScalarF32 ] );
		m_finalQuat48Keys = blockReader.Read< U16 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion48 ] * 3 );
		m_finalVec3U16Keys = blockReader.Read< U16 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U16 ] * 3 );
		m_finalVec3U8Keys = blockReader.Read< U8 >( numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U8 ] * 3 );
	}

	inline F32 C_CycleSegmentReader::FindVec3F32Keys(
		const void*& outPrevKey,
		const void*& outNextKey
	)
	{
		U32 numKeys = 0;
		F32 blend = FindKeys(
			m_initVec3F32Keys,
			m_vec3F32Keys,
			m_finalVec3F32Keys,
			m_keyTimes,
			m_timeInFrames,
			m_timeAsInteger,
			m_numFrames,
			m_fNumFrames,
			12,
			outPrevKey,
			outNextKey,
			numKeys
		);
		
		m_initVec3F32Keys += 3;
		m_vec3F32Keys += 3 * numKeys;
		m_finalVec3F32Keys += 3;
		m_keyTimes += numKeys + 1;
		return blend;
	}
	
	inline F32 C_CycleSegmentReader::FindQuat32Keys(
		const void*& outPrevKey,
		const void*& outNextKey
	)
	{
		U32 numKeys = 0;
		F32 blend = FindKeys(
			m_initQuat32Keys,
			m_quat32Keys,
			m_finalQuat32Keys,
			m_keyTimes,
			m_timeInFrames,
			m_timeAsInteger,
			m_numFrames,
			m_fNumFrames,
			4,
			outPrevKey,
			outNextKey,
			numKeys
		);
		
		m_initQuat32Keys++;
		m_quat32Keys += numKeys;
		m_finalQuat32Keys++;
		m_keyTimes += numKeys + 1;
		return blend;
	}
	
	inline F32 C_CycleSegmentReader::FindScalarF32Keys(
		const void*& outPrevKey,
		const void*& outNextKey
	)
	{
		U32 numKeys = 0;
		F32 blend = FindKeys(
			m_initScalarF32Keys,
			m_scalarF32Keys,
			m_finalScalarF32Keys,
			m_keyTimes,
			m_timeInFrames,
			m_timeAsInteger,
			m_numFrames,
			m_fNumFrames,
			4,
			outPrevKey,
			outNextKey,
			numKeys
		);
		
		m_initScalarF32Keys++;
		m_scalarF32Keys += numKeys;
		m_finalScalarF32Keys++;
		m_keyTimes += numKeys + 1;
		return blend;
	}

	inline F32 C_CycleSegmentReader::FindQuat48Keys(
		const void*& outPrevKey,
		const void*& outNextKey
	)
	{
		U32 numKeys = 0;
		F32 blend = FindKeys(
			m_initQuat48Keys,
			m_quat48Keys,
			m_finalQuat48Keys,
			m_keyTimes,
			m_timeInFrames,
			m_timeAsInteger,
			m_numFrames,
			m_fNumFrames,
			6,
			outPrevKey,
			outNextKey,
			numKeys
		);
		
		m_initQuat48Keys += 3;
		m_quat48Keys += 3 * numKeys;
		m_finalQuat48Keys += 3;
		m_keyTimes += numKeys + 1;
		return blend;
	}

	inline F32 C_CycleSegmentReader::FindVec3U16Keys(
		const void*& outPrevKey,
		const void*& outNextKey
	)
	{
		U32 numKeys = 0;
		F32 blend = FindKeys(
			m_initVec3U16Keys,
			m_vec3U16Keys,
			m_finalVec3U16Keys,
			m_keyTimes,
			m_timeInFrames,
			m_timeAsInteger,
			m_numFrames,
			m_fNumFrames,
			6,
			outPrevKey,
			outNextKey,
			numKeys
		);
		
		m_initVec3U16Keys += 3;
		m_vec3U16Keys += 3 * numKeys;
		m_finalVec3U16Keys += 3;
		m_keyTimes += numKeys + 1;
		return blend;
	}

	inline F32 C_CycleSegmentReader::FindVec3U8Keys(
		const void*& outPrevKey,
		const void*& outNextKey
	)
	{
		U32 numKeys = 0;
		F32 blend = FindKeys(
			m_initVec3U8Keys,
			m_vec3U8Keys,
			m_finalVec3U8Keys,
			m_keyTimes,
			m_timeInFrames,
			m_timeAsInteger,
			m_numFrames,
			m_fNumFrames,
			3,
			outPrevKey,
			outNextKey,
			numKeys
		);
		
		m_initVec3U8Keys += 3;
		m_vec3U8Keys += 3 * numKeys;
		m_finalVec3U8Keys += 3;
		m_keyTimes += numKeys + 1;
		return blend;
	}

}

//
// EvaluationContext
//

class C_EvaluationContext
{
public:
	C_EvaluationContext(
		C_Pose& inPose,
		const C_CycleMapping& inMapping,
		F32 inWeight,
		U32 inTrajRootRotChannel,
		U32 inTrajRootTransChannel,
		E_BlendMode inBlendMode,
		U32 inUpdateCounter,
		const C_PoseFieldMask& fieldMask
	):
		pose(inPose),
		poseLayout(inPose.GetPoseLayout()),
		mapping( inMapping ),
		weight( Maths::MakeVF32( inWeight ) ),
		trajectoryRootRotationChannel( inTrajRootRotChannel ),
		trajectoryRootTranslationChannel( inTrajRootTransChannel ), // scale?
		updateCounter(inUpdateCounter),
		blendMode( inBlendMode )
	{
		trajectoryRootTransform.SetIdentity();
		
		for ( U32 i = 0; i < E_PoseFieldElement_Count; ++i )
		{
			fieldElementEnabled[i] = fieldMask.IsEnabled( static_cast<E_PoseFieldElement>(i) ) ? 1 : 0;
		}
		
	}
	
	U32 IsFieldElementEnabled( E_PoseFieldElement e ) const
	{
		return fieldElementEnabled[ e ];
	}
		
	C_Pose&					pose;
	const C_PoseLayout*		poseLayout;
	const C_CycleMapping&	mapping;
	const Maths::VF32		weight;
	const U32				trajectoryRootRotationChannel;
	const U32				trajectoryRootTranslationChannel;
	const U32				updateCounter;
	C_JointTransform		trajectoryRootTransform;
	const E_BlendMode		blendMode;
	U8						fieldElementEnabled[ E_PoseFieldElement_Count ];
};


//
// Evaluation buffer sizes.
//
static const U32 kVector4BufferSize	= 16;
static const U32 kQuaternionBufferSize	= 16;

void EvaluateConstantVector3F32s(
	C_EvaluationContext& context,
	C_MemoryBlockReader& constantCurveReader,
	const U16*& channelInfo,
	const U32 count
)
{
	const F32* src = constantCurveReader.Read< F32 >( count * 3 );
	
	C_Vector3Field evalBuffer[ kVector4BufferSize ];
	C_Vector3Field* evalDestPtrs[ kVector4BufferSize ];
	
	U32 curveIndex = 0;
	
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kVector4BufferSize && curveIndex < count )
		{
			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );
			
			const bool isTrajectoryElement = (channelInfo[curveIndex] == context.trajectoryRootTranslationChannel);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				const Maths::Vector3 vector3 = Maths::Load( (const Maths::StorageVector3*)( src ) );
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					evalBuffer[ batchCount ] = Maths::MakeVec4F32( vector3, one );
					evalDestPtrs[ batchCount ] = context.pose.GetVector3FieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}
				
				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetTranslation( vector3 );
				}
			}
			++curveIndex;
			src += 3;
		}
		
		BlendVector3ArrayIndirect( context.blendMode, evalDestPtrs, evalBuffer, batchCount, context.weight );
	}
	
	channelInfo += count;
}

void EvaluateConstantScalarF32s(
	C_EvaluationContext& context,
	C_MemoryBlockReader& constantCurveReader,
	const U16*& channelInfo,
	const U32 count
)
{
	const F32* src = constantCurveReader.Read< F32 >( count );
	
	C_ScalarField evalBuffer[ kVector4BufferSize ];
	C_ScalarField* evalDestPtrs[ kVector4BufferSize ];
	
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	U32 curveIndex = 0;
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kVector4BufferSize && curveIndex < count )
		{
			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );
			if ( poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement) )
			{
				const Maths::VF32 s = Maths::MakeVF32(src);
				evalBuffer[ batchCount ] = Maths::MakeVec4F32(s, s, s, one );
				evalDestPtrs[ batchCount ] = context.pose.GetScalarFieldElement( fieldElement, poseFieldIndex );
				++batchCount;
			}
			++curveIndex;
			++src;
		}
		
		BlendScalarArrayIndirect( context.blendMode, evalDestPtrs, evalBuffer, batchCount, context.weight );
	}
	
	channelInfo += count;
}	


void EvaluateConstantQuaternion48s(
	C_EvaluationContext& context,
	C_MemoryBlockReader& constantCurveReader,
	const U16*& channelInfo,
	const U32 count
)
{
	const U16* src = constantCurveReader.Read< U16 >( count * 3 );
	
	C_QuaternionField evalBuffer[ kQuaternionBufferSize ];
	C_QuaternionField* evalDestPtrs[ kQuaternionBufferSize ];
	
	U32 curveIndex = 0;
	
	const U16* pFieldInfo = channelInfo;
	const U32 trajRotFieldInfo = context.trajectoryRootRotationChannel;
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kQuaternionBufferSize && curveIndex < count )
		{
			E_PoseFieldElement fieldElement;
			const U32 currFieldInfo = pFieldInfo[ curveIndex ];
			const S32 poseFieldIndex = GetPoseFieldIndex( currFieldInfo, context.mapping, fieldElement );

			const bool isTrajectoryElement = (pFieldInfo[curveIndex] == trajRotFieldInfo);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				Maths::Quaternion q;
				Animation::DecompressQuaternion48( src, q );
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					evalBuffer[ batchCount ] = q;
					evalDestPtrs[ batchCount ] = context.pose.GetQuaternionFieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}

				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetRotation( q );
				}
			}
			src += 3;
			++curveIndex;
		}
		
		BlendQuaternionArrayIndirect( context.blendMode, evalDestPtrs, evalBuffer, batchCount, context.weight );
	}
	
	channelInfo += count;
}		

void EvaluateVaryingVector3F32s(
	C_EvaluationContext& context,
	C_CycleSegmentReader& segmentReader,
	const U16*& channelInfo,
	const U32 count
)
{
	C_Vector3Field evalBuffer[ kVector4BufferSize ];
	C_Vector3Field* evalDestPtrs[ kVector4BufferSize ];
	
	U32 curveIndex = 0;
	
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kVector4BufferSize && curveIndex < count )
		{
			const void* prevPackedKey;
			const void* nextPackedKey;
			F32 blend = segmentReader.FindVec3F32Keys( prevPackedKey, nextPackedKey );

			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );
			
			const bool isTrajectoryElement = (channelInfo[curveIndex] == context.trajectoryRootTranslationChannel);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				const Maths::Vector3 prevKey = Maths::Load( (const Maths::StorageVector3*) prevPackedKey );
				const Maths::Vector3 nextKey = Maths::Load( (const Maths::StorageVector3*) nextPackedKey );
				Maths::Vector3 blendedKey = Maths::Lerp( prevKey, nextKey, Maths::MakeVF32( blend ) );
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					evalBuffer[ batchCount ] = Maths::MakeVec4F32(blendedKey, one);
					evalDestPtrs[ batchCount ] = context.pose.GetVector3FieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}

				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetTranslation( blendedKey );
				}
			}
			++curveIndex;
		}
		BlendVector3ArrayIndirect( context.blendMode, evalDestPtrs, evalBuffer, batchCount, context.weight );
	}
	channelInfo += count;
}

void EvaluateVaryingVector3U16s(
	C_EvaluationContext& context,
	C_CycleSegmentReader& segmentReader,
	const F32*& varyingVector3Bounds,
	const U16*& channelInfo,
	const U32 count
)
{
	C_Vector3Field evalBuffer[ kVector4BufferSize ];
	C_Vector3Field* evalDestPtrs[ kVector4BufferSize ];
	
	U32 curveIndex = 0;
	
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kVector4BufferSize && curveIndex < count )
		{
			const void* prevPackedKey;
			const void* nextPackedKey;
			F32 blend = segmentReader.FindVec3U16Keys( prevPackedKey, nextPackedKey );

			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );
			
			const bool isTrajectoryElement = (channelInfo[curveIndex] == context.trajectoryRootTranslationChannel);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				Maths::Vector3 prevKey;
				Maths::Vector3 nextKey;
				
				const F32* offset = varyingVector3Bounds;
				const F32* scale = varyingVector3Bounds + 3;
				
				Animation::DecompressVector3U16( (const U16*) prevPackedKey, offset, scale, prevKey );
				Animation::DecompressVector3U16( (const U16*) nextPackedKey, offset, scale, nextKey );
				
				const Maths::Vector3 blendedKey = Maths::Lerp(
					prevKey,
					nextKey,
					Maths::MakeVF32( blend )
				);
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					evalBuffer[ batchCount ] = Maths::MakeVec4F32( blendedKey, one );
					evalDestPtrs[ batchCount ] = context.pose.GetVector3FieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}

				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetTranslation( blendedKey );
				}
			}
			++curveIndex;
			varyingVector3Bounds += 6;
		}
		BlendVector3ArrayIndirect( context.blendMode, evalDestPtrs, evalBuffer, batchCount, context.weight );
	}
	channelInfo += count;
}
					
void EvaluateVaryingVector3U8s(
	C_EvaluationContext& context,
	C_CycleSegmentReader& segmentReader,
	const F32*& varyingVector3Bounds,
	const U16*& channelInfo,
	const U32 count
)
{
	C_Vector3Field evalBuffer[ kVector4BufferSize ];
	C_Vector3Field* evalDestPtrs[ kVector4BufferSize ];
	
	U32 curveIndex = 0;
	
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kVector4BufferSize && curveIndex < count )
		{
			const void* prevPackedKey;
			const void* nextPackedKey;
			F32 blend = segmentReader.FindVec3U8Keys( prevPackedKey, nextPackedKey );

			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );

			const bool isTrajectoryElement = (channelInfo[curveIndex] == context.trajectoryRootTranslationChannel);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				Maths::Vector3 prevKey;
				Maths::Vector3 nextKey;
				
				const F32* offset = varyingVector3Bounds;
				const F32* scale = varyingVector3Bounds + 3;
				
				Animation::DecompressVector3U8( (const U8*) prevPackedKey, offset, scale, prevKey );
				Animation::DecompressVector3U8( (const U8*) nextPackedKey, offset, scale, nextKey );
				
				const Maths::Vector3 blendedKey = Maths::Lerp(
					prevKey,
					nextKey,
					Maths::MakeVF32( blend )
				);
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					evalBuffer[ batchCount ] = Maths::MakeVec4F32( blendedKey, one );
					evalDestPtrs[ batchCount ] = context.pose.GetVector3FieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}

				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetTranslation( blendedKey );
				}
			}
			++curveIndex;
			varyingVector3Bounds += 6;
		}
		BlendVector3ArrayIndirect( context.blendMode, evalDestPtrs, evalBuffer, batchCount, context.weight );
	}
	channelInfo += count;
}	

void EvaluateVaryingQuat48s(
	C_EvaluationContext& context,
	C_CycleSegmentReader& segmentReader,
	const U16*& channelInfo,
	const U32 count
)
{
	C_QuaternionField sampledQuats[ kQuaternionBufferSize ];
	C_QuaternionField* destQuats[ kQuaternionBufferSize ];
	
	U32 curveIndex = 0;
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kQuaternionBufferSize && curveIndex < count )
		{
			const void* prevPackedKey;
			const void* nextPackedKey;
			F32 blend = segmentReader.FindQuat48Keys( prevPackedKey, nextPackedKey );

			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );

			const bool isTrajectoryElement = (channelInfo[curveIndex] == context.trajectoryRootRotationChannel);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				Maths::Quaternion prevKey;
				Maths::Quaternion nextKey;
				
				Animation::DecompressQuaternion48( (const U16*) prevPackedKey , prevKey );
				Animation::DecompressQuaternion48( (const U16*) nextPackedKey , nextKey );
				
				Maths::Quaternion blendedKey = Maths::LerpNormalised( prevKey, nextKey, Maths::MakeVF32( blend ) );
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					sampledQuats[ batchCount ] = blendedKey;
					destQuats[ batchCount ] = context.pose.GetQuaternionFieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}
				
				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetRotation( blendedKey );
				}
			}
			++curveIndex;
		}
		BlendQuaternionArrayIndirect( context.blendMode, destQuats, sampledQuats, batchCount, context.weight );
	}
	channelInfo += count;
}	


void EvaluateVaryingQuat32s(
	C_EvaluationContext& context,
	C_CycleSegmentReader& segmentReader,
	const U16*& channelInfo,
	const U32 count
)
{
	C_QuaternionField sampledQuats[ kQuaternionBufferSize ];
	C_QuaternionField* destQuats[ kQuaternionBufferSize ];
	
	U32 curveIndex = 0;
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kQuaternionBufferSize && curveIndex < count )
		{
			const void* prevPackedKey;
			const void* nextPackedKey;
			F32 blend = segmentReader.FindQuat32Keys( prevPackedKey, nextPackedKey );

			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );
			
			const bool isTrajectoryElement = (channelInfo[curveIndex] == context.trajectoryRootRotationChannel);
			const bool isTransformElement = (poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement));
			
			if (isTrajectoryElement || isTransformElement)
			{
				Maths::Quaternion prevKey;
				Maths::Quaternion nextKey;
				
				Animation::DecompressQuaternion32( * (const U32*) prevPackedKey , prevKey );
				Animation::DecompressQuaternion32( * (const U32*) nextPackedKey , nextKey );
				
				Maths::Quaternion blendedKey = Maths::LerpNormalised( prevKey, nextKey, Maths::MakeVF32( blend ) );
				
				if (BRANCH_HINT_LIKELY(isTransformElement))
				{
					sampledQuats[ batchCount ] = blendedKey;
					destQuats[ batchCount ] = context.pose.GetQuaternionFieldElement( fieldElement, poseFieldIndex );
					++batchCount;
				}
				
				if (BRANCH_HINT_UNLIKELY(isTrajectoryElement))
				{
					context.trajectoryRootTransform.SetRotation( blendedKey );
				}
			}
			++curveIndex;
		}
		BlendQuaternionArrayIndirect( context.blendMode, destQuats, sampledQuats, batchCount, context.weight );
	}
	channelInfo += count;
}

void EvaluateVaryingScalarF32s(
	C_EvaluationContext& context,
	C_CycleSegmentReader& segmentReader,
	const U16*& channelInfo,
	const U32 count
)
{
	C_ScalarField evalBuffer[ kVector4BufferSize ];
	C_ScalarField* destPtrs[ kVector4BufferSize ];
			
	U32 curveIndex = 0;
	
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	
	while ( curveIndex < count )
	{
		U32 batchCount = 0;
		while ( batchCount < kQuaternionBufferSize && curveIndex < count )
		{
			const void* prevPackedKey;
			const void* nextPackedKey;
			F32 blend = segmentReader.FindScalarF32Keys( prevPackedKey, nextPackedKey );

			E_PoseFieldElement fieldElement;
			const S32 poseFieldIndex = GetPoseFieldIndex( channelInfo[ curveIndex ], context.mapping, fieldElement );
							
			if ( poseFieldIndex >= 0 && context.IsFieldElementEnabled(fieldElement) )
			{
				F32 prevKey = *(const F32*)prevPackedKey;
				F32 nextKey = *(const F32*)nextPackedKey;
				F32 blendedKey = Maths::Lerp( prevKey, nextKey, blend );
				
				const Maths::VF32 s = Maths::MakeVF32( blendedKey );
				
				evalBuffer[ batchCount ] = Maths::MakeVec4F32( s, s, s, one );
				destPtrs[ batchCount ] = context.pose.GetScalarFieldElement( fieldElement, poseFieldIndex );
				++batchCount;
			}
			++curveIndex;
		}
		BlendScalarArrayIndirect( context.blendMode, destPtrs, evalBuffer, batchCount, context.weight );
	}
	channelInfo += count;
}

void C_Cycle::Evaluate(
	E_BlendMode blendMode,
	F32 time,
	C_CycleInstance& instance,
	U32 updateCounter,
	F32 weight,
	C_Pose& pose,
	E_LoopMode loopMode,
	const C_PoseFieldMask& fieldMask
) const
{
	//PERF_EVENT( "Animation.Cycle.Evaluate" );

	if (IsPartiallyLoaded())
	{
		return;
	}
	
	Spu::MemBaseMarker evalMemBaseMarker;

	BIZ_ASSERT( instance.GetCycleMapping() != NULL );
	const C_CycleMapping& mapping = *instance.GetCycleMapping();
	
	BIZ_ASSERT( mapping.GetCycle() != NULL );
	
#ifndef PLATFORM_SPU
	BIZ_ASSERT( mapping.GetCycle() == this );
#endif

	BIZ_ASSERT( this != NULL );
	
	C_EvaluationContext evalContext(
		pose,
		mapping,
		weight,
		PackFieldInfo( m_trajectoryRootFieldIndex, E_PoseFieldElement_JointTransformRotation ),
		PackFieldInfo( m_trajectoryRootFieldIndex, E_PoseFieldElement_JointTransformTranslation ),
		blendMode,
		updateCounter,
		fieldMask
	);
		
	//
	// Start fetching segment array, constant curve data and joint indices.
	//
	const U32 constantCurveDataDmaTag = 0;
	const U32 segmentsDmaTag = 1;
	
	const C_Segment* segments = (const C_Segment*) Spu::FetchAllocAsync( m_segments, m_numSegments * sizeof( C_Segment ), segmentsDmaTag );
	const U8* constantCurveData = (const U8*) Spu::FetchAllocAsync( m_constantCurveData, m_constantCurveDataSize, constantCurveDataDmaTag );
	const U16* channelInfo = (const U16*) Spu::FetchAllocAsync( m_channelInfo, Btl::AlignUp( U32( (m_totalVaryingCurves + m_totalConstantCurves) * sizeof(U16) ), 16u ) /* [Blur2PC][x64] size_t is 64-bit */, constantCurveDataDmaTag );
		
	const F32 timeInFrames = GetFramesFromSeconds( time, loopMode );
	const U32 prevFrame = static_cast< U32 >( timeInFrames );
	const U32 nextFrame = Maths::Min( prevFrame + 1, m_numFrames - 1 );
	
	const Maths::VF32 blendWeight = Maths::MakeVF32( weight );
	
	//
	// Determine which segment is required and asynchronously fetch it.
	// Note that the initial frame data from the subsequent block is also fetched.
	//
	
	// Wait for segment header array.
	Spu::Wait( segmentsDmaTag );
	
	const U32 segmentIndex = FindSegmentIndex( segments, timeInFrames );
	const U32 segmentSize = Btl::AlignUp( segments[ segmentIndex ].m_size + m_initialFrameSize, 16 );
	const U8* segmentData = (const U8*) Spu::FetchAllocAsync( segments[ segmentIndex ].m_data, segmentSize, segmentsDmaTag );

		
	//
	// Process joints with constant data.
	//
	
	Spu::Wait( constantCurveDataDmaTag );
	
	C_MemoryBlockReader constantCurveReader( constantCurveData, m_constantCurveDataSize );
	
	const F32* varyingVector3Bounds = constantCurveReader.Read< F32 >(
		6 * ( m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U16 ] + m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U8 ] ) );
	
	EvaluateConstantVector3F32s(
		evalContext,
		constantCurveReader,
		channelInfo,
		m_numConstantCurvesOfType[ E_ConstantCurveType_ConstantVector3F32 ]
	);
	
	EvaluateConstantScalarF32s(
		evalContext,
		constantCurveReader,
		channelInfo,
		m_numConstantCurvesOfType[ E_ConstantCurveType_ConstantScalarF32 ]
	);

	EvaluateConstantQuaternion48s(
		evalContext,
		constantCurveReader,
		channelInfo,
		m_numConstantCurvesOfType[ E_ConstantCurveType_ConstantQuaternion48 ]
	);

	//
	// Process segment containing varying curves.
	//
		
	Spu::Wait( segmentsDmaTag );
	
	const C_Segment& segment = segments[ segmentIndex ];
	
	const F32 segmentTime = timeInFrames - segment.m_startFrame;
	BIZ_ASSERT( segmentTime >= 0.f && segmentTime <= (F32)segment.m_numFrames );	
	
	C_CycleSegmentReader segmentReader( segmentData, segmentSize, segment.m_numFrames, segmentTime, m_numVaryingCurvesOfType, m_totalVaryingCurves, segment.m_totalKeys );
	
	EvaluateVaryingVector3F32s(
		evalContext,
		segmentReader,
		channelInfo,
		m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3F32 ]
	);

	EvaluateVaryingQuat32s(
		evalContext,
		segmentReader,
		channelInfo,
		m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion32 ]
	);

	EvaluateVaryingScalarF32s(
		evalContext,
		segmentReader,
		channelInfo,
		m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingScalarF32 ]
	);

	EvaluateVaryingQuat48s(
		evalContext,
		segmentReader,
		channelInfo,
		m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion48 ]
	);

	EvaluateVaryingVector3U16s(
		evalContext,
		segmentReader,
		varyingVector3Bounds,
		channelInfo,
		m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U16 ]
	);
	
	EvaluateVaryingVector3U8s(
		evalContext,
		segmentReader,
		varyingVector3Bounds,
		channelInfo,
		m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U8 ]
	);

	//
	// Evaluate and optionally blend trajectory root information, depending on fieldMask.
	//
	if ( m_trajectoryRootFieldIndex >= 0 )
	{
		// Always evaluate the trajectory to update the cached trajectory transforms in C_CycleInstance.
		C_JointTransform trajectoryDelta;
		EvaluateTrajectoryDelta( trajectoryDelta, instance, evalContext, time, timeInFrames );
		
		if ( pose.HasTrajectoryDeltaTransform() )
		{
			// Blend in trajectory delta transform.
			C_QuaternionField* destQ = pose.GetQuaternionFieldElement( E_PoseFieldElement_TrajectoryDeltaTransformRotation, 0 );
			C_Vector3Field* destT = pose.GetVector3FieldElement( E_PoseFieldElement_TrajectoryDeltaTransformTranslation, 0 );
		
			if ( fieldMask.IsEnabled( E_PoseFieldElement_TrajectoryDeltaTransformRotation ) )
				BlendQuaternion( evalContext.blendMode, destQ, &trajectoryDelta.GetRotationField(), evalContext.weight );
				
			if ( fieldMask.IsEnabled( E_PoseFieldElement_TrajectoryDeltaTransformTranslation ) )
				BlendVector3( evalContext.blendMode, destT, &trajectoryDelta.GetTranslationField(), evalContext.weight );
		}
	
		if ( pose.HasTrajectoryTransform() )
		{
			// Blend in trajectory transform.
			C_QuaternionField* destQ = pose.GetQuaternionFieldElement( E_PoseFieldElement_TrajectoryTransformRotation, 0 );
			C_Vector3Field* destT = pose.GetVector3FieldElement( E_PoseFieldElement_TrajectoryTransformTranslation, 0 );
			
			if ( fieldMask.IsEnabled( E_PoseFieldElement_TrajectoryTransformRotation ) )
				BlendQuaternion( evalContext.blendMode, destQ, &evalContext.trajectoryRootTransform.GetRotationField(), evalContext.weight );
				
			if ( fieldMask.IsEnabled( E_PoseFieldElement_TrajectoryTransformTranslation ) )
				BlendVector3( evalContext.blendMode, destT, &evalContext.trajectoryRootTransform.GetTranslationField(), evalContext.weight );
		}
	}
}

void C_Cycle::EvaluateTrajectoryDelta(
	C_JointTransform& outTrajectoryDelta,
	C_CycleInstance& instance,
	const C_EvaluationContext& evalContext,
	const F32 time,
	const F32 timeInFrames
) const
{
	C_JointTransform trajectoryRootDelta;
	if ( instance.GetPreviousUpdateCounter() == ( evalContext.updateCounter - 1 ) )
	{
		// Calculate delta transform and replace previous cached root transform with the new one.
		C_JointTransform prevTrajectoryRootTransform = instance.GetPreviousTrajectoryTransform();
		
		const F32 prevTime = instance.GetPreviousTime();
		const F32 prevTimeInFrames = instance.GetPreviousTimeInFrames();
		
		if ( time > prevTime && timeInFrames < prevTimeInFrames )
		{
			// Wrapped around during forward playback, so add total trajectory root delta.
			
			// R1 relative to R2 = R2 * inv(R1)
			// current traj transform relative to traj start * traj finish  = finish followed by movement since first frame.
			
			trajectoryRootDelta = evalContext.trajectoryRootTransform * m_trajectoryDelta * prevTrajectoryRootTransform.Inverse();
		}
		else if ( time < prevTime && timeInFrames > prevTimeInFrames )
		{
			// Wrapped around during reverse playback, so add inverse of total trajectory delta.
			// TODO: check this.
			// curr traj relative to end = 
			trajectoryRootDelta = evalContext.trajectoryRootTransform * prevTrajectoryRootTransform.Inverse() * m_trajectoryDelta.Inverse();
		}
		else
		{
			trajectoryRootDelta = evalContext.trajectoryRootTransform * prevTrajectoryRootTransform.Inverse();
		}
	}
	else
	{
		// Animation wasn't playing last frame, so to avoid pops set the trajectory delta according to the average velocity.
		// Unfortunately we have to make an assumption about the rate at which this animation is being sampled at, currently 30Hz.
		// This is inevitable as there's not enough information at this point to derive the sample rate automatically.
		// It might be better to pass this sample rate value into the Evaluate function and scale pre-stored velocity curves.
		Maths::VF32 f = Maths::MakeVF32(m_frameRate * (1.f / 30.f) / m_playbackDuration);

		Animation::C_JointTransform partialDelta;
		partialDelta.SetIdentity();
		partialDelta.SetRotation(Maths::NearestSlerp(partialDelta.GetRotation(), m_trajectoryDelta.GetRotation(), f));
		partialDelta.SetTranslation(Maths::Lerp(partialDelta.GetTranslation(), m_trajectoryDelta.GetTranslation(), f));

		trajectoryRootDelta = m_trajectoryFirst * partialDelta * m_trajectoryFirst.Inverse();
	}

	instance.SetPreviousUpdateCounter( evalContext.updateCounter );
	instance.SetPreviousTime( time, timeInFrames );
	instance.SetPreviousTrajectoryTransform( evalContext.trajectoryRootTransform );
	
	outTrajectoryDelta = trajectoryRootDelta;
}

#ifndef PLATFORM_SPU

U32 C_Cycle::GetFieldMemoryUsage( U32 fieldIndex ) const
{
	if (IsPartiallyLoaded()) return 0;

	U32 memoryUsed = 0;
	const U16* pChannelInfo = m_channelInfo;

	static const U8 kConstantCurveSizes[E_ConstantCurveType_Count] =
	{
		sizeof(F32) * 3,
		sizeof(F32),
		sizeof(U16) * 3
	};

	for (U32 constantCurveType = 0; constantCurveType < E_ConstantCurveType_Count; ++constantCurveType)
	{
		for (U32 curveIndex = 0; curveIndex < m_numConstantCurvesOfType[constantCurveType]; ++curveIndex)
		{
			U32 cycleFieldIndex = *(pChannelInfo++) >> 6;
			if (cycleFieldIndex == fieldIndex)
			{
				memoryUsed += kConstantCurveSizes[constantCurveType];
			}
		}
	}

	for (U32 varyingCurveType = 0; varyingCurveType < E_VaryingCurveType_Count; ++varyingCurveType)
	{
		for (U32 curveIndex = 0; curveIndex < m_numVaryingCurvesOfType[varyingCurveType]; ++curveIndex)
		{
			U32 cycleFieldIndex = *(pChannelInfo++) >> 6;
			if (cycleFieldIndex == fieldIndex)
			{
				memoryUsed += GetVaryingCurveMemoryUsage(static_cast<E_VaryingCurveType>(varyingCurveType), curveIndex);
			}
		}
	}
	return memoryUsed;
}

U32 C_Cycle::GetVaryingCurveMemoryUsage(E_VaryingCurveType curveType, U32 curveIndex) const
{
	U32 numKeys = GetNumVaryingCurveKeys(curveType, curveIndex);

	// Size of key and size of constant data.
	static const U8 kKeySizes[E_VaryingCurveType_Count][2] =
	{
		{ sizeof(F32) * 3,	0 },
		{ sizeof(U32),		0 },
		{ sizeof(F32),		0 },
		{ sizeof(U16) * 3,	sizeof(F32) * 3 },
		{ sizeof(U8) * 3,	sizeof(F32) * 3 }
	};

	// Key time values for each key and one used as a key count per segment.
	U32 numKeyTimes = numKeys + m_numSegments;

	// Include initial keys in each segment.
	numKeys += m_numSegments + 1;

	U32 perKeySize = kKeySizes[curveType][0];

	// constant decompression data
	U32 overheadSize = kKeySizes[curveType][1];

	return numKeys * perKeySize + numKeyTimes * sizeof(U8) + overheadSize;
}

U32 C_Cycle::GetNumVaryingCurveKeys(E_VaryingCurveType curveType, U32 curveIndex) const
{
	if (curveIndex > m_numVaryingCurvesOfType[curveType])
	{
		BIZ_ASSERT_FAIL("Out of range curve index");
		return 0;
	}

	U32 totalKeys = 0;
	
	for (U32 i = 0; i < m_numSegments; ++i)
	{
		const C_Segment& segment = m_segments[i];
		C_CycleSegmentReader segmentReader(
				segment.m_data,
				segment.m_size,
				segment.m_numFrames,
				0.f,
				m_numVaryingCurvesOfType,
				m_totalVaryingCurves,
				segment.m_totalKeys);

		const U8* pKeyTimes = segmentReader.GetKeyTimes();

		U32 currCurveIndex = 0;
		U32 totalSegmentKeys = 0;
		U32 numKeys = 0;
		while (currCurveIndex < curveIndex && totalSegmentKeys < segment.m_totalKeys)
		{
			numKeys = pKeyTimes[0];
			pKeyTimes += numKeys + 1;
			++currCurveIndex;
			totalSegmentKeys += numKeys;
		}

		if (currCurveIndex == curveIndex)
		{
			totalKeys += numKeys;
		}
	}

	return totalKeys;
}


#endif

} // namespace Animation
