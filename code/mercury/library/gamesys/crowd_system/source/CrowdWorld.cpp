#include "CrowdSystemPch.h"

#include <crowd_system/CrowdWorld.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdSpectators.h>
#include <crowd_system/CrowdBlock.h>
#include <crowd_system/CrowdBlockCache.h>
#include <crowd_system/CrowdBlockGrid.h>
#include <crowd_system/CrowdBlockRef.h>
#include <crowd_system/CrowdBlockVisibilityTask.h>
#include <crowd_system/CrowdActorUpdateTask.h>
#include <crowd_system/CrowdContext.h>
#include <crowd_system/CrowdCharacterRef.h>
#include <crowd_system/CrowdDebug.h>
#include <crowd_system/CrowdRandom.h>
#include <crowd_system/CrowdFlagInstanceData.h>

#include <game_resource/CrowdData.h>

#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>

#include <btl/containers/Algorithm.h>

#include <Display/View/View.h>

#include <scenegraph/SceneGraph.h>
#include <SceneGraph/View.h>
#include <SceneGraph/MultiView.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/RenderNode.h>

#include <system/Perf.h>

#include <Btl/Containers/Radix.h>

namespace AnimatedCrowd
{
#ifndef CFG_GOLD
Debug::Bool		g_DebugDisableAnimation("GameSys.CrowdSystem.Animation.Disable", false);
Debug::Bool		g_DebugCycles("GameSys.CrowdSystem.Animation.DebugCycles", false);
Debug::Bool		g_DebugRenderForceLoopCurrentCycle("GameSys.CrowdSystem.Animation.LoopCurrentCycle", false);
Debug::Bool		g_DebugRenderGrid("GameSys.CrowdSystem.World.RenderGrid", false);

Debug::Bool		g_DebugRenderEvents("GameSys.CrowdSystem.Events.DebugRender", false, Debug::Persist());
Debug::Bool		g_DebugRenderEffects("GameSys.CrowdSystem.Effects.DebugRender", false, Debug::Persist());
#endif // CFG_GOLD

Debug::Float	g_MinReactionTime("GameSys.CrowdSystem.Events.MinReactionTime", 0.1f, Debug::Range(0.f, 1.f), Debug::Step(0.01f));
Debug::Float	g_MaxReactionTime("GameSys.CrowdSystem.Events.MaxReactionTime", 0.15f, Debug::Range(0.f, 1.f), Debug::Step(0.01f));

Debug::Float	g_RandomStartTime("GameSys.CrowdSystem.Animation.RandomStartTime", 2.f, Debug::Range(0.f, 4.f), Debug::Step(0.1f));

extern Debug::Float	g_FlagOffset;

#ifndef CFG_GOLD
Debug::Int		g_DebugBlockRoute("GameSys.CrowdSystem.World.BlockRoute", -1);
Debug::Bool		g_DebugRenderRouteBlocks("GameSys.CrowdSystem.World.DrawRouteBlocks", false);
Debug::Bool		g_DebugRenderAllBlocks("GameSys.CrowdSystem.World.DrawAllBlocks", false);

Debug::Bool		g_DebugRenderBlocksInstances("GameSys.CrowdSystem.World.RenderBlockInstances", false);
#endif // CFG_GOLD

Debug::Bool		g_CullBlocksAgainstFrustum("GameSys.CrowdSystem.World.CullBlocksAgainstFrustum", true);
Debug::Int		g_CrowdBlockFrameThreshold("GameSys.CrowdSystem.World.BlockFrameThreshold", 15, Debug::Range(1, 30));

CrowdGlobalMood C_CrowdWorld::s_GlobalMood = E_CROWD_MOOD_EXCITEMENT_LOW;

C_CrowdWorld::C_CrowdWorld(const SceneGraph::C_SceneGraph* pSceneGraph, const U32 viewTypeFlags)
	: SceneGraph::I_GatherNode(SceneGraph::E_GatherNodeType_Unknown, pSceneGraph),
	m_pPaletteMap(NULL),
	m_pResources(NULL),
	m_pSpectators(NULL),
	m_FrameStamp(0),
	m_Blocks(NULL),
	m_NumBlocks(0),
	m_RouteBlockOffsets(NULL),
	m_ViewTypeFlags(viewTypeFlags),
	m_pBlockVisibilityTask(NULL),
	m_pActorUpdateTask(NULL),
	m_BlockVisibilityGroupHandle(-1),
	m_ActorUpdateGroupHandle(-1),
	m_VisibleBlockDataIndex(0),
	m_HipJointIndex(0),
	m_RouteMask(0)
{
	m_pPaletteMap = new U8[32 * 1024];

	m_pBlockCache = new C_CrowdBlockCache();

	s_GlobalMood = E_CROWD_MOOD_EXCITEMENT_LOW;

	m_VisibleBlockData[0].Clear();
	m_VisibleBlockData[1].Clear();
}

C_CrowdWorld::~C_CrowdWorld()
{
	Destroy();

	SAFE_DELETE_ARY(m_pPaletteMap);

	SAFE_DELETE_PTR(m_pBlockCache);
}

bool C_CrowdWorld::Create(C_CrowdResources* pResources, C_CrowdSpectators* pSpectators, const U32 routeMask)
{
	m_pResources = pResources;
	m_pSpectators = pSpectators;

	if (!m_pResources || !m_pResources->GetCrowdData())
		return false;

	m_RouteMask = CrowdMaskRoutes(routeMask);

	const S32 numBlocks = m_pResources->GetCrowdData()->GetNumBlocks(m_RouteMask);

	if (numBlocks <= 0)
		return true;

	AllocateBlocks(numBlocks);

	BIZ_ASSERT(m_pResources->GetCrowdData()->GetBlockGrid());

	m_GridCells.Resize(m_pResources->GetCrowdData()->GetBlockGrid()->GetGridDimensions() * 2);
	m_GridCells.ClearBits();

	if (!m_pBlockCache)
		return false;

	S32 hipJointFieldIndex = m_pResources->GetPoseFormat().FindFieldByName(Animation::E_PoseFieldType_JointTransform, "hips");
	if (hipJointFieldIndex >= 0)
		m_HipJointIndex = static_cast<U32>(hipJointFieldIndex);
	
	m_pBlockCache->Create(m_pResources, m_Blocks, m_pSpectators);

	m_BlockVisibilityGroupHandle = Scheduler::CreateGroup();
	m_ActorUpdateGroupHandle = Scheduler::CreateGroup();

	m_pBlockVisibilityTask = new C_CrowdBlockVisibilityTask();
	m_pActorUpdateTask = new C_CrowdActorUpdateTask();

	m_SortedBlocksXAxis.Reserve(numBlocks * 2);
	m_SortedBlocksZAxis.Reserve(numBlocks * 2);

	for (U32 iBlock = 0, count = m_NumBlocks; iBlock < count; ++iBlock)
	{
		BIZ_ASSERT(m_Blocks[iBlock].GetBlockRefIndex() >= 0);
		const C_CrowdBlockRef& blockRef = m_pResources->GetCrowdData()->GetCrowdBlockRef(m_Blocks[iBlock].GetBlockRefIndex());

		{
			CrowdDelimiter beginBlock;
			beginBlock.m_Data = iBlock;
			beginBlock.m_BoundaryType = 0;

			Maths::HVec3 vMin = blockRef.GetMin();

			beginBlock.m_Value = vMin.X();
			m_SortedBlocksXAxis.PushBack(beginBlock);
			
			beginBlock.m_Value = vMin.Z();
			m_SortedBlocksZAxis.PushBack(beginBlock);
		}

		{
			CrowdDelimiter endBlock;
			endBlock.m_Data = iBlock;
			endBlock.m_BoundaryType = 1;

			Maths::HVec3 vMax = blockRef.GetMax();

			endBlock.m_Value = vMax.X();
			m_SortedBlocksXAxis.PushBack(endBlock);
			
			endBlock.m_Value = vMax.Z();
			m_SortedBlocksZAxis.PushBack(endBlock);
		}
	}

	Btl::RadixSorter<Btl::Vector<CrowdDelimiter> > radixSorter;
	radixSorter.Sort(m_SortedBlocksXAxis);
	radixSorter.Sort(m_SortedBlocksZAxis);

#ifdef CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD
	// Spiking matrix palettes with good data for all behaviours on-load (prevents t-poses when in photomode)
	{
		C_UpdateContext context;
		context.m_Resources		= m_pResources;
		context.m_DeltaTime		= 0.f;
		context.m_FrameCount	= 0;
		context.m_HipJointIndex	= m_HipJointIndex;
		
		for (U32 i = 0, count = E_BEHAVIOUR_TYPE_COUNT; i < count; ++i)
		{
			bool bShared = false;
			C_CrowdAnimation* pAnimation = m_pResources->GetAnimation(E_DETAIL_LEVEL_LOW, static_cast<CrowdBehaviourType>(i), bShared);

			if (pAnimation)
			{
				if (!bShared)
					m_pResources->FreeLocalAnimation(pAnimation);
				else
				{
					C_CrowdSharedAnimation* pSharedAnimation = (C_CrowdSharedAnimation*)pAnimation;
					pSharedAnimation->SetDirty();
					pSharedAnimation->AddRef();
					m_SharedAnimations.PushBack(pSharedAnimation);
				}
			}
		}
	}
#endif // CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD

	m_VisibleBlockData[0].Clear();
	m_VisibleBlockData[1].Clear();

	return true;
}

void C_CrowdWorld::Destroy()
{
	if (m_pBlockCache)
		m_pBlockCache->Destroy();

	DestroyBlocks();

#ifdef CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD
	m_SharedAnimations.Clear();
#endif // CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD

	m_GridCells.Destroy();

	if (m_BlockVisibilityGroupHandle >= 0)
	{
		Scheduler::DestroyGroup(m_BlockVisibilityGroupHandle);
		m_BlockVisibilityGroupHandle = -1;
	}

	if (m_ActorUpdateGroupHandle >= 0)
	{
		Scheduler::DestroyGroup(m_ActorUpdateGroupHandle);
		m_ActorUpdateGroupHandle = -1;
	}

	SAFE_DELETE_PTR(m_pBlockVisibilityTask);
	SAFE_DELETE_PTR(m_pActorUpdateTask);

	m_VisibleBlockData[0].Clear();
	m_VisibleBlockData[1].Clear();

	m_PotentiallyVisibleBlocks.Destroy();
	m_PotentiallyVisibleBlockMap.Clear();

	m_SortedBlocksXAxis.Destroy();
	m_SortedBlocksZAxis.Destroy();
}

#ifndef CFG_GOLD
void C_CrowdWorld::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!m_pResources)
		return;

	const GameResource::C_CrowdData* pCrowdData = m_pResources->GetCrowdData();

	if (pCrowdData)
	{
		if (m_pBlockCache)
			m_pBlockCache->DebugRender(debugRenderPtr, m_Blocks, m_FrameStamp, pCrowdData->GetZDirection());

		if (g_DebugRenderGrid)
		{
			DebugRenderGrid(
				pCrowdData->GetBlockGrid(),
				debugRenderPtr
			);
		}

		if (g_DebugRenderRouteBlocks)
		{
			for (U32 i = 0, count = static_cast<U32>(pCrowdData->GetNumBlocks()); i < count; ++i)
			{
				const C_CrowdBlockRef& blockRef = pCrowdData->GetCrowdBlockRef(i);

				if (g_DebugRenderAllBlocks || g_DebugBlockRoute == blockRef.GetRoute())
				{
					debugRenderPtr->Draw3dBox(
						blockRef.GetPosition() - blockRef.GetExtents() + Maths::HVec3(0.f, 0.5f * blockRef.GetRoute(), 0.f),
						blockRef.GetPosition() + blockRef.GetExtents() + Maths::HVec3(0.f, 0.5f * blockRef.GetRoute(), 0.f),
						Maths::Constants<Maths::HMat43>::Identity(),
						GetRouteColour(blockRef.GetRoute())
					);

					char routeBuffer[8];
					sprintf_s(routeBuffer, lengthof(routeBuffer), "%d", blockRef.GetRoute());

					debugRenderPtr->Draw3dText(blockRef.GetPosition() + Maths::HVec3(0.f, 0.5f * blockRef.GetRoute(), 0.f), Util::C_String(routeBuffer));

					if (g_DebugRenderBlocksInstances)
					{
						for (U32 iInstance = 0, instanceCount = blockRef.GetNumInstances(); iInstance < instanceCount; ++iInstance)
						{
							const C_CrowdInstance& instance = pCrowdData->GetCrowdInstance(blockRef.GetInstanceIndex(iInstance));
							debugRenderPtr->Draw3dPoint(instance.GetPosition() + Maths::HVec3(0.f, 0.5f * blockRef.GetRoute(), 0.f), 0.25f, GetRouteColour(blockRef.GetRoute()));
						}
					}
				}
			}
		}
	}
}

