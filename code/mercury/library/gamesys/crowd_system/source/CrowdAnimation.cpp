#include "CrowdSystemPch.h"

#include <crowd_system/CrowdAnimation.h>
#include <crowd_system/CrowdMaths.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdResources.h>
#endif // !PLATFORM_SPU

#ifndef PLATFORM_SPU
#include <animation/cycle/Cycle.h>
#include <animation/deferred/CommandList.h>
#include <animation/pose/EventLogger.h>
#endif // !PLATFORM_SPU

#include <animation/task/EvaluationState.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseMatrixCache.h>

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
extern Debug::Bool g_DebugDisableAnimation;
extern Debug::Bool g_DebugRenderSkeleton;
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

inline U32 GetNextActiveCycleIndex(const U32 activeCycleIndex)
{
	U32 index = activeCycleIndex + 1;
	return (index < CROWD_MAX_BLENDED_CYCLES) ? index : 0;
}

inline U32 GetPreviousActiveCycleIndex(const U32 activeCycleIndex)
{
	return (activeCycleIndex > 0) ? (activeCycleIndex - 1) : (CROWD_MAX_BLENDED_CYCLES - 1);
}

#ifndef PLATFORM_SPU
C_CrowdAnimation::C_CrowdAnimation()
	: m_Dirty(0),
	m_pEvaluationState(Animation::C_EvaluationState::New(16)),
	m_NumCycles(0),
	m_ActiveCycleStartIndex(0),
	m_Index(0xff),
	m_BlendSharedAnimation(-1),
	m_BlendAnimationFactor(1.f),
	m_Zombie(false),
	m_pEventLogger(NULL),
	m_State(kState_Initialised)
{
	for (U32 i = 0, count = CROWD_MAX_BLENDED_CYCLES; i < count; ++i)
	{
		m_ActiveCycles[i].Reset();
		m_ActiveCycles[i].m_CycleInstance.Clear();
	}
}

C_CrowdAnimation::~C_CrowdAnimation()
{
	ClearIndex();
	Destroy();

	SAFE_DELETE_PTR(m_pEvaluationState);
	SAFE_DELETE_PTR(m_pEventLogger);
}

bool C_CrowdAnimation::Create(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton, const Animation::C_StringId* pEventStringIds, const U32 numEvents, const U8 index)
{
	if (!pSkeleton)
		return false;

	BIZ_ASSERT(m_State == kState_Initialised || m_State == kState_Destroyed);

	m_State = kState_Created;

	m_NumCycles = 0;
	
	for (U32 i = 0, count = CROWD_MAX_BLENDED_CYCLES; i < count; ++i)
	{
		m_ActiveCycles[i].Reset();
		m_ActiveCycles[i].m_CycleInstance.Clear();
	}

	m_pEvaluationState->GetCommandList().Clear();

	m_pEvaluationState->GetSkeletonInstance().Create(*pSkeleton);

	Animation::C_Pose& pose = m_pEvaluationState->GetPose();

	pose.Create(poseFormat);
	pose.Clear();

	m_pEvaluationState->GetPoseMatrixCache().Create(pose);
	m_pEvaluationState->GetPoseMatrixCache().Update(pose, pSkeleton);

	if (pEventStringIds)
	{
		if (!m_pEventLogger)
			m_pEventLogger = new Animation::C_EventLogger();
		m_pEventLogger->Create(m_pEvaluationState->GetPose(), numEvents, pEventStringIds);
	}

	BIZ_ASSERT(numEvents <= static_cast<U32>(m_AnimationEventBits.Size())); // Currently 32, could increase if more animation events are needed
	m_AnimationEventBits.ClearBits();

	m_Index = index;

	m_Dirty = true;
	m_Zombie = false;

	m_BlendSharedAnimation = -1;

	return true;
}

void C_CrowdAnimation::Destroy()
{
	BIZ_ASSERT(m_Index == 0xff);

	m_NumCycles = 0;

	m_State = kState_Destroyed;

	m_Zombie = false;
}

void C_CrowdAnimation::ClearIndex()
{
	m_Index = 0xff;

	m_NumCycles = 0;

	m_State = kState_Cleared;
}
#endif // !PLATFORM_SPU

