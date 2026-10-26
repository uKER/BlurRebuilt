#ifndef ANIMATION_TOOLS_CYCLEBUILDER_H
#define ANIMATION_TOOLS_CYCLEBUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Curve.h"
#include <animation/common/JointTransform.h>
#include <animation/pose/PoseFieldTypes.h>
#include <animation/tools/Status.h>
#include <animation/tools/PatternList.h>
#include <btl/Containers/vector.h>
#include <btl/Containers/bitvector.h>
#include <btl/Containers/map.h>
#include <btl/utility/pair.h>

namespace Sys
{
	class I_BinaryStream;
	class I_File;
	class BinarySerialiserAutoEndian;
}

namespace Progress
{
	class I_ProgressBar;
}

namespace Animation
{
	class C_Skeleton;
}

namespace AnimationTools
{
	class C_CustomCurveBuilder;
	class C_Scene;
	
	/// @addtogroup Animation_Tools Animation Tools
	//! @{

	///
	/// Parameters describing how a cycle is be exported from a scene.
	///
	class C_CycleParameters
	{
	public:
		/// Default constructor.
		C_CycleParameters();

		F32		worldOrigin[3];					///< Point to use as world origin, default (0,0,0).
		
		F32		oversampling;						///< Time oversampling factor.
		F32		maxGlobalRotationError;				///< Maximum global rotation error, in degrees.
		F32		maxGlobalTranslationError;			///< Maximum global translation error, in game units.
		F32		maxGlobalScaleError;				///< Maximum global scale error, in game units.
		F32		maxHighFidelityRotationError;		///< Maximum high fidelity bone rotation error in degrees.
		F32		maxHighFidelityTranslationError;	///< Maximum high fidelity bone translation error in game units.
		F32		maxHighFidelityScaleError;			///< Maximum high fidelity scale error, in game units.
		F32		maxRelativeCustomCurveError;		///< Maximum custom curve error, relative to min/max.
		bool	removeTranslationChannelsMatchingBasePose;	///< True if constant translation channels that match base pose can be removed.
		bool	removeScaleChannelsMatchingBasePose;	///< True if constant scale channels that match base pose can be removed.
		bool	removeChannelsWithNoKeys;			///< If true, remove channels without keys to create a partial animation.
		bool	removeIdentityScaleChannels;		///< If true, remove channels with a scale of (1,1,1). Only applies if enableScale is also true.
		bool	inWorldSpace;						///< If true, all transforms will be stored in world space (default is false).
		bool	enableScale;						///< If true, all transforms will be stored as ObjectTransforms with scale (but not shear!) support.
		
		bool operator == ( const C_CycleParameters& rhs ) const;
	};
	
	struct C_CurveSegment
	{
		C_CurveSegment() {}
		C_CurveSegment( U32 initialKeyIndex, F32 initialKeyBlend ):
			m_initialKeyIndex( initialKeyIndex ),
			m_initialKeyBlend( initialKeyBlend )
		{
		}
		U32	m_initialKeyIndex;
		U32 m_numKeys;
		F32 m_initialKeyBlend;
	};


	struct C_CycleSegment
	{
		C_CycleSegment( U32 index, U32 startFrame, U32 numFrames ):
			m_index( index ),
			m_startFrame( startFrame ),
			m_numFrames( numFrames )
		{
		}
		
		C_CycleSegment()
		{
		}
		
		U32 m_index;
		U32 m_startFrame;
		U32 m_numFrames;
		U32 m_size;
		
		U32 m_numVaryingCurves[ Animation::E_VaryingCurveType_Count ];
		U32 m_numVaryingCurveKeys[ Animation::E_VaryingCurveType_Count ];
	};
	
	struct C_CycleCurve
	{
		C_CycleCurve():
			m_fieldType( Animation::E_PoseFieldType_Count ), m_fieldIndex( -1 ), m_curve( 0 ), m_rawCurve( 0 ) {}
		
