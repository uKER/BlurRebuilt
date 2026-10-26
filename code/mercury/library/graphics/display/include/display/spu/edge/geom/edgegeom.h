/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOM_H
#define EDGEGEOM_H

#include <spu_intrinsics.h>
#include <stdint.h>
#include <stdbool.h>
#include <cell/gcm_spu.h>
#include "edge/geom/edgegeom_structs.h"
#include "edge/geom/edgegeom_decompress.h"
#include "edge/geom/edgegeom_compress.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/*
	EDGE_GEOM_ALLOW_CUSTOM_CODE is false by default to prevent running unsigned code on the SPU.
	If you have linked your custom code block(s) into a .self or a .sprx, you can set this to 1
	to enable your custom code.  
*/
#define EDGE_GEOM_ALLOW_CUSTOM_CODE 0

void edgeGeomInitialize(EdgeGeomSpuContext *ctx, EdgeGeomSpuConfigInfo *spuConfigInfo, void *scratchBuffer, uint32_t dmaTag, 
						EdgeGeomViewportInfo *inViewportInfo = 0,
						EdgeGeomLocalToWorldMatrix *inLocalToWorldMatrix = 0,
						EdgeGeomCustomCallbackInfo *customCallbackInfo = 0);
EdgeGeomSpuConfigInfo *edgeGeomGetSpuConfigInfo(EdgeGeomSpuContext *ctx);
EdgeGeomViewportInfo *edgeGeomGetViewportInfo(EdgeGeomSpuContext *ctx);
EdgeGeomLocalToWorldMatrix *edgeGeomGetLocalToWorldMatrix(EdgeGeomSpuContext *ctx);
qword *edgeGeomGetUniformTable(EdgeGeomSpuContext *ctx, uint32_t index);
qword **edgeGeomGetUniformTables(EdgeGeomSpuContext *ctx);
uint32_t edgeGeomGetUniformTableCount(EdgeGeomSpuContext *ctx);
qword *edgeGeomGetPositionUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetPositionUniformTable(EdgeGeomSpuContext *ctx, qword *table);
qword *edgeGeomGetTransformUniformTable(EdgeGeomSpuContext *ctx);
qword *edgeGeomGetNormalUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetNormalUniformTable(EdgeGeomSpuContext *ctx, qword *table);
qword *edgeGeomGetTangentUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetTangentUniformTable(EdgeGeomSpuContext *ctx, qword *table);
qword *edgeGeomGetBinormalUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetBinormalUniformTable(EdgeGeomSpuContext *ctx, qword *table);
void *edgeGeomGetFreePtr(EdgeGeomSpuContext *ctx);
void edgeGeomSetFreePtr(EdgeGeomSpuContext *ctx, const void *ptr);
bool edgeGeomIsAllocatedFromRingBuffer(EdgeGeomAllocationInfo *info);
uint32_t edgeGeomGetOutputFlavorStride(EdgeGeomSpuContext *ctx, uint32_t flavor);
	void edgeGeomDecompressVertexes(EdgeGeomSpuContext *ctx, const void *vertexesA, const void *fixedOffsetsA, const void *vertexesB = 0, const void *fixedOffsetsB = 0);
void edgeGeomDecompressIndexes(EdgeGeomSpuContext *ctx, const void *indexes);
void edgeGeomProcessBlendShapes(EdgeGeomSpuContext *ctx, uint32_t numShapes, uint32_t shapeInfosEa);	
void edgeGeomNormalizeUniformTable(qword *pUniform, int32_t vertexCount);
void edgeGeomSkinVertexes(EdgeGeomSpuContext *ctx, void *matrices, uint32_t matrixCount, void *indexesAndWeights);
uint32_t edgeGeomCalculateDefaultOutputSize(EdgeGeomSpuContext *ctx, uint32_t numIndexes);
bool edgeGeomAllocateOutputSpace(EdgeGeomSpuContext *ctx, uint32_t outputBufferInfoEa, uint32_t allocSize, EdgeGeomAllocationInfo *outInfo, uint32_t spuId);
void edgeGeomOutputIndexes(EdgeGeomSpuContext *ctx, uint32_t numIndexes, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc);
void edgeGeomOutputVertexes(EdgeGeomSpuContext *ctx, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc);
void edgeGeomBeginCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos);
void edgeGeomSetVertexDataArrays(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, EdgeGeomLocation *vtxLoc);
void edgeGeomEndCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos);

#ifdef EDGE_GEOM_DEBUG
uint32_t edgeGeomValidateBufferOrder(
	const EdgeGeomGetCustomCallbackBlockFunc pCustomOutputCode, 
	const void *pIndexes, const void *pSkinMatrices, const void *pSkinWeights, 
	const EdgeGeomGetCustomCallbackBlockFunc pCustomBlendCode, 
	const void *pVertexesA, const void *pVertexesB, const void *pViewportInfo, 
	const void *pLocalToWorld, const void *pSpuConfigInfo, 	const void *pFixedOffsetsA,
	const void *pFixedOffsetsB,	const EdgeGeomGetCustomCallbackBlockFunc pCustomInputCode);
#else
static inline uint32_t edgeGeomValidateBufferOrder(
	const EdgeGeomGetCustomCallbackBlockFunc, 
	const void *, const void *, const void *, 
	const EdgeGeomGetCustomCallbackBlockFunc,
	const void *, const void *, const void *, 
	const void *, const void *, const void *,
	const void *,
	const EdgeGeomGetCustomCallbackBlockFunc)
{
	return 0;
}
#endif

// In edgegeom_decompress.cpp
	void edgeGeomDecompressVertexesByFlavor(EdgeGeomSpuContext *ctx, const void *vertexesA, const void *vertexesB, const void *fixedOffsetsA, const void *fixedOffsetsB);
void edgeGeomBlendVertexesByFlavor(EdgeGeomSpuContext *ctx, const void *vertexDeltas, const void *fixedOffsets, float alpha);

// In edgegeom_compress.cpp
void *edgeGeomCompressVertexes(EdgeGeomSpuContext *ctx);

// In edgegeom_cull.cpp
uint32_t edgeGeomCullTriangles(EdgeGeomSpuContext *ctx, uint32_t cullingFlavor);

// In edgegeom_transform.cpp
void edgeGeomTransformVertexes(uint32_t numVertexes, void *inVertexes, void *outVertexes, void *matrix);

#ifdef __cplusplus
}
#endif


#endif // EDGEGEOM_H