void C_CrowdWorld::DebugRenderGrid(const C_CrowdBlockGrid* pBlockGrid, DbUtil::I_DebugRenderPtr debugRenderPtr)
{
	if (pBlockGrid)
	{
		debugRenderPtr->SetDepthEnable(false);
		for (S32 i = 0, count = pBlockGrid->GetGridDimensions() + 1; i < count; ++i)
		{
			debugRenderPtr->Draw3dLine(Maths::HVec3(pBlockGrid->GetGridMin().X() + (CROWD_GRID_CELL_SIZE * i), 0.f, pBlockGrid->GetGridMin().Y()),
				Maths::HVec3(pBlockGrid->GetGridMin().X() + (CROWD_GRID_CELL_SIZE * i), 0.f, pBlockGrid->GetGridMax().Y()), Maths::RGBA(128, 128, 255, 255));

			debugRenderPtr->Draw3dLine(Maths::HVec3(pBlockGrid->GetGridMin().X(), 0.f, pBlockGrid->GetGridMin().Y() + (CROWD_GRID_CELL_SIZE * i)),
				Maths::HVec3(pBlockGrid->GetGridMax().X(), 0.f, pBlockGrid->GetGridMin().Y() + (CROWD_GRID_CELL_SIZE * i)), Maths::RGBA(128, 128, 255, 255));
		}

		debugRenderPtr->SetDepthEnable(true);
	}
}

