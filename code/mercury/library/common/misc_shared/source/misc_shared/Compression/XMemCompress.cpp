#include "MiscSharedPch.h"
#include <System/Scheduler/JobQueue.h>
#include "Misc_Shared/Compression/xmemcompress.h"
#include <System/DebugLog.h>
#include "system\perf\perfevent.h"
#include "btl/containers/vector.h"

#ifdef USE_XMEM_COMPRESSION
#include "xcompress.h"
#endif

extern Scheduler::jqModule XMemDecompressModule;

namespace Compression
{

static Debug::Logger& g_Log = Debug::GetLogger("misc_shared.XMemCompress");

#ifdef USE_XMEM_COMPRESSION

U32 XMemCompress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize)
{
    if(pInData == 0 || pOutData == 0)
    {
		LOGFATAL(g_Log, "XMemCompress error.\n");
        return 0;
    }

	XMEMCOMPRESSION_CONTEXT context;
	HRESULT hr = E_FAIL;

	if (FAILED(hr = XMemCreateCompressionContext(XMEMCODEC_DEFAULT, 0, 0, &context)))
	{
		LOGFATAL(g_Log, "XMemCreateCompressionContext failed.\n");
		return 0;
	}

	SIZE_T compressedSize = 0;
	hr = ::XMemCompress(context, 0, &compressedSize, pInData, nInSize);
	
	if (hr != XMCDERR_MOREDATA && FAILED(hr))
	{
		LOGFATAL(g_Log, "XMemCompress failed.\n");
		XMemDestroyCompressionContext(context);
		return 0;
	}

	if (nOutSize < compressedSize)
	{
		LOGFATAL(g_Log, "XMemCompress: output buffer too small.\n");
		XMemDestroyCompressionContext(context);
		return 0;
	}

	hr = ::XMemCompress(context, pOutData, &compressedSize, pInData, nInSize);
	XMemDestroyCompressionContext(context);

	if (FAILED(hr))
	{
		LOGFATAL(g_Log, "XMemCompress failed.\n");
		return 0;
	}

//#define TEST_DECOMPRESSION
#ifdef TEST_DECOMPRESSION

	U8* pTest = new U8[nInSize];
	XMemDecompress(pOutData, compressedSize, pTest, nInSize);

	for (U32 i = 0; i < nInSize; i++)
	{
		if (pTest[i] != pInData[i])
		{
			LOGFATAL(g_Log, "XMemCompress failed.\n");
		}
	}

	delete [] pTest;

#endif

	return compressedSize;
}

struct DecompressionContext
{
	DecompressionContext() : context(0) {}
	XMEMDECOMPRESSION_CONTEXT context;
};

Btl::Vector<DecompressionContext, Btl::VectorParameters_Array<6> > g_decompressionContexts;

struct CompressionContext
{
	CompressionContext() : context(0) {}
	XMEMCOMPRESSION_CONTEXT context;
};

Btl::Vector<CompressionContext, Btl::VectorParameters_Array<6> > g_compressionContexts;

U32 XMemDecompress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize, void* pContext)
{
	static Util::C_String128 pixName;
	PERF_EVENT(pixName.Printf("XMemDecompress %d Kb", nOutSize / 1024));

    HRESULT hr = E_FAIL;

    if(pInData == 0 || pOutData == 0)
    {
		LOGFATAL(g_Log, "XMemCompress: error.\n");
        return 0;
    }

	XMEMDECOMPRESSION_CONTEXT newContext = NULL;
	XMEMDECOMPRESSION_CONTEXT context = NULL;

	if (pContext == 0)
	{
		if (FAILED(hr = XMemCreateDecompressionContext(XMEMCODEC_DEFAULT, NULL, 0, &newContext)))
		{
			LOGFATAL(g_Log, "XMemCreateDecompressionContext failed.\n");
			return 0;
		}

		context = newContext;
	}
	else
	{
		context = pContext;
	}

	SIZE_T decompressedSize = nOutSize;
	hr = ::XMemDecompress(context, pOutData, &decompressedSize, pInData, nInSize);


    // Destroy the decompression context.
	if (newContext)	
	    XMemDestroyDecompressionContext( newContext );

    if (FAILED(hr))
	{
		LOGFATAL(g_Log, "XMemDecompress failed.\n");
		return 0;
	}

	return decompressedSize;
}

