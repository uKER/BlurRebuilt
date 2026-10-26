#include "CrowdSystemPch.h"

#include <crowd_system/CrowdSharedAnimationPool.h>
#include <crowd_system/CrowdRandom.h>

#ifndef PLATFORM_SPU
#include <crowd_system/CrowdResources.h>

#include <crowd_system/CrowdBlockRef.h>
#include <crowd_system/CrowdCharacterRef.h>
#include <crowd_system/CrowdInstance.h>

#include <game_resource/CrowdData.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
C_CrowdSharedAnimationPool::C_CrowdSharedAnimationPool()
	: m_pAnimationData(NULL),
	m_pAnimationDataDmaPtr(NULL),
	m_pResources(NULL),
	m_NumBehaviours(0),
	m_NumSpareAnimations(0),
	m_NumSpareAnimationsFree(0),
	m_NumUsedAnimations(0)
{
	m_AnimationList.Resize(CROWD_MAX_SHARED_ANIMATIONS);
	m_AnimationRefCounts.Resize(CROWD_MAX_SHARED_ANIMATIONS);
	m_AnimationCreationList.Resize(CROWD_MAX_SHARED_ANIMATIONS);

	for (U32 i = 0; i < CROWD_MAX_SHARED_ANIMATIONS; ++i)
	{
		m_AnimationRefCounts[i] = 0;

		m_AnimationCreationList[i].m_pPoseFormat = NULL;
		m_AnimationCreationList[i].m_pSkeleton = NULL;
		m_AnimationCreationList[i].m_BehaviourIndex = -1;
		m_AnimationCreationList[i].m_pEventStringIds = NULL;
		m_AnimationCreationList[i].m_NumEvents = 0;
	}

	ClearAnimationData();
}

C_CrowdSharedAnimationPool::~C_CrowdSharedAnimationPool()
{
	Destroy();
}

void C_CrowdSharedAnimationPool::Create(C_CrowdResources* pResources, U32 numBehaviours, const U32 routeMask, const GameResource::C_CrowdData* pCrowdData)
{
	m_pResources = pResources;

	if (!numBehaviours)
		numBehaviours = 1;

	BIZ_ASSERT(m_pAnimationData == NULL);

	m_pAnimationData = new AnimationData[numBehaviours];
	m_pAnimationDataDmaPtr = m_pAnimationData;

	U16* instanceCounts = (U16*)alloca(numBehaviours * sizeof(U16));
	memset(instanceCounts, 0, numBehaviours * sizeof(U16));

	U32 totalRouteInstanceCount = 0;

	if (pCrowdData)
	{
		const Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>& blockRefs(pCrowdData->GetCrowdBlockRefs());
		const Btl::Vector<AnimatedCrowd::C_CrowdInstance>& instanceRefs(pCrowdData->GetCrowdInstances());
		const Btl::Vector<U8>& behaviourIndexes(pCrowdData->GetCrowdCharacterRefs());
		const Btl::Vector<Btl::Pair<S16, S16> >& routeAndInstanceCounts(pCrowdData->GetRouteBlockAndInstanceCounts());

		S32 blockOffset = 0;

		for (S32 route = 0, routeCount = routeAndInstanceCounts.Size(); route < routeCount; ++route)
		{
			if (routeMask & (1 << route))
			{
				for (S32 i = 0, count = routeAndInstanceCounts[route].first; i < count; ++i)
				{
					BIZ_ASSERT((blockOffset + i) < blockRefs.Size());
					const AnimatedCrowd::C_CrowdBlockRef& blockRef(blockRefs[blockOffset + i]);

					for (U32 iInstance = 0, instanceCount = blockRef.GetNumInstances(); iInstance < instanceCount; ++iInstance)
						instanceCounts[behaviourIndexes[instanceRefs[blockRef.GetInstanceIndex(iInstance)].GetCharacterIndex()]]++;
				}

				totalRouteInstanceCount += routeAndInstanceCounts[route].second;
			}

			blockOffset += routeAndInstanceCounts[route].first;
		}
	}

	F32* instancePercentages = (F32*)alloca(numBehaviours * sizeof(F32));
	memset(instancePercentages, 0, numBehaviours * sizeof(F32));

	F32 size = 0.f;

	for (U32 i = 0; i < numBehaviours; ++i)
	{
		if (pCrowdData && totalRouteInstanceCount)
		{
			instancePercentages[i] = ::sqrtf((F32)instanceCounts[i]);
			size += instancePercentages[i];
		}
	}

	if (size > 0.f)
	{
		for (U32 i = 0; i < numBehaviours; ++i)
			instancePercentages[i] /= size;
	}

	for (U32 i = 0; i < numBehaviours; ++i)
	{
		U32 count = 1;

		if (pCrowdData && totalRouteInstanceCount)
		{
			U32 sharedAnimations = (U32)::floorf((instancePercentages[i] * CROWD_MAX_SHARED_ANIMATIONS) + 0.5f);
			if (!((instancePercentages[i] > 0.f) && !sharedAnimations))
				count = sharedAnimations;
		}

		count = Maths::Min<U32>(count, instanceCounts[i]);

		memset(m_pAnimationData[i].AnimationIndexes, -1, CROWD_MAX_SHARED_ANIMATIONS);
		m_pAnimationData[i].Count = 0;
		m_pAnimationData[i].Max = count;
	}
	
	m_NumBehaviours = numBehaviours;

	if (CROWD_MAX_SHARED_ANIMATIONS > numBehaviours)
	{
		m_NumSpareAnimationsFree = CROWD_MAX_SHARED_ANIMATIONS - numBehaviours;
		m_NumSpareAnimations = m_NumSpareAnimationsFree;
		BIZ_ASSERT(m_NumSpareAnimations <= static_cast<U32>(m_FreeSpareAnimations.Size()));
		m_FreeSpareAnimations.ClearBits();
	}
}

