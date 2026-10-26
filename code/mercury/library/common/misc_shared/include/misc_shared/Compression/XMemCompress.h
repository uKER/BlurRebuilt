#ifndef MISC_SHARED_XMEMCOMPRESS_H
#define MISC_SHARED_XMEMCOMPRESS_H
#if _MSC_VER > 1000
#pragma once
#endif

#ifdef USE_XMEM_COMPRESSION
#include "xcompress.h"
#endif

namespace Compression
{
	// Returns the size of the compressed data, or 0 if the compression failed.
	U32 XMemCompress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize);

	// Returns the size of the decompressed data, or 0 if the decompression failed
	U32 XMemDecompress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize, void* pContext);

	void XMemDecompress_Asynch(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize, 
								Scheduler::GroupHandle group = -1, Scheduler::PolicyHandle policy = 0);
#ifdef USE_XMEM_COMPRESSION
	// Get a XMemCompress Context from global list
	XMEMDECOMPRESSION_CONTEXT XMemDecompressGetContext( S32 id );
	XMEMCOMPRESSION_CONTEXT XMemCompressGetContext( S32 id );
#endif

};

#endif