void C_CrowdAnimation::PushCycle(const Animation::C_Cycle* pCycle, const F32 duration, const F32 blendTime, const CycleTimeMode timeMode,
								 const F32 startTime, const TimeFormat startTimeFormat, const bool blendSingleCycle)
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (!pCycle)
		return;

	BIZ_ASSERT(m_NumCycles < CROWD_MAX_BLENDED_CYCLES);

	F32 b = blendTime;

	if (m_NumCycles > 0)
	{
		FadeOut(b);

		m_ActiveCycleStartIndex = GetNextActiveCycleIndex(m_ActiveCycleStartIndex);
	}
	else if (!blendSingleCycle)
		b = 0.f;

	bool blending = (b > Maths::Constants<F32>::Epsilon());

	m_NumCycles = Maths::Min<S32>(m_NumCycles + 1, CROWD_MAX_BLENDED_CYCLES);

	ActiveCycle& cycle(m_ActiveCycles[m_ActiveCycleStartIndex]);

#ifndef PLATFORM_SPU
	BIZ_ASSERT(cycle.m_CycleInstance.GetCycle() == NULL); // [rthomas]	It should be NULL but don't want to spu fetch cycle instance.
#endif // !PLATFORM_SPU

	cycle.m_pCycle			= pCycle;
	cycle.m_TimeMode		= timeMode;
	cycle.m_CycleTime		= startTime;
	cycle.m_Duration		= duration;
	cycle.m_Weight			= blending ? 0.f : 1.f;
	cycle.m_EndBlendTime	= b;
	cycle.m_State			= kActiveCycleState_Initialised;

	if (timeMode != E_TIME_MODE_CUSTOM)
	{
		cycle.m_RunningTime	= cycle.m_CycleTime;
	}

	if (timeMode == E_TIME_MODE_REVERSE)
	{
		cycle.m_CycleTime = cycle.m_Duration - cycle.m_CycleTime;
	}
}

void C_CrowdAnimation::BlendAnimation(const S32 sharedAnimation, const F32 blendFactor)
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	m_BlendSharedAnimation = sharedAnimation;
	m_BlendAnimationFactor = blendFactor;
}

