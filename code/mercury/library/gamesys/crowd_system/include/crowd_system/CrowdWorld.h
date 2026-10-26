#ifndef CROWD_WORLD_H
#define CROWD_WORLD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/CrowdVisibleBlockData.h>

#include <btl/containers/Vector.h>
#include <btl/containers/OpenHashMap.h>
#include <btl/containers/BitVector.h>
#include <btl/utility/Pair.h>

#include <Display/View/ViewFrustum.h>
#include <Display/View/ViewSettings.h>
#include <SceneGraph/GatherNode.h>
#include <SceneGraph/ViewData.h>

#include <system/scheduler/jobqueue.h>

#ifndef CFG_GOLD
#include <system/Debug/I_DebugRender.h>
#endif

//#include <Display.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
	class C_MultiView;
};

namespace AnimatedCrowd
{
	class C_CrowdResources;
	class C_CrowdSpectator;
	class C_CrowdSpectators;
	class C_CrowdBlockCache;
	class C_CrowdBlock;
	class C_CrowdBlockGrid;
	class C_CrowdBlockVisibilityTask;
	class C_CrowdActorUpdateTask;
	class C_CrowdSharedAnimation;
	class C_CrowdTextureCache;
	//class C_Event;
};

namespace Animation
{
	class C_Pose;
	class C_Skeleton;
};

namespace AnimatedCrowd
{

//! C_CrowdWorld

class ALIGN16 C_CrowdWorld : public SceneGraph::I_GatherNode
{
public:

	C_CrowdWorld(const SceneGraph::C_SceneGraph* pSceneGraph, const U32 viewTypeFlags = 0);
	~C_CrowdWorld();

	bool							Create(C_CrowdResources* pResources, C_CrowdSpectators* pSpectators, const U32 routeMask = 0);
	void							Destroy();

#ifndef CFG_GOLD
	void							DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
	static void						DebugRenderGrid(const C_CrowdBlockGrid* pBlockGrid, DbUtil::I_DebugRenderPtr debugRenderPtr);
	static void						DebugRenderGridCell(const C_CrowdBlockGrid* pBlockGrid, const S32 gridCellX, const S32 gridCellY, DbUtil::I_DebugRenderPtr debugRenderPtr);
	
	void							RenderStats();
#endif // CFG_GOLD

	void							PreUpdate(const Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >& activeEvents,
										const C_CrowdTextureCache* pFlagTextureCache, const Scheduler::PolicyHandle jobQueuePolicyHandle, const F32 deltaTime, const U32 frameCount);
	void							Update(const F32 deltaTime, const U32 frameCount);
	void							UpdateEffects(const F32 deltaTime);
	
	// SceneGraph integration
	virtual void					Accept(SceneGraph::I_Visitor& visitor) const;

	bool							Flush();

	U8*								GetPaletteMap() { return m_pPaletteMap; }

	void							SetGlobalMood(const CrowdGlobalMood mood);
	static CrowdGlobalMood			GetGlobalMood() { return s_GlobalMood; }

	const C_CrowdVisibleBlockData&	GetVisibleBlockData() const { return m_VisibleBlockData[m_VisibleBlockDataIndex]; }

	const U32						GetViewTypeFlags() const { return m_ViewTypeFlags; }

private:
	// SceneGraph integration
	virtual void					OnBeginFrame() const;
	virtual void					OnGather(const SceneGraph::C_View& view) const;

	void							BeginFrame() const;
	void							EndFrame() const;
	void							Gather(const SceneGraph::C_View& view) const __restrict;
	void							Gather(const SceneGraph::C_MultiView& multiView) const;

	const bool						VisibleGridCellGathered(const S32 cellIndex) const;
	const bool						VisibleBlockGathered(const S32 routeBlockIndex) const;
	void							AllocateBlocks(const U32 numBlocks);
	void							DestroyBlocks();

	void							CacheVisibleBlocks(const Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >& activeEvents, const U32 frameNumber);

public:
	void							ScheduleBlockVisibilityTasks(const Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >* pActiveEvents, const Scheduler::PolicyHandle jobQueuePolicyHandle) const;
	void							ScheduleCachedActorUpdateTasks(const Scheduler::PolicyHandle jobQueuePolicyHandle, const F32 fDeltaTime) const;

private:
	U8*									m_pPaletteMap;

	C_CrowdResources*					m_pResources;
	C_CrowdSpectators*					m_pSpectators;

	C_CrowdBlockCache*					m_pBlockCache;

	mutable C_CrowdBlockVisibilityTask*	m_pBlockVisibilityTask;
	mutable C_CrowdActorUpdateTask*		m_pActorUpdateTask;

	mutable C_CrowdVisibleBlockData		m_VisibleBlockData[2];
	mutable U32							m_VisibleBlockDataIndex;

	mutable SceneGraph::ViewData		m_Views[CROWD_MAX_VIEWS];
	mutable U32							m_NumViews;

	mutable Btl::Vector<C_CrowdPotentiallyVisibleBlock>
										m_PotentiallyVisibleBlocks;
	
	mutable Btl::OpenHashMap<U16, U16>	m_PotentiallyVisibleBlockMap;

	Btl::Vector<CrowdDelimiter>			m_SortedBlocksXAxis;
	Btl::Vector<CrowdDelimiter>			m_SortedBlocksZAxis;

	Scheduler::GroupHandle				m_BlockVisibilityGroupHandle;
	Scheduler::GroupHandle				m_ActorUpdateGroupHandle;

	C_CrowdBlock* 						m_Blocks;
	U32									m_NumBlocks;

	mutable Btl::BitVector				m_GridCells;
	
	U32									m_FrameStamp;

	U32									m_RouteMask;
	U32*								m_RouteBlockOffsets;

	U32									m_ViewTypeFlags;

	U32									m_HipJointIndex;

#ifdef CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD
	Btl::Vector<C_CrowdSharedAnimation*, Btl::VectorParameters_Static<E_BEHAVIOUR_TYPE_COUNT> >
										m_SharedAnimations;
#endif // CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD

	static CrowdGlobalMood				s_GlobalMood;
};

}; // namespace AnimatedCrowd

#endif // CROWD_WORLD_H
