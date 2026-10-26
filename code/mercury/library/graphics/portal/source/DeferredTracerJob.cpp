#include <stdafx.h>
#include <Portal/DeferredTracerJob.h>
#include <system/perf.h>

#ifdef PLATFORM_SPU
#include <System/Debug/Log/DebugLogSPU.h>
#include <Core/Maths/hvec/VectorConstGlobals.inl>
#endif // PLATFORM_SPU

namespace Portal
{
	//! SPU job for deferred ray tracing.
	void DeferredTracerJob(Scheduler::Task* pTask)
	{
		PERF_EVENT("DeferredTracerJob");
		Portal::C_DeferredTracerJob* pJobData			= NULL;
		Portal::C_DeferredTracerJob::Input* pJobInput	= NULL;
		Portal::C_DeferredTracerJob::Output* pJobOutput = NULL;
		C_BSPContainer* pBSPContainer					= NULL;

#ifdef PLATFORM_SPU
		// Dma up the job input (including ray vector).
		const uint32_t tagGroup							= 0x0;
		const uint32_t dmaSize							= (sizeof(C_DeferredTracerJob::Input)+0xf) & ~0xf;
		Portal::C_DeferredTracerJob __attribute__ ((aligned(16)) deferredJob;
		pJobData										= &deferredJob;
		BIZ_WARNING(dmaSize % 16 == 0);							// Sanity check: Bad dma size.
		BIZ_WARNING(((U32)pJobData & 0xf) == 0);				// Sanity check: Unaligned ls address.
		BIZ_WARNING(((U32)pTask->Input & 0xf) == 0);			// Sanity check: Unaligned ea.
		cellDmaLargeGet((void*)&deferredJob.m_Input, (uint64_t)pTask->Input, dmaSize, tagGroup, 0, 0);
		cellDmaWaitTagStatusAll(1 << tagGroup);

		// Dma up the BSP container.
		const uint64_t ea = (uint64_t)pJobData->m_Input.m_pTreeEA;
		BIZ_WARNING(ea);
		BIZ_WARNING(ea == (ea & ~0xf));
		uint8_t __attribute__ ((aligned(16)) buffer[(sizeof(C_BSPContainer)+0xf) & ~0xf];
		BIZ_STATIC_ASSERT(sizeof(buffer) % 16 == 0);
		pBSPContainer									= (C_BSPContainer*)&buffer;
		cellDmaGet((void*)pBSPContainer, ea, sizeof(buffer), tagGroup, 0, 0);
		cellDmaWaitTagStatusAll(1<<tagGroup);	// Wait for Dma of BSPContainer object.
		const uint32_t iNodeCount						= pBSPContainer->GetNodeCount();
		const uint32_t iNodeSize						= ((sizeof(C_BSPSplitPlane)*iNodeCount)+0xf) & ~0xf;
		C_BSPSplitPlane* pNodes							= (C_BSPSplitPlane*)Spu::Alloc(iNodeSize);
		BIZ_WARNING(((U32)pNodes & 0xf) == 0);					// Sanity check: Unaligned ls address.
		BIZ_WARNING(pNodes);									// Sanity check: NULL ptr.
		BIZ_WARNING(iNodeSize % 16 == 0);						// Sanity check: dmaSize not a 16byte multiple.
		cellDmaLargeGet((void*)pNodes, (uint64_t)pBSPContainer->GetNodes(), iNodeSize, tagGroup, 0, 0);
		cellDmaWaitTagStatusAll(1 << tagGroup);
		pJobInput										= &pJobData->m_Input;
		pJobOutput										= &pJobData->m_Output;
		pBSPContainer->SetNodePtr(pNodes);
#else
		pJobData										= (Portal::C_DeferredTracerJob*)pTask->ParamData;
		pJobInput										= (Portal::C_DeferredTracerJob::Input*)pTask->Input;
		pJobOutput										= (Portal::C_DeferredTracerJob::Output*)pTask->Output;
		pBSPContainer									= pJobInput->m_pTreeEA;
#endif // PLATFORM_SPU

		// Trace all the rays!
		for(U32 iCurrentRay=0; iCurrentRay<pJobInput->m_iRayCount; ++iCurrentRay)
		{
			pJobOutput->m_vResults[iCurrentRay] = pJobInput->m_vRays[iCurrentRay].Trace(pBSPContainer);
			pJobOutput->m_vResults[iCurrentRay].SetOrigin(pJobInput->m_vRays[iCurrentRay].GetOrigin());
			pJobOutput->m_vResults[iCurrentRay].SetEndPoint(pJobInput->m_vRays[iCurrentRay].GetEndPoint());
		}

		
#ifdef PLATFORM_SPU
		BIZ_WARNING((uint32_t)&pJobOutput->m_vResults == ((uint32_t)&pJobOutput->m_vResults & ~0xf));	// Sanity check: unaligned local store address.
		BIZ_WARNING((uint32_t)pTask->Output == ((uint32_t)pTask->Output & ~0xf));						// Sanity check: unaligned effective address.
		cellDmaLargePut((const void*)&pJobOutput->m_vResults, (uint64_t)pTask->Output, sizeof(C_TraceResults)*C_DeferredTracerJob::MaxBatchSize, tagGroup, 0, 0);
		cellDmaWaitTagStatusAll(1 << tagGroup);
#endif // PLATFORM_SPU
		
		return;
	}
}

JQ_DECLARE_SPU_MODULE(DeferredTracerJobModule, Portal::DeferredTracerJob, deferredtracerjob, "portal.raytrace");
