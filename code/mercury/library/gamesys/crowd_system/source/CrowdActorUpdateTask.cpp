#include "CrowdSystemPch.h"

#include <crowd_system/CrowdActorUpdateTask.h>
#include <crowd_system/CrowdActorUpdateModule.h>
#include <crowd_system/CrowdBlockCache.h>
#include <crowd_system/CrowdRandom.h>
#include <crowd_system/CrowdSharedAnimationPool.h>
#include <crowd_system/CrowdSpectator.h>
#include <crowd_system/CrowdUniqueAnimationPool.h>

#include <system/Scheduler.h>
#include <system/Perf.h>

namespace AnimatedCrowd
{

C_CrowdActorUpdateTask::C_CrowdActorUpdateTask()
{
}

#ifndef PLATFORM_SPU
bool C_CrowdActorUpdateTask::Schedule(C_CrowdActorUpdateTask* pTask, S32 group, S32 policy)
{
	if (Scheduler::Active())
	{
		Scheduler::AddTask(
			CrowdActorUpdateModule,
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

bool C_CrowdActorUpdateTask::Run(C_CrowdActorUpdateTask* pTask)
{
	if (!pTask)
		return true;

	bool valid = (pTask->m_SpectatorUpdateInput.m_pPoseFormat != NULL);
	valid &= (pTask->m_SpectatorUpdateInput.m_pSkeleton != NULL);
	valid &= (pTask->m_SpectatorUpdateInput.m_pAnimationEventEffectJointIndexes != NULL);
	valid &= (pTask->m_SpectatorUpdateInput.m_pActionSelect != NULL);
	valid &= (pTask->m_SpectatorUpdateInput.m_pCycleGroupDataList != NULL);

	valid &= (pTask->m_SpectatorUpdateOutput.m_pBlockCache != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pPaletteMap != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pUniqueAnimationPool != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pSharedAnimationPool != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pEffectSpawnData != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pRandom != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pBlocks != NULL);
	valid &= (pTask->m_SpectatorUpdateOutput.m_pSpectators != NULL);

	if (!valid)
		return true;

	C_CrowdBlockCache* pBlockCacheEa														= pTask->m_SpectatorUpdateOutput.m_pBlockCache;
	U8* pPaletteMapEa																		= pTask->m_SpectatorUpdateOutput.m_pPaletteMap;
	C_CrowdUniqueAnimationPool* pUniqueAnimationPoolEa										= pTask->m_SpectatorUpdateOutput.m_pUniqueAnimationPool;
	C_CrowdSharedAnimationPool* pSharedAnimationPoolEa										= pTask->m_SpectatorUpdateOutput.m_pSharedAnimationPool;
	EffectSpawnData* pEffectSpawnDataEa														= pTask->m_SpectatorUpdateOutput.m_pEffectSpawnData;
	FlagInstanceData* pFlagInstanceDataEa													= pTask->m_SpectatorUpdateOutput.m_pFlagInstanceData;
	AnimatedCrowd::C_Random* pRandomEa														= pTask->m_SpectatorUpdateOutput.m_pRandom;
	C_CrowdBlock* pBlocksEa																	= pTask->m_SpectatorUpdateOutput.m_pBlocks;
	C_CrowdSpectator* pSpectatorsEa															= pTask->m_SpectatorUpdateOutput.m_pSpectators;

	Fetch(pTask->m_SpectatorUpdateInput);
	Fetch(pTask->m_SpectatorUpdateOutput, pTask->m_SpectatorUpdateInput.m_NumBlocks);

	pTask->m_SpectatorUpdateOutput.m_pBlockCache->UpdateCachedActors(pTask->m_SpectatorUpdateInput, pTask->m_SpectatorUpdateOutput);

#ifdef PLATFORM_SPU
	Spu::Store(pBlockCacheEa, pTask->m_SpectatorUpdateOutput.m_pBlockCache, sizeof(C_CrowdBlockCache));
	Spu::Store(pPaletteMapEa, pTask->m_SpectatorUpdateOutput.m_pPaletteMap, 32 * 1024);
	Spu::Store(pUniqueAnimationPoolEa, pTask->m_SpectatorUpdateOutput.m_pUniqueAnimationPool, sizeof(C_CrowdUniqueAnimationPool));
	pTask->m_SpectatorUpdateOutput.m_pSharedAnimationPool->StoreInternal();
	Spu::Store(pSharedAnimationPoolEa, pTask->m_SpectatorUpdateOutput.m_pSharedAnimationPool, sizeof(C_CrowdSharedAnimationPool));
	Spu::Store(pEffectSpawnDataEa, pTask->m_SpectatorUpdateOutput.m_pEffectSpawnData, sizeof(EffectSpawnData));
	Spu::Store(pFlagInstanceDataEa, pTask->m_SpectatorUpdateOutput.m_pFlagInstanceData, sizeof(FlagInstanceData));
	Spu::Store(pRandomEa, pTask->m_SpectatorUpdateOutput.m_pRandom, sizeof(AnimatedCrowd::C_Random)); // TODO: [rthomas] To store or not to store, that is the question.
	Spu::Store(pBlocksEa, pTask->m_SpectatorUpdateOutput.m_pBlocks, sizeof(C_CrowdBlock) * pTask->m_SpectatorUpdateInput.m_NumBlocks);
	Spu::Store(pSpectatorsEa, pTask->m_SpectatorUpdateOutput.m_pSpectators, sizeof(C_CrowdSpectator) * CROWD_MAX_SPECTATORS);
#endif

	return true;
}

void C_CrowdActorUpdateTask::Fetch(C_SpectatorUpdateInput& input)
{
#ifdef PLATFORM_SPU
	input.m_pActionSelect = (const C_CrowdActionSelect*)Spu::FetchAlloc(input.m_pActionSelect, sizeof(C_CrowdActionSelect));

	const Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >* pAnimationEventEffectJointIndexes = input.m_pAnimationEventEffectJointIndexes;
	Spu::FetchAlloc(pAnimationEventEffectJointIndexes);
	input.m_pAnimationEventEffectJointIndexes = pAnimationEventEffectJointIndexes;

	const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >* pCycleGroupDataList = input.m_pCycleGroupDataList;
	Spu::FetchAlloc(pCycleGroupDataList);
	input.m_pCycleGroupDataList = pCycleGroupDataList;
#endif // PLATFORM_SPU
}

void C_CrowdActorUpdateTask::Fetch(C_SpectatorUpdateOutput& output, const U32 numBlocks)
{
#ifdef PLATFORM_SPU
	output.m_pBlockCache = (C_CrowdBlockCache*)Spu::FetchAlloc(output.m_pBlockCache, sizeof(C_CrowdBlockCache));
	output.m_pPaletteMap = (U8*)Spu::FetchAlloc(output.m_pPaletteMap, 32 * 1024);
	output.m_pUniqueAnimationPool = (C_CrowdUniqueAnimationPool*)Spu::FetchAlloc(output.m_pUniqueAnimationPool, sizeof(C_CrowdUniqueAnimationPool));
	output.m_pSharedAnimationPool = (C_CrowdSharedAnimationPool*)Spu::FetchAlloc(output.m_pSharedAnimationPool, sizeof(C_CrowdSharedAnimationPool));
	output.m_pSharedAnimationPool->FetchInternal();
	output.m_pEffectSpawnData = (EffectSpawnData*)Spu::FetchAlloc(output.m_pEffectSpawnData, sizeof(EffectSpawnData));
	output.m_pFlagInstanceData = (FlagInstanceData*)Spu::FetchAlloc(output.m_pFlagInstanceData, sizeof(FlagInstanceData));
	output.m_pRandom = (AnimatedCrowd::C_Random*)Spu::FetchAlloc(output.m_pRandom, sizeof(AnimatedCrowd::C_Random));
	Spu::FetchAllocArray(output.m_pBlocks, numBlocks);
	Spu::FetchAllocArray(output.m_pSpectators, CROWD_MAX_SPECTATORS);
#endif
}

}; // namespace AnimatedCrowd