#ifndef PLATFORM_SPU
void C_CrowdAnimation::OnUpdate(const F32 deltaTime, const C_CrowdResources& resources, const U32 hipJointIndex, const Animation::C_JointTransform& hipJointTransform,
								C_CrowdAnimation* pBlendAnimation, const F32 blendFactor) __restrict
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	for (S32 i = 0, count = CROWD_MAX_BLENDED_CYCLES; i < count; ++i)
	{
		if ((m_ActiveCycles[i].m_State == kActiveCycleState_Initialised) && m_ActiveCycles[i].m_pCycle)
		{
			m_ActiveCycles[i].m_CycleInstance.Create(m_ActiveCycles[i].m_pCycle, &m_pEvaluationState->GetPose());
			m_ActiveCycles[i].m_State = kActiveCycleState_Created;
		}
		else if (m_ActiveCycles[i].m_State == kActiveCycleState_RequiresClear)
		{
			--m_NumCycles;
			m_ActiveCycles[i].Reset();
			m_ActiveCycles[i].m_CycleInstance.Clear();
		}
	}

	if (!m_NumCycles)
		return;

	S32 activeCycleStartIndex = m_ActiveCycleStartIndex;

	for (S32 i = 1, count = m_NumCycles; i < count; ++i)
		activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);

	for (S32 i = 0, count = m_NumCycles; i < count; ++i)
	{
		ActiveCycle& cycle(m_ActiveCycles[activeCycleStartIndex]);

		cycle.m_RunningTime += deltaTime;

		F32 preCycleTime = cycle.m_CycleTime;
		F32 newCycleTime = preCycleTime;

		U32 cycleTimeMode = cycle.m_TimeMode;
		if (cycleTimeMode == E_TIME_MODE_NORMAL || cycleTimeMode == E_TIME_MODE_LOOPING)
		{
			newCycleTime += deltaTime;
		}
		else if (cycleTimeMode == E_TIME_MODE_REVERSE || cycleTimeMode == E_TIME_MODE_REVERSE_LOOPING)
		{
			newCycleTime -= deltaTime;
		}

		cycle.m_LoopedLastUpdate = false;

		if (newCycleTime >= cycle.m_Duration)
		{
			if (cycleTimeMode == E_TIME_MODE_NORMAL || cycleTimeMode == E_TIME_MODE_CUSTOM)
			{
				newCycleTime = cycle.m_Duration;
			}
			else if (cycleTimeMode == E_TIME_MODE_LOOPING)
			{
				cycle.m_LoopedLastUpdate = true;
				newCycleTime = fmodf(newCycleTime, cycle.m_Duration);
			}
		}
		else if (newCycleTime <= 0.f)
		{
			if (cycleTimeMode == E_TIME_MODE_REVERSE || cycleTimeMode == E_TIME_MODE_CUSTOM)
			{
				newCycleTime = 0.f;
			}
			else if (cycleTimeMode == E_TIME_MODE_REVERSE_LOOPING)
			{
				cycle.m_LoopedLastUpdate = true;
				newCycleTime = fmodf(newCycleTime, cycle.m_Duration);
			}
		}

		cycle.m_CycleTime = newCycleTime;

		if (cycle.m_StartBlendTime >= 0.f)
		{
			if (cycle.m_RunningTime < cycle.m_StartBlendTime)
			{
				cycle.m_Weight = cycle.m_StartWeight;
			}
			else if (cycle.m_RunningTime >= cycle.m_EndBlendTime)
			{
				cycle.m_Weight = cycle.m_EndWeight;
				
				if (cycle.m_Weight < Maths::Constants<F32>::Epsilon())
					cycle.m_State = kActiveCycleState_RequiresClear;
			}
			else
			{
				F32 f = (cycle.m_RunningTime - cycle.m_StartBlendTime) / (cycle.m_EndBlendTime - cycle.m_StartBlendTime);
				f = Maths::Clamp<F32>(f, 0.f, 1.f);

				f = EaseInEaseOut(f);

				cycle.m_Weight = Maths::Lerp(cycle.m_StartWeight, cycle.m_EndWeight, f);
			}
		}

		activeCycleStartIndex = GetPreviousActiveCycleIndex(activeCycleStartIndex);
	}

	Animation::C_CommandList& commandList = m_pEvaluationState->GetCommandList();

	commandList.Clear();

	if (pBlendAnimation != NULL)
	{
		BIZ_ASSERT(pBlendAnimation->m_State == kState_Created);

		S32 activeCycleStartIndex = pBlendAnimation->m_ActiveCycleStartIndex;

		for (S32 i = 0, count = pBlendAnimation->m_NumCycles; i < count; ++i)
		{
			if (pBlendAnimation->m_ActiveCycles[activeCycleStartIndex].m_CycleInstance.IsValid())
			{
				commandList.BlendCycle(
					Animation::E_BlendMode_Accumulate,
					&pBlendAnimation->m_ActiveCycles[activeCycleStartIndex].m_CycleInstance,
					pBlendAnimation->m_ActiveCycles[activeCycleStartIndex].m_CycleTime,
					pBlendAnimation->m_ActiveCycles[activeCycleStartIndex].m_Weight * (1.f - blendFactor),
					Animation::E_LoopMode_Continuous,
					resources.GetAnimationEventMask(false)
				);
			}

			activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);
		}

		activeCycleStartIndex = m_ActiveCycleStartIndex;

		for (S32 i = 0, count = m_NumCycles; i < count; ++i)
		{
			if (m_ActiveCycles[activeCycleStartIndex].m_CycleInstance.IsValid())
			{
				commandList.BlendCycle(
					Animation::E_BlendMode_Accumulate,
					&m_ActiveCycles[activeCycleStartIndex].m_CycleInstance,
					m_ActiveCycles[activeCycleStartIndex].m_CycleTime,
					m_ActiveCycles[activeCycleStartIndex].m_Weight * blendFactor,
					Animation::E_LoopMode_Continuous,
					resources.GetAnimationEventMask(i == 0)
				);
			}

			activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);
		}
	}
	else
	{
		S32 activeCycleStartIndex = m_ActiveCycleStartIndex;

		for (S32 i = 0, count = m_NumCycles; i < count; ++i)
		{
			if (m_ActiveCycles[activeCycleStartIndex].m_CycleInstance.IsValid() && m_ActiveCycles[activeCycleStartIndex].m_CycleInstance.IsBound())
			{
				commandList.BlendCycle(
					Animation::E_BlendMode_Accumulate,
					&m_ActiveCycles[activeCycleStartIndex].m_CycleInstance,
					m_ActiveCycles[activeCycleStartIndex].m_CycleTime,
					m_ActiveCycles[activeCycleStartIndex].m_Weight,
					Animation::E_LoopMode_Continuous,
					resources.GetAnimationEventMask(i == 0)
				);
			}

			activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);
		}
	}
	
	commandList.SetRootToIdentity();

	{
		commandList.TransformJointTranslationInModelSpace(hipJointIndex, hipJointTransform);
	}
}

