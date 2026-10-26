#ifndef CROWD_ANIMATION_H
#define CROWD_ANIMATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

#include <animation/cycle/CycleInstance.h>

#include <Btl/Containers/bitvector.h>

namespace Animation
{
	class C_Cycle;
	class C_EvaluationState;
	class C_EventLogger;
	class C_PoseFormat;
	class C_PoseMatrixCache;
	class C_Skeleton;
};

namespace AnimatedCrowd
{
	class C_CrowdResources;
	class C_CrowdAnimation;
};

namespace AnimatedCrowd
{

//! AnimationCreationParams
struct ALIGN16 AnimationCreationParams
{
public:
	const Animation::C_PoseFormat*	m_pPoseFormat;
	const Animation::C_Skeleton*	m_pSkeleton;
	U16								m_BehaviourIndex;
	const Animation::C_StringId*	m_pEventStringIds;
	U16								m_NumEvents;
};

//! C_CrowdAnimation

class ALIGN16 C_CrowdAnimation
{
public:
	enum CycleTimeMode
	{
		E_TIME_MODE_NORMAL			= 0,
		E_TIME_MODE_LOOPING,
		E_TIME_MODE_REVERSE,
		E_TIME_MODE_REVERSE_LOOPING,
		E_TIME_MODE_CUSTOM
	};

	enum TimeFormat
	{
		E_TIME_FORMAT_SECONDS,
		E_TIME_FORMAT_MSECONDS,
		E_TIME_FORMAT_FRACTION		// As a fraction of the cycle's duration
	};

#ifndef PLATFORM_SPU
	C_CrowdAnimation();
	~C_CrowdAnimation();

	bool								Create(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
											const Animation::C_StringId* pEventStringIds, const U32 numEvents, const U8 index);
private:
	void								Destroy();

public:
	void								ClearIndex();
#endif // !PLATFORM_SPU

	void								PushCycle(const Animation::C_Cycle* pCycle, const F32 duration, const F32 blendTime, const CycleTimeMode timeMode,
											const F32 startTime = 0.f, const TimeFormat startTimeFormat = E_TIME_FORMAT_SECONDS, const bool blendSingleCycle = false);

	void								BlendAnimation(const S32 sharedAnimation, const F32 blendFactor);
	void								Update(const F32 deltaTime, const C_CrowdResources& resources, const U32 hipJointIndex, const Animation::C_JointTransform& hipJointTransform);

	const bool							Empty() const;
	const bool							Finished(const F32 blendTime) const;

	const F32							GetTimeInfoByType(const CrowdTimeInfoType type, const bool activeOnly = false, const F32 blendTime = 0.f) const;

	bool								HasPose() const;
#ifndef PLATFORM_SPU
	const Animation::C_Pose*			GetPose() const;
	const Animation::C_Pose*			GetPoseNonBlocking() const;
	
	const Animation::C_PoseMatrixCache*	GetPoseMatrixCache() const;
	const Animation::C_PoseMatrixCache* GetPoseMatrixCacheNonBlocking() const;

	const Animation::C_Skeleton*		GetSkeleton() const;
#endif // !PLATFORM_SPU

	const S32							GetNumCycles() const;

	const bool							IsValid() const;

	const Animation::C_CycleInstance*	GetCurrentCycleInstance() const;

#ifndef PLATFORM_SPU
	const Maths::HMat43					GetJointTransform(const U16 jointIndex) const;
#endif // !PLATFORM_SPU

	//void								SetIndex(const U8 index);
	const U8							GetIndex() const;

	const F32							GetBlendTimeRemaining() const;

	//void								ClearEffectBits();
	const bool							CanSpawnAnyEffects() const;
	const bool							CanSpawnEffect(const U32 eventIndex) const;

#ifndef PLATFORM_SPU
	bool								Flush();
#endif // !PLATFORM_SPU

	void								SetDirty();
	const bool							IsDirty() const;

	void								Zombify();
	const bool							IsZombie() const;

protected:
	const Animation::C_EvaluationState*	GetEvaluationState() const;

private:
	void								OnUpdate(const F32 deltaTime, const C_CrowdResources& resources, const U32 hipJointIndex, const Animation::C_JointTransform& hipJointTransform,
											C_CrowdAnimation* pSharedAnimation = NULL, const F32 blendFactor = 1.f) __restrict;

	void								FadeOut(const F32 fadeTime);

	static const U32 kActiveCycleState_Invalid			= 0;
	static const U32 kActiveCycleState_Initialised		= 1;
	static const U32 kActiveCycleState_Created			= 2;
	static const U32 kActiveCycleState_NeedsUpdate		= 3;
	static const U32 kActiveCycleState_Valid			= 4;
	static const U32 kActiveCycleState_RequiresClear	= 5;

	struct ALIGN16 ActiveCycle
	{
		Animation::C_CycleInstance	m_CycleInstance;
		const Animation::C_Cycle*	m_pCycle;

		U32		m_TimeMode			: 3;
		U32		m_LoopedLastUpdate	: 1;
		U32		m_State				: 28;
		
		F32		m_RunningTime;
		F32		m_CycleTime;
		F32		m_Duration;
		F32		m_StartBlendTime;
		F32		m_EndBlendTime;
		F32		m_Weight;

		F32		m_StartWeight;
		F32		m_EndWeight;

		void	Reset();
	};

	ActiveCycle						m_ActiveCycles[CROWD_MAX_BLENDED_CYCLES];
	
	Animation::C_EvaluationState*	m_pEvaluationState;
	Btl::BitArray32					m_AnimationEventBits;

	U32								m_ActiveCycleStartIndex;
	S16								m_NumCycles; // TODO: [rthomas]	Get rid of m_State and make this S32 again for perfect 16 byte alignment? :)

	S32								m_BlendSharedAnimation;
	F32								m_BlendAnimationFactor;

	U32								m_Index		: 30;
	U32								m_Dirty		: 1;
	U32								m_Zombie	: 1;
	
	Animation::C_EventLogger*		m_pEventLogger;

	static const U32 kState_Initialised	= 0;
	static const U32 kState_Created		= 1;
	static const U32 kState_Cleared		= 2;
	static const U32 kState_Destroyed	= 3;

	U16								m_State;

	friend class C_CrowdSharedAnimation;
	friend class C_CrowdSharedAnimationPool;
	friend class C_CrowdUniqueAnimationPool;
};

}; // namespace AnimatedCrowd

#endif // CROWD_ANIMATION_H
