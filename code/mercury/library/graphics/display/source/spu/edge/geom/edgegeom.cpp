/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <cell/dma.h>
#include <cell/spurs/common.h>
#include <spu_printf.h>
#include <spu_mfcio.h>

#include "edge/geom/edgegeom.h"

#define DONT_INCLUDE_DECOMPRESS_CPP
#define DONT_INCLUDE_SKIN_CPP

// from edgegeom_masks.cpp
extern const vec_uchar16 s_AAAa;
extern const vec_uchar16 s_AaBb;
extern const vec_uchar16 s_bCaD;
extern const vec_uchar16 s_caBD;
extern const vec_uchar16 s_CcCc;
extern const vec_uchar16 s_cdAB;

const qword s_0A0a =	{0x80, 0x80, 0x80, 0x80,  0x00, 0x01, 0x02, 0x03,  0x80, 0x80, 0x80, 0x80,  0x10, 0x11, 0x12, 0x13};

// In edgegeom_skin*.cpp
#ifndef DONT_INCLUDE_SKIN_CPP
extern "C" void edgeGeomSkinUniformTable(EdgeGeomSpuContext *ctx, vec_float4 *uniform, const vec_uchar16 *indexesAndWeights,
	vec_float4 *matrices, int matrixCount);
extern "C" void edgeGeomSkinVertexesUniformScale(EdgeGeomSpuContext *ctx, vec_float4 *uniforms[4], int uniformCount,
	const vec_uchar16 *indexesAndWeights, vec_float4 *matrices, int matrixCount);
extern "C" void edgeGeomSkinVertexesNonUniformScale(EdgeGeomSpuContext *ctx, vec_float4 *uniforms[4], int uniformCount,
	const vec_uchar16 *indexesAndWeights, vec_float4 *matrices, int matrixCount);
extern "C" void edgeGeomSkinVertexesUnscaled(EdgeGeomSpuContext *ctx, vec_float4 *uniforms[4], int uniformCount,
	const vec_uchar16 *indexesAndWeights, vec_float4 *matrices, int matrixCount);
#endif // DONT_INCLUDE_SKIN_CPP

#ifdef EDGE_GEOM_DEBUG
void edgeGeomValidateProcessingOrder(EdgeGeomSpuContext *ctx, uint32_t currentOperation)
{
	/* 	For now, we're keeping this here, partly because we don't think that there should
		be multiple contexts at once.  If that changes, this should be moved into the context. */
	static uint32_t previousOperation;
	bool isValid = true;
	switch(currentOperation)
	{
	case EDGE_GEOM_OPERATION_INITIALIZE:
		/* previousOperation will be undefined in this case */
		break;
	case EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_SET_CALLBACKS ||
			previousOperation == EDGE_GEOM_OPERATION_INITIALIZE;
		break;
	case EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES:
		isValid = previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES;
		break;
	case EDGE_GEOM_OPERATION_SKIN_VERTEXES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES ||
			previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES;
		break;
	case EDGE_GEOM_OPERATION_DECOMPRESS_INDEXES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_SKIN_VERTEXES ||
			previousOperation == EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES ||
			previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES;
		break;
	case EDGE_GEOM_OPERATION_CULL_TRIANGLES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_INDEXES ||
			previousOperation == EDGE_GEOM_OPERATION_SKIN_VERTEXES ||
			previousOperation == EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES ||
			previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES;
		break;
	case EDGE_GEOM_OPERATION_OUTPUT_INDEXES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_CULL_TRIANGLES ||
			previousOperation == EDGE_GEOM_OPERATION_SKIN_VERTEXES ||
			previousOperation == EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES ||
			previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES ||
			previousOperation == EDGE_GEOM_OPERATION_COMPRESS_VERTEXES;
		break;
	case EDGE_GEOM_OPERATION_COMPRESS_VERTEXES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_CULL_TRIANGLES ||
			previousOperation == EDGE_GEOM_OPERATION_SKIN_VERTEXES ||
			previousOperation == EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES ||
			previousOperation == EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES ||
			previousOperation == EDGE_GEOM_OPERATION_OUTPUT_INDEXES;
		break;
	case EDGE_GEOM_OPERATION_OUTPUT_VERTEXES:
		isValid = 
			previousOperation == EDGE_GEOM_OPERATION_OUTPUT_VERTEXES ||
			(previousOperation == EDGE_GEOM_OPERATION_OUTPUT_INDEXES
				&& (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)) ||
			previousOperation == EDGE_GEOM_OPERATION_COMPRESS_VERTEXES;
		break;
	case EDGE_GEOM_OPERATION_FILL_COMMAND_BUFFER_HOLE:
		isValid = previousOperation == EDGE_GEOM_OPERATION_OUTPUT_VERTEXES ||
			(previousOperation == EDGE_GEOM_OPERATION_CULL_TRIANGLES && ctx->numVisibleIndexes == 0);
		break;
	}

	if(!isValid)
	{
		spu_printf("Edge error: operation %d cannot follow operation %d\n",
			currentOperation, previousOperation);
		si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
	}

	previousOperation = currentOperation;
}

