#include "MiscSharedPch.h"
#include <Misc_Shared/Compression/InflateTask.h>
#include <System/Scheduler.h>
#include <System/DebugLog.h>
#include "system\perf\perfevent.h"

#if PLATFORM_SPU
	#include <cell/atomic.h>
	#include <cell/dma.h>
	#include <../../../../Vendor/Zlib/Current_Edge/edgezlib_spu.h>
	#include "../../../../Vendor/Zlib/Current_Mercury/Zlib.h"
#else
	#include "../../../../Vendor/Zlib/Current_Mercury/Zlib.h"
#endif

namespace Compression
{
	
#ifndef PLATFORM_SPU
static Debug::Logger& g_ZlibLog = Debug::GetLogger("vendor.zlib");
#endif

U32 InflateRawData(U8* pOutData, U32 nOutSize, U8* pInData, U32 nInSize)
{
	z_stream compressionStream;
	compressionStream.next_in = pInData;
	compressionStream.avail_in = nInSize;
	compressionStream.next_out = pOutData;
	compressionStream.avail_out = nOutSize;
	compressionStream.zalloc = Z_NULL;
	compressionStream.zfree = Z_NULL;
	compressionStream.opaque = Z_NULL;

#if PLATFORM_SPU
	PERF_EVENT("InflateRawData");
#else
	static char temp_name[128];
	_snprintf(temp_name, 128, "Inflate %d Kb", compressionStream.avail_out / 1024);
	PERF_EVENT(temp_name);
#endif

	{
		S32 nResult = inflateInit(&compressionStream);

		if (nResult != Z_OK)
		{
			LOGFATAL(g_ZlibLog, "Zlib did not return success for inflateInit!\n");
			if (compressionStream.msg)
			{
				LOGFATAL(g_ZlibLog, "%s", compressionStream.msg);
			}
			return nResult;
		}
	}

	{
		S32 nRetCode = inflate(&compressionStream, Z_FINISH);

		if (compressionStream.avail_in != 0)
		{
			LOGFATAL(g_ZlibLog, "Zlib did not return success for inflate!\n");
			if (compressionStream.msg)
			{
				LOGFATAL(g_ZlibLog, "%s", compressionStream.msg);
			}
			return nRetCode;
		}
	}

	inflateEnd(&compressionStream);

	return 0;
}


#if PLATFORM_SPU

void InflateTask(Scheduler::Task* pTask)
{
	const U32 nInputBufferMaxSize	= 64 * 1024;
	const U32 nOutputBufferMaxSize	= 64 * 1024;
	const U32 nDmaTagId				= 0;

	U8 pInflateInputBuffer[64 * 1024 + 128] __attribute__((aligned( 128 )));
	U8 pInflateOutputBuffer[64 * 1024 + 128] __attribute__((aligned( 128 )));

	C_InflateParameters* pParameters = (C_InflateParameters*)pTask->ParamData;

	const U32 compressedAddress		= (U32) pParameters->pInData;
	const U32 uncompressedAddress	= (U32) pParameters->pOutData;
	const U32 compressedSize		= (U32) pParameters->nInSize;
	const U32 uncompressedSize		= (U32) pParameters->nOutSize;

	if ((compressedSize > nInputBufferMaxSize) || (uncompressedSize > nOutputBufferMaxSize))
	{
		// Can't use Edge ZLib as it is incompatible with Mercury ZLib.
		//edgeZlibFetchAndInflateRawData(uncompressedAddress, uncompressedSize, compressedAddress, compressedSize, nDmaTagId, pInflateInputBuffer, sizeof(pInflateInputBuffer), pInflateOutputBuffer, sizeof(pInflateOutputBuffer));
//		LOGDEBUG(g_ZlibLog, "Decompression failed.");
		return;
	}
	else
	{
		U32	compressedAddressRoundDown	= compressedAddress & ~0x7F;
		U32	inputAlignOffset			= compressedAddress & 0x7F;
		U32	compressedSizeRoundUp		= compressedSize + inputAlignOffset;
			compressedSizeRoundUp		= (compressedSizeRoundUp + 0x7F) & ~0x7F;

		cellDmaLargeGet(pInflateInputBuffer, compressedAddressRoundDown, compressedSizeRoundUp, nDmaTagId, 0, 0);
		cellDmaWaitTagStatusAll(1 << nDmaTagId);

		U32 outputAlignOffset = uncompressedAddress & 0x7F;

		U8* pLsOutputBuffer = &pInflateOutputBuffer[outputAlignOffset];
		if (InflateRawData(pLsOutputBuffer, uncompressedSize, &pInflateInputBuffer[inputAlignOffset], compressedSize))
		{
//			LOGDEBUG(g_ZlibLog, "Decompression failed.");
			return;
		}

		_edgeZlibUnalignedLargeDmaPut(pLsOutputBuffer, uncompressedAddress, uncompressedSize, nDmaTagId);
		cellDmaWaitTagStatusAll(1 << nDmaTagId);
	}
}



#else

void InflateTask(Scheduler::Task* pTask)
{	
	C_InflateParameters* pParameters = (C_InflateParameters*)pTask->ParamData;

	InflateRawData(pParameters->pOutData, pParameters->nOutSize, pParameters->pInData, pParameters->nInSize);
}

#endif

} // namespace Compression

//JQ_DECLARE_PPU_MODULE(InflateModule, Compression::InflateTask);
JQ_DECLARE_DUAL_MODULE(InflateModule, Compression::InflateTask, InflateSpu, "vendor.zlib");
