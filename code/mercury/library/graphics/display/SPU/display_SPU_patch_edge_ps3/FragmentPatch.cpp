#include <stdint.h>
#include <stdio.h>
#include <spu_intrinsics.h>
#include <sys/spu_thread.h>
#include <cell/spurs/types.h>
#include <cell/spurs/task.h>
#include <spu_printf.h>
#include <cell/dma.h>
#include <cell/gcm_spu.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Containers.h>
#include <Core/Threads.h>

#include <System/debuglog.h>
#include <System/debug/variables.h>
#include <System/SPU/Queue.h>
#include <System/SPU/Scheduler_SPU.h>
#include <System/SPU/AutoPointer.h>

#include "Display/SPU/FPEdgeDefines.h"
#include "Display/SPU/FPEdgeParams.h"
#include "Display/SPU/Edge/Geom/EdgeGeom.h"



namespace Display
{

void FragmentPatchEdge(void* pVoidParams)
{
	
	Spu::AutoPointer<SpuFPEdgeParams> pParams = (SpuFPEdgeParams *) pVoidParams;

	U8 scratchBuffer[256];
	U8 commandBuffer[(pParams->m_CommandBufferHoleSize << 4) + 0x80];

	EdgeGeomSpuContext ctx;
	ctx.inputDmaTag = 1;
	ctx.outputDmaTag = 2;
	ctx.spuConfigInfo.commandBufferHoleSize = pParams->m_CommandBufferHoleSize;
	ctx.vertexesLs = (U32) scratchBuffer;
	ctx.freePtr = commandBuffer;

	// bring in the fragment program
	U8 fragmentProgram[pParams->m_FragmentProgramSize] __attribute__((__aligned__(16)));
	U32 fragmentProgramAddress = pParams->m_FragmentProgramEa;
	U32 fragmentProgramSize = (pParams->m_FragmentProgramSize + 0xF) & ~0xF;

	cellDmaGet(fragmentProgram, fragmentProgramAddress, fragmentProgramSize, 0, 0, 0);

	cellDmaWaitTagStatusAll(1 << 0);

	// patch
	U32 * pOffset = pParams->m_Offsets;
	for (U32 paramNo = 0; paramNo < pParams->m_NumberToPatch; ++paramNo)
	{
		qword patch = si_roti(si_lqd(si_from_ptr(pParams->m_Data + 4 * paramNo), 0x0), 16);
		si_stqx(patch, si_from_ptr(fragmentProgram), si_from_uint(*pOffset++));
	}

	// write fragment program to ring buffer
	EdgeGeomAllocationInfo info;
	edgeGeomAllocateOutputSpace(&ctx, pParams->m_OutputBufferInfoEa, fragmentProgramSize, &info, cellSpursGetCurrentSpuId());

	cellDmaPut(fragmentProgram, info.ea, fragmentProgramSize, 0, 0, 0);

	cellDmaWaitTagStatusAll(1 << 0);

	// fill in the command buffer hole
	CellGcmContextData gcmCtx;
	edgeGeomBeginCommandBufferHole(&ctx, &gcmCtx, pParams->m_CommandBufferHoleEa, &info, 1);
	{
		if (edgeGeomIsAllocatedFromRingBuffer(&info))
			cellGcmSetInvalidateTextureCacheUnsafeInline(&gcmCtx, CELL_GCM_INVALIDATE_TEXTURE);
		
		cellGcmSetUpdateFragmentProgramParameterUnsafeInline(&gcmCtx, info.offset);

		cellGcmSetDrawIndexArrayUnsafeInline(&gcmCtx, pParams->m_PrimitiveMode, pParams->m_IndexCount, CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, pParams->m_IndexLocation, pParams->m_IndexOffset);
	}
	edgeGeomEndCommandBufferHole(&ctx, &gcmCtx, pParams->m_CommandBufferHoleEa, &info, 1);

}

}; // namespace Display

void cellSpursMain(qword argTask, U64 argTaskSet)
{
	DebugBreak();
	Spu::Scheduler::WorkerThread("FragmentPatchEdge", argTask, argTaskSet, Display::FragmentPatchEdge);
}
