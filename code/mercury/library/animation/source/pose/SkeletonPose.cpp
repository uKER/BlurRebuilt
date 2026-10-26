#include "stdafx.h"
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include <animation/skeleton/Skeleton.h>
#include "pose/Pose.inl"

namespace Animation
{

C_SkeletonPose::C_SkeletonPose( const C_Skeleton& skeleton, C_Pose& pose ):
	m_skeleton(skeleton),
	m_pose(pose),
	m_localJointTransforms(&pose.GetJointTransform(0), Maths::Min(skeleton.GetNumJoints(), pose.GetNumJointTransforms()))
{
}

//
// Skeleton related operations.
//
		
void C_SkeletonPose::ClearToBasePose()
{
	C_JointTransform* localJointTransforms = &m_localJointTransforms[ 0 ];
	const C_Skeleton& skeleton = m_skeleton;
	const U32 numJoints = GetNumJoints();
	for ( U32 i = 0; i < numJoints; ++i )
	{
		localJointTransforms[ i ] = skeleton.GetLocalBasePoseJointTransform( i );
	}
}

//
// Model space operations.
//

C_JointTransform C_SkeletonPose::GetNormalisedModelJointTransform( U32 jointIndex ) const
{
	const C_Skeleton& skeleton = m_skeleton;
	const C_JointTransform* localJointTransforms = &m_localJointTransforms[ 0 ];

	S32 parentIndex = skeleton.GetJointParentIndex( jointIndex );

	BIZ_ASSERT(jointIndex < m_localJointTransforms.GetSize());
	
	C_JointTransform transform = localJointTransforms[ jointIndex ];
	transform.Normalise(skeleton.GetLocalBasePoseJointTransform(jointIndex));
	Maths::Quaternion q = transform.GetRotation();
	Maths::Vector3 t = transform.GetTranslation();
	Maths::Vector3 s = transform.GetScale();
	
	while ( parentIndex >= 0 )
	{
		C_JointTransform rhs = localJointTransforms[ parentIndex ];
		rhs.Normalise(skeleton.GetLocalBasePoseJointTransform(parentIndex));
		const Maths::Quaternion qRhs = rhs.GetRotation();
		const Maths::Vector3 tRhs = rhs.GetTranslation();
		const Maths::Vector3 sRhs = rhs.GetScale();
		
		q = q * qRhs;
		t = t * qRhs + tRhs;
		s = s * sRhs;

		parentIndex = skeleton.GetJointParentIndex( parentIndex );
	}
	
	return C_JointTransform( q, t, s );

}

const C_JointTransform C_SkeletonPose::GetModelJointTransform( U32 jointIndex ) const
{
	const C_Skeleton& skeleton = m_skeleton;
	const C_JointTransform* localJointTransforms = &m_localJointTransforms[ 0 ];

	S32 parentIndex = skeleton.GetJointParentIndex( jointIndex );

	BIZ_ASSERT(jointIndex < m_localJointTransforms.GetSize());
	
	C_JointTransform transform = localJointTransforms[ jointIndex ];
	
	Maths::Quaternion q = transform.GetRotation();
	Maths::Vector3 t = transform.GetTranslation();
	Maths::Vector3 s = transform.GetScale();
	
	while ( parentIndex >= 0 )
	{
		C_JointTransform rhs = localJointTransforms[ parentIndex ];
		const Maths::Quaternion qRhs = rhs.GetRotation();
		const Maths::Vector3 tRhs = rhs.GetTranslation();
		const Maths::Vector3 sRhs = rhs.GetScale();
		
		q = q * qRhs;
		t = t * qRhs + tRhs;
		s = s * sRhs;

		parentIndex = skeleton.GetJointParentIndex( parentIndex );
	}
	
	return C_JointTransform( q, t, s );
}

const Maths::Quaternion C_SkeletonPose::GetModelJointRotation( U32 jointIndex ) const
{
	const C_Skeleton& skeleton = m_skeleton;
	const C_JointTransform* localJointTransforms = &m_localJointTransforms[ 0 ];

	S32 parentIndex = skeleton.GetJointParentIndex( jointIndex );
	Maths::Quaternion rotation = localJointTransforms[ jointIndex ].GetRotation();
	while ( parentIndex >= 0 )
	{
		rotation = rotation * localJointTransforms[ parentIndex ].GetRotation();
		parentIndex = skeleton.GetJointParentIndex( parentIndex );
	}
	
	return rotation;
}

void C_SkeletonPose::GetModelJointTransforms( C_JointTransform* destTransforms, U32 numDestTransforms ) const
{
	const U32 numJoints = Maths::Min( GetNumJoints(), numDestTransforms );
	BIZ_ASSERT( numJoints <= numDestTransforms && "Not enough space in destination buffer" );
	
	C_JointTransform* __restrict destModelJointTransforms = &destTransforms[ 0 ];
	const C_JointTransform* __restrict localJointTransforms = &m_localJointTransforms[ 0 ];
	const C_Skeleton& skeleton = m_skeleton;
	
	for ( U32 i = 0; i < numJoints; ++i )
	{
		const S32 parentIndex = skeleton.GetJointParentIndex( i );
		C_JointTransform worldJointTransform = localJointTransforms[ i ];
		if ( parentIndex >= 0 )
		{
			worldJointTransform = worldJointTransform * destModelJointTransforms[ parentIndex ];
		}
		destModelJointTransforms[ i ] = worldJointTransform;
	}	
}

#define ASSERT_16_BYTE_ALIGNED(x)	BIZ_ASSERT( (((U32)x) & 0xf) == 0 );

void C_SkeletonPose::GetLocalJointMatrices( Maths::Mat43* destMatrices, U32 numDestMatrices ) const
{
	const U32 numJoints = Maths::Min( GetNumJoints(), numDestMatrices );
	BIZ_ASSERT( numJoints <= numDestMatrices && "Not enough space in destination buffer" );
	
    const Maths::VF32 zero( Maths::Const<Maths::VF32>::Zero() );
	const Maths::VF32 one( Maths::Const<Maths::VF32>::One() );
	
	const C_JointTransform*  localJointTransforms = &m_localJointTransforms[ 0 ];

	for ( U32 i = 0; i < numJoints; ++i )
	{
		const Maths::Quaternion q = localJointTransforms[i].GetRotation();
		const Maths::Vector3 t = localJointTransforms[i].GetTranslation();
		const Maths::Vector3 s = localJointTransforms[i].GetScale();
		Maths::Mat43 jointMatrix;
		
		// Manually inlined version of MakeMat33 - optimises nicely.
		{
			const Maths::VF32 qx( X( q.m_Q ) );
			const Maths::VF32 qy( Y( q.m_Q ) );
			const Maths::VF32 qz( Z( q.m_Q ) );
			const Maths::VF32 qw( W( q.m_Q ) );
		    
			const Maths::VF32 _2x( qx + qx );
			const Maths::VF32 _2y( qy + qy ); 
			const Maths::VF32 _2z( qz + qz );
		    
			const Maths::VF32 _2xx( qx * _2x );
			const Maths::VF32 _2xy( qx * _2y );
			const Maths::VF32 _2xz( qx * _2z );
		                                
			const Maths::VF32 _2yy( qy * _2y );
			const Maths::VF32 _2yz( qy * _2z );
			const Maths::VF32 _2zz( qz * _2z );
		                                
			const Maths::VF32 _2ax( qw * _2x );
			const Maths::VF32 _2ay( qw * _2y );
			const Maths::VF32 _2az( qw * _2z );
		    
			Maths::Vec4F32 row_x( Maths::MakeVec4F32( one - _2yy - _2zz,       _2xy + _2az,       _2xz - _2ay, zero ) );
			Maths::Vec4F32 row_y( Maths::MakeVec4F32(       _2xy - _2az, one - _2xx - _2zz,       _2yz + _2ax, zero ) );
			Maths::Vec4F32 row_z( Maths::MakeVec4F32(       _2xz + _2ay,       _2yz - _2ax, one - _2xx - _2yy, zero ) );
			
			row_x *= Maths::X(s);
			row_y *= Maths::Y(s);
			row_z *= Maths::Z(s);
		    
			SetRows( &jointMatrix, row_x, row_y, row_z, Maths::MakeVec4F32( t ) );
		}
		
		destMatrices[i] = jointMatrix;
	}
}

void C_SkeletonPose::GetModelJointMatrices( Maths::Mat43* destMatrices, Animation::C_JointTransform* tempModelJointTransforms, U32 numDestMatrices ) const
{
	GetWorldJointMatrices( destMatrices, tempModelJointTransforms, numDestMatrices, Maths::Const<Maths::Mat43>::Identity() );
}

void C_SkeletonPose::GetWorldJointMatrices( Maths::Mat43* destMatrices, Animation::C_JointTransform* tempModelJointTransforms, U32 numDestMatrices, const Maths::Mat43& modelWorldMatrix ) const
{
	const U32 numJoints = Maths::Min( GetNumJoints(), numDestMatrices );
	BIZ_ASSERT( numJoints <= numDestMatrices && "Not enough space in destination buffer" );

	// Due to the way the scale is kept separate from the rotation and translation channels,
	// it's not possible to concatenate matrices created from local space C_JointTransforms.
	// Instead it's necessary to concatenate the local space C_JointTransforms and convert
	// the result into matrices.

	// Evaluate model joint transforms into temporary buffer.
	GetModelJointTransforms(tempModelJointTransforms, numJoints);

    const Maths::VF32 zero( Maths::Const<Maths::VF32>::Zero() );
	const Maths::VF32 one( Maths::Const<Maths::VF32>::One() );

	U32 jointIndex = 0;
	
	for (U32 i = 0; i < numJoints; ++i)
	{
		const Maths::Quaternion q = tempModelJointTransforms[i].GetRotation();
		const Maths::Vector3 t = tempModelJointTransforms[i].GetTranslation();
		const Maths::Vector3 s = tempModelJointTransforms[i].GetScale();
		
		Maths::Mat43 matrix;
				
		// Manually inlined version of MakeMat33
		{
			const Maths::VF32 qx( X( q.m_Q ) );
			const Maths::VF32 qy( Y( q.m_Q ) );
			const Maths::VF32 qz( Z( q.m_Q ) );
			const Maths::VF32 qw( W( q.m_Q ) );
		    
			const Maths::VF32 _2x( qx + qx );
			const Maths::VF32 _2y( qy + qy ); 
			const Maths::VF32 _2z( qz + qz );
		    
			const Maths::VF32 _2xx( qx * _2x );
			const Maths::VF32 _2xy( qx * _2y );
			const Maths::VF32 _2xz( qx * _2z );
		                                
			const Maths::VF32 _2yy( qy * _2y );
			const Maths::VF32 _2yz( qy * _2z );
			const Maths::VF32 _2zz( qz * _2z );
		                                
			const Maths::VF32 _2ax( qw * _2x );
			const Maths::VF32 _2ay( qw * _2y );
			const Maths::VF32 _2az( qw * _2z );
		    
			Maths::Vec4F32 row_x( Maths::MakeVec4F32( one - _2yy - _2zz,       _2xy + _2az,       _2xz - _2ay, zero ) );
			Maths::Vec4F32 row_y( Maths::MakeVec4F32(       _2xy - _2az, one - _2xx - _2zz,       _2yz + _2ax, zero ) );
			Maths::Vec4F32 row_z( Maths::MakeVec4F32(       _2xz + _2ay,       _2yz - _2ax, one - _2xx - _2yy, zero ) );

			row_x *= Maths::X(s);
			row_y *= Maths::Y(s);
			row_z *= Maths::Z(s);
		    
			SetRows( &matrix, row_x, row_y, row_z, Maths::MakeVec4F32( t ) );
		}

		destMatrices[i] = matrix * modelWorldMatrix;
	}
}

inline bool IsLocalPtr(const void* ptr)
{
#if defined(PLATFORM_SPU)
	return ((uintptr_t)ptr) < 0x40000;
#else
	return true;
#endif
}

void C_SkeletonPose::GetSkinningMatrices( Maths::Mat43* destMatrices, Animation::C_JointTransform* tempModelJointTransforms, U32 numDestMatrices ) const
{
	const U32 numJoints = Maths::Min( GetNumJoints(), numDestMatrices );
	const C_Skeleton& skeleton = m_skeleton;

	
#ifndef PLATFORM_SPU
	// Convert joint transforms into model space (on SPU these are calculated on demand to save memory)
	GetModelJointTransforms(tempModelJointTransforms, numJoints);
#endif

	const U32 kSpuBatchSize = 16;	// 16 = 768 bytes, 32 = 1536 bytes, 64 = 3072 bytes

	const bool isLocalDest = IsLocalPtr(destMatrices);

	U32 numJointsPerBatch = isLocalDest ? numJoints : kSpuBatchSize;
	U32 numBatches = (numJoints + numJointsPerBatch - 1) / numJointsPerBatch;

	Maths::Mat43* localDestMatrices = destMatrices;
	
#ifdef PLATFORM_SPU
	Maths::Mat43* mainDestMatrices = destMatrices;
	Spu::MemBaseMarker memBaseMarker;
	if (!isLocalDest)
	{
		localDestMatrices = (Maths::Mat43*) Spu::Alloc(sizeof(Maths::Mat43) * numJointsPerBatch);
	}
#endif

	skeleton.SpuFetchInverseWorldBasePose();

    const Maths::VF32 zero( Maths::Const<Maths::VF32>::Zero() );
	const Maths::VF32 one( Maths::Const<Maths::VF32>::One() );

	for (U32 firstJointIndex = 0; firstJointIndex < numJoints; firstJointIndex += numJointsPerBatch)
	{
		U32 endJointIndex = Maths::Min(firstJointIndex + numJointsPerBatch, numJoints);

		for (U32 jointIndex = firstJointIndex; jointIndex < endJointIndex; ++jointIndex)
		{
#if defined(PLATFORM_SPU)
			C_JointTransform transform = GetModelJointTransform(jointIndex);
#else
			const C_JointTransform& transform = tempModelJointTransforms[jointIndex];
#endif

			const Maths::Quaternion q = transform.GetRotation();
			const Maths::Vector3 t = transform.GetTranslation();
			const Maths::Vector3 s = transform.GetScale();
			
			Maths::Mat43 matrix;
					
			// Manually inlined version of MakeMat33
			{
				const Maths::VF32 qx( X( q.m_Q ) );
				const Maths::VF32 qy( Y( q.m_Q ) );
				const Maths::VF32 qz( Z( q.m_Q ) );
				const Maths::VF32 qw( W( q.m_Q ) );
			    
				const Maths::VF32 _2x( qx + qx );
				const Maths::VF32 _2y( qy + qy ); 
				const Maths::VF32 _2z( qz + qz );
			    
				const Maths::VF32 _2xx( qx * _2x );
				const Maths::VF32 _2xy( qx * _2y );
				const Maths::VF32 _2xz( qx * _2z );
			                                
				const Maths::VF32 _2yy( qy * _2y );
				const Maths::VF32 _2yz( qy * _2z );
				const Maths::VF32 _2zz( qz * _2z );
			                                
				const Maths::VF32 _2ax( qw * _2x );
				const Maths::VF32 _2ay( qw * _2y );
				const Maths::VF32 _2az( qw * _2z );
			    
				Maths::Vec4F32 row_x( Maths::MakeVec4F32( one - _2yy - _2zz,       _2xy + _2az,       _2xz - _2ay, zero ) );
				Maths::Vec4F32 row_y( Maths::MakeVec4F32(       _2xy - _2az, one - _2xx - _2zz,       _2yz + _2ax, zero ) );
				Maths::Vec4F32 row_z( Maths::MakeVec4F32(       _2xz + _2ay,       _2yz - _2ax, one - _2xx - _2yy, zero ) );

				row_x *= Maths::X(s);
				row_y *= Maths::Y(s);
				row_z *= Maths::Z(s);
			    
				SetRows( &matrix, row_x, row_y, row_z, Maths::MakeVec4F32( t ) );
			}
			
			localDestMatrices[jointIndex - firstJointIndex] = skeleton.GetInverseWorldBasePoseMatrix(jointIndex) * matrix;
		}

#if defined(PLATFORM_SPU)
		if (!isLocalDest)
		{
			// DMA batch of matrices back into main memory.
			U32 numMatrices = endJointIndex - firstJointIndex;
			Spu::Store(mainDestMatrices, localDestMatrices, sizeof(Maths::Mat43) * numMatrices);
			mainDestMatrices += numMatrices;
		}
#endif
	}
}

const Maths::Quaternion C_SkeletonPose::GetModelParentJointRotation( U32 jointIndex ) const
{
	Maths::Quaternion parentRotation = Maths::Const<Maths::Quaternion>::Identity();
	const S32 parentIndex = m_skeleton.GetJointParentIndex( jointIndex );
	if ( parentIndex >= 0 )
	{	
		parentRotation = GetModelJointRotation( parentIndex );
	}
	return parentRotation;
}

const C_JointTransform C_SkeletonPose::GetModelParentJointTransform( U32 jointIndex ) const
{
	C_JointTransform parentTransform;
	parentTransform.SetIdentity();
	const S32 parentIndex = m_skeleton.GetJointParentIndex( jointIndex );
	if ( parentIndex >= 0 )
	{	
		parentTransform = GetModelJointTransform( parentIndex );
	}
	return parentTransform;
}

} // namespace Animation
