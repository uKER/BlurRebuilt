#ifndef CROWD_SPECTATOR_H
#define CROWD_SPECTATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU
#include <Core/Containers/String.h>
#endif // !PLATFORM_SPU

#include <crowd_system/CrowdActionSelect.h>
#include <crowd_system/CrowdEffectItem.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdAnimationEventEffect.h>
#endif // !PLATFORM_SPU

#include <btl/containers/bitvector.h>

namespace Animation
{
	class C_PoseMatrixCache;
	class C_PoseFormat;
	class C_Skeleton;
	class C_StringId;

	class C_Cycle;
	class C_Pose;
};

namespace AnimatedCrowd
{
	class C_Event;
	class C_SpectatorUpdateInput;
	class C_SpectatorUpdateOutput;
	class C_Random;

	class C_CrowdAnimation;
	class C_CrowdResources;
	class C_CrowdSharedAnimation;
	class C_CrowdSharedAnimationPool;
	class C_CrowdUniqueAnimationPool;
};

namespace AnimatedCrowd
{

struct ALIGN16 SpectatorSpawnParams
{
	SpectatorSpawnParams()
		: m_pResources(NULL),
		m_UniqueAnimation(-1),
		m_SharedAnimation(-1),
		m_BehaviourIndex(-1),
		m_ForceShared(0)
	{ }

	Maths::HVec3		m_Position;
	Maths::HVec3		m_Direction;

	C_CrowdResources*	m_pResources;

	S16					m_UniqueAnimation;
	S16					m_SharedAnimation;

	U16					m_BehaviourIndex;
	U32					m_RandomSeed;
	
	U16					m_InstanceIndex;
	S32					m_InstanceUniqueId;

	U32					m_FlagTextureIndex	: 15;
	U32					m_ForceShared		: 1;
};

//! C_CrowdSpectator

class ALIGN16 C_CrowdSpectator
{
public:
	friend class C_CrowdSpectators;

#ifndef PLATFORM_SPU
	C_CrowdSpectator();
	~C_CrowdSpectator();

	void					Create(const SpectatorSpawnParams& spawnParams);
	void					Destroy(C_CrowdResources& resources);
#endif // !PLATFORM_SPU

	void					Update(const C_SpectatorUpdateInput& input, C_SpectatorUpdateOutput& output);

#ifndef PLATFORM_SPU
	void					HandleEvent(const C_CrowdResources* pResources, const C_Event& crowdEvent, const CrowdGlobalMood globalMood);
#endif // !PLATFORM_SPU

	const U32				GetInstanceIndex() const;
	const S16				GetInstanceUniqueId() const;

#ifndef PLATFORM_SPU
	const Animation::C_PoseMatrixCache*
							GetPoseMatrixCacheNonBlocking(const C_CrowdResources& resources) const;
#endif // !PLATFORM_SPU

	void					ForceDetailLevel(const CrowdDetailLevel detailLevel, C_CrowdUniqueAnimationPool* pUniqueAnimationPool, C_CrowdSharedAnimationPool* pSharedAnimationPool);

	void					SetDetailLevel(const CrowdDetailLevel detailLevel, AnimatedCrowd::C_Random& random);
	const CrowdDetailLevel	GetDetailLevel() const;

	const CrowdDetailLevel	GetDestDetailLevel() const;

	const bool				ForceShared() const { return m_ForceShared; }

#ifndef PLATFORM_SPU
	const Maths::RGBA		GetColour() const;

#ifndef CFG_GOLD
	const char*				GetMainCycleName(const C_CrowdResources& resources) const;
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

private:
	void					SetLocalAnimation(const S32 localAnimation);
	void					ClearLocalAnimation(C_CrowdUniqueAnimationPool* pUniqueAnimationPool);

	void					SetSharedAnimation(const S32 sharedAnimation, C_CrowdSharedAnimationPool* pSharedAnimationPool);
	void					ClearSharedAnimation(C_CrowdSharedAnimationPool* pSharedAnimationPool);

	const Animation::C_Cycle*	GetCurrentCycle(const C_CrowdResources& resources) const;

public:
// <CrowdMember>
	const U16				GetBehaviourIndex() const;

	void					SetPosition(const Maths::HVec3& position);
	const Maths::HVec3&		GetPosition() const;

	void					SetDirection(const Maths::HVec3& direction);
	const Maths::HVec3&		GetDirection() const;

	const Animation::C_Cycle*	ChooseCycle(const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList, const CrowdAction action,
									const Animation::C_Cycle* pCurrentCycle, AnimatedCrowd::C_Random& random, F32& cycleDuration);

	//const Maths::HMat43		GetTransform(const Maths::HVec3& zDir) const;
	const Maths::Mat43		GetTransform(const Maths::Vector3& zDir) const;
// </CrowdMember>

// <CrowdAnimatedMember>
public:
	const U8				GetAnimationIndex(const C_CrowdUniqueAnimationPool* pUniqueAnimationPool, const C_CrowdSharedAnimationPool* pSharedAnimationPool) const;