uint32_t edgeGeomValidateBufferOrder(
	const EdgeGeomGetCustomCallbackBlockFunc pCustomOutputCode, 
	const void *pIndexes, 
	const void *pSkinMatrices, 
	const void *pSkinWeights, 
	const EdgeGeomGetCustomCallbackBlockFunc pCustomBlendCode,
	const void *pVertexesA, 
	const void *pVertexesB,	
	const void *pViewportInfo, 
	const void *pLocalToWorld, 
	const void *pSpuConfigInfo, 
	const void *pFixedOffsetsA, 
	const void *pFixedOffsetsB, 
	const EdgeGeomGetCustomCallbackBlockFunc pCustomInputCode)
{
	// cast input pointers to ints, so we can compare them.
	const uint32_t customOutputCode = (uint32_t)pCustomOutputCode;
	const uint32_t indexes          = (uint32_t)pIndexes;
	const uint32_t viewportInfo     = (uint32_t)pViewportInfo;
	const uint32_t localToWorld     = (uint32_t)pLocalToWorld;
	const uint32_t skinMatrices     = (uint32_t)pSkinMatrices;
	const uint32_t skinWeights      = (uint32_t)pSkinWeights;
	const uint32_t vertexesA        = (uint32_t)pVertexesA;
	const uint32_t vertexesB        = (uint32_t)pVertexesB;
	const uint32_t spuConfigInfo    = (uint32_t)pSpuConfigInfo;
	const uint32_t fixedOffsetsA    = (uint32_t)pFixedOffsetsA;
	const uint32_t fixedOffsetsB    = (uint32_t)pFixedOffsetsB;
	const uint32_t customBlendCode  = (uint32_t)pCustomBlendCode;
	const uint32_t customInputCode  = (uint32_t)pCustomInputCode;

	uint32_t errorCode = 0;

	// Remember that 0 is a perfectly valid pointer target on the SPU,
	// so we can't simply test the pointers against 0 to see if
	// they're valid. Instead, we use the SpuConfigInfo fields to
	// determine what data we're expecting.
	EdgeGeomSpuConfigInfo *pSpuInfo = (EdgeGeomSpuConfigInfo*)pSpuConfigInfo;
	bool hasCustomInputCode = (pSpuInfo->inputFlavor == 0xFF);
	bool hasCustomBlendCode = (pSpuInfo->shapeFlavor == 0xFF);
	bool hasCustomOutputCode = (pSpuInfo->outputFlavor == 0xFF);
	bool isSkinning = ((pSpuInfo->indexesFlavorAndSkinningFlavor & 0xF) != EDGE_GEOM_SKIN_NONE);
	bool hasIndexes = (pSpuInfo->numIndexes > 0);
	bool isFastPath = (pSpuInfo->flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH);

	// Custom input code, viewport info, local-to-world matrix,
	// SpuConfigInfo and fixed offsets can be in any order and can
	// technically appear anywhere in the buffer, although it's
	// pointlessly wasteful to put them anywhere except after the
	// vertex buffer.
	if (hasCustomInputCode != 0 && vertexesA > customInputCode)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_CUSTOM_INPUT_CODE;
	if (viewportInfo != 0 && vertexesA > viewportInfo)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_VIEWPORT_INFO;
	if (localToWorld != 0 && vertexesA > localToWorld)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_LOCAL_TO_WORLD;
	if (vertexesA > spuConfigInfo)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_SPU_CONFIG_INFO;
	if (fixedOffsetsA != 0 && vertexesA > fixedOffsetsA)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_FIXED_OFFSETS;
	if (fixedOffsetsB != 0 && vertexesB > fixedOffsetsB)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_FIXED_OFFSETS;

	// Keep track of the ealiest buffer we've found so far.
	uint32_t vertexes = vertexesB ? ((vertexesA < vertexesB) ? vertexesA : vertexesB) : vertexesA;
	uint32_t earliest = vertexes;

	// If we have custom blend code, it must come before the vertexes,
	// but after the skinning tables.
	if (hasCustomBlendCode)
	{
		if (customBlendCode > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_CUSTOM_BLEND_CODE;
		earliest = customBlendCode;
	}

	// If we have skinning matrices or weights, we must have both.  It
	// doesn't matter which comes first, but they must both come
	// before the input vertex stream (or blend code, if present).
	//
	if (isSkinning)
	{
		if (skinMatrices > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_SKINNING_MATRICES;
		if (skinWeights > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_SKINNING_WEIGHTS;
		earliest = (skinMatrices < skinWeights) ? skinMatrices : skinWeights;
	}
	
	// If we're using the fast path, certain buffers shouldn't be
	// present (warning) and certain operations must not be performed
	// (error)
	if (isFastPath)
	{		
		if (isSkinning)
			errorCode |= EDGE_GEOM_VALIDATE_WARNING_FAST_PATH;

		//if (vertexesB != 0)
		//	errorCode |= EDGE_GEOM_VALIDATE_WARNING_FAST_PATH;

		// TEMP: compressed indexes are currently incompatible with
		// the fast path; they are decompressed in-place and blow away
		// whatever comes after the input buffer.
		uint8_t indexesFlavor = (pSpuInfo->indexesFlavorAndSkinningFlavor & 0xF0);
		if (indexesFlavor == EDGE_GEOM_INDEXES_SW_TRIANGLE_LIST_CW ||
			indexesFlavor == EDGE_GEOM_INDEXES_SW_TRIANGLE_LIST_CCW)
		{
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_FAST_PATH;
		}
	}

	// If we have indexes, they must come before the earliest table
	// found above (either the input vertex stream or the earlier of
	// the two skinning tables).
	if (hasIndexes)
	{
		if(indexes > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_INDEXES;
		earliest = indexes;
	}

	// If there's custom output code, it must be the first item in the buffer.
	if (hasCustomOutputCode != 0)
	{
		if (customOutputCode > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_CUSTOM_OUTPUT_CODE;
		earliest = customOutputCode;
	}

	return errorCode;
}
#else
static inline void edgeGeomValidateProcessingOrder(EdgeGeomSpuContext *ctx, uint32_t currentOperation)
{
}
#endif

/**
 *	Copies input data to internal variables.  Sets up pointers in scratch 
 * 	memory.  Sets callbacks.
 *  @param ctx EdgeGeom job context
 * 	@param info configuration info for this job
 * 	@param scratchBuffer pointer to scratch memory area
 * 	@param dmaTag tag for this job to use with DMAs
 * 	@param inViewportInfo pointer to input viewport info structure to be copied to an internal variable
 * 	@param inLocalToWorldMatrix pointer to input local to world matrix structure to be copied to an internal variable
 *  @param customCallBackInfo pointer to custom code and callbacks
 */
void edgeGeomInitialize(
	EdgeGeomSpuContext *ctx,
	EdgeGeomSpuConfigInfo *info, void *scratchBuffer, uint32_t dmaTag,
	EdgeGeomViewportInfo *inViewportInfo /*= 0*/,
	EdgeGeomLocalToWorldMatrix *inLocalToWorldMatrix /*= 0*/,
	EdgeGeomCustomCallbackInfo *customCallbackInfo /*= 0*/)
{
	ctx->freePtr = 0;
	ctx->indexesLs = 0;
	ctx->vertexesLs = 0;
	ctx->spuConfigInfo = *info;
	ctx->outputDmaTag = dmaTag;
	ctx->inputDmaTag = dmaTag;
	ctx->positionTable = 0;
	ctx->normalTable   = 0;
	ctx->tangentTable  = 0;
	ctx->binormalTable = 0;
	uint32_t numTables = (ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF) + 1;
	for(uint32_t i = 0; i < numTables; ++i)
	{
		ctx->uniformTables[i] = (float*)( (qword*)scratchBuffer + i * ctx->spuConfigInfo.numVertexes );
	}
	if(inViewportInfo != 0)
		ctx->viewportInfo = *inViewportInfo;
	if(inLocalToWorldMatrix != 0)
		ctx->localToWorldMatrix = *inLocalToWorldMatrix;

	if (!customCallbackInfo)
	{
		ctx->decompressInputCallback = 0;
		ctx->decompressBlendCallback = 0;
		ctx->outputStrideCallback = 0;
		ctx->compressOutputCallback = 0;
		ctx->setVertexDataArraysCallback = 0;
	}
	else
	{
/*
	EDGE_GEOM_ALLOW_CUSTOM_CODE is false by default to prevent running unsigned code on the SPU.
    If you have linked your custom code block(s) into a .self or a .sprx, you can enable this.
	See the definition in edgegeom.h.
*/
#if EDGE_GEOM_ALLOW_CUSTOM_CODE
		// Extract input callbacks, if present.
		if (ctx->spuConfigInfo.inputFlavor == 0xFF && customCallbackInfo->customInputCode != 0)
		{
			EdgeGeomCustomCallbackBlock *inputCallbacks = (*customCallbackInfo->customInputCode)();
			if (inputCallbacks->numCallbacks != 1)
			{
				spu_printf("Edge error: Incorrect number of callbacks in custom input code (expected 1, found %d)\n",
					inputCallbacks->numCallbacks);
				si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
			}
			ctx->decompressInputCallback = (EdgeGeomDecompressVertexStreamCallback)((uint32_t)inputCallbacks->callbacks[0]
				+ (uint32_t)customCallbackInfo->customInputCode - 16);
		}
		
		// Extract blend callbacks, if present.
		if (ctx->spuConfigInfo.shapeFlavor == 0xFF && customCallbackInfo->customBlendCode != 0)
		{
			EdgeGeomCustomCallbackBlock *blendCallbacks = (*customCallbackInfo->customBlendCode)();
			if (blendCallbacks->numCallbacks != 1)
			{
				spu_printf("Edge error: Incorrect number of callbacks in custom blend code (expected 1, found %d)\n",
					blendCallbacks->numCallbacks);
				si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
			}
			ctx->decompressBlendCallback = (EdgeGeomBlendVertexStreamCallback)((uint32_t)blendCallbacks->callbacks[0]
				+ (uint32_t)customCallbackInfo->customBlendCode - 16);
		}
		
		// Extract output callbacks, if necessary.
		if (ctx->spuConfigInfo.outputFlavor == 0xFF && customCallbackInfo->customOutputCode != 0)
		{
			EdgeGeomCustomCallbackBlock *outputCallbacks = (*customCallbackInfo->customOutputCode)();
			if (outputCallbacks->numCallbacks != 3)
			{
				spu_printf("Edge error: Incorrect number of callbacks in custom output code (expected 3, found %d)\n",
					outputCallbacks->numCallbacks);
				si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
			}
			ctx->compressOutputCallback = (EdgeGeomCompressVertexStreamCallback)((uint32_t)outputCallbacks->callbacks[0]
				+ (uint32_t)customCallbackInfo->customOutputCode - 16);
			ctx->outputStrideCallback = (EdgeGeomGetOutputFlavorStrideCallback)((uint32_t)outputCallbacks->callbacks[1]
				+ (uint32_t)customCallbackInfo->customOutputCode - 16);
			ctx->setVertexDataArraysCallback = (EdgeGeomSetVertexDataArraysCallback)((uint32_t)outputCallbacks->callbacks[2]
				+ (uint32_t)customCallbackInfo->customOutputCode - 16);
		}
#else
		if (ctx->spuConfigInfo.inputFlavor == 0xFF && customCallbackInfo->customInputCode != 0)
		{
			spu_printf("Edge error: Custom input code is disabled by default.\n");
				si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
		}
		
		// Extract blend callbacks, if present.
		if (ctx->spuConfigInfo.shapeFlavor == 0xFF && customCallbackInfo->customBlendCode != 0)
		{
				spu_printf("Edge error: Custom blend code is disabled by default.\n");
				si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
		}
		
		// Extract output callbacks, if necessary.
		if (ctx->spuConfigInfo.outputFlavor == 0xFF && customCallbackInfo->customOutputCode != 0)
		{
				spu_printf("Edge error: Custom output code is disabled by default.");
				si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
		}
#endif
		// If individual function callbacks were provided, they take precedence.
		if (customCallbackInfo->decompressInputCallback != 0)
			ctx->decompressInputCallback = customCallbackInfo->decompressInputCallback;
		if (customCallbackInfo->decompressBlendCallback != 0)
			ctx->decompressBlendCallback = customCallbackInfo->decompressBlendCallback;
		if (customCallbackInfo->outputStrideCallback != 0)
			ctx->outputStrideCallback = customCallbackInfo->outputStrideCallback;
		if (customCallbackInfo->compressOutputCallback != 0)
			ctx->compressOutputCallback = customCallbackInfo->compressOutputCallback;
		if (customCallbackInfo->setVertexDataArraysCallback != 0)
			ctx->setVertexDataArraysCallback = customCallbackInfo->setVertexDataArraysCallback;
	}

    edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_INITIALIZE);
}

qword *edgeGeomGetUniformTable(EdgeGeomSpuContext *ctx, uint32_t index)
{
	return (qword *)(ctx->uniformTables[index]);
}

EdgeGeomSpuConfigInfo *edgeGeomGetSpuConfigInfo(EdgeGeomSpuContext *ctx)
{
	return &(ctx->spuConfigInfo);
}

EdgeGeomViewportInfo *edgeGeomGetViewportInfo(EdgeGeomSpuContext *ctx)
{
	return &ctx->viewportInfo;
}

EdgeGeomLocalToWorldMatrix *edgeGeomGetLocalToWorldMatrix(EdgeGeomSpuContext *ctx)
{
	return &(ctx->localToWorldMatrix);
}

uint32_t edgeGeomGetUniformTableCount(EdgeGeomSpuContext *ctx)
{
	return (ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF) + 1;
}

qword **edgeGeomGetUniformTables(EdgeGeomSpuContext *ctx)
{
	return (qword **)(void*)(ctx->uniformTables);
}

qword *edgeGeomGetPositionUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->positionTable;
}

void edgeGeomSetPositionUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->positionTable = (void*)table;
}

/**
 * 	Get the uniform table containing the screen-space transformed vertices.
 * 	For normal EdgeGeom usage, vertices will be in 
 * 	[X, Y, flags, quantized] format as needed for culling.
 *  @param ctx EdgeGeom job context
 * 	@return uniform table containing transformed vertices.
 */
qword *edgeGeomGetTransformUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword *)(ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF]);
}

