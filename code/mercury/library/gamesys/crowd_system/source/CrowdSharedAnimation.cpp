#include "CrowdSystemPch.h"

#include <crowd_system/CrowdSharedAnimation.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdCycle.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdActionSelect.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/CrowdRandom.h>

#include <animation/cycle/Cycle.h>
#include <animation/task/EvaluationState.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
extern Debug::Bool	g_DebugCycles;
extern Debug::Bool	g_DebugRenderForceLoopCurrentCycle;
#endif

extern Debug::Float	g_MinReactionTime;
extern Debug::Float	g_MaxReactionTime;
extern Debug::Float g_RandomStartTime;
#endif // !PLATFORM_SPU

#ifndef PLATFORM_SPU
C_CrowdSharedAnimation::C_CrowdSharedAnimation()
	: m_BehaviourIndex(0)
{ }

C_CrowdSharedAnimation::~C_CrowdSharedAnimation()
{
	m_Animation.ClearIndex();
	Destroy();
}

bool C_CrowdSharedAnimation::Create(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
									const U16 behaviourIndex, const Animation::C_StringId* pEventStringIds, const U32 numEvents, const U8 index)
{
	bool success = m_Animation.Create(poseFormat, pSkeleton, pEventStringIds, numEvents, index);

	m_BehaviourIndex		= behaviourIndex;

	m_NextActionCountdown	= 0.f;

	m_CurrentAction			= E_CROWD_ACTION_UNKNOWN;
	m_CurrentActionPriority	= E_ACTION_PRIORITY_LOWEST;
	
	m_NextAction			= E_CROWD_ACTION_IDLE;
	m_NextActionPriority	= E_ACTION_PRIORITY_LOW;
	m_NextActionIsQueued	= 0;

	m_NextEventType			= E_CROWD_EVENT_UNKNOWN;

	return success;
}

void C_CrowdSharedAnimation::Destroy()
{
	m_BehaviourIndex		= 0;

	m_NextActionCountdown	= 0.f;

	m_CurrentAction			= E_CROWD_ACTION_UNKNOWN;
	m_CurrentActionPriority	= E_ACTION_PRIORITY_LOWEST;
	
	m_NextAction			= E_CROWD_ACTION_IDLE;
	m_NextActionPriority	= E_ACTION_PRIORITY_LOW;
	m_NextActionIsQueued	= 0;

	m_NextEventType			= E_CROWD_EVENT_UNKNOWN;

	m_Animation.ClearIndex();
	m_Animation.Destroy();
}

void C_CrowdSharedAnimation::Update(const C_UpdateContext& context, const Animation::C_JointTransform& hipJointTransform, const CrowdGlobalMood globalMood)
{
	if (!m_Animation.IsDirty() && (m_Animation.GetNumCycles() >= 0))
		return;

	bool playNextAction = false;

	if (m_NextActionIsQueued)
	{
		playNextAction = (m_Animation.Finished(0.75f /*g_BlendTime*/) || ((m_NextActionPriority > m_CurrentActionPriority) && (m_Animation.GetNumCycles() < CROWD_MAX_BLENDED_CYCLES)));
	}
	else
	{
		playNextAction = ((m_NextActionCountdown <= 0.f) && (m_Animation.GetNumCycles() < CROWD_MAX_BLENDED_CYCLES));
		m_NextActionCountdown -= context.m_DeltaTime;
	}

	if (playNextAction)
	{
		PlayNextAnimation(context.m_Resources->GetPopulation().GetCycleGroupDataList(), context.m_Resources->GetPopulation().GetActionSelect(),
			globalMood, 0.75f /*g_BlendTime*/, (m_Animation.GetNumCycles() == 0));
	}

	m_Animation.Update(context.m_DeltaTime, *context.m_Resources, context.m_HipJointIndex, hipJointTransform);
}

