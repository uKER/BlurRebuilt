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

#include "Display/SPU/GeomCullingDefines.h"
#include "Display/SPU/GeomCullingParams.h"
#include "Display/SPU/Edge/Geom/EdgeGeom.h"



namespace Display
{

void GeomCulling(void* pVoidParams)
{
	//U32 time0 = spu_read_decrementer();

	static F32 * pPositionTable = (F32 *) memalign(128, GEOM_CULLING_BUFFER_SIZE + 8 * 1024);
	static F32 * pVertices = (F32 *) memalign(128, GEOM_CULLING_BUFFER_SIZE + 8 * 1024);
	
	Spu::AutoPointer<SpuGeomCullingParams> pParams = (SpuGeomCullingParams *) pVoidParams;

	EdgeGeomViewportInfo* pViewportInfo			= &pParams->m_ViewportInfo;
	EdgeGeomLocalToWorldMatrix* pLocalToWorld	= &pParams->m_LocalToWorld;
	EdgeGeomSpuConfigInfo* pSpuConfigInfo		= &pParams->m_SpuConfigInfo;
	uint32_t holeEa								= pParams->m_HoleEa;
	uint32_t outputBufferInfoEa					= pParams->m_OutputBufferInfoEa;

	// make sure we have a multiple of 8 vertices
	pParams->m_SpuConfigInfo.numVertexes = (pParams->m_SpuConfigInfo.numVertexes + 0x07) & ~0x07;

	// pull in the vertices from the specified location
	S32 vertexSize = (pParams->m_VertexStride * pParams->m_SpuConfigInfo.numVertexes + 0xF) & ~0xF; // round to next 16 bytes
	U32 vertexAddress = (U32) pParams->m_VertexAddress;
	U8 * vertexPtr = (U8 *) pVertices;
	while (vertexSize > 16 * 1024)
	{
		cellDmaGet(vertexPtr, vertexAddress, 16 * 1024, 0, 0, 0); 
		vertexSize -= 16 * 1024;
		vertexAddress += 16 * 1024;
		vertexPtr += 16 * 1024;
	}
	cellDmaGet(vertexPtr, vertexAddress, vertexSize, 0, 0, 0);

	// wait for vertex data to come through
	cellDmaWaitTagStatusAll(1 << 0);

	// build a position table
	F32 * pVertex = pVertices;
	F32 * pPosition = pPositionTable;
	for (U32 i = 0; i < pParams->m_SpuConfigInfo.numVertexes; ++i)
	{
		pPosition[0] = pVertex[0];
		pPosition[1] = pVertex[1];
		pPosition[2] = pVertex[2];
		pPosition[3] = 1.f;

		pVertex += pParams->m_VertexStride >> 2;
		pPosition += 4;
	}

	// set the index buffer pointer to where the vertex buffer was (don't need that anymore)
	U16 * pIndices = (U16 *) pVertices;

	// pull in the indices from the specified location
	S32 indexSize = (2 * pParams->m_SpuConfigInfo.numIndexes + 0xF) & ~0xF; // round to next 16 bytes
	U32 indexAddress = (U32) pParams->m_IndexAddress;
	U8 * indexPtr = (U8 *) pIndices;
	while (indexSize > 16 * 1024)
	{
		cellDmaGet(indexPtr, indexAddress, 16 * 1024, 0, 0, 0); 
		indexSize -= 16 * 1024;
		indexAddress += 16 * 1024;
		indexPtr += 16 * 1024;
	}
	cellDmaGet(indexPtr, indexAddress, indexSize, 0, 0, 0);

	// initialise Edge
	pSpuConfigInfo->flagsAndUniformTableCount = EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH;
	EdgeGeomSpuContext spuContext;
	edgeGeomInitialize(&spuContext, pSpuConfigInfo, 0, 1, pViewportInfo, pLocalToWorld, 0);

	// set data
	spuContext.positionTable = (void *) pPositionTable;
	spuContext.vertexesLs = (uint32_t) pPositionTable;
	spuContext.indexesLs = (uint32_t) pIndices;

	// wait for index data to come through
	cellDmaWaitTagStatusAll(1 << 0);

	// cull triangles
	U32 numVisibleIdxs = edgeGeomCullTriangles(&spuContext, EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM);
	if (numVisibleIdxs == 0)
	{
		CellGcmContextData ctx;
		edgeGeomBeginCommandBufferHole(&spuContext, &ctx, holeEa, 0, 0);
		edgeGeomEndCommandBufferHole(&spuContext, &ctx, holeEa, 0, 0);
		return;
	}

	// write the indices to the output buffer
	EdgeGeomAllocationInfo info;
	uint32_t allocSize = edgeGeomCalculateDefaultOutputSize(&spuContext, numVisibleIdxs);
	if(!edgeGeomAllocateOutputSpace(&spuContext, outputBufferInfoEa, allocSize, &info, cellSpursGetCurrentSpuId()))
	{
		CellGcmContextData ctx;
		edgeGeomBeginCommandBufferHole(&spuContext, &ctx, holeEa, 0, 0);
		edgeGeomEndCommandBufferHole(&spuContext, &ctx, holeEa, 0, 0);
		return;
	}

	EdgeGeomLocation idx;
	edgeGeomOutputIndexes(&spuContext, numVisibleIdxs, &info, &idx);

	// allocate command buffer space
	edgeGeomSetFreePtr(&spuContext, pPositionTable); // use this now free position table memory for the command buffer

	// write data
	CellGcmContextData ctx;
	edgeGeomBeginCommandBufferHole(&spuContext, &ctx, holeEa, &info, 1);
	
	cellGcmSetDrawIndexArrayUnsafeInline(&ctx, CELL_GCM_PRIMITIVE_TRIANGLES, numVisibleIdxs, CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, idx.location, idx.offset);
	
	edgeGeomEndCommandBufferHole(&spuContext, &ctx, holeEa, &info, 1);

	//U32 time1 = spu_read_decrementer();

	//spu_printf("Visible indices: %d, Total indices: %d\n", numVisibleIdxs, pSpuConfigInfo->numIndexes);
	//spu_printf("%f\n", (time0 - time1) / 80000.0f);
}

}; // namespace Display

void cellSpursMain(qword argTask, U64 argTaskSet)
{
	//DebugBreak();
	//spu_printf("Running GeomCulling...\n");
	Spu::Scheduler::WorkerThread("GeomCulling", argTask, argTaskSet, Display::GeomCulling);
}