qword *edgeGeomGetNormalUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->normalTable;
}

void edgeGeomSetNormalUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->normalTable = (void*)table;
}

qword *edgeGeomGetTangentUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->tangentTable;
}

void edgeGeomSetTangentUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->tangentTable = (void*)table;
}

qword *edgeGeomGetBinormalUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->binormalTable;
}

void edgeGeomSetBinormalUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->binormalTable = (void*)table;
}

/**
 *	Get a pointer to the start of free memory in the IOBuffer
 *  @param ctx EdgeGeom job context
 * 	@return the free pointer
 */
void *edgeGeomGetFreePtr(EdgeGeomSpuContext *ctx)
{
	return ctx->freePtr;
}

/**
 * 	Sets the free pointer to a specified address.  Care should be
 *  taken not to set this below data needed by EdgeGeom.
 *  @param ctx EdgeGeom job context
 *  @param ptr the new free pointer
 */
void edgeGeomSetFreePtr(EdgeGeomSpuContext *ctx, const void *ptr)
{
	ctx->freePtr = (void*)ptr;
}

bool edgeGeomIsAllocatedFromRingBuffer(EdgeGeomAllocationInfo *info)
{
	return (info->rsxLabelEa);
}

/**
 * 	Decompress vertices into uniform tables
 *  @param ctx EdgeGeom job context
 * 	@param vertexesA first input stream
 * 	@param fixedOffsetsA array of offsets for fixed point attributes for the first stream
 * 	@param vertexesB second input stream
 * 	@param fixedOffsetsB array of offsets for fixed point attributes for the second stream
 */
#ifndef DONT_INCLUDE_DECOMPRESS_CPP
void edgeGeomDecompressVertexes(EdgeGeomSpuContext *ctx, const void *vertexesA, const void *fixedOffsetsA, const void *vertexesB /*= 0*/, const void *fixedOffsetsB /*= 0*/)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES);

	edgeGeomDecompressVertexesByFlavor(ctx, vertexesA, vertexesB, fixedOffsetsA, fixedOffsetsB);
}
#endif // DONT_INCLUDE_DECOMPRESS_CPP

/**
 * 	For each blend shape, DMAs blend shape infos and deltas to local store
 * 	and blends vertices.  
 *  @param ctx EdgeGeom job context
 * 	@param numShapes the number of shapes
 * 	@param shapeInfosEa the effective address of the first shape info structure.
 */
