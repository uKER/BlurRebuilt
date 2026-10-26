#include "stdafx.h"
#include <animation/skeleton/SkeletonConstraints.h>
#include <animation/skeleton/DynamicJointConstraints.h>
#include <animation/skeleton/StaticJointConstraints.h>
#include <system/Scheduler.h>

namespace Animation
{

//
// Construction.
//

C_SkeletonConstraints::C_SkeletonConstraints():
	m_numConstraints(0)
{
	for ( U32 i = 0; i < E_ConstraintType_Count; ++i )
	{
		m_numConstraintsOfType[ i ] = 0;
	}
}

C_SkeletonConstraints::~C_SkeletonConstraints()
{
}

#if !PLATFORM_SPU
void C_SkeletonConstraints::InitialiseDynamicJoints(
	const C_ArrayProxy<C_DynamicAimJoint>& dynamicAimJoints,
	const C_ArrayProxy<C_DynamicSliderJoint>& dynamicSliderJoints
) const
{
	for ( U32 i = 0, count = GetNumConstraintsByType( E_ConstraintType_DynamicAimJoint ); i < count; ++i )
	{
		dynamicAimJoints[ i ].Initialise( m_dynamicAimJointParams(m_constraintData)[ i ] );
	}
	
	for ( U32 i = 0, count = GetNumConstraintsByType( E_ConstraintType_DynamicSliderJoint ); i < count; ++i )
	{
		dynamicSliderJoints[ i ].Initialise( m_dynamicSliderJointParams(m_constraintData)[ i ] );
	}
}
#endif

void C_SkeletonConstraints::Clear()
{
	m_constraintData.Destroy();
	
	for ( U32 i = 0; i < E_ConstraintType_Count; ++i )
	{
		m_numConstraintsOfType[ i ] = 0;
	}
	m_numConstraints = 0;
	m_numDrivenKeys = 0;
}

void C_SkeletonConstraints::ApplyConstraints(
	C_SkeletonPose& skeletonPose,
	const C_ArrayProxy<C_DynamicAimJoint>& dynamicAimJoints,
	const C_ArrayProxy<C_DynamicSliderJoint>& dynamicSliderJoints,
	const C_DynamicJointContext& dynamicJointContext
) const
{
	Spu::MemBaseMarker memBaseMarker;

	// TODO: don't need to fetch the whole lot - just the static constraint data.
	m_constraintData.SpuFetch();
	
	const C_ArrayProxy<const C_DrivenKey> drivenKeys( m_drivenKeys(m_constraintData), m_numDrivenKeys );
	
	const C_ConstraintInfo* constraints = m_constraints(m_constraintData);
	
	for ( U32 i = 0, count = GetNumConstraints(); i < count; ++i )
	{
		const U32 index = constraints[ i ].index;
		switch ( constraints[ i ].type )
		{
			case E_ConstraintType_Aim:
				m_aimConstraints(m_constraintData)[ index ].Apply( skeletonPose );
				break;
				
			case E_ConstraintType_Orientation:
				m_orientationConstraints(m_constraintData)[ index ].Apply( skeletonPose );
				break;
				
			case E_ConstraintType_Position:
				m_positionConstraints(m_constraintData)[ index ].Apply( skeletonPose );
				break;
				
			case E_ConstraintType_DrivenKeyRotation:
				m_drivenKeyRotationConstraints(m_constraintData)[ index ].Apply( skeletonPose, drivenKeys );
				break;
				
			case E_ConstraintType_DrivenKeyTranslation:
				m_drivenKeyTranslationConstraints(m_constraintData)[ index ].Apply( skeletonPose, drivenKeys );
				break;
				
			case E_ConstraintType_DynamicSliderJoint:
				dynamicSliderJoints[ index ].SimulateAndApply( skeletonPose, dynamicJointContext );
				break;
				
			case E_ConstraintType_DynamicAimJoint:
				dynamicAimJoints[ index ].SimulateAndApply( skeletonPose, dynamicJointContext );
				break;
				
			case E_ConstraintType_Custom:
				// Custom constraint breakpoint - return ID to caller.
				break;
		}
	}
}

} // namespace Animation
