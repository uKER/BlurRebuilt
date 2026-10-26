#include <stdafx.h>
#include <SceneGraph/RenderingData/PortalCullNodeClassify.h>
#include <SceneGraph/ViewData.h>
#include <SceneGraph/Enums.h>
#include <system/perf.h>
#include <Portal/SystemContainmentQuery.h>

#ifdef PLATFORM_SPU
#include <System/Debug/Log/DebugLogSPU.h>
#include <../../Graphics/portal/source/SystemContainmentQuery.cpp>
#include <../source/maths/hvec/VectorConst.cpp>
#else
#include <Portal/System.h>	// Only on PPU.
#endif // PLATFORM_SPU


// Some random gibberish.
template <typename _base>
class mask_gen
{
public:
	static const U32 value = 1<<((sizeof(_base)<<3)-1);
};
template <typename _base, typename _t>
const _base s_extend(_t x)
{
	return (x^mask_gen<_t>::value)-x;
}

namespace SceneGraph
{
	/// Task to classify spatial probes w.r.t to the portal room collection.
	void PortalCullNodeClassify(Scheduler::Task* pTask)
	{
		const Portal::C_BSPContainer*	pTreeEA		= NULL;
		PortalCullNodeClassifyInput*	pInput		= NULL;
		PortalCullNodeClassifyData*		pData		= NULL;

		PERF_EVENT("PortalCullNodeClassifyTask");

#ifdef PLATFORM_SPU

		// Dma up the input structure.
		//snPause();
		pInput = (PortalCullNodeClassifyInput*)pTask->Input;
		PortalCullNodeClassifyData		dataObject;
		const uint32_t					sizeInBytes = (sizeof(PortalCullNodeClassifyData::Input)+0xf) & ~0xf;
		const uint32_t					tagGroup = 0x0;
		BIZ_WARNING(sizeInBytes%16==0);															// Size should be a 16byte multiple.
		BIZ_WARNING((uint32_t)pInput==((uint32_t)pInput & ~0xf));								// Input effective address should be aligned to 16byte boundary!
		BIZ_WARNING(sizeInBytes < (16*1024));													// Must be less than 16KiB.
		cellDmaGet((void*)&dataObject.m_Input, (uint64_t)pInput, sizeInBytes, tagGroup, 0, 0);
		cellDmaWaitTagStatusAll(1 << tagGroup);
		pInput->pData = &dataObject;	// Patch the input structure pointer.
		if(dataObject.m_Input.m_iProbeCount == 0)
			return;
		BIZ_WARNING(dataObject.m_Input.m_iProbeCount <= PortalCullNodeClassifyData::MaxBatchSize);
		pData = &dataObject;

#else
		
		// Setup the pointer from the task input.
		pInput = (PortalCullNodeClassifyInput*)pTask->ParamData;
		pData  = pInput->pData;

#endif // PLATFORM_SPU

		// TODO: [Stove] Change this in order to support multiple portal system objects -> placement new into query space.
		pTreeEA = pInput->pData->m_Input.m_vTreeEAs[0];
		BIZ_WARNING(pTreeEA);
		const Portal::C_SystemContainmentQuery	containmentQuery(pTreeEA);

		for(U32 iCurrentProbe=0; iCurrentProbe<pData->m_Input.m_iProbeCount; ++iCurrentProbe)
		{
			// TODO: [Stove] Change the lines below in order to support multiple portal system objects.
			const U32 iLastIndex	= (U32)((S16)pData->m_Input.m_vLastRoom[iCurrentProbe]);
			const U32 iPortalIndex	= pData->m_Input.m_vLastSystem[iCurrentProbe];
			const U32 iRoomIndex	= containmentQuery.Classify(pData->m_Input.m_vSpatialProbes[iCurrentProbe], iLastIndex);	// This is the expensive call :-(
			pData->m_Output.m_vRoomIndices[iCurrentProbe] = iRoomIndex;
			pData->m_Output.m_vPortalIndices[iCurrentProbe] = iPortalIndex;
		}


#ifdef PLATFORM_SPU
		
		BIZ_STATIC_ASSERT((sizeof(U16) * PortalCullNodeClassifyData::MaxBatchSize)%16 == 0);	// Bad batch size!!
		const uint64_t roomIndicesEA = (uint64_t)dataObject.m_Input.m_pOutRoomVec;
		const uint64_t portalIndicesEA = (uint64_t)dataObject.m_Input.m_pOutPortalVec;
		BIZ_WARNING(roomIndicesEA ==(roomIndicesEA  & ~0xf));		// Unaligned target effective address.
		BIZ_WARNING(portalIndicesEA ==(portalIndicesEA & ~0xf));	// Unaligned target effective address.
		const uint32_t dmaSize = sizeof(U16) * PortalCullNodeClassifyData::MaxBatchSize;
		BIZ_WARNING(dmaSize % 16 == 0);		// Sanity check: dma not even 16byte multiple.
		BIZ_WARNING(dmaSize < 16 * 1024);	// Sanity check: dma too large.
		BIZ_WARNING(dmaSize > 16);			// Sanity check: dma too small.
		cellDmaPut((const void*)&dataObject.m_Output.m_vRoomIndices[0], roomIndicesEA, dmaSize, tagGroup, 0, 0);
		cellDmaPut((const void*)&dataObject.m_Output.m_vPortalIndices[0], portalIndicesEA, dmaSize, tagGroup, 0, 0);
		cellDmaWaitTagStatusAll(1 << tagGroup);

#endif // PLATFORM_SPU
		return;
	}
}

JQ_DECLARE_SPU_MODULE(PortalCullNodeClassifyModule, SceneGraph::PortalCullNodeClassify, scenegraph_portalcullnodeclassify, "scenegraph.scheduler");
