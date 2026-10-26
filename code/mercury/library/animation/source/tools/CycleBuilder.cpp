#include "stdafx.h"
#include <animation/tools/CycleBuilder.h>
#include <animation/tools/KeyReductionUtils.h>
#include <animation/tools/CompressedCurveUtils.h>
#include <animation/tools/CompressionParameters.h>
#include <animation/tools/Scene.h>
#include <animation/pose/PoseFieldTypes.h>
#include <animation/common/MemoryBlock.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <system/memory/MemoryTracker.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/misc/Progress.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <btl/containers/vector.h>
#include <btl/containers/bitvector.h>

#if 0
#define DBGLOG( ... )	LOGINFO( gLog, __VA_ARGS__ )
#else
#define DBGLOG( ... )
#endif

#define kCycleBuilderMemoryTag					"mercury.animation.tools.cyclebuilder"
#define kCycleBuilderRawFramesMemoryTag			"mercury.animation.tools.cyclebuilder.rawframes"
#define kCycleBuilderRawCurvesMemoryTag			"mercury.animation.tools.cyclebuilder.rawcurves"
#define kCycleBuilderCompressedCurvesMemoryTag	"mercury.animation.tools.cyclebuilder.compressedcurves"

namespace AnimationTools
{

namespace
{
	Debug::Logger& gLog = Debug::GetLogger( "mercury.animation.tools" );
	
	// Varying curve types, ordered by alignment (largest alignment first)
	static const E_CurveType kVaryingCurveTypes [] =
	{
		E_CurveType_VaryingVector3F32,
		E_CurveType_VaryingQuaternion32,
		E_CurveType_VaryingScalarF32,		
		E_CurveType_VaryingQuaternion48,
		E_CurveType_VaryingVector3U16,
		E_CurveType_VaryingVector3U8
	};
	
	static const U32 kNumVaryingCurves = sizeof( kVaryingCurveTypes ) / sizeof( kVaryingCurveTypes[ 0 ] );

	const CHAR8* GetElementName(Animation::E_PoseFieldElement e)
	{
		static const CHAR8* elementTypeNames[Animation::E_PoseFieldElement_Count] =
		{
			"Rotation",
			"Translation",
			"Scale",
			"Scalar",
			"Vector3",
			"Trajectory Delta Rotation",
			"Trajectory Delta Translation",
			"Trajectory Rotation",
			"Trajectory Translation"
			"Vector4 XYZ",
			"Vector4 W"
		};
		BIZ_ASSERT(e < Animation::E_PoseFieldElement_Count);
		return elementTypeNames[e];
	}
}

#ifdef PLATFORM_PC
}
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace AnimationTools
{
#endif

// Copy an array of values to the clipboard, ready to be pasted into Excel...
void CopyArrayToClipboard( const F32* values, U32 count )
{
#ifdef PLATFORM_PC
	if ( OpenClipboard(NULL) )
	{
		EmptyClipboard();

		// 60 bytes per line.
		size_t numBytes = 60 * count;

		HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, numBytes * sizeof(char));
		if (NULL != hData)
		{
			char* text = (char *) GlobalLock(hData);

			int numChars = 0;
			for (unsigned int i = 0; i < count && numChars >= 0; ++i, text += numChars)
			{
				numChars = _snprintf(text, numBytes, "%d\t%.3f\n", i, values[ i ] );
			}

			GlobalUnlock(hData);
			SetClipboardData(CF_TEXT, hData);
		}
		CloseClipboard();
	}
#endif
}



//
// C_CycleParameters
//
C_CycleParameters::C_CycleParameters():
	oversampling(1.f),
	maxGlobalRotationError(0.7f),
	maxGlobalTranslationError( 0.005f ),
	maxGlobalScaleError(1.f / 128.f),
	maxRelativeCustomCurveError( 1.f / 128.f ),	// Less than 1% relative error.
	maxHighFidelityRotationError(0.1f),
	maxHighFidelityTranslationError(0.0001f),
	maxHighFidelityScaleError(1.f / 512.f),
	removeTranslationChannelsMatchingBasePose( true ),
	removeScaleChannelsMatchingBasePose( true ),
	removeChannelsWithNoKeys( true ),
	removeIdentityScaleChannels( true ),
	inWorldSpace( false )
{
	worldOrigin[0] = 0.f;
	worldOrigin[1] = 0.f;
	worldOrigin[2] = 0.f;
}

bool C_CycleParameters::operator == ( const C_CycleParameters& rhs ) const
{
	return //oversampling == rhs.oversampling &&
		maxGlobalRotationError == rhs.maxGlobalRotationError &&
		maxGlobalTranslationError == rhs.maxGlobalTranslationError &&
		maxGlobalScaleError == rhs.maxGlobalScaleError &&
		maxHighFidelityRotationError == rhs.maxHighFidelityRotationError &&
		maxHighFidelityTranslationError == rhs.maxHighFidelityTranslationError &&
		maxHighFidelityScaleError == rhs.maxHighFidelityScaleError &&
		maxRelativeCustomCurveError == rhs.maxRelativeCustomCurveError &&
		removeTranslationChannelsMatchingBasePose == rhs.removeTranslationChannelsMatchingBasePose &&
		removeScaleChannelsMatchingBasePose == rhs.removeScaleChannelsMatchingBasePose &&
		removeIdentityScaleChannels == rhs.removeIdentityScaleChannels &&
		removeChannelsWithNoKeys == rhs.removeChannelsWithNoKeys &&
		inWorldSpace == rhs.inWorldSpace &&
		worldOrigin[0] == rhs.worldOrigin[0] &&
		worldOrigin[1] == rhs.worldOrigin[1] &&
		worldOrigin[2] == rhs.worldOrigin[2]
		;
}

//
// C_CycleBuilder
//

C_CycleBuilder::C_CycleBuilder()
{
	Clear();
}

C_CycleBuilder::~C_CycleBuilder()
{
	Clear();
}

void C_CycleBuilder::Clear()
{
	m_numFrames = 0;
	m_durationFrames = 0;
	m_frameRate = 0.f;
	m_numReferenceTransforms = 0;
	
	
	m_cycleJoints.Clear();
	m_customScalarCurves.Clear();
	m_customVector3Curves.Clear();
	m_customVector4Curves.Clear();
	m_cycleJointFromSkeletonJoint.Clear();
	
	for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
	{
		m_varyingCycleCurvesByType[ i ].Clear();
	}
	
	for ( U32 i = 0; i < Animation::E_ConstantCurveType_Count; ++i )
	{
		m_constantCycleCurvesByType[ i ].Clear();
	}

	for (U32 i = 0, count = m_cycleCurves.Size(); i < count; ++i)
	{
		delete m_cycleCurves[i];
		m_cycleCurves[i] = 0;
	}
	m_cycleCurves.Clear();

	m_cycleSegments.Clear();
	
	m_totalFields = 0;
	m_fieldKeyToIndexMap.Clear();
	m_fieldNames.Clear();
	m_fieldTypes.Clear();
		
	m_isAdditive = false;
}

bool C_CycleBuilder::IsValid() const
{
	return m_numFrames > 0;
}

namespace
{
	U32 GetNumFramesInScene( const Aftk::C_Scene& scene, F32 oversampling )
	{
		// 1.5 is to ensure final frame is included and round up non-integer end times.
		return static_cast< U32 >( Maths::Max( 0, static_cast<S32>( 1.5f + ( scene.GetDuration() * oversampling ) ) ) );
	}
}

C_Status C_CycleBuilder::BuildCycle(
	C_Scene& cycleScene,
	const C_CycleParameters& cycleParameters,
	Progress::I_ProgressBar* progressBar
)
{	
	MEMORY_TRACKER_PUSH( kCycleBuilderMemoryTag );
	
	Clear();
	
	m_isAdditive = false;
	
	m_numFrames = cycleScene.GetNumFrames();
		
	if ( 0 == m_numFrames )
	{
		return C_Status( false, "Cycle has zero frames" );	
	}
	
	const bool isLooped = true;
	if ( isLooped )
	{
		m_durationFrames = m_numFrames - 1;
	}
	else
	{
		m_durationFrames = m_numFrames;
	}

	if (0 == m_durationFrames)
	{
		return C_Status(false, "Animation has zero duration");
	}
		
	m_frameRate = cycleScene.GetFrameRate();
	
	cycleScene.SetCurrentFrame( 0 );
	C_Status status = ExtractConstantData( cycleScene, cycleParameters );
	
	if ( !status )
	{
		return status;
	}
	
	Progress::UpdateStepped buildProgress( progressBar, 3, "Building..." );
	
	// Sample animated data for each frame.
	SampleFrames( cycleScene, cycleParameters, &buildProgress );
	buildProgress.Step();
	
	BuildRawCurves();
	
	buildProgress.Step();
	
	// Build compressed curves.
	status  = BuildCompressedCurves( cycleParameters, cycleScene, &buildProgress );
	
	buildProgress.Step();
	
	if ( !status )
	{
		return status;
	}
	
	CalculateTrajectoryDelta(cycleParameters);
	
	MEMORY_TRACKER_POP( kCycleBuilderMemoryTag );
		
	return C_Status( true );
}

C_Status C_CycleBuilder::BuildAdditiveCycle(
	C_Scene& cycleLhsScene,
	C_Scene& cycleRhsScene,
	const C_CycleParameters& cycleParameters,
	Progress::I_ProgressBar* progressBar
)
{	
	MEMORY_TRACKER_PUSH( kCycleBuilderMemoryTag );
	
	Clear();

	if ( cycleLhsScene.GetNumNodes() != cycleRhsScene.GetNumNodes() )
	{
		return C_Status( false, "Cannot build additive animation as skeletons are different" );
	}

	// Build custom curves from LHS scene.
	//C_CustomCurveBuilder customCurveBuilder;
	//customCurveBuilder.BuildCustomCurves( cycleLhsScene, cycleParameters );
	
	//const F32 oversampling = cycleParameters.oversampling;
	
	U32 numFrames1 = cycleLhsScene.GetNumFrames();
	U32 numFrames2 = cycleRhsScene.GetNumFrames();
	
	// Always use the length of the source/lhs animation for the number of frames.
	m_numFrames = numFrames1; // Maths::Max( numFrames1, numFrames2 );
	
	if ( 0 == m_numFrames )
	{
		return C_Status( false, "Additive cycle has zero frames" );
	}
	
	m_isAdditive = true;
	
	const bool isLooped = true;
	if ( isLooped )
	{
		m_durationFrames = m_numFrames - 1;
	}
	else
	{
		m_durationFrames = m_numFrames;
	}
	
	F32 frameRate1 = cycleLhsScene.GetFrameRate();
	F32 frameRate2 = cycleLhsScene.GetFrameRate();
	
	if ( Maths::Abs( frameRate1 - frameRate2 ) > ( 1.f / 1024.f ) )
	{
		return C_Status( false, "Additive cycle inputs must have the same frame rate" );
	}
	
	m_frameRate = frameRate1;
	
	cycleLhsScene.SetCurrentFrame( 0 );
	C_Status status = ExtractConstantData( cycleLhsScene, cycleParameters );
	
	if ( !status )
	{
		return status;
	}	
	
	SampleAdditiveFrames( cycleLhsScene, cycleRhsScene, cycleParameters, progressBar );
	
	BuildRawCurves();
	
	// Build compressed curves.
	bool compressedOk = BuildCompressedCurves( cycleParameters, cycleLhsScene, progressBar );
	
	if ( !compressedOk )
	{
		return C_Status( false, "Error whilst building compressed curves." );
	}
	
	CalculateTrajectoryDelta(cycleParameters);
	
	MEMORY_TRACKER_POP( kCycleBuilderMemoryTag );
		
	return C_Status( true );
}

namespace
{
	bool IsVaryingCurveType( E_CurveType curveType )
	{
		return ( curveType >= E_CurveType_VaryingQuaternion32 && curveType <= E_CurveType_VaryingScalarF32 );
	}

	bool IsConstantCurveType( E_CurveType curveType )
	{
		return ( curveType >= E_CurveType_ConstantQuaternion48 && curveType <= E_CurveType_ConstantScalarF32 );
	}

	bool IsBasePoseCurveType( E_CurveType curveType )
	{
		return curveType >= E_CurveType_BasePoseRotation && curveType <= E_CurveType_BasePoseTranslation;
	}
	
	static const U32 kSparseBlockSize = 256;

	U32 GetNumSparseKeyTimes( U32 numKeys )
	{
		return ( numKeys + kSparseBlockSize - 1 ) / kSparseBlockSize;
	}

	void WriteU16SparseKeyTimes( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		const U32 numSparseKeys = GetNumSparseKeyTimes( curve.GetNumKeys() );
		for ( U32 i = 0; i < numSparseKeys; ++i )
		{
			U32 keyTime = curve.GetKeyTime( i * kSparseBlockSize );
			BIZ_ASSERT( keyTime < 65536 );
			out << U16( keyTime );
		}
	}
	
	void WriteU16KeyTimes( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		const U32 numKeys = curve.GetNumKeys();
		for ( U32 i = 0; i < numKeys; ++i )
		{
			U32 keyTime = curve.GetKeyTime( i );
			BIZ_ASSERT( keyTime < 65536 );
			out << U16( keyTime );			
		}
	}

	void WriteVaryingCurveHeaderAndKeyTimes(
		const C_Curve& curve,
		Sys::BinarySerialiserAutoEndian& out )
	{
		U32 numKeys = curve.GetNumKeys();
		U32 numSparseKeyTimes = GetNumSparseKeyTimes( numKeys );
		
		out << U16( numKeys );

		WriteU16SparseKeyTimes( curve, out );
		WriteU16KeyTimes( curve, out );
	}

	void WriteVec4F32Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			F32 key[ 4 ];
			curve.GetKeyDataVec4F32( keyIndex, key );
			out << key[ 0 ];
			out << key[ 1 ];
			out << key[ 2 ];
			out << key[ 3 ];
		}
	}

	void WriteVec3F32Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			F32 key[ 3 ];
			curve.GetKeyDataVec3F32( keyIndex, key );
			out << key[ 0 ];
			out << key[ 1 ];
			out << key[ 2 ];
		}
	}

	void WriteVec3U16Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			U16 key[ 3 ];
			curve.GetKeyDataVec3U16( keyIndex, key );
			out << key[ 0 ];
			out << key[ 1 ];
			out << key[ 2 ];
		}
	}

	void WriteVec3U8Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			U8 key[ 3 ];
			curve.GetKeyDataVec3U8( keyIndex, key );
			out << key[ 0 ];
			out << key[ 1 ];
			out << key[ 2 ];
		}
	}

	void WriteF32Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			F32 key;
			curve.GetKeyDataF32( keyIndex, key );
			out << key;
		}
	}

	void WriteU32Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			U32 key;
			curve.GetKeyDataU32( keyIndex, key );
			out << key;
		}
	}

	void WriteU16Keys( const C_Curve& curve, Sys::BinarySerialiserAutoEndian& out )
	{
		for ( U32 keyIndex = 0, keyCount = curve.GetNumKeys(); keyIndex < keyCount; ++keyIndex )
		{
			U16 key;
			curve.GetKeyDataU16( keyIndex, key );
			out << key;
		}
	}
}

