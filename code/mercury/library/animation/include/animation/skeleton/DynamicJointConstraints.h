#ifndef ANIMATION_DYNAMICJOINTCONSTRAINTS_H
#define ANIMATION_DYNAMICJOINTCONSTRAINTS_H

#ifdef _MSC_VER
#pragma once
#endif

/**
	@file DynamicJointConstraints.h
	
	@defgroup DynamicJointConstraints Dynamic Joint Constraints
	
	These are joint constraints with persistent state, used for secondary animation effects. 
	\note All dynamic joints must handle invalid joint indices gracefully, since they may
	be remapped. (see Animation::C_SkeletonInstance::Remap)
	
	@see C_SkeletonInstance
*/

namespace Animation
{
	class C_Skeleton;
	class C_StringId;
	class C_SkeletonPose;
	class C_JointTransform;
	class C_DynamicJointContext;
	
	// This ought to be in its own file...
	enum E_SkeletonConstraintsVersion
	{
		E_SkeletonConstraintsVersion_DynamicAimJointAimAxis			= 2,
		E_SkeletonConstraintsVersion_DynamicAimJointUpVector		= 3,
		E_SkeletonConstraintsVersion_WindAndBuoyancyInfluence		= 4
	};
	
	/**
	Base class containing common parameters for dynamic joints.
	*/
	class C_DynamicJoint
	{
	public:	
		C_DynamicJoint();
		// Derived classes never allocate memory, so it should be safe to omit virtual destructor.
	
		void	SetJointIndex(S32 jointIndex);
		S32		GetJointIndex() const;
		
		void	SetTargetJointIndex(S32 jointIndex);
		S32		GetTargetJointIndex() const;
	
		void	SetMass(F32 mass);
		F32		GetMass() const;
		
		void	SetAirResistance(F32 ar);
		F32		GetAirResistance() const;
		
		void	SetWindInfluence(F32 f);
		F32		GetWindInfluence() const;
		
		void	SetBuoyancyInfluence(F32 b);
		F32		GetBuoyancyInfluence() const;
	
	protected:
	
		F32		m_oneOverMass;
		F32		m_oneMinusDrag;
		F32 	m_windInfluence;
		F32 	m_buoyancyInfluence;
		S16		m_jointIndex;
		S16		m_targetJointIndex;		// Optional target joint index.
	};	

	/**
		Constant parameters for a dynamic aim joint.
		
		These parameters are stored along with other constraints
		in a skeleton, but can be overridden in a skeleton instance.
		
		@ingroup DynamicJointConstraints
	*/
	class C_DynamicAimJointParams
	{
	public:
		Maths::Normal3	m_aimAxis;
		Maths::Vector3	m_upVectorJointOffset;
		Maths::Normal3	m_upVector;
		Maths::Vector3	m_targetJointOffset;
		F32 			m_minHeading;
		F32 			m_maxHeading;
		F32 			m_minPitch;
		F32 			m_maxPitch;
		F32				m_pitchSpring;
		F32				m_pitchDamping;
		F32				m_headingSpring;
		F32				m_headingDamping;
		F32				m_mass;
		F32				m_drag;
		F32 			m_windInfluence;
		F32 			m_buoyancyInfluence;
	
		S16 			m_jointIndex;
		S16 			m_targetJointIndex;
		S16				m_upVectorJointIndex;
		
		/// Deserialise from a stream.
		template <typename T>
		void Deserialise( T& in, U32 version )
		{
			if (version >= E_SkeletonConstraintsVersion_DynamicAimJointAimAxis)
			{
				in >> m_aimAxis;
			}
			else
			{
				m_aimAxis = Maths::Const<Maths::Normal3>::AxisZ();
			}
			
			in >> m_minHeading;
			in >> m_maxHeading;
			in >> m_minPitch;
			in >> m_maxPitch;
			in >> m_jointIndex;
			in >> m_targetJointIndex;

			if (version >= E_SkeletonConstraintsVersion_DynamicAimJointUpVector)
			{
				in >> m_upVectorJointIndex;
				in >> m_targetJointOffset;
				in >> m_upVectorJointOffset;
				in >> m_upVector;
								
				in >> m_pitchSpring;
				in >> m_pitchDamping;
				in >> m_headingSpring;
				in >> m_headingDamping;
				in >> m_mass;
				in >> m_drag;
			}
			else
			{
				m_targetJointOffset = Maths::Const<Maths::Vector3>::Zero();
				m_upVectorJointOffset = Maths::Const<Maths::Vector3>::Zero();
				m_upVector = Maths::Const<Maths::Normal3>::AxisY();
				m_upVectorJointIndex = m_jointIndex;
				m_pitchSpring = 100.f;
				m_pitchDamping = 0.5f;
				m_headingSpring = 100.f;
				m_headingDamping = 0.5f;
				m_mass = 1.f;
				m_drag = 0.f;
			}
			
			if (version >= E_SkeletonConstraintsVersion_WindAndBuoyancyInfluence)
			{
				in >> m_windInfluence;
				in >> m_buoyancyInfluence;
			}

		}
		
	};
	
	/**
		Drives a joint's Z axis to point at a target joint.
		
		@ingroup DynamicJointConstraints
	*/
	class C_DynamicAimJoint: public C_DynamicJoint
	{
	public:
	
		//
		// Construction.
		//
	
	#ifndef PLATFORM_SPU
		void Initialise( const C_Skeleton& skeleton, const C_StringId& jointName, const C_StringId& targetJointName );
		
		void Initialise( const C_DynamicAimJointParams& params );

