#include "CrowdSystemPch.h"

#include <crowd_system/CrowdSpectator.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdContext.h>
#include <crowd_system/CrowdAnimation.h>
#include <crowd_system/CrowdSharedAnimation.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/Effects/CrowdFlags.h>
#endif // !PLATFORM_SPU
#include <crowd_system/CrowdCycle.h>
#include <crowd_system/CrowdRandom.h>
#include <crowd_system/CrowdSharedAnimationPool.h>
#include <crowd_system/CrowdUniqueAnimationPool.h>
#include <crowd_system/CrowdSpectatorUpdateData.h>

#ifndef PLATFORM_SPU
#include <crowd_system/CrowdBlockRef.h>
#include <game_resource/CrowdData.h>
#endif // !PLATFORM_SPU

#include <animation/cycle/Cycle.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseMatrixCache.h>

#ifndef PLATFORM_SPU
#include <system/Perf.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU

#ifndef CFG_GOLD
extern Debug::Bool	g_DebugCycles;
#endif // CFG_GOLD

extern Debug::Float	g_MinReactionTime;
extern Debug::Float	g_MaxReactionTime;

#ifndef CFG_GOLD
extern Debug::Bool g_DebugRenderForceLoopCurrentCycle;
#endif // CFG_GOLD

#endif // !PLATFORM_SPU

///////////////////////////////////////////////////////////////////////////////////////

inline const CrowdDetailBlendType GetDetailBlendType(const U32 src, const U32 dest, const S32 localAnimation, const S32 sharedAnimation)
{
	if (src != dest)
	{
		if (dest == E_DETAIL_LEVEL_LOW)
			return E_BLEND_HIGH_LOW;
		else
			return E_BLEND_LOW_HIGH;
	}

	if (localAnimation >= 0)
	{
		if (dest == E_DETAIL_LEVEL_LOW)
			return E_BLEND_HIGH_LOW;
	}

	if (sharedAnimation >= 0)
	{
		if (dest == E_DETAIL_LEVEL_HIGH)
			return E_BLEND_LOW_HIGH;
	}

	return E_NO_BLEND;
}

///////////////////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_SPU
C_CrowdSpectator::C_CrowdSpectator()
	:
	m_BehaviourIndex(0)
	, m_LocalAnimation(-1)
	, m_SharedAnimation(-1)
	, m_InstanceIndex(0)
	, m_InstanceUniqueId(-1)
	, m_DetailLevel(E_DETAIL_LEVEL_LOW)
	, m_DestDetailLevel(E_DETAIL_LEVEL_LOW)
	, m_DetailBlendStart(0.f)
	, m_DetailBlendTime(0.75f /*g_BlendTime*/)
	, m_DetailBlendCounter(0.f)
	, m_ForceShared(0)
{ }

C_CrowdSpectator::~C_CrowdSpectator()
{ }

void C_CrowdSpectator::Create(const SpectatorSpawnParams& spawnParams)
{
	// CrowdMember
	SetDirection(spawnParams.m_Direction);

	SetPosition(spawnParams.m_Position);

	m_BehaviourIndex = spawnParams.m_BehaviourIndex;

	// CrowdAnimatedMember
	if (spawnParams.m_pResources)
	{
		m_LocalAnimation = spawnParams.m_UniqueAnimation;
		m_SharedAnimation = spawnParams.m_SharedAnimation;

		if (m_SharedAnimation >= 0)
			spawnParams.m_pResources->GetSharedAnimationPool()->AddRef(m_SharedAnimation);
	}

	m_InstanceIndex			= spawnParams.m_InstanceIndex;
	m_InstanceUniqueId		= spawnParams.m_InstanceUniqueId;

	m_NextActionCountdown	= 0.f;

	m_CurrentAction			= E_CROWD_ACTION_UNKNOWN;
	m_CurrentActionPriority	= E_ACTION_PRIORITY_LOWEST;
	
	m_NextAction			= E_CROWD_ACTION_IDLE;
	m_NextActionPriority	= E_ACTION_PRIORITY_LOW;
	m_NextActionIsQueued	= 0;

	m_ForceShared			= spawnParams.m_ForceShared;

	m_FlagTextureIndex		= static_cast<U32>(spawnParams.m_FlagTextureIndex);

	m_NextEventType			= E_CROWD_EVENT_UNKNOWN;
}

void C_CrowdSpectator::Destroy(C_CrowdResources& resources)
{
	// CrowdAnimatedMember
	m_LocalAnimation = -1;

	if (m_SharedAnimation >= 0)
		resources.GetSharedAnimationPool()->RemoveRef(m_SharedAnimation);

	m_SharedAnimation = -1;
}
#endif // !PLATFORM_SPU

