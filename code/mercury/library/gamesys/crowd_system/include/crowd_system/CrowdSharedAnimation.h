#ifndef CROWD_SHARED_ANIMATION_H
#define CROWD_SHARED_ANIMATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdActionSelect.h>
#include <crowd_system/CrowdAnimation.h>
#include <crowd_system/CrowdContext.h>

namespace AnimatedCrowd
{
	class C_Event;
};

namespace AnimatedCrowd
{

//! C_CrowdSharedAnimation

class ALIGN16 C_CrowdSharedAnimation
{
public:
#ifndef PLATFORM_SPU
	C_CrowdSharedAnimation();
	~C_CrowdSharedAnimation();

	bool						Create(const Animation::C_PoseFormat& poseFormat, const Animation::C_Skeleton* pSkeleton,
										const U16 behaviourIndex, const Animation::C_StringId* pEventStringIds, const U32 numEvents, const U8 index);

private:
	void						Destroy();

public:
	void						Update(const C_UpdateContext& context, const Animation::C_JointTransform& hipJointTransform, const CrowdGlobalMood globalMood);
	void						HandleEvent(const C_CrowdResources* pResources, const C_Event& crowdEvent, const CrowdGlobalMood globalMood);
#endif // !PLATFORM_SPU

	const U16					GetBehaviourIndex() const;

	C_CrowdAnimation&			GetAnimation() { return m_Animation; }
	const C_CrowdAnimation&		GetAnimation() const { return m_Animation; }

#ifndef PLATFORM_SPU
	bool						Flush();

private:
	const Animation::C_Cycle*	ChooseCycle(const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList, const CrowdAction action,
									const Animation::C_Cycle* pCurrentCycle, F32& cycleDuration);
	
	void						PlayNextAnimation(const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList, const C_CrowdActionSelect& actionSelect,
									const CrowdGlobalMood globalMood, const F32 blendTime, const bool firstAnimation);
	
	void						SelectNextAction(const C_CrowdActionSelect& actionSelect, const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >& cycleGroupDataList,
									const CrowdEventType eventType, const CrowdActionPriority actionPriority, const U32 actionFlags,
									const F32 minDelayTime, const F32 maxDelayTime, const CrowdGlobalMood globalMood, const F32 dist = 0.f);

	const Animation::C_Cycle*	GetCurrentCycle() const;
#endif // !PLATFORM_SPU

private:
	C_CrowdAnimation	m_Animation;

	F32					m_NextActionCountdown;

	U32					m_BehaviourIndex		: 10;

	U32					m_CurrentAction			: 5;
	U32					m_CurrentActionPriority	: 3;
			
	U32					m_NextAction			: 5;
	U32					m_NextActionPriority	: 3;
	U32					m_NextActionIsQueued	: 1;

	U32					m_NextEventType			: 5;

	friend class C_CrowdSharedAnimationPool;
};

}; // namespace AnimatedCrowd

#endif // CROWD_SHARED_ANIMATION_H
