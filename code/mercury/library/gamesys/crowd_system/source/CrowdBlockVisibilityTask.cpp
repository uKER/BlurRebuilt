#include "CrowdSystemPch.h"

#include <crowd_system/CrowdBlockVisibilityTask.h>
#include <crowd_system/CrowdBlockVisibilityModule.h>

#include <Display/View/ViewFrustum.h>
#include <Display/View/ViewSettings.h>
#include <SceneGraph/ViewData.h>

#include <system/Scheduler.h>
#include <system/Perf.h>

#include <Btl/Containers/Radix.h>

namespace AnimatedCrowd
{

C_CrowdBlockVisibilityTask::C_CrowdBlockVisibilityTask()
	: m_PotentiallyVisibleBlocks(NULL),
	m_NumPotentiallyVisibleBlocks(0),
	m_Views(NULL),
	m_NumViews(0),
	m_ActiveEvents(NULL),
	m_NumActiveEvents(0),
	m_VisibleBlockData(NULL)
{
}

#ifndef PLATFORM_SPU
bool C_CrowdBlockVisibilityTask::Schedule(C_CrowdBlockVisibilityTask* pTask, S32 group, S32 policy)
{
	if (Scheduler::Active())
	{
		Scheduler::AddTask(
			CrowdBlockVisibilityModule,
			0, // input ptr
			0, // output ptr
			group,	// group ID
			policy, // policy
			&pTask,
			sizeof(pTask)
		);

		return true;
	}
	
	return false;
}
#endif

bool C_CrowdBlockVisibilityTask::Run(C_CrowdBlockVisibilityTask* pTask)
{
	if (!pTask)
		return true;

	if (!pTask->m_NumViews || !pTask->m_NumPotentiallyVisibleBlocks)
		return true;

#ifdef PLATFORM_SPU
	//DebugBreak();
#endif

	const SceneGraph::ViewData*	views								= NULL;
	const C_CrowdPotentiallyVisibleBlock* potentiallyVisibleBlocks	= NULL;
	C_CrowdVisibleBlockData* pVisibleBlockData						= NULL;
	const C_Event* activeEvents										= NULL;
	const CrowdDelimiter* sortedBlocksXAxis							= NULL;
	const CrowdDelimiter* sortedBlocksZAxis							= NULL;

	{
		// Fetch views
		views = (const SceneGraph::ViewData*)Spu::FetchAlloc(pTask->m_Views, sizeof(SceneGraph::ViewData) * pTask->m_NumViews);
		
		// Fetch potentially visible blocks
		potentiallyVisibleBlocks = (const C_CrowdPotentiallyVisibleBlock*)Spu::FetchAlloc(pTask->m_PotentiallyVisibleBlocks, sizeof(C_CrowdPotentiallyVisibleBlock) *pTask->m_NumPotentiallyVisibleBlocks);

		// Alloc visible block data
		pVisibleBlockData = (C_CrowdVisibleBlockData*)Spu::FetchAlloc(pTask->m_VisibleBlockData, sizeof(C_CrowdVisibleBlockData));
		pVisibleBlockData->Clear();
		
		// Fetch active events and sorted block delimiters for sweep & prune against events
		if (pTask->m_NumActiveEvents)
		{
			activeEvents = (const C_Event*)Spu::FetchAlloc(pTask->m_ActiveEvents, sizeof(C_Event) * pTask->m_NumActiveEvents);

			sortedBlocksXAxis = (const CrowdDelimiter*)Spu::FetchAlloc(pTask->m_SortedBlocksXAxis, sizeof(CrowdDelimiter) * pTask->m_NumSortedBlocks);
			sortedBlocksZAxis = (const CrowdDelimiter*)Spu::FetchAlloc(pTask->m_SortedBlocksZAxis, sizeof(CrowdDelimiter) * pTask->m_NumSortedBlocks);
		}
	}

	U32* blockEventOverlapsX = NULL;
	U32* blockEventOverlapsZ = NULL;

	if (pTask->m_NumActiveEvents && pTask->m_NumSortedBlocks && pTask->m_NumBlocks)
	{
		blockEventOverlapsX = (U32*)alloca(sizeof(U32) * pTask->m_NumBlocks);
		memset(blockEventOverlapsX, 0, sizeof(U32) * pTask->m_NumBlocks);

		blockEventOverlapsZ = (U32*)alloca(sizeof(U32) * pTask->m_NumBlocks);
		memset(blockEventOverlapsZ, 0, sizeof(U32) * pTask->m_NumBlocks);

		U32 validLocalEventCount = 0;

		Btl::Vector<CrowdDelimiter, Btl::VectorParameters_Static<CROWD_MAX_LOCAL_EVENTS * 2> >	localEventDelimitersX;
		Btl::Vector<CrowdDelimiter, Btl::VectorParameters_Static<CROWD_MAX_LOCAL_EVENTS * 2> >	localEventDelimitersZ;

		for (U32 i = 0, count = pTask->m_NumActiveEvents; i < count; ++i)
		{
			const C_Event& crowdEvent(activeEvents[i]);

			if (!crowdEvent.HasExpired() && crowdEvent.GetAreaType() != C_Event::E_AREA_TYPE_GLOBAL)
			{
				validLocalEventCount++;

				{
					Maths::HVec3 eventMin = crowdEvent.GetPosition() - Maths::HVec3(crowdEvent.GetRadius());

					CrowdDelimiter eventBegin;
					eventBegin.m_Data = i;
					eventBegin.m_BoundaryType = 0;
					
					eventBegin.m_Value = eventMin.X();
					localEventDelimitersX.PushBack(eventBegin);

					eventBegin.m_Value = eventMin.Z();
					localEventDelimitersZ.PushBack(eventBegin);
				}

				{
					Maths::HVec3 eventMax = crowdEvent.GetPosition() + Maths::HVec3(crowdEvent.GetRadius());

					CrowdDelimiter eventEnd;
					eventEnd.m_Data = i;
					eventEnd.m_BoundaryType = 1;
					
					eventEnd.m_Value = eventMax.X();
					localEventDelimitersX.PushBack(eventEnd);

					eventEnd.m_Value = eventMax.Z();
					localEventDelimitersZ.PushBack(eventEnd);
				}
			}
		}

		// X Axis
		if (validLocalEventCount)
		{
			S8* localEventMappingX = (S8*)alloca(CROWD_MAX_EVENTS * 2);
			memset(localEventMappingX, -1, CROWD_MAX_EVENTS * 2);
			S8* localEventMappingZ = (S8*)alloca(CROWD_MAX_EVENTS * 2);
			memset(localEventMappingZ, -1, CROWD_MAX_EVENTS * 2);

			Btl::RadixSorter<Btl::Vector<CrowdDelimiter, Btl::VectorParameters_Static<CROWD_MAX_LOCAL_EVENTS * 2> > > radixSorter;
			radixSorter.Sort(localEventDelimitersX);
			radixSorter.Sort(localEventDelimitersZ);
			
			U32 localEventDelimiterCount = static_cast<U32>(localEventDelimitersX.Size());
			S32 iCurrentX = static_cast<S32>(localEventDelimiterCount) - 1;
			S32 iCurrentZ = static_cast<S32>(localEventDelimiterCount) - 1;

			U16* localEventDelimiterIndexesX = (U16*)alloca(sizeof(U16) * localEventDelimiterCount);
			memset(localEventDelimiterIndexesX, 0, sizeof(U16) * localEventDelimiterCount);
			U16* localEventDelimiterIndexesZ = (U16*)alloca(sizeof(U16) * localEventDelimiterCount);
			memset(localEventDelimiterIndexesZ, 0, sizeof(U16) * localEventDelimiterCount);

			for (S32 i = static_cast<S32>(pTask->m_NumSortedBlocks) - 1; i >= 0; --i)
			{
				while (iCurrentX >= 0 && localEventDelimitersX[iCurrentX].m_Value >= sortedBlocksXAxis[i].m_Value)
				{
					localEventMappingX[(localEventDelimitersX[iCurrentX].m_Data * 2) + localEventDelimitersX[iCurrentX].m_BoundaryType] = static_cast<S8>(iCurrentX);

					localEventDelimiterIndexesX[iCurrentX--] = i;
				}

				while (iCurrentZ >= 0 && localEventDelimitersZ[iCurrentZ].m_Value >= sortedBlocksZAxis[i].m_Value)
				{
					localEventMappingZ[(localEventDelimitersZ[iCurrentZ].m_Data * 2) + localEventDelimitersZ[iCurrentZ].m_BoundaryType] = static_cast<S8>(iCurrentZ);

					localEventDelimiterIndexesZ[iCurrentZ--] = i;
				}

				if ((iCurrentX < 0) && (iCurrentZ < 0))
					break;
			}

			while (iCurrentX >= 0)
			{
				localEventMappingX[(localEventDelimitersX[iCurrentX].m_Data * 2) + localEventDelimitersX[iCurrentX].m_BoundaryType] = static_cast<S8>(iCurrentX);
				localEventDelimiterIndexesX[iCurrentX--] = 0;
			}

			while (iCurrentZ >= 0)
			{
				localEventMappingZ[(localEventDelimitersZ[iCurrentZ].m_Data * 2) + localEventDelimitersZ[iCurrentZ].m_BoundaryType] = static_cast<S8>(iCurrentZ);
				localEventDelimiterIndexesZ[iCurrentZ--] = 0;
			}

			for (U32 i = 0, count = CROWD_MAX_EVENTS * 2; i < count; i += 2)
			{
				if (localEventMappingX[i] >= 0) // X Axis
				{
					U32 startIndex = localEventDelimiterIndexesX[localEventMappingX[i]];
					U32 endIndex = localEventDelimiterIndexesX[localEventMappingX[i + 1]];

					for (U32 iBlockDelimiter = startIndex + 1; iBlockDelimiter <= endIndex; ++iBlockDelimiter)
						blockEventOverlapsX[sortedBlocksXAxis[iBlockDelimiter].m_Data] |= (1 << localEventDelimitersX[localEventMappingX[i]].m_Data);
				}

				if (localEventMappingZ[i] >= 0) // Z Axis
				{
					U32 startIndex = localEventDelimiterIndexesZ[localEventMappingZ[i]];
					U32 endIndex = localEventDelimiterIndexesZ[localEventMappingZ[i + 1]];

					for (U32 iBlockDelimiter = startIndex + 1; iBlockDelimiter <= endIndex; ++iBlockDelimiter)
						blockEventOverlapsZ[sortedBlocksZAxis[iBlockDelimiter].m_Data] |= (1 << localEventDelimitersZ[localEventMappingZ[i]].m_Data);
				}
			}
		}
	}

	U32 highestPriority = 0;

	{
		for (U32 iEvent = 0, eventCount = pTask->m_NumActiveEvents; iEvent < eventCount; ++iEvent)
			highestPriority = Maths::Max<U32>(highestPriority, activeEvents[iEvent].GetPriority());
	}

	{
		for (U32 iBlock = 0, blockCount = pTask->m_NumPotentiallyVisibleBlocks; iBlock < blockCount; ++iBlock)
		{
			if (pVisibleBlockData->GetNumVisibleBlocks() >= CROWD_MAX_VISIBLE_BLOCKS)
				break;

			const C_CrowdPotentiallyVisibleBlock& pvb(potentiallyVisibleBlocks[iBlock]);
			Maths::C_AxisBox axisBox(pvb.GetMin(), pvb.GetMax());
			const Maths::HVec3 centre = axisBox.Centre();

			U32 priority = CROWD_BLOCK_CACHE_BUCKET_COUNT + 1;

			for (U32 iView = 0, viewCount = pTask->m_NumViews; iView < viewCount; ++iView)
			{
				if (views[iView].viewFrustum.CullAxisBox(axisBox))
				{
					F32 dist = Maths::Length<Maths::HVec3>(views[iView].viewSettings3D.GetCameraPosition() - centre);
					U32 viewPriority = Maths::Clamp<U32>((U32)(((dist * views[iView].instanceDrawDistanceScaleRcp) / CROWD_GRID_CELL_RANGE_OF_INFLUENCE) * CROWD_BLOCK_CACHE_BUCKET_COUNT), 0, CROWD_BLOCK_CACHE_BUCKET_COUNT);
					priority = Maths::Min<U32>(priority, viewPriority);
				}
			}

			if (priority < CROWD_BLOCK_CACHE_BUCKET_COUNT)
			{
				if (pVisibleBlockData->GetNumVisibleBlocks() < CROWD_MAX_VISIBLE_BLOCKS)
				{
					C_CrowdVisibleBlock visibleBlock(pvb.GetBlockIndex(), priority, -1);

					if (pTask->m_NumActiveEvents)
					{
						U32 routeBlockIndex = pvb.GetRouteBlockIndex();
						U32 blockEventOverlapX = blockEventOverlapsX[routeBlockIndex];
						U32 blockEventOverlapZ = blockEventOverlapsZ[routeBlockIndex];
						
						U32 eventOverlaps = (blockEventOverlapX & blockEventOverlapZ);
						
						if (eventOverlaps) // eventOverlaps contains bits of events which overlap on both x and z
						{
							for (U32 iEvent = 0, eventCount = pTask->m_NumActiveEvents; iEvent < eventCount; ++iEvent)
							{
								if (eventOverlaps & (1 << iEvent)) // We got one!
								{
									const C_Event& crowdEvent = activeEvents[iEvent];

									if (crowdEvent.HasExpired())
										continue;

									// Need to take nearest event
									if (visibleBlock.GetEventIndex() >= 0)
									{
										if ((crowdEvent.GetPriority() > activeEvents[visibleBlock.GetEventIndex()].GetPriority()) &&
											(crowdEvent.AffectsBlock(axisBox) != C_Event::E_AFFECT_RESULT_NONE))
										{
											visibleBlock.SetEventIndex(iEvent);
											if (crowdEvent.GetPriority() == highestPriority)
												break;
										}
									}
									else
									{
										if (crowdEvent.AffectsBlock(axisBox) != C_Event::E_AFFECT_RESULT_NONE)
										{
											visibleBlock.SetEventIndex(iEvent);
											if (crowdEvent.GetPriority() == highestPriority)
												break;
										}
									}
								}
							}
						}
					}

					pVisibleBlockData->AddVisibleBlock(visibleBlock);
				}
			}
		}
	}

	// Spu::Store visible block data back into main memory using m_VisibleBlockData ptr
#ifdef PLATFORM_SPU
	{
		Spu::Store(pTask->m_VisibleBlockData, pVisibleBlockData, sizeof(C_CrowdVisibleBlockData));
	}
#endif

	return true;
}

}; // namespace AnimatedCrowd