void C_CrowdSharedAnimationPool::Destroy()
{
	Purge();

	for (S32 i = 0, count = CROWD_MAX_SHARED_ANIMATIONS; i < count; ++i)
	{
		if (m_AnimationList[i].GetAnimation().GetIndex() != 0xff)
		{
			m_AnimationList[i].GetAnimation().ClearIndex();
			m_AnimationList[i].Destroy();

			if (i >= m_NumBehaviours)
			{
				m_FreeSpareAnimations.ClearBit(i - m_NumBehaviours);
				m_NumSpareAnimationsFree++;
			}
			m_NumUsedAnimations--;
		}

		m_AnimationRefCounts[i] = 0;
	}

	BIZ_ASSERT(m_NumSpareAnimationsFree == CROWD_MAX_SHARED_ANIMATIONS - m_NumBehaviours);
	BIZ_ASSERT(!m_FreeSpareAnimations.AnyBitsSet());

	ClearAnimationData();

	SAFE_DELETE_ARY(m_pAnimationData);
	m_pAnimationDataDmaPtr = NULL;
	m_NumBehaviours = 0;
	m_NumSpareAnimationsFree = CROWD_MAX_SHARED_ANIMATIONS;
	m_FreeSpareAnimations.ClearBits();
}

bool C_CrowdSharedAnimationPool::Flush()
{
	for (S32 i = 0, count = CROWD_MAX_SHARED_ANIMATIONS; i < count; ++i)
	{
		if (m_AnimationList[i].GetAnimation().GetIndex() != 0xff)
		{
			if (!m_AnimationList[i].Flush())
				return false;
		}
	}

	return true;
}
#endif // !PLATFORM_SPU

