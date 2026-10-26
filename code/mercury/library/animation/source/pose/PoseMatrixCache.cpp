#include "stdafx.h"
#include <animation/pose/PoseMatrixCache.h>
#include <animation/pose/Pose.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include <animation/skeleton/Skeleton.h>
#include <animation/common/JointTransform.inl>
#include <system/Perf.h>

namespace Animation
{

//
// C_PoseMatrixCache
//

#ifndef PLATFORM_SPU
C_PoseMatrixCache::C_PoseMatrixCache():
	m_numSkinningMatrices(0),
	m_maxMatrices(0),
	m_numMatrices(0),
	m_matrices(NULL),
	m_tempJointTransforms(NULL)
{
}

C_PoseMatrixCache::C_PoseMatrixCache(const C_Pose& pose):
	m_numSkinningMatrices(0),
	m_maxMatrices(0),
	m_numMatrices(0),
	m_matrices(NULL),
	m_tempJointTransforms(NULL)
{
	Create(pose);
}

C_PoseMatrixCache::~C_PoseMatrixCache()
{
	Destroy();
}

void C_PoseMatrixCache::Create(const C_Pose& pose)
{
	const U32 numMatrices = pose.GetNumJointTransforms();
	m_numSkinningMatrices = numMatrices;
	Resize(numMatrices);
}

void C_PoseMatrixCache::Destroy()
{
	delete[] m_matrices;
	m_matrices = NULL;
	delete[] m_tempJointTransforms;
	m_tempJointTransforms = NULL;
	m_numSkinningMatrices = 0;
	m_numMatrices = 0;
	m_maxMatrices = 0;
}
#endif

void C_PoseMatrixCache::SpuFetch()
{
#ifdef PLATFORM_SPU
	Spu::FetchAllocArray(m_matrices, m_numMatrices);
#endif
}

void C_PoseMatrixCache::Update(const C_Pose& pose, const C_Skeleton* skeleton)
{
//#ifdef PLATFORM_SPU
//	Spu::MemBaseMarker memBaseMarker;
//	m_tempJointTransforms = (Animation::C_JointTransform*) Spu::Alloc(m_numMatrices * sizeof(Animation::C_JointTransform));
//#endif

	const U32 numMatrices = pose.GetNumJointTransforms();
	BIZ_ASSERT(m_numMatrices == numMatrices);
	
	if (0 == numMatrices)
	{
		return;
	}
	
	BuildModelSpaceSkinningMatrices(pose, skeleton, m_matrices);
}

const Maths::Mat43*	 C_PoseMatrixCache::GetSkinningMatrices() const
{
	if (m_numSkinningMatrices > 0)
	{
		return &m_matrices[0];
	}
	return NULL;
}
	
#ifndef PLATFORM_SPU
void C_PoseMatrixCache::Resize(U32 numMatrices)
{
	if (numMatrices > m_maxMatrices)
	{
		delete[] m_matrices;
		m_matrices = new Maths::Mat43[numMatrices];
		delete[] m_tempJointTransforms;
		m_tempJointTransforms = new Animation::C_JointTransform[numMatrices];	// TODO: don't need these if there's no skeleton.
		m_maxMatrices = numMatrices;
#ifdef PLATFORM_PS3
		BIZ_ASSERT(((U32)m_matrices) > 0x400000);
#endif
	}
	m_numMatrices = numMatrices;
}
#endif


// NB. destMatrices is assumed to be in ppu/main memory.
void C_PoseMatrixCache::BuildModelSpaceSkinningMatrices(const C_Pose& pose, const C_Skeleton* skeleton, Maths::Mat43* destMatrices)
{
	if (skeleton != NULL)
	{
		U32 numSkeletonJoints = skeleton->GetNumJoints();
		U32 numExtraJoints = pose.GetNumJointTransforms() - numSkeletonJoints;
		
		C_SkeletonPose skeletonPose(*skeleton, const_cast<C_Pose&>(pose));	// TODO: move GetSkinningMatrices into C_Pose.
		
		// TODO: Wrap up skeleton/extra joints logic into skeletonPose.
		if (numSkeletonJoints > 0)
		{
			skeletonPose.GetSkinningMatrices(destMatrices, m_tempJointTransforms, pose.GetNumJointTransforms());
		}
		
		if (numExtraJoints > 0)
		{
			Maths::Mat43* localDestMatrices = destMatrices + numSkeletonJoints;
#ifdef PLATFORM_SPU
			Spu::MemBaseMarker memBaseMarker;
			localDestMatrices = (Maths::Mat43*) Spu::Alloc(sizeof(Maths::Mat43) * numExtraJoints);
#endif
			for (U32 i = 0; i < numExtraJoints; ++i)
			{
				localDestMatrices[i] = pose.GetJointTransform(i + numSkeletonJoints).AsMat43();
			}
#ifdef PLATFORM_SPU
			Spu::Store(destMatrices + numSkeletonJoints, localDestMatrices, sizeof(Maths::Mat43) * numExtraJoints);
#endif
		}
	}
	else
	{
		Maths::Mat43* localDestMatrices = destMatrices;
#ifdef PLATFORM_SPU
		Spu::MemBaseMarker memBaseMarker;
		localDestMatrices = (Maths::Mat43*) Spu::Alloc(sizeof(Maths::Mat43) * pose.GetNumJointTransforms());
#endif
		// No skeleton, so transforms are assumed to be in model space already and just need converting to matrices.
		for (U32 i = 0, count = pose.GetNumJointTransforms(); i < count; ++i)
		{
			const C_JointTransform transform = pose.GetJointTransform(i);
			localDestMatrices[i] = transform.AsMat43();
		}
#ifdef PLATFORM_SPU
		Spu::Store(destMatrices, localDestMatrices, pose.GetNumJointTransforms() * sizeof(Maths::Mat43));
#endif
	}
}

} // end Animation