class C_CurveAnalyser
{
public:

	void Reset()
	{
		for ( U32 i = 0; i < NUM_TIME_GAPS; ++i )
		{
			m_timeGaps[ i ] = 0;
		}
		m_totalGaps = 0;
	}

	void Analyse( const C_Curve& curve )
	{
		if ( curve.IsConstant() )
		{
			return;
		}
		
		m_totalGaps += curve.GetNumKeys() - 1;
		
		U32 prevKeyTime = curve.GetKeyTime( 0 );
		S32 prevTimeGap = 0;
		for ( U32 i = 1, count = curve.GetNumKeys(); i < count; ++i )
		{
			U32 currKeyTime = curve.GetKeyTime( i );
			S32 timeGap = currKeyTime - prevKeyTime;
			
			prevKeyTime = currKeyTime;
			
			S32 deltaTimeGap = Maths::Abs( timeGap - prevTimeGap );
			prevTimeGap = timeGap;
			
			
			timeGap = Maths::Abs( deltaTimeGap );	// check second order
			BIZ_ASSERT( timeGap >= 0 );
			
			if ( timeGap >= NUM_TIME_GAPS - 1 )
			{
				m_timeGaps[ NUM_TIME_GAPS - 1 ]++;
			}
			else
			{
				m_timeGaps[ timeGap ]++;
			}
		}
	}
	
	void Print( Debug::Logger& logger )
	{
		LOGINFO( logger, "==== Curve Analysis ===\n" );
		
		LOGINFO( logger, "Time gaps:\n" );
		
		for ( U32 i = 0; i < NUM_TIME_GAPS; ++i )
		{
			//LOGINFO( logger, "  %5d: %d (%.3f percent)\n", i, m_timeGaps[ i ], m_timeGaps[ i ] * 100.f / m_totalGaps );
			LOGINFO( logger, "%d\n", m_timeGaps[ i ] ); // , m_timeGaps[ i ] * 100.f / m_totalGaps );
		}
		LOGINFO( logger, "=======================\n" );
	}
	
private:
	static const U32 NUM_TIME_GAPS = 256;

	U32 m_timeGaps[ NUM_TIME_GAPS ];
	U32 m_totalGaps;
};

template < typename T >
U32 SerialiseKey( T& out, Animation::E_VaryingCurveType curveType, const KeyValue& keyValue )
{
	U32 numBytes = 0;
	switch ( curveType )
	{
		case Animation::E_VaryingCurveType_VaryingVector3F32:
			out << keyValue.m_vec3F32[ 0 ];
			out << keyValue.m_vec3F32[ 1 ];
			out << keyValue.m_vec3F32[ 2 ];
			numBytes = 12;
			break;
			
		case Animation::E_VaryingCurveType_VaryingQuaternion32:
			out << keyValue.m_u32;
			numBytes = 4;
			break;
			
		case Animation::E_VaryingCurveType_VaryingScalarF32:
			out << keyValue.m_f32;
			numBytes = 4;
			break;
			
		case Animation::E_VaryingCurveType_VaryingQuaternion48:
			out << keyValue.m_vec3U16[ 0 ];
			out << keyValue.m_vec3U16[ 1 ];
			out << keyValue.m_vec3U16[ 2 ];
			numBytes = 6;
			break;
			
		case Animation::E_VaryingCurveType_VaryingVector3U16:
			out << keyValue.m_vec3U16[ 0 ];
			out << keyValue.m_vec3U16[ 1 ];
			out << keyValue.m_vec3U16[ 2 ];
			numBytes = 6;
			break;
			
		case Animation::E_VaryingCurveType_VaryingVector3U8:
			out << keyValue.m_vec3U8[ 0 ];
			out << keyValue.m_vec3U8[ 1 ];
			out << keyValue.m_vec3U8[ 2 ];
			numBytes = 3;
			break;
		
		default:
			BIZ_ASSERT( false );
			break;
	}
	return numBytes;
}

inline U32 PackFieldElementInfo( U32 fieldIndex, Animation::E_PoseFieldElement elementType )
{
	BIZ_STATIC_ASSERT( Animation::E_PoseFieldElement_Count < 64 );
	BIZ_ASSERT( fieldIndex < 1024 );
	return (fieldIndex << 6) | elementType;
}

bool C_CycleBuilder::Serialise( Sys::I_BinaryStream& stream ) const
{
	Sys::BinarySerialiserAutoEndian out( stream );
	
	static const U32 kCycleVersion = 15;
	
	out << U32( kCycleVersion );

	// Reference transforms.
	out << U32( m_numReferenceTransforms );
	if ( m_numReferenceTransforms > 0 )
	{
		out << m_referenceTransform;
	}
	
	// Trajectory delta (now calculated at load time).
	out << m_trajectoryStart;
	out << m_trajectoryFinish;
	out << m_trajectoryRootFieldIndex;
	
	out << U32( m_numFrames );
	out << F32( m_durationFrames );
	out << F32( m_frameRate );
		
	//
	// Pose fields.
	//

	if ( m_totalFields >= 65536 )
	{
		LOGERROR( gLog, "Too many (greater than 65535) fields in cycle" );
		BIZ_ASSERT( false );
		return false;
	}
	
	out << U32( m_totalFields );
	
	// Write field types.
	for ( U32 i = 0; i < m_totalFields; ++i )
	{
		out << U8( m_fieldTypes[ i ] );
	}
	
	//
	// Write field names.
	//
	out << U32( m_totalFields );
	for ( U32 i = 0; i < m_totalFields; ++i )
	{
		Util::C_String tmpStr( m_fieldNames[ i ] );
		out << tmpStr;
	}
	
	//
	// Collect info about curves and fields.
	//
	static const U32 kConstantCurveDataSize[ Animation::E_ConstantCurveType_Count ] =
	{
		12,		//E_ConstantCurveType_ConstantVector3F32,
		4,		//E_ConstantCurveType_ConstantScalarF32,
		6		//E_ConstantCurveType_ConstantQuaternion48,
	};
	
	static const U32 kVaryingCurveConstantDataSize[ Animation::E_VaryingCurveType_Count ] =
	{
		0,			//E_VaryingCurveType_VaryingVector3F32	
		0,			//E_VaryingCurveType_VaryingQuaternion32
		0,			//E_VaryingCurveType_VaryingScalarF32,
		0,			//E_VaryingCurveType_VaryingQuaternion48
		6 * 4,		//E_VaryingCurveType_VaryingVector3U16,
		6 * 4		//E_VaryingCurveType_VaryingVector3U8,	
	};
	
	// First pass: count curves and fields.
	

	U32 numConstantCurves = 0;
	U32 numVaryingCurves = 0;
	U32 constantDataSize = 0;
	
	for ( U32 i = 0; i < Animation::E_ConstantCurveType_Count; ++i )
	{
		U32 numCurves = m_constantCycleCurvesByType[ i ].Size();
		numConstantCurves += numCurves;
		constantDataSize += numCurves * kConstantCurveDataSize[ i ];
	}
	
	for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
	{
		U32 numCurves = m_varyingCycleCurvesByType[ i ].Size();
		numVaryingCurves += numCurves;
		constantDataSize += numCurves * kVaryingCurveConstantDataSize[ i ];
	}
	
	
	// Second pass: write curve counts by type.
	BIZ_ASSERT( numConstantCurves < 65536 );
	BIZ_ASSERT( numVaryingCurves < 65536 );
	out << U16( numConstantCurves );
	out << U16( numVaryingCurves );

	for ( U32 i = 0; i < Animation::E_ConstantCurveType_Count; ++i )
	{
		out << U16( m_constantCycleCurvesByType[ i ].Size() );
	}
	
	for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
	{
		out << U16( m_varyingCycleCurvesByType[ i ].Size() );
	}
		
	//
	// Write field info for all constant and varying curves.
	//
	for ( U32 i = 0; i < Animation::E_ConstantCurveType_Count; ++i )
	{
		const Btl::Vector< C_CycleCurve* >& cycleCurves = m_constantCycleCurvesByType[ i ];
		for ( U32 curveIndex = 0, count = cycleCurves.Size(); curveIndex < count; ++curveIndex )
		{
			U32 channelInfo = PackFieldElementInfo( cycleCurves[ curveIndex ]->m_fieldIndex, cycleCurves[ curveIndex ]->m_fieldElement );
			out << U16( channelInfo );
		}
	}

	for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
	{
		const Btl::Vector< C_CycleCurve* >& cycleCurves = m_varyingCycleCurvesByType[ i ];
		for ( U32 curveIndex = 0, count = cycleCurves.Size(); curveIndex < count; ++curveIndex )
		{
			U32 channelInfo = PackFieldElementInfo( cycleCurves[ curveIndex ]->m_fieldIndex, cycleCurves[ curveIndex ]->m_fieldElement );
			out << U16( channelInfo );
		}
	}
	
	//
	// Write constant curve data. This consists of any constant data used by varying curves and
	// the values of constant curves. By storing the size of all the constant data it should be
	// possible for the runtime to load this into a single contiguous memory block.
	//
	out << U32( constantDataSize );

	// Bounded vector3 varying curves each have a bounding box.
	static const Animation::E_VaryingCurveType kBoundedVector3CurveTypes[] =
	{
		Animation::E_VaryingCurveType_VaryingVector3U16,
		Animation::E_VaryingCurveType_VaryingVector3U8
	};
	static const U32 kNumBoundedVector3CurveTypes = sizeof( kBoundedVector3CurveTypes ) / sizeof( kBoundedVector3CurveTypes[ 0 ] );
	
	for ( U32 i = 0; i < kNumBoundedVector3CurveTypes; ++i )
	{
		Animation::E_VaryingCurveType curveType = kBoundedVector3CurveTypes[ i ];
		const Btl::Vector< C_CycleCurve* >& cycleCurves = m_varyingCycleCurvesByType[ curveType ];
		
		for ( U32 curveIndex = 0, count = cycleCurves.Size(); curveIndex < count; ++curveIndex )
		{
			const C_Curve& curve = *cycleCurves[ curveIndex ]->m_curve;

			out << curve.GetOffset()[ 0 ];
			out << curve.GetOffset()[ 1 ];
			out << curve.GetOffset()[ 2 ];		

			out << curve.GetScale()[ 0 ];
			out << curve.GetScale()[ 1 ];
			out << curve.GetScale()[ 2 ];
		}
	}

	// Write constant vec3 F32 values.
	{
		const Btl::Vector< C_CycleCurve* >& cycleCurves = m_constantCycleCurvesByType[ Animation::E_ConstantCurveType_ConstantVector3F32 ];
		for ( U32 curveIndex = 0, count = cycleCurves.Size(); curveIndex < count; ++curveIndex )
		{
			WriteVec3F32Keys( *cycleCurves[ curveIndex ]->m_curve, out );
		}
	}

	// Write constant scalar F32 values.
	{
		const Btl::Vector< C_CycleCurve* >& cycleCurves = m_constantCycleCurvesByType[ Animation::E_ConstantCurveType_ConstantScalarF32 ];
		for ( U32 curveIndex = 0, count = cycleCurves.Size(); curveIndex < count; ++curveIndex )
		{
			BIZ_ASSERT( cycleCurves[ curveIndex ]->m_curve->GetNumKeys() == 1 );
			out << F32( GetFirstScalarKey( *cycleCurves[ curveIndex ]->m_curve ) );
		}
	}
		
	// Write constant quaternion 48 values.
	{
		const Btl::Vector< C_CycleCurve* >& cycleCurves = m_constantCycleCurvesByType[ Animation::E_ConstantCurveType_ConstantQuaternion48 ];
		for ( U32 curveIndex = 0, count = cycleCurves.Size(); curveIndex < count; ++curveIndex )
		{
			WriteVec3U16Keys( *cycleCurves[ curveIndex ]->m_curve, out );
		}
	}
	
	//
	// Write segment data.
	//
	const U32 numSegments = m_cycleSegments.Size();
	
	// First pass: find total segment size.
	U32 segmentDataSize = 0;
	for ( U32 i = 0; i < numSegments; ++i )
	{
		segmentDataSize += m_cycleSegments[ i ].m_size;
		BIZ_ASSERT( ( segmentDataSize & 0xf ) == 0 );
	}
	
	// Write number of segments and total size of segment data.
	out << U32( numSegments );
	out << U32( segmentDataSize );
		
	// Write segment headers.
	for ( U32 i = 0; i < numSegments; ++i )
	{
		out << U32( m_cycleSegments[ i ].m_startFrame );
		out << U16( m_cycleSegments[ i ].m_numFrames );
		out << U16( m_cycleSegments[ i ].m_size );
	}
	
//	#define USE_BITFIELDS

	// Write segments.
	for ( U32 i = 0; i < numSegments; ++i )
	{
		//SerialiseCycleSegment( out, m_cycleSegments[ i ] );
		
		const C_CycleSegment& seg = m_cycleSegments[ i ];
		
		// Calculate and write initial keys for all varying curves.
		U32 size = 0;
		for ( U32 varyingCurveTypeIndex = 0; varyingCurveTypeIndex < Animation::E_VaryingCurveType_Count; ++varyingCurveTypeIndex )
		{
			U32 numCurves = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ].Size();
			
			for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
			{
				const C_CycleCurve* cycleCurve = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ][ curveIndex ];

				// Search for *first* key corresponding to the segment's start frame in the compressed and key reduced curve.
				S32 keyIndex = cycleCurve->m_curve->FindFirstKeyIndexForTime(seg.m_startFrame);

				KeyValue compressedKeyValue;

				if (keyIndex >= 0)
				{
					// Get initial key value directly from the compressed and key reduced curve.
					cycleCurve->m_curve->GetKeyData(keyIndex, compressedKeyValue);
				}
				else
				{
					// Get initial key value from raw (frame-by-frame) curve.
					KeyValue keyValue;
					cycleCurve->m_rawCurve->GetKeyData(seg.m_startFrame, keyValue);
				
					// Convert.
					ConvertKeyValue(
						cycleCurve->m_rawCurve->GetKeyType(),
						cycleCurve->m_curve->GetKeyType(),
						cycleCurve->m_curve->GetOffset(),
						cycleCurve->m_curve->GetScale(),
						keyValue,
						compressedKeyValue );
				}

				size += SerialiseKey( out, (Animation::E_VaryingCurveType) varyingCurveTypeIndex, compressedKeyValue );
			}						
		}
		
		if ( i < numSegments - 1 )
		{
			// TODO: pad to align?
			
			// Write number of keys for each varying curve type.
			for ( U32 varyingCurveTypeIndex = 0; varyingCurveTypeIndex < Animation::E_VaryingCurveType_Count; ++varyingCurveTypeIndex )
			{
				U32 numKeys = seg.m_numVaryingCurveKeys[ varyingCurveTypeIndex ];
				BIZ_ASSERT( numKeys < 65536 );
				out << U16( numKeys );
			}

#ifdef USE_BITFIELDS
			// Create bitstream indicating which curves have keys for each frame of the segment.
			Btl::BitVector keyPresentBitstream;
			
			for ( U32 varyingCurveTypeIndex = 0; varyingCurveTypeIndex < Animation::E_VaryingCurveType_Count; ++varyingCurveTypeIndex )
			{
				U32 numCurves = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ].Size();
				
				U32 totalKeys = 0;
				
				for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
				{
					const C_CycleCurve& cycleCurve = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ][ curveIndex ];
					
					for ( U32 frame = 0; frame < seg.m_numFrames; ++frame )
					{
						bool keyPresent = cycleCurve.m_curveSegments[ seg.m_index ].m_keyPresentAtFrame.GetBitValue( frame );
						keyPresentBitstream.PushBack( keyPresent );
						totalKeys += keyPresent ? 1 : 0;
					}
				}
				
				BIZ_ASSERT( totalKeys == seg.m_numVaryingCurveKeys[ varyingCurveTypeIndex ] );
			}
			
			// Swap order of bits.
			Btl::Vector< U8 > reversedBytes;
			reversedBytes.Resize( keyPresentBitstream.SizeInBytes() );
			
			for ( U32 j = 0; j < reversedBytes.Size(); ++j )
			{
				reversedBytes[ j ] = 0;
			}
			
			for ( U32 j = 0; j < keyPresentBitstream.Size(); ++j )
			{
				U32 byteIndex = j / 8;
				U32 bitIndex = 7 - ( j & 7 );
				if ( keyPresentBitstream.GetBitValue( j ) )
				{
					reversedBytes[ byteIndex ] |= 1 << bitIndex;
				}
			}
								
			// Write bitstream as bytes.
			//out.Write( keyPresentBitstream.Data(), keyPresentBitstream.SizeInBytes() );
			if ( reversedBytes.Size() > 0 )
			{
				out.Write( &reversedBytes[ 0 ], reversedBytes.Size() );
				size += keyPresentBitstream.SizeInBytes();
			}
