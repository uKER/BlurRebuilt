#ifndef CROWD_SHARED_ANIMATION_POOL_H
#define CROWD_SHARED_ANIMATION_POOL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdSharedAnimation.h>
#include <crowd_system/CrowdContext.h>

#include <btl/utility/pair.h>
#include <btl/Allocators/pool.h>
#include <btl/containers/bitvector.h>

namespace GameResource
{
	class C_CrowdData;
};

namespace AnimatedCrowd
{

//! C_CrowdSharedAnimationPool
class ALIGN16 C_CrowdSharedAnimationPool
{
public:

#ifndef PLATFORM_SPU
	C_CrowdSharedAnimationPool();
	~C_CrowdSharedAnimationPool();

	void					Create(C_CrowdResources* pResources, U32 numBehaviours, const U32 routeMask, const GameResource::C_CrowdData* pCrowdData);

	void					Destroy();
	bool					Flush();
#endif // !PLATFORM_SPU

	S32						CreateAnimation(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
								const U16 behaviourIndex, const Animation::C_StringId* pEventStringIds, const U32 numEvents);

#ifndef PLATFORM_SPU
	void					AllocateAnimations();
	
	void					RemoveAnimation(const U8 index);

	void					Purge();

	bool					UpdateAnimation(const C_UpdateContext& context, const S32 index, const Animation::C_JointTransform& hipJointTransform, const CrowdGlobalMood globalMood);
#endif // !PLATFORM_SPU

	C_CrowdSharedAnimation*	GetAnimation(const S32 index);
	const C_CrowdSharedAnimation*
							GetAnimation(const S32 index) const;

	const U32				GetNumAnimations() const;
	const U32				GetNumFreeAnimations() const;

	const U16				GetNumBehaviours() const { return m_NumBehaviours; }

	void					AddRef(const U32 index);
	void					RemoveRef(const U32 index);

	const U32				GetRefCount(const U32 index) const;

#ifdef PLATFORM_SPU
	void					FetchInternal();
	void					StoreInternal();
#endif // PLATFORM_SPU

private:
	typedef Btl::Vector<C_CrowdSharedAnimation, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >		AnimationList;
	typedef Btl::Vector<U16, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >						AnimationRefCounts;
	typedef Btl::Vector<AnimationCreationParams, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >	AnimationCreationParamList;
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >							AnimationDeleteList;
	typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U8, Btl::VectorParameters_Array<8> > > >		FreeAnimationsBitArray;

	struct ALIGN16 AnimationData
	{
		void RemoveAnimationIndex(const S8 animationIndex)
		{
			if (!Count || !Max)
				return;

			S8*	animationIndexes = (S8*)alloca(Max);
			
			U32 iter = 0;
			for (U32 i = 0, count = Max; i < count; ++i)
			{
				if (AnimationIndexes[iter] == animationIndex)
					iter++;

				if (iter < Max)
					animationIndexes[i] = AnimationIndexes[iter];
				else
					animationIndexes[i] = -1;

				iter++;
			}

			memcpy(AnimationIndexes, animationIndexes, Max);
			Count--;
		}

		S8	AnimationIndexes[CROWD_MAX_SHARED_ANIMATIONS];
		U32	Count;
		U32	Max;
	};

public:
	const U32	GetMaxAnimationsByBehaviour(const U16 behaviourIndex) const { return m_pAnimationData[behaviourIndex].Max; }
	const U32	GetNumAnimationsByBehaviour(const U16 behaviourIndex) const { return m_pAnimationData[behaviourIndex].Count; }

#ifndef CFG_GOLD
	const S8	GetSharedAnimationIndex(const U16 behaviourIndex, const S32 animationIndex) const
				{
					if (behaviourIndex < m_NumBehaviours)
					{
						if (animationIndex >= 0 && animationIndex < static_cast<S32>(m_pAnimationData[behaviourIndex].Max))
						{
							return m_pAnimationData[behaviourIndex].AnimationIndexes[animationIndex];
						}
					}
					return -1;
				}
#endif

private:
	S32								GetRandomAnimation(const AnimationData& animationData) const;
	void							ClearAnimationData();
	const U32						GetNumFreeSpareAnimations() const;

private:
	AnimationList					m_AnimationList;
	AnimationRefCounts				m_AnimationRefCounts;
	AnimationCreationParamList		m_AnimationCreationList;

	AnimationData*					m_pAnimationData;
	AnimationData*					m_pAnimationDataDmaPtr;
	
	C_CrowdResources*				m_pResources;

	U16								m_NumBehaviours;
	U16								m_NumUsedAnimations;

	AnimationDeleteList				m_DeleteList;

	FreeAnimationsBitArray			m_FreeSpareAnimations;
	U16								m_NumSpareAnimations;
	U16								m_NumSpareAnimationsFree;
};

}; // namespace AnimatedCrowd

#endif // CROWD_SHARED_ANIMATION_POOL_H