void C_CrowdWorld::DebugRenderGridCell(const C_CrowdBlockGrid* pBlockGrid, const S32 gridCellX, const S32 gridCellY, DbUtil::I_DebugRenderPtr debugRenderPtr)
{
	if (!pBlockGrid)
		return;

	const C_CrowdBlockGridCell* pCell = pBlockGrid->GetGridCell(gridCellX, gridCellY);

	if (pCell)
	{
		debugRenderPtr->Draw3dBox(Maths::HVec3(pBlockGrid->GetGridMin().X() + (CROWD_GRID_CELL_SIZE * gridCellX), 0.f, pBlockGrid->GetGridMin().Y() + (CROWD_GRID_CELL_SIZE * gridCellY)),
			Maths::HVec3(pBlockGrid->GetGridMin().X() + (CROWD_GRID_CELL_SIZE * (gridCellX + 1)), 0.f, pBlockGrid->GetGridMin().Y() + (CROWD_GRID_CELL_SIZE * (gridCellY + 1))), Maths::Constants<Maths::HMat43>::Identity(), Maths::RGBA(192, 192, 255));
	}
}

void C_CrowdWorld::RenderStats()
{
	Maths::HVec2 statsPos(30.f, 500.f);

	if (m_pResources && m_pResources->GetCrowdData())
	{
		Display::FontRender()->Draw(Util::C_String().Printf("Instances: [ %d | %d ]", m_pResources->GetCrowdData()->GetNumInstances(m_RouteMask), m_pResources->GetCrowdData()->GetNumInstances()).Chars(),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);
	}

	if (m_pSpectators)
	{
		Display::FontRender()->Draw(Util::C_String().Printf("Spectator Cache: [ %d | %d | %d ]", m_pSpectators->GetNumSpectators(), m_pSpectators->GetNumFreeSpectators(), CROWD_MAX_SPECTATORS).Chars(),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);
	}

	if (m_pBlockCache)
	{
		Display::FontRender()->Draw(Util::C_String().Printf("Active Spectators: [ %d | %d | %d ]", m_pBlockCache->GetNumActiveSpectators(), CROWD_MAX_SPECTATORS - m_pBlockCache->GetNumActiveSpectators(), CROWD_MAX_SPECTATORS).Chars(),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);

		Display::FontRender()->Draw(Util::C_String().Printf("Block Cache: [ %d | %d | %d ]", m_pBlockCache->GetNumCachedBlocks(), m_NumBlocks - m_pBlockCache->GetNumCachedBlocks(), m_NumBlocks).Chars(),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);

		Display::FontRender()->Draw(Util::C_String().Printf("Visible Blocks: [ %d | %d | %d ]", m_pBlockCache->GetNumLowDetailBlocks(), m_pBlockCache->GetNumHighDetailBlocks(), (m_pBlockCache->GetNumLowDetailBlocks() + m_pBlockCache->GetNumHighDetailBlocks())).Chars(),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);
	}
	
	if (m_pResources)
	{
		Display::FontRender()->Draw(Util::C_String().Printf("Animations: [ %d | %d | %d ]", m_pResources->GetNumUsedSharedAnimations(), m_pResources->GetNumUsedUniqueAnimations(), (m_pResources->GetNumUsedSharedAnimations() + m_pResources->GetNumUsedUniqueAnimations())).Chars(),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);

		Display::FontRender()->Draw(Util::C_String().Printf("Unique Pool: [ %d | %d | %d ]", m_pResources->GetNumUsedUniqueAnimations(), m_pResources->GetNumFreeUniqueAnimations(), CROWD_MAX_LOCAL_ANIMATIONS),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);

		Display::FontRender()->Draw(Util::C_String().Printf("Shared Pool: [ %d | %d | %d ]", m_pResources->GetNumUsedSharedAnimations(), m_pResources->GetNumFreeSharedAnimations(), CROWD_MAX_SHARED_ANIMATIONS),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);

		U32 sizeOfCrowdData = 0;
		U32 sizeOfBehaviours = m_pResources->GetCrowdData()->GetNumBehaviours() * sizeof(U32); sizeOfCrowdData += sizeOfBehaviours;
		U32 sizeOfInstances = m_pResources->GetCrowdData()->GetNumInstances() * sizeof(C_CrowdInstance); sizeOfCrowdData += sizeOfInstances;
		U32 sizeOfBlockRefs = m_pResources->GetCrowdData()->GetNumBlocks() * sizeof(C_CrowdBlockRef); sizeOfCrowdData += sizeOfBlockRefs;
		U32 sizeOfSHNodes = m_pResources->GetCrowdData()->GetNumSHNodes() * sizeof(C_CrowdSHNodeData); sizeOfCrowdData += sizeOfSHNodes;
		U32 sizeOfCharacters = m_pResources->GetCrowdData()->GetNumCharacters() * sizeof(U8); sizeOfCrowdData += sizeOfCharacters;
		U32 sizeOfBlockAndInstanceCounts = m_pResources->GetCrowdData()->GetRouteBlockAndInstanceCounts().Size() * sizeof(Btl::Pair<S16, S16>); sizeOfCrowdData += sizeOfBlockAndInstanceCounts;
		
		U32 sizeOfCrowdGrid = 0;
		sizeOfCrowdGrid += sizeof(C_CrowdBlockGrid);
		sizeOfCrowdGrid += m_pResources->GetCrowdData()->GetBlockGrid()->GetNumGridCells() * sizeof(C_CrowdBlockGridCell);
		for (U32 i = 0, count = m_pResources->GetCrowdData()->GetBlockGrid()->GetNumGridCells(); i < count; ++i)
		{
			const C_CrowdBlockGridCell* pGridCell = m_pResources->GetCrowdData()->GetBlockGrid()->GetGridCell(i);
			if (pGridCell)
				sizeOfCrowdGrid += pGridCell->GetNumBlockIndexes() * sizeof(U16);
		}

		sizeOfCrowdData += sizeOfCrowdGrid;

		Display::FontRender()->Draw(Util::C_String().Printf("Crowd Data: [ %d | %d | %d | %d | %d | %d | %d | %d ] bytes", sizeOfBehaviours, sizeOfInstances, sizeOfBlockRefs, sizeOfSHNodes, sizeOfCharacters, sizeOfBlockAndInstanceCounts, sizeOfCrowdGrid, sizeOfCrowdData),
			statsPos.X(), statsPos.Y(), 10.f, 10.f, Maths::RGBA(255, 255, 255, 255));

		statsPos.Y(statsPos.Y() + 15.f);
	}
}
#endif // CFG_GOLD