void C_CrowdAnimation::Update(const F32 deltaTime, const C_CrowdResources& resources, const U32 hipJointIndex, const Animation::C_JointTransform& hipJointTransform)
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (!m_pEvaluationState)
		return;

	if (m_pEventLogger)
	{
		m_pEventLogger->Update(m_pEvaluationState->GetPose()); // [rthomas]	Confirm this doesn't cause flush of evaluation tasks
		
		m_AnimationEventBits.ClearBits();

		for (U32 i = 0, count = m_pEventLogger->GetNumEvents(); i < count; ++i)
		{
			if (m_pEventLogger->HasChangedToTrue(i))
				m_AnimationEventBits.SetBit(i);
		}
	}

	{
		BIZ_ASSERT(m_pEvaluationState->GetSkeletonInstance().GetSkeleton());

		if (m_NumCycles < 0)
		{
			m_NumCycles = 0;

			for (S32 i = 0, count = CROWD_MAX_BLENDED_CYCLES; i < count; ++i)
			{
				m_ActiveCycles[i].Reset();
			}
		}

		S32 activeCycleStartIndex = m_ActiveCycleStartIndex;

		for (S32 i = 0, count = m_NumCycles; i < count; ++i)
		{
			if (m_ActiveCycles[activeCycleStartIndex].m_State == kActiveCycleState_Created)
				m_ActiveCycles[activeCycleStartIndex].m_State = kActiveCycleState_NeedsUpdate;
			else if (m_ActiveCycles[activeCycleStartIndex].m_State == kActiveCycleState_NeedsUpdate)
				m_ActiveCycles[activeCycleStartIndex].m_State = kActiveCycleState_Valid;

			activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);
		}
	}

#ifndef CFG_GOLD
	if (!g_DebugDisableAnimation)
#endif
	{
		C_CrowdAnimation* pBlendAnimation = NULL;
		if (m_BlendSharedAnimation >= 0)
		{
			pBlendAnimation = const_cast<C_CrowdAnimation*>(resources.GetSharedAnimationByIndex(m_BlendSharedAnimation)); // TODO: [rthomas] Remove const cast?

			BIZ_ASSERT(pBlendAnimation->GetIndex() != 0xff);

			if (pBlendAnimation->GetIndex() == 0xff)
				pBlendAnimation = NULL;
		}

		OnUpdate(deltaTime, resources, hipJointIndex, hipJointTransform, pBlendAnimation, m_BlendAnimationFactor);
	}

	if (!m_pEvaluationState->GetCommandList().IsEmpty())
	{
		m_pEvaluationState->ScheduleEvaluationTask();
	}
	else
	{
		volatile S32 breakhere = -1;
	}

	m_BlendSharedAnimation = -1;
	m_BlendAnimationFactor = 1.f;

	m_Dirty = false;
}
#endif // !PLATFORM_SPU

const bool C_CrowdAnimation::Empty() const
{
	return (m_NumCycles <= 0);
}