#else

			// Write array of key times for each curve.
			for ( U32 varyingCurveTypeIndex = 0; varyingCurveTypeIndex < Animation::E_VaryingCurveType_Count; ++varyingCurveTypeIndex )
			{
				U32 numCurves = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ].Size();
				
				U32 totalKeys = 0;
				
				for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
				{
					const C_CycleCurve* cycleCurve = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ][ curveIndex ];
					
					// Get number of keys in segment, excluding initial and final frames.
					U32 numKeys = cycleCurve->m_curveSegments[ seg.m_index ].m_numKeys;
					
					totalKeys += numKeys;

					// Write number of keys in this curve.
					BIZ_ASSERT( numKeys < 256 );
					out << U8( numKeys );
					
					// Write key times.
					U32 firstKeyIndex = cycleCurve->m_curveSegments[ seg.m_index ].m_initialKeyIndex;
			
					for ( U32 keyIndex = 0; keyIndex < numKeys; ++keyIndex )
					{
						U32 keyTime = cycleCurve->m_curve->GetKeyTime( firstKeyIndex + keyIndex );
						S32 localKeyTime = keyTime - seg.m_startFrame;
						BIZ_ASSERT( localKeyTime >= 0 );
						BIZ_ASSERT( localKeyTime < 256 );
						BIZ_ASSERT( localKeyTime <= S32( seg.m_numFrames ) );
						out << U8( localKeyTime );
					}
					
				}
				
				BIZ_ASSERT( totalKeys == seg.m_numVaryingCurveKeys[ varyingCurveTypeIndex ] );
			}
#endif			
			
			// TODO: pad up to 4 byte boundary?
			
			// Write keys 
			for ( U32 varyingCurveTypeIndex = 0; varyingCurveTypeIndex < Animation::E_VaryingCurveType_Count; ++varyingCurveTypeIndex )
			{
				U32 numCurves = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ].Size();
				
				for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
				{
					const C_CycleCurve* cycleCurve = m_varyingCycleCurvesByType[ varyingCurveTypeIndex ][ curveIndex ];
					
					U32 initialKeyIndex = cycleCurve->m_curveSegments[ seg.m_index ].m_initialKeyIndex;
					U32 numKeys = cycleCurve->m_curveSegments[ seg.m_index ].m_numKeys;
					
					for ( U32 keyIndex = 0; keyIndex < numKeys; ++keyIndex )
					{
						KeyValue keyValue;
						cycleCurve->m_curve->GetKeyData( keyIndex + initialKeyIndex, keyValue );
						
						size += SerialiseKey( out, (Animation::E_VaryingCurveType) varyingCurveTypeIndex, keyValue );
					}
				}						
			}
			
			// TODO: pad up to 16 byte boundary?
		}
	}

	// TODO: move string tables towards start of file in order to allow streaming of segments

	
	return true;
}


bool C_CycleBuilder::IsAdditive() const
{
	return m_isAdditive;
}

F32 C_CycleBuilder::GetFrameRate() const
{
	return m_frameRate;
}

F32 C_CycleBuilder::GetDurationSeconds() const
{
	return m_durationFrames / GetFrameRate();
}

U32 C_CycleBuilder::GetDurationFrames() const
{
	return m_durationFrames;
}

F32 CalcSampleTime( F32 timeFrames, U32 durationFrames )
{
	F32 fDurationFrames = static_cast< F32 >( durationFrames );
	
	F32 animTime = 0.f;
	
	if ( true ) // isLooped
	{
		F32 fLoopCount = floorf( timeFrames / fDurationFrames );
		animTime = timeFrames - fDurationFrames * fLoopCount;

		BIZ_ASSERT( animTime >= 0.f );
	}
	return animTime;
}

Animation::C_JointTransform C_CycleBuilder::GetLocalJointTransform( U32 skeletonJointIndex, F32 timeSeconds, E_Source source ) const
{
	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
	
	if ( cycleJointIndex < 0 )
	{
		Animation::C_JointTransform identity;
		identity.SetIdentity();
		return identity;
	}
	
	F32 timeFrames = CalcSampleTime( timeSeconds * GetFrameRate(), m_durationFrames );

	S32 frame = static_cast< S32 >( timeFrames );
	S32 nextFrame = Maths::Min( frame + 1, static_cast< S32 >( m_numFrames - 1 ) );
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	Maths::C_Quaternion rotation;
	Maths::HVec3 translation;
	
	if ( source == E_Source_Compressed )
	{
		if ( cycleJoint.m_rotationCurve.HasKeys() )
		{
			rotation = SampleQuaternionCurve( cycleJoint.m_rotationCurve, timeFrames );
		}
		else if ( cycleJoint.m_rotationCurve.IsBasePose() )
		{
			rotation = Maths::C_Quaternion( cycleJoint.m_localBasePoseTransform.GetRotation() );
		}
		else
		{
			// identity rotation.
		}
		
		if ( cycleJoint.m_translationCurve.HasKeys() )
		{
			translation = SampleVector3Curve( cycleJoint.m_translationCurve, timeFrames );
		}
		else if ( cycleJoint.m_translationCurve.IsBasePose() )
		{
			translation = Maths::HVec3( cycleJoint.m_localBasePoseTransform.GetTranslation() );
		}
		else
		{
			// zero translation.
		}
	}
	else if ( source == E_Source_Original )
	{
		Maths::HF32 frameFrac = Maths::HF32( timeFrames - frame );
		rotation = !( cycleJoint.m_rotationFrames[ frame ].ShortestSlerp( cycleJoint.m_rotationFrames[ nextFrame ], frameFrac ) );
		//rotation = cycleJoint.m_rotationFrames[ frame ];
		
		Maths::HVec3 t1( cycleJoint.m_translationFrames[ frame ].v );
		Maths::HVec3 t2( cycleJoint.m_translationFrames[ nextFrame ].v );
		
		translation = t1.Lerp( t2, frameFrac );
	}
	else
	{
		BIZ_ASSERT( false );
	}
	
	return Animation::C_JointTransform( rotation, translation );
}


Animation::C_JointTransform C_CycleBuilder::GetLocalJointDeltaTransform(
	U32 skeletonJointIndex,
	F32 prevTimeSeconds,
	F32 currTimeSeconds,
	E_Source source
) const
{
	F32 prevTimeFrames = CalcSampleTime( prevTimeSeconds * GetFrameRate(), m_durationFrames );
	F32 currTimeFrames = CalcSampleTime( currTimeSeconds * GetFrameRate(), m_durationFrames );

	// Sample root transforms for current and previous times.
	Animation::C_JointTransform prevTransform = GetLocalJointTransform( skeletonJointIndex, prevTimeSeconds, source );
	Animation::C_JointTransform currTransform = GetLocalJointTransform( skeletonJointIndex, currTimeSeconds, source );

	if ( currTimeSeconds > prevTimeSeconds &&
		currTimeFrames < prevTimeFrames )
	{
		// Wrapped around during forward playback. Need to add the entire
		// animation delta.

		BIZ_ASSERT( prevTimeFrames < static_cast<S32>( m_durationFrames ) );
		BIZ_ASSERT( currTimeFrames < static_cast<S32>( m_durationFrames ) );

		const Animation::C_JointTransform startFrame = GetFirstLocalJointTransform( skeletonJointIndex, source );
		const Animation::C_JointTransform endFrame = GetLastLocalJointTransform( skeletonJointIndex, source );
		const Animation::C_JointTransform fullDelta = endFrame * startFrame.Inverse();
		
		currTransform *= fullDelta;
	}
	else if ( currTimeSeconds < prevTimeSeconds &&
			 currTimeFrames > prevTimeFrames )
	{
		// Wrapped around during reverse playback.
		// TODO
		BIZ_ASSERT(!"TODO");
	}

	// Calculate delta from previous transform to current.
	//currTransform.RightMultiplyByInverse( prevTransform );
	currTransform *= prevTransform.Inverse();
		
	//currTransform.NormaliseRotation();
	currTransform.Normalise();

	return currTransform;
}

Animation::C_JointTransform C_CycleBuilder::GetFirstLocalJointTransform( U32 skeletonJointIndex, E_Source source ) const
{
	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
		
	if ( m_numFrames == 0 || cycleJointIndex < 0 )
	{
		Animation::C_JointTransform identity;
		identity.SetIdentity();
		return identity;
	}
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	
	Maths::C_Quaternion rotation;
	Maths::HVec3 translation;

	if ( source == E_Source_Compressed )
	{
		if ( cycleJoint.m_rotationCurve.HasKeys() )
		{
			rotation = GetFirstQuaternionKey( cycleJoint.m_rotationCurve );
		}
		else if ( cycleJoint.m_rotationCurve.IsBasePose() )
		{
			rotation = Maths::C_Quaternion( cycleJoint.m_localBasePoseTransform.GetRotation() );
		}
		else
		{
			// identity rotation
		}
		
		if ( cycleJoint.m_translationCurve.HasKeys() )
		{
			translation = GetFirstVector3Key( cycleJoint.m_translationCurve );
		}
		else if ( cycleJoint.m_translationCurve.IsBasePose() )
		{
			translation = Maths::HVec3( cycleJoint.m_localBasePoseTransform.GetTranslation() );
		}
		else
		{
			// zero translation
		}
	}
	else if ( source == E_Source_Original )
	{
		rotation = !cycleJoint.m_rotationFrames[ 0 ];
		translation = Maths::HVec3( cycleJoint.m_translationFrames[ 0 ].v );
	}
	else
	{
		BIZ_ASSERT( false );
	}
	
	return Animation::C_JointTransform( rotation, translation );
}

Animation::C_JointTransform C_CycleBuilder::GetLastLocalJointTransform( U32 skeletonJointIndex, E_Source source ) const
{
	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
		
	if ( m_numFrames == 0 || cycleJointIndex < 0 )
	{
		Animation::C_JointTransform identity;
		identity.SetIdentity();
		return identity;
	}
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	
	Maths::C_Quaternion rotation;
	Maths::HVec3 translation;

	if ( source == E_Source_Compressed )
	{
		if ( cycleJoint.m_rotationCurve.HasKeys() )
		{
			rotation = GetLastQuaternionKey( cycleJoint.m_rotationCurve );
		}
		else if ( cycleJoint.m_rotationCurve.IsBasePose() )
		{
			rotation = Maths::C_Quaternion( cycleJoint.m_localBasePoseTransform.GetRotation() );
		}
		else
		{
			// identity rotation
		}
		
		if ( cycleJoint.m_translationCurve.HasKeys() )
		{
			translation = GetLastVector3Key( cycleJoint.m_translationCurve );
		}
		else if ( cycleJoint.m_translationCurve.IsBasePose() )
		{
			translation = Maths::HVec3( cycleJoint.m_localBasePoseTransform.GetTranslation() );
		}
		else
		{
			// zero translation
		}
	}
	else if ( source == E_Source_Original )
	{
		rotation = !cycleJoint.m_rotationFrames[ m_numFrames - 1 ];
		translation = Maths::HVec3( cycleJoint.m_translationFrames[ m_numFrames - 1 ].v );
	}
	else
	{
		BIZ_ASSERT( false );
	}
	
	return Animation::C_JointTransform( rotation, translation );
}