#ifndef DONT_INCLUDE_DECOMPRESS_CPP
void edgeGeomProcessBlendShapes(EdgeGeomSpuContext *ctx, uint32_t numShapes, uint32_t shapeInfosEa)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES);

	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return;

	uint32_t freePtr = (uint32_t)ctx->freePtr;

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	
	// TODO: Double buffering the input here would improve performance.
	for(uint32_t i = 0; i < numShapes; ++i)
	{
		spu_idisable();

		// dma in the shape Infos to freePtr
		spu_writech(MFC_LSA, (uint32_t)freePtr);
		spu_writech(MFC_EAL, shapeInfosEa);
		spu_writech(MFC_Size, sizeof(EdgeGeomBlendShapeInfo));
		spu_writech(MFC_TagID, ctx->inputDmaTag);
		spu_writech(MFC_Cmd, MFC_GET_CMD);

		// dma in the info->dmaTag w/ barrier
		spu_writech(MFC_LSA, (uint32_t)freePtr + sizeof(EdgeGeomBlendShapeInfo));
		spu_writech(MFC_EAL, (uint32_t)freePtr);
		spu_writech(MFC_Size, 8);
		spu_writech(MFC_TagID, ctx->inputDmaTag);
		spu_writech(MFC_Cmd, MFC_GETLB_CMD);

		// wait for dmas
		spu_writech(MFC_WrTagMask, 1 << ctx->inputDmaTag);
		spu_writech(MFC_WrTagUpdate, 2);
		spu_readch(MFC_RdTagStat);

		if(interruptsEnabled)
			spu_ienable();

		EdgeGeomBlendShapeInfo *info = (EdgeGeomBlendShapeInfo *)freePtr;
		void *fixedOffsets = (void*)( (uint32_t)freePtr + sizeof(EdgeGeomBlendShapeInfo) );
		void *vertexDeltas = (void*)( (uint32_t)freePtr + sizeof(EdgeGeomBlendShapeInfo)
			+ info->fixedOffsetsSize );

		edgeGeomBlendVertexesByFlavor(ctx, vertexDeltas, fixedOffsets, info->alpha);

		shapeInfosEa += sizeof(EdgeGeomBlendShapeInfo);
	}
}
#endif // DONT_INCLUDE_DECOMPRESS_CPP

#ifndef DONT_INCLUDE_SKIN_CPP
static void edgeGeomPreprocessMatrices(vec_float4 *matrices, uint32_t matrixCount)
{
	/* The skinning matrices must be preprocessed before the vertexes
	 * are skinned.  Each matrix is modified in-place using an
	 * operation that's difficult to describe, but should be clear
	 * from this example:
	 *
	 * Original input matrix (48 bytes / 3 quadwords)      
	 * A B C D
	 * E F G H
	 * I J K L
	 *
	 * Matrix after preprocessing:
	 * A F K D
	 * B G I H
	 * C E J L
	 */

	const qword mask_0X00 = si_fsmbi(0x0F00);
	const qword mask_00X0 = si_fsmbi(0x00F0);
	const qword qs_bCaD = (qword)s_bCaD;
	const qword qs_caBD = (qword)s_caBD;

	const qword pOut1 = si_from_ptr(matrices + (3 * matrixCount));
	const qword pOut2 = si_ai(pOut1, 0x10);
	const qword pOut3 = si_ai(pOut1, 0x20);
	const qword pIn1  = si_ai(pOut1, 0x60);//30
	const qword pIn2  = si_ai(pOut1, 0x70);//40
	const qword pIn3  = si_ai(pOut1, 0x80);//50
	qword offset = si_from_int( - (48 * matrixCount));
	
	qword in1 = si_lqx(pOut1, offset);
	qword in2 = si_lqx(pOut2, offset);
	qword in3 = si_lqx(pOut3, offset);

	qword temp2 = si_selb(in3, in1, mask_0X00);
	qword temp3 = si_selb(in2, in1, mask_00X0);
	qword temp1 = si_selb(in1, in2, mask_0X00);
	qword out2  = si_shufb(in2, temp2, qs_bCaD);
	qword out3  = si_shufb(in3, temp3, qs_caBD);
	qword out1  = si_selb(temp1, in3, mask_00X0);

	offset = si_ai(offset, 0x30);
	in1 = si_lqx(pOut1, offset);
	in2 = si_lqx(pOut2, offset);
	in3 = si_lqx(pOut3, offset);

	qword m1, m2, m3;
	offset = si_from_int( - (48 * matrixCount));

	do
	{
		m1  = in1;
		in1 = si_lqx(pIn1, offset);
		m2  = in2;
		in2 = si_lqx(pIn2, offset);
		m3  = in3;
		in3 = si_lqx(pIn3, offset);

		temp3 = si_selb(m2, m1, mask_00X0);
		si_stqx(out1, pOut1, offset);
		temp2 = si_selb(m3, m1, mask_0X00);
		si_stqx(out2, pOut2, offset);
		temp1 = si_selb(m1, m2, mask_0X00);
		si_stqx(out3, pOut3, offset);

		offset = si_ai(offset, 0x30);
		out2 = si_shufb(m2, temp2, qs_bCaD);
		out1 = si_selb(temp1, m3, mask_00X0);
		out3 = si_shufb(m3, temp3, qs_caBD);
	}
	while(si_to_int(offset) != 0);
}
#endif // DONT_INCLUDE_SKIN_CPP

#ifndef DONT_INCLUDE_SKIN_CPP
void edgeGeomSkinVertexes(EdgeGeomSpuContext *ctx, void *matrices, uint32_t matrixCount, void *indexesAndWeights)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_SKIN_VERTEXES);

	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return;

	if(matrixCount == 0)
		return;
	if((ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor & 0xF) == EDGE_GEOM_SKIN_NONE)
		return;

	edgeGeomPreprocessMatrices((vec_float4*)matrices, matrixCount);

	vec_float4 *skinnableTables[4] = {(vec_float4*)ctx->positionTable, 0, 0, 0};
	uint32_t skinnableTableCount = 1;
	if (ctx->normalTable)
		skinnableTables[skinnableTableCount++] = (vec_float4*)ctx->normalTable;
	if (ctx->tangentTable)
		skinnableTables[skinnableTableCount++] = (vec_float4*)ctx->tangentTable;
	if (ctx->binormalTable)
		skinnableTables[skinnableTableCount++] = (vec_float4*)ctx->binormalTable;

	if(skinnableTableCount == 1)
	{
		edgeGeomSkinUniformTable(ctx, (vec_float4*)ctx->positionTable, (vec_uchar16*)indexesAndWeights,
			(vec_float4*)matrices, matrixCount);
	}
	else
	{
		switch(ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor & 0xF)
		{
		case EDGE_GEOM_SKIN_UNIFORM_SCALING:
			edgeGeomSkinVertexesUniformScale(ctx, skinnableTables, skinnableTableCount,
				(vec_uchar16*)indexesAndWeights, (vec_float4*)matrices, matrixCount);
			break;
		case EDGE_GEOM_SKIN_NON_UNIFORM_SCALING:
			edgeGeomSkinVertexesNonUniformScale(ctx, skinnableTables, skinnableTableCount,
				(vec_uchar16*)indexesAndWeights, (vec_float4*)matrices, matrixCount);
			break;
		case EDGE_GEOM_SKIN_NO_SCALING:
			edgeGeomSkinVertexesUnscaled(ctx, skinnableTables, skinnableTableCount,
				(vec_uchar16*)indexesAndWeights, (vec_float4*)matrices, matrixCount);
			break;
		}
	}

	ctx->freePtr = matrices;
}
#endif // DONT_INCLUDE_SKIN_CPP

