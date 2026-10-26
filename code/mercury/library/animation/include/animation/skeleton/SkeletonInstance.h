#ifndef ANIMATION_SKELETONINSTANCE_H
#define ANIMATION_SKELETONINSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/MemoryBlock.h>
#include <system/Scheduler.h>

namespace Animation
{
	class C_Skeleton;
	class C_JointTransform;
	class C_Pose;
	
	class C_DynamicJoint;
	class C_DynamicAimJoint;
	class C_DynamicSliderJoint;

	class C_WindParams;
		
	/**
	Types of dynamic joint.
	*/
	enum E_DynamicJointType
	{
		E_DynamicJointType_Aim	= 0,		///< A dynamic aim joint.
		E_DynamicJointType_Slider,			///< A dynamic slider joint.
		E_DynamicJointType_Count,
		E_DynamicJointType_Invalid = -1
	};

	/**
		An instance a skeleton with optional dynamic joints.
	*/
	class ALIGN16 C_SkeletonInstance
	{
	public:
				
		//
		// Construction and destruction.
		//
		C_SkeletonInstance();
		
#if !defined(PLATFORM_SPU)
		/// Create an instance of the specified skeleton.
		void Create( const C_Skeleton& skeleton );	
		
		/// Remap onto a new skeleton, preserving the current dynamic joint state.
		void Remap( const C_Skeleton& newSkeleton );
						
		/// Destroy instance.
		void Destroy();
#endif		

		/// Resets dynamic joints to sensible initial states.
		void Reset();

		/// DMAs data from main memory into SPU local memory.
		void SpuFetchAsync(U32 dmaTag);

		/// DMAs data from main memory into SPU local memory.
		void SpuFetch();

		/// DMAs data back to main memory from SPU local memory.
		void SpuStore();

		/// Apply joint constraints, updating dynamic joints in the process. If @a skeleton is not NULL then it is used instead
		/// of the internal skeleton.  This is useful for SPU code where the skeleton may have already
		/// been copied into local memory and a call to SpuFetch() would be redundant.
		void ApplyJointConstraints( C_Pose& pose, const C_JointTransform& modelWorldTransform, const C_Skeleton* skeleton = NULL );

		/// Get skeleton used by this instance.
		const C_Skeleton* GetSkeleton() const		{ return m_skeleton; }
		
		/// Returns true if SkeletonInstance has any constraints. If false, then ApplyJointConstraints() does not need to be called.
		bool HasConstraints() const				{ return m_hasConstraints; }

		/// Returns true if SkeletonInstance has any dynamic joints.
		bool HasDynamicJoints() const			{ return m_hasDynamicJoints; }
		
		//
		// Dynamic joints.
		//
	
		/// Update time for dynamic joints.
		void UpdateDynamicJoints( F32 timeStep );
		
		/// Enables or disables dynamic joint evaluation.
		void SetDynamicJointsEnabled(bool enabled);
		
		/// Returns true if dynamic joints are enabled.
		bool AreDynamicJointsEnabled() const;
		
		/// Gets gravitational acceleration that will be applied to all dynamic joints.
		Maths::Vector3 GetGravity() const;
		
		/// Sets gravitational acceleration applied to all dynamic joints.
		void SetGravity(const Maths::Vector3& g);
		
		/// Gets the number of dynamic joints, of all types.
		U32 GetNumDynamicJoints() const;
		
		/// Applies a force to the specified dynamic joint instance.
		void ApplyForceToDynamicJoint(S32 instance, const Maths::Vector3& force);
		
		/// Applies a wind force to the specified dynamic joint instance. The force will be scaled by the
		/// dynamic joint's wind influence parameter so that certain joints can be excluded from wind effects.
		void ApplyWindForceToDynamicJoint(S32 instance, const Maths::Vector3& force);

		/// Applies wind forces to all dynamics joints, scaled by their wind influence factors.
		void ApplyWindForces(const C_WindParams& windParams, F32 timeStep);

		/// Returns the current time value used by the internal wind simulation.
		U32 GetWindTime() const		{ return m_windTime; }
		
		/// Gets the type of a dynamic joint.
		E_DynamicJointType GetDynamicJointType(U32 instance) const;
		
		/// Finds the dynamic joint instance associated with a skeleton joint. Returns -1 if skeleton joint is not dynamic.
		S32 FindDynamicJoint(S32 skeletonJointIndex) const;

		/// Returns the index of the skeleton joint being associated with a dynamic joint.
		S32 GetDynamicJointSkeletonJointIndex(U32 instance) const;
		
