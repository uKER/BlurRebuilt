#include "CrowdSystemPch.h"

#include <crowd_system/CrowdActionSelect.h>
#include <crowd_system/CrowdCycle.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdWorld.h>
#include <crowd_system/CrowdPopulation.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

C_CrowdActionGroup::C_CrowdActionGroup()
{
}

C_CrowdActionGroup::~C_CrowdActionGroup()
{
}

void C_CrowdActionGroup::Add(const CrowdAction action, const F32 weight)
{
	ActionGroupItem item;
	item.Action = action;
	item.Weight = weight;

	m_ActionItems.PushBack(item);
}

void C_CrowdActionGroup::Clear()
{
	m_ActionItems.Destroy();
}

const S32 C_CrowdActionGroup::GetNumAction() const
{
	return m_ActionItems.Size();
}

const CrowdAction C_CrowdActionGroup::ChooseAction(const C_CrowdCycleGroup& cycleGroup, const F32 seed) const
{
	F32 totalWeight = 0.f;

	if (!m_ActionItems.Size())
		return E_CROWD_ACTION_IDLE;

	for (S32 i = 0, count = m_ActionItems.Size(); i < count; ++i)
	{
		if (cycleGroup.HasCyclesForAction(static_cast<CrowdAction>(m_ActionItems[i].Action)))
			totalWeight += m_ActionItems[i].Weight;
	}

	if (totalWeight < Maths::Constants<F32>::Epsilon())
		return E_CROWD_ACTION_IDLE;

	F32 upperWeight = 0.f;
	F32 randomWeight = seed * totalWeight;

	S32 lastIndex = -1;
	for (S32 i = 0, count = m_ActionItems.Size(); i < count; ++i)
	{
		if (!cycleGroup.HasCyclesForAction(static_cast<CrowdAction>(m_ActionItems[i].Action)))
			continue;

		lastIndex = i;
		upperWeight += m_ActionItems[i].Weight;

		if (upperWeight >= randomWeight)
		{
			BIZ_ASSERT(m_ActionItems[i].Action < E_CROWD_ACTION_COUNT);
			return static_cast<CrowdAction>(m_ActionItems[i].Action);
		}
	}

	if (lastIndex >= 0)
	{
		BIZ_ASSERT(m_ActionItems[lastIndex].Action < E_CROWD_ACTION_COUNT);
		return static_cast<CrowdAction>(m_ActionItems[lastIndex].Action);
	}

	return E_CROWD_ACTION_IDLE;
}

const CrowdAction C_CrowdActionGroup::GetAction(const U32 index) const
{
	BIZ_ASSERT(index < static_cast<U32>(m_ActionItems.Size()));

	return static_cast<CrowdAction>(m_ActionItems[index].Action);
}

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_SPU
C_CrowdActionSelect::C_CrowdActionSelect()
{
}

C_CrowdActionSelect::~C_CrowdActionSelect()
{
}

void C_CrowdActionSelect::Initialise()
{
	m_ActionGroups[E_CROWD_ACTION_GROUP_IDLE].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_IDLE].Add(				E_CROWD_ACTION_IDLE,		2.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_IDLE].Add(				E_CROWD_ACTION_CLAP,		0.005f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_IDLE].Add(				E_CROWD_ACTION_LOOK,		0.15f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_IDLE].Add(				E_CROWD_ACTION_TAKE_PHOTO,	0.75f);

	m_ActionGroups[E_CROWD_ACTION_GROUP_INTERESTED].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_INTERESTED].Add(		E_CROWD_ACTION_IDLE,		0.3f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_INTERESTED].Add(		E_CROWD_ACTION_CLAP,		1.8f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_INTERESTED].Add(		E_CROWD_ACTION_CELEBRATE,	0.1f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_INTERESTED].Add(		E_CROWD_ACTION_LOOK,		1.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_INTERESTED].Add(		E_CROWD_ACTION_TAKE_PHOTO,	1.25f);

	m_ActionGroups[E_CROWD_ACTION_GROUP_EXCITED].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_EXCITED].Add(			E_CROWD_ACTION_IDLE,		0.1f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_EXCITED].Add(			E_CROWD_ACTION_CLAP,		2.2f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_EXCITED].Add(			E_CROWD_ACTION_CELEBRATE,	0.8f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_EXCITED].Add(			E_CROWD_ACTION_LOOK,		0.5f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_EXCITED].Add(			E_CROWD_ACTION_TAKE_PHOTO,	1.75f);
	
	m_ActionGroups[E_CROWD_ACTION_GROUP_ECSTATIC].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_ECSTATIC].Add(			E_CROWD_ACTION_CLAP,		4.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_ECSTATIC].Add(			E_CROWD_ACTION_CELEBRATE,	3.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_ECSTATIC].Add(			E_CROWD_ACTION_TAKE_PHOTO,	2.f);

	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_NEAR].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_NEAR].Add(		E_CROWD_ACTION_IDLE,		0.001f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_NEAR].Add(		E_CROWD_ACTION_RECOIL,		10.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_NEAR].Add(		E_CROWD_ACTION_DISAGREE,	0.025f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_NEAR].Add(		E_CROWD_ACTION_CELEBRATE,	0.01f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_NEAR].Add(		E_CROWD_ACTION_TAKE_PHOTO,	0.025f);

	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_FAR].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_FAR].Add(		E_CROWD_ACTION_IDLE,		0.01f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_FAR].Add(		E_CROWD_ACTION_DISAGREE,	0.5f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_FAR].Add(		E_CROWD_ACTION_LOOK,		1.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_DANGER_FAR].Add(		E_CROWD_ACTION_TAKE_PHOTO,	1.f);

	m_ActionGroups[E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER].Add(	E_CROWD_ACTION_IDLE,		0.01f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER].Add(	E_CROWD_ACTION_DISAGREE,	0.75f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER].Add(	E_CROWD_ACTION_LOOK,		0.25f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER].Add(	E_CROWD_ACTION_RECOIL,		1.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER].Add(	E_CROWD_ACTION_TAKE_PHOTO,	0.01f);

	m_ActionGroups[E_CROWD_ACTION_GROUP_UNHAPPY].Clear();
	m_ActionGroups[E_CROWD_ACTION_GROUP_UNHAPPY].Add(			E_CROWD_ACTION_IDLE,		0.01f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_UNHAPPY].Add(			E_CROWD_ACTION_DISAGREE,	1.f);
	m_ActionGroups[E_CROWD_ACTION_GROUP_UNHAPPY].Add(			E_CROWD_ACTION_TAKE_PHOTO,	0.5f);
}
#endif // !PLATFORM_SPU