void C_CrowdWorld::PreUpdate(const Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >& activeEvents, const C_CrowdTextureCache* pFlagTextureCache,
							 const Scheduler::PolicyHandle jobQueuePolicyHandle, const F32 deltaTime, const U32 frameCount)
{
	if (m_BlockVisibilityGroupHandle >= 0)
		Scheduler::Flush(m_BlockVisibilityGroupHandle);

	if (!m_pSpectators || !m_pBlockCache || !m_pResources)
		return;

	m_pResources->ClearMatrixPalettes();
	m_pBlockCache->EmptyBuckets();
	CacheVisibleBlocks(activeEvents, frameCount);
	m_pBlockCache->Update(m_Blocks, pFlagTextureCache, m_pPaletteMap);

	for (U32 i = 0, count = static_cast<U32>(activeEvents.Size()); i < count; ++i)
	{
		if (activeEvents[i].GetAreaType() == C_Event::E_AREA_TYPE_GLOBAL)
		{
			if (!activeEvents[i].HasExpired())
				m_pResources->HandleEvent(activeEvents[i], GetGlobalMood());
			break;
		}
	}

	ScheduleCachedActorUpdateTasks(jobQueuePolicyHandle, deltaTime);
}

void C_CrowdWorld::Update(const F32 deltaTime, const U32 frameCount)
{
	if (m_ActorUpdateGroupHandle >= 0)
		Scheduler::Flush(m_ActorUpdateGroupHandle);

	if (!m_pSpectators || !m_pBlockCache || !m_pResources)
		return;

	SceneGraph::SetPaletteMap(m_pResources->GetSceneGraph(), m_pPaletteMap);

	if (m_pActorUpdateTask)
	{
		PERF_EVENT("CrowdUpdate.World.FlagInstances");
		const FlagInstanceData& flagInstanceData = m_pResources->GetFlagInstanceData();
		for (U32 i = 0, count = flagInstanceData.m_NumInstances; i < count; ++i)
		{
			const FlagInstance& flagInstance = flagInstanceData.m_Instances[i];
			m_pResources->AddFlag(flagInstance.m_Transform, m_pResources->GetCrowdData()->GetCrowdInstanceSH(flagInstance.m_InstanceIndex), flagInstance.m_AnimationIndex, flagInstance.m_TextureIndex);
		}
	}

	C_UpdateContext context;
	context.m_Resources		= m_pResources;
	context.m_DeltaTime		= deltaTime;
	context.m_FrameCount	= frameCount;
	context.m_HipJointIndex	= m_HipJointIndex;

#ifndef CFG_GOLD
	if (!g_DebugDisableAnimation)
#endif
	{
		PERF_EVENT("CrowdUpdate.World.Animations");
		m_pResources->UpdateAnimations(context, GetGlobalMood());
	}

#ifdef CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD
	{
		U32 i = 0;
		
		while (i < static_cast<U32>(m_SharedAnimations.Size()))
		{
			if (m_SharedAnimations[i]->IsValid())
			{
				m_SharedAnimations[i]->RemoveRef();

				if (m_SharedAnimations.Size() > 1)
					m_SharedAnimations[i] = m_SharedAnimations[m_SharedAnimations.Size() - 1];
				
				m_SharedAnimations.PopBack();
			}
			else
			{
				m_SharedAnimations[i++]->SetDirty();
			}
		}
	}
#endif // CROWD_SPIKE_SHARED_ANIMATIONS_ON_LOAD

	m_FrameStamp = frameCount;
}