U32 C_CycleBuilder::GetNumRotationKeys( U32 skeletonJointIndex, E_Source source ) const
{
	if ( !IsValid() )
	{
		return 0;		
	}

	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
		
	if ( cycleJointIndex < 0 )
	{
		return 0;
	}
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	
	U32 numKeys = 0;
	
	switch ( source )
	{
		case E_Source_Compressed:
			if ( cycleJoint.m_rotationCurve.HasKeys() )
			{
				numKeys = cycleJoint.m_rotationCurve.GetNumKeys();
			}
			break;
		
		case E_Source_Original:
			numKeys = m_numFrames;
			break;
		
		default:
			BIZ_ASSERT( false );
			break;
	}
	
	return numKeys;
}

void C_CycleBuilder::GetRotationKey( U32 skeletonJointIndex, U32 keyIndex, U32& outKeyTime, Maths::C_Quaternion& outKey, E_Source source ) const
{
	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
		
	if ( cycleJointIndex < 0 )
	{
		outKey = Maths::C_Quaternion();
		outKeyTime = 0;
		return;
	}
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	
	switch ( source )
	{
		case E_Source_Compressed:
			if ( cycleJoint.m_rotationCurve.HasKeys() )
			{
				outKey = cycleJoint.m_rotationCurve.GetQuaternionKey( keyIndex );
				outKeyTime = cycleJoint.m_rotationCurve.GetKeyTime( keyIndex );
			}
			else
			{
				outKey = Maths::C_Quaternion( cycleJoint.m_localBasePoseTransform.GetRotation() );
				outKeyTime = 0;
			}
			break;
		
		case E_Source_Original:
			outKey = cycleJoint.m_rotationFrames[ keyIndex ];
			outKeyTime = keyIndex;
			break;
		
		default:
			BIZ_ASSERT( false );
			break;
	}
}

U32 C_CycleBuilder::GetNumTranslationKeys( U32 skeletonJointIndex, E_Source source ) const
{
	if ( !IsValid() )
	{
		return 0;		
	}

	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
		
	if ( cycleJointIndex < 0 )
	{
		return 0;
	}
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	
	U32 numKeys = 0;
	
	switch ( source )
	{
		case E_Source_Compressed:
			if ( cycleJoint.m_translationCurve.HasKeys() )
			{
				numKeys = cycleJoint.m_translationCurve.GetNumKeys();
			}
			break;
		
		case E_Source_Original:
			numKeys = m_numFrames;
			break;
		
		default:
			BIZ_ASSERT( false );
			break;
	}
	
	return numKeys;
}

void C_CycleBuilder::GetTranslationKey( U32 skeletonJointIndex, U32 keyIndex, U32& outKeyTime, Maths::HVec3& outKey, E_Source source ) const
{
	S32 cycleJointIndex = m_cycleJointFromSkeletonJoint[ skeletonJointIndex ];
		
	if ( cycleJointIndex < 0 )
	{
		outKey = Maths::HVec3();
		outKeyTime = 0;
		return;
	}
	
	const CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
	
	switch ( source )
	{
		case E_Source_Compressed:
			if ( cycleJoint.m_translationCurve.HasKeys() )
			{
				outKey = cycleJoint.m_translationCurve.GetVector3Key( keyIndex );
				outKeyTime = cycleJoint.m_translationCurve.GetKeyTime( keyIndex );
			}
			else
			{
				outKey = Maths::HVec3( cycleJoint.m_localBasePoseTransform.GetTranslation() );
				outKeyTime = 0;
			}
			break;
		
		case E_Source_Original:
			outKey = Maths::HVec3( cycleJoint.m_translationFrames[ keyIndex ].v );
			outKeyTime = keyIndex;
			break;
		
		default:
			BIZ_ASSERT( false );
			break;
	}
}

void C_CycleBuilder::AddCycleCurve( C_CycleCurve* cycleCurve )
{
	E_CurveType curveType = cycleCurve->m_curve->GetType();
	m_cycleCurves.PushBack(cycleCurve);
	
	U32 newFieldIndex = m_totalFields;
	C_FieldKeyToIndexMap::InsertionReturnType result = m_fieldKeyToIndexMap.Insert(
		C_FieldKeyToIndexMap::DataType( C_FieldKey( cycleCurve->m_fieldType, cycleCurve->m_fieldName.Lower() ), newFieldIndex )
	);
	
	if ( result.inserted )
	{
		// New field.
		m_totalFields++;
		m_fieldNames.PushBack( cycleCurve->m_fieldName );
		m_fieldTypes.PushBack( cycleCurve->m_fieldType );
	}
	
	cycleCurve->m_fieldIndex = result.iterator->GetValue();

	if ( cycleCurve->m_curve->IsConstant() )
	{
		Animation::E_ConstantCurveType constantCurveType = GetConstantCurveType( curveType );
		m_constantCycleCurvesByType[constantCurveType ].PushBack( cycleCurve );
		BIZ_ASSERT( m_constantCycleCurvesByType[ constantCurveType ].Back()->m_curve->IsConstant() );
	}
	else
	{
		BIZ_ASSERT( cycleCurve->m_curve->IsVarying() );
		Animation::E_VaryingCurveType varyingCurveType = GetVaryingCurveType( curveType );
		m_varyingCycleCurvesByType[ varyingCurveType ].PushBack( cycleCurve );
		BIZ_ASSERT( m_varyingCycleCurvesByType[ varyingCurveType ].Back()->m_curve->IsVarying() );
	}
}

void C_CycleBuilder::BuildListOfCycleJoints( const C_Scene& scene, const C_CycleParameters& cycleParameters )
{
	const U32 numJoints = scene.GetNumNodes();
	
	m_cycleJointFromSkeletonJoint.Resize( numJoints );
	
	for ( U32 i = 0; i < numJoints; ++i )
	{
		m_cycleJointFromSkeletonJoint[ i ] = -1;
	}
	
	// Clear existing joint curves.
	m_cycleJoints.Clear();
	m_cycleJoints.Reserve( numJoints );
	
	// Add joint curves for all skeleton joints to be included in the cycle.
	// Joints that are present in the skeleton but don't have corresponding scene nodes in 
	// the cycle scene are removed.
	for ( U32 i = 0; i < numJoints; ++i )
	{
		bool includeJointInCycle = true; 
		bool includeRotation = true;
		bool includeTranslation = true;
		bool includeScale = true;
		
		// Ignore joints that have their rotation and translation entirely constrained.
		if ( scene.NodeHasRotationConstraint( i ) )
		{
			includeRotation = false;
		}
		
		if ( scene.NodeHasTranslationConstraint( i ) )
		{
			includeTranslation = false;
		}
		
		// Optionally reject joints with no keys. Only do this if working in local space as world space
		// animations will have nodes that inherit animation from their parents, even if those nodes have no keys
		// themselves.
		if ( cycleParameters.removeChannelsWithNoKeys && !cycleParameters.inWorldSpace )
		{
			if ( !scene.NodeHasRotationKeys( i ) )
			{
				includeRotation = false;
			}
			
			if ( !scene.NodeHasTranslationKeys( i ) )
			{
				includeTranslation = false;
			}
			
			if ( !scene.NodeHasScaleKeys( i ) )
			{
				includeScale = false;
			}
		}
		
		if ( includeJointInCycle && !(includeRotation || includeTranslation || includeScale) )
		{
			
			//LOGDEBUG( gLog, "Joint %d '%s' removed as it has no rotation, translation or scale\n", i, scene.GetNodeName( i ) );
			includeJointInCycle = false;
		}
				
		if ( includeJointInCycle )
		{
			if (!includeRotation)
			{
				//LOGDEBUG( gLog, "Rotation channel removed for joint %d '%s' as it has no keys", i, scene.GetNodeName( i ) );
			}
			if (!includeTranslation)
			{
				//LOGDEBUG( gLog, "Translation channel removed for joint %d '%s' as it has no keys", i, scene.GetNodeName( i ) );
			}
			if (!includeScale)
			{
				//LOGDEBUG( gLog, "Scale channel removed for joint %d '%s' as it has no keys", i, scene.GetNodeName( i ) );
			}
		}
				
		if ( includeJointInCycle )
		{
			m_cycleJointFromSkeletonJoint[ i ] = m_cycleJoints.Size();
			m_cycleJoints.PushBack( CycleJoint( i ) );
			m_cycleJoints.Back().m_name = scene.GetNodeName( i );
			m_cycleJoints.Back().m_rotationFrames.Reserve( m_numFrames );
			m_cycleJoints.Back().m_rotationDiscontinuityFlags.Reserve( m_numFrames );
			m_cycleJoints.Back().m_translationFrames.Reserve( m_numFrames );
			m_cycleJoints.Back().m_translationDiscontinuityFlags.Reserve( m_numFrames );
			m_cycleJoints.Back().m_scaleFrames.Reserve( m_numFrames );
			m_cycleJoints.Back().m_scaleDiscontinuityFlags.Reserve( m_numFrames );
			m_cycleJoints.Back().m_localBasePoseTransform;
			scene.GetLocalNodeBindPoseTransform( i, m_cycleJoints.Back().m_localBasePoseTransform );
			m_cycleJoints.Back().m_hasRotation = includeRotation;
			m_cycleJoints.Back().m_hasTranslation = includeTranslation;
			m_cycleJoints.Back().m_hasScale = includeScale;
		}
	}
}

void C_CycleBuilder::BuildListOfCustomCurves( const C_Scene& scene, const C_CycleParameters& cycleParameters )
{
	// Custom scalars.
	m_customScalarCurves.Clear();
	m_customScalarCurves.Resize( scene.GetNumCustomScalars() );
	
	for ( U32 i = 0; i < scene.GetNumCustomScalars(); ++i )
	{
		CustomScalarCurve& c = m_customScalarCurves[i];
		c.name = scene.GetCustomScalarName( i );
		c.frames.Reserve( m_numFrames );
		c.discontinuityFlags.Reserve( m_numFrames );
	}
	
	// Custom vector3s.
	m_customVector3Curves.Clear();
	m_customVector3Curves.Resize( scene.GetNumCustomVector3s() );
	
	for ( U32 i = 0; i < scene.GetNumCustomVector3s(); ++i )
	{
		CustomVector3Curve& c = m_customVector3Curves[i];
		c.name = scene.GetCustomVector3Name( i );
		c.frames.Reserve( m_numFrames );
		c.discontinuityFlags.Reserve( m_numFrames );
	}

	// Custom vector4s.
	m_customVector4Curves.Clear();
	m_customVector4Curves.Resize( scene.GetNumCustomVector4s() );
	
	for ( U32 i = 0; i < scene.GetNumCustomVector4s(); ++i )
	{
		CustomVector4Curve& c = m_customVector4Curves[i];
		c.name = scene.GetCustomVector4Name( i );
		c.framesXYZ.Reserve( m_numFrames );
		c.framesW.Reserve( m_numFrames );
		c.discontinuityFlags.Reserve( m_numFrames );
	}
}

C_Status C_CycleBuilder::ExtractReferenceTransforms( const C_Scene& scene, const C_CycleParameters& cycleParameters )
{	
	m_numReferenceTransforms = 0;
	if ( scene.GetNumReferenceTransforms() > 0 )
	{
		m_numReferenceTransforms = 1;
		scene.GetCurrentWorldReferenceTransform( 0, m_referenceTransform );
		
		if ( scene.GetNumReferenceTransforms() > 1 )
		{
			return C_Status( false, "Too many reference transforms in scene - only one is supported" );
		}
		
		if ( scene.IsReferenceTransformAnimated( 0 ) )
		{
			return C_Status(
				false,
				Btl::String().Printf("Reference transform '%s' is animated - only static reference markers are supported", scene.GetReferenceTransformName( 0 ) )
			);
		}
	}
	return C_Status( true );
}

C_Status C_CycleBuilder::ExtractConstantData( const C_Scene& scene, const C_CycleParameters& cycleParameters )
{
	C_Status status = ExtractReferenceTransforms( scene, cycleParameters );
	if ( !status ) return status;
	return status;
}

void C_CycleBuilder::SampleFrames(
	C_Scene& cycleScene,
	const C_CycleParameters& cycleParameters,
	Progress::I_ProgressBar* progressBar
)
{
	MEMORY_TRACKER_PUSH( kCycleBuilderRawFramesMemoryTag );
	
	Progress::UpdateSimple prog( progressBar, "Sampling..." );

	const U32 numJoints = cycleScene.GetNumNodes();
	
	// Build list of joints in this cycle.
	BuildListOfCycleJoints( cycleScene, cycleParameters );
	
	// Build list of custom curves.
	BuildListOfCustomCurves( cycleScene, cycleParameters );
	
	//
	// Sample curves at regular intervals dictated by the animation's frame rate and the
	// specified oversampling rate.
	//
	const F32 oversampling = cycleParameters.oversampling;
	const U32 numFrames = m_numFrames;
	const U32 numCycleJoints = m_cycleJoints.Size();
	const U32 numCustomScalarCurves = m_customScalarCurves.Size();
	const U32 numCustomVector3Curves = m_customVector3Curves.Size();
	const U32 numCustomVector4Curves = m_customVector4Curves.Size();
	
	prog.SetProgress( 0.f );
	
	
	for ( U32 frame = 0; frame < numFrames; ++frame )
	{
		if ( 0 == ( frame & 0x3 ) )
		{
			prog.SetProgress( (100.f * frame) / numFrames );
		}
		
		cycleScene.SetCurrentFrame( frame );
		
		// Sample joint transforms and store rotation and translation values.
		
		Animation::C_JointTransform transform;
		
		for ( U32 cycleJointIndex = 0; cycleJointIndex < numCycleJoints; ++cycleJointIndex )
		{
			CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
			
			if ( cycleParameters.inWorldSpace )
			{
				cycleScene.GetCurrentWorldNodeTransform( cycleJoint.m_skeletonJointIndex, transform );
			}
			else
			{
				cycleScene.GetCurrentLocalNodeTransform( cycleJoint.m_skeletonJointIndex, transform );
			}
			
			Maths::StorageVector3 translation;
			Maths::Store( transform.GetTranslation(), &translation );
			
			Maths::StorageVector3 scale;
			Maths::Store( transform.GetScale(), &scale );
			
			if ( cycleJoint.m_hasRotation )
			{
				cycleJoint.m_rotationFrames.PushBack( !Maths::C_Quaternion( transform.GetRotation() ) );
				cycleJoint.m_rotationDiscontinuityFlags.PushBack( cycleScene.NodeHasRotationDiscontinuity( cycleJoint.m_skeletonJointIndex ) );
			}
			if ( cycleJoint.m_hasTranslation )
			{
				cycleJoint.m_translationFrames.PushBack( translation.m_Data );
				cycleJoint.m_translationDiscontinuityFlags.PushBack( cycleScene.NodeHasTranslationDiscontinuity( cycleJoint.m_skeletonJointIndex ) );
			}
			if ( cycleJoint.m_hasScale )
			{
				cycleJoint.m_scaleFrames.PushBack( scale.m_Data );
				cycleJoint.m_scaleDiscontinuityFlags.PushBack( cycleScene.NodeHasScaleDiscontinuity( cycleJoint.m_skeletonJointIndex ) );
			}
		}
		
		// Sample custom curves.
		SampleCustomCurves( cycleScene );
		
		// TODO: sample events?
	}
	
	ForceRotationsToNearestNeighbours();
	
#if 0 // defined(USER_CCOOKSON) && defined(CFG_DEBUG)
	for ( U32 customCurveIndex = 0; customCurveIndex < numCustomCurves; ++customCurveIndex )
	{
		CustomCurve& customCurve = m_customCurves[ customCurveIndex ];
		CopyArrayToClipboard( &customCurve.m_frames[0], customCurve.m_frames.Size() );
		DebugBreak();
	}
#endif
	
	MEMORY_TRACKER_POP( kCycleBuilderRawFramesMemoryTag );
}