struct XMemDecompressParameters
{
	U8* pInData; U32 nInSize; U8* pOutData; U32 nOutSize;
};

void XMemDecompress_Asynch(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize,
						   Scheduler::GroupHandle group, Scheduler::PolicyHandle policy)
{
	assert(pInData && nInSize && pOutData && nOutSize);
	XMemDecompressParameters parameters = {pInData, nInSize, pOutData, nOutSize};
	//XMemDecompress(parameters.pInData, parameters.nInSize, parameters.pOutData,  parameters.nOutSize);
	Scheduler::AddTask(XMemDecompressModule, 0, 0, group, policy, &parameters, sizeof(parameters));
}

void XMemDecompressTask(Scheduler::Task* pTask)
{
	XMEMDECOMPRESSION_CONTEXT context = NULL;
	HRESULT hr = E_FAIL;

	S32 id = Scheduler::GetWorkerID();

	if (id > 0 && id < g_decompressionContexts.Size())
	{
		if (g_decompressionContexts[id].context == 0)
		{
			if (FAILED(hr = XMemCreateDecompressionContext(XMEMCODEC_DEFAULT, NULL, 0, &context)))
			{
				LOGFATAL(g_Log, "XMemCreateDecompressionContext failed.\n");
			}
			else
			{
				g_decompressionContexts[id].context = context;
			}
		}
		else
		{
			context = g_decompressionContexts[id].context;
		}
	}

	XMemDecompressParameters* pParameters = (XMemDecompressParameters*)pTask->ParamData;
	XMemDecompress(pParameters->pInData, pParameters->nInSize, pParameters->pOutData,  pParameters->nOutSize, context);
}

XMEMDECOMPRESSION_CONTEXT XMemDecompressGetContext( S32 id )
{
	HRESULT hr = E_FAIL;

	XMEMDECOMPRESSION_CONTEXT context = NULL;

	if (id >= 0 && id < g_decompressionContexts.Size())
	{
		if (g_decompressionContexts[id].context == 0)
		{
			if (FAILED(hr = XMemCreateDecompressionContext(XMEMCODEC_DEFAULT, NULL, 0, &context)))
			{
				LOGFATAL(g_Log, "XMemCreateDecompressionContext failed.\n");
			}
			else
			{
				g_decompressionContexts[id].context = context;
			}
		}
		else
		{
			context = g_decompressionContexts[id].context;
		}
	}

	return context;
}

XMEMCOMPRESSION_CONTEXT XMemCompressGetContext( S32 id )
{
	HRESULT hr = E_FAIL;

	XMEMCOMPRESSION_CONTEXT context = NULL;

	if (id >= 0 && id < g_compressionContexts.Size())
	{
		if (g_compressionContexts[id].context == 0)
		{
			if (FAILED(hr = XMemCreateCompressionContext(XMEMCODEC_DEFAULT, NULL, 0, &context)))
			{
				LOGFATAL(g_Log, "XMemCreateCompressionContext failed.\n");
			}
			else
			{
				g_compressionContexts[id].context = context;
			}
		}
		else
		{
			context = g_compressionContexts[id].context;
		}
	}

	return context;
}

#else

U32 XMemCompress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize)
{
	assert(!"Not Supported");
	return 0;
}

U32 XMemDecompress(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize)
{
	assert(!"Not Supported");
	return 0;
}

void XMemDecompress_Asynch(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize, Scheduler::GroupHandle group, Scheduler::PolicyHandle policy)
{
	assert(!"Not Supported");
}

void XMemDecompressTask(Scheduler::Task* pTask)
{
	assert(!"Not Supported");
}

#endif

}

JQ_DECLARE_PPU_MODULE(XMemDecompressModule, Compression::XMemDecompressTask);
