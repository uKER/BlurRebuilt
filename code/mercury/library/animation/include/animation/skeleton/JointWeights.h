#ifndef ANIMATION_JOINTWEIGHTS_H
#define ANIMATION_JOINTWEIGHTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/bitvector.h>

namespace Animation
{

//
/// \brief Describes which joints will be used in a blending operation.
///
/// \see JointWeightsUtils for additional helper functions.
/// \see C_Cycle for examples of joint masks being used during blending.
///
/// \todo In the future, this class could be extended to store a fractional weight for each
/// joint to allow "feathered" blending of animations. 
//
class C_JointWeights
{
public:
	/// Constructs a joint mask for the specified number of joints.
	/// All joints will be enabled by default.
	C_JointWeights( U32 numJoints );

	/// Destructor.
	//~C_JointWeights();
	
	/// Enable all joints.
	void EnableAllJoints();

	/// Disable all joints.
	void DisableAllJoints();

	/// Enable a specific joint.
	void EnableJoint( S32 jointIndex );
	
	/// Disable a specific joint.
	void DisableJoint( S32 jointIndex );

	/// Returns true if the specified joint is enabled.
	bool IsJointEnabled( S32 jointIndex ) const;
		
	/// Returns the number of enabled joints.
	S32 GetNumEnabledJoints() const;

	/// Returns the number of joints in the mask.
	S32 GetNumJoints() const;

private:
	Btl::BitVector m_jointBits;
};

class C_Skeleton;

///
/// \brief Utility functions for working with joint masks.
///
///
namespace JointWeightUtils
{
	bool IsJointDescendedFrom( const C_Skeleton& skeleton, S32 jointIndex, S32 potentialParentIndex );

	/// Enable all joints in a branch of a skeleton.
	/// \param skeleton The skeleton used by the joint mask.
	/// \param rootJointIndex Index of the first joint in the branch.
	/// \param JointWeights The joint mask to modify.
	void EnableBranch( const C_Skeleton& skeleton, S32 rootJointIndex, C_JointWeights& JointWeights );
	
	/// Disable all joints in a branch of a skeleton.
	/// \param skeleton The skeleton used by the joint mask.
	/// \param rootJointIndex Index of the first joint in the branch.
	/// \param JointWeights The joint mask to modify.
	void DisableBranch( const C_Skeleton& skeleton, S32 rootJointIndex, C_JointWeights& JointWeights );
}

} // end Animation

#endif