const CrowdAction C_CrowdActionSelect::SelectAction(const C_CrowdCycleGroup& cycleGroup, const CrowdActionGroup actionGroup, const F32 seed) const
{
	return m_ActionGroups[actionGroup].ChooseAction(cycleGroup, seed);
}

const CrowdAction C_CrowdActionSelect::SelectReaction(const C_CrowdCycleGroup& cycleGroup, const CrowdEventType eventType, const F32 seed, const CrowdGlobalMood globalMood, const F32 distance) const
{
	CrowdActionGroup actionGroup = E_CROWD_ACTION_GROUP_IDLE;

	switch (eventType)
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
		actionGroup = E_CROWD_ACTION_GROUP_UNHAPPY;
		break;
	case E_CROWD_EVENT_LOCAL_IDLE:
		break;
	case E_CROWD_EVENT_LOCAL_INTERESTED:
		{
			// TODO: [rthomas]	Make use of distance?
			actionGroup = E_CROWD_ACTION_GROUP_INTERESTED;
		}
		break;
	case E_CROWD_EVENT_LOCAL_EXCITED:
		{
			// TODO: [rthomas]	Make use of distance?
			actionGroup = E_CROWD_ACTION_GROUP_EXCITED;
		}
		break;
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
		{
			// TODO: [rthomas]	Make use of distance?
			actionGroup = E_CROWD_ACTION_GROUP_ECSTATIC;
		}
		break;
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		{
			// TODO: [rthomas]	Make use of distance to decide whether actor should:
			//					a)	Anticipate a crash,
			//					b)	Take a photo of some crash in the distance,
			//					c)	Recoil
			actionGroup = E_CROWD_ACTION_GROUP_DANGER_NEAR;
		}
		break;
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
		actionGroup = E_CROWD_ACTION_GROUP_UNHAPPY;
		break;
	case E_CROWD_EVENT_GLOBAL_IDLE:
		break;
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
		actionGroup = E_CROWD_ACTION_GROUP_INTERESTED;
		break;
	case E_CROWD_EVENT_GLOBAL_EXCITED:
		actionGroup = E_CROWD_ACTION_GROUP_EXCITED;
		break;
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
		actionGroup = E_CROWD_ACTION_GROUP_ECSTATIC;
		break;
	case E_CROWD_EVENT_STATE_OVERRIDE:
	default:
		{
			if (globalMood == E_CROWD_MOOD_UNHAPPY)
				actionGroup = E_CROWD_ACTION_GROUP_UNHAPPY;
			if (globalMood == E_CROWD_MOOD_EXCITEMENT_LOW)
				actionGroup = E_CROWD_ACTION_GROUP_IDLE;
			else if (globalMood == E_CROWD_MOOD_EXCITEMENT_MEDIUM)
				actionGroup = E_CROWD_ACTION_GROUP_INTERESTED;
			else if (globalMood == E_CROWD_MOOD_EXCITEMENT_HIGH)
				actionGroup = E_CROWD_ACTION_GROUP_EXCITED;
			else if (globalMood == E_CROWD_MOOD_EXCITEMENT_VERY_HIGH)
				actionGroup = E_CROWD_ACTION_GROUP_ECSTATIC;
		}
		break;
	}

	return m_ActionGroups[actionGroup].ChooseAction(cycleGroup, seed);
}

}; // namespace AnimatedCrowd
