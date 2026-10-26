#ifndef CROWD_ACTION_SELECT_H
#define CROWD_ACTION_SELECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdCycle.h>

#include <btl/containers/vector.h>

namespace AnimatedCrowd
{

//! CrowdCycleGroupData
struct ALIGN16 CrowdCycleGroupData
{
public:
	C_CrowdCycleGroup	m_CycleGroup;
	U32					m_BehaviourNameHash;
};

//! C_CrowdActionGroup

class ALIGN16 C_CrowdActionGroup
{
public:
	C_CrowdActionGroup();
	~C_CrowdActionGroup();

	void				Add(const CrowdAction action, const F32 weight = 1.f);
	void				Clear();

	const CrowdAction	ChooseAction(const C_CrowdCycleGroup& cycleGroup, const F32 seed = 0.f) const;

	const S32			GetNumAction() const;
	const CrowdAction	GetAction(const U32 index) const;

private:
	struct ActionGroupItem
	{
		U32				Action;
		F32				Weight;
	};

	Btl::Vector<ActionGroupItem, Btl::VectorParameters_Static<16> >	m_ActionItems;
};

//! C_CrowdActionSelect

class ALIGN16 C_CrowdActionSelect
{
public:
#ifndef PLATFORM_SPU
	C_CrowdActionSelect();
	~C_CrowdActionSelect();

	void		Initialise();
#endif // !PLATFORM_SPU

	const CrowdAction	SelectAction(const C_CrowdCycleGroup& cycleGroup, const CrowdActionGroup actionGroup, const F32 seed) const;
	const CrowdAction	SelectReaction(const C_CrowdCycleGroup& cycleGroup, const CrowdEventType eventType, const F32 seed, const CrowdGlobalMood globalMood, const F32 distance = 0.f) const;

private:
	C_CrowdActionGroup	m_ActionGroups[E_CROWD_ACTION_GROUP_COUNT];
};

}; // namespace AnimatedCrowd

#endif // CROWD_ACTION_SELECT_H
