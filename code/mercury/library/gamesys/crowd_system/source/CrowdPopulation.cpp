#include "CrowdSystemPch.h"

#include <crowd_system/CrowdCycle.h>
#include <crowd_system/CrowdPopulation.h>
#include <crowd_system/CrowdDebug.h>

#include <animation/cycle/Cycle.h>
#include <animation/skeleton/Skeleton.h>

namespace AnimatedCrowd
{

///////////////////////////////////////////////////////////////////////////////////////

C_CrowdSkeleton::C_CrowdSkeleton()
	: m_Valid(0)
{
}

C_CrowdSkeleton::~C_CrowdSkeleton()
{
}

void C_CrowdSkeleton::Create(const Util::C_String& filename)
{
	MEMORY_TRACKER_PUSH("mercury.crowd");
	m_SkeletonResourceHandle.Load(filename, this);
	MEMORY_TRACKER_POP("mercury.crowd");
}

void C_CrowdSkeleton::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	switch(result)
	{
	case Resource::E_Notify_DataReady:
		m_Valid = 1;
		break;
		
	default:
		break;
	}
}

const bool C_CrowdSkeleton::IsValid() const
{
	return (m_Valid > 0);
}

const Animation::C_SkeletonResource* C_CrowdSkeleton::GetSkeletonResource() const
{
	if (!IsValid())
		return NULL;

	return static_cast<const Animation::C_SkeletonResource*>(m_SkeletonResourceHandle);
}

///////////////////////////////////////////////////////////////////////////////////////

C_CrowdPopulation::C_CrowdPopulation()
	: m_pSkeleton(NULL)
{
	for (S32 i = 0, count = E_CROWD_EFFECT_SYSTEM_COUNT; i < count; ++i)
		m_EffectSystemEvents[i] = NULL;

	m_ActionSelect.Initialise();
}

C_CrowdPopulation::~C_CrowdPopulation()
{
	Clear();

	m_CycleGroupDataList.Destroy();
}

void C_CrowdPopulation::Create(const Btl::Vector<U32>& behaviourNameHashes)
{
	Clear();

	m_CycleGroupDataList.Destroy();

	BIZ_ASSERT(static_cast<U32>(behaviourNameHashes.Size()) <= CROWD_MAX_SHARED_ANIMATIONS);

	for (U32 i = 0, count = static_cast<U32>(behaviourNameHashes.Size()); i < count; ++i)
	{
		CrowdCycleGroupData cycleGroupData;
		cycleGroupData.m_BehaviourNameHash = behaviourNameHashes[i];
		cycleGroupData.m_CycleGroup.Clear();

		m_CycleGroupDataList.PushBack(cycleGroupData);
	}
}

void C_CrowdPopulation::Clear()
{
	if (m_pSkeleton)
		delete m_pSkeleton;
	m_pSkeleton = NULL;

	for (S32 i = 0, count = E_CROWD_EFFECT_SYSTEM_COUNT; i < count; ++i)
		m_EffectSystemEvents[i] = NULL;

	m_AnimationEventEffects.Destroy();
	m_AnimationEventEffectJointIndexes.Destroy();
}

void C_CrowdPopulation::AddCycle(const char* behaviourName, const Util::C_String& filename, const F32 weight, const CrowdAction action)
{
	if (m_CycleGroupDataList.Size() > 0)
	{
		S32 behaviourIndex = GetBehaviourIndex(Btl::Hash(behaviourName));
		if (behaviourIndex >= 0)
		{
			BIZ_ASSERT(behaviourIndex < m_CycleGroupDataList.Size());

			m_CycleGroupDataList[behaviourIndex].m_CycleGroup.AddCycle(filename, weight, action);
		}
	}
}

const S32 C_CrowdPopulation::GetNumCycles(const U16 behaviourIndex) const
{
	BIZ_ASSERT(behaviourIndex < static_cast<U32>(m_CycleGroupDataList.Size()));

	return m_CycleGroupDataList[behaviourIndex].m_CycleGroup.GetNumCycles();
}