void C_CrowdSharedAnimation::HandleEvent(const C_CrowdResources* pResources, const C_Event& crowdEvent, const CrowdGlobalMood globalMood)
{
	const C_CrowdActionSelect& actionSelect = pResources->GetPopulation().GetActionSelect();
	const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList = pResources->GetPopulation().GetCycleGroupDataList();

	switch (crowdEvent.GetType())
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
	case E_CROWD_EVENT_LOCAL_IDLE:
	case E_CROWD_EVENT_LOCAL_INTERESTED:
	case E_CROWD_EVENT_LOCAL_EXCITED:
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		break; // Shared animations care not for such things :)
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
	case E_CROWD_EVENT_GLOBAL_IDLE:
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
	case E_CROWD_EVENT_GLOBAL_EXCITED:
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
		{
			SelectNextAction(actionSelect, cycleGroupDataList,
				crowdEvent.GetType(),
				E_ACTION_PRIORITY_HIGH,
				E_ACTION_FLAG_DISCARD_IF_LOWER_PRIORITY | E_ACTION_FLAG_DISCARD_IF_SAME_ACTION,
				0.1f, 0.5f,
				globalMood, 0.f
			);
		}
		break;
	case E_CROWD_EVENT_STATE_OVERRIDE:
		{
			SelectNextAction(actionSelect, cycleGroupDataList,
				crowdEvent.GetType(),
				E_ACTION_PRIORITY_HIGH,
				E_ACTION_FLAG_FORCE_QUEUE,
				g_MinReactionTime, g_MaxReactionTime,
				globalMood, 0.f
			);
		}
		break;
	default:
		break;
	}
}
#endif // !PLATFORM_SPU

const U16 C_CrowdSharedAnimation::GetBehaviourIndex() const
{
	return m_BehaviourIndex;
}

#ifndef PLATFORM_SPU
bool C_CrowdSharedAnimation::Flush()
{
	return m_Animation.Flush();
}

const Animation::C_Cycle* C_CrowdSharedAnimation::ChooseCycle(const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
															  const CrowdAction action, const Animation::C_Cycle* pCurrentCycle, F32& cycleDuration)
{
	BIZ_ASSERT(m_BehaviourIndex < static_cast<U32>(cycleGroupDataList.Size()));

	const Animation::C_Cycle* pCycle	= NULL;
	const C_CrowdCycle* pCrowdCycle		= NULL;

	S32 cycleCount = cycleGroupDataList[m_BehaviourIndex].m_CycleGroup.GetNumCycles();
	
	if (!cycleCount)
		return NULL;

	F32 totalWeight = cycleGroupDataList[m_BehaviourIndex].m_CycleGroup.GetTotalWeight(action);

	if (totalWeight > 0.f)
	{
		S32 cycleIndex = 0;

		F32 accumulatedWeight = 0.f;
		F32 randomWeight = AnimatedCrowd::C_Random::Instance().RandomF32(totalWeight);

		while (accumulatedWeight < randomWeight && cycleIndex < cycleCount)
		{
			const CrowdCycleData& cycleData = cycleGroupDataList[m_BehaviourIndex].m_CycleGroup.GetCycleData(cycleIndex++);

			if (cycleData.m_Action == static_cast<U32>(action))
			{
				if (cycleData.m_pCycle && cycleData.m_Valid)
				{
					pCycle = cycleData.m_pCycle;
					cycleDuration = cycleData.m_Duration;
				}
				accumulatedWeight += cycleData.m_Weight;
			}
		}
	}

	return pCycle;
}