void edgeGeomNormalizeUniformTable(qword *pUniform, int32_t vertexCount)
{
	const qword sq_AAAa = (qword)s_AAAa;
	const qword sq_AaBb = (qword)s_AaBb;
	const qword sq_BbAa = si_rotqbyi(sq_AaBb, 8);;
	const qword sq_cdAB = (qword)s_cdAB;
	const qword sq_CcCc = (qword)s_CcCc;
	const qword k1_0f = si_ilhu(0x3f80);
	const qword m_00FF = si_fsmbi(0x00FF);
	const qword sq_BBBb = si_orbi(sq_AAAa, 0x04);
	const qword sq_CCCc = si_orbi(sq_AAAa, 0x08);
	const qword sq_DDDd = si_orbi(sq_AAAa, 0x0C);

	const qword pInVerts1  = si_from_ptr(pUniform + vertexCount + 12);
	const qword pInVerts2  = si_from_ptr(pUniform + vertexCount + 13);
	const qword pInVerts3  = si_from_ptr(pUniform + vertexCount + 14);
	const qword pInVerts4  = si_from_ptr(pUniform + vertexCount + 15);
	const qword pOutVerts1 = si_from_ptr(pUniform + vertexCount + 0);
	const qword pOutVerts2 = si_from_ptr(pUniform + vertexCount + 1);
	const qword pOutVerts3 = si_from_ptr(pUniform + vertexCount + 2);
	const qword pOutVerts4 = si_from_ptr(pUniform + vertexCount + 3);

	qword offset = si_from_int(-vertexCount * 16);

	// Loop preamble
	qword in1__ = *(pUniform + 0);
	qword in2__ = *(pUniform + 1);
	qword in3__ = *(pUniform + 2);
	qword in4__ = *(pUniform + 3);
	qword temp1 = si_shufb(in1__, in2__, sq_AaBb);
	qword temp2 = si_shufb(in3__, in4__, sq_BbAa);
	qword temp3 = si_shufb(in1__, in2__, sq_CcCc);
	qword temp4 = si_shufb(in3__, in4__, sq_CcCc);
	qword xxxx  = si_selb(temp1, temp2, m_00FF);
	qword yyyy  = si_shufb(temp2, temp1, sq_cdAB);
	qword zzzz  = si_selb(temp3, temp4, m_00FF);
	qword partialSum = si_fma(yyyy, yyyy, si_fm(xxxx, xxxx));
	qword sum   = si_fma(zzzz, zzzz, partialSum);
	qword rsqrt = si_fi(sum, si_frsqest(sum));

	qword in1_  = *(pUniform + 4);
	qword in2_  = *(pUniform + 5);
	qword in3_  = *(pUniform + 6);
	qword in4_  = *(pUniform + 7);
	temp1 = si_shufb(in1_, in2_, sq_AaBb);
	temp2 = si_shufb(in3_, in4_, sq_BbAa);
	temp3 = si_shufb(in1_, in2_, sq_CcCc);
	temp4 = si_shufb(in3_, in4_, sq_CcCc);
	xxxx = si_selb(temp1, temp2, m_00FF);
	yyyy = si_shufb(temp2, temp1, sq_cdAB);
	zzzz = si_selb(temp3, temp4, m_00FF);
	partialSum = si_fma(yyyy, yyyy, si_fm(xxxx, xxxx));

	qword in1 = *(pUniform + 8);
	qword in2 = *(pUniform + 9);
	qword in3 = *(pUniform + 10);
	qword in4 = *(pUniform + 11);

	do
	{
		// part 4 -- scale input verts by inverse sqrt and write them back to the uniform table
		si_stqx(si_fm(in1__, si_shufb(rsqrt, k1_0f, sq_AAAa)), pOutVerts1, offset);
		si_stqx(si_fm(in2__, si_shufb(rsqrt, k1_0f, sq_BBBb)), pOutVerts2, offset);
		si_stqx(si_fm(in3__, si_shufb(rsqrt, k1_0f, sq_CCCc)), pOutVerts3, offset);
		si_stqx(si_fm(in4__, si_shufb(rsqrt, k1_0f, sq_DDDd)), pOutVerts4, offset);

		// part 3 -- compute reciprocal square root of all four sums
		sum = si_fma(zzzz, zzzz, partialSum);
		rsqrt = si_fi(sum, si_frsqest(sum));
		in1__ = in1_;
		in2__ = in2_;
		in3__ = in3_;
		in4__ = in4_;

		// part 2 -- sum squares of vertex components
		temp1 = si_shufb(in1, in2, sq_AaBb);
		temp2 = si_shufb(in3, in4, sq_BbAa);
		temp3 = si_shufb(in1, in2, sq_CcCc);
		temp4 = si_shufb(in3, in4, sq_CcCc);
		xxxx = si_selb(temp1, temp2, m_00FF);
		yyyy = si_shufb(temp2, temp1, sq_cdAB);
		zzzz = si_selb(temp3, temp4, m_00FF);
		partialSum = si_fma(yyyy, yyyy, si_fm(xxxx, xxxx));
		in1_ = in1;
		in2_ = in2;
		in3_ = in3;
		in4_ = in4;

		// part 1 -- load input verts
		in1 = si_lqx(pInVerts1, offset);
		in2 = si_lqx(pInVerts2, offset);
		in3 = si_lqx(pInVerts3, offset);
		in4 = si_lqx(pInVerts4, offset);

		offset = si_ai(offset, 0x40);
	} while(si_to_uint(offset) != 0);
}


static inline bool allocateOutputSpaceFromSharedBuffer(uint32_t sharedBufferInfoEa, uint32_t size,
	EdgeGeomAllocationInfo *info)
{
	uint32_t currentEa, result;
	uint8_t sharedInfoBuffer[0x80] __attribute__((__aligned__(128)));
	EdgeGeomSharedBufferInfo volatile *sharedInfo = (EdgeGeomSharedBufferInfo volatile *)sharedInfoBuffer;

	// Get the shared output buffer address and update it in an atomic operation.
	do 
	{
		// Atomic-get the shared buffer info
		spu_writech(MFC_LSA, (uint32_t)sharedInfo);
		spu_writech(MFC_EAL, sharedBufferInfoEa);
		spu_writech(MFC_Cmd, MFC_GETLLAR_CMD);
		result = spu_readch(MFC_RdAtomicStat);
		
		// Check to see if memory is available for this allocation.
		currentEa = sharedInfo->currentEa;
		if (currentEa + size > sharedInfo->endEa)
		{
			// If not, inform the system of an allocation failure.
			sharedInfo->failedAllocSize += size;
			currentEa = 0;
		}
		else
		{
			// Otherwise update the shared buffer info with the new address from which allocate.
			sharedInfo->currentEa = currentEa + size;
			info->location = sharedInfo->locationId;
		}
		
		// Write the shared buffer info back to main memory, making
		// sure nothing it hasn't been changed since it was read.
		spu_writech(MFC_LSA, (uint32_t)sharedInfo);
		spu_writech(MFC_EAL, sharedBufferInfoEa);
		spu_writech(MFC_Cmd, MFC_PUTLLC_CMD);
		result = spu_readch(MFC_RdAtomicStat);
	} while (result & 1);
	
	info->ea = currentEa;
	info->offset = sharedInfo->startOffset + (currentEa - sharedInfo->startEa);
	return (currentEa != 0);
}