const bool C_CrowdAnimation::Finished(const F32 blendTime) const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	bool finished = true;

	if (m_NumCycles > 0)
	{
		const ActiveCycle& activeCycle(m_ActiveCycles[m_ActiveCycleStartIndex]);

		if (activeCycle.m_TimeMode != E_TIME_MODE_CUSTOM)
			finished = ((activeCycle.m_RunningTime + blendTime) >= activeCycle.m_Duration);
		else
			finished = false;
	}

	return finished;
}

const F32 C_CrowdAnimation::GetTimeInfoByType(const CrowdTimeInfoType type, const bool activeOnly, const F32 blendTime) const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	F32 t = 0.f;

	U32 activeCycleStartIndex = m_ActiveCycleStartIndex;

	for (S32 i = 0, count = activeOnly ? Maths::Min<S32>(m_NumCycles, 1) : m_NumCycles; i < count; ++i)
	{
		switch (type)
		{
		case E_TIME_INFO_RUNNING:
			t = Maths::Max<F32>(t, m_ActiveCycles[activeCycleStartIndex].m_RunningTime);
			break;
		case E_TIME_INFO_CYCLE:
			t = Maths::Max<F32>(t, m_ActiveCycles[activeCycleStartIndex].m_CycleTime);
			break;
		case E_TIME_INFO_DURATION:
			t = Maths::Max<F32>(t, m_ActiveCycles[activeCycleStartIndex].m_Duration - blendTime);
			break;
		case E_TIME_INFO_BLEND_REMAINING:
			t = Maths::Max<F32>(t, m_ActiveCycles[activeCycleStartIndex].m_EndBlendTime - m_ActiveCycles[activeCycleStartIndex].m_RunningTime);
			break;
		case E_TIME_INFO_REMAINING:
			t = Maths::Max<F32>(t, m_ActiveCycles[activeCycleStartIndex].m_Duration - blendTime - m_ActiveCycles[activeCycleStartIndex].m_RunningTime);
			break;
		default:
			break;
		}

		activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);
	}

	return t;
}

bool C_CrowdAnimation::HasPose() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	return m_pEvaluationState != NULL;
}

#ifndef PLATFORM_SPU
const Animation::C_Pose* C_CrowdAnimation::GetPose() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (m_pEvaluationState)
		return &m_pEvaluationState->GetPose();

	return NULL;
}

const Animation::C_Pose* C_CrowdAnimation::GetPoseNonBlocking() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (m_pEvaluationState)
		return &m_pEvaluationState->GetPoseNonBlocking();

	return NULL;
}

const Animation::C_PoseMatrixCache* C_CrowdAnimation::GetPoseMatrixCache() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (m_pEvaluationState)
		return &m_pEvaluationState->GetPoseMatrixCache();

	return NULL;
}

const Animation::C_PoseMatrixCache* C_CrowdAnimation::GetPoseMatrixCacheNonBlocking() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (m_pEvaluationState)
		return &m_pEvaluationState->GetPoseMatrixCacheNonBlocking();
	
	return NULL;
}

const Animation::C_Skeleton* C_CrowdAnimation::GetSkeleton() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (m_pEvaluationState)
		return m_pEvaluationState->GetSkeleton();

	return NULL;
}
#endif // !PLATFORM_SPU

const S32 C_CrowdAnimation::GetNumCycles() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	return m_NumCycles;
}

const bool C_CrowdAnimation::IsValid() const
{
	BIZ_ASSERT(!m_Zombie);

	if (m_NumCycles <= 0)
		return false;

	U32 activeCycleStartIndex = m_ActiveCycleStartIndex;

	for (S32 i = 0, count = m_NumCycles; i < count; ++i)
	{
		if (m_ActiveCycles[activeCycleStartIndex].m_State == kActiveCycleState_Valid)
			return true;

		activeCycleStartIndex = GetNextActiveCycleIndex(activeCycleStartIndex);
	}
	
	return false;
}

