#include <stdint.h>
#include <stdio.h>
#include <spu_intrinsics.h>
#include <sys/spu_thread.h>
#include <cell/spurs/types.h>
#include <cell/spurs/task.h>
#include <spu_printf.h>
#include <cell/dma.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Containers.h>
#include <Core/Threads.h>

#include <System/debuglog.h>
#include <System/debug/variables.h>
#include <System/SPU/Queue.h>
#include <System/SPU/Scheduler.h>
#include <System/SPU/AutoPointer.h>

#include "Display/SPU/GeomCullingParams.h"
#include "Display/SPU/Edge/Geom/EdgeGeom.h"

namespace Display
{

void GeomCulling(void* pVoidParams)
{
	Spu::AutoPointer<SpuGeomCullingParams> pParams = (SpuGeomCullingParams*)pVoidParams;

#if 1
#else
	void* pScratchBuffer						= &pParams->m_pScratchBuffer;
	uint32_t dmaTag								= pParams->m_DmaTag;
	EdgeGeomViewportInfo* pViewportInfo			= &pParams->m_ViewportInfo;
	EdgeGeomLocalToWorldMatrix* pLocalToWorld	= &pParams->m_LocalToWorld;
	EdgeGeomSpuConfigInfo* pSpuConfigInfo		= &pParams->m_SpuConfigInfo;
	uint32_t holeEa								= pParams->m_HoleEa;
	uint32_t outputBufferIndoEa					= pParams->m_OutputBufferInfoEa;

	EdgeGeomCustomCallbackInfo customCallbackInfo = 
	{
		0,
		0,
		0,
		0,0,0,0,0
	};

	edgeGeomInitialize(
		pSpuConfigInfo,
		pScratchBuffer,
		dmaTag,
		pViewportInfo,
		pLocalToWorld,
		&customCallbackInfo);

	uint32_t numVisibleIdxs = edgeGeomCullTriangles(
		EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM);

	if(numVisibleIdxs == 0)
	{
		CellGcmContextData ctx;
		edgeGeomBeginCommandBufferHole(&ctx, holeEa);
		edgeGeomEndCommandBufferHole(&ctx, holeEa, 0, 0);
		return;
	}

	EdgeGeomAllocationInfo info;
	uint32_t allocSize = edgeGeomCalculateDefaultOutputSize(numVisibleIdxs);

	if(!edgeGeomAllocateOutputSpace(
		outputBufferIndoEa,
		allocSize,
		&info,
		cellSpursGetCurrentSpuId()))
	{
		CellGcmContextData ctx;
		edgeGeomBeginCommandBufferHole(&ctx, holeEa);
		edgeGeomEndCommandBufferHole(&ctx, holeEa, 0, 0);
		return;
	}

	EdgeGeomLocation idx;
	edgeGeomOutputIndexes(
		numVisibleIdxs,
		&info,
		&idx);

	EdgeGeomLocation vtx;
	edgeGeomOutputVertexes(
		&info,
		&vtx);

	CellGcmContextData ctx;
	edgeGeomBeginCommandBufferHole(
		&ctx,
		holeEa);

	edgeGeomSetVertexDataArrays(
		&ctx,
		&vtx);
	
	cellGcmSetDrawIndexArrayUnsafeInline(
		&ctx,
		CELL_GCM_PRIMITIVE_TRIANGLES,
		numVisibleIdxs,
		CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, 
		idx.location,
		idx.offset);

	edgeGeomEndCommandBufferHole(
		&ctx,
		holeEa,
		&info,
		1);
#endif
}

}; // namespace Display

void cellSpursMain(qword argTask, U64 argTaskSet)
{
	DebugBreak();
	Spu::Scheduler::WorkerThread("GeomCulling", argTask, argTaskSet, Display::GeomCulling);
}