static inline bool allocateOutputSpaceFromRingBuffer(EdgeGeomSpuContext *ctx, EdgeGeomRingBufferInfo *ringInfo, uint32_t size,
	bool waitForGpu, EdgeGeomAllocationInfo *info)
{
	// Grab the start and end pointers from the mutex.
	uint32_t labelEa         = ringInfo->rsxLabelEa;
	uint32_t start           = ringInfo->startEa;
	uint32_t end             = ringInfo->endEa;
	uint32_t current         = ringInfo->currentEa;
	uint32_t currentPlusSize = current+size;
	uint32_t startPlusSize   = start+size;

	// If the ring buffer is not at least twice as big as the
	// allocation size, there's a possibility for deadlock.  In that
	// case, we just fail.
	if (2*size > (end-start))
		return false;
	
	uint8_t labelBuffer[0x80] __attribute__((__aligned__(128)));
	uint32_t volatile *pLabelValue = (uint32_t volatile *)labelBuffer;
	bool success = false;
	do
	{
		// Get the RSX label into LS.
		spu_writech(MFC_LSA, (uint32_t)pLabelValue);
		spu_writech(MFC_EAL, labelEa);
		spu_writech(MFC_Size, 4);
		spu_writech(MFC_TagID, ctx->inputDmaTag);
		spu_writech(MFC_Cmd, MFC_GET_CMD);
		
		// Wait for the DMA to complete
		spu_writech(MFC_WrTagMask, 1 << ctx->inputDmaTag);
		spu_writech(MFC_WrTagUpdate, 2);
		spu_readch(MFC_RdTagStat);
		
		uint32_t freeEnd = *pLabelValue;

		// Test to see if the allocation will fit.
		if (((freeEnd <= current) || (currentPlusSize < freeEnd)) &&
			((currentPlusSize <= end) || (startPlusSize < freeEnd)))
		{
			success = true;
			// Check whether we need to flip back to the beginning of the ring buffer
			if (currentPlusSize > end)
			{
				current = start;
				currentPlusSize = startPlusSize;
			}
		}
		else if (!waitForGpu)
		{
			return false;
		}
		// May want to wait for a bit if this fails...
		else
		{
			// Wait ~1000 cycles to avoid flooding the GPU with label
			// reads.  Each read of the SPU decrementer takes 40
			// cycles.
			for(uint32_t i=0; i<25; ++i)
			{
				spu_readch(SPU_RdDec);
			}
		}
	}
	while(!success);
	
	ringInfo->currentEa = currentPlusSize;
	
	info->ea = current;
	info->offset = ringInfo->startOffset + (current - start);
	info->location = ringInfo->locationId;

	return true;
}

uint32_t edgeGeomCalculateDefaultOutputSize(EdgeGeomSpuContext *ctx, uint32_t numIndexes)
{
	uint32_t outputStride = edgeGeomGetOutputFlavorStride(ctx, ctx->spuConfigInfo.outputFlavor);
	// padded size of indexes
	uint32_t allocSize = ((numIndexes + 7) & ~7) * 2; 
	// padded size of vertexes
	allocSize += ctx->spuConfigInfo.numVertexes * outputStride; 
	// all rounded up to a multiple of 128 bytes
	return (allocSize + 0x7F) & ~0x7F;
}

bool edgeGeomAllocateOutputSpace(EdgeGeomSpuContext *ctx, uint32_t outputBufferInfoEa, uint32_t allocSize, EdgeGeomAllocationInfo *info, uint32_t spuId)
{
	// This will be set to true if this info is passed to OutputIndexes or OutputVertexes
	info->isVertexData = false;

	// If the allocator address has the high bit set, then we're
	// allocating from a specific address that's guaranteed to be
	// available, and we don't need to go any further.
	if(outputBufferInfoEa & 0x80000000)
	{
		// direct address
		info->ea = outputBufferInfoEa & 0x7FFFFFFE;
		info->offset = -1;
		info->location = outputBufferInfoEa & 0x1;
		return true;
	}

	// round up to a multiple of 128 bytes
	allocSize = (allocSize + 0x7F) & ~0x7F; 

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	// Load the ring buffer info
	uint8_t ringInfoBuffer[0x80] __attribute__((__aligned__(128)));
	EdgeGeomRingBufferInfo volatile *ringInfo = (EdgeGeomRingBufferInfo volatile *)ringInfoBuffer;
	const uint32_t ringInfoEa = outputBufferInfoEa + sizeof(EdgeGeomSharedBufferInfo)
		+ spuId * sizeof(EdgeGeomRingBufferInfo);
	spu_writech(MFC_LSA, (uint32_t)ringInfo);
	spu_writech(MFC_EAL, ringInfoEa);
	spu_writech(MFC_Size, sizeof(EdgeGeomRingBufferInfo));
	spu_writech(MFC_TagID, ctx->inputDmaTag);
	spu_writech(MFC_Cmd, MFC_GET_CMD);

	// Wait for the DMA to complete
	spu_writech(MFC_WrTagMask, 1 << ctx->inputDmaTag);
	spu_writech(MFC_WrTagUpdate, 2);
	spu_readch(MFC_RdTagStat);

	bool outputToRingBuffer = false;
	bool hasRingBuffer = (ringInfo->endEa - ringInfo->startEa > 0);
	bool success = false;

	if (!hasRingBuffer)
	{
		// Allocate from the shared buffer
		success = allocateOutputSpaceFromSharedBuffer(outputBufferInfoEa, allocSize, info);
	}
	else
	{
		// Attempt to allocate from the ring buffer once.  waitForGpu
		// is set to false, so if the allocation wouldn't succeed
		// we'll return immediately instead of blocking.
		success = allocateOutputSpaceFromRingBuffer(ctx, (EdgeGeomRingBufferInfo*)ringInfo, allocSize, false, info);
		if (success)
		{
			outputToRingBuffer = true;
		}
		else
		{
			// If the initial ring buffer allocate attempt failed, attempt to
			// allocate from the shared buffer.
			success = allocateOutputSpaceFromSharedBuffer(outputBufferInfoEa, allocSize, info);
			
			if (!success)
			{
				// If the overflow allocate attempt failed, go back to
				// the ring buffer and block until it succeeds.
				success = allocateOutputSpaceFromRingBuffer(ctx, (EdgeGeomRingBufferInfo*)ringInfo,
					allocSize, true, info);
				outputToRingBuffer = true;
			}
		}
	}

	if (outputToRingBuffer) {
		// Store the ring buffer info back to main memory
		spu_writech(MFC_LSA, (uint32_t)ringInfo);
		spu_writech(MFC_EAL, ringInfoEa);
		spu_writech(MFC_Size, sizeof(EdgeGeomRingBufferInfo));
		spu_writech(MFC_TagID, ctx->outputDmaTag);
		spu_writech(MFC_Cmd, MFC_PUTF_CMD);
	
		spu_writech(MFC_WrTagMask, 1 << ctx->outputDmaTag);
		spu_writech(MFC_WrTagUpdate, 2);
		spu_readch(MFC_RdTagStat);
	}

	if (interruptsEnabled)
		spu_ienable();

	// the allocation failed?
	if(!success)
		return false; 

	// Set some info if the allocation succeeded and is
	// from a ring buffer.
	if (outputToRingBuffer)
	{
		info->rsxLabelEa = ringInfo->rsxLabelEa;
		info->endEa = info->ea + allocSize;
	}
	else
	{
		info->rsxLabelEa = 0;
		info->endEa = 0;
	}

	return true;
}