		C_CycleCurve( Animation::E_PoseFieldType fieldType, const Util::C_String& fieldName, Animation::E_PoseFieldElement fieldElement, const C_Curve* curve, const C_Curve* rawCurve ):
			m_fieldType( fieldType ),
			m_fieldName( fieldName ),
			m_fieldElement( fieldElement ),
			m_curve( curve ),
			m_rawCurve( rawCurve )
		{}
		
		Animation::E_PoseFieldType		m_fieldType;	// Type of field that this curve applies to.
		S32								m_fieldIndex;	// Field index, relative to type. (i.e. custom curve number or joint number)
		Animation::E_PoseFieldElement	m_fieldElement;	// Element of field controlled by this curve (multiple curves can contribute to a single field)
		Util::C_String					m_fieldName;
		
		const C_Curve* 					m_curve;		// Pointer to curve (in m_cycleJoints, so ensure that m_cycleJoints isn't modified!)
		const C_Curve* 					m_rawCurve;		// Pointer to raw curve.
		
		Btl::Vector< C_CurveSegment >	m_curveSegments;

		C_CycleCurve*					m_nextByField;
	};
	
	///
	/// Builds a cycle from a scene. The resulting cycle can be serialised into a
	/// format that can be read by the C_Cycle class. 
	/// Also supports evaluation of compressed and raw data for previewing purposes.
	///
	class C_CycleBuilder
	{
	public:
	
		enum E_Source
		{
			E_Source_Original	= 0,		/// Original uncompressed animation data, without key reduction.
			E_Source_Compressed				/// Key-reduced, compressed animation data.
		};
	
		/// Default constructor, builds an empty cycle.
		C_CycleBuilder();

		~C_CycleBuilder();
		
		/// Clears all cycle data.
		void Clear();
		
		
		/// Returns true if cycle data is valid.
		bool IsValid() const;
		
		/// Build a cycle by importing all supported animated data from the specified scene.
		C_Status BuildCycle(
			C_Scene& cycleScene,
			const C_CycleParameters& cycleParameters,
			Progress::I_ProgressBar* progressBar = NULL
		);
		
		/// Build an additive cycle by importing animated data from two scenes.
		/// The subtraction performed is essentially cycleLhs - cycleRhs.
		C_Status BuildAdditiveCycle(
			C_Scene& cycleLhsScene,
			C_Scene& cycleRhsScene,
			const C_CycleParameters& cycleParameters,
			Progress::I_ProgressBar* progressBar = NULL
		);
		
		/// Serialise cycle into a stream.
		bool Serialise( Sys::I_BinaryStream& stream ) const;
		
		//
		// Methods for reading imported animation data before and after compression.
		//
		
		/// Is the cycle additive?
		bool IsAdditive() const;
		
		/// Get the frame rate of the cycle.
		F32 GetFrameRate() const;
		
		/// Get playback duration of cycle, in seconds.
		F32 GetDurationSeconds() const;
		
		/// Get playback duration of cycle, in frames.
		U32 GetDurationFrames() const;
		
		/// Get local space joint transform. The \a source parameter can be used to choose between the compressed and key-reduced
		/// animation curves or the original animation data, sampled per frame.
		Animation::C_JointTransform GetLocalJointTransform(
			U32 skeletonJointIndex,
			F32 timeSeconds,
			E_Source source = E_Source_Compressed
		) const;
		
		Animation::C_JointTransform GetLocalJointDeltaTransform(
			U32 skeletonJointIndex,
			F32 prevTimeSeconds,
			F32 currTimeSeconds,
			E_Source source = E_Source_Compressed
		) const;
		
		Animation::C_JointTransform GetFirstLocalJointTransform( U32 skeletonJointIndex, E_Source source = E_Source_Compressed ) const;
		
		Animation::C_JointTransform GetLastLocalJointTransform( U32 skeletonJointIndex, E_Source source = E_Source_Compressed ) const;
		
		U32 GetNumRotationKeys( U32 skeletonJointIndex, E_Source source = E_Source_Compressed ) const;
		
