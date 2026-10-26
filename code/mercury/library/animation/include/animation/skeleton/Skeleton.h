#ifndef ANIMATION_SKELETON_H
#define ANIMATION_SKELETON_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/StringId.h>
#include <animation/common/JointTransform.h>
#include <animation/common/MemoryBlock.h>
#include "SkeletonConstraints.h"

namespace Animation
{

	class C_JointConstraint;
	class C_StringId;

	///
	/// A hierarchy of named joints with a reference base pose.
	///
	class ALIGN16 C_Skeleton
	{
	public:
		friend class C_Pose;
		
		C_Skeleton();
				
		~C_Skeleton();
		
		void Destroy();
		
		/// Set name of skeleton, for debugging.
		void SetName( const CHAR8* name );
		
		/// Get name of skeleton, for debugging.
		const C_StringId& GetName() const;

		void SpuFetchHierarchy() const;
		void SpuFetchHierarchyAsync(U32 tag) const;
		void SpuFetchLocalBasePose() const;
		void SpuFetchLocalBasePoseAsync(U32 tag) const;
		void SpuFetchInverseWorldBasePose() const;
		void SpuFetchInverseWorldBasePoseAsync(U32 tag) const;
		
		/// Returns the number of joints in the skeleton.
		U32 GetNumJoints() const;

		/// Get index of a joint's parent. Returns -1 if none exists.
		S32 GetJointParentIndex( S32 jointIndex ) const;
		
		/// Get index of first parented joint, i.e. the first joint whose parent index is greater than or equal to zero.
		S32 GetFirstParentedJointIndex() const;

		/// Get name of joint as a string ID. PPU only.
		const C_StringId& GetJointName( U32 index ) const;
		
		// TODO: add this function.
		//S32 GetJointIndex( const C_StringId& name ) const;
		
		/// Searches for a joint by name. PPU only.
		/// \returns Index of matching joint or -1 if none found.
		/// \todo CHANGE TO STRINGID
		S32 FindJointByName( const CHAR8* name ) const;
		
		/// Searches for a joint by name. PPU only.
		/// \param name The name of the joint as a C_StringId.
		/// \returns Index of matching joint or -1 if none found.
		
		S32 FindJointByName( const C_StringId& name ) const;
				
		/// Test to see if a joint is a descendant of another joint.
		bool IsDescendant( S32 jointIndex, S32 potentialDescendantJointIndex ) const;
		
		//
		// Base pose.
		//
		
		/// Get local space joint base pose transform.
		const C_JointTransform GetLocalBasePoseJointTransform( U32 jointIndex ) const;

		/// Get local space joint base pose rotation.
		const Maths::Quaternion GetLocalBasePoseJointRotation( U32 jointIndex ) const;

		/// Get local space joint base pose translation.
		const Maths::Vector3 GetLocalBasePoseJointTranslation( U32 jointIndex ) const;

		/// Get inverse world base pose matrix for skinning.
		Maths::Mat43 GetInverseWorldBasePoseMatrix( U32 jointIndex ) const;

		/// Get model space joint base pose transform. Note that this is computed by concatenating local space transforms.
		C_JointTransform GetModelBasePoseJointTransform( U32 jointIndex ) const;
		
	private:
		/// Returns a pointer to the start of the array of local space joint transforms.
		const C_JointTransform* GetLocalBasePoseJointTransformArray() const;
		
	public:
	
		//
		// Constraints.
		//
		
		/// Get skeleton constraints.
		const C_SkeletonConstraints& GetConstraints() const;
		
		/// Returns true if skeleton has constraints and therefore needs a C_SkeletonInstance object.
		bool HasConstraints() const;
		
		//
		// Serialisation.
		//
		
		/// Load from a binary stream.
		template < typename T >
		bool Deserialise( T& in );

	private:
		U16								m_numJoints;					///< Number of joints.
		S16								m_firstParentedJointIndex;		///< Index of first joint that has a parent.
		S16*							m_parentIndices;
		C_JointTransform*				m_basePoseTransforms;
		Maths::Mat43*					m_invWorldBasePoseMatrices;
		C_StringIdArray					m_jointNames;
		C_SkeletonConstraints			m_constraints;
		C_StringId						m_name;
	};

	///
	///
	///
	class C_SkeletonMapping
	{
	public:
		C_SkeletonMapping();
		~C_SkeletonMapping();
		
		bool Create(
			const C_Skeleton& skeletonA,
			const C_Skeleton& skeletonB
		);
		
		// Assumptions
		//	- fromTransforms has same skeleton as m_fromSkeleton
		//  - toTransforms has same skeleton as m_toSkeleton
		//  - how are joints in toSkeleton not fromSkeleton handled? toSkeleton base pose.
		// *** FIXME: Replace this with a function that takes Poses/Evaluations
		void MapTransforms(
			const C_JointTransform* transformsA, 
			C_JointTransform* transformsB
		) const;
		
		//S32 MapJointIndexFromAToB( U32 jointIndexA ) const;
		S32 MapJointIndexFromBToA( U32 jointIndexB ) const;
		
		
	private:
		const C_Skeleton* m_skeletonA;
		const C_Skeleton* m_skeletonB;
		S16* m_jointMappingFromBToA;
	};

} // end Animation

#include "Skeleton.inl"

#endif