S32 C_CrowdSharedAnimationPool::CreateAnimation(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
												const U16 behaviourIndex, const Animation::C_StringId* pEventStringIds, const U32 numEvents)
{
	S32 sharedAnimation = -1;

	const AnimationData& animationData(m_pAnimationData[behaviourIndex]);

	BIZ_ASSERT(animationData.Count <= animationData.Max);

	if (!animationData.Max)
		return -1;

	if (!animationData.Count)
	{
		sharedAnimation = static_cast<S32>(behaviourIndex);

		BIZ_ASSERT(m_AnimationList[behaviourIndex].GetAnimation().m_State == C_CrowdAnimation::kState_Destroyed || m_AnimationList[behaviourIndex].GetAnimation().m_State == C_CrowdAnimation::kState_Initialised);

		m_pAnimationData[behaviourIndex].AnimationIndexes[m_pAnimationData[behaviourIndex].Count] = sharedAnimation;

		{
			m_AnimationCreationList[behaviourIndex].m_pPoseFormat = &poseFormat;
			m_AnimationCreationList[behaviourIndex].m_pSkeleton = pSkeleton;
			m_AnimationCreationList[behaviourIndex].m_BehaviourIndex = behaviourIndex;
			m_AnimationCreationList[behaviourIndex].m_pEventStringIds = pEventStringIds;
			m_AnimationCreationList[behaviourIndex].m_NumEvents = numEvents;
		}
		
		m_pAnimationData[sharedAnimation].Count++;
		m_NumUsedAnimations++;
	}
	else if ((animationData.Count < animationData.Max) && GetNumFreeSpareAnimations())
	{
		S32 firstFree = -1;
		for (U32 i = 0, count = m_NumSpareAnimations; i < count; ++i)
		{
			if (!m_FreeSpareAnimations.GetBitValue(i))
			{
				firstFree = static_cast<S32>(i);
				break;
			}
		}

		if (firstFree >= 0)
		{
			sharedAnimation = m_NumBehaviours + firstFree;

			BIZ_ASSERT(m_AnimationList[sharedAnimation].GetAnimation().m_State == C_CrowdAnimation::kState_Destroyed || m_AnimationList[sharedAnimation].GetAnimation().m_State == C_CrowdAnimation::kState_Initialised);

			m_FreeSpareAnimations.SetBit(firstFree);
			m_NumSpareAnimationsFree--;

			{
				m_AnimationCreationList[sharedAnimation].m_pPoseFormat = &poseFormat;
				m_AnimationCreationList[sharedAnimation].m_pSkeleton = pSkeleton;
				m_AnimationCreationList[sharedAnimation].m_BehaviourIndex = behaviourIndex;
				m_AnimationCreationList[sharedAnimation].m_pEventStringIds = pEventStringIds;
				m_AnimationCreationList[sharedAnimation].m_NumEvents = numEvents;
			}
			
			m_pAnimationData[behaviourIndex].AnimationIndexes[m_pAnimationData[behaviourIndex].Count] = sharedAnimation;
			m_pAnimationData[behaviourIndex].Count++;
			m_NumUsedAnimations++;
		}
		else
		{
			sharedAnimation = GetRandomAnimation(animationData);
		}
	}
	else
	{
		sharedAnimation = GetRandomAnimation(animationData);
	}

	if (sharedAnimation >= 0)
		return sharedAnimation;

	return -1; // No space left at the Inn :(
}

#ifndef PLATFORM_SPU
void C_CrowdSharedAnimationPool::AllocateAnimations()
{
	for (U32 i = 0; i < CROWD_MAX_SHARED_ANIMATIONS; ++i)
	{
		AnimationCreationParams& creationParams = m_AnimationCreationList[i];

		if (m_AnimationList[i].GetAnimation().GetIndex() == 0xff && creationParams.m_pPoseFormat)
		{
			if (i >= m_NumBehaviours && !m_FreeSpareAnimations.GetBitValue(i - m_NumBehaviours))
				continue;

			BIZ_ASSERT(m_AnimationList[i].GetAnimation().m_State == C_CrowdAnimation::kState_Destroyed || m_AnimationList[i].GetAnimation().m_State == C_CrowdAnimation::kState_Initialised);

			m_AnimationList[i].Create(*creationParams.m_pPoseFormat, creationParams.m_pSkeleton, creationParams.m_BehaviourIndex, creationParams.m_pEventStringIds, creationParams.m_NumEvents, i);

			if (m_pResources && (creationParams.m_BehaviourIndex == m_pResources->GetFlagBehaviourIndex()))
				m_pResources->AddFlagAnimation(i);

			{
				creationParams.m_pPoseFormat = NULL;
				creationParams.m_pSkeleton = NULL;
				creationParams.m_BehaviourIndex = -1;
				creationParams.m_pEventStringIds = NULL;
				creationParams.m_NumEvents = 0;
			}
		}
	}
}

void C_CrowdSharedAnimationPool::RemoveAnimation(const U8 index)
{
	BIZ_ASSERT(index < CROWD_MAX_SHARED_ANIMATIONS);
	BIZ_ASSERT(m_AnimationList[index].GetAnimation().GetIndex() != 0xff);
	BIZ_ASSERT(m_AnimationList[index].GetAnimation().GetIndex() == index);

	m_DeleteList.PushBack(index);
	m_AnimationList[index].GetAnimation().ClearIndex();

	{
		m_AnimationCreationList[index].m_pPoseFormat = NULL;
		m_AnimationCreationList[index].m_pSkeleton = NULL;
		m_AnimationCreationList[index].m_BehaviourIndex = -1;
		m_AnimationCreationList[index].m_pEventStringIds = NULL;
		m_AnimationCreationList[index].m_NumEvents = 0;
	}
}