		void GetRotationKey( U32 skeletonJointIndex, U32 keyIndex, U32& outKeyTime, Maths::C_Quaternion& outKey, E_Source source = E_Source_Compressed ) const;
		
		U32 GetNumTranslationKeys( U32 skeletonJointIndex, E_Source source = E_Source_Compressed ) const;
		
		void GetTranslationKey( U32 skeletonJointIndex, U32 keyIndex, U32& outKeyTime, Maths::HVec3& outKey, E_Source source = E_Source_Compressed ) const;
	
		
	private:

		// Add a new cycle curve, adding to constant or varying lists and assigning an appropriate field index.
		// Takes ownership of cycle curve, which must have been allocated on the heap.
		void AddCycleCurve( C_CycleCurve* cycleCurve );

		const C_CycleCurve* FindCycleCurveByFieldIndex(S32 fieldIndex) const;
	
		void BuildListOfCycleJoints( const C_Scene& scene, const C_CycleParameters& cycleParameters );
		void BuildListOfCustomCurves( const C_Scene& scene, const C_CycleParameters& cycleParameters );

		C_Status ExtractReferenceTransforms( const C_Scene& scene, const C_CycleParameters& cycleParameters );
		C_Status ExtractConstantData( const C_Scene& scene, const C_CycleParameters& cycleParameters );
	
		void SampleFrames(
			C_Scene& cycleScene,
			const C_CycleParameters& cycleParameters,
			Progress::I_ProgressBar* progressBar
		);

		void SampleAdditiveFrames(
			C_Scene& cycleScene1,
			C_Scene& cycleScene2,
			const C_CycleParameters& cycleParameters,
			Progress::I_ProgressBar* progressBar
		);
		
		void SampleCustomCurves(
			const C_Scene& scene
		);
		
		void ForceRotationsToNearestNeighbours();
		void CalculateTrajectoryDelta(const C_CycleParameters& cycleParameters);
		
		C_Status BuildCompressedCurves(
			const C_CycleParameters& cycleParameters,
			const C_Scene& scene,
			Progress::I_ProgressBar* progressBar
		);
		
		C_Status BuildCompressedCustomScalarCurves(
			const C_CycleParameters& cycleParameters,
			const C_Scene& scene,
			Progress::I_ProgressBar* progressBar
		);

		C_Status BuildCompressedCustomVector3Curves(
			const C_CycleParameters& cycleParameters,
			const C_Scene& scene,
			Progress::I_ProgressBar* progressBar
		);

		C_Status BuildCompressedCustomVector4Curves(
			const C_CycleParameters& cycleParameters,
			const C_Scene& scene,
			Progress::I_ProgressBar* progressBar
		);

		
		void BuildRawCurves();
		
		C_Status BuildVaryingCurveSegments();
		
		void EstimateJointErrorThresholds(
			const C_CycleParameters& cycleParams,
			const C_Scene& scene,
			class C_CompressionParameters& outCompressionParams ) const;
	
		struct CycleJoint
		{
			CycleJoint(): m_skeletonJointIndex( -1 ) {}
			CycleJoint( U32 skeletonJointIndex ): m_skeletonJointIndex( skeletonJointIndex ) {}
			
			U32				m_skeletonJointIndex;
			Util::C_String	m_name;
			
			bool 			m_hasRotation;
			bool 			m_hasTranslation;
			bool 			m_hasScale;
			
			// Base pose.
			Animation::C_JointTransform			m_localBasePoseTransform;
		
			// Regularly-sampled rotation, translation and scale frames.
			Btl::Vector< Maths::C_Quaternion >	m_rotationFrames;
			Btl::Vector< MathsStorage::Float3 > m_translationFrames;
			Btl::Vector< MathsStorage::Float3 > m_scaleFrames;
			
			// Per-frame discontinuity flags.
			Btl::BitVector		m_rotationDiscontinuityFlags;
			Btl::BitVector		m_translationDiscontinuityFlags;
			Btl::BitVector		m_scaleDiscontinuityFlags;
						
