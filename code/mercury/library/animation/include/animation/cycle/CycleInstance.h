#ifndef ANIMATION_CYCLEINSTANCE_H
#define ANIMATION_CYCLEINSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/JointTransform.h>
#include <system/scheduler/jobqueue.h>

namespace Animation
{

	class C_Cycle;
	class C_Pose;
	class C_CycleMapping;
	class C_StringId;
	
	/**
	An instance of a C_Cycle.
	
	On an SPU a C_CycleInstance's mapping from cycle to pose is immutable.
		
	@warning Deleting or reloading a cycle will make all CycleInstances bound to that cycle invalid.
	
	@see C_PoseLayout
	*/
	class ALIGN16 C_CycleInstance
	{
	public:
		
		friend class C_CommandProcessor;
		
#ifndef PLATFORM_SPU
		
		/// Default constructor.
		C_CycleInstance();
		
		/// Construct a reference to a cycle bound to the specified C_Pose. PPU-only.
		C_CycleInstance( const C_Cycle* cycle, const C_Pose* pose );
		
		/// Construct a reference to a cycle unassociated with a particular pose layout. PPU-only.
		explicit C_CycleInstance( const C_Cycle* cycle );

		C_CycleInstance(const C_CycleInstance& other);

		void operator=(const C_CycleInstance& rhs);
		
		~C_CycleInstance();
		
		/// Bind the referenced cycle to a pose. PPU-only.
		void BindToPose( const C_Pose& pose );

		/// Clear cycle instance, setting cycle and mapping pointers to null.
		void Clear();
		
		/// Create cycle instance for the specified cycle and pose.
		void Create( const C_Cycle* cycle, const C_Pose* pose );

#endif

		/// Fetch cycle and cycle mapping into SPU local memory.
		void SpuFetchCycleAndMapping();
		
		/// Returns true if reference is valid (i.e. non-null cycle and mapping pointers).
		bool IsValid() const;
		
		/// Returns true if reference is bound to a pose layout.
		bool IsBound() const;
		
		/// Get name of cycle, for debugging.
		C_StringId GetCycleName() const;
		
		/// Get duration of cycle in seconds.
		F32 GetDuration() const;
		
		/// Get frame rate of cycle.
		F32 GetFrameRate() const;
		
		/// Get memory usage of cycle in bytes (not including the C_CycleInstance).
		U32 GetCycleMemoryUsage() const;
		
		
		//
		// Cycle trajectory properties.
		//
		
		/// Get average linear velocity (in game units per second) of the trajectory joint.
		Maths::Vector3 GetLinearVelocity() const;
		
		/// Get average angular velocity (axis * radians per second) of the trajectory joint.
		Maths::Vector3 GetAngularVelocity() const;
		
		/// Get trajectory transform at first frame of animation.
		C_JointTransform GetFirstTrajectoryTransform() const;

		/// Get trajectory transform at final frame of animation.
		C_JointTransform GetFinalTrajectoryTransform() const;		
		
		//
		// Cycle reference transforms.
		//
		
		/// Get cycle reference transform.
		C_JointTransform GetReferenceTransform() const;
		
		//
		// Cycle reference marker properties.
		//
		
		//U32 GetNumReferenceMarkers() const;
		
		//C_JointTransform GetReferenceMarker( U32 index ) const;
				
		/// Get cycle to skeleton mapping.
		const C_CycleMapping* GetCycleMapping() const;
		
		//
		// Trajectory delta calculation.
		// 
		
		/// Get previous trajectory transform.
		C_JointTransform GetPreviousTrajectoryTransform() const;
		
		/// Set previous trajectory transform.
		void SetPreviousTrajectoryTransform( const C_JointTransform& t );
		
		/// Get value of update counter when this cycle instance was last evaluated.
		U32 GetPreviousUpdateCounter() const;
		
		/// Set value of update counter to indicate that the previous trajectory transform
		/// and update time are now valid.
		void SetPreviousUpdateCounter( U32 currentCounter );
		
		F32 GetPreviousTime() const;
		
		F32 GetPreviousTimeInFrames() const;
		
		void SetPreviousTime( F32 time, F32 timeInFrames );
		
		const C_Cycle* GetCycle() const	{ return m_cycle; }
	
	private:
		void ReleaseCycleMapping();

		Spu::Ptr<const C_Cycle> m_cycle;
		Spu::Ptr<const C_CycleMapping> m_cycleMapping;
		
		MathsStorage::Float4 m_prevTrajectoryRotation;
		MathsStorage::Float3 m_prevTrajectoryTranslation;
		F32					 m_duration;
		
		U32 m_mappingSize;
		
		F32 m_prevTime;
		F32 m_prevTimeInFrames;		// Must be stored due as deriving this value from m_prevTime causes inconsistencies between SPU and PPU.

		U32 m_prevUpdateCounter;	// Update counter when cycle was last evaluated
	};
}

#include "CycleInstance.inl"

#endif
