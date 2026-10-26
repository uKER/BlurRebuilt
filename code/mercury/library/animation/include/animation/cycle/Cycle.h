#ifndef ANIMATION_CYCLE_H
#define ANIMATION_CYCLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/StringId.h>
#include <animation/common/JointTransform.h>
#include <animation/common/BlendModes.h>
#include <animation/pose/PoseFieldTypes.h>
#include <animation/pose/PoseFieldMask.h>
#include "CurveTypes.h"

namespace Sys
{
	class I_BinaryStream;
}

namespace Animation
{
	class C_Skeleton;
	class C_CycleMapping;
	class C_JointWeights;
	class C_StringId;
	class C_StringTable;
	class C_Pose;
	class C_CycleInstance;
	class C_PoseLayout;
	class C_EvaluationContext;

	///
	/// Animation cycle.
	///
	///
	class ALIGN16 C_Cycle
	{
	public:

#ifndef PLATFORM_SPU
		C_Cycle();
		~C_Cycle();
#endif

		void Destroy();
		
		/// Returns true if cycle is valid.
		bool IsValid() const;

		/// Returns true if the cycle does not have any animation data, just header information. This will mean
		/// that the cycle can be used to construct PoseFormats but cannot be evaluated.
		bool IsPartiallyLoaded() const;
		
		/// Get frame rate of cycle in frames per second.
		F32 GetFrameRate() const;

		/// Get duration of cycle in frames.
		/// This will exclude the extra end frame if the cycle is looped.
		F32 GetDurationFrames() const;

		/// Get duration of cycle in seconds.
		/// This will exclude the extra end frame if the cycle is looped.
		F32 GetDuration() const;
		
		/// Get average linear velocity (in game units per second) of the trajectory joint.
		Maths::Vector3 GetLinearVelocity() const;
		
		/// Get average angular velocity (axis * radians per second) of the trajectory joint.
		Maths::Vector3 GetAngularVelocity() const;
		
		/// Get trajectory transform at first frame of animation.
		const C_JointTransform& GetFirstTrajectoryTransform() const;

		/// Get trajectory transform at final frame of animation.
		const C_JointTransform& GetFinalTrajectoryTransform() const;
		
		/// Evaluates the trajectory transform at an arbitrary time.
		C_JointTransform EvaluateTrajectoryTransform( F32 time, E_LoopMode loopMode = E_LoopMode_Continuous ) const;
				
		/// Get reference transform.
		const C_JointTransform& GetReferenceTransform() const;
		
		/// Set cycle name.
		void SetName( const CHAR8* name );
		
		/// Get cycle name.
		const C_StringId& GetName() const;
		
		/// Get total memory used by cycle in bytes.
		U32 GetMemoryUsage() const;
		
		
		//
		// Cycle mapping list management.
		// 
		
#ifndef PLATFORM_SPU
		/// Get a mapping from this cycle onto the specified pose layout. PPU-only.
		const C_CycleMapping* GetMapping( const C_PoseLayout& poseLayout ) const;
		
		/// Release all cached mappings. PPU-only.
		void ClearMappingCache() const;
		
		void SetMappingListHead( C_CycleMapping* mapping ) const
		{
			m_mappings = mapping;
		}
		
		C_CycleMapping* GetMappingListHead() const
		{
			return m_mappings;
		}
#endif
				
		/// Evaluate cycle channels into a C_Pose.		
		void Evaluate(
			E_BlendMode blendMode,
			F32 time,
			C_CycleInstance& instance,
			U32 updateCounter,
			F32 weight,
			C_Pose& pose,
			E_LoopMode loopMode = E_LoopMode_Continuous,
			const C_PoseFieldMask& fieldMask = C_PoseFieldMask()
		) const;
		
	private:
		void EvaluateTrajectoryDelta(
			C_JointTransform& outTrajectoryDelta,
			C_CycleInstance& instance,
			const C_EvaluationContext& evalContext,
			const F32 time,
			const F32 timeInFrames
		) const;
		
	public:

		/// Get number of fields of animated data.
		U32 GetNumFields() const;
		
#ifndef PLATFORM_SPU
		/// Get a data field's name. PPU-only.
		const C_StringId& GetFieldName( U32 fieldIndex ) const;
		
