/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_printf.h>

#include "edge/geom/edgegeom.h"

/* from edgegeom.cpp */
#ifdef EDGE_GEOM_DEBUG
extern void edgeGeomValidateProcessingOrder(EdgeGeomSpuContext *ctx, uint32_t currentOp);
#else
static inline void edgeGeomValidateProcessingOrder(EdgeGeomSpuContext *ctx, uint32_t currentOp){}
#endif


uint32_t edgeGeomGetOutputFlavorStride(EdgeGeomSpuContext *ctx, uint32_t flavor)
{
	switch(flavor)
	{
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3:
		return 12;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N:
		return 20;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4:
		return 24;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_F16c2:
		return 24;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4_F16c2:
		return 28;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		return 24;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N_F16c2:
		return 28;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_F16c2:
		return 16;
	default:
		if(ctx->outputStrideCallback != 0)
			return (*ctx->outputStrideCallback)(ctx, flavor);
	}
	return 0;
}

void *edgeGeomCompressVertexes(EdgeGeomSpuContext *ctx)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_COMPRESS_VERTEXES);
	
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return (void*)ctx->vertexesLs;

	void *outVertexes = ctx->freePtr;
	ctx->vertexesLs = (uint32_t)outVertexes;

	if(ctx->spuConfigInfo.numVertexes == 0)
		return outVertexes;

	// This sets up some global decompression structures which are
	// independent of the input flavor.  To reduce code bloat, it's
	// done once outside the loop instead of being duplicated inside
	// each case block.
	EDGE_GEOM_COMPRESS_INIT_GLOBAL_COMMON(ctx, ctx->spuConfigInfo.numVertexes, outVertexes);
		
	switch(ctx->spuConfigInfo.outputFlavor)
	{
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(12);
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();

				EDGE_GEOM_COMPRESS_LOOP_F32(0);

				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(20);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(1, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(2, 16, 4);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(2);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(1, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_F16(2, 16, 8);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_COMPRESS_LOOP_F16(2);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_F16c2:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(1, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(2, 16, 4);
			EDGE_GEOM_COMPRESS_INIT_F16(3, 20, 4);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(2);
				EDGE_GEOM_COMPRESS_LOOP_F16(3);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4_F16c2:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(28);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(1, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_F16(2, 16, 8);
			EDGE_GEOM_COMPRESS_INIT_F16(3, 24, 4);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_COMPRESS_LOOP_F16(2);
				EDGE_GEOM_COMPRESS_LOOP_F16(3);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(1, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(2, 16, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(3, 20, 4);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(2);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(3);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N_F16c2:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(28);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(1, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(2, 16, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(3, 20, 4);
			EDGE_GEOM_COMPRESS_INIT_F16(4, 24, 4);

			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(2);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(3);
				EDGE_GEOM_COMPRESS_LOOP_F16(4);

				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_OUTPUT_FLAVOR_F32c3_F16c2:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(16);
			
			EDGE_GEOM_COMPRESS_INIT_F32(0, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_F16(1, 12, 4);

			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(0);
				EDGE_GEOM_COMPRESS_LOOP_F16(1);

				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	default:
		if(ctx->compressOutputCallback != 0)
		{
			(*ctx->compressOutputCallback)(ctx, ctx->spuConfigInfo.numVertexes, outVertexes, ctx->spuConfigInfo.outputFlavor);
		}
		else
		{
			spu_printf("EDGE error: unknown output flavor %d for vertex stream", ctx->spuConfigInfo.outputFlavor);
			si_stopd(si_from_uint(0), si_from_uint(0), si_from_uint(0));
		}
		break;
	}

	ctx->freePtr = (void*)((uint32_t)ctx->freePtr + ((edgeGeomGetOutputFlavorStride(ctx, ctx->spuConfigInfo.outputFlavor) * ctx->spuConfigInfo.numVertexes + 0xF) & ~0xF));
	return outVertexes;
}
