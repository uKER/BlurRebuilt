#include "MiscSharedPch.h"
#include "Misc_Shared/Compression/Deflate.h"
#include <System/DebugLog.h>
#include <System/Scheduler.h>
#include "../../../../Vendor/Zlib/Current_Mercury/Zlib.h"

JQ_DECLARE_PPU_MODULE(DeflateModule, Compression::DeflateTask);

#ifndef PLATFORM_SPU
static Debug::Logger& g_ZlibLog = Debug::GetLogger("vendor.zlib");
#endif

namespace Compression
{

C_Deflate::C_Deflate() : m_hJob(Scheduler::JQ_GROUP_NONE)
{
	m_bIsRunning = false;
}

C_Deflate::~C_Deflate()
{
}

bool C_Deflate::Create()
{
	m_hJob = Scheduler::CreateGroup();

	return true;
}

void C_Deflate::Destroy()
{
	Scheduler::DestroyGroup(m_hJob);
	m_hJob = Scheduler::JQ_GROUP_NONE;
}

void C_Deflate::Start(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize)
{
	if (m_bIsRunning)
		return;

	m_Parameters.pInData = pInData;
	m_Parameters.nInSize = nInSize;
	m_Parameters.pOutData = pOutData;
	m_Parameters.nOutSize = nOutSize;

	Scheduler::AddTask(DeflateModule, 0, &m_Parameters, m_hJob, m_hPolicy, 0, 0);

	m_bIsRunning = true;
}

void C_Deflate::Finish(U32& nOutSize)
{
	if (m_bIsRunning)
	{
		Scheduler::Flush(m_hJob);
		m_bIsRunning = false;
	}

	nOutSize = m_Parameters.totalOut;
}

void DeflateTask(Scheduler::Task* pTask)
{
	C_DeflateParameters* pParameters = (C_DeflateParameters*)pTask->Output;

	z_stream compressionStream;

	compressionStream.next_in = pParameters->pInData;
	compressionStream.avail_in = pParameters->nInSize;
	compressionStream.next_out = pParameters->pOutData;
	compressionStream.avail_out = pParameters->nOutSize;
	compressionStream.zalloc = Z_NULL;
	compressionStream.zfree = Z_NULL;
	compressionStream.opaque = Z_NULL;

	pParameters->totalOut = 0;

	S32 nResult = deflateInit(&compressionStream, Z_DEFAULT_COMPRESSION);
	if (nResult != Z_OK)
	{
		#ifndef PLATFORM_SPU
		LOGFATAL(g_ZlibLog, "Zlib did not return success for compression initialisation!\n");
		if (compressionStream.msg)
		{
			LOGFATAL(g_ZlibLog, "%s", compressionStream.msg);
		}
		#endif
		return;
	}

	U8	pBatchBuffer[32768];
	U8*	pOutData	= compressionStream.next_out;
	U32	nOutSize	= compressionStream.avail_out;

	while (true)
	{
		const U32 nBufferSize = (nOutSize < sizeof(pBatchBuffer)) ? nOutSize : sizeof(pBatchBuffer);

		compressionStream.next_out = pBatchBuffer;
		compressionStream.avail_out = nBufferSize;

		const S32 nResult = deflate(&compressionStream, Z_FINISH);
		if (nResult < 0)
		{
			#ifndef PLATFORM_SPU
			LOGFATAL(g_ZlibLog, "Zlib did not return success for compression!\n");
			if (compressionStream.msg)
			{
				LOGFATAL(g_ZlibLog, "%s", compressionStream.msg);
			}
			#endif
			return;
		}
		const S32 nBytesToCopy = nBufferSize - compressionStream.avail_out;
		memcpy(pOutData, pBatchBuffer, nBytesToCopy);

		pOutData += nBytesToCopy;

		if (nResult == Z_STREAM_END)
			break;
		if ((nOutSize - compressionStream.total_out) <= 0)
			break;
	}

	deflateEnd(&compressionStream);
	pParameters->totalOut = compressionStream.total_out;
}

}; // namespace Compression
