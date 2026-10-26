/* SCE CONFIDENTIAL 
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOMSTRUCTS_H
#define EDGEGEOMSTRUCTS_H

#include "Display/SPU/Edge/edge_stdint.h"
#include "Display/SPU/Edge/edge_stdbool.h"

// We want certain structures to be aligned on the PS3 (for DMA purposes) without padding
// their size.  GCC's attribute-align directive allows this; Visual C++'s declspec-align
// directive automatically pads the structures.  Since we really only care about defining
// the structures in VC so we can use sizeof(), we opt to just not align them in VC at all.
#if !defined(_MSC_VER)
#define EDGE_ALIGN_NO_PAD(x) __attribute__((__aligned__(x)))
#else
#define EDGE_ALIGN_NO_PAD(x)
#endif

#define EDGE_GEOM_CULL_NONE                   0
#define EDGE_GEOM_CULL_FRUSTUM                1
#define EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM  2
#define EDGE_GEOM_CULL_FRONTFACES_AND_FRUSTUM 3

#define EDGE_GEOM_SKIN_NONE                0
#define EDGE_GEOM_SKIN_NO_SCALING          1
#define EDGE_GEOM_SKIN_UNIFORM_SCALING     2
#define EDGE_GEOM_SKIN_NON_UNIFORM_SCALING 3

#define EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CW  0
#define EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CCW 1
#define EDGE_GEOM_INDEXES_SW_TRIANGLE_LIST_CW   2
#define EDGE_GEOM_INDEXES_SW_TRIANGLE_LIST_CCW  3

#define EDGE_GEOM_INPUT_FLAVOR_F32c3                                        0
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N                  1
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4                       2
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_F16c2            3
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4_F16c2                 4
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N       5
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N_F16c2 6
#define EDGE_GEOM_INPUT_FLAVOR_F32c3_F16c2                                  7
#define EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC                        8
#define EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC_F16c2                  9
#define EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC_UNITVEC                10
#define EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC_UNITVEC_F16c2          11

#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3                                        0
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N                  1
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4                       2
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_F16c2            3
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4_F16c2                 4
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N       5
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N_F16c2 6
#define EDGE_GEOM_OUTPUT_FLAVOR_F32c3_F16c2                                  7

#define EDGE_GEOM_SHAPE_FLAVOR_FIXEDc3       0
#define EDGE_GEOM_SHAPE_FLAVOR_FIXEDc3_F16c3 1
#define EDGE_GEOM_SHAPE_FLAVOR_F32c3_F32c3   2

#define EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC  0
#define EDGE_GEOM_ATTRIBUTE_USAGE_POSITION 1
#define EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL   2
#define EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT  3
#define EDGE_GEOM_ATTRIBUTE_USAGE_BINORMAL 4

/* These masks are used to interpret the return value of
 * edgeGeomValidateBufferOrder().  Each buffer has a bit which is set if
 * that buffer isn't in a valid location.  In some cases, a misplaced
 * buffer is a fatal error; in others, it's simply a performance
 * warning. See the LibEdge-Overview file for more information on the
 * correct buffer ordering.
 */
#define EDGE_GEOM_VALIDATE_WARNING_VIEWPORT_INFO     0x00000001
#define EDGE_GEOM_VALIDATE_WARNING_LOCAL_TO_WORLD    0x00000002
#define EDGE_GEOM_VALIDATE_WARNING_SPU_CONFIG_INFO   0x00000004
#define EDGE_GEOM_VALIDATE_WARNING_FIXED_OFFSETS     0x00000008
#define EDGE_GEOM_VALIDATE_WARNING_CUSTOM_INPUT_CODE 0x00000010
#define EDGE_GEOM_VALIDATE_WARNING_FAST_PATH         0x00000020
#define EDGE_GEOM_VALIDATE_ERROR_SKINNING_MATRICES   0x00010000
#define EDGE_GEOM_VALIDATE_ERROR_SKINNING_WEIGHTS    0x00020000
#define EDGE_GEOM_VALIDATE_ERROR_INDEXES             0x00040000
#define EDGE_GEOM_VALIDATE_ERROR_CUSTOM_OUTPUT_CODE  0x00080000
#define EDGE_GEOM_VALIDATE_ERROR_CUSTOM_BLEND_CODE   0x00100000
#define EDGE_GEOM_VALIDATE_ERROR_FAST_PATH           0x00200000

/* These masks can be used to quickly check whether any errors or
 * warnings were reported.
 */
#define EDGE_GEOM_VALIDATE_ERROR_MASK                0xFFFF0000
#define EDGE_GEOM_VALIDATE_WARNING_MASK              0x0000FFFF

/*
 * These are the various discrete operations that can be performed,
 * used to ensure that processing occurs in the correct order.
 */