void C_CrowdWorld::UpdateEffects(const F32 deltaTime)
{
	if (!m_pSpectators || !m_pBlockCache || !m_pResources)
		return;

	m_pResources->UpdateEffects(deltaTime);
}

void C_CrowdWorld::ScheduleBlockVisibilityTasks(const Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >* pActiveEvents, const Scheduler::PolicyHandle jobQueuePolicyHandle) const
{
	if (m_pBlockVisibilityTask)
	{
		BIZ_ASSERT(m_NumViews <= CROWD_MAX_VIEWS);

		m_pBlockVisibilityTask->m_PotentiallyVisibleBlocks = m_PotentiallyVisibleBlocks.First();
		m_pBlockVisibilityTask->m_NumPotentiallyVisibleBlocks = static_cast<U32>(m_PotentiallyVisibleBlocks.Size());

		m_pBlockVisibilityTask->m_Views = &m_Views[0];
		m_pBlockVisibilityTask->m_NumViews = m_NumViews;
		
		m_pBlockVisibilityTask->m_ActiveEvents = (*pActiveEvents).First();
		m_pBlockVisibilityTask->m_NumActiveEvents = static_cast<U32>(pActiveEvents->Size());

		m_pBlockVisibilityTask->m_SortedBlocksXAxis = m_SortedBlocksXAxis.First();
		m_pBlockVisibilityTask->m_SortedBlocksZAxis = m_SortedBlocksZAxis.First();
		m_pBlockVisibilityTask->m_NumSortedBlocks = static_cast<U32>(m_SortedBlocksXAxis.Size());
		m_pBlockVisibilityTask->m_NumBlocks = m_NumBlocks;

		m_VisibleBlockDataIndex = !m_VisibleBlockDataIndex;
		m_pBlockVisibilityTask->m_VisibleBlockData = &m_VisibleBlockData[m_VisibleBlockDataIndex];

		C_CrowdBlockVisibilityTask::Schedule(m_pBlockVisibilityTask, m_BlockVisibilityGroupHandle, jobQueuePolicyHandle);
	}
}