void C_CycleBuilder::SampleAdditiveFrames(
	C_Scene& cycleScene1,
	C_Scene& cycleScene2,
	const C_CycleParameters& cycleParameters,
	Progress::I_ProgressBar* progressBar
)
{
	MEMORY_TRACKER_PUSH( kCycleBuilderRawFramesMemoryTag );
	
	BIZ_ASSERT( cycleScene1.GetNumNodes() == cycleScene2.GetNumNodes() );
	
	Progress::UpdateSimple prog( progressBar, "Sampling..." );
	
	// Build list of joints in this cycle.
	BuildListOfCycleJoints( cycleScene1, cycleParameters );
	
	// Build list of custom curves in this cycle.
	BuildListOfCustomCurves( cycleScene1, cycleParameters );
		
	//
	// Sample curves at regular intervals dictated by the animation's frame rate and the
	// specified oversampling rate.
	//
	const U32 numFrames = m_numFrames;
	const U32 numCycleJoints = m_cycleJoints.Size();
	
	const U32 numFrames1 = cycleScene1.GetNumFrames();
	const U32 numFrames2 = cycleScene2.GetNumFrames();
	
	if ( cycleParameters.inWorldSpace )
	{
		BIZ_ASSERT( false );
		LOGERROR( gLog, "World space additive cycles not supported!" );
		return;
	}
	
	prog.SetProgress( 0.f );
	
	for ( U32 frame = 0; frame < numFrames; ++frame )
	{
		if ( 0 == ( frame & 0x3) ) 
		{
			prog.SetProgress( (100.f * frame) / numFrames );
		}
		
		cycleScene1.SetCurrentFrame( frame % numFrames1 );
		cycleScene2.SetCurrentFrame( frame % numFrames2 );

		// Sample transforms and calculate "subtraction".
		for ( U32 cycleJointIndex = 0; cycleJointIndex < numCycleJoints; ++cycleJointIndex )
		{
			CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
		
			Animation::C_JointTransform localJointTransform1;
			cycleScene1.GetCurrentLocalNodeTransform( cycleJoint.m_skeletonJointIndex, localJointTransform1 );
			Animation::C_JointTransform localJointTransform2;
			cycleScene2.GetCurrentLocalNodeTransform( cycleJoint.m_skeletonJointIndex, localJointTransform2 );
			
			// Perform the "subtraction".
			Animation::C_JointTransform localDeltaTransform(
				localJointTransform2.GetRotation() * !localJointTransform1.GetRotation(),
				localJointTransform1.GetTranslation() - localJointTransform2.GetTranslation(),
				localJointTransform1.GetScale() * Maths::Rcp(localJointTransform2.GetScale())
			);
						
			Maths::StorageVector3 translationDelta;
			Maths::Store( localDeltaTransform.GetTranslation(), &translationDelta );

			Maths::StorageVector3 scaleDelta;
			Maths::Store( localDeltaTransform.GetScale(), &scaleDelta );
			
			if ( cycleJoint.m_hasRotation )
			{
				cycleJoint.m_rotationFrames.PushBack( localDeltaTransform.GetRotation() );
				cycleJoint.m_rotationDiscontinuityFlags.PushBack( false );
			}
			if ( cycleJoint.m_hasTranslation )
			{
				cycleJoint.m_translationFrames.PushBack( translationDelta.m_Data );
				cycleJoint.m_translationDiscontinuityFlags.PushBack( false );
			}
			if ( cycleJoint.m_hasScale )
			{
				cycleJoint.m_scaleFrames.PushBack( scaleDelta.m_Data );
				cycleJoint.m_scaleDiscontinuityFlags.PushBack( false );
			}
		}
		
		SampleCustomCurves( cycleScene1 );
	}
	
	ForceRotationsToNearestNeighbours();
	
	MEMORY_TRACKER_POP( kCycleBuilderRawFramesMemoryTag );
}

void C_CycleBuilder::SampleCustomCurves( const C_Scene& scene )
{
	// Sample custom scalar curves.
	U32 numCustomScalarCurves = scene.GetNumCustomScalars();
	for ( U32 i = 0; i < numCustomScalarCurves; ++i )
	{
		m_customScalarCurves[i].frames.PushBack( scene.GetCurrentCustomScalarValue( i ) );
		m_customScalarCurves[i].discontinuityFlags.PushBack( scene.DoesCustomScalarHaveDiscontinuity( i ) );
	}

	// Sample custom vector3 curves.
	U32 numCustomVector3Curves = scene.GetNumCustomVector3s();
	for ( U32 i = 0; i < numCustomVector3Curves; ++i )
	{
		MathsStorage::Float3 value;
		scene.GetCurrentCustomVector3Value( i, value.v );
		m_customVector3Curves[i].frames.PushBack( value );
		m_customVector3Curves[i].discontinuityFlags.PushBack( scene.DoesCustomVector3HaveDiscontinuity( i ) );
	}

	// Sample custom vector4 curves.
	U32 numCustomVector4Curves = scene.GetNumCustomVector4s();
	for ( U32 i = 0; i < numCustomVector4Curves; ++i )
	{
		MathsStorage::Float4 value;
		scene.GetCurrentCustomVector4Value( i, value.v );
		m_customVector4Curves[i].framesXYZ.PushBack( MathsStorage::Float3( value.x, value.y, value.z ) );
		m_customVector4Curves[i].framesW.PushBack( value.w );
		m_customVector4Curves[i].discontinuityFlags.PushBack( scene.DoesCustomVector4HaveDiscontinuity( i ) );
	}
}

void C_CycleBuilder::ForceRotationsToNearestNeighbours()
{
	const U32 numCycleJoints = m_cycleJoints.Size();
	
	for ( U32 cycleJointIndex = 0; cycleJointIndex < numCycleJoints; ++cycleJointIndex )
	{
		CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
		
		U32 numFrames = cycleJoint.m_rotationFrames.Size();
		
		if ( numFrames < 1 )
		{
			continue;
		}
		
		const Maths::C_Quaternion firstQuat = cycleJoint.m_rotationFrames[ 0 ];
		
		Maths::C_Quaternion prevQuat = firstQuat;
		const Maths::HF32 zero = Maths::Constants< Maths::HF32 >::Zero();
		
		for ( U32 i = 1; i < numFrames; ++i )
		{
			const Maths::C_Quaternion currQuat = cycleJoint.m_rotationFrames[ i ];
			const Maths::HF32 dot = currQuat | prevQuat;
			const Maths::C_Quaternion nearestQuat = Maths::Select( dot.LT( zero ), -currQuat, currQuat );
			cycleJoint.m_rotationFrames[ i ] = nearestQuat;
			prevQuat = nearestQuat;
			
			if ( ( nearestQuat | firstQuat ) < zero )
			{
				// Rotation greater than 180 degrees...
				volatile bool greaterThan180 = true;
			}
			
		}
	}
	
}


void C_CycleBuilder::CalculateTrajectoryDelta(const C_CycleParameters& cycleParameters)
{
	m_trajectoryRootFieldIndex = -1;
	
	// Find cycle joint (if any) that animates the trajectory root bone.
	S32 trajectoryRootSkeletonJointIndex = 0;
	S32 trajectoryRootCycleJointIndex = -1;
	
	// Firstly, find the CycleJoint corresponding to the trajectory root skeleton joint index.
	for ( S32 i = 0; i < m_cycleJoints.Size(); ++i )
	{
		if ( m_cycleJoints[ i ].m_skeletonJointIndex == trajectoryRootSkeletonJointIndex )
		{
			trajectoryRootCycleJointIndex = i;
			break;
		}
	}
	
	// Next, find the index of the corresponding field in the cycle, which may not exist
	// if the trajectory root has no animation for whatever reason.
	if (trajectoryRootCycleJointIndex >= 0)
	{
		const Animation::E_PoseFieldType transformFieldType = Animation::E_PoseFieldType_JointTransform;
		for (U32 i = 0; i < m_totalFields; ++i)
		{
			if (!stricmp(m_fieldNames[i].Chars(), m_cycleJoints[trajectoryRootCycleJointIndex].m_name.Chars()) &&
				m_fieldTypes[i] == transformFieldType)
			{
				m_trajectoryRootFieldIndex = i;
				break;
			}
		}
	}
	
	if ( m_trajectoryRootFieldIndex >= 0 )
	{
		const S32 skeletonJointIndex = m_trajectoryRootFieldIndex;
	
		m_trajectoryStart = GetFirstLocalJointTransform( trajectoryRootSkeletonJointIndex, E_Source_Compressed );
		m_trajectoryFinish = GetLastLocalJointTransform( trajectoryRootSkeletonJointIndex, E_Source_Compressed );
		
		// Remove scale.
		m_trajectoryStart.GetScaleField().SetOne();
		m_trajectoryFinish.GetScaleField().SetOne();
		
		Animation::C_JointTransform fullDelta = m_trajectoryStart.Inverse() * m_trajectoryFinish;
		
		fullDelta.Normalise();
		
		Maths::StorageVec4F32 q;
		Maths::StorageVec4F32 t;
		
		q = Maths::GetVec4F32(m_trajectoryStart.GetRotation());
		t = Maths::GetVec4F32(m_trajectoryStart.GetTranslation());

		LOGINFO( gLog, "Trajectory start: R (%.3f, %.3f, %.3f, %.3f) T (%.3f, %.3f, %.3f)\n",
			q.m_Data.x, q.m_Data.y, q.m_Data.z, q.m_Data.w,
			t.m_Data.x, t.m_Data.y, t.m_Data.z
		);

		q = Maths::GetVec4F32(m_trajectoryFinish.GetRotation());
		t = Maths::GetVec4F32(m_trajectoryFinish.GetTranslation());

		LOGINFO( gLog, "Trajectory finish: R (%.3f, %.3f, %.3f, %.3f) T (%.3f, %.3f, %.3f)\n",
			q.m_Data.x, q.m_Data.y, q.m_Data.z, q.m_Data.w,
			t.m_Data.x, t.m_Data.y, t.m_Data.z
		);

		q = Maths::GetVec4F32( fullDelta.GetRotation() );
		t = Maths::MakeVec4F32( fullDelta.GetTranslation() );
				
		Util::C_String str;
		LOGINFO( gLog, "Trajectory delta: R (%.3f, %.3f, %.3f, %.3f) T (%.3f, %.3f, %.3f)\n",
			q.m_Data.x, q.m_Data.y, q.m_Data.z, q.m_Data.w,
			t.m_Data.x, t.m_Data.y, t.m_Data.z
		);

				
		m_trajectoryDelta = fullDelta;
	}
	else
	{
		//LOGINFO( gLog, "No trajectory joint animation in cycle\n" );
		m_trajectoryDelta.SetIdentity();
		m_trajectoryStart.SetIdentity();
		m_trajectoryFinish.SetIdentity();
	}
}

void C_CycleBuilder::BuildRawCurves()
{
	MEMORY_TRACKER_PUSH( kCycleBuilderRawCurvesMemoryTag );
	
	KeyReductionUtils::QuaternionVector rotationKeys;
	KeyReductionUtils::Float3Vector translationKeys;
	KeyReductionUtils::KeyIndexVector keyTimes;
	
	rotationKeys.Reserve( m_numFrames );
	translationKeys.Reserve( m_numFrames );
	
	keyTimes.Reserve( m_numFrames );
	
	for ( U32 i = 0; i < m_numFrames; ++i )
	{
		keyTimes.PushBack( i );
	}
	
	//
	// Build raw joint transform curves.
	//
	const U32 numCycleJoints = m_cycleJoints.Size();
	for ( U32 cycleJointIndex = 0; cycleJointIndex < numCycleJoints; ++cycleJointIndex )
	{
		CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
		
		const U32 skeletonJointIndex = cycleJoint.m_skeletonJointIndex;
		
		if ( cycleJoint.m_hasRotation )
		{
			cycleJoint.m_rawRotationCurve.CreateQuaternionCurve(
				E_KeyType_QuaternionF32,
				m_numFrames,
				&keyTimes[ 0 ],
				&cycleJoint.m_rotationFrames[ 0 ]
			);
		}
			
		if ( cycleJoint.m_hasTranslation )
		{
			cycleJoint.m_rawTranslationCurve.CreateVector3Curve(
				E_KeyType_Vector3F32,
				m_numFrames,
				&keyTimes[ 0 ],
				&cycleJoint.m_translationFrames[ 0 ],
				0,
				0
			);						
		}
		
		if ( cycleJoint.m_hasScale )
		{
			cycleJoint.m_rawScaleCurve.CreateVector3Curve(
				E_KeyType_Vector3F32,
				m_numFrames,
				&keyTimes[0],
				&cycleJoint.m_scaleFrames[0],
				0,
				0
			);
		}
	}

	//
	// Build raw custom curves.
	//

	for ( U32 i = 0, count = m_customScalarCurves.Size(); i < count; ++i )
	{
		CustomScalarCurve& c = m_customScalarCurves[i];
		c.rawCurve.CreateScalarCurve(
			E_KeyType_ScalarF32,
			m_numFrames,
			&keyTimes[0],
			&c.frames[0]
		);
	}

	for ( U32 i = 0, count = m_customVector3Curves.Size(); i < count; ++i )
	{
		CustomVector3Curve& c = m_customVector3Curves[i];
		c.rawCurve.CreateVector3Curve(
			E_KeyType_Vector3F32,
			m_numFrames,
			&keyTimes[0],
			&c.frames[0]
		);
	}

	for ( U32 i = 0, count = m_customVector4Curves.Size(); i < count; ++i )
	{
		CustomVector4Curve& c = m_customVector4Curves[i];
		c.rawCurveXYZ.CreateVector3Curve(
			E_KeyType_Vector3F32,
			m_numFrames,
			&keyTimes[0],
			&c.framesXYZ[0]
		);
		c.rawCurveW.CreateScalarCurve(
			E_KeyType_ScalarF32,
			m_numFrames,
			&keyTimes[0],
			&c.framesW[0]
		);
	}
	
	MEMORY_TRACKER_POP( kCycleBuilderRawCurvesMemoryTag );
}


