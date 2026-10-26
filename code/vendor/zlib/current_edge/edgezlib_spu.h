/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ZLIB_SPU_H__
#define __EDGE_ZLIB_SPU_H__

#include "edge_stdint.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////

// EDGE_ZLIB_ENABLE_DATA_ASSERTS can be set to either 0 or 1, to
// respectively disable or enable the optional asserts for bad data.
// By default they are disabled.
//
// If enabled, the SPU will assert as soon as it sees an error which
// might be caused by bad data.
// When disabled, if the SPU sees an error cause by bad data, instead of
// asserting, it will report the error by setting the top bit of the
// work-to-do-counter.  The PPU can then, say, re-load the data and
// re-submit it for decompression.
// Note that if the data asserts are disabled, then we still output the
// message anyway, but leave the SPU to carry on and recover.

#ifndef EDGE_ZLIB_ENABLE_DATA_ASSERTS
	#define EDGE_ZLIB_ENABLE_DATA_ASSERTS 0
#endif

#if EDGE_ZLIB_ENABLE_DATA_ASSERTS
	#define EDGE_ZLIB_DATA_ASSERT_MSG( condition, format )			\
		do															\
		{															\
			if ( __builtin_expect( !( condition), 0 ) )				\
			{														\
				EDGE_PRINTF format ;								\
				_edgeAssertionFailure( __FILE__, __LINE__ );		\
			}														\
		} while (0)
#else
	#define EDGE_ZLIB_DATA_ASSERT_MSG( condition, format )			\
		do															\
		{															\
			if ( __builtin_expect( !( condition), 0 ) )				\
			{														\
				EDGE_PRINTF format ;								\
			}														\
		} while (0)
#endif

//////////////////////////////////////////////////////////////////////////

typedef uint32_t EdgeZlibInflateQHandle;

enum EdgeZlibInflateTaskProcessing
{
	kEdgeZlibInflateTask_Memcpy		= 0,
	kEdgeZlibInflateTask_Inflate	= 1,
};

//////////////////////////////////////////////////////////////////////////

extern int edgeZlibInflateRawData(		unsigned char* pUncompr, uint32_t expectedUncompSize,
										const unsigned char* pComprData, uint32_t comprDataSize );

extern int edgeZlibFetchAndInflateRawData(	uint32_t eaUncompressedOutput, uint32_t expectedUncompSize,
											uint32_t eaCompressed, uint32_t compressedSize,
											uint32_t dmaTag,
											unsigned char* pLsInputTempBuffer,
											uint32_t inputTempBufferSize,
											unsigned char* pLsOutputTempBuffer,
											uint32_t outputTempBufferSize	);

extern void edgeZlibAddInflateQueueElement(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag );

extern bool edgeZlibTryAddInflateQueueElement(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag );

extern void edgeZlibAddInflateQueueElementPartialCopyOut(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompPartialBuff,
        uint16_t	                    outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
	    uint16_t	                    outputUncompSkipEndSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag );

extern bool edgeZlibTryAddInflateQueueElementPartialCopyOut(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompPartialBuff,
        uint16_t	                    outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
	    uint16_t	                    outputUncompSkipEndSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag );

//////////////////////////////////////////////////////////////////////////

extern void _edgeZlibUnalignedLargeDmaPut( const void* ls, uint32_t ea, uint32_t size, uint32_t tag );

//////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
}
#endif // __cplusplus

#endif	//__EDGE_ZLIB_SPU_H__