void C_CrowdSharedAnimation::PlayNextAnimation(const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
											   const C_CrowdActionSelect& actionSelect, const CrowdGlobalMood globalMood, const F32 blendTime, const bool firstAnimation)
{
	CrowdAction action = static_cast<CrowdAction>(m_NextAction);

	BIZ_ASSERT(m_NextAction <= E_CROWD_ACTION_UNKNOWN);

	if (m_NextAction == E_CROWD_ACTION_UNKNOWN)
		action = E_CROWD_ACTION_IDLE;

	bool spawnAction = (m_CurrentAction == E_CROWD_ACTION_UNKNOWN);

	m_CurrentActionPriority	= m_NextActionPriority;
	m_CurrentAction			= action;

	const Animation::C_Cycle* pCurrentCycle = NULL;

#ifndef CFG_GOLD
	if (g_DebugCycles)
		pCurrentCycle = GetCurrentCycle();
#endif

	F32 cycleDuration = 0.f;
	const Animation::C_Cycle* pCycle = ChooseCycle(cycleGroupDataList, action, pCurrentCycle, cycleDuration);

	if (pCycle && pCycle->IsValid())
	{
		bool looped = false;
		m_Animation.PushCycle(pCycle, cycleDuration, blendTime, (looped ? C_CrowdAnimation::E_TIME_MODE_LOOPING : C_CrowdAnimation::E_TIME_MODE_NORMAL),
			firstAnimation ? AnimatedCrowd::C_Random::Instance().RandomF32(g_RandomStartTime) : 0.f, C_CrowdAnimation::E_TIME_FORMAT_SECONDS, false);
	}

	SelectNextAction(actionSelect, cycleGroupDataList, E_CROWD_EVENT_GLOBAL_IDLE, E_ACTION_PRIORITY_LOWEST, E_ACTION_FLAG_FORCE_QUEUE, 0.f, 0.f, globalMood);
}

void C_CrowdSharedAnimation::SelectNextAction(const C_CrowdActionSelect& actionSelect, const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
											  const CrowdEventType eventType, const CrowdActionPriority actionPriority, const U32 actionFlags,
											  const F32 minDelayTime, const F32 maxDelayTime, const CrowdGlobalMood globalMood, const F32 dist)
{
	if (eventType == static_cast<CrowdEventType>(m_NextEventType))
		return;

	bool queueNextAction = false;

	if (actionFlags & E_ACTION_FLAG_FORCE_QUEUE)
	{
		queueNextAction = true;
	}
	else
	{
		if (static_cast<U32>(actionPriority) <= m_CurrentActionPriority)
		{
			if (actionFlags & E_ACTION_FLAG_DISCARD_IF_LOWER_PRIORITY)
				return;
			else if (actionFlags & E_ACTION_FLAG_QUEUE_IF_LOWER_PRIORITY)
				queueNextAction = true;
		}
		else
		{
			queueNextAction = true;
		}
	}

	CrowdAction action = actionSelect.SelectReaction(cycleGroupDataList[m_BehaviourIndex].m_CycleGroup, eventType, AnimatedCrowd::C_Random::Instance().RandomF32(), globalMood, dist);

	if ((actionFlags & E_ACTION_FLAG_DISCARD_IF_SAME_ACTION) && m_CurrentAction == action)
		return;

	m_NextAction = action;
	m_NextActionPriority = actionPriority;

	BIZ_ASSERT(m_NextAction <= E_CROWD_ACTION_UNKNOWN);

	if (m_CurrentAction != E_CROWD_ACTION_UNKNOWN)
		m_NextActionCountdown = minDelayTime + (maxDelayTime - minDelayTime) * AnimatedCrowd::C_Random::Instance().RandomF32(0.1f, 0.75f);
	else
		m_NextActionCountdown = 0.f;

	if (!queueNextAction)
		m_NextActionCountdown += m_Animation.GetBlendTimeRemaining();

	m_NextActionIsQueued	= queueNextAction ? 1 : 0;
	m_NextEventType			= eventType;
}

const Animation::C_Cycle* C_CrowdSharedAnimation::GetCurrentCycle() const
{
	if (m_Animation.GetCurrentCycleInstance())
		return m_Animation.GetCurrentCycleInstance()->GetCycle();
	return NULL;
}
#endif // !PLATFORM_SPU

}; // namespace AnimatedCrowd