void C_CrowdSharedAnimationPool::Purge()
{
	for (U32 i = 0, iCount = static_cast<U32>(m_DeleteList.Size()); i < iCount; ++i)
	{
		U8 deleteAnimIndex = m_DeleteList[i];

		if (deleteAnimIndex != 0xff)
		{
			BIZ_ASSERT(m_AnimationList[deleteAnimIndex].GetAnimation().GetIndex() == 0xff);

			AnimationData& animationData(m_pAnimationData[static_cast<U32>(m_AnimationList[deleteAnimIndex].GetBehaviourIndex())]);

			if (m_pResources && m_pResources->GetFlagBehaviourIndex() == m_AnimationList[deleteAnimIndex].GetBehaviourIndex() /*pSharedAnimation->GetBehaviourIndex()*/)
				m_pResources->RemoveFlagAnimation(deleteAnimIndex);

			animationData.RemoveAnimationIndex(deleteAnimIndex);
			
			if (deleteAnimIndex >= m_NumBehaviours)
			{
				m_FreeSpareAnimations.ClearBit(deleteAnimIndex - m_NumBehaviours);
				m_NumSpareAnimationsFree++;
			}

			m_AnimationList[deleteAnimIndex].Destroy();
		
			m_NumUsedAnimations--;
		}
	}

	m_DeleteList.Clear();
}

bool C_CrowdSharedAnimationPool::UpdateAnimation(const C_UpdateContext& context, const S32 index, const Animation::C_JointTransform& hipJointTransform, const CrowdGlobalMood globalMood)
{
	BIZ_ASSERT(index < CROWD_MAX_SHARED_ANIMATIONS);
	BIZ_ASSERT(m_AnimationList[index].GetAnimation().GetIndex() != 0xff);

	if (m_AnimationList[index].GetAnimation().IsDirty())
	{
		m_AnimationList[index].Update(context, hipJointTransform, globalMood);
		return true;
	}

	return false;
}
#endif // !PLATFORM_SPU

C_CrowdSharedAnimation* C_CrowdSharedAnimationPool::GetAnimation(const S32 index)
{
	if (index < CROWD_MAX_SHARED_ANIMATIONS)
		return &m_AnimationList[index];
	return NULL;
}

const C_CrowdSharedAnimation* C_CrowdSharedAnimationPool::GetAnimation(const S32 index) const
{
	if (index < CROWD_MAX_SHARED_ANIMATIONS)
		return &m_AnimationList[index];
	return NULL;
}

const U32 C_CrowdSharedAnimationPool::GetNumAnimations() const
{
	return m_NumUsedAnimations;
}

const U32 C_CrowdSharedAnimationPool::GetNumFreeAnimations() const
{
	return (CROWD_MAX_SHARED_ANIMATIONS - GetNumAnimations());
}

void C_CrowdSharedAnimationPool::AddRef(const U32 index)
{
	BIZ_ASSERT(index < CROWD_MAX_SHARED_ANIMATIONS);
	m_AnimationRefCounts[index]++;
}

void C_CrowdSharedAnimationPool::RemoveRef(const U32 index)
{
	BIZ_ASSERT(index < CROWD_MAX_SHARED_ANIMATIONS);
	if (m_AnimationRefCounts[index] > 0)
		m_AnimationRefCounts[index]--;
}

const U32 C_CrowdSharedAnimationPool::GetRefCount(const U32 index) const
{
	BIZ_ASSERT(index < CROWD_MAX_SHARED_ANIMATIONS);
	return m_AnimationRefCounts[index];
}

#ifdef PLATFORM_SPU
void C_CrowdSharedAnimationPool::FetchInternal()
{
	m_pAnimationData = m_pAnimationDataDmaPtr;
	Spu::FetchAllocArray(m_pAnimationData, m_NumBehaviours);
}

void C_CrowdSharedAnimationPool::StoreInternal()
{
	Spu::Store(m_pAnimationDataDmaPtr, m_pAnimationData, sizeof(AnimationData) * m_NumBehaviours);
	m_pAnimationData = m_pAnimationDataDmaPtr;
}
#endif // PLATFORM_SPU

S32 C_CrowdSharedAnimationPool::GetRandomAnimation(const AnimationData& animationData) const
{
	BIZ_ASSERT(animationData.Count);

	U32 startIndex = AnimatedCrowd::C_Random::Instance().RandomU32(animationData.Count);

	if (startIndex != 0)
		volatile S32 breakhere = 1;

	return animationData.AnimationIndexes[startIndex];
}

void C_CrowdSharedAnimationPool::ClearAnimationData()
{
	for (U32 i = 0; i < m_NumBehaviours; ++i)
	{
		for (U32 j = 0; j < m_pAnimationData[i].Max; ++j)
			m_pAnimationData[i].AnimationIndexes[j] = -1;

		m_pAnimationData[i].Count = 0;
		m_pAnimationData[i].Max = 0;
	}
}

const U32 C_CrowdSharedAnimationPool::GetNumFreeSpareAnimations() const
{
	return m_NumSpareAnimationsFree;
}

}; // namespace AnimatedCrowd