const CHAR8* ToString( Animation::E_VaryingCurveType curveType )
{
	switch ( curveType )
	{
		case Animation::E_VaryingCurveType_VaryingVector3F32: return "Vector3F32"; break;
		case Animation::E_VaryingCurveType_VaryingQuaternion32: return "Quaternion32"; break;
		case Animation::E_VaryingCurveType_VaryingScalarF32: return "ScalarF32"; break;
		case Animation::E_VaryingCurveType_VaryingQuaternion48: return "Quaternion48"; break;
		case Animation::E_VaryingCurveType_VaryingVector3U16: return "Vector3U16"; break;
		case Animation::E_VaryingCurveType_VaryingVector3U8: return "Vector3U8"; break;
	}
	return "";
}

const CHAR8* ToString( Animation::E_ConstantCurveType curveType )
{
	switch ( curveType )
	{
		case Animation::E_ConstantCurveType_ConstantVector3F32: return "ConstantVector3F32"; break;
		case Animation::E_ConstantCurveType_ConstantScalarF32: return "ConstantScalarF32"; break;
		case Animation::E_ConstantCurveType_ConstantQuaternion48: return "ConstantQuaternion48"; break;
		default: break;
	}
	
	return "Invalid";
}

C_Status C_CycleBuilder::BuildCompressedCurves(
	const C_CycleParameters& cycleParameters,
	const C_Scene& scene,
	Progress::I_ProgressBar* progressBar
)
{
	MEMORY_TRACKER_PUSH( kCycleBuilderCompressedCurvesMemoryTag );
	
	Progress::UpdateSimple prog( progressBar, "Compressing..." );

	KeyReductionUtils::QuaternionVector rotationKeys;
	KeyReductionUtils::Float3Vector vector3Keys;
	KeyReductionUtils::KeyIndexVector keyTimes;
	
	rotationKeys.Reserve( m_numFrames );
	vector3Keys.Reserve( m_numFrames );
	keyTimes.Reserve( m_numFrames );
	
	const U32 numCycleJoints = m_cycleJoints.Size();
	
	// Calculate joint error thresholds.
	C_CompressionParameters compressionParams( scene.GetNumNodes() );
	EstimateJointErrorThresholds( cycleParameters, scene, compressionParams );
	
	// Set up the appropriate transform type according to whether or not scale information is required.
	const Animation::E_PoseFieldType transformFieldType = Animation::E_PoseFieldType_JointTransform;
	const Animation::E_PoseFieldElement transformRotationElement = Animation::E_PoseFieldElement_JointTransformRotation;
	const Animation::E_PoseFieldElement transformTranslationElement = Animation::E_PoseFieldElement_JointTransformTranslation;
	const Animation::E_PoseFieldElement transformScaleElement = Animation::E_PoseFieldElement_JointTransformScale;
	
	//
	// Compress joint transform curves.
	//
	for ( U32 cycleJointIndex = 0; cycleJointIndex < numCycleJoints; ++cycleJointIndex )
	{
		prog.SetProgress( (80.f * cycleJointIndex) / numCycleJoints );
	
		CycleJoint& cycleJoint = m_cycleJoints[ cycleJointIndex ];
		
		const Btl::String fieldName = m_cycleJoints[ cycleJointIndex ].m_name.Chars();
		
		const U32 skeletonJointIndex = cycleJoint.m_skeletonJointIndex;
		
		Animation::C_JointTransform basePoseTransform;
		scene.GetLocalNodeBindPoseTransform( skeletonJointIndex, basePoseTransform );
		
		//
		// Build compressed joint rotation curve.
		//

		if ( cycleJoint.m_hasRotation )
		{
			const F32 maxJointRotationError = compressionParams.GetJointMaxRotationError( skeletonJointIndex );

			KeyReductionUtils::GetKeyReducedQuaternionCurve(
				&cycleJoint.m_rotationFrames[ 0 ],
				cycleJoint.m_rotationDiscontinuityFlags,
				m_numFrames,
				maxJointRotationError,
				rotationKeys,
				keyTimes
			);

			//DBGLOG( "Key reduced curve '%s' from %d keys to %d keys", fieldName.Chars(), cycleJoint.m_rotationFrames.Size(), rotationKeys.Size() );			

			BIZ_ASSERT( rotationKeys.Size() == keyTimes.Size() );
			
			const bool isConstantRotation = ( rotationKeys.Size() == 1 );
			
			bool buildRotationCurve = true;
			bool addRotationCurve = true;
			
			if ( IsAdditive() )
			{
				// Additive animation: optimise storage of identity rotations.
				if ( isConstantRotation &&
					CompressedCurveUtils::RotationsEqualWithinTolerance( rotationKeys[ 0 ], Maths::C_Quaternion(), maxJointRotationError )
				)
				{
					cycleJoint.m_rotationCurve.CreateIdentityRotationCurve();
					buildRotationCurve = false;
					
					// No point storing identity curves in additive animations since they are the default.
					addRotationCurve = false;
				}
			}
			
			if ( buildRotationCurve )
			{
				bool ok = CompressedCurveUtils::BuildCompressedQuaternionCurve(
					&rotationKeys[ 0 ],
					&keyTimes[ 0 ],
					rotationKeys.Size(),
					m_numFrames,
					maxJointRotationError,
					cycleJoint.m_rotationCurve
				);
				
				if ( !ok )
				{
					return C_Status( false, Btl::String().Printf("Error building compressed quaternion curve '%s'", fieldName.Chars() ) );
				}
			}
			
			if ( addRotationCurve )
			{
				C_CycleCurve* rotationCycleCurve = new C_CycleCurve(
					transformFieldType,
					m_cycleJoints[ cycleJointIndex ].m_name,
					transformRotationElement,
					&cycleJoint.m_rotationCurve,
					&cycleJoint.m_rawRotationCurve
				);
				
				AddCycleCurve( rotationCycleCurve );
			}
		}
				
		//
		// Build compressed joint translation curve.
		//
		if ( cycleJoint.m_hasTranslation )
		{
			const F32 maxJointTranslationError = compressionParams.GetJointMaxTranslationError( skeletonJointIndex );
				
			
			KeyReductionUtils::GetKeyReducedVector3Curve(
				&cycleJoint.m_translationFrames[ 0 ],
				cycleJoint.m_translationDiscontinuityFlags,
				m_numFrames,
				maxJointTranslationError,
				vector3Keys,
				keyTimes
			);
			//DBGLOG( "Key reduced translation curve for '%s' from %d to %d keys", m_cycleJoints[ cycleJointIndex ].m_name.Chars(), cycleJoint.m_translationFrames.Size(), vector3Keys.Size() );
			
			BIZ_ASSERT( vector3Keys.Size() == keyTimes.Size() );
			
			const bool isConstantTranslation = ( vector3Keys.Size() == 1 );
			
			Maths::StorageVector3 basePoseTranslation;
			Maths::Store( basePoseTransform.GetTranslation(), &basePoseTranslation );
			
			bool buildTranslationCurve = true;
			bool addTranslationCurve = true;
			
			if ( IsAdditive() )
			{
				// Additive animation: optimise storage of zero translations.
				static const F32 zeroVector3[ 3 ] = { 0.f, 0.f, 0.f };
				
				if ( isConstantTranslation &&
					CompressedCurveUtils::VectorsEqualWithinTolerance(
						&vector3Keys[ 0 ].x,
						zeroVector3,
						3,
						maxJointTranslationError )
				)
				{
					cycleJoint.m_translationCurve.CreateZeroTranslationCurve();
					buildTranslationCurve = false;
					addTranslationCurve = false;
				}
			}
			else
			{
				// Non-additive animation: optionally remove non-root (and those directly under the root) constant translation channels that match base pose.
				if ( cycleParameters.removeTranslationChannelsMatchingBasePose
					&& scene.GetNodeDepth(cycleJoint.m_skeletonJointIndex) > 1
					&& isConstantTranslation
					&& CompressedCurveUtils::VectorsEqualWithinTolerance(
						&vector3Keys[ 0 ].x,
						basePoseTranslation.m_Data.v,
						3,
						maxJointTranslationError )
					)
				{
					// Helper nodes must keep their animated translation channels since they don't have a meaningful base pose
					// and their position is important (e.g. gun/ammo bones or head point bones).
					if (!scene.IsHelperNode(cycleJoint.m_skeletonJointIndex))
					{
						buildTranslationCurve = false;
						addTranslationCurve = false;
					}
				}
			}
			
			if ( buildTranslationCurve )
			{
				bool ok = CompressedCurveUtils::BuildCompressedVector3Curve(
					&vector3Keys[ 0 ],
					&keyTimes[ 0 ],
					vector3Keys.Size(),
					m_numFrames,
					maxJointTranslationError,
					false,	// Use absolute error for joints.
					cycleJoint.m_rawTranslationCurve.GetOffset(),
					cycleJoint.m_rawTranslationCurve.GetScale(),
					cycleJoint.m_translationCurve
				);
				
				if ( !ok )
				{
					return C_Status( false, Btl::String().Printf("Error building compressed vector3 curve '%s'", fieldName.Chars() ) );
				}	
			}
			
			if ( addTranslationCurve )
			{
			
				C_CycleCurve* translationCycleCurve = new C_CycleCurve(
					transformFieldType,
					m_cycleJoints[ cycleJointIndex ].m_name,
					transformTranslationElement,
					&cycleJoint.m_translationCurve,
					&cycleJoint.m_rawTranslationCurve
				);
				
				AddCycleCurve( translationCycleCurve );
			}
			
		}
		
		//
		// Build compressed joint scale curve.
		//
		
		if ( cycleJoint.m_hasScale )
		{
			//DBGLOG( "Building key reduced scale curve for '%s' with %d keys", m_cycleJoints[ cycleJointIndex ].m_name.Chars(), vector3Keys.Size() );
			
			F32 maxScaleError = compressionParams.GetJointMaxScaleError( skeletonJointIndex );
			
			KeyReductionUtils::GetKeyReducedVector3Curve(
				&cycleJoint.m_scaleFrames[ 0 ],
				cycleJoint.m_scaleDiscontinuityFlags,
				m_numFrames,
				maxScaleError,
				vector3Keys,
				keyTimes
			);
			BIZ_ASSERT( vector3Keys.Size() == keyTimes.Size() );
			
			const bool isConstantScale = ( vector3Keys.Size() == 1 );
			
			Maths::StorageVector3 basePoseScale;
			Maths::Store( basePoseTransform.GetScale(), &basePoseScale );
			
			bool buildScaleCurve = true;
			bool addScaleCurve = true;
			
			if ( IsAdditive() || cycleParameters.removeIdentityScaleChannels )
			{
				// Remove channels where scale is identity (1,1,1)
				static const F32 oneVector3[ 3 ] = { 1.f, 1.f, 1.f };
				
				if ( isConstantScale &&
					CompressedCurveUtils::VectorsEqualWithinTolerance(
						&vector3Keys[ 0 ].x,
						oneVector3,
						3,
						maxScaleError )
				)
				{
					//cycleJoint.m_scaleCurve.CreateZeroTranslationCurve();
					buildScaleCurve = false;
					addScaleCurve = false;
				}
			}
			else
			{
				// Non-additive animation: optionally remove non-root (and those directly under the root) constant scale channels that match base pose.
				if ( cycleParameters.removeScaleChannelsMatchingBasePose
					&& scene.GetNodeDepth(cycleJoint.m_skeletonJointIndex) > 1
					&& isConstantScale
					&& CompressedCurveUtils::VectorsEqualWithinTolerance(
						&vector3Keys[ 0 ].x,
						basePoseScale.m_Data.v,
						3,
						maxScaleError )
					)
				{
					buildScaleCurve = false;
					addScaleCurve = false;
				}
			}
			

			if ( buildScaleCurve )
			{
				bool ok = CompressedCurveUtils::BuildCompressedVector3Curve(
					&vector3Keys[ 0 ],
					&keyTimes[ 0 ],
					vector3Keys.Size(),
					m_numFrames,
					maxScaleError,
					false,
					cycleJoint.m_rawScaleCurve.GetOffset(),
					cycleJoint.m_rawScaleCurve.GetScale(),
					cycleJoint.m_scaleCurve
				);
				
				if ( !ok )
				{
					return C_Status( false, Btl::String().Printf("Error building compressed vector3 curve '%s'", fieldName.Chars() ) );
				}	
			}
			
			if ( addScaleCurve )
			{
			
				C_CycleCurve* scaleCycleCurve = new C_CycleCurve(
					transformFieldType,
					m_cycleJoints[ cycleJointIndex ].m_name,
					transformScaleElement,
					&cycleJoint.m_scaleCurve,
					&cycleJoint.m_rawScaleCurve
				);
				
				AddCycleCurve( scaleCycleCurve );
			}
			
		}
	}
		
	//
	// Compress custom curves.
	//
	
	BuildCompressedCustomScalarCurves( cycleParameters, scene, progressBar );
	BuildCompressedCustomVector3Curves( cycleParameters, scene, progressBar );
	BuildCompressedCustomVector4Curves( cycleParameters, scene, progressBar );
	
	//
	// Build segments
	//
	
	C_Status status = BuildVaryingCurveSegments();
	
	MEMORY_TRACKER_POP( kCycleBuilderCompressedCurvesMemoryTag );
	
	if ( !status )
	{
		return status;
	}
	
#if 0
	
	LOGDEBUG( gLog, "Cycle Fields:" );
	
	static const char* fieldTypeNames[ Animation::E_PoseFieldType_Count ] =
	{
		"JointTransform",
		"TrajectoryDeltaTransform",
		"CustomVector3",
		"CustomScalar",
		"TrajectoryTransform",
		"CustomVector4"
	};
	
	for ( U32 i = 0; i < m_totalFields; ++i )
	{
		LOGINFO( gLog, "[%3d] %s %s\n", i, fieldTypeNames[ m_fieldTypes[ i ] ], m_fieldNames[ i ].Chars() );
	}
#endif

#if 1
	// Summary information about curve types.
	LOGDEBUG( gLog, "Constant curves:\n" );
	for ( S32 i = 0; i < m_constantCycleCurvesByType.Size(); ++i )
	{
		LOGINFO( gLog, "[%3d] %s %d\n", i, ToString(static_cast<Animation::E_ConstantCurveType>(i)), m_constantCycleCurvesByType[i].Size() );
	}


	LOGDEBUG( gLog, "Varying curves:\n" );
	for ( S32 i = 0; i < m_varyingCycleCurvesByType.Size(); ++i )
	{
		LOGINFO( gLog, "[%3d] %s %d\n", i, ToString(static_cast<Animation::E_VaryingCurveType>(i)), m_varyingCycleCurvesByType[i].Size() );
	}
#endif

#if 1
	// Information about each individual curve.
	LOGDEBUG( gLog, "Curves:\n" );
	for (S32 i = 0; i < m_cycleCurves.Size(); ++i)
	{
		// <name> <element type> <curve type>
		const C_CycleCurve* c = m_cycleCurves[i];
		// 'hips'; Rotation; VarQuat48
		// "hips" Rotation, VarQuat48
		LOGDEBUG(gLog, "  [%2d] '%s' %s %s\n", i, c->m_fieldName.Chars(), GetElementName(c->m_fieldElement), AnimationTools::ToString(c->m_curve->GetType()));
	}

#endif
	
	return C_Status( true );
}

