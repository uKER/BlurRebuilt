#include "stdafx.h"
#include "Deferred/Prepass/SPU/JobQueue.h"
#include "System/Scheduler/JobQueue.h"

namespace Deferred
{

void LightingPrepassMaster(Scheduler::Task* pTask)
{
#if defined(PLATFORM_SPU)
	PERF_EVENT("LightingPrepassMaster");

	// fetch things onto SPU
	LightingData::MasterInput* pInput = (LightingData::MasterInput*)Spu::FetchAlloc(pTask->Input, sizeof(LightingData::MasterInput));

	// wait on RSX
	U32 fenceValue = pInput->fenceValue - 1;
	while((fenceValue - pInput->fenceValue) >> 31)
	{
		U8		pFenceArray[16] ALIGN16;
		U32		iFenceAlignment = pInput->fenceAddress & 0xF;
		U32*	pFenceAddress	= (U32*)&pFenceArray[iFenceAlignment];
		cellDmaSmallGet(pFenceAddress, pInput->fenceAddress, 4, 0, 0, 0);
		cellDmaWaitTagStatusAll(1 << 0);
		fenceValue = *pFenceAddress;
	}
#endif
}

}; // namespace Deferred

JQ_DECLARE_SPU_MODULE(LightingPrepassMasterModule, Deferred::LightingPrepassMaster, lightingprepassmaster, "deferred.scheduler");