void edgeGeomOutputIndexes(EdgeGeomSpuContext *ctx, uint32_t numIndexes, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_OUTPUT_INDEXES);

	if(!numIndexes)
		return;

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	
	spu_idisable();
	uint32_t size = ((numIndexes + 7) & ~7) * 2;
	uint32_t ls = ctx->indexesLs;
	uint32_t ea = info->ea;
	uint32_t offset = info->offset;
	outLoc->ea = ea;
	outLoc->offset = offset;
	outLoc->location = info->location;
	do
	{
		uint32_t s = (size > 0x4000) ? 0x4000 : size;
		spu_writech(MFC_LSA, ls);
		spu_writech(MFC_EAL, ea);
		spu_writech(MFC_Size, s);
		spu_writech(MFC_TagID, ctx->outputDmaTag);
		spu_writech(MFC_Cmd, MFC_PUT_CMD);
		ls += s;
		ea += s;
		offset += s;
		size -= s;
	} while(size > 0);
	info->ea = ea;
	info->offset = offset;
	if(interruptsEnabled)
		spu_ienable();

	info->isVertexData = true;
}

void edgeGeomOutputVertexes(EdgeGeomSpuContext *ctx, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_OUTPUT_VERTEXES);

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();
	uint32_t stride = edgeGeomGetOutputFlavorStride(ctx, ctx->spuConfigInfo.outputFlavor);
	uint32_t size = (ctx->spuConfigInfo.numVertexes * stride + 0xF) & ~0xF;
	uint32_t ls = ctx->vertexesLs;
	uint32_t ea = info->ea;
	uint32_t offset = info->offset;
	outLoc->ea = ea;
	outLoc->offset = offset;
	outLoc->location = info->location;
	do
	{
		uint32_t s = (size > 0x4000) ? 0x4000 : size;
		spu_writech(MFC_LSA, ls);
		spu_writech(MFC_EAL, ea);
		spu_writech(MFC_Size, s);
		spu_writech(MFC_TagID, ctx->outputDmaTag);
		spu_writech(MFC_Cmd, MFC_PUT_CMD);
		ls += s;
		ea += s;
		offset += s;
		size -= s;
	} while(size > 0);
	info->ea = ea;
	info->offset = offset;
	if(interruptsEnabled)
		spu_ienable();

	info->isVertexData = true;
}

static const qword s_Dabc = {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};

// Patches a command buffer in the appropriate places for jump to self sync.
// Note: Will not correctly parse Jumps, Calls, or Returns.
static inline uint32_t patchForLocalStallSync(uint32_t *cmd, uint32_t size)
{
	if(!size)
		return 0;
		
	qword cmdEnd = si_from_ptr(cmd + size / 4);
    qword cmd0 = si_from_ptr(cmd);

    qword cmd1         = (qword)(0);
    qword notPastEnd   = (qword)(0);
    qword startAligned = (qword)(0);
    qword endAligned   = startAligned;
    qword cmd2         = (qword)(0);
    qword num          = (qword)(0);
    qword get0         = (qword)(0);
    qword sizeQw = si_from_uint(size);

    const qword mask0 = (qword)spu_splats(0x1ffc);
    const qword alignMask = (qword)spu_splats(0x7F);
    const qword getMask = (qword)spu_splats(0xE003FFFF);
    const qword s_Aabc = si_selb(si_ila(0x10203), s_Dabc, si_fsm(si_from_uint(0x7)));

    qword out0 = (qword)(0);
    qword out1 = (qword)(0);
    qword out2 = (qword)(0);
    qword out3 = (qword)(0);
    qword in4  = (qword)(0);

    //note: Tabbing indicates loop iteration.
    do 
	{
            notPastEnd = si_clgt(cmdEnd, cmd1);
        qword get0Input = si_lqd(cmd0, 0x0);
            qword isAligned = si_ceq(startAligned, endAligned);

        startAligned = si_andc(cmd0, alignMask);
        if(__builtin_expect(si_to_uint(notPastEnd) == 0, false))
            break;

            cmd2 = si_ori(cmd1, 0);
        cmd1 = si_ori(cmd0, 0);
            qword get2 = si_ori(get0, 0);

            qword cmd2Added = si_ai(cmd2, 0x80);
        get0 = si_rotqby(get0Input, cmd0);
            qword cmd2Sub = si_ai(cmd2, 4);
            qword get2Rotated = si_rotqmbii(si_rotqmbyi(get2, -3), -4);
            
            qword splitAt = si_andc(cmd2Added, alignMask);
        qword get0Rotated = si_rotqmbyi(get0, -2);
            qword newNum = si_sf(cmd2Sub, splitAt);
            qword get2Anded1 = si_andi(get2Rotated, 0x4);
            qword num2 = si_sf(newNum, num);
            qword inc = si_xori(get2Anded1, 0x4);
        num = si_and(get0Rotated, mask0);
            qword newNumShifted = si_shli(newNum, 16);
        qword end = si_a(cmd0, num);
            qword incXnewNum = si_mpyu(inc, si_rotqmbii(newNum,-2));
        cmd0 = si_ai(end, 4);
        endAligned = si_andc(end, alignMask);

        if (__builtin_expect(si_to_uint(isAligned), true))
            continue;

        qword oldCmdValue = si_lqd(cmd2, 0x0);
        qword pInTmp0 = si_sf(splitAt, cmdEnd);
        qword pInTmp1 = si_ai(pInTmp0, 0x30);
        qword iterCount = si_ai(si_rotmi(pInTmp1, -6), 1);
        qword pInTmp2 = si_andi(pInTmp1, -0x40);
        qword pIn = si_a(splitAt, pInTmp2);

        qword newCmd = si_selb(newNumShifted, get2, getMask);
        qword s_mask = si_cwd(cmd2, 0x0);
        qword newCmdOut = si_shufb(newCmd, oldCmdValue, s_mask);
        si_stqd(newCmdOut, cmd2, 0x00);

        do
        {
            iterCount = si_ai(iterCount, -1);
            in4 = si_lqd(pIn, 0x00);
            qword in3 = si_lqd(pIn, -0x10);
            qword in2 = si_lqd(pIn, -0x20);
            qword in1 = si_lqd(pIn, -0x30);
            qword in0 = si_lqd(pIn, -0x40);
            si_stqd(out3, pIn, 0x40);
            si_stqd(out2, pIn, 0x30);
            si_stqd(out1, pIn, 0x20);
            si_stqd(out0, pIn, 0x10);
            out3 = si_shufb(in3, in4, s_Dabc);
            out2 = si_shufb(in2, in3, s_Dabc);
            out1 = si_shufb(in1, in2, s_Dabc);
            out0 = si_shufb(in0, in1, s_Dabc);
            pIn = si_ai(pIn, -0x40);
        } while(si_to_uint(iterCount) != 0);

        qword num2Shifted = si_shli(num2, 16);
        qword tmp0 = si_selb(num2Shifted, get2, getMask);
        qword newSplitAt = si_a(tmp0, incXnewNum);

        qword splitAtShifted = si_shufb(newSplitAt, in4, s_Aabc);
		
        si_stqd(splitAtShifted, splitAt, 0x0);

		cmd0 = splitAt;
        cmd1 = cmd0;
		cmdEnd = si_ai(cmdEnd,4);
		sizeQw = si_ai(sizeQw, 4);
        endAligned = startAligned;
	} while(true);

	return si_to_uint(sizeQw);
}

void edgeGeomBeginCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos)
{
	uint32_t syncData = (uint32_t)ctx->freePtr;
    ctx->freePtr = (void*)((uint32_t)ctx->freePtr + 16);

	uint32_t *cmd = (uint32_t *)ctx->freePtr;
	// Align the LS address to the EA address
	cmd = (uint32_t*)((((uint32_t)cmd + 0x7F) & ~0x7F) + (holeEa & 0x7F));
	gcmCtx->begin = cmd;
	gcmCtx->current = cmd;
	gcmCtx->end = cmd + (ctx->spuConfigInfo.commandBufferHoleSize << 2) + 4;
	gcmCtx->callback = 0;
	ctx->freePtr = (void*)(((uint32_t)gcmCtx->end + 127) & -128);

	/* 	If we've written output data to video memory, we need to make sure
		that those output DMAs are finished before the command buffer hole
		is filled, otherwise the RSX might try to access the data before
		it's been committed to video memory. */
	for (uint32_t i = 0; i < numInfos; i++) {
		if (infos[i].location == CELL_GCM_LOCATION_LOCAL) {

			bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();

			/* 	Read 4 bytes from video memory with a fence, to make sure
				that the proceeding output DMAs finish before the command 
				buffer hole is filled. */
			spu_writech(MFC_LSA, syncData);
			spu_writech(MFC_EAL, infos[i].ea);
			spu_writech(MFC_Size, 4);
			spu_writech(MFC_TagID, ctx->outputDmaTag);
			spu_writech(MFC_Cmd, MFC_GETF_CMD);

			if(interruptsEnabled)
				spu_ienable();

			/*  We only need to do this once, since all of the outputs have
				already been started, so break. */
			break;
		}
	}

	bool invalidateVertexCache = false;
	for (uint32_t i = 0; i < numInfos; i++) {
		invalidateVertexCache = invalidateVertexCache || (infos[i].rsxLabelEa && infos[i].isVertexData);
	}

	if (invalidateVertexCache) {
	//	cellGcmSetInvalidateVertexCacheUnsafeInline(gcmCtx);
	}

	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_FILL_COMMAND_BUFFER_HOLE);
} 

void edgeGeomSetVertexDataArrays(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, EdgeGeomLocation *loc)
{
	// Write Attribute Addresses and formats
	uint32_t offset = loc->offset;
	uint32_t location = loc->location;
	uint32_t stride = edgeGeomGetOutputFlavorStride(ctx, ctx->spuConfigInfo.outputFlavor);
	switch(ctx->spuConfigInfo.outputFlavor)
	{
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 2, 0, stride, 1, 6, location, offset + 12);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 14, 0, stride, 1, 6, location, offset + 16);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 2, 0, stride, 1, 6, location, offset + 12);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 14, 0, stride, 4, 3, location, offset + 16);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_F16c2:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 2, 0, stride, 1, 6, location, offset + 12);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 14, 0, stride, 1, 6, location, offset + 16);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 8, 0, stride, 2, 3, location, offset + 20);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4_F16c2:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 2, 0, stride, 1, 6, location, offset + 12);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 14, 0, stride, 4, 3, location, offset + 16);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 8, 0, stride, 2, 3, location, offset + 24);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 2, 0, stride, 1, 6, location, offset + 12);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 14, 0, stride, 1, 6, location, offset + 16);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 15, 0, stride, 1, 6, location, offset + 20);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N_F16c2:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 2, 0, stride, 1, 6, location, offset + 12);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 14, 0, stride, 1, 6, location, offset + 16);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 15, 0, stride, 1, 6, location, offset + 20);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 8, 0, stride, 2, 3, location, offset + 24);
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_F16c2:
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 0, 0, stride, 3, 2, location, offset);
		cellGcmSetVertexDataArrayUnsafeInline(gcmCtx, 8, 0, stride, 2, 3, location, offset + 12);
		break;
	default:
		if(ctx->setVertexDataArraysCallback != 0)
		{
			(*ctx->setVertexDataArraysCallback)(ctx, ctx->spuConfigInfo.outputFlavor, gcmCtx, location, offset);
		}
		break;
	}
}

void edgeGeomEndCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos)
{
	if(!holeEa)
		return;
		
	for(uint32_t i = 0; i < numInfos; ++i)
	{
		// rsxLabelEa is only set when using a ring buffer.
		uint32_t rsxLabelEa = infos[i].rsxLabelEa;

		// If using ring buffers for output, then generate the command 
		// buffer commands that will update the RSX label value once 
		// the data created by this job is completely consumed.
		if(rsxLabelEa)
			cellGcmSetWriteTextureLabelUnsafeInline(gcmCtx, (rsxLabelEa & 0x0FF0) >> 4, infos[i].endEa);
	}

	uint32_t size = (gcmCtx->current - gcmCtx->begin) * 4;
	uint32_t newSize = patchForLocalStallSync(gcmCtx->begin, size);

	// Zero out the rest of the command buffer hole
	uint32_t *cmd = gcmCtx->current + ((newSize - size) / 4);
	uint32_t *cmdEnd = gcmCtx->end;
	const qword zero = (qword)spu_splats(0);
	const qword negativeOne = si_fsm(si_from_uint(0xf));
	const qword rotAmount = si_sfi(si_andi(si_from_ptr(cmd), 0xf),0);
	const qword mask = si_rotqmby(negativeOne, rotAmount);
	const qword tmp = si_lqx(si_from_ptr(cmd), zero);
	const qword out = si_selb(tmp, zero, mask);
	si_stqx(out, si_from_ptr(cmd), zero);
	qword aligned = si_andi(si_ai(si_from_ptr(cmd), 0xf), ~0xf);
	qword loop;
	do
	{
		si_stqx(zero, aligned, zero);
		aligned = si_ai(aligned, 0x10);
		loop = si_clgt(si_from_ptr(cmdEnd), aligned);
	} while(si_to_uint(loop));

	uint32_t holeSize = ctx->spuConfigInfo.commandBufferHoleSize ? (ctx->spuConfigInfo.commandBufferHoleSize << 4) : 4;
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();
	spu_writech(MFC_LSA, (uint32_t)gcmCtx->begin);
	spu_writech(MFC_EAL, holeEa);
	spu_writech(MFC_Size, holeSize);
	spu_writech(MFC_TagID, ctx->outputDmaTag);
	spu_writech(MFC_Cmd, MFC_PUTF_CMD);
	if(interruptsEnabled)
		spu_ienable();
}