			// Key-reduced curves.
			C_Curve 			m_rotationCurve;
			C_Curve 			m_translationCurve;
			C_Curve 			m_scaleCurve;
			
			// Raw curves (duplicates of rotationFrames and translationFrames).
			C_Curve 			m_rawRotationCurve;
			C_Curve 			m_rawTranslationCurve;
			C_Curve 			m_rawScaleCurve;
		};
		
		struct CustomScalarCurve
		{
			Util::C_String			name;
			Btl::Vector<F32>		frames;
			C_Curve					curve;
			C_Curve					rawCurve;
			Btl::BitVector			discontinuityFlags;
		};

		struct CustomVector3Curve
		{
			Util::C_String			name;
			Btl::Vector<MathsStorage::Float3>		frames;
			C_Curve					curve;
			C_Curve					rawCurve;
			Btl::BitVector			discontinuityFlags;
		};

		struct CustomVector4Curve
		{
			Util::C_String			name;
			Btl::Vector<MathsStorage::Float3>	framesXYZ;
			Btl::Vector<F32>					framesW;
			C_Curve					curveXYZ;
			C_Curve					curveW;
			C_Curve					rawCurveXYZ;
			C_Curve					rawCurveW;
			Btl::BitVector			discontinuityFlags;	// Separate for XYZ and W?
		};
		
		U32 m_numFrames;							///< Total number of frames in the animation.
		U32 m_durationFrames;						///< Duration of the animation in frames, for playback. For looped animations, this excludes the final frame.
		F32 m_frameRate;							///< Frame rate (fps/Hz).
		bool m_isAdditive;							///< True if animation is additive.
		
		S32 m_trajectoryRootFieldIndex;				///< Field index of trajectory root.
		
		Animation::C_JointTransform m_trajectoryDelta;
		Animation::C_JointTransform m_trajectoryStart;
		Animation::C_JointTransform m_trajectoryFinish;
		
		U32											m_numReferenceTransforms;
		Animation::C_JointTransform					m_referenceTransform;	// Single reference transform - may allow more in the future.
		
		typedef Btl::Pair< Animation::E_PoseFieldType, Util::C_String > C_FieldKey;
		typedef Btl::Map< C_FieldKey, U32 >	 C_FieldKeyToIndexMap;
		
		C_FieldKeyToIndexMap						m_fieldKeyToIndexMap;
		U32											m_totalFields;
		Btl::Vector< Util::C_String >				m_fieldNames;
		Btl::Vector< Animation::E_PoseFieldType >	m_fieldTypes;
		
		Btl::Vector< S32 >							m_cycleJointFromSkeletonJoint;	///< Map from skeleton joints onto cycle joints. -1 means skeleton joint not present (partial anims).
		Btl::Vector< CycleJoint >					m_cycleJoints;
		
		// Custom curves.
		Btl::Vector< CustomScalarCurve >			m_customScalarCurves;
		Btl::Vector< CustomVector3Curve >			m_customVector3Curves;
		Btl::Vector< CustomVector4Curve >			m_customVector4Curves;

		Btl::Vector<C_CycleCurve*>					m_cycleCurves;	// List of cycle curves for each 

		// Linked Lists of CycleCurves for each field.
		Btl::Vector<C_CycleCurve*>					m_cycleCurvesByField;

		typedef Btl::Vector< Btl::Vector<C_CycleCurve* >, Btl::VectorParameters_Array< Animation::E_ConstantCurveType_Count > > ConstantCycleCurveVector;
		typedef Btl::Vector< Btl::Vector<C_CycleCurve* >, Btl::VectorParameters_Array< Animation::E_VaryingCurveType_Count > > VaryingCycleCurveVector;
		
		ConstantCycleCurveVector		m_constantCycleCurvesByType;
		VaryingCycleCurveVector			m_varyingCycleCurvesByType;
		Btl::Vector< C_CycleSegment >	m_cycleSegments;
	};
	
	//! @}
}



#endif