		//
		// Common dynamic joint settings.
		// 
#if !defined(PLATFORM_SPU)
				
		/// Sets the mass of a dynamic joint. Works for all dynamic joint types.
		void SetDynamicJointMass(U32 instance, F32 mass);
		
		/// Gets the mass of a dynamic joint. Works for all dynamic joint types.
		F32 GetDynamicJointMass(U32 instance) const;

		/// Sets the air resistance for a dynamic joint. Works for all dynamic joint types.
		void SetDynamicJointAirResistance(U32 instance, F32 mass);
		
		/// Gets the air resistance for a dynamic joint. Works for all dynamic joint types.
		F32 GetDynamicJointAirResistance(U32 instance) const;
		
		/// Sets the wind influence for a dynamic joint. Works for all dynamic joint types.
		void SetDynamicJointWindInfluence(U32 instance, F32 influence);

		/// Gets the wind influence for a dynamic joint. Works for all dynamic joint types.
		F32 GetDynamicJointWindInfluence(U32 instance) const;

		/// Sets the buoyancy influence for a dynamic joint. Works for all dynamic joint types.
		void SetDynamicJointBuoyancyInfluence(U32 instance, F32 influence);

		/// Gets the buoyancy influence for a dynamic joint. Works for all dynamic joint types.
		F32 GetDynamicJointBuoyancyInfluence(U32 instance) const;
				
		//
		// Dynamic slider joint settings.
		//
				
		/// Sets the spring constants for a dynamic slider joint. Does nothing if specified dynamic joint is not the correct type.
		void SetDynamicSliderJointSpringConstants(U32 instance, F32 spring, F32 damping);
		
		/// Gets the spring constants for a dynamic slider joint. Returns 0.0 if dynamic joint is not of the correct type.
		void GetDynamicSliderJointSpringConstants(U32 instance, F32* outSpring, F32* outDamping) const;

		/// Sets the translation limits a dynamic slider joint. Does nothing if specified dynamic joint is not the correct type.
		void SetDynamicSliderJointLimits(U32 instance, F32 minLimit, F32 maxLimit);
		
		/// Gets the translation limits for a dynamic slider joint. Returns 0.0 if dynamic joint is not of the correct type.
		void GetDynamicSliderJointLimits(U32 instance, F32* outMin, F32* outMax) const;
		
		//
		// Dynamic aim joint settings.
		//
		
		/// Set the target joint for a dynamic aim joint.
		void SetDynamicAimJointTarget(U32 instance, S32 targetJointIndex);
		
		/// Gets dynamic aim joint spring constants.
		void GetDynamicAimJointSpringConstants(U32 instance, F32* outPitchSpring, F32* outPitchDamping, F32* outHeadingSpring, F32* outHeadingDamping) const;
		
		/// Sets dynamic aim joint spring constants.
		void SetDynamicAimJointSpringConstants(U32 instance, F32 pitchSpring, F32 pitchDamping, F32 headingSpring, F32 headingDamping);
		
		
		/// Gets dynamic aim joint limits.
		void GetDynamicAimJointLimits(U32 instance, F32* outPitchMin, F32* outPitchMax, F32* outHeadingMin, F32* outHeadingMax) const;
		
		/// Sets dynamic aim joint limits.
		void SetDynamicAimJointLimits(U32 instance, F32 pitchMin, F32 pitchMax, F32 headingMin, F32 headingMax);
		
		/// Gets the skeleton joint index of the specified dynamic aim joint's aim target. Returns -1 if target joint is invalid.
		S32 GetDynamicAimJointTargetJointIndex(U32 instance) const;
#endif
	
	private:
		C_DynamicJoint*	GetDynamicJoint(U32 instance);
		const C_DynamicJoint*	GetDynamicJoint(U32 instance) const;
	
		Spu::Ptr<const C_Skeleton> m_skeleton;
		Maths::Vector3	m_gravity;
		F32				m_timeLeft;						// Dynamic joint time step accumulator.
		F32				m_fixedTimeStep;
		U32				m_windTime;
		U16 			m_numDynamicAimJoints;
		U16 			m_numDynamicSliderJoints;
		C_MemoryBlock	m_dynamicJointData;
		C_RelativePtr<C_DynamicAimJoint>	m_dynamicAimJoints;
		C_RelativePtr<C_DynamicSliderJoint>	m_dynamicSliderJoints;
		bool			m_dynamicJointsEnabled;
		bool			m_hasConstraints;
		bool			m_hasDynamicJoints;
		bool			m_reset;
	};
}

#endif