		/// Get a field's type. PPU-only.
		E_PoseFieldType GetFieldType( U32 fieldIndex ) const;

		/// Returns the total amount of memory used by a field's animation data. For debug only.
		U32 GetFieldMemoryUsage( U32 fieldIndex ) const;

	private:
		U32 GetVaryingCurveMemoryUsage(E_VaryingCurveType curveType, U32 curveIndex) const;
		U32 GetNumVaryingCurveKeys(E_VaryingCurveType curveType, U32 curveIndex) const;
#endif

	public:
		//
		// Serialisation.
		//
		
#ifndef PLATFORM_SPU

		static const U32 kLoadFull			= 0;
		static const U32 kLoadHeaderOnly	= 1;

		/// Load cycle from a binary stream. Returns true on success. PPU-only.
		template < typename T >
		bool Deserialise( T& in, U32 flags = kLoadFull );
#endif
		
	private:
		/// Segment header.
		class C_Segment
		{
		public:
			U32	m_startFrame;	//< First frame of segment.
			U8* m_data;			//< Segment data.
			U16 m_numFrames;	//< Number of frames in segment.
			U16 m_size;			//< Size of segment in bytes.
			U32 m_totalKeys;	//< Total number of keys in segment (i.e. no of key times)
		};
		
		/// Convert a time in seconds into animation frames.
		F32 GetFramesFromSeconds( F32 timeInSeconds, E_LoopMode loopMode = E_LoopMode_Continuous ) const;
		
		/// Get index of segment containing data for the specified time.
		U32 FindSegmentIndex( const C_Segment* segments, F32 timeInFrames ) const;

			
	private:
		//
		// Cycle header.
		// 
		C_JointTransform	m_trajectoryDelta;			// Delta for trajectory root over entire animation.
		C_JointTransform	m_trajectoryFirst;			// First frame of trajectory root.
		C_JointTransform	m_trajectoryFinal;			// Last frame of trajectory root.
		C_JointTransform	m_referenceTransform;		// Reference transform (only one at the moment, but may end up as an array).
		Maths::Vector3		m_angularVelocity;			// Average angular velocity of trajectory root over entire animation (radians/sec)
		Maths::Vector3		m_linearVelocity;			// Average linear velocity of trajectory root over entire animation (game units/sec).
		S32					m_trajectoryRootFieldIndex;	// Index of trajectory root field, or -1 if no trajectory.

		U32					m_numFrames;				// Number of frames in animation.
		F32					m_playbackDuration;			// Playback duration in frames (won't include final frame in looped animations).
		F32					m_frameRate;				// Number of frames per second.
		U32					m_numReferenceTransforms;	// Number of reference transforms. Currently either one or zero.

		U32					m_numFields;				///< Total number of animated data fields in this cycle.
		
		U8*					m_fieldTypes;				///< Type of each data field.
		C_StringIdArray		m_fieldNames;				///< Name of each data field.

		//
		// Cycle data.
		//

		// Number of curves of type.
		U16					m_totalConstantCurves;
		U16					m_totalVaryingCurves;
		U16					m_numConstantCurvesOfType[ E_ConstantCurveType_Count ];
		U16					m_numVaryingCurvesOfType[ E_VaryingCurveType_Count ];
		
		// Channel info for all constant and varying curves.
		U16*				m_channelInfo;
		
		U32 				m_constantCurveDataSize;	//< Size of all constant curve data.
		U8* 				m_constantCurveData;		//< Memory block containing all constant curve data.
		
		U32					m_numSegments;
		U32					m_totalSegmentDataSize;		//< Size of all segment data.
		U8*					m_segmentData;				//< Pointer to memory block containing all segment data.
		C_Segment*			m_segments;					//< Array of segments.
		
		U32					m_initialFrameSize;			//< Size of initial frame data at start of each segment.
		
		// Linked list of cycle to skeleton mappings (managed on PPU).
		mutable C_CycleMapping*	m_mappings;
		
		C_StringId			m_name;						/// Name of cycle, for debugging.
		U16					m_isPartiallyLoaded : 1;	/// Flag indicating that cycle is only partially loaded and cannot be evaluated.
	};
 
}

#include "Cycle.inl"

#endif


