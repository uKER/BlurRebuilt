#include "stdafx.h"
#include <animation/skeleton/Skeleton.h>
#include <animation/common/JointTransform.inl>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/memory/MemoryTracker.h>
#include <system/scheduler.h>

#define kSkeletonJointConstraintsMemorySubTag	"animation.skeleton.jointconstraints"

namespace Animation
{

//
// C_Skeleton
//

#ifndef PLATFORM_SPU


C_Skeleton::C_Skeleton():
	m_numJoints(0),
	m_firstParentedJointIndex( -1 ),
	m_parentIndices(0),
	m_basePoseTransforms(0),
	m_invWorldBasePoseMatrices(0)
{
}

C_Skeleton::~C_Skeleton()
{
	Destroy();
}

void C_Skeleton::Destroy()
{
	delete[] m_parentIndices;
	m_parentIndices = 0;
	delete[] m_basePoseTransforms;
	m_basePoseTransforms = 0;
	delete[] m_invWorldBasePoseMatrices;
	m_invWorldBasePoseMatrices = 0;
	m_numJoints = 0;
}

#endif

void C_Skeleton::SetName( const CHAR8* name )
{
#if !defined(CFG_GOLD) && !defined(PLATFORM_SPU)
	m_name = C_StringId( name );
#endif
}

const C_StringId& C_Skeleton::GetName() const
{
	return m_name;
}

void C_Skeleton::SpuFetchHierarchy() const
{
	SpuFetchHierarchyAsync(0);
	Spu::Wait(0);
}

void C_Skeleton::SpuFetchHierarchyAsync(U32 tag) const
{
	// TODO: needs to be multiple of 16 bytes in length?
	Spu::FetchAllocArray(const_cast<S16*&>(m_parentIndices), m_numJoints);
}

void C_Skeleton::SpuFetchLocalBasePose() const
{
	SpuFetchLocalBasePoseAsync(0);
	Spu::Wait(0);
}

void C_Skeleton::SpuFetchLocalBasePoseAsync(U32 tag) const
{
	Spu::FetchAllocArray(const_cast<C_JointTransform*&>(m_basePoseTransforms), m_numJoints);
}

void C_Skeleton::SpuFetchInverseWorldBasePose() const
{
	SpuFetchInverseWorldBasePoseAsync(0);
	Spu::Wait(0);
}

void C_Skeleton::SpuFetchInverseWorldBasePoseAsync(U32 tag) const
{
	Spu::FetchAllocArray(const_cast<Maths::Mat43*&>(m_invWorldBasePoseMatrices), m_numJoints);
}

#ifndef PLATFORM_SPU

S32 C_Skeleton::FindJointByName( const CHAR8* name ) const
{
	return FindJointByName( C_StringId( name, false ) );
}

S32 C_Skeleton::FindJointByName( const C_StringId& name ) const
{
	S32 index = -1;
	if (name.IsValid())
	{
		for ( U32 i = 0, count = m_numJoints; i < count; ++i )
		{
			if ( name == m_jointNames[ i ] )
			{
				index = i;
				break;
			}	
		}
	}
	return index;
}

#endif

/**
	Note that if @a jointIndex is equal to @a potentialDescendantJointIndex then this
	function will return true.
*/
bool C_Skeleton::IsDescendant( S32 jointIndex, S32 potentialDescendantJointIndex ) const
{
	S32 parentIndex = potentialDescendantJointIndex;
	
	while ( parentIndex >= 0 && parentIndex != jointIndex )
	{
		parentIndex = GetJointParentIndex( parentIndex );
	}
	
	return parentIndex >= 0;	
}

C_JointTransform C_Skeleton::GetModelBasePoseJointTransform( U32 jointIndex ) const
{
	C_JointTransform result = GetLocalBasePoseJointTransform(jointIndex);
	S32 parentIndex = GetJointParentIndex(jointIndex);
	while (parentIndex >= 0)
	{
		result *= GetLocalBasePoseJointTransform(parentIndex);
		parentIndex = GetJointParentIndex(parentIndex);
	}
	return result;
}

bool C_Skeleton::HasConstraints() const
{
	return (GetConstraints().GetNumConstraints() > 0);
}


//
// C_SkeletonMapping
//

#ifndef PLATFORM_SPU

C_SkeletonMapping::C_SkeletonMapping():
	m_skeletonA( 0 ),
	m_skeletonB( 0 ),
	m_jointMappingFromBToA( 0 )
{
}

C_SkeletonMapping::~C_SkeletonMapping()
{
	delete[] m_jointMappingFromBToA;
}
	
bool C_SkeletonMapping::Create(
	const C_Skeleton& skeletonA,
	const C_Skeleton& skeletonB
)
{
	m_skeletonA = &skeletonA;
	m_skeletonB = &skeletonB;

	delete[] m_jointMappingFromBToA;
	
	m_jointMappingFromBToA = 0;
	
	if ( !skeletonB.GetNumJoints() )
	{
		return true;
	}
	
	m_jointMappingFromBToA = new S16[ skeletonB.GetNumJoints() ];
	
	for ( U32 i = 0, count = skeletonB.GetNumJoints(); i < count; ++i )
	{
		S32 destJointIndex = skeletonA.FindJointByName( skeletonB.GetJointName( i ) );
		m_jointMappingFromBToA[ i ] = destJointIndex;
	}
	
	return true;
}

void C_SkeletonMapping::MapTransforms(
	const C_JointTransform* transformsA, 
	C_JointTransform* transformsB
) const
{
	BIZ_ASSERT( m_skeletonA );
	BIZ_ASSERT( m_skeletonB );
	BIZ_ASSERT( m_jointMappingFromBToA );
	
	for ( U32 i = 0, count = m_skeletonB->GetNumJoints(); i < count; ++i )
	{
		S32 jointIndexA = m_jointMappingFromBToA [ i ];
		
		if ( jointIndexA >= 0 )
		{
			transformsB[ i ] = transformsA[ jointIndexA ];
		}
		else
		{
			transformsB[ i ] = m_skeletonB->GetLocalBasePoseJointTransform( i );
		}
	}
	
}

//S32 C_SkeletonMapping::MapJointIndexFromAToB( U32 jointIndexA ) const
//{
//	return m_jointMappingFromAToB[ jointIndexA ];
//}

S32 C_SkeletonMapping::MapJointIndexFromBToA( U32 jointIndexB ) const
{
	return m_jointMappingFromBToA ? m_jointMappingFromBToA[ jointIndexB ] : -1;
}

#endif

} // namespace Animation
