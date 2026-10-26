#ifndef CROWD_UNIQUE_ANIMATION_POOL_H
#define CROWD_UNIQUE_ANIMATION_POOL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdAnimation.h>

#include <btl/Allocators/pool.h>

#include <btl/utility/pair.h>
#include <btl/containers/vector.h>
#include <btl/containers/bitvector.h>

namespace AnimatedCrowd
{
	class C_CrowdResources;
};

namespace AnimatedCrowd
{

//! C_CrowdUniqueAnimationPool
class ALIGN16 C_CrowdUniqueAnimationPool
{
public:

#ifndef PLATFORM_SPU
	C_CrowdUniqueAnimationPool();
	~C_CrowdUniqueAnimationPool();

	void				Destroy();
	bool				Flush();
#endif // !PLATFORM_SPU

	S32					CreateAnimation(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
													const U16 behaviourIndex, const Animation::C_StringId* pEventStringIds, const U32 numEvents);

#ifndef PLATFORM_SPU
	void				AllocateAnimations();

	void				RemoveAnimation(const U8 index);

	void				Purge();
#endif // !PLATFORM_SPU

	bool				UpdateAnimation(const C_CrowdResources* pResources, const S32 index, const F32 deltaTime);

	C_CrowdAnimation*	GetAnimation(const S32 index);
	const C_CrowdAnimation*
						GetAnimation(const S32 index) const;

	const U32			GetNumAnimations() const;
	const U32			GetNumFreeAnimations() const;

private:
	typedef Btl::Vector<C_CrowdAnimation, Btl::VectorParameters_Static<CROWD_MAX_LOCAL_ANIMATIONS> >		AnimationList;
	typedef Btl::Vector<AnimationCreationParams, Btl::VectorParameters_Static<CROWD_MAX_LOCAL_ANIMATIONS> >	AnimationCreationParamList;
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_LOCAL_ANIMATIONS> >						AnimationDeleteList;
	typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U8, Btl::VectorParameters_Array<8> > > >	FreeAnimationsBitArray;

	AnimationList					m_AnimationList;
	AnimationCreationParamList		m_AnimationCreationList;

	U32								m_NumUsedAnimations;

	AnimationDeleteList				m_DeleteList;

	FreeAnimationsBitArray			m_FreeAnimations;
};

}; // namespace AnimatedCrowd

#endif // CROWD_UNIQUE_ANIMATION_POOL_H
