#include "CrowdSystemPch.h"

#include <crowd_system/CrowdUniqueAnimationPool.h>

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
C_CrowdUniqueAnimationPool::C_CrowdUniqueAnimationPool()
	: m_NumUsedAnimations(0)
{
	BIZ_ASSERT(m_FreeAnimations.Size() <= CROWD_MAX_LOCAL_ANIMATIONS);
	m_FreeAnimations.ClearBits();

	m_AnimationList.Resize(CROWD_MAX_LOCAL_ANIMATIONS);
	m_AnimationCreationList.Resize(CROWD_MAX_LOCAL_ANIMATIONS);

	for (U32 i = 0; i < CROWD_MAX_LOCAL_ANIMATIONS; ++i)
	{
		m_AnimationCreationList[i].m_pPoseFormat = NULL;
		m_AnimationCreationList[i].m_pSkeleton = NULL;
		m_AnimationCreationList[i].m_BehaviourIndex = -1;
		m_AnimationCreationList[i].m_pEventStringIds = NULL;
		m_AnimationCreationList[i].m_NumEvents = 0;
	}
}

C_CrowdUniqueAnimationPool::~C_CrowdUniqueAnimationPool()
{
	Destroy();
}

void C_CrowdUniqueAnimationPool::Destroy()
{
	Purge();

	for (S32 i = 0, count = CROWD_MAX_LOCAL_ANIMATIONS; i < count; ++i)
	{
		if (m_AnimationList[i].GetIndex() != 0xff)
		{
			m_AnimationList[i].ClearIndex();
			m_AnimationList[i].Destroy();
			m_FreeAnimations.ClearBit(i);
			BIZ_ASSERT(m_NumUsedAnimations > 0);
			m_NumUsedAnimations--;			
		}
	}

	BIZ_ASSERT(m_NumUsedAnimations == 0);
	BIZ_ASSERT(!m_FreeAnimations.AnyBitsSet());
}

bool C_CrowdUniqueAnimationPool::Flush()
{
	for (S32 i = 0, count = CROWD_MAX_LOCAL_ANIMATIONS; i < count; ++i)
	{
		if (m_AnimationList[i].GetIndex() != 0xff)
		{
			if (!m_AnimationList[i].Flush())
				return false;
		}
	}

	return true;
}
#endif // !PLATFORM_SPU

S32 C_CrowdUniqueAnimationPool::CreateAnimation(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
												const U16 behaviourIndex, const Animation::C_StringId* pEventStringIds, const U32 numEvents)
{
	if (GetNumFreeAnimations())
	{
		S32 firstFree = -1;
		for (S32 i = 0, count = CROWD_MAX_LOCAL_ANIMATIONS; i < count; ++i)
		{
			if (!m_FreeAnimations.GetBitValue(i))
			{
				firstFree = i;
				break;
			}
		}

		if (firstFree >= 0)
		{
			m_NumUsedAnimations++;

			BIZ_ASSERT(m_AnimationList[firstFree].m_State == C_CrowdAnimation::kState_Destroyed || m_AnimationList[firstFree].m_State == C_CrowdAnimation::kState_Initialised);

			m_FreeAnimations.SetBit(firstFree);

			{
				m_AnimationCreationList[firstFree].m_pPoseFormat = &poseFormat;
				m_AnimationCreationList[firstFree].m_pSkeleton = pSkeleton;
				m_AnimationCreationList[firstFree].m_BehaviourIndex = behaviourIndex;
				m_AnimationCreationList[firstFree].m_pEventStringIds = pEventStringIds;
				m_AnimationCreationList[firstFree].m_NumEvents = numEvents;
			}

			return firstFree;
		}

		return -1;
	}

	return -1; // No space left at the Inn
}

#ifndef PLATFORM_SPU
void C_CrowdUniqueAnimationPool::AllocateAnimations()
{
	for (U32 i = 0; i < CROWD_MAX_LOCAL_ANIMATIONS; ++i)
	{
		AnimationCreationParams& creationParams = m_AnimationCreationList[i];

		if (m_AnimationList[i].GetIndex() == 0xff && creationParams.m_pPoseFormat && m_FreeAnimations.GetBitValue(i))
		{
			BIZ_ASSERT(m_AnimationList[i].m_State == C_CrowdAnimation::kState_Destroyed || m_AnimationList[i].m_State == C_CrowdAnimation::kState_Initialised);

			m_AnimationList[i].Create(*creationParams.m_pPoseFormat, creationParams.m_pSkeleton, creationParams.m_pEventStringIds, creationParams.m_NumEvents, i);

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

void C_CrowdUniqueAnimationPool::RemoveAnimation(const U8 index)
{
	BIZ_ASSERT(index < CROWD_MAX_LOCAL_ANIMATIONS);
	BIZ_ASSERT(m_AnimationList[index].GetIndex() != 0xff);
	BIZ_ASSERT(m_AnimationList[index].GetIndex() == index);
	
	m_DeleteList.PushBack(index);
	m_AnimationList[index].ClearIndex();

	{
		m_AnimationCreationList[index].m_pPoseFormat = NULL;
		m_AnimationCreationList[index].m_pSkeleton = NULL;
		m_AnimationCreationList[index].m_BehaviourIndex = -1;
		m_AnimationCreationList[index].m_pEventStringIds = NULL;
		m_AnimationCreationList[index].m_NumEvents = 0;
	}
}

void C_CrowdUniqueAnimationPool::Purge()
{
	for (U32 i = 0, count = static_cast<U32>(m_DeleteList.Size()); i < count; ++i)
	{
		U8 deleteAnimIndex = m_DeleteList[i];

		if (deleteAnimIndex != 0xff)
		{
			BIZ_ASSERT(m_AnimationList[deleteAnimIndex].GetIndex() == 0xff);

			m_AnimationList[deleteAnimIndex].Destroy();
		
			m_FreeAnimations.ClearBit(deleteAnimIndex);

			m_NumUsedAnimations--;
		}
	}

	m_DeleteList.Clear();
}
#endif // !PLATFORM_SPU

bool C_CrowdUniqueAnimationPool::UpdateAnimation(const C_CrowdResources* pResources, const S32 index, const F32 deltaTime)
{
	BIZ_ASSERT_FAIL("Not sure!");
	
	return true;
}

C_CrowdAnimation* C_CrowdUniqueAnimationPool::GetAnimation(const S32 index)
{
	if (index < CROWD_MAX_LOCAL_ANIMATIONS)
		return &m_AnimationList[index];
	return NULL;
}

const C_CrowdAnimation* C_CrowdUniqueAnimationPool::GetAnimation(const S32 index) const
{
	if (index < CROWD_MAX_LOCAL_ANIMATIONS)
		return &m_AnimationList[index];
	return NULL;
}

const U32 C_CrowdUniqueAnimationPool::GetNumAnimations() const
{
	return m_NumUsedAnimations;
}

const U32 C_CrowdUniqueAnimationPool::GetNumFreeAnimations() const
{
	S32 numFreeAnimations = m_FreeAnimations.Size() - m_NumUsedAnimations;

	BIZ_ASSERT(numFreeAnimations >= 0);
	BIZ_ASSERT(m_FreeAnimations.CountBitsSet() == m_NumUsedAnimations);

	return static_cast<U32>(numFreeAnimations);
}

}; // namespace AnimatedCrowd