void C_CrowdWorld::ScheduleCachedActorUpdateTasks(const Scheduler::PolicyHandle jobQueuePolicyHandle, const F32 fDeltaTime) const
{
	if (m_pActorUpdateTask)
	{
		// Inputski
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_pPoseFormat = &m_pResources->GetPoseFormat();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_pSkeleton = m_pResources->GetPopulation().GetSkeleton()->GetSkeletonResource()->GetSkeleton();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_pEventStringIds = m_pResources->GetEventStringIds();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_NumEvents = m_pResources->GetNumEventStringIds();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_fDeltaTime = fDeltaTime;
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_ZDir = m_pResources->GetCrowdData()->GetZDirection();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_FlagBehaviourIndex = m_pResources->GetFlagBehaviourIndex();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_pAnimationEventEffectJointIndexes = &m_pResources->GetPopulation().GetAnimationEventEffectJointIndexes();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_pActionSelect = &m_pResources->GetPopulation().GetActionSelect();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_pCycleGroupDataList = &m_pResources->GetPopulation().GetCycleGroupDataList();
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_GlobalMood = static_cast<U32>(GetGlobalMood());
		m_pActorUpdateTask->m_SpectatorUpdateInput.m_NumBlocks = m_NumBlocks;

#if 0
		U32 sizeOfEffectJointIndexes = 4;
		U32 sizeOfActionSelect = sizeof(C_CrowdActionSelect);
		U32 sizeOfCycleGroupDataList = sizeof(CrowdCycleGroupData) * CROWD_MAX_SHARED_ANIMATIONS;
		U32 sizeOfCycleGroups = sizeof(C_CrowdCycleGroup) * CROWD_MAX_SHARED_ANIMATIONS;
		
		U32 sizeOfBlockCache = sizeof(C_CrowdBlockCache);
		U32 sizeOfPaletteMap = 32 * 1024;
		U32 sizeOfUniqueAnimPool = sizeof(C_CrowdUniqueAnimationPool);
		U32 sizeOfSharedAnimPool = sizeof(C_CrowdSharedAnimationPool);
		U32 sizeOfEffectSpawnBuffer = sizeof(EffectSpawnItem) * 64;
		U32 sizeOfFlagInstanceData = sizeof(FlagInstanceData);
		U32 sizeOfRandom = sizeof(AnimatedCrowd::C_Random);
		U32 sizeOfBlock = sizeof(C_CrowdBlock);
		U32 sizeOfBlocks = sizeof(C_CrowdBlock) * m_NumBlocks;
		U32 sizeOfSpectator = sizeof(C_CrowdSpectator);
		U32 sizeOfSpectators = sizeof(C_CrowdSpectator) * CROWD_MAX_SPECTATORS;

		U32 totalSize = sizeOfEffectJointIndexes + sizeOfActionSelect + sizeOfCycleGroupDataList + sizeOfCycleGroups +
			sizeOfBlockCache + sizeOfPaletteMap + sizeOfUniqueAnimPool + sizeOfSharedAnimPool + sizeOfEffectSpawnBuffer + sizeOfFlagInstanceData + sizeOfRandom + sizeOfBlocks + sizeOfSpectators;
#endif

		// Outputski
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pBlockCache = m_pBlockCache;
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pPaletteMap = m_pPaletteMap;
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pUniqueAnimationPool = m_pResources->GetUniqueAnimationPool();
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pSharedAnimationPool = m_pResources->GetSharedAnimationPool();
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pEffectSpawnData = m_pResources->GetEffectSpawnDataPtr();
		
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pFlagInstanceData = m_pResources->GetFlagInstanceDataPtr();
		if (m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pFlagInstanceData)
			m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pFlagInstanceData->m_NumInstances = 0;
		
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pRandom = &AnimatedCrowd::C_Random::Instance();
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pBlocks = m_Blocks;
		m_pActorUpdateTask->m_SpectatorUpdateOutput.m_pSpectators = m_pSpectators->GetSpectators();

		C_CrowdActorUpdateTask::Schedule(m_pActorUpdateTask, m_ActorUpdateGroupHandle, jobQueuePolicyHandle);
	}
}

