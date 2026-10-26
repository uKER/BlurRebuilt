#include "stdafx.h"
#include "renderingdata/memcopyworker.h"
#include "system\perf\perfevent.h"
#include "Core/Utility/MemCpy.h"
#include <system/scheduler/jobqueue.h>

#if PLATFORM_PS3

void CopyGPUData(void* pDst, const void* pSrc, S32 size)
{
	FastMemCpy(pDst, pSrc, size);

	// flush the gpu cache
	for(int offset=0; offset < size; offset+=128)
		__dcbf((U8*)pDst + offset);
}

#elif PLATFORM_SPU

void CopyGPUData(void* pDst, const void* pSrc, S32 size)
{
	Spu::MemBaseMarker marker;

	void* p = Spu::Alloc(128);

	for(int offset=0; offset < size; offset+=128)
	{
		Spu::Fetch(p, (U8*)pSrc + offset, 128);
		Spu::Store((U8*)pDst + offset, p, 128);
	}
}

#else

void CopyGPUData(void* pDst, const void* pSrc, S32 size)
{
	assert(0);
}

#endif

void MemCopyWorker::Process() const
{
	PERF_EVENT("MemCopyWorker");

	const AsynchCopy* pItems = (const AsynchCopy*)Spu::FetchAllocAsync(m_pItems, sizeof(AsynchCopy) * m_count, 0);
	Spu::Wait(0);

	for (int i = 0; i < m_count; i++)
	{
		void * pDst = pItems[i].pDst;
		const void* pSrc = pItems[i].pSrc;
		S32 size = pItems[i].size;
		//assert(*((U32*)pDst) == 0x0f0f0f0f);

		CopyGPUData(pDst, pSrc, size);
	}
}

void MemCopyTask(Scheduler::Task* pTask)
{
	const MemCopyWorker* p = (const MemCopyWorker*)Spu::FetchAllocAsync(*(void**)pTask->ParamData, sizeof(MemCopyWorker), 0);	// [Blur2PC][x64] a pointer-sized parameter
	Spu::Wait(0);
	p->Process();
}

//JQ_DECLARE_PPU_MODULE(MemCopyModule, MemCopyTask)
JQ_DECLARE_SPU_MODULE(MemCopyModule, MemCopyTask, memcopyworker, "Graphics.RenderingData.Memcopy");
