#ifndef ANIMATION_SKELETON_INL
#define ANIMATION_SKELETON_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/JointTransform.inl>

namespace Animation
{

	//
	// C_Skeleton
	//

	FORCE_INLINE U32 C_Skeleton::GetNumJoints() const
	{
		return m_numJoints;
	}

	FORCE_INLINE S32 C_Skeleton::GetJointParentIndex( S32 jointIndex ) const
	{
	#ifdef CFG_DEBUG
		BIZ_ASSERT(jointIndex >= 0 && jointIndex < S32(m_numJoints));
	#endif
		return m_parentIndices[jointIndex];
	}
	
	FORCE_INLINE S32 C_Skeleton::GetFirstParentedJointIndex() const
	{
		return m_firstParentedJointIndex;
	}

#ifndef PLATFORM_SPU
	inline const C_StringId& C_Skeleton::GetJointName( U32 index ) const
	{
		return m_jointNames[ index ];
	}
#endif
	
	FORCE_INLINE const C_JointTransform C_Skeleton::GetLocalBasePoseJointTransform( U32 jointIndex ) const
	{
		BIZ_ASSERT( jointIndex < m_numJoints );
		return m_basePoseTransforms[ jointIndex ];
	}
	
	FORCE_INLINE const Maths::Quaternion C_Skeleton::GetLocalBasePoseJointRotation( U32 jointIndex ) const
	{
	#ifdef CFG_DEBUG
		BIZ_ASSERT( jointIndex < m_numJoints );
	#endif
		return m_basePoseTransforms[ jointIndex ].GetRotation();
	}

	FORCE_INLINE const Maths::Vector3 C_Skeleton::GetLocalBasePoseJointTranslation( U32 jointIndex ) const
	{
	#ifdef CFG_DEBUG
		BIZ_ASSERT( jointIndex < m_numJoints );
	#endif
		return m_basePoseTransforms[ jointIndex ].GetTranslation();
	}
	
	inline const C_JointTransform* C_Skeleton::GetLocalBasePoseJointTransformArray() const
	{
		return m_basePoseTransforms;
	}
		
	inline Maths::Mat43 C_Skeleton::GetInverseWorldBasePoseMatrix( U32 jointIndex ) const
	{
		BIZ_ASSERT( jointIndex < m_numJoints );
		return m_invWorldBasePoseMatrices[ jointIndex ];
	}
	
	inline const C_SkeletonConstraints& C_Skeleton::GetConstraints() const
	{
		return m_constraints;
	}



}

#endif