		void SetLimits( F32 minHeading, F32 maxHeading, F32 minPitch, F32 maxPitch );
		void GetLimits( F32* outMinHeading, F32* outMaxHeading, F32* outMinPitch, F32* outMaxPitch) const;
		void SetAimAxis( const Maths::Normal3& axis );
		void SetSpringConstants(F32 pitchSpring, F32 pitchDamping, F32 headingSpring, F32 headingDamping);
		void GetSpringConstants(F32* outPitchSpring, F32* outPitchDamping, F32* outHeadingSpring, F32* outHeadingDamping) const;
	#endif

		bool IsValid() const;
		bool IsInvalid() const;
		
		/// Applies a force to the joint. Accumulated until the next call to SimulateAndApply().
		void ApplyForce(const Maths::Vector3& force);
		
		/// Clears any applied force.
		void ClearForce();
	
		void SimulateAndApply( C_SkeletonPose& skeletonPose, const C_DynamicJointContext& context );
				
	private:
	
		void SimulateFollower( const C_DynamicJointContext& context, const C_JointTransform& desiredModelTransform );
		Maths::Vector3 GetModelUpVector( C_SkeletonPose& skeletonPose ) const;
		Maths::Quaternion GetDesiredModelJointRotation( C_SkeletonPose& skeletonPose, const C_DynamicJointContext& context ) const;
		
		// Model-space positions that try to follow the target joint position.
		Maths::Point3		m_targetFollowerPos;
		Maths::Point3		m_prevTargetFollowerPos;
		
		// Aim constraint parameters.
		Maths::Point3		m_targetJointOffset;
		Maths::Point3		m_upVectorJointOffset;
		Maths::Normal3		m_upVector;
		Maths::Normal3		m_aimAxis;
		
		Maths::Vector3		m_force;
		
		Maths::Quaternion	m_prevLocalRotation;
		Maths::Quaternion	m_currLocalRotation;
		Maths::Vector3		m_velocity;

		Maths::Vector4		m_sinPitchHeadingLimits;		// sine of 1/2 minHeading, maxHeading, minPitch, maxPitch
		Maths::Vector4		m_pitchHeadingSpringDamping;	// pitchSpring, headingSpring, pitchDamping, headingDamping
	
		S16 				m_upVectorJointIndex;
	};
	
	/**
		Constant parameters for a dynamic slider joint.
		@ingroup DynamicJointConstraints
	*/
	class C_DynamicSliderJointParams
	{
	public:
		Maths::Vector3 m_axis;
		F32 m_minLimit;
		F32 m_maxLimit;
		F32 m_spring;
		F32 m_damping;
		F32 m_mass;
		F32 m_drag;
		F32 m_windInfluence;
		F32 m_buoyancyInfluence;
		S16 m_jointIndex;

		/// Deserialise from a stream.
		template <typename T>
		void Deserialise( T& in, U32 version )
		{
			in >> m_axis;
			in >> m_minLimit;
			in >> m_maxLimit;
			
			if (version >= E_SkeletonConstraintsVersion_DynamicAimJointUpVector)
			{
				in >> m_spring;
				in >> m_damping;
				in >> m_mass;
				in >> m_drag;
			}
			else
			{
				m_spring = 50.f;
				m_damping = 0.5f;
				m_mass = 1.f;
				m_drag = 0.f;
			}
			
			if (version >= E_SkeletonConstraintsVersion_WindAndBuoyancyInfluence)
			{
				in >> m_windInfluence;
				in >> m_buoyancyInfluence;
			}			
			
			in >> m_jointIndex;
		}

	};

	/**
		A dynamic joint that can slide along a specified local axis. 
		@ingroup DynamicJointConstraints
	*/
	class C_DynamicSliderJoint: public C_DynamicJoint
	{
	public:
		C_DynamicSliderJoint();
			
	#ifndef PLATFORM_SPU
		void Initialise( const C_Skeleton& skeleton, const C_StringId& jointName, const Maths::Normal3& slideAxis );
		
		void Initialise( const C_DynamicSliderJointParams& params );
	#endif

		bool IsValid() const		{ return m_jointIndex >= 0; }
		bool IsInvalid() const		{ return m_jointIndex < 0; }
		
		/// Set limits.
		void SetLimits( F32 minLimit, F32 maxLimit );
		F32 GetMinLimit() const;
		F32 GetMaxLimit() const;
		
		void SetSpringConstants(F32 strength, F32 damping);
		F32 GetSpring() const;
		F32 GetDamping() const;
		
		void ApplyForce(const Maths::Vector3& force);
		void ClearForce();
	
		// Simulate and apply to a C_SkeletonPose.
		void SimulateAndApply( C_SkeletonPose& skeletonPose, const C_DynamicJointContext& context );

		template <typename T>
		void Deserialise( T& stream );
		
	private:
		Maths::Point3 m_position;			// Particle's current position in model space.
		Maths::Point3 m_prevPosition;		// Particle's previous position in model space.
		Maths::Point3 m_prevLocalPosition;	// Previous local space joint position.
		Maths::Point3 m_currLocalPosition;	// Current local space joint position.
		Maths::Normal3 m_axis;				// Axis along which joint can slide.
		Maths::Vector3 m_force;

		F32 m_stiffness;					// Spring stiffness.
		F32 m_damping;						// Spring damping, a fraction of sqrt(stiffness)
	
		F32 m_minLimit;						// Minimum allowed difference from joint's animated position along slide axis.
		F32 m_maxLimit;						// Maximum allowed difference from joint's animated position along slide axis.
	};
	
}

#endif
