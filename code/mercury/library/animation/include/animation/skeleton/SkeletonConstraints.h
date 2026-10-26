#ifndef ANIMATION_SKELETONCONSTRAINTS_H
#define ANIMATION_SKELETONCONSTRAINTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/MemoryBlock.h>
#include <animation/common/ArrayProxy.h>
#include "ConstraintTypes.h"

namespace Animation
{

	class C_SkeletonPose;
	class C_DynamicJointConstraints;
	class C_DynamicJointContext;
	class C_JointTransform;
	
	class C_AimConstraint;
	class C_OrientationConstraint;
	class C_PositionConstraint;
	class C_DrivenKey;
	class C_DrivenKeyRotationConstraint;
	class C_DrivenKeyTranslationConstraint;
	class C_DynamicAimJoint;
	class C_DynamicAimJointParams;
	class C_DynamicSliderJoint;
	class C_DynamicSliderJointParams;

	/**
		Joint constraints to be applied to a skeleton after it has been posed.
	*/
	class C_SkeletonConstraints
	{
	public:
		//
		// Construction.
		//
		
		C_SkeletonConstraints();
		~C_SkeletonConstraints();
		
		/// Initialise dynamic joints using constant parameters.
		void InitialiseDynamicJoints( 
			const C_ArrayProxy<C_DynamicAimJoint>& dynamicAimJoints,
			const C_ArrayProxy<C_DynamicSliderJoint>& dynamicSliderJoints
		) const;
		
	private:	
		void Clear();

	public:		
		//
		// Queries.
		//
		
		U32 GetNumConstraints() const;
		
		U32 GetNumConstraintsByType( E_ConstraintType type ) const;
		
		
	public:
	
		//
		// Evaluation.
		//
	
		void ApplyConstraints(
			C_SkeletonPose& skeletonPose,
			const C_ArrayProxy<C_DynamicAimJoint>& dynamicAimJoints,
			const C_ArrayProxy<C_DynamicSliderJoint>& dynamicSliderJoints,
			const C_DynamicJointContext& dynamicJointContext
		) const;
		
		//
		// Serialisation.
		//
		
		template <typename T>
		void Deserialise( T& stream );
	
	private:
	
		struct C_ConstraintInfo
		{
			U8 type;	//< Constraint type.
			U8 index;	//< Index of constraint in array of constraints of same type.
		};
		
		C_MemoryBlock	m_constraintData;
		U16				m_numDrivenKeys;
		U16				m_numConstraints;
		
		C_RelativePtr<C_ConstraintInfo>					m_constraints; // evaluation sequence.
		C_RelativePtr<C_AimConstraint>					m_aimConstraints;
		C_RelativePtr<C_OrientationConstraint>			m_orientationConstraints;
		C_RelativePtr<C_PositionConstraint>				m_positionConstraints;
		C_RelativePtr<C_DrivenKey>						m_drivenKeys;
		C_RelativePtr<C_DrivenKeyRotationConstraint>	m_drivenKeyRotationConstraints;
		C_RelativePtr<C_DrivenKeyTranslationConstraint> m_drivenKeyTranslationConstraints;
		C_RelativePtr<C_DynamicAimJointParams>			m_dynamicAimJointParams;
		C_RelativePtr<C_DynamicSliderJointParams>		m_dynamicSliderJointParams;
		
		U8 m_numConstraintsOfType[ E_ConstraintType_Count ];
	};
}

//
// SkeletonConstraints.inl
//

namespace Animation
{
	inline U32 C_SkeletonConstraints::GetNumConstraints() const
	{
		return m_numConstraints;
	}
	
	inline U32 C_SkeletonConstraints::GetNumConstraintsByType( E_ConstraintType type ) const
	{
		return m_numConstraintsOfType[ type ];
	}
}

#endif