#ifndef PLATFORM_SPU
bool C_CrowdAnimation::Flush()
{
	BIZ_ASSERT(!m_Zombie);

	m_Dirty	= 1;
	m_NumCycles = -1;

	m_ActiveCycleStartIndex = 0;

	for (S32 i = 0, count = CROWD_MAX_BLENDED_CYCLES; i < count; ++i)
		m_ActiveCycles[i].m_State = kActiveCycleState_Invalid;

	m_pEvaluationState->GetCommandList().Clear();
	m_pEvaluationState->GetSkeletonInstance().Destroy();
	m_pEvaluationState->GetPoseMatrixCache().Destroy();

	return true;
}
#endif // !PLATFORM_SPU

void C_CrowdAnimation::Zombify()
{
	BIZ_ASSERT(m_State == kState_Created);

	m_Zombie = true;
}

const bool C_CrowdAnimation::IsZombie() const
{
	return m_Zombie;
}

void C_CrowdAnimation::SetDirty()
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);
	
	BIZ_ASSERT(m_State == kState_Created);

	m_Dirty = true;
}

const bool C_CrowdAnimation::IsDirty() const // ;)
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	return (m_Dirty || (GetNumCycles() < 0));
}

const Animation::C_EvaluationState* C_CrowdAnimation::GetEvaluationState() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	return m_pEvaluationState;
}

const Animation::C_CycleInstance* C_CrowdAnimation::GetCurrentCycleInstance() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	return &m_ActiveCycles[m_ActiveCycleStartIndex].m_CycleInstance;
}

#ifndef PLATFORM_SPU
const Maths::HMat43 C_CrowdAnimation::GetJointTransform(const U16 jointIndex) const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	Animation::C_JointTransform jointTransform = GetPose()->GetModelJointTransform(static_cast<U32>(jointIndex), GetSkeleton());
	return jointTransform.AsMat43();
}
#endif // !PLATFORM_SPU

const U8 C_CrowdAnimation::GetIndex() const
{
	return m_Index;
}

const F32 C_CrowdAnimation::GetBlendTimeRemaining() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	F32 blendTimeRemaining = 0.f;
	
	for (S32 i = 0, count = CROWD_MAX_BLENDED_CYCLES; i < count; ++i)
		blendTimeRemaining = Maths::Max<F32>(blendTimeRemaining, m_ActiveCycles[i].m_EndBlendTime - m_ActiveCycles[i].m_RunningTime);

	return blendTimeRemaining;
}

const bool C_CrowdAnimation::CanSpawnAnyEffects() const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	return m_AnimationEventBits.AnyBitsSet();
}

const bool C_CrowdAnimation::CanSpawnEffect(const U32 eventIndex) const
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	BIZ_ASSERT(eventIndex < static_cast<U32>(m_AnimationEventBits.Size()));
	return m_AnimationEventBits.GetBitValue(static_cast<S32>(eventIndex));
}

void C_CrowdAnimation::FadeOut(const F32 fadeTime)
{
	BIZ_ASSERT(!m_Zombie && m_Index != 0xff);

	BIZ_ASSERT(m_State == kState_Created);

	if (m_NumCycles <= 0)
		return;

	m_ActiveCycles[m_ActiveCycleStartIndex].m_StartBlendTime	= m_ActiveCycles[m_ActiveCycleStartIndex].m_RunningTime;
	m_ActiveCycles[m_ActiveCycleStartIndex].m_EndBlendTime		= m_ActiveCycles[m_ActiveCycleStartIndex].m_RunningTime + fadeTime;

	m_ActiveCycles[m_ActiveCycleStartIndex].m_StartWeight		= m_ActiveCycles[m_ActiveCycleStartIndex].m_Weight;
	m_ActiveCycles[m_ActiveCycleStartIndex].m_EndWeight			= 0.f;
}

void C_CrowdAnimation::ActiveCycle::Reset()
{
	m_pCycle			= NULL;

	m_TimeMode			= E_TIME_MODE_NORMAL;
	m_LoopedLastUpdate	= 0;
	m_State				= kActiveCycleState_Invalid;
	
	m_RunningTime		= 0.f;
	m_CycleTime			= 0.f;
	m_Duration			= 0.f;
	m_StartBlendTime	= 0.f;
	m_EndBlendTime		= 0.f;
	m_Weight			= 0.f;

	m_StartWeight		= 0.f;
	m_EndWeight			= 1.f;
}

}; // namespace AnimatedCrowd
