#ifndef CROWD_POPULATION_H
#define CROWD_POPULATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Containers/String.h>

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdActionSelect.h>
#include <crowd_system/CrowdAnimationEventEffect.h>

#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>

#include <btl/containers/Vector.h>
#include <btl/containers/Map.h>

namespace DynamicEffect
{
	namespace Particle
	{
		class C_Event;
	};

	class C_Node;
};

namespace AnimatedCrowd
{
	class C_CrowdCycle;
	class C_CrowdCycleGroup;
};

namespace AnimatedCrowd
{

//! C_CrowdSkeleton
class C_CrowdSkeleton: public Resource::C_Listener
{
public:
	C_CrowdSkeleton();
	~C_CrowdSkeleton();

	void			Create(const Util::C_String& filename);

	virtual void	ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);

	const bool		IsValid() const;

	const Animation::C_SkeletonResource*	GetSkeletonResource() const;

private:
	Animation::C_SkeletonResourceHandle		m_SkeletonResourceHandle;

	U32				m_Valid : 1;
};

//! C_CrowdPopulation

class ALIGN16 C_CrowdPopulation
{
private:
	typedef Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >		CycleGroupDataList;
	typedef Btl::Vector<C_CrowdAnimationEventEffect, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >	EventEffectList;
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >								EventEffectJointIndexList;

public:
	C_CrowdPopulation();
	~C_CrowdPopulation();

	void	Create(const Btl::Vector<U32>& behaviourNameHashes);

	void	Clear();

	void								AddCycle(const char* behaviourName, const Util::C_String& filename,
											const F32 weight = 1.f, const CrowdAction = E_CROWD_ACTION_IDLE);

	const S32							GetNumCycles(const U16 behaviourIndex) const;
	const F32							GetTotalWeight(const U16 behaviourIndex, const CrowdAction action) const;

	void								SetSkeleton(const Util::C_String& filename);
	const C_CrowdSkeleton*				GetSkeleton() const;

	const C_CrowdActionSelect&			GetActionSelect() const;

	const C_CrowdCycleGroup&			GetCycleGroup(const U16 behaviourIndex) const;
	const CycleGroupDataList&			GetCycleGroupDataList() const;

	void								SetEffectSystemEvent(const CrowdEffectSystemType type, DynamicEffect::Particle::C_Event* pEvent);
	DynamicEffect::Particle::C_Event*	GetEffectSystemEvent(const CrowdEffectSystemType type) const;

	void								AddAnimationEventEffect(XtHandle<DynamicEffect::Particle::XtInterface::Event> eventHandle, const char* pEventName, const char* pJointName);
	const U32							GetNumAnimationEventEffects() const;
	const C_CrowdAnimationEventEffect&	GetAnimationEventEffect(const U32 index) const;

	const EventEffectList&				GetAnimationEventEffects() const;
	const EventEffectJointIndexList&	GetAnimationEventEffectJointIndexes() const;

	const S32							GetBehaviourIndex(const U32 behaviourNameHash) const;

private:
	CycleGroupDataList ALIGN16			m_CycleGroupDataList;

	C_CrowdSkeleton*					m_pSkeleton;

	DynamicEffect::Particle::C_Event*	m_EffectSystemEvents[E_CROWD_EFFECT_SYSTEM_COUNT];

	
	EventEffectList						m_AnimationEventEffects;
	EventEffectJointIndexList ALIGN16	m_AnimationEventEffectJointIndexes;

	C_CrowdActionSelect					m_ActionSelect;
};

}; // namespace AnimatedCrowd

#endif // CROWD_POPULATION_H