C_Status C_CycleBuilder::BuildCompressedCustomScalarCurves(
	const C_CycleParameters& cycleParameters,
	const C_Scene& scene,
	Progress::I_ProgressBar* progressBar
)
{
	KeyReductionUtils::FloatVector scalarKeys;
	KeyReductionUtils::KeyIndexVector keyTimes;
	
	for ( U32 i = 0, count = m_customScalarCurves.Size(); i < count; ++i )
	{
		CustomScalarCurve& c = m_customScalarCurves[i];
		
		Btl::String fieldName = c.name.Chars();
		
		const F32 maxFractionalError = cycleParameters.maxRelativeCustomCurveError;
		const F32 maxError = maxFractionalError * Maths::Abs(c.rawCurve.GetScale()[0]);

		KeyReductionUtils::GetKeyReducedScalarCurve(
			&c.frames[ 0 ],
			c.discontinuityFlags,
			m_numFrames,
			maxError,
			scalarKeys,
			keyTimes
		);
		
		BIZ_ASSERT( scalarKeys.Size() == keyTimes.Size() );
		
		bool ok = CompressedCurveUtils::BuildCompressedScalarCurve(
			&scalarKeys[ 0 ],
			&keyTimes[ 0 ],
			scalarKeys.Size(),
			m_numFrames,
			maxFractionalError,
			true,
			*c.rawCurve.GetOffset(),
			*c.rawCurve.GetScale(),
			c.curve
		);
		
		if ( !ok )
		{
			return C_Status( false, Btl::String().Printf("Error building compressed scalar curve '%s'", fieldName.Chars() ) );
		}
		
		const E_CurveType curveType = c.curve.GetType();
		
		C_CycleCurve* customScalarCurve = new C_CycleCurve(
			Animation::E_PoseFieldType_CustomScalar,
			c.name,
			Animation::E_PoseFieldElement_CustomScalar,
			&c.curve,
			&c.rawCurve
			);
			
		AddCycleCurve( customScalarCurve );
	}
	
	return C_Status( true );
}

C_Status C_CycleBuilder::BuildCompressedCustomVector3Curves(
	const C_CycleParameters& cycleParameters,
	const C_Scene& scene,
	Progress::I_ProgressBar* progressBar
)
{
	KeyReductionUtils::Float3Vector vector3Keys;
	KeyReductionUtils::KeyIndexVector keyTimes;
	
	for ( U32 i = 0, count = m_customVector3Curves.Size(); i < count; ++i )
	{
		CustomVector3Curve& c = m_customVector3Curves[i];
		
		Btl::String fieldName = c.name.Chars();
		
		const F32 maxError = cycleParameters.maxRelativeCustomCurveError;

		// FIXME: error is absolute
		KeyReductionUtils::GetKeyReducedVector3Curve(
			&c.frames[ 0 ],
			c.discontinuityFlags,
			m_numFrames,
			maxError,
			vector3Keys,
			keyTimes
		);
		
		BIZ_ASSERT( vector3Keys.Size() == keyTimes.Size() );
		
		bool ok = CompressedCurveUtils::BuildCompressedVector3Curve(
			&vector3Keys[ 0 ],
			&keyTimes[ 0 ],
			vector3Keys.Size(),
			m_numFrames,
			maxError,
			true, // Use relative error for custom curves.
			c.rawCurve.GetOffset(),
			c.rawCurve.GetScale(),
			c.curve
		);
		
		if ( !ok )
		{
			return C_Status( false, Btl::String().Printf("Error building compressed vector3 curve '%s'", fieldName.Chars() ) );
		}
		
		const E_CurveType curveType = c.curve.GetType();
		
		C_CycleCurve* customVector3Curve = new C_CycleCurve(
			Animation::E_PoseFieldType_CustomVector3,
			c.name,
			Animation::E_PoseFieldElement_CustomVector3,
			&c.curve,
			&c.rawCurve
			);
			
		AddCycleCurve( customVector3Curve );
	}
	
	return C_Status( true );
}

C_Status C_CycleBuilder::BuildCompressedCustomVector4Curves(
	const C_CycleParameters& cycleParameters,
	const C_Scene& scene,
	Progress::I_ProgressBar* progressBar
)
{
	KeyReductionUtils::Float3Vector		vector3Keys;
	KeyReductionUtils::KeyIndexVector	vector3KeyTimes;
	KeyReductionUtils::FloatVector		scalarKeys;
	KeyReductionUtils::KeyIndexVector	scalarKeyTimes;
	
	for ( U32 i = 0, count = m_customVector4Curves.Size(); i < count; ++i )
	{
		CustomVector4Curve& c = m_customVector4Curves[i];
		
		Btl::String fieldName = c.name.Chars();
		
		//
		// Key reduce.
		//
				
		const F32 maxError = cycleParameters.maxRelativeCustomCurveError;

		// FIXME: error is absolute
		KeyReductionUtils::GetKeyReducedVector3Curve(
			&c.framesXYZ[ 0 ],
			c.discontinuityFlags,
			m_numFrames,
			maxError,
			vector3Keys,
			vector3KeyTimes
		);

		KeyReductionUtils::GetKeyReducedScalarCurve(
			&c.framesW[ 0 ],
			c.discontinuityFlags,
			m_numFrames,
			maxError,
			scalarKeys,
			scalarKeyTimes
		);
		
		//
		// Quantise, if possible.
		//
		
		bool ok1 = CompressedCurveUtils::BuildCompressedVector3Curve(
			&vector3Keys[ 0 ],
			&vector3KeyTimes[ 0 ],
			vector3Keys.Size(),
			m_numFrames,
			maxError,
			true, // Use relative error for custom curves.
			c.rawCurveXYZ.GetOffset(),
			c.rawCurveXYZ.GetScale(),
			c.curveXYZ
		);
		
		if ( !ok1 )
		{
			return C_Status( false, Btl::String().Printf("Error building compressed vector4 XYZ curve '%s'", fieldName.Chars() ) );
		}		

		bool ok2 = CompressedCurveUtils::BuildCompressedScalarCurve(
			&scalarKeys[ 0 ],
			&scalarKeyTimes[ 0 ],
			scalarKeys.Size(),
			m_numFrames,
			maxError,
			true,
			*c.rawCurveW.GetOffset(),
			*c.rawCurveW.GetScale(),
			c.curveW
		);
		
		if ( !ok2 )
		{
			return C_Status( false, Btl::String().Printf("Error building compressed vector4 W curve '%s'", fieldName.Chars() ) );
		}
		
		//
		// Add curves to cycle.
		//
		
		const E_CurveType curveTypeXYZ = c.curveXYZ.GetType();
		const E_CurveType curveTypeW   = c.curveW.GetType();

		C_CycleCurve* xyzCurve = new C_CycleCurve(
			Animation::E_PoseFieldType_CustomVector4,
			c.name,
			Animation::E_PoseFieldElement_CustomVector4XYZ,
			&c.curveXYZ,
			&c.rawCurveXYZ
			);
		
		C_CycleCurve* wCurve = new C_CycleCurve(
			Animation::E_PoseFieldType_CustomVector4,
			c.name,
			Animation::E_PoseFieldElement_CustomVector4W,
			&c.curveW,
			&c.rawCurveW
			);
			
		AddCycleCurve( xyzCurve );
		AddCycleCurve( wCurve );
	}
	
	return C_Status( true );
}



// A collection of varying curves for a time segment.

U32 GetVaryingCurveKeySize( U32 varyingCurveType )
{
	static const U32 kVaryingCurveKeySizes[ Animation::E_VaryingCurveType_Count ] =
	{
		12,
		4,
		4,
		6,
		6,
		3
	};
	return kVaryingCurveKeySizes[ varyingCurveType ];
}

U32 GetVaryingCurveKeyAlignment( U32 varyingCurveType )
{
	static const U32 kVaryingCurveKeyAlignments[ Animation::E_VaryingCurveType_Count ] =
	{
		4,
		4,
		4,
		2,
		2,
		1
	};
	return kVaryingCurveKeyAlignments[ varyingCurveType ];
}



template < typename T >
T AlignUp( T ptr, U32 align )
{
	U32 offset = reinterpret_cast< U32 >( ptr );
	offset = ( ( offset + align - 1 ) & ~( align - 1 ) );
	return reinterpret_cast< T >( offset );
}

template < typename T >
void Relocate( T& ptr, void* newBasePtr, void* prevBasePtr )
{
	ptr = (T) ( (U8*) newBasePtr + ( (U8*) ptr - (U8*) prevBasePtr ) );
}


class C_CycleSegmentLayout
{
public:

	U32 Layout(
		U32 numSegmentFrames,
		const U32 numVaryingCurves[ Animation::E_VaryingCurveType_Count ],
		const U32 numVaryingCurveKeys[ Animation::E_VaryingCurveType_Count ]
	)
	{
		Animation::C_MemoryBlockSizer sizer;
	
		// Array of initial curve keys.
		for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
		{
			sizer.Allocate(
				GetVaryingCurveKeySize( i ) * numVaryingCurves[ i ],
				GetVaryingCurveKeyAlignment( i )
			);
		}
		
		U32 sizeOfCurveKeys = sizer.GetSize();
		
		// No. keys for each curve type.
		sizer.Align( 2 );
		U32 totalCurves = 0;
		
		for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
		{
			sizer.Allocate<U16>( 1 );
			totalCurves += numVaryingCurves[ i ];
		}		

		// Key time arrays for each curve.
		for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
		{
			sizer.Allocate<U8>( numVaryingCurves[ i ] + numVaryingCurveKeys[ i ] );
		}

		// Keys.
		for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
		{
			sizer.Allocate(
				numVaryingCurveKeys[ i ] * GetVaryingCurveKeySize( i ),
				GetVaryingCurveKeyAlignment( i )
			);
		}
		
		// Store size with padding for 16-byte alignment.
		sizer.Align( 16 );
		m_sizeWithoutFinalKeys = sizer.GetSize();
		
		// Final key data.
		sizer.Allocate( sizeOfCurveKeys, 1 );
		m_sizeWithFinalKeys = sizer.GetSize();
		
		return m_sizeWithFinalKeys;
	}

	U32 m_sizeWithoutFinalKeys;
	U32 m_sizeWithFinalKeys;
};