void C_CrowdSpectator::Update(const C_SpectatorUpdateInput& input, C_SpectatorUpdateOutput& output)
{
	const CrowdDetailBlendType blendType = GetDetailBlendType(m_DetailLevel, m_DestDetailLevel, m_LocalAnimation, m_SharedAnimation);

	F32 detailBlendAmount = 0.f;

	// TODO: [rthomas]	Simplify!!!!
	//	For the purposes of blending local and shared, m_DetailBlendAmount of 1 means local, m_DetailBlendAmount of 0 means shared.
	//	So 1.f - m_DetailBlendAmount for one of them and let pointer checks take over after that (m_pLocalAnimation != NULL etc.)
	if (blendType != E_NO_BLEND)
	{
		m_DetailBlendCounter += input.m_fDeltaTime;

		if (m_DetailBlendCounter >= m_DetailBlendStart)
		{
			if (!HasLocalAnimation())
			{
				S32 localAnimation = -1;
				S32 sharedAnimation = -1;
				if (output.m_pUniqueAnimationPool->GetNumFreeAnimations())
					localAnimation = output.m_pUniqueAnimationPool->CreateAnimation(*input.m_pPoseFormat, input.m_pSkeleton, GetBehaviourIndex(), input.m_pEventStringIds, input.m_NumEvents);
				else
				{
					if (!HasSharedAnimation())
						sharedAnimation = output.m_pSharedAnimationPool->CreateAnimation(*input.m_pPoseFormat, input.m_pSkeleton, GetBehaviourIndex(), input.m_pEventStringIds, input.m_NumEvents);
				}

				if (localAnimation >= 0)
					SetLocalAnimation(localAnimation);
				else
				{
					ForceDetailLevel(E_DETAIL_LEVEL_LOW, output.m_pUniqueAnimationPool, output.m_pSharedAnimationPool);
					if (sharedAnimation >= 0)
						SetSharedAnimation(sharedAnimation, output.m_pSharedAnimationPool);
				}
			}

			if (!HasSharedAnimation())
			{
				S32 sharedAnimation = output.m_pSharedAnimationPool->CreateAnimation(*input.m_pPoseFormat, input.m_pSkeleton, GetBehaviourIndex(), input.m_pEventStringIds, input.m_NumEvents);

				if (sharedAnimation >= 0)
					SetSharedAnimation(sharedAnimation, output.m_pSharedAnimationPool);
			}

			detailBlendAmount = (m_DetailBlendCounter - m_DetailBlendStart) / m_DetailBlendTime;
			if (blendType == E_BLEND_HIGH_LOW)
				detailBlendAmount = 1.f - detailBlendAmount;
		}

		const F32 epsilon = Maths::Constants<F32>::Epsilon();
		if (detailBlendAmount <= epsilon && m_DestDetailLevel == E_DETAIL_LEVEL_LOW)
		{
			m_DetailLevel = m_DestDetailLevel; // Kill off local
			ClearLocalAnimation(output.m_pUniqueAnimationPool);
		}
		if ((1.f - detailBlendAmount) <= epsilon && m_DestDetailLevel == E_DETAIL_LEVEL_HIGH)
		{
			m_DetailLevel = m_DestDetailLevel; // Kill off shared
			ClearSharedAnimation(output.m_pSharedAnimationPool);
		}

		detailBlendAmount = Maths::Clamped<F32>(detailBlendAmount, 0.f, 1.f);
	}
	else
	{
		if (!HasLocalAnimation() && !HasSharedAnimation())
		{
			S32 localAnimation = -1;
			S32 sharedAnimation = -1;
			if ((static_cast<CrowdDetailLevel>(m_DetailLevel) == E_DETAIL_LEVEL_HIGH) && output.m_pUniqueAnimationPool->GetNumFreeAnimations())
				localAnimation = output.m_pUniqueAnimationPool->CreateAnimation(*input.m_pPoseFormat, input.m_pSkeleton, GetBehaviourIndex(), input.m_pEventStringIds, input.m_NumEvents);
			else
				sharedAnimation = output.m_pSharedAnimationPool->CreateAnimation(*input.m_pPoseFormat, input.m_pSkeleton, GetBehaviourIndex(), input.m_pEventStringIds, input.m_NumEvents);

			if (localAnimation >= 0)
				SetLocalAnimation(localAnimation);
			else
			{
				ForceDetailLevel(E_DETAIL_LEVEL_LOW, output.m_pUniqueAnimationPool, output.m_pSharedAnimationPool);

				SetSharedAnimation(sharedAnimation, output.m_pSharedAnimationPool);
			}
		}
	}

	Update(input, output, detailBlendAmount);
}