void C_CrowdWorld::Accept(SceneGraph::I_Visitor &visitor) const
{
	visitor.Visit(*this);
}

bool C_CrowdWorld::Flush()
{
	if (m_pBlockCache && m_Blocks)
	{
		if (!m_pBlockCache->Flush(m_Blocks))
			return false;
	}

	return true;
}

void C_CrowdWorld::SetGlobalMood(const CrowdGlobalMood mood)
{
	s_GlobalMood = mood;
}

void C_CrowdWorld::OnBeginFrame() const
{
	BeginFrame();
}

void C_CrowdWorld::OnGather(const SceneGraph::C_View& view) const
{
	if(m_ViewTypeFlags & view.GetTypeFlag())
	{
		// TODO: [rthomas]	See if there is a better way to avoid gathering for main and main prepass
		if (!(view.GetGroup(0) && _stricmp(view.GetGroup(0)->GetName(), "PrepassView") == 0))
			Gather(view);
	}
}

void C_CrowdWorld::BeginFrame() const
{
	m_GridCells.ClearBits();
	
	m_PotentiallyVisibleBlocks.Clear();
	m_PotentiallyVisibleBlockMap.Clear();

	m_NumViews = 0;
}

void C_CrowdWorld::Gather(const SceneGraph::C_View& view) const __restrict
{
	PERF_EVENT("CrowdUpdate.Gather.View");

	if (!m_GridCells.Size())
		return;

	m_Views[m_NumViews] = view.GetViewData();

	const Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>* __restrict pBlockRefs(&m_pResources->GetCrowdData()->GetCrowdBlockRefs());
	const C_CrowdBlockGrid* __restrict pBlockGrid(m_pResources->GetCrowdData()->GetBlockGrid());

	S32 cellX = -1;
	S32 cellY = -1;

	if (const C_CrowdBlockGridCell* __restrict pGridCell = pBlockGrid->GetGridCell(m_Views[m_NumViews].viewSettings3D.GetCameraPosition(), cellX, cellY))
	{
		const S16 gridDimensions = pBlockGrid->GetGridDimensions();

		if (!(m_GridCells.GetBitValue(cellX) && m_GridCells.GetBitValue(cellY + gridDimensions)))
		{
			for (S32 iBlock = 0, countB = pGridCell->GetNumBlockIndexes(); iBlock < countB; ++iBlock)
			{
				S32 blockRefIndex = pGridCell->GetBlockIndex(iBlock);
				U16 visibleBlockIndex = static_cast<U16>(m_PotentiallyVisibleBlocks.Size());

				const C_CrowdBlockRef& blockRef((*pBlockRefs)[blockRefIndex]);
				
				if (m_RouteMask & (1 << (blockRef.GetRoute() + 1)))
				{
					Btl::OpenHashMap<U16, U16>::Iterator iter = m_PotentiallyVisibleBlockMap.Find(blockRefIndex);

					if (iter == m_PotentiallyVisibleBlockMap.End())
					{
						U32 routeBlockIndex = (blockRefIndex - m_RouteBlockOffsets[blockRef.GetRoute() + 1]);
						
						m_PotentiallyVisibleBlockMap.Insert(blockRefIndex, visibleBlockIndex);
						m_PotentiallyVisibleBlocks.PushBack(C_CrowdPotentiallyVisibleBlock(blockRefIndex, routeBlockIndex, blockRef.GetMin(), blockRef.GetMax()));
					}
				}
			}
		}
		
		m_GridCells.SetBit(cellX);
		m_GridCells.SetBit(cellY + gridDimensions);
	}

	m_NumViews++;
}

void C_CrowdWorld::Gather(const SceneGraph::C_MultiView& multiView) const
{
	PERF_EVENT("CrowdUpdate.Gather");

	if (!m_pResources || !m_pResources->GetCrowdData() || !m_pResources->GetCrowdData()->GetBlockGrid())
		return;

	for(S32 i = 0, countA = multiView.Size(); i < countA; ++i)
	{
		const SceneGraph::C_View* pView = multiView.GetView(i);
		
		if (pView)
		{
			Gather(*pView);
		}
	}
}

