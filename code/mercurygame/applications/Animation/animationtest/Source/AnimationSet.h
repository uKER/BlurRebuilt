#ifndef ANIMATIONSET_H
#define ANIMATIONSET_H

#include <animation/cycle/CycleInstance.h>
#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>
#include <animation/pose/PoseFormat.h>


namespace Animation
{
	class C_ResourceManager;
	class C_SkeletonResource;
	class C_CycleResource;
}

enum E_CycleId
{
	E_CycleId_WalkF	= 0,
	E_CycleId_WalkB,
	E_CycleId_WalkL,
	E_CycleId_WalkR,
	E_CycleId_RunF,
	E_CycleId_RunB,
	E_CycleId_RunL,
	E_CycleId_RunR,
	
	E_CycleId_AimHAdditive,
	E_CycleId_RunFAim,
	
	E_CycleId_StandIdle,

	E_CycleId_Count
};

#ifndef PLATFORM_SPU
class C_AnimationStore
{
public:
	Animation::C_SkeletonResourceHandle m_skeletonHandle;
	Animation::C_CycleResourceHandle m_cycleHandles[ E_CycleId_Count ];
	
	bool LoadResources();
	
	const Animation::C_Skeleton& GetSkeleton() const
	{
		return *static_cast<Animation::C_SkeletonResource*>(m_skeletonHandle)->GetSkeleton();
	}
	
	const Animation::C_Cycle& GetCycle( E_CycleId cycleId ) const
	{
		return *static_cast<Animation::C_CycleResource*>(m_cycleHandles[ cycleId ])->GetCycle();
	}
};

extern C_AnimationStore gAnimationStore;
#endif

class C_AnimationSet
{
public:
	C_AnimationSet();
	~C_AnimationSet();
	
	void Create();
	
	const Animation::C_Skeleton* m_skeleton;

	Animation::C_CycleInstance m_cycles[ E_CycleId_Count ];
	Animation::C_PoseFormat m_poseFormat;
	
	
};

#endif