	void					PlayLocalAnimation(C_CrowdAnimation* pLocalAnimation, const Animation::C_Cycle* pCycle, const F32 duration, const F32 blendTime, const F32 startTime = 0.f,
								const bool fractionalTimeOffset = false, const bool looped = false);

	const bool				IsLocalAnimFinished(C_CrowdAnimation* pLocalAnimation, const F32 blendTime) const;

	const F32				GetAnimationTimeRunning(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation, const CrowdDetailBlendType blendType) const;
	const F32				GetAnimationDuration(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation, const CrowdDetailBlendType blendType, const F32 blendTime) const;
	const F32				GetAnimationTimeRemaining(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation, const CrowdDetailBlendType blendType, const F32 blendTime) const;

	const bool				HasLocalAnimation() const { return m_LocalAnimation >= 0; }
	const bool				HasSharedAnimation() const { return m_SharedAnimation >= 0; }

	bool							HasPose(const C_CrowdAnimation* pAnimation) const;
#ifndef PLATFORM_SPU
	const Animation::C_Pose*		GetPose(const C_CrowdAnimation* pLocalAnimation, const C_CrowdSharedAnimation* pSharedAnimation) const;
	const Animation::C_Pose*		GetPose(const C_CrowdAnimation* pAnimation) const;
	const Animation::C_Pose*		GetPoseNonBlocking(const C_CrowdAnimation* pLocalAnimation, const C_CrowdSharedAnimation* pSharedAnimation) const;
	const Animation::C_Skeleton*	GetSkeleton(const C_CrowdAnimation* pLocalAnimation, const C_CrowdSharedAnimation* pSharedAnimation) const;
#ifndef CFG_GOLD
	const Animation::C_Pose*		GetPose(const C_CrowdResources& resources) const;
	const C_CrowdSharedAnimation*	GetSharedAnimation(const C_CrowdResources& resources) const;
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU
	
	const Maths::RGBA		GetColour(const CrowdDetailLevel detailLevel, const CrowdDetailBlendType blendType, const F32 blendFactor) const;

private:
	const F32				GetTimeInfoByType(C_CrowdAnimation* pLocalAnimation, C_CrowdSharedAnimation* pSharedAnimation,
								const CrowdDetailBlendType blendType, const CrowdTimeInfoType type, const bool activeOnly = false, const F32 blendTime = 0.f) const;
// </CrowdAnimatedMember>

private:
	void					PlayNextAnimation(C_CrowdAnimation* pLocalAnimation, const C_CrowdActionSelect& actionSelect,
								const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupData,
								const CrowdGlobalMood globalMood, AnimatedCrowd::C_Random& random, const F32 blendTime);

	void					SelectNextAction(const C_CrowdAnimation* pLocalAnimation, const C_CrowdActionSelect& actionSelect,
								const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupData,
								AnimatedCrowd::C_Random& random, const CrowdEventType eventType, const CrowdActionPriority actionPriority, const U32 actionFlags,
								const F32 minDelayTime, const F32 maxDelayTime, const CrowdGlobalMood globalMood, const F32 dist = 0.f);
	
	void					Update(const C_SpectatorUpdateInput& input, C_SpectatorUpdateOutput& output, const F32 animationBlendFactor);

	void					SpawnEffects(const C_CrowdAnimation* pAnimation, const bool sharedAnim, EffectSpawnData& effectSpawnData, const bool randomStartTime,
								const Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >& animationEventEffectJointIndexes, const Maths::HMat43& transform, AnimatedCrowd::C_Random& random) const;

private:
	// <CrowdMember>
	Maths::HVec3				m_Position;
	Maths::HVec3				m_Direction;

	U16							m_BehaviourIndex;
	// </CrowdMember>

	// <CrowdAnimatedMember>
	S16							m_LocalAnimation;
	S16							m_SharedAnimation;
	// </CrowdAnimatedMember>

	U16		m_InstanceIndex;
	S32		m_InstanceUniqueId;

	F32		m_DetailBlendStart;
	F32		m_DetailBlendTime;
	F32		m_DetailBlendCounter;

	// Behaviour stuff
	F32		m_NextActionCountdown;

	// The rest
	U32		m_DetailLevel			: 2;
	U32		m_DestDetailLevel		: 2;

	U32		m_CurrentAction			: 5;
	U32		m_CurrentActionPriority	: 3;

	U32		m_NextAction			: 5;
	U32		m_NextActionPriority	: 3;
	U32		m_NextActionIsQueued	: 1;

	U32		m_ForceShared			: 1;

	U32		m_FlagTextureIndex		: 5;

	U32		m_NextEventType			: 5; // Reduce as needed
};

}; // namespace AnimatedCrowd

#endif // CROWD_SPECTATOR_H
