#ifndef CROWD_RESOURCES_H
#define CROWD_RESOURCES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdPopulation.h>
#include <crowd_system/CrowdAnimation.h>
#include <crowd_system/CrowdUniqueAnimationPool.h>
#include <crowd_system/CrowdSharedAnimationPool.h>
#include <crowd_system/CrowdContext.h>
#include <crowd_system/Effects/CrowdFlags.h>
#include <crowd_system/CrowdEffectItem.h>
#include <crowd_system/CrowdFlagInstanceData.h>

#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseFieldMask.h>

#include <btl/containers/vector.h>
#include <btl/containers/OpenHashMap.h>

#include <Core/Threads.h>
#include <Core/Containers/String.h>

namespace Animation
{
	class C_CycleResource;
	class C_SkeletonResource;
};

namespace GameResource
{
	class C_CrowdData;
};

namespace SceneGraph
{
	class C_SceneGraph;
};

namespace AnimatedCrowd
{
	class C_CrowdActionSelect;
	class C_Event;
};

namespace DynamicEffect
{
	namespace Particle
	{
		class C_Event;
	};
};

namespace AnimatedCrowd
{

//! C_CrowdCachedPalette
class C_CrowdCachedPalette
{
public:
	C_CrowdCachedPalette();

	bool			m_bValid;
	Maths::Mat43*	m_pBoneMatrices;
};

//! C_CrowdResources

class C_CrowdResources
{
public:

	C_CrowdResources(const GameResource::C_CrowdData* pCrowdData, SceneGraph::C_SceneGraph* pSceneGraph, const U32 routeMask);
	~C_CrowdResources();

	void											Destroy();

	const GameResource::C_CrowdData*				GetCrowdData() const;

	SceneGraph::C_SceneGraph*						GetSceneGraph() const;

	C_CrowdPopulation&								GetPopulation();
	const C_CrowdPopulation&						GetPopulation() const;

	bool											CreatePoseFormat();
	void											DestroyPoseFormat();
	const Animation::C_PoseFormat&					GetPoseFormat() const;

	void											CreateAnimationEvents();

	const bool										IsLoaded() const;
	void											SetLoaded(U8* pPaletteMap);

	S32												GetUniqueAnimation(const U16 behaviourIndex);
	S32												GetSharedAnimation(const U16 behaviourIndex);

	void											UpdateAnimations(const C_UpdateContext& context, const CrowdGlobalMood globalMood);
	void											DestroySharedAnimations();

	void											PurgeAnimationPools();

	void											ClearMatrixPalettes() const;

	void											UpdateEffects(const F32 deltaTime);

	const Animation::C_PoseFieldMask&				GetAnimationEventMask(bool enableEvents) const;

	// TODO: [rthomas]	This stuff is all pretty hacky, need to refactor ////////////////////////////////////////////////////////////////////////////////////
	void											SetFlagInstanceNodeTypes(const C_CrowdFlags* pFlags);
	void											SetFlagBehaviourIndex(const U16 behaviourIndex) { m_FlagBehaviourIndex = behaviourIndex; }
	const U16										GetFlagBehaviourIndex() const { return m_FlagBehaviourIndex; }
	void											AddFlagAnimation(const S32 sharedAnimation);
	void											RemoveFlagAnimation(const S32 sharedAnimation);
	const bool										RequiresFlag(const U16 behaviourIndex) const;
	void											ClearFlags();
	void											AddFlag(const Maths::HMat43& transform, const F32* sh, const U8 animationIndex, const S32 textureIndex) const;
	const C_CrowdFlagInstance*						GetFlags() const { return m_Flags; }
	const U16										GetNumFlags() const { return m_NumFlags; }
	const U16										GetNumFlagAnimations() const { return static_cast<U32>(m_FlagAnimations.Size()); }
	const S32										GetFlagAnimation(const U32 index) const;
	
	const U16										GetPropJointIndex() const { return m_PropJointIndex; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void											HandleEvent(const C_Event& globalEvent, const CrowdGlobalMood globalMood);

	bool											Flush();

	const U32										GetNumUsedUniqueAnimations() const;
	const U32										GetNumFreeUniqueAnimations() const;
	C_CrowdUniqueAnimationPool*						GetUniqueAnimationPool() { return &m_UniqueAnimationPool; }
	const C_CrowdUniqueAnimationPool&				GetUniqueAnimationPool() const { return m_UniqueAnimationPool; }

	const U32										GetNumUsedSharedAnimations() const;
	const U32										GetNumFreeSharedAnimations() const;
	C_CrowdSharedAnimationPool*						GetSharedAnimationPool() { return &m_SharedAnimationPool; }
	const C_CrowdSharedAnimationPool&				GetSharedAnimationPool() const { return m_SharedAnimationPool; }

	const U32										GetMaxSharedAnimationsByBehaviour(const U16 behaviourIndex) const;
	const U32										GetNumSharedAnimationsByBehaviour(const U16 behaviourIndex) const;

	EffectSpawnData*								GetEffectSpawnDataPtr() { return &m_EffectSpawnData; }
	const EffectSpawnData&							GetEffectSpawnData() const { return m_EffectSpawnData; }

	FlagInstanceData*								GetFlagInstanceDataPtr() { return &m_FlagInstanceData; }
	const FlagInstanceData&							GetFlagInstanceData() const { return m_FlagInstanceData; }

	const Animation::C_StringId*					GetEventStringIds() const { return m_pEventStringIds; }
	const U32										GetNumEventStringIds() const { return m_NumEventStringIds; }

#ifndef CFG_GOLD
	const C_CrowdSharedAnimation*					GetSharedAnimation(const U16 behaviourIndex, const S32 animationIndex) const;
#endif

	const C_CrowdAnimation*							GetSharedAnimationByIndex(const S32 animationIndex) const;

private:
	void											UpdateUniqueAnimations(const F32 deltaTime, const U32 iHipJointIndex, const Animation::C_JointTransform& hipJointTransform);
	const U32										UpdateSharedAnimations(const C_UpdateContext& context, const Animation::C_JointTransform& hipJointTransform, const CrowdGlobalMood globalMood);

	const GameResource::C_CrowdData*				m_pCrowdData;

	SceneGraph::C_SceneGraph*						m_pSceneGraph;

	C_CrowdPopulation								m_Population;

	Animation::C_PoseFormat							m_PoseFormat;
	Animation::C_StringId*							m_pEventStringIds;
	U32												m_NumEventStringIds;

	U32												m_Loaded : 1;

	C_CrowdUniqueAnimationPool						m_UniqueAnimationPool;
	C_CrowdSharedAnimationPool						m_SharedAnimationPool;
	
	Btl::Vector<C_CrowdCachedPalette>				m_BackupPaletteCache;

	EffectSpawnData									m_EffectSpawnData;

	Animation::C_PoseFieldMask						m_EventEnabledPoseFieldMask;
	Animation::C_PoseFieldMask						m_EventDisabledPoseFieldMask;

	// TODO: [rthomas]	This stuff is all pretty hacky, need to refactor /////////////////////////////////////////////////////////////////////////////////////
	FlagInstanceData								m_FlagInstanceData;
	U16												m_FlagBehaviourIndex;
	mutable U16										m_NumFlags;
	mutable C_CrowdFlagInstance						m_Flags[CROWD_MAX_FLAG_INSTANCES];

	U16												m_PropJointIndex;

	Btl::Vector<S32>								m_FlagAnimations;
	Btl::OpenHashMap<U32, U32>						m_FlagAnimationMap;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

}; // namespace AnimatedCrowd

#endif // CROWD_RESOURCES_H