#ifndef PLATFORM_SPU
void C_CrowdSpectator::HandleEvent(const C_CrowdResources* pResources, const C_Event& crowdEvent, const CrowdGlobalMood globalMood)
{
	BIZ_ASSERT(pResources);

	if (!HasLocalAnimation())
		return;

	if (!crowdEvent.AffectsSpectator(m_Position, m_Direction, 1.309f)) // 60 degrees
		return;

	const C_CrowdAnimation* pLocalAnimation = pResources->GetUniqueAnimationPool().GetAnimation(m_LocalAnimation);
	if (pLocalAnimation && pLocalAnimation->GetIndex() == 0xff)
		pLocalAnimation = NULL;

	const C_CrowdActionSelect& actionSelect = pResources->GetPopulation().GetActionSelect();
	const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList = pResources->GetPopulation().GetCycleGroupDataList();

	switch (crowdEvent.GetType())
	{
	case E_CROWD_EVENT_LOCAL_UNHAPPY:
		{
			SelectNextAction(pLocalAnimation, actionSelect, cycleGroupDataList, AnimatedCrowd::C_Random::Instance(),
				crowdEvent.GetType(),
				E_ACTION_PRIORITY_HIGH,
				E_ACTION_FLAG_DISCARD_IF_LOWER_PRIORITY | E_ACTION_FLAG_DISCARD_IF_SAME_ACTION,
				g_MinReactionTime, g_MaxReactionTime,
				globalMood, 0.f
			);
		}
		break;
	case E_CROWD_EVENT_LOCAL_IDLE:
	case E_CROWD_EVENT_LOCAL_INTERESTED:
		{
			SelectNextAction(pLocalAnimation, actionSelect, cycleGroupDataList, AnimatedCrowd::C_Random::Instance(),
				crowdEvent.GetType(),
				E_ACTION_PRIORITY_MEDIUM,
				E_ACTION_FLAG_DISCARD_IF_LOWER_PRIORITY | E_ACTION_FLAG_DISCARD_IF_SAME_ACTION,
				g_MinReactionTime, g_MaxReactionTime,
				globalMood, 0.f
			);
		}
		break;
	case E_CROWD_EVENT_LOCAL_EXCITED:
	case E_CROWD_EVENT_LOCAL_ECSTATIC:
		{
			SelectNextAction(pLocalAnimation, actionSelect, cycleGroupDataList, AnimatedCrowd::C_Random::Instance(),
				crowdEvent.GetType(),
				E_ACTION_PRIORITY_HIGH,
				E_ACTION_FLAG_DISCARD_IF_LOWER_PRIORITY | E_ACTION_FLAG_DISCARD_IF_SAME_ACTION,
				g_MinReactionTime, g_MaxReactionTime,
				globalMood, 0.f
			);
		}
		break;
	case E_CROWD_EVENT_LOCAL_REACT_TO_DANGER:
		{
			SelectNextAction(pLocalAnimation, actionSelect, cycleGroupDataList, AnimatedCrowd::C_Random::Instance(),
				crowdEvent.GetType(),
				E_ACTION_PRIORITY_HIGHEST,
				E_ACTION_FLAG_DISCARD_IF_LOWER_PRIORITY | E_ACTION_FLAG_DISCARD_IF_SAME_ACTION,
				0.f, 0.f,
				globalMood, 0.f
			);
		}
		break;
	case E_CROWD_EVENT_GLOBAL_UNHAPPY:
	case E_CROWD_EVENT_GLOBAL_IDLE:
	case E_CROWD_EVENT_GLOBAL_INTERESTED:
	case E_CROWD_EVENT_GLOBAL_EXCITED:
	case E_CROWD_EVENT_GLOBAL_ECSTATIC:
		break; // Local animations care not for such things :)
	case E_CROWD_EVENT_STATE_OVERRIDE:
		{
			SelectNextAction(pLocalAnimation, actionSelect, cycleGroupDataList, AnimatedCrowd::C_Random::Instance(),
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

void C_CrowdSpectator::PlayNextAnimation(C_CrowdAnimation* pLocalAnimation, const C_CrowdActionSelect& actionSelect,
										 const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
										 const CrowdGlobalMood globalMood, AnimatedCrowd::C_Random& random, const F32 blendTime)
{
	CrowdAction action = static_cast<CrowdAction>(m_NextAction);

	BIZ_ASSERT(m_NextAction <= E_CROWD_ACTION_UNKNOWN);

	if (m_NextAction == E_CROWD_ACTION_UNKNOWN)
		action = E_CROWD_ACTION_IDLE;

	bool spawnAction = (m_CurrentAction == E_CROWD_ACTION_UNKNOWN);

	m_CurrentActionPriority = m_NextActionPriority;
	m_CurrentAction = action;

	const Animation::C_Cycle* pCurrentCycle = NULL;

	F32 cycleDuration = 0.f;
	const Animation::C_Cycle* pCycle = ChooseCycle(cycleGroupDataList, action, pCurrentCycle, random, cycleDuration);

	if (pCycle)
	{
#ifndef PLATFORM_SPU
		BIZ_ASSERT(pCycle->IsValid()); // ...will have to assume it is valid for now :)
#endif // !PLATFORM_SPU
		
		PlayLocalAnimation(pLocalAnimation, pCycle, cycleDuration, blendTime);
	}

	SelectNextAction(pLocalAnimation, actionSelect, cycleGroupDataList, random, E_CROWD_EVENT_LOCAL_IDLE, E_ACTION_PRIORITY_LOWEST, E_ACTION_FLAG_FORCE_QUEUE, 0.f, 0.f, globalMood);
}

void C_CrowdSpectator::SelectNextAction(const C_CrowdAnimation* pLocalAnimation, const C_CrowdActionSelect& actionSelect,
										const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
										AnimatedCrowd::C_Random& random, const CrowdEventType eventType, const CrowdActionPriority actionPriority, const U32 actionFlags,
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

	CrowdAction action = actionSelect.SelectReaction(cycleGroupDataList[m_BehaviourIndex].m_CycleGroup, eventType, random.RandomF32(), globalMood, dist);

	if ((actionFlags & E_ACTION_FLAG_DISCARD_IF_SAME_ACTION) && m_CurrentAction == action)
		return;

	m_NextAction = action;
	m_NextActionPriority = actionPriority;

	BIZ_ASSERT(m_NextAction <= E_CROWD_ACTION_UNKNOWN);

	if (m_CurrentAction != E_CROWD_ACTION_UNKNOWN)
		m_NextActionCountdown = minDelayTime + (maxDelayTime - minDelayTime) * random.RandomF32(0.1f, 0.75f);
	else
		m_NextActionCountdown = 0.f;

	if (!queueNextAction)
		m_NextActionCountdown += pLocalAnimation->GetBlendTimeRemaining();

	m_NextActionIsQueued	= queueNextAction ? 1 : 0;
	m_NextEventType			= eventType;
}

void C_CrowdSpectator::Update(const C_SpectatorUpdateInput& input, C_SpectatorUpdateOutput& output, const F32 animationBlendFactor)
{
	BIZ_ASSERT(HasLocalAnimation() || HasSharedAnimation());

	const Maths::Mat43& transform = GetTransform(input.m_ZDir);

	C_CrowdSharedAnimation* pSharedAnimation = NULL;

	if (HasSharedAnimation())
	{
		pSharedAnimation = output.m_pSharedAnimationPool->GetAnimation(m_SharedAnimation);

		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
		{
			pSharedAnimation->GetAnimation().SetDirty();
			if (!HasLocalAnimation() && HasPose(&pSharedAnimation->GetAnimation()))
				SpawnEffects(&pSharedAnimation->GetAnimation(), true, *output.m_pEffectSpawnData, true, *input.m_pAnimationEventEffectJointIndexes, transform, *output.m_pRandom);

			if (GetBehaviourIndex() == input.m_FlagBehaviourIndex)
			{
				FlagInstance& flagInstance = output.m_pFlagInstanceData->m_Instances[output.m_pFlagInstanceData->m_NumInstances++];
				flagInstance.m_Transform = transform;
				flagInstance.m_InstanceIndex = m_InstanceIndex;
				flagInstance.m_AnimationIndex = pSharedAnimation->GetAnimation().GetIndex(); // TODO: [rthomas]	GetIndex == m_SharedAnimation???
				flagInstance.m_TextureIndex = static_cast<S32>(m_FlagTextureIndex);
			}
		}
	}

	if (!HasLocalAnimation()) // TODO: [rthomas]	Check for zombie flag?
		return;

	C_CrowdAnimation* pLocalAnimation = output.m_pUniqueAnimationPool->GetAnimation(m_LocalAnimation);

	if (pLocalAnimation && pLocalAnimation->GetIndex() != 0xff)
	{
		if (HasPose(pLocalAnimation))
			SpawnEffects(pLocalAnimation, false, *output.m_pEffectSpawnData, false, *input.m_pAnimationEventEffectJointIndexes, transform, *output.m_pRandom);

		bool playNextAction = false;

		if (m_NextActionIsQueued)
		{
			if (IsLocalAnimFinished(pLocalAnimation, 0.75f /*g_BlendTime*/) || ((m_NextActionPriority > m_CurrentActionPriority) && (pLocalAnimation->GetNumCycles() < CROWD_MAX_BLENDED_CYCLES)))
				playNextAction = true;
		}
		else
		{
			playNextAction = ((m_NextActionCountdown <= 0.f) && (pLocalAnimation->GetNumCycles() < CROWD_MAX_BLENDED_CYCLES));
			m_NextActionCountdown -= input.m_fDeltaTime;
		}

		if (playNextAction)
			PlayNextAnimation(pLocalAnimation, *input.m_pActionSelect, *input.m_pCycleGroupDataList, static_cast<CrowdGlobalMood>(input.m_GlobalMood), *output.m_pRandom, 0.75f /*g_BlendTime*/);

		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
		{
			BIZ_ASSERT(m_SharedAnimation == pSharedAnimation->GetAnimation().GetIndex());
			pLocalAnimation->BlendAnimation(pSharedAnimation->GetAnimation().GetIndex(), animationBlendFactor);
		}
			
		pLocalAnimation->SetDirty();
	}
}

void C_CrowdSpectator::SpawnEffects(const C_CrowdAnimation* pAnimation, const bool sharedAnim, EffectSpawnData& effectSpawnData, const bool randomStartTime,
									const Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >& animationEventEffectJointIndexes, const Maths::HMat43& transform, AnimatedCrowd::C_Random& random) const
{
	if (effectSpawnData.m_NumItems < CROWD_EFFECT_SPAWN_BUFFER_SIZE)
	{
		if (!pAnimation)
			return;

		if (!pAnimation->CanSpawnAnyEffects())
			return;

		BIZ_ASSERT(pAnimation->GetIndex() != 0xff);

		for (U32 i = 0, count = static_cast<U32>(animationEventEffectJointIndexes.Size()); i < count; ++i)
		{
			if (pAnimation->CanSpawnEffect(i))
			{
				EffectSpawnItem effectSpawnItem;
				effectSpawnItem.EventIndex = i;
				effectSpawnItem.AnimPool = sharedAnim ? 1 : 2;
				effectSpawnItem.AnimIndex = pAnimation->GetIndex();
				effectSpawnItem.JointIndex = animationEventEffectJointIndexes[i];
				effectSpawnItem.Transform = transform;
				effectSpawnItem.StartTime = randomStartTime ? random.RandomF32(0.f, 0.08f) : 0.f;

				effectSpawnData.AddItem(effectSpawnItem);
			}
		}
	}
}

#ifndef PLATFORM_SPU
const Animation::C_Cycle* C_CrowdSpectator::GetCurrentCycle(const C_CrowdResources& resources) const
{
	if (HasLocalAnimation())
	{
		const C_CrowdAnimation* pAnimation = resources.GetUniqueAnimationPool().GetAnimation(m_LocalAnimation);
		if (pAnimation && (pAnimation->GetIndex() != 0xff) && pAnimation->GetCurrentCycleInstance())
			return pAnimation->GetCurrentCycleInstance()->GetCycle();
	}
	return NULL;
}
#endif // !PLATFORM_SPU

const U32 C_CrowdSpectator::GetInstanceIndex() const
{
	return m_InstanceIndex;
}

const S16 C_CrowdSpectator::GetInstanceUniqueId() const
{
	return m_InstanceUniqueId;
}

#ifndef PLATFORM_SPU
const Animation::C_PoseMatrixCache* C_CrowdSpectator::GetPoseMatrixCacheNonBlocking(const C_CrowdResources& resources) const
{
	if (HasLocalAnimation())
	{
		const C_CrowdAnimation* pAnimation = resources.GetUniqueAnimationPool().GetAnimation(m_LocalAnimation);
		if (pAnimation && (pAnimation->GetIndex() != 0xff) && pAnimation->IsValid())
			return pAnimation->GetPoseMatrixCacheNonBlocking();
	}
	else if (HasSharedAnimation())
	{
		const C_CrowdSharedAnimation* pSharedAnimation = resources.GetSharedAnimationPool().GetAnimation(m_SharedAnimation);
		if (pSharedAnimation && (pSharedAnimation->GetAnimation().GetIndex() != 0xff) && pSharedAnimation->GetAnimation().IsValid())
			return pSharedAnimation->GetAnimation().GetPoseMatrixCacheNonBlocking();
	}

	return NULL;
}
#endif // !PLATFORM_SPU

void C_CrowdSpectator::ForceDetailLevel(const CrowdDetailLevel detailLevel, C_CrowdUniqueAnimationPool* pUniqueAnimationPool, C_CrowdSharedAnimationPool* pSharedAnimationPool)
{
	m_DetailLevel			= static_cast<U32>(detailLevel);
	m_DestDetailLevel		= static_cast<U32>(detailLevel);
	m_DetailBlendStart		= 0.f;
	m_DetailBlendTime		= 0.75f /*g_BlendTime*/;
	m_DetailBlendCounter	= 0.f;

	if (detailLevel == E_DETAIL_LEVEL_LOW)
		ClearLocalAnimation(pUniqueAnimationPool);
	else if (detailLevel == E_DETAIL_LEVEL_HIGH)
		ClearSharedAnimation(pSharedAnimationPool);
}

void C_CrowdSpectator::SetDetailLevel(const CrowdDetailLevel detailLevel, AnimatedCrowd::C_Random& random)
{
	if (m_DestDetailLevel != detailLevel)
	{
		CrowdDetailBlendType blendType = GetDetailBlendType(m_DetailLevel, detailLevel, m_LocalAnimation, m_SharedAnimation);

		m_DetailBlendTime		= 0.25f /*g_DetailBlendTime*/ + random.RandomF32(0.5f);
		m_DetailBlendStart		= 0.f;
		m_DetailBlendCounter	= 0.f;
	}

	m_DestDetailLevel = static_cast<U32>(detailLevel);
}

const CrowdDetailLevel C_CrowdSpectator::GetDetailLevel() const
{
	return static_cast<CrowdDetailLevel>(m_DetailLevel);
}

const CrowdDetailLevel C_CrowdSpectator::GetDestDetailLevel() const
{
	return static_cast<CrowdDetailLevel>(m_DestDetailLevel);
}

#ifndef PLATFORM_SPU
const Maths::RGBA C_CrowdSpectator::GetColour() const
{
	if (HasLocalAnimation())
	{
		if (HasSharedAnimation())
			return CROWD_COLOUR_RED;
		return CROWD_COLOUR_WHITE;
	}
	
	return CROWD_COLOUR_BLACK;
}

#ifndef CFG_GOLD
const char* C_CrowdSpectator::GetMainCycleName(const C_CrowdResources& resources) const
{
	if (HasLocalAnimation())
	{
		const C_CrowdAnimation* pAnimation = resources.GetUniqueAnimationPool().GetAnimation(m_LocalAnimation);
		if (pAnimation && (pAnimation->GetIndex() != 0xff) && pAnimation->GetCurrentCycleInstance())
			return pAnimation->GetCurrentCycleInstance()->GetCycleName().GetString();
	}
	
	if (HasSharedAnimation())
	{
		const C_CrowdSharedAnimation* pSharedAnimation = resources.GetSharedAnimationPool().GetAnimation(m_SharedAnimation);
		if (pSharedAnimation && (pSharedAnimation->GetAnimation().GetIndex() != 0xff) && pSharedAnimation->GetAnimation().GetCurrentCycleInstance())
			return pSharedAnimation->GetAnimation().GetCurrentCycleInstance()->GetCycleName().GetString();
	}

	return "None";
}
#endif
#endif // !PLATFORM_SPU

void C_CrowdSpectator::SetLocalAnimation(const S32 localAnimation)
{
	BIZ_ASSERT(!HasLocalAnimation());

	m_LocalAnimation = localAnimation;
}

void C_CrowdSpectator::ClearLocalAnimation(C_CrowdUniqueAnimationPool* pUniqueAnimationPool)
{
	if (m_LocalAnimation < 0)
		return;

	if (pUniqueAnimationPool)
	{
		C_CrowdAnimation* pAnimation = pUniqueAnimationPool->GetAnimation(m_LocalAnimation);
		if (pAnimation && pAnimation->GetIndex() != 0xff)
			pAnimation->Zombify();
	}

	m_LocalAnimation = -1;
}

void C_CrowdSpectator::SetSharedAnimation(const S32 sharedAnimation, C_CrowdSharedAnimationPool* pSharedAnimationPool)
{
	BIZ_ASSERT(m_SharedAnimation < 0);
	BIZ_ASSERT(sharedAnimation >= 0);

	m_SharedAnimation = sharedAnimation;
	pSharedAnimationPool->AddRef(sharedAnimation);
}

void C_CrowdSpectator::ClearSharedAnimation(C_CrowdSharedAnimationPool* pSharedAnimationPool)
{
	if (m_SharedAnimation < 0)
		return;

	if (pSharedAnimationPool)
		pSharedAnimationPool->RemoveRef(m_SharedAnimation);

	m_SharedAnimation = -1;
}

// <CrowdMember>
const U16 C_CrowdSpectator::GetBehaviourIndex() const
{
	return m_BehaviourIndex;
}

void C_CrowdSpectator::SetPosition(const Maths::HVec3& position)
{
	m_Position = position;
}

const Maths::HVec3& C_CrowdSpectator::GetPosition() const
{
	return m_Position;
}

void C_CrowdSpectator::SetDirection(const Maths::HVec3 &direction)
{
	m_Direction = direction;
}

const Maths::HVec3& C_CrowdSpectator::GetDirection() const
{
	return m_Direction;
}

const Animation::C_Cycle* C_CrowdSpectator::ChooseCycle(const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
														const CrowdAction action, const Animation::C_Cycle* pCurrentCycle, AnimatedCrowd::C_Random& random, F32& cycleDuration)
{
	BIZ_ASSERT(m_BehaviourIndex < static_cast<U32>(cycleGroupDataList.Size()));

	const Animation::C_Cycle* pCycle = NULL;
	const C_CrowdCycle* pCrowdCycle = NULL;

	S32 cycleCount = cycleGroupDataList[m_BehaviourIndex].m_CycleGroup.GetNumCycles();
		
	if (!cycleCount)
		return NULL;

	F32 totalWeight = cycleGroupDataList[m_BehaviourIndex].m_CycleGroup.GetTotalWeight(action);

	if (totalWeight > 0.f)
	{
		S32 cycleIndex = 0;

		F32 accumulatedWeight = 0.f;
		F32 randomWeight = random.RandomF32(totalWeight);

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

#if 0
const Maths::Mat43 C_CrowdSpectator::GetTransform(const Maths::Vector3& zDir) const
{
	Maths::HMat43 worldMatrix;
	Maths::HVec3 hvZDir(Maths::X(zDir), Maths::Y(zDir), Maths::Z(zDir));

	Maths::HVec3 direction = GetDirection();
	Maths::HVec3 position = GetPosition();
	Maths::HVec3 scale = Maths::HVec3(1.f);

	F32 dot = direction | hvZDir;
	F32 signDot = Maths::Sign<F32>(dot);
	F32 absDot = fabsf(dot);

	if ((1.f - absDot) <= 0.0001f)
	{
		Maths::HVec3 unitY(0.f, 1.f, 0.f);
		F32 pi = 3.141593f;
		Maths::C_Quaternion rotation(unitY, pi * signDot);
		worldMatrix.SetScaleRotTrans(scale, rotation, GetPosition());
	}
	else
	{
		Maths::HVec3 axis = direction ^ hvZDir;
		F32 angle = Maths::ArcCos(dot);
		Maths::C_Quaternion rotation(axis, angle);
		worldMatrix.SetScaleRotTrans(scale, rotation, GetPosition());
	}

	return worldMatrix;
}
#else
const Maths::Mat43 C_CrowdSpectator::GetTransform(const Maths::Vector3& zDir) const
{
	const Maths::Vector3 direction = GetDirection();
	const Maths::Vector3 position = GetPosition();
	
	const Maths::VF32 dot = zDir | direction;
	
	const Maths::VF32 signDot = Maths::Sign(dot);
	const Maths::VF32 absDot = Maths::Abs(dot);

	if ((Maths::Const<Maths::VF32>::One() - absDot) <= Maths::Const<Maths::VF32>::Epsilon())
	{
		const Maths::Vector3 unitY = Maths::Const<Maths::Vector3>::UnitY();
		const Maths::VF32 pi = Maths::Const<Maths::VF32>::Pi();
		const Maths::Mat33 rotation = Maths::MakeMat33_AxisAngle(unitY, pi * signDot);
		return Maths::MakeMat43(rotation, position);
	}
	else
	{
		const Maths::Vector3 axis = zDir ^ direction;
		const Maths::VF32 angle = Maths::ArcCos(dot);
		const Maths::Mat33 rotation = Maths::MakeMat33_AxisAngle(axis, angle);
		return Maths::MakeMat43(rotation, position);
	}
}
#endif
// </CrowdMember>

// <CrowdAnimatedMember>
const U8 C_CrowdSpectator::GetAnimationIndex(const C_CrowdUniqueAnimationPool* pUniqueAnimationPool, const C_CrowdSharedAnimationPool* pSharedAnimationPool) const
{
	if (HasLocalAnimation() && pUniqueAnimationPool)
	{
		const C_CrowdAnimation* pLocalAnimation = pUniqueAnimationPool->GetAnimation(m_LocalAnimation);
		if (pLocalAnimation && pLocalAnimation->GetIndex() != 0xff)
		{
			if (pLocalAnimation->GetNumCycles() && pLocalAnimation->IsValid())
				return (pLocalAnimation->GetIndex() + CROWD_MAX_SHARED_ANIMATIONS);
		}
	}
	if (HasSharedAnimation() && pSharedAnimationPool)
	{
		const C_CrowdSharedAnimation* pSharedAnimation = pSharedAnimationPool->GetAnimation(m_SharedAnimation);
		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
		{
			if (pSharedAnimation->GetAnimation().GetNumCycles())
				return pSharedAnimation->GetAnimation().GetIndex();
		}
	}

	return 0xff;
}

void C_CrowdSpectator::PlayLocalAnimation(C_CrowdAnimation* pLocalAnimation, const Animation::C_Cycle* pCycle, const F32 cycleDuration, const F32 blendTime, const F32 startTime, const bool fractionalTimeOffset, const bool looped)
{
	if (pLocalAnimation)
	{
		pLocalAnimation->PushCycle(pCycle, cycleDuration, blendTime, (looped ? C_CrowdAnimation::E_TIME_MODE_LOOPING : C_CrowdAnimation::E_TIME_MODE_NORMAL),
			startTime, fractionalTimeOffset ? C_CrowdAnimation::E_TIME_FORMAT_FRACTION : C_CrowdAnimation::E_TIME_FORMAT_SECONDS, false);
	}
}

const bool C_CrowdSpectator::IsLocalAnimFinished(C_CrowdAnimation* pLocalAnimation, const F32 blendTime) const
{
	if (!pLocalAnimation)
		return true;

	return pLocalAnimation->Finished(blendTime);
}

const F32 C_CrowdSpectator::GetAnimationTimeRunning(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation, const CrowdDetailBlendType blendType) const
{
	return GetTimeInfoByType(pLocalAnimation, pSharedAnimation, blendType, E_TIME_INFO_RUNNING, false);
}

const F32 C_CrowdSpectator::GetAnimationDuration(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation, const CrowdDetailBlendType blendType, const F32 blendTime) const
{
	return GetTimeInfoByType(pLocalAnimation, pSharedAnimation, blendType, E_TIME_INFO_DURATION, true, blendTime);
}

const F32 C_CrowdSpectator::GetAnimationTimeRemaining(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation, const CrowdDetailBlendType blendType, const F32 blendTime) const
{
	return GetTimeInfoByType(pLocalAnimation, pSharedAnimation, blendType, E_TIME_INFO_REMAINING, true, blendTime);
}

const F32 C_CrowdSpectator::GetTimeInfoByType(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation,
											  const CrowdDetailBlendType blendType, const CrowdTimeInfoType type, const bool activeOnly, const F32 blendTime) const
{
	if (blendType == E_BLEND_LOW_HIGH)
	{
		if (pSharedAnimation)
			return pSharedAnimation->GetAnimation().GetTimeInfoByType(type, activeOnly, blendTime);
	}
	else
	{
		if (pLocalAnimation)
			return pLocalAnimation->GetTimeInfoByType(type, activeOnly, blendTime);
	}

	return 0.f;
}

bool C_CrowdSpectator::HasPose(const C_CrowdAnimation* pAnimation) const
{
	if (pAnimation)
		return pAnimation->HasPose();
	return false;
}

#ifndef PLATFORM_SPU
const Animation::C_Pose* C_CrowdSpectator::GetPose(const C_CrowdAnimation* pLocalAnimation, const C_CrowdSharedAnimation* pSharedAnimation) const
{
	if (pLocalAnimation)
		return pLocalAnimation->GetPose(); // local animation should have shared stuff blended into it if it exists
	else if(pSharedAnimation)
		return pSharedAnimation->GetAnimation().GetPose();

	return NULL;
}

const Animation::C_Pose* C_CrowdSpectator::GetPose(const C_CrowdAnimation* pAnimation) const
{
	if (pAnimation)
		return pAnimation->GetPose();
	return NULL;
}

const Animation::C_Pose* C_CrowdSpectator::GetPoseNonBlocking(const C_CrowdAnimation* pLocalAnimation, const C_CrowdSharedAnimation* pSharedAnimation) const
{
	if (pLocalAnimation)
		return pLocalAnimation->GetPoseNonBlocking(); // local animation should have shared stuff blended into it if it exists
	else if(pSharedAnimation)
		return pSharedAnimation->GetAnimation().GetPoseNonBlocking();

	return NULL;
}

const Animation::C_Skeleton* C_CrowdSpectator::GetSkeleton(const C_CrowdAnimation* pLocalAnimation, const C_CrowdSharedAnimation* pSharedAnimation) const
{
	if (pLocalAnimation)
		return pLocalAnimation->GetSkeleton();
	else if (pSharedAnimation)
		return pSharedAnimation->GetAnimation().GetSkeleton();

	return NULL;
}

#ifndef CFG_GOLD
const Animation::C_Pose* C_CrowdSpectator::GetPose(const C_CrowdResources& resources) const
{
	const C_CrowdAnimation* pLocalAnimation = NULL;
	const C_CrowdSharedAnimation* pSharedAnimation = NULL;

	if (m_LocalAnimation)
	{
		pLocalAnimation = resources.GetUniqueAnimationPool().GetAnimation(m_LocalAnimation);
		if (pLocalAnimation && pLocalAnimation->GetIndex() == 0xff)
			pLocalAnimation = NULL;
	}
	if (m_SharedAnimation)
	{
		pSharedAnimation = resources.GetSharedAnimationPool().GetAnimation(m_SharedAnimation);
		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() == 0xff)
			pSharedAnimation = NULL;
	}

	return GetPose(pLocalAnimation, pSharedAnimation);
}

const C_CrowdSharedAnimation* C_CrowdSpectator::GetSharedAnimation(const C_CrowdResources& resources) const
{
	if (m_SharedAnimation)
	{
		const C_CrowdSharedAnimation* pSharedAnimation = resources.GetSharedAnimationPool().GetAnimation(m_SharedAnimation);
		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
			return pSharedAnimation;
	}
	
	return NULL;
}
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

const Maths::RGBA C_CrowdSpectator::GetColour(const CrowdDetailLevel detailLevel, const CrowdDetailBlendType blendType, const F32 blendFactor) const
{
	Maths::RGBA colour = CROWD_SPECTATOR_NO_ANIMATION;
	colour.A(255);

	switch (blendType)
	{
	case E_BLEND_LOW_HIGH:
	case E_BLEND_HIGH_LOW:
		{
			colour.R((U8)(Maths::Lerp(CROWD_SPECTATOR_SHARED_ANIMATION.R() / 255.f, CROWD_SPECTATOR_UNIQUE_ANIMATION.R() / 255.f, blendFactor) * 255.f));
			colour.G((U8)(Maths::Lerp(CROWD_SPECTATOR_SHARED_ANIMATION.G() / 255.f, CROWD_SPECTATOR_UNIQUE_ANIMATION.G() / 255.f, blendFactor) * 255.f));
			colour.B((U8)(Maths::Lerp(CROWD_SPECTATOR_SHARED_ANIMATION.B() / 255.f, CROWD_SPECTATOR_UNIQUE_ANIMATION.B() / 255.f, blendFactor) * 255.f));
		}
		break;
	case E_NO_BLEND:
	default:
		{
			switch (detailLevel)
			{
			case E_DETAIL_LEVEL_HIGH:
				{
					colour = CROWD_SPECTATOR_UNIQUE_ANIMATION;
				}
				break;
			case E_DETAIL_LEVEL_LOW:
			default:
				{
					colour = CROWD_SPECTATOR_SHARED_ANIMATION;
				}
				break;
			}
		}
		break;
	}

	return colour;
}
// </CrowdAnimatedMember>

}; // namespace AnimatedCrowd
