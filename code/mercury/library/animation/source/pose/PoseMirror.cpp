#include "stdafx.h"
#include <animation/pose/PoseMirror.h>
//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFields.h>
#include <animation/pose/PoseFields.inl>
#include <animation/skeleton/Skeleton.h>
#include <btl/containers/bitvector.h>
#include <system/scheduler.h>

namespace Animation
{

// Preserves magnitude of Q.
inline Maths::Quaternion ReflectQuat( const Maths::Quaternion& q, const Maths::Vector3& mirrorScale )
{
	Maths::Vector3 mirroredImag = Maths::Imag( q ) * -mirrorScale;
	return Maths::MakeQuaternion( Maths::MakeVec4F32( mirroredImag, Maths::Real( q ) ) );
}

// Preserves weights.
inline void MirrorJointTransform( Animation::C_JointTransform& transform, const Maths::Vector3& mirrorScale )
{
	const Maths::Quaternion q = transform.GetRotation();
	const Maths::Quaternion mirroredQ = ReflectQuat( q, mirrorScale );
		
	const Maths::Vec4F32 t = transform.GetTranslationField().GetValueAndWeight();
	const Maths::Vector3 mirroredT = Maths::MakeVector3(t) * mirrorScale;
		
	transform.GetRotationField() = mirroredQ;
	transform.GetTranslationField() = Maths::MakeVec4F32( mirroredT, Maths::W(t) );
}

//
// C_PoseMirror
//

#ifndef PLATFORM_SPU

C_PoseMirror::C_PoseMirror()
{
	SetMirrorAxis( E_MirrorAxis_X );
}

C_PoseMirror::~C_PoseMirror()
{
}

void C_PoseMirror::Create( const C_Skeleton& skeleton )
{
	Clear();
		
	Btl::Vector< Util::C_String > leftPrefixes;
	Btl::Vector< Util::C_String > rightPrefixes;
	Btl::Vector< Util::C_String > nonMirroredJoints;
	
	leftPrefixes.PushBack( Util::C_String("l_") );
	leftPrefixes.PushBack( Util::C_String("left") );
	rightPrefixes.PushBack( Util::C_String("r_") );
	rightPrefixes.PushBack( Util::C_String("right") );
	
	U32 numJoints = skeleton.GetNumJoints();
	
	Util::C_String leftJointName;
	Util::C_String rightJointName;
	Util::C_String suffix;
	Util::C_String leftPrefix;
	Util::C_String rightPrefix;
	
	const Maths::VF32 kEpsilon = Maths::MakeVF32( 0.01f );
	
	Btl::BitVector mirrorFlags;
	mirrorFlags.Resize( numJoints );
	mirrorFlags.ClearBits();
	
	for ( U32 i = 0; i < numJoints; ++i )
	{
		bool mirror = false;
		S32 leftJointIndex = i;
		S32 rightJointIndex = -1;
		
		const Maths::Vector3 localPos = skeleton.GetLocalBasePoseJointTranslation( leftJointIndex );
		const Maths::VF32 distFromMirrorPlane = localPos | m_mirrorNormal;
		
		if ( Maths::Abs( distFromMirrorPlane ) < kEpsilon )
		{
			// on sagittal plane - mirror it regardless.
			mirror = true;
		}
								
		leftJointName = skeleton.GetJointName(i).GetString();
		
		for ( S32 prefixIndex = 0; prefixIndex < leftPrefixes.Size() && rightJointIndex < 0; ++prefixIndex )
		{
			leftPrefix = leftPrefixes[ prefixIndex ];
			rightPrefix = rightPrefixes[ prefixIndex ];
		
			if ( leftJointName.ISearch( leftPrefix, 0 ) == 0 )
			{
				rightJointName = rightPrefix + leftJointName( leftPrefix.Length(), leftJointName.Length() );
	
				// Find corresponding joint with right prefix.
				for ( U32 j = 0; j < numJoints && rightJointIndex < 0; ++j )
				{
					if ( j != leftJointIndex )
					{
						if ( rightJointName == skeleton.GetJointName( j ).GetString() )
						{
							// Found it.
							rightJointIndex = j;
						}
					}
				}
			}
		}
		
		if ( rightJointIndex >= 0 )
		{
			// Add to list.
			AddMirroredJointPair( leftJointIndex, rightJointIndex );
			mirrorFlags.SetBit( leftJointIndex );
			mirrorFlags.SetBit( rightJointIndex );
			
			// Recursively ensure that parent joints are also mirrored.
			S32 parentIndex = skeleton.GetJointParentIndex( leftJointIndex );
			while ( parentIndex >= 0 )
			{
				if ( !mirrorFlags.GetBitValue( parentIndex ) )
				{
					mirrorFlags.SetBit( parentIndex );
					AddMirroredJoint( parentIndex );
				}
				parentIndex = skeleton.GetJointParentIndex( parentIndex );
			}
			parentIndex = skeleton.GetJointParentIndex( rightJointIndex );
			while ( parentIndex >= 0 )
			{
				if ( !mirrorFlags.GetBitValue( parentIndex ) )
				{
					mirrorFlags.SetBit( parentIndex );
					AddMirroredJoint( parentIndex );
				}
				parentIndex = skeleton.GetJointParentIndex( parentIndex );
			}
		}
		else if ( mirror && !mirrorFlags.GetBitValue(leftJointIndex) )
		{
			mirrorFlags.SetBit( leftJointIndex );
			AddMirroredJoint( leftJointIndex );
		}
	}
}

void C_PoseMirror::SetMirrorAxis( E_MirrorAxis axis )
{
	m_mirrorAxis = axis;
	
	const Maths::Vector3 one = Maths::Const<Maths::Vector3>::One();
	Maths::Normal3 normal;
	Maths::Vector3 scale;
	switch ( axis )
	{
		default:
		case E_MirrorAxis_X:
			normal = Maths::Const<Maths::Normal3>::AxisX();
			scale = Maths::NegateX( one );
			break;
		case E_MirrorAxis_Y:
			normal = Maths::Const<Maths::Normal3>::AxisY();
			scale = Maths::NegateY( one );
			break;
		case E_MirrorAxis_Z:
			normal = Maths::Const<Maths::Normal3>::AxisZ();
			scale = Maths::NegateZ( one );
			break;
	}
	
	m_mirrorNormal = normal;
	m_mirrorScale = scale;
}

E_MirrorAxis C_PoseMirror::GetMirrorAxis() const
{
	return m_mirrorAxis;
}

void C_PoseMirror::Clear()
{
	m_jointIndices.Clear();
}

void C_PoseMirror::AddMirroredJoint( S32 jointIndex )
{
	if ( jointIndex < 0 ) return;
	m_jointIndices.PushBack( -(jointIndex + 1) );
}

void C_PoseMirror::AddMirroredJointPair( S32 leftJointIndex, S32 rightJointIndex )
{
	if ( leftJointIndex >= 0 && rightJointIndex >= 0 )
	{
		m_jointIndices.PushBack( leftJointIndex );
		m_jointIndices.PushBack( rightJointIndex );
	}
	else
	{
		AddMirroredJoint( leftJointIndex );
		AddMirroredJoint( rightJointIndex );
	}
}

#endif

void C_PoseMirror::MirrorPose( C_Pose& pose ) const
{
	U32 numJointIndices = m_jointIndices.Size();
	if ( numJointIndices == 0 )
	{
		return;
	}
	
	Spu::MemBaseMarker memBase;
	
	const S16* jointIndices = (const S16*) Spu::FetchAllocUnaligned( &m_jointIndices[0], sizeof(S16) * numJointIndices );
	
	U32 i = 0;
	const Maths::Vector3 mirrorScale = m_mirrorScale;
	
	while ( i < numJointIndices )
	{
		if ( jointIndices[ i ] < 0 )
		{
			// Mirror single joint.
			S32 jointIndex = -(jointIndices[i] + 1);
			++i;
			MirrorJointTransform( pose.GetJointTransform( jointIndex ), mirrorScale );
		}
		else
		{
			// Mirror and swap a pair of joints.
			S32 leftJointIndex = jointIndices[i];
			S32 rightJointIndex = jointIndices[i + 1];
			i += 2;
			
			C_JointTransform& leftTransform = pose.GetJointTransform( leftJointIndex );
			C_JointTransform& rightTransform = pose.GetJointTransform( rightJointIndex );
			C_JointTransform leftTransformCopy = leftTransform;
			MirrorJointTransform( leftTransformCopy, mirrorScale );
			MirrorJointTransform( rightTransform, mirrorScale );
			leftTransform = rightTransform;
			rightTransform = leftTransformCopy;
		}
	}
	
	// Finally, reflect the trajectory transform and its delta.
	if ( pose.HasTrajectoryTransform() )
	{
		MirrorJointTransform( pose.GetTrajectoryTransform(), mirrorScale );
	}
	if ( pose.HasTrajectoryDeltaTransform() )
	{
		MirrorJointTransform( pose.GetTrajectoryDeltaTransform(), mirrorScale );
	}
}


} // end Animation
