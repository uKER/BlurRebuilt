#ifndef ANIMATION_POSEMIRROR_H
#define ANIMATION_POSEMIRROR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

namespace Animation
{
	
	class C_Skeleton;
	class C_Pose;
	
	/// Defines the reflected axis of a mirroring operation.
	enum E_MirrorAxis
	{
		E_MirrorAxis_X	= 0,		/// Reflect X axis.
		E_MirrorAxis_Y,				/// Reflect Y axis.
		E_MirrorAxis_Z				/// Reflect Z axis.
	};
	
	/*
	Mirrors the skeleton joints and trajectory transform in a C_Pose.
	*/
	class ALIGN16 C_PoseMirror
	{
	public:
		/// Default constructor.
		C_PoseMirror();
		
		/// Destructor.
		~C_PoseMirror();
		
		/// Create a default mirroring of the specified skeleton.
		/// Joints beginning with "l_" or "left_" are matched up with their right hand side counterparts.
		void Create( const C_Skeleton& skeleton );
		
		/// Set the mirror axis. Default is E_MirrorAxis_X.
		void SetMirrorAxis( E_MirrorAxis axis );
		
		/// Get the current mirror axis.
		E_MirrorAxis GetMirrorAxis() const;
		
		/// Remove all mirrored joints.
		void Clear();
		
		/// Specify that a single joint is to be mirrored.
		void AddMirroredJoint( S32 jointIndex );
		
		/// Specify a pair of joints to be mirrored and have their transforms swapped.
		void AddMirroredJointPair( S32 leftJointIndex, S32 rightJointIndex );
		
		/// Mirror the current joint transforms and trajectory transform in @a pose.
		void MirrorPose( C_Pose& pose ) const;
		
	private:
		Btl::Vector<S16>	m_jointIndices;
		Maths::Vector3		m_mirrorScale;		// -1 for mirrored axis component, 1 for others.
		Maths::Normal3		m_mirrorNormal;		// Normal of mirror plane (e.g. <1,0,0> for X axis)
		E_MirrorAxis		m_mirrorAxis;
	};
}

#endif
