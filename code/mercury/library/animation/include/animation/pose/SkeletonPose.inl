#ifndef ANIMATION_SKELETONPOSE_INL
#define ANIMATION_SKELETONPOSE_INL

#if !defined(ANIMATION_ALLOW_INCLUDE) && defined(ANIMATION_SHOW_INCLUDES)
#pragma message("******* Including SkeletonPose.inl outside of animation library")
#endif

#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>

namespace Animation
{

	//
	// Skeleton operations.
	//

	inline U32 C_SkeletonPose::GetNumJoints() const
	{
		return m_localJointTransforms.GetSize();
	}
	
	inline const C_Skeleton& C_SkeletonPose::GetSkeleton() const
	{
		return m_skeleton;
	}
	
	//
	// Local space operations.
	//
	
	inline void C_SkeletonPose::SetLocalJointTransform( U32 jointIndex, const C_JointTransform& jointTransform )
	{
		m_localJointTransforms[ jointIndex ] = jointTransform;
	}

	inline const C_JointTransform C_SkeletonPose::GetLocalJointTransform( U32 jointIndex ) const
	{
		return m_localJointTransforms[ jointIndex ];
	}
		
	inline void C_SkeletonPose::SetLocalJointRotation( U32 jointIndex, const Maths::Quaternion& rotation )
	{
		m_localJointTransforms[ jointIndex ].SetRotation( rotation );
	}
		
	inline const Maths::Quaternion C_SkeletonPose::GetLocalJointRotation( U32 jointIndex ) const
	{
		return m_localJointTransforms[ jointIndex ].GetRotation();
	}
		
	inline void C_SkeletonPose::SetLocalJointTranslation( U32 jointIndex, const Maths::Vector3& translation )
	{
		m_localJointTransforms[ jointIndex ].SetTranslation( translation );
	}
				
	inline const Maths::Vector3 C_SkeletonPose::GetLocalJointTranslation( U32 jointIndex ) const
	{
		return m_localJointTransforms[ jointIndex ].GetTranslation();
	}
	
	//
	// Model space operations.
	//
	
	inline void C_SkeletonPose::SetModelJointTransform( U32 jointIndex, const C_JointTransform& jointTransform )
	{
		m_localJointTransforms[ jointIndex ] = jointTransform * GetModelParentJointTransform( jointIndex ).Inverse();
	}

	inline void C_SkeletonPose::SetModelJointRotation( U32 jointIndex, const Maths::Quaternion& rotation )
	{
		m_localJointTransforms[ jointIndex ].SetRotation( rotation * !GetModelParentJointRotation( jointIndex ) );
	}
				
	inline void C_SkeletonPose::SetModelJointTranslation( U32 jointIndex, const Maths::Vector3& translation )
	{
		const C_JointTransform parentModelTransform = GetModelParentJointTransform( jointIndex ).Inverse();
		Maths::Vector3 localTranslation = Maths::MakeVector3( parentModelTransform.TransformPoint( Maths::MakePoint3( translation ) ) );
		m_localJointTransforms[ jointIndex ].SetTranslation( localTranslation );
	}

	inline const Maths::Vector3 C_SkeletonPose::GetModelJointTranslation( U32 jointIndex ) const
	{
		return GetModelJointTransform( jointIndex ).GetTranslation();
	}
}


#endif