void C_CrowdWorld::AllocateBlocks(const U32 numBlocks)
{
	if (!numBlocks)
		return;

	DestroyBlocks();

	m_NumBlocks = numBlocks;

	m_Blocks = new C_CrowdBlock[m_NumBlocks];

	const Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>& blockRefs(m_pResources->GetCrowdData()->GetCrowdBlockRefs());
	const Btl::Vector<Btl::Pair<S16, S16> >& routeAndInstanceCounts(m_pResources->GetCrowdData()->GetRouteBlockAndInstanceCounts());

	S32 blockOffset = 0;
	S32 iBlock = 0;

	for (S32 route = 0, routeCount = routeAndInstanceCounts.Size(); route < routeCount; ++route)
	{
		if (m_RouteMask & (1 << route))
		{
			for (S32 i = 0, count = routeAndInstanceCounts[route].first; i < count; ++i)
			{
				BIZ_ASSERT((blockOffset + i) < blockRefs.Size());
				const C_CrowdBlockRef& blockRef = blockRefs[blockOffset + i];
				m_Blocks[iBlock++].Create(blockOffset + i, blockRef.GetBlockType(), blockRef.GetNumInstances());
			}
		}

		blockOffset += routeAndInstanceCounts[route].first;
	}

	m_RouteBlockOffsets = new U32[routeAndInstanceCounts.Size()];

	for (S32 route = 0, routeCount = routeAndInstanceCounts.Size(); route < routeCount; ++route)
	{
		blockOffset = 0;
		for (S32 i = 0, count = route; i < count; ++i)
		{
			if (!(m_RouteMask & (1 << i)))
				blockOffset += routeAndInstanceCounts[i].first;
		}
		m_RouteBlockOffsets[route] = static_cast<U32>(blockOffset);
	}

	BIZ_ASSERT(iBlock == m_NumBlocks);
}

void C_CrowdWorld::DestroyBlocks()
{
	if (m_Blocks)
	{
		for (U32 i = 0, count = m_NumBlocks; i < count; ++i)
		{
			m_Blocks[i].Destroy();
		}
	}

	delete[] m_Blocks;
	m_Blocks = NULL;
	m_NumBlocks = 0;

	delete[] m_RouteBlockOffsets;
	m_RouteBlockOffsets = NULL;
}

void C_CrowdWorld::CacheVisibleBlocks(const Btl::Vector<C_Event, Btl::VectorParameters_Static<CROWD_MAX_EVENTS> >& activeEvents, const U32 frameNumber)
{
	if (!m_pResources || !m_pResources->GetCrowdData() || !m_pResources->GetCrowdData()->GetBlockGrid())
		return;

	{
		const C_CrowdVisibleBlockData& visibleBlockData = m_VisibleBlockData[m_VisibleBlockDataIndex];

		for (U32 i = 0, count = visibleBlockData.GetNumVisibleBlocks(); i < count; ++i)
		{
			const C_CrowdVisibleBlock& visibleBlock = visibleBlockData.GetVisibleBlock(i);

			S32 routeBlockIndex = (visibleBlock.GetBlockRefIndex() - m_RouteBlockOffsets[m_pResources->GetCrowdData()->GetCrowdBlockRefs()[visibleBlock.GetBlockRefIndex()].GetRoute() + 1]);

			BIZ_ASSERT(static_cast<U32>(routeBlockIndex) < m_NumBlocks);

			m_pBlockCache->InsertVisibleBlock(m_Blocks, routeBlockIndex, visibleBlock.GetPriority());
			m_Blocks[routeBlockIndex].IsVisible(true);
			m_Blocks[routeBlockIndex].UpdateFrameNumber(frameNumber);
			
			if (visibleBlock.GetEventIndex() >= 0 && visibleBlock.GetEventIndex() < activeEvents.Size())
			{
				const C_Event& crowdEvent = activeEvents[visibleBlock.GetEventIndex()];

				m_Blocks[routeBlockIndex].HandleEvent(m_pResources, m_pSpectators->GetSpectators(), activeEvents[visibleBlock.GetEventIndex()], GetGlobalMood());
			}
		}
	}

	{
		const C_CrowdVisibleBlockData& visibleBlockData = m_VisibleBlockData[!m_VisibleBlockDataIndex];

		for (U32 i = 0, count = visibleBlockData.GetNumVisibleBlocks(); i < count; ++i)
		{
			const C_CrowdVisibleBlock& visibleBlock = visibleBlockData.GetVisibleBlock(i);

			S32 routeBlockIndex = (visibleBlock.GetBlockRefIndex() - m_RouteBlockOffsets[m_pResources->GetCrowdData()->GetCrowdBlockRefs()[visibleBlock.GetBlockRefIndex()].GetRoute() + 1]);

			if (!m_Blocks[routeBlockIndex].IsVisible())
			{
				if (frameNumber - m_Blocks[routeBlockIndex].GetFrameNumber() < static_cast<U32>(g_CrowdBlockFrameThreshold.Value()))
				{
					m_pBlockCache->InsertVisibleBlock(m_Blocks, routeBlockIndex, CROWD_BLOCK_CACHE_BUCKET_COUNT);
					m_VisibleBlockData[m_VisibleBlockDataIndex].AddVisibleBlock(C_CrowdVisibleBlock(visibleBlock.GetBlockRefIndex(), (CROWD_BLOCK_CACHE_BUCKET_COUNT + 1), -1));
				}
				else
				{
					if (m_Blocks[routeBlockIndex].HasHighDetailSpectators()/* && !m_Blocks[routeBlockIndex].IsVisible()*/)
					{
						m_Blocks[routeBlockIndex].ForceDetailLevel(m_pSpectators->GetSpectators(), E_DETAIL_LEVEL_LOW, m_pResources->GetUniqueAnimationPool(), m_pResources->GetSharedAnimationPool());
					}
				}
			}
		}
	}
}

}; // namespace AnimatedCrowd
