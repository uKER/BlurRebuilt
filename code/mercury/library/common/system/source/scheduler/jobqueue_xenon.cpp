#include "SystemPch.h"
#include "system/scheduler/jobqueue.h"
#include <stdio.h>
#include <system/perf.h>
#include <system/time/Time.h>

namespace Scheduler
{

HANDLE g_WorkerAlert[8];
bool jqStopSignal = false;

extern TaskPool jqPool;

extern int jqNWorkers;
extern jqWorker* jqWorkers;

extern int jqMaxWorkers;

jqWorker jqMainThreadWorker;

int jqPoolLock = 0;

void InitWorker(jqWorker* Worker);
void ShutdownWorker(jqWorker* Worker);

void _Init()
{
	jqPoolLock = 0;

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	jqMaxWorkers = si.dwNumberOfProcessors - 1;

	for (int i=0; i<8; i++)
	{
		Util::C_String sName;
		sName.Printf("jqAlert%d", i);
		g_WorkerAlert[i] = CreateEvent(NULL, FALSE, FALSE, sName.Chars());
	}
}

void _Shutdown()
{
	for (int i=0; i<8; i++)
		CloseHandle(g_WorkerAlert[i]);
}

DWORD WINAPI jqWorkerThread(void* _this);

static inline int _jqGetNextBitSet(int& last, U32 mask)
{
	last++;
	while( (last < 32) && ((mask & (1 << last)) == 0) )
		last++;
	
	return last;
}

void _Start(U32 processorsMask)
{
	GenerateWorkerPolicyOrder(&jqMainThreadWorker.PolicyOrder, 0);

	// Allocate and start the worker threads.
	int curProcessor = -1;
	for ( int i = 0; i < jqNWorkers; i++ )
	{
		char ThreadName[256];
		//	On PC we don't have any way to set threads to cores so just make jqNWorkers threads and let the
		//	OS distribute them
		sprintf(ThreadName, "JQ Worker %d", i);
		_jqGetNextBitSet(curProcessor, processorsMask);
		assert(curProcessor < 32); // bit twiddling error
		jqWorkers[i].Processor = curProcessor;
		GenerateWorkerPolicyOrder(&jqWorkers[i].PolicyOrder, i+1);
		jqWorkers[i].Thread = (HANDLE)tlCreateThread(jqWorkerThread, (INTPTR)&jqWorkers[i], 0, 65536, CREATE_SUSPENDED, ThreadName);
		ResumeThread(jqWorkers[i].Thread);
	}

	InitWorker(&jqMainThreadWorker);
	jqMainThreadWorker.Processor = JQ_CORE_0;
	tlSetCurrentThreadName("Game Master");
}

void _jqStop()
{
	// Notify threads to stop by keeping the event raised permanently.
	jqStopSignal = true;

	for (int i = 0; i < jqNWorkers; i++)
	{
		do
		{
			jqAlertWorker(jqWorkers[i].Processor);
		}
		while (	WaitForSingleObject(jqWorkers[i].Thread, 10) != WAIT_OBJECT_0);
		CloseHandle(jqWorkers[i].Thread);
	}

	jqStopSignal = false;

	ShutdownWorker(&jqMainThreadWorker);

	for (int i = 0; i < jqNWorkers; i++)
		ShutdownWorker(&jqWorkers[i]);

	jqNWorkers = 0;
	BizAlignedFree(jqWorkers);
	jqWorkers = NULL;
}

void _AddTask(const Task* Batch, int iPolicy)
{
}

void jqAlertWorker(int iWorkerID)
{
	SetEvent(g_WorkerAlert[iWorkerID]);
}

void jqLockBatchPoolInternal()
{
	while(!MT::AtomicCompareAndSwap((U32*)&jqPoolLock, 1, 0 )) 
		MT::YieldThread();
	MT::MemoryFence();
}

void jqUnlockBatchPoolInternal()
{
	MT::MemoryFence();
	while(!MT::AtomicCompareAndSwap((U32*)&jqPoolLock, 0, 1)) {}
}

void jqWorkerLoop(jqWorker* Worker, int GroupID, int iWorkerID);

void Flush( GroupHandle GroupID )
{
	Flush(GroupID, true);
}

void Flush(int GroupID, bool bRunJobs, U32 _timeout)
{
	PERF_EVENT("Scheduler.Flush");

	// flush dependencies
	if (GroupID != JQ_GROUP_NONE)
	{
		TaskGroup *pGroup = &jqPool.BatchGroupPool[GroupID];
		for (int i=0; i<pGroup->NDependencies; i++)
		{
			if (pGroup->Dependencies[i] != JQ_GROUP_SPURWLOCK &&
				pGroup->Dependencies[i] != JQ_GROUP_LABEL)
				Flush(pGroup->Dependencies[i]);
		}
	}

	volatile int* BatchCount;
	volatile int* ExecutingBatchCount;
	MT::SpuEvent *pEvent;
	bool bUsingEvent;
	U64 timeout = Time::GetPerformanceCounter() + (U64)_timeout*1000;
	if (GroupID == JQ_GROUP_NONE)
	{
		BatchCount          = &jqPool.QueuedBatchCount;
		ExecutingBatchCount = &jqPool.ExecutingBatchCount;
		pEvent				= jqPool.pTaskFinishedEvent;
		bUsingEvent			= false;
	}
	else
	{
		BatchCount          = &jqPool.BatchGroupPool[GroupID].QueuedBatchCount;
		ExecutingBatchCount = &jqPool.BatchGroupPool[GroupID].ExecutingBatchCount;
		pEvent				= &jqPool.BatchGroupPool[GroupID].taskFinishedEvent;
		bUsingEvent			= jqPool.BatchGroupPool[GroupID].bUsingEvent!=0;
	}

	for (;;) 
	{
		// Force BatchCount to be read before ExecutingBatchCount using a memory fence.
		// Without this, ec's old value can be read before bc's new value, because of processor memory read reordering.
		int bc = *BatchCount;
		MT::MemoryFence();
		int ec = *ExecutingBatchCount;

		// Check to see if flush is satisfied.
		if((bc + ec) == 0)
			break;

		// if there are jobs left to do, process as many of them as possible on this CPU by running the worker loop
		if (bc && bRunJobs)
			jqWorkerLoop(&jqMainThreadWorker, GroupID, -1);

		if (_timeout != 0xfffffff && Time::GetPerformanceCounter() >= timeout)
			break;

		// wait for another CPU to finish processing jobs
		if (bUsingEvent)
			pEvent->Wait();
		else
			MT::YieldThread();
	}

//	tlEndNamedEvent();
}

} // namespace Scheduler
