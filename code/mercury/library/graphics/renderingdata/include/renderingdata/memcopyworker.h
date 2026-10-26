#ifndef RD_MEMCOPYWORKER_H
#define RD_MEMCOPYWORKER_H
#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU
#include "btl/containers/vector.h"
#endif

#include "system/scheduler.h"

class ALIGN16 MemCopyWorker
{
public:
	struct ALIGN16 AsynchCopy
	{
		void* pDst;
		const void* pSrc;
		S32 size;
	};

	const AsynchCopy* m_pItems;
	S32 m_count;

	void Process() const;

#ifndef PLATFORM_SPU
	Btl::Vector<AsynchCopy> m_cpuCopyList;
	Btl::Vector<AsynchCopy> m_gpuCopyList;
#endif
};

void MemCopyTask(Scheduler::Task* pTask);

#endif