C_Status C_CycleBuilder::BuildVaryingCurveSegments()
{
	// Starting from the first frame, keep adding keys until block size is exceeded.
	U32 startFrame = 0;
	U32 segmentIndex = 0;
	m_cycleSegments.Clear();
	// BlockSize, PS3, 360, filesize
	// 2048 - 5800us 14000us 23k
	// 3K   - 5900us 20.5k
	// 4096 - 6000us 14000us 19.9K
	// 6K -   6100us 19.1K
	const U32 kBlockSizeLimit= 6 * 1024;	// Ideal limit on block size.
	
	U32 totalBlockSize = 0;
	U32 largestSegmentSize = 0;
	U32 largestSegmentIndex = 0;
		
	Btl::Vector< Btl::Vector< U32 >, Btl::VectorParameters_Array< Animation::E_VaryingCurveType_Count > > currCurveKeyIndices;

	Btl::Vector< U32 > numVaryingCurveKeysInSegment; 
	Btl::Vector< U32 > numVaryingCurvesInSegment; 

	numVaryingCurveKeysInSegment.Resize( Animation::E_VaryingCurveType_Count );
	numVaryingCurvesInSegment.Resize( Animation::E_VaryingCurveType_Count );
	
	// Count number of varying curves.
	U32 numVaryingCurves = 0;
	
	for ( U32 curveTypeIndex = 0; curveTypeIndex < Animation::E_VaryingCurveType_Count; ++curveTypeIndex )
	{
		Btl::Vector< C_CycleCurve* >& curves = m_varyingCycleCurvesByType[ curveTypeIndex ];
		const U32 numCurves = curves.Size();
		
		// Store number of varying curves.
		numVaryingCurvesInSegment[ curveTypeIndex ] = numCurves;

		numVaryingCurves += numCurves;
		
		currCurveKeyIndices[ curveTypeIndex ].Reserve( numCurves );
		
		for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
		{
			BIZ_ASSERT( curves[ curveIndex ]->m_curve->IsVarying() );
			
			// Set initial key indices to zero.
			currCurveKeyIndices[ curveTypeIndex ].PushBack( 0 );
			
			// Remove any existing curve segments.
			curves[ curveIndex ]->m_curveSegments.Clear();

			DBGLOG("Curve %s (%s) has %d keys", curves[curveIndex]->m_fieldName.Chars(), GetElementName(curves[curveIndex]->m_fieldElement), curves[curveIndex]->m_curve->GetNumKeys());
		}
		
	}
		
	BIZ_STATIC_ASSERT( kNumVaryingCurves == Animation::E_VaryingCurveType_Count );
		
	while ( startFrame < ( m_numFrames - 1 ) )
	{
		C_CycleSegmentLayout segment;

		DBGLOG("*** Segment %d ***", startFrame);
			
		//
		// For each curve, skip over keys that coincide with the start frame (they are always included)
		//
		for ( U32 curveTypeIndex = 0; curveTypeIndex < Animation::E_VaryingCurveType_Count; ++curveTypeIndex )
		{
			Btl::Vector< C_CycleCurve* >& curves = m_varyingCycleCurvesByType[ curveTypeIndex ];
			const U32 numCurves = curves.Size();
			
			// Varying curves have no keys initially.
			numVaryingCurveKeysInSegment[ curveTypeIndex ] = 0;
			
			for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
			{
				const C_Curve& curve = *curves[ curveIndex ]->m_curve;
				BIZ_ASSERT( curve.IsVarying() );
				

				U32& keyIndex = currCurveKeyIndices[ curveTypeIndex ][ curveIndex ];
				
				U32 initialFrameKeyIndex = keyIndex;
				
				// Ensure that currCurveKeyIndices is always equal to the index of the next curve key whose
				// time is greater than startFrame.

				// Skip over first keys in each that coincide with the start frame.
				U32 keyTime = curve.GetKeyTime( keyIndex );

				/*
				if (curves[curveIndex]->m_fieldName.ISearch("camera") >= 0)
				{
					DBGLOG( "  * Initial key %s, '%s', Curve %d, Index %d, Time %d\n", ToString( (Animation::E_VaryingCurveType) curveTypeIndex ), curves[curveIndex]->m_fieldName.Chars(), curveIndex, keyIndex, curve.GetKeyTime(keyIndex) );
				}
				*/

				if (keyTime == startFrame)
				{
					// Skip initial key.
					++keyIndex;
					initialFrameKeyIndex = keyIndex;

					// Add all other keys that coincide with this time. These will be stepped tangents/discontinuities
					// that need duplicate key times in order to evaluate correctly. If a segment just contains initial
					// key values, then these duplicate key times will be lost and the stepped values will end up being
					// smoothly interpolated, causing problems with camera cuts etc.
					keyTime = curve.GetKeyTime(keyIndex);
					while (keyTime == startFrame)
					{
						++keyIndex;
						++numVaryingCurveKeysInSegment[curveTypeIndex];
						keyTime = curve.GetKeyTime(keyIndex);
					}
				}
				else
				{
					BIZ_ASSERT(keyTime > startFrame);
				}
				curves[ curveIndex ]->m_curveSegments.PushBack(C_CurveSegment(initialFrameKeyIndex, 0.f));
			}
		}
	
		U32 endFrame = startFrame + 1; 	// End frame will be one frame beyond the last frame with keys.
		
		U32 segmentSize = segment.Layout(
			0,
			&numVaryingCurvesInSegment[ 0 ],
			&numVaryingCurveKeysInSegment[ 0 ]
		);
		
		//
		// Keep adding frames until segment exceeds maximum size.					
		//
		// Need to have at least two frames per segment otherwise stepped tangents may not work.
		//
		U32 prevSegmentSize = 0;
		while ( endFrame < ( m_numFrames - 1 ) && ( endFrame - startFrame ) < 255 && segmentSize < kBlockSizeLimit )
		{
			//
			// For each curve, add keys that coincide with this frame.
			//
			for ( U32 curveTypeIndex = 0; curveTypeIndex < Animation::E_VaryingCurveType_Count; ++curveTypeIndex )
			{
				Btl::Vector< C_CycleCurve* >& curves = m_varyingCycleCurvesByType[ curveTypeIndex ];
				const U32 numCurves = curves.Size();

				for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
				{
					const C_Curve& curve = *curves[ curveIndex ]->m_curve;
					bool keyPresent = false;
					
					U32& keyIndex = currCurveKeyIndices[curveTypeIndex][curveIndex];
					U32 keyTime = curve.GetKeyTime( keyIndex );
					
					while ( keyTime <= endFrame )
					{
						// Add key from this curve.
						//if (curves[curveIndex]->m_fieldName.ISearch("camera") >= 0)
						{
							DBGLOG( "  + Added key: %s, Curve %d, Index %d, Time %d\n", ToString( (Animation::E_VaryingCurveType) curveTypeIndex ), curveIndex, currCurveKeyIndices[ curveTypeIndex ][ curveIndex ], keyTime );
						}
						++keyIndex;
						++numVaryingCurveKeysInSegment[ curveTypeIndex ];
						keyTime = curve.GetKeyTime( keyIndex );
					}
				}
				
				DBGLOG( "%s has %d keys in segment, %d curves\n", ToString( (Animation::E_VaryingCurveType) curveTypeIndex ), numVaryingCurveKeysInSegment[ curveTypeIndex ], numVaryingCurvesInSegment[ curveTypeIndex ] );
			}
			
			++endFrame;
			
			// Calculate segment size.
			prevSegmentSize = segmentSize;
			segmentSize = segment.Layout(
				endFrame - startFrame,
				&numVaryingCurvesInSegment[ 0 ],
				&numVaryingCurveKeysInSegment[ 0 ] );
				
			
			DBGLOG( "Segment frames %d, Size %d\n", endFrame - startFrame, segmentSize );
		}

		// If there's more than one frame, try and remove keys from the last one to bring the segment size down.
		if (segmentSize > kBlockSizeLimit && endFrame > startFrame + 1)
		{
			BIZ_ASSERT(endFrame > startFrame + 1);

			// Step back to the previous frame.
			--endFrame;

			// Segment too big, remove all keys added in the last frame.
			for ( U32 curveTypeIndex = 0; curveTypeIndex < Animation::E_VaryingCurveType_Count; ++curveTypeIndex )
			{
				Btl::Vector< C_CycleCurve* >& curves = m_varyingCycleCurvesByType[ curveTypeIndex ];
				const U32 numCurves = curves.Size();
				
				for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
				{
					const C_Curve& curve = *curves[ curveIndex ]->m_curve;

					U32& currKeyIndex = currCurveKeyIndices[curveTypeIndex][curveIndex];

					if (currKeyIndex == 0)
					{
						continue;
					}
					
					// Remove keys corresponding to last frame, but not any duplicate keys for the start frame.
					U32 keyTime = curve.GetKeyTime(currKeyIndex - 1);
					while (currKeyIndex > 0 && keyTime == endFrame)
					{
						//if (curves[curveIndex]->m_fieldName.ISearch("camera") >= 0)
						{
							DBGLOG( "  - Removed key: %s, Curve %d, Index %d, Time %d\n", ToString( (Animation::E_VaryingCurveType) curveTypeIndex ), curveIndex, currKeyIndex - 1, curve.GetKeyTime(currKeyIndex - 1) );
						}
						// remove key
						--currKeyIndex;
						
						if ( endFrame > startFrame )
						{
							--numVaryingCurveKeysInSegment[ curveTypeIndex ];
						}

						keyTime = curve.GetKeyTime(currKeyIndex - 1);
					}
				}
				
				//DBGLOG( "%s has %d keys in segment, %d curves\n", ToString( (Animation::E_VaryingCurveType) curveTypeIndex ), numVaryingCurveKeysInSegment[ curveTypeIndex ], numVaryingCurvesInSegment[ curveTypeIndex ] );
			}
			
		}
		
		// Calculate number of keys used by each curve segment.
		for ( Animation::E_VaryingCurveType varCurveType = Animation::E_VaryingCurveType_First; varCurveType < Animation::E_VaryingCurveType_Count; ++varCurveType )
		{
			Btl::Vector< C_CycleCurve* >& curves = m_varyingCycleCurvesByType[ varCurveType ];
			const U32 numCurves = curves.Size();
			
			U32 totalKeys = 0;
			
			for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
			{
				const C_Curve& curve = *curves[ curveIndex ]->m_curve;
				
				U32 currKeyIndex = currCurveKeyIndices[ varCurveType ][ curveIndex ];
				
				C_CurveSegment& curveSeg = curves[ curveIndex ]->m_curveSegments.Back();
				curveSeg.m_numKeys = currKeyIndex - curveSeg.m_initialKeyIndex;
				
				totalKeys += curveSeg.m_numKeys;
			}
			
			//DBGLOG( "%s has %d keys in segment (counted %d), %d curves\n", ToString(varCurveType), numVaryingCurveKeysInSegment[ varCurveType ], totalKeys, numVaryingCurvesInSegment[ varCurveType ] );
			BIZ_ASSERT( totalKeys == numVaryingCurveKeysInSegment[ varCurveType ] );
		}		
		
		U32 numSegmentFrames = endFrame - startFrame;
			
		// Calculate final segment size.
		segmentSize = segment.Layout(
			numSegmentFrames,
			&numVaryingCurvesInSegment[ 0 ],
			&numVaryingCurveKeysInSegment[ 0 ] );

		if (segmentSize > largestSegmentSize)
		{
			// Track size of largest segment.
			largestSegmentSize = segmentSize;
			largestSegmentIndex = segmentIndex;
		}
		
		m_cycleSegments.PushBack( C_CycleSegment( segmentIndex, startFrame, numSegmentFrames ) );
		m_cycleSegments.Back().m_size = segment.m_sizeWithoutFinalKeys;
		
		for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
		{
			m_cycleSegments.Back().m_numVaryingCurves[ i ] = numVaryingCurvesInSegment[ i ];
			m_cycleSegments.Back().m_numVaryingCurveKeys[ i ] = numVaryingCurveKeysInSegment[ i ];
		}
		
		DBGLOG("segment %d startFrame: %d endFrame: %d numFrames: %d blockSize: %d bytes (%d without final keys)\n",
			//ToString( curveType ),
			segmentIndex,
			startFrame,
			endFrame,
			numSegmentFrames,
			segmentSize,
			segment.m_sizeWithoutFinalKeys
		);
		
		for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
		{
			const Animation::E_VaryingCurveType curveType = static_cast< Animation::E_VaryingCurveType >( i );
			DBGLOG("    [%s] %d curves (%d bytes), %d segment keys (%d bytes)\n",
				ToString( curveType ),
				numVaryingCurvesInSegment[ i ],
				numVaryingCurvesInSegment[ i ] * GetVaryingCurveKeySize( curveType ),
				numVaryingCurveKeysInSegment[ i ],
				numVaryingCurveKeysInSegment[ i ] * GetVaryingCurveKeySize( curveType )
			);
		}
		
		++segmentIndex;
		
		startFrame = endFrame;
		
		totalBlockSize += segmentSize;
	}
	
	// Create final segment which just stores keys for final frame.
	C_CycleSegmentLayout segment;
	
	m_cycleSegments.PushBack( C_CycleSegment( segmentIndex, m_numFrames - 1, 0 ) );

	for ( U32 i = 0; i < Animation::E_VaryingCurveType_Count; ++i )
	{
		m_cycleSegments.Back().m_numVaryingCurves[ i ] = numVaryingCurvesInSegment[ i ];
		m_cycleSegments.Back().m_numVaryingCurveKeys[ i ] = 0;
	}
	
	U32 segmentSize = segment.Layout(
		0,
		m_cycleSegments.Back().m_numVaryingCurves,
		m_cycleSegments.Back().m_numVaryingCurveKeys );

	totalBlockSize += segmentSize;


	if (segmentSize > largestSegmentSize)
	{
		// Track size of largest segment.
		largestSegmentSize = segmentSize;
		largestSegmentIndex = segmentIndex;
	}
	

	m_cycleSegments.Back().m_size = segment.m_sizeWithoutFinalKeys;
	
	LOGINFO( gLog, "Total size of all blocks: %d bytes (%.3fK)\n", totalBlockSize, totalBlockSize / 1024.f );

	if (largestSegmentSize > kBlockSizeLimit)
	{
		LOGWARN(gLog, "Could not fit animation data within block size limit (largest segment at frames %d to %d, size was %d, limit is %d): if this causes crashes on PS3, increase animation compression error threholds or reduce number of animated objects", 
			m_cycleSegments[largestSegmentIndex].m_startFrame,
			m_cycleSegments[largestSegmentIndex].m_startFrame + m_cycleSegments[largestSegmentIndex].m_numFrames,
			largestSegmentSize, 
			kBlockSizeLimit);
	}
	
	return C_Status( true );
}

void C_CycleBuilder::EstimateJointErrorThresholds(
	const C_CycleParameters& cycleParams,
	const C_Scene& scene,
	C_CompressionParameters& outCompressionParams
) const
{
	// Derive per-joint error thresholds by dividing global joint error threshold
	// by the maximum chain length of each joint. This helps compensate for
	// errors that propogate down the skeleton hierarchy by improving the accuracy
	// of animated joints nearer the root.
	const F32 maxGlobalRotationError = cycleParams.maxGlobalRotationError;
	const F32 maxGlobalTranslationError = cycleParams.maxGlobalTranslationError;
	const F32 maxGlobalScaleError = cycleParams.maxGlobalScaleError;

	for ( U32 i = 0; i < scene.GetNumNodes(); ++i )
	{
		U32 maxChainLength = scene.GetMaxNodeChainLength( i );
		BIZ_ASSERT( maxChainLength >= 1 );

		// Select base error threshold.
		F32 maxRotationError = maxGlobalRotationError;
		F32 maxTranslationError = maxGlobalTranslationError;
		F32 maxScaleError = maxGlobalScaleError;
		
		if (scene.IsHighFidelityNode(i))
		{
			maxRotationError = cycleParams.maxHighFidelityRotationError;
			maxTranslationError = cycleParams.maxHighFidelityTranslationError;
			maxScaleError = cycleParams.maxHighFidelityScaleError;
		}
		maxRotationError /= maxChainLength;
		maxTranslationError /= maxChainLength;
		maxScaleError /= maxChainLength;
		
		
	#if 0
		F32 maxChainDistance = scene.GetMaxNodeChainDistance( i );
		if ( maxChainDistance > 0.f )
		{
			F32 a = Maths::RadToDeg( asinf( maxTranslationError / maxChainDistance ) );
			maxRotationError = Maths::Min( maxRotationError, a );
		}
		
		LOGINFO( gLog, "Joint %s has a maximum chain distance of %g, length of %d. Max rot error: %g, max tran error: %g\n", scene.GetNodeName(i), maxChainDistance, maxChainLength, maxRotationError, maxTranslationError );
	#endif

		outCompressionParams.SetJointMaxRotationError( i, maxRotationError );
		outCompressionParams.SetJointMaxTranslationError( i, maxTranslationError );
		outCompressionParams.SetJointMaxScaleError( i, maxScaleError );
	}
}

} // namespace Animation