#define EDGE_GEOM_OPERATION_INITIALIZE                1
#define EDGE_GEOM_OPERATION_SET_CALLBACKS             2
#define EDGE_GEOM_OPERATION_DECOMPRESS_VERTEXES       3
#define EDGE_GEOM_OPERATION_PROCESS_BLEND_SHAPES      4
#define EDGE_GEOM_OPERATION_SKIN_VERTEXES             5
#define EDGE_GEOM_OPERATION_DECOMPRESS_INDEXES        6
#define EDGE_GEOM_OPERATION_CULL_TRIANGLES            7
#define EDGE_GEOM_OPERATION_OUTPUT_INDEXES            8
#define EDGE_GEOM_OPERATION_COMPRESS_VERTEXES         9
#define EDGE_GEOM_OPERATION_OUTPUT_VERTEXES           10
#define EDGE_GEOM_OPERATION_FILL_COMMAND_BUFFER_HOLE  11

/*
 * These flags define the various processing options for the SPU.
 * They exist in the upper 4 bits of the first byte of EdgeGeomSpuConfigInfo.
 */
#define EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH  0x10

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomSpuConfigInfo
{
	uint8_t flagsAndUniformTableCount; // uniform count - 1
	uint8_t commandBufferHoleSize; // >> 4
	uint8_t inputFlavor; // Custom is 0xFF
	uint8_t outputFlavor; // Custom is 0xFF
	uint8_t shapeFlavor; // Custom is 0xFF
	uint8_t indexesFlavorAndSkinningFlavor;
	uint16_t pad;
	uint16_t numVertexes;
	uint16_t numIndexes;
	uint32_t indexesOffset; // Low bit is location
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomPpuConfigInfo
{
	EdgeGeomSpuConfigInfo spuConfigInfo;
	void *indexes;
	uint16_t indexesSizes[2];
	void *vertexes[2];
	uint16_t vertexesSizes[6];
	uint16_t skinMatricesByteOffsets[2];
	uint16_t skinMatricesSizes[2];
	uint16_t skinIndexesAndWeightsSizes[2];
	uint16_t shapeSize;
	uint16_t shapeFixedOffsetsSize;
	void *skinIndexesAndWeights;
	uint32_t ioBufferSize;
	uint32_t scratchSize;
	uint32_t *shapeTags; // Null if no blendshapes
	void *customDecompress;
	void *customCompress;
	void *customBlend;
	uint16_t customDecompressSize;
	uint16_t customCompressSize;
	uint16_t customBlendSize;
	uint16_t pad0;
	uint32_t pad1;
	uint32_t pad2;
	uint32_t fixedOffsetsSize[2];
	void * fixedOffsets[2];
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomBlendShapeInfo
{
	uint64_t dmaTag;
	float alpha;
	uint32_t fixedOffsetsSize; // offsets are before vertex data
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomViewportInfo
{
	uint16_t scissorArea[4];
	float depthRange[2];
	float viewProjectionMatrix[16];
	float viewportScales[4];
	float viewportOffsets[4];
    uint8_t sampleFlavor;
    uint8_t pad[3];
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomLocalToWorldMatrix
{
	float matrixData[12];
};

// A simple output buffer that's shared (via atomic locks) between all SPUs.
// When it's full, subsequent allocations just fail.
struct EDGE_ALIGN_NO_PAD(128) EdgeGeomSharedBufferInfo
{
	uint32_t startEa;     // address of buffer start
	uint32_t startOffset; // RSX offset of buffer start 
	uint32_t endEa;       // address of buffer end (start + size)
	uint32_t currentEa;   // address of beginning of buffer free area
	uint32_t locationId;  // CELL_GCM_LOCATION_MAIN or CELL_GCM_LOCATION_LOCAL
	uint32_t failedAllocSize; // total size of allocations that couldn't be fulfilled per frame.

	uint32_t pad1[2]; // pad out to 128 bytes, to facilitate atomic DMA access.
	uint32_t pad2[24]; 
};

// An effectively infinite output buffer that's synchronized with the
// RSX, so that it knows when it's safe to loop back on itself.  If an
// allocation won't fit, the SPU blocks until the RSX consumes enough
// data to free up the required space.
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomRingBufferInfo
{
	uint32_t startEa;     // address of buffer start
	uint32_t startOffset; // RSX offset of buffer start
	uint32_t endEa;       // address of buffer end (start + size)
	uint32_t currentEa;   // address of beginning of buffer free area
	uint32_t locationId;  // CELL_GCM_LOCATION_MAIN or CELL_GCM_LOCATION_LOCAL
	uint32_t rsxLabelEa;  // Address of RSX label used for synchronization

	uint32_t pad1[2]; // pad out to 32 bytes
};

// Describes the application's output buffering scheme.  A buffer is
// considered empty/nonexistent if its startEa and endEa fields are
// the same.
//
// If only the shared buffer non-empty, single buffering output is used.
//
// If only the ring buffer is non-empty, ring buffering output is used.
//
// If both buffers are non-empty, then ring buffering output is used,
// with the shared buffer providing overflow space when the SPU would
// otherwise have to wait for the RSX.  This is the recommended
// configuration.
struct EDGE_ALIGN_NO_PAD(128) EdgeGeomOutputBufferInfo
{
	EdgeGeomSharedBufferInfo sharedInfo; // shared by all SPUs
	EdgeGeomRingBufferInfo ringInfo[6];  // one per SPU addressable by SPURS
};

// Describes the allocation reserved by edgeGeomAllocateOutputSpace().
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomAllocationInfo
{
	// Effective address of the allocation
	uint32_t ea;
	// RSX offset of the allocation
	uint32_t offset;
	// RSX location of the allocation
	uint32_t location;
	// the end of the allocation (ring buffers only)
	uint32_t endEa;
	// Effective address of an rsx label (ring buffers only)
	uint32_t rsxLabelEa;
	// Set if used in edgeGeomOutputVertexes() or EdgeGeomOutputIndexes()
	bool isVertexData;
};

// Desribes where in memory something is located from both the 
// CELL and RSX perspectives.
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomLocation
{
	// Effective address
	uint32_t ea;
	// RSX offset
	uint32_t offset;
	// RSX location (main or video memory)
	uint32_t location;
};

//predeclare for the sake of the typedefs
struct EdgeGeomSpuContext;

typedef uint32_t (*EdgeGeomGetOutputFlavorStrideCallback)(EdgeGeomSpuContext *ctx, uint32_t flavor);
typedef uint32_t (*EdgeGeomDecompressVertexStreamCallback)(EdgeGeomSpuContext *ctx,
	const void *vertexesA, const void *vertexesB, uint32_t numVertexes, const void *fixedOffsetsA,
	const void *fixedOffsetsB, uint32_t flavor);
typedef uint32_t (*EdgeGeomBlendVertexStreamCallback)(EdgeGeomSpuContext *ctx, const void *vertexes, uint32_t numVertexes, const void *fixedOffsets, uint32_t flavor, float alpha);
typedef uint32_t (*EdgeGeomCompressVertexStreamCallback)(EdgeGeomSpuContext *ctx, uint32_t numVertexes, void *outVertexes, uint32_t flavor);

//The tools don't need to know that ctx is a Gcm structure, and we'd rather not include the Gcm
//headers when we don't have to.
#ifndef EDGE_TOOLS 
typedef uint32_t (*EdgeGeomSetVertexDataArraysCallback)(EdgeGeomSpuContext *ctx, uint32_t flavor, CellGcmContextData *gcmCtx,
	uint32_t outputLocation, uint32_t vertexOffset);
#else
typedef uint32_t (*EdgeGeomSetVertexDataArraysCallback)(EdgeGeomSpuContext *ctx, uint32_t flavor, void *gcmCtx,
	uint32_t outputLocation, uint32_t vertexOffset);
#endif

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomSpuContext
{
	EdgeGeomSpuConfigInfo spuConfigInfo;
	float *uniformTables[16];
	uint32_t indexesLs;
	uint32_t vertexesLs;
	void *freePtr;
	void *positionTable;
	void *normalTable;
	void *tangentTable;
	void *binormalTable;
	EdgeGeomDecompressVertexStreamCallback decompressInputCallback;
	EdgeGeomBlendVertexStreamCallback decompressBlendCallback;
	EdgeGeomGetOutputFlavorStrideCallback outputStrideCallback;
	EdgeGeomCompressVertexStreamCallback compressOutputCallback;
	EdgeGeomSetVertexDataArraysCallback setVertexDataArraysCallback;
	EdgeGeomLocalToWorldMatrix localToWorldMatrix;
	EdgeGeomViewportInfo viewportInfo;
	uint32_t outputDmaTag;
	uint32_t inputDmaTag;
	uint32_t numVisibleIndexes;
};

// Used to extract custom callbacks from a block of data
struct EdgeGeomCustomCallbackBlock
{
	uint32_t numCallbacks;
	void *callbacks[1]; // actually contains numCallbacks entries
};
typedef EdgeGeomCustomCallbackBlock* (*EdgeGeomGetCustomCallbackBlockFunc)(void);

struct EdgeGeomCustomCallbackInfo
{
	// These point to custom input, output and blend code blocks
	// generated by the tools and transfered with the job data.
	// If no custom code is present, set them to NULL.
	EdgeGeomGetCustomCallbackBlockFunc customInputCode;
	EdgeGeomGetCustomCallbackBlockFunc customBlendCode;
	EdgeGeomGetCustomCallbackBlockFunc customOutputCode;
	
	// Alternately, individual functions can be overloaded with custom
	// SPU code that's linked in with the edge SPU elf.  If present,
	// these functions override the custom code blocks specified
	// above.  Otherwise, set them to NULL.
	EdgeGeomDecompressVertexStreamCallback decompressInputCallback;
	EdgeGeomBlendVertexStreamCallback decompressBlendCallback;
	EdgeGeomGetOutputFlavorStrideCallback outputStrideCallback;
	EdgeGeomCompressVertexStreamCallback compressOutputCallback;
	EdgeGeomSetVertexDataArraysCallback setVertexDataArraysCallback;
};

#ifdef EDGE_ALIGN_NO_PAD
#undef EDGE_ALIGN_NO_PAD
#endif

#endif // EDGEGEOMSTRUCTS_H