const F32 C_CrowdPopulation::GetTotalWeight(const U16 behaviourIndex, const CrowdAction action) const
{
	BIZ_ASSERT(behaviourIndex < static_cast<U32>(m_CycleGroupDataList.Size()));

	return m_CycleGroupDataList[behaviourIndex].m_CycleGroup.GetTotalWeight(action);
}

void C_CrowdPopulation::SetSkeleton(const Util::C_String& filename)
{
	if (!m_pSkeleton)
		m_pSkeleton = new C_CrowdSkeleton();

	m_pSkeleton->Create(filename);
}

const C_CrowdSkeleton* C_CrowdPopulation::GetSkeleton() const
{
	return m_pSkeleton;
}

const C_CrowdActionSelect& C_CrowdPopulation::GetActionSelect() const
{
	return m_ActionSelect;
}

const C_CrowdCycleGroup& C_CrowdPopulation::GetCycleGroup(const U16 behaviourIndex) const
{
	BIZ_ASSERT(behaviourIndex < static_cast<U32>(m_CycleGroupDataList.Size()));

	return m_CycleGroupDataList[behaviourIndex].m_CycleGroup;
}

const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& C_CrowdPopulation::GetCycleGroupDataList() const
{
	return m_CycleGroupDataList;
}

void C_CrowdPopulation::SetEffectSystemEvent(const CrowdEffectSystemType type, DynamicEffect::Particle::C_Event* pEvent)
{
	m_EffectSystemEvents[type] = pEvent;
}

DynamicEffect::Particle::C_Event* C_CrowdPopulation::GetEffectSystemEvent(const CrowdEffectSystemType type) const
{
	return m_EffectSystemEvents[type];
}

void C_CrowdPopulation::AddAnimationEventEffect(XtHandle<DynamicEffect::Particle::XtInterface::Event> eventHandle, const char* pEventName, const char* pJointName)
{
	BIZ_ASSERT(m_AnimationEventEffects.Size() < CROWD_MAX_EVENT_EFFECTS);

	BIZ_ASSERT(eventHandle->pEvent);

	BIZ_ASSERT(GetSkeleton() && GetSkeleton()->IsValid());

	if (!GetSkeleton()->IsValid())
		return;

	S32 jointIndex = GetSkeleton()->GetSkeletonResource()->GetSkeleton()->FindJointByName(pJointName);
	
	if (jointIndex >= 0)
	{
		C_CrowdAnimationEventEffect eventEffect;
		eventEffect.SetEvent(eventHandle);
		eventEffect.SetEventName(pEventName);

		m_AnimationEventEffects.PushBack(eventEffect);
		m_AnimationEventEffectJointIndexes.PushBack(static_cast<U32>(jointIndex)); // Can extend to multiple skeletons if needed
	}
}

const U32 C_CrowdPopulation::GetNumAnimationEventEffects() const
{
	return static_cast<U32>(m_AnimationEventEffects.Size());
}

const C_CrowdAnimationEventEffect& C_CrowdPopulation::GetAnimationEventEffect(const U32 index) const
{
	BIZ_ASSERT(index < GetNumAnimationEventEffects());
	return m_AnimationEventEffects[index];
}

const Btl::Vector<C_CrowdAnimationEventEffect, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >& C_CrowdPopulation::GetAnimationEventEffects() const
{
	return m_AnimationEventEffects;
}

const Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >& C_CrowdPopulation::GetAnimationEventEffectJointIndexes() const
{
	return m_AnimationEventEffectJointIndexes;
}

const S32 C_CrowdPopulation::GetBehaviourIndex(const U32 behaviourNameHash) const
{
	for (U32 i = 0, count = static_cast<U32>(m_CycleGroupDataList.Size()); i < count; ++i)
	{
		if (m_CycleGroupDataList[i].m_BehaviourNameHash == behaviourNameHash)
			return static_cast<S32>(i);
	}

	return -1;
}

}; // namespace AnimatedCrowd
