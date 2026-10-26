#ifndef ANIMATION_SKELETONPOSE_H
#define ANIMATION_SKELETONPOSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/pose/Pose.h>
#include <animation/common/ArrayProxy.h>

//
// Include SkeletonPose.inl for inline functions.
//

namespace Animation
{
	class C_Pose;
	class C_Skeleton;
	class C_JointTransform;
		
	/**
		Functional interface for working with a character skeleton inside a C_Pose.
		
		 All joints are assumed to be stored in local space. Model space transforms are
		 calculated on demand as profiling revealed that this is considerably more
		 efficient than caching using flags or other methods.
		
		@ingroup PoseFunctions
						
		@note It is currently assumed that the joint transform fields of the specified
		pose exactly map onto the joints of the specified skeleton. In the future,
		multiple skeletons may be required inside a single pose and this class
		will require an externally managed skeleton to pose layout mapping.
	*/
	class C_SkeletonPose
	{
	public:
		/// Construct a skeleton pose.
		C_SkeletonPose( const C_Skeleton& skeleton, C_Pose& pose );
		
		//
		// Skeleton related operations.
		//
		
		/// Get number of joints in the skeleton.
		U32 GetNumJoints() const;
		
		/// Get skeleton.
		const C_Skeleton& GetSkeleton() const;
		
		/// Set pose to base pose.
		void ClearToBasePose();
				
		//
		// Local space operations.
		//

		/// Set local joint transform.
		void SetLocalJointTransform( U32 jointIndex, const C_JointTransform& jointTransform );

		/// Get local joint transform.
		const C_JointTransform GetLocalJointTransform( U32 jointIndex ) const;	
		
		/// Set local space joint rotation.
		void SetLocalJointRotation( U32 jointIndex, const Maths::Quaternion& rotation );
		
		/// Get local space joint rotation.
		const Maths::Quaternion GetLocalJointRotation( U32 jointIndex ) const;
		
		/// Set local space joint translation.
		void SetLocalJointTranslation( U32 jointIndex, const Maths::Vector3& translation );
				
		/// Get local space joint translation.
		const Maths::Vector3 GetLocalJointTranslation( U32 jointIndex ) const;
		
	
	public:

		//
		// Model space operations.
		//

		/// Set joint local to model transform.
		void SetModelJointTransform( U32 jointIndex, const C_JointTransform& jointTransform );
		
		/// Returns a normalised model space transform, useful if the underlying pose has not been normalised.
		C_JointTransform GetNormalisedModelJointTransform( U32 jointIndex ) const;

		/// Get joint local to model transform. 
		const C_JointTransform GetModelJointTransform( U32 jointIndex ) const;
		
		/// Set joint model space rotation. The change in rotation is propogated down the joint hierarchy.
		void SetModelJointRotation( U32 jointIndex, const Maths::Quaternion& rotation );
		
		/// Get joint model space rotation.
		const Maths::Quaternion GetModelJointRotation( U32 jointIndex ) const;
				
		/// Set joint model space translation. The change in translation is propogated down the joint hierarchy.
		void SetModelJointTranslation( U32 jointIndex, const Maths::Vector3& translation );

		/// Get joint model space translation.
		const Maths::Vector3 GetModelJointTranslation( U32 jointIndex ) const;
		
		/// Calculate and copy model space joint transforms into an external array.
		void GetModelJointTransforms( C_JointTransform* destTransforms, U32 numDestTransforms ) const;
		
		/// Convert local space joint transforms to local space matrices.
		/// @numDestMatrices should be the same size as the number of joints in the skeleton.
		void GetLocalJointMatrices( Maths::Mat43* destMatrices, U32 numDestMatrices ) const;

		/// Convert local space joint transforms into model space.
		/// @a numDestMatrices should be the same size as the number of joints in the skeleton.
		void GetModelJointMatrices( Maths::Mat43* destMatrices, Animation::C_JointTransform* tempModelJointTransforms, U32 numDestMatrices ) const;
		
		/// Convert local space joint transforms into world space.
		/// @a numDestMatrices should be the same size as the number of joints in the skeleton.
		void GetWorldJointMatrices( Maths::Mat43* destMatrices, Animation::C_JointTransform* tempModelJointTransforms, U32 numDestMatrices, const Maths::Mat43& modelWorldMatrix ) const;
		
		/// Converts each local space joint transform into model space, premultiplied by inverse base pose matrices.
		void GetSkinningMatrices( Maths::Mat43* destMatrices, Animation::C_JointTransform* tempModelJointTransforms, U32 numDestMatrices) const;
		
		/// Get model space joint rotation of the parent of joint @a jointIndex.
		const Maths::Quaternion GetModelParentJointRotation( U32 jointIndex ) const;
		
		/// Get model space joint transform of the parent of joint @a jointIndex.
		const C_JointTransform GetModelParentJointTransform( U32 jointIndex ) const;

	private:
		// Not to be implemented.			
		C_SkeletonPose& operator = ( const C_SkeletonPose& rhs );
			
	private:
		const C_Skeleton& m_skeleton;
		C_Pose& m_pose;
		C_ArrayProxy<C_JointTransform> m_localJointTransforms;
	};
	
	
}

#endif
