#include "stdafx.h"
#include <animation/skeleton/JointWeights.h>
#include <animation/skeleton/Skeleton.h>

namespace Animation
{

//
// C_JointWeights.
//

C_JointWeights::C_JointWeights( U32 numJoints )
{
	m_jointBits.Resize( numJoints );
}

void C_JointWeights::EnableAllJoints()
{
	m_jointBits.SetBits();
}

void C_JointWeights::DisableAllJoints()
{
	m_jointBits.ClearBits();
}

void C_JointWeights::EnableJoint( S32 jointIndex )
{
	m_jointBits.SetBit( jointIndex );
}

void C_JointWeights::DisableJoint( S32 jointIndex )
{
	m_jointBits.ClearBit( jointIndex );
}

bool C_JointWeights::IsJointEnabled( S32 jointIndex ) const
{
	return m_jointBits.GetBitValue( jointIndex );
}

S32 C_JointWeights::GetNumEnabledJoints() const
{
	S32 numEnabledJoints = 0;
	for ( S32 i = 0, count = GetNumJoints(); i < count; ++i )
	{
		numEnabledJoints += IsJointEnabled( i ) ? 1 : 0;
	}
	return numEnabledJoints;
}

S32 C_JointWeights::GetNumJoints() const
{
	return m_jointBits.Size();
}

//
// JointWeightUtils.
//

bool JointWeightUtils::IsJointDescendedFrom( const C_Skeleton& skeleton, S32 jointIndex, S32 potentialParentIndex )
{
	BIZ_ASSERT( jointIndex >= 0 );
	S32 j = jointIndex;
	while ( j > potentialParentIndex )
	{
		j = skeleton.GetJointParentIndex( j );
	}
	return j == potentialParentIndex;
}

void JointWeightUtils::EnableBranch( const C_Skeleton& skeleton, S32 rootJointIndex, C_JointWeights& jointWeights )
{
	if ( rootJointIndex < 0 )
	{
		jointWeights.EnableAllJoints();
		return;
	}
	
	S32 firstJointIndex = rootJointIndex;
	for ( S32 j = firstJointIndex, numJoints = skeleton.GetNumJoints(); j < numJoints; ++j )
	{
		if ( IsJointDescendedFrom( skeleton, j, rootJointIndex ) )
		{
			jointWeights.EnableJoint( j );
		}
	}

}

void JointWeightUtils::DisableBranch( const C_Skeleton& skeleton, S32 rootJointIndex, C_JointWeights& jointWeights )
{
	if ( rootJointIndex < 0 )
	{
		jointWeights.DisableAllJoints();
		return;
	}
	
	S32 firstJointIndex = rootJointIndex;
	for ( S32 j = firstJointIndex, numJoints = skeleton.GetNumJoints(); j < numJoints; ++j )
	{
		if ( IsJointDescendedFrom( skeleton, j, rootJointIndex ) )
		{
			jointWeights.DisableJoint( j );
		}
	}
}

} // namespace Animation
