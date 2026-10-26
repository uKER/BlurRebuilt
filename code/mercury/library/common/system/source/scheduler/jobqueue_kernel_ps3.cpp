#include <core\Core.h>
#include <core\threads\thread.h>
#include <system\DebugLog.h>
#include <system\Perf\Profiler.h>
#include "system/scheduler/jobqueue.h"

#define ENABLE_SPURS_TRACE

#if TL_PLAT_PS3_SPU

#include <cell/spurs/policy_module.h>

namespace Scheduler
{

TaskPool jqPool;

typedef void (*TaskFunction)();

int jqSPUID = 0;

void* CachedCode = NULL;
int CachedCodeSize = 0;
Task* jqCurBatch = NULL;
extern "C" {
extern char __stack_limit[];
};

U32 g_pszFlushString = 0;

CellSpursTracePacket jqTracePacket;

vector unsigned char jqSpursTraceWorkerPacket = (vector unsigned char)
	{ CELL_SPURS_TRACE_TAG_START, 0, 0, 0, 0, 0, 0, 0,
	  'J', 'Q', 'K', 'R', 0, 1, ((JQ_MODULE_BASE >> 2) >> 8), (JQ_MODULE_BASE >> 2) & 0xff };

void jqSpursStartWorkerTrace()
{
	cellSpursModulePutTrace((CellSpursTracePacket*)&jqSpursTraceWorkerPacket, 31);
	cellDmaWaitTagStatusAll(1<<31);
}

void jqSpursStopWorkerTrace()
{
	*(vector unsigned char*)&jqTracePacket = (vector unsigned char)( 0 );
	jqTracePacket.header.tag = CELL_SPURS_TRACE_TAG_STOP;
	
	qword insn = si_roti(*(qword*)0xa00, 7);
	qword pattern = (qword)(vec_uchar16){0,1,4,5,8,9,12,13,0,1,4,5,8,9,12,13};
	uint64_t guid = si_to_ullong(si_shufb(insn, insn, pattern));
	jqTracePacket.data.guid = guid;
	
	cellSpursModulePutTrace(&jqTracePacket, 31);
	cellDmaWaitTagStatusAll(1<<31);
}

vector unsigned char jqSpursTraceBatchPacket = (vector unsigned char)
	{ CELL_SPURS_TRACE_TAG_START, 0, 0, 0, 0, 0, 0, 0,
	  'J', 'Q', 'B', 'T', 0, 2, ((JQ_MODULE_BASE >> 2) >> 8), (JQ_MODULE_BASE >> 2) & 0xff };
	  
void jqSpursStartBatchTrace()
{
	cellSpursModulePutTrace((CellSpursTracePacket*)&jqSpursTraceBatchPacket, 31);
	cellDmaWaitTagStatusAll(1<<31);
}

void jqSpursStopBatchTrace()
{
	*(vector unsigned char*)&jqTracePacket = (vector unsigned char)( 0 );
	jqTracePacket.header.tag = CELL_SPURS_TRACE_TAG_STOP;
	
	qword insn = si_roti(*(qword*)JQ_MODULE_BASE, 7);
	qword pattern = (qword)(vec_uchar16){0,1,4,5,8,9,12,13,0,1,4,5,8,9,12,13};
	uint64_t guid = si_to_ullong(si_shufb(insn, insn, pattern));
	jqTracePacket.data.guid = guid;
	
	cellSpursModulePutTrace(&jqTracePacket, 31);	
	cellDmaWaitTagStatusAll(1<<31);
}

void jqExecuteBatch(Task* Batch)
{
	// Update SPURS ready count.
	cellSpursModuleReadyCountAdd(jqKernelContext, -1);
	_jqThisWorker->pLogger = 0;

	_jqThisWorker->MemBase = JQ_MODULE_BASE;
	_jqThisWorker->MemLimit = (int)__stack_limit;
	_jqThisWorker->pLogger = Batch->_Batch.pLogger;
	_jqThisWorker->PPUModuleAddress = Batch->_Batch.Code;
	_jqThisWorker->bBreakInDebugger = Batch->_Batch.iBreakIntoDebugger;
	g_pszFlushString = Batch->_Batch.pszFlushString;

	// DMA up the ELF image.
	if (Batch->_Batch.Code != CachedCode || Batch->_Batch.CodeSize != (unsigned)CachedCodeSize)
	{
		Spu::FetchAlloc((void*)Batch->_Batch.Code, tl_align(Batch->_Batch.CodeSize, 16));
		CachedCode = Batch->_Batch.Code;
		CachedCodeSize = Batch->_Batch.CodeSize;
	}
	else
		Spu::Alloc(tl_align(Batch->_Batch.CodeSize, 16));

	__builtin_memcpy((Task*)JQ_KERNEL_BATCHES, Batch, sizeof(Task));

	// Wrap the excecution in a SPURS trace.
#ifdef ENABLE_SPURS_TRACE
	jqSpursStartBatchTrace();
#endif

	// Execute the batch code.
	jqCurBatch = Batch;
	TaskFunction fn = (TaskFunction)(JQ_MODULE_BASE);
	fn();
	jqCurBatch = NULL;
	Perf::FlushEventBuffer();
	Spu::WaitMultiple(0xffffffff);

#ifdef ENABLE_SPURS_TRACE
	jqSpursStopBatchTrace();
#endif

#ifdef CFG_DEBUG
	// check the code hasn't been overwritten
	// (setting breakpoints in the SPU code will cause false positives)
	U8 buffer[16];
	for (int i=0; i<CachedCodeSize-0x200; i+=16)
	{
		Spu::Fetch(buffer, ((U8*)CachedCode)+i, 16);
		for (int x=0; x<16; x++)
		{
			if (buffer[x] != ((U8*)JQ_MODULE_BASE)[i+x])
			{
				DebugBreak();
				i=1024*256;
				break;
			}
		}
	}
#endif
}

bool jqSafeToPopBatch(jqWorker* Worker, jqPriority Priority, bool IsLocal)
{
	return true;
};

uintptr_t jqKernelContext;

bool jqWorkerSleep(jqWorker*, int iWorkerID)
{
	if (cellSpursModulePoll(jqKernelContext))
	{
		//tlPrintf("SPU %d worker thread preempted.\n", jqSPUID);
		return false;
	}

	return true;
}

bool jqCanBatchExecute( Task* Batch, jqWorker* Worker )
{
	return Batch->_Batch.Code != NULL;
}

extern "C" void jq_main(uintptr_t pKernelContext, uint64_t argp);

int g_iWorkerNumber = 0;

extern "C" extern char _end[];

void jq_main(uintptr_t pKernelContext, uint64_t argp)
{
	if ((U32)_end > JQ_KERNEL_WORKER) DebugBreak();	// jobqueue kernel code size is too big

#ifdef ENABLE_SPURS_TRACE
	jqSpursStartWorkerTrace();
#endif

	jqKernelContext = pKernelContext;

	jqSPUID = cellSpursModuleGetSpuId(jqKernelContext);

	// Need to preinitialize the JQ worker MemBase so jqFetch doesn't assert about stack overflow.
	_jqThisWorker->MemBase = JQ_MODULE_BASE;
	_jqThisWorker->MemLimit = (int)__stack_limit;
	_jqThisWorker->pLogger = 0;

	jqWorker* Worker = (jqWorker*)JQ_KERNEL_WORKER;
	Spu::Fetch(Worker, ((jqWorker*)argp)+jqSPUID, sizeof(jqWorker));

	Spu::Fetch(&jqPool, Worker->PoolPtr, sizeof(TaskPool));

	_jqThisWorker->MemBase = JQ_MODULE_BASE;
	_jqThisWorker->MemLimit = (int)__stack_limit;
	_jqThisWorker->pLogger = 0;

	CachedCode = NULL;
	CachedCodeSize = 0;

	//tlPrintf("SPU %d worker thread started.\n", jqSPUID);

	g_iWorkerNumber = 2+jqSPUID;
	jqWorkerLoop(Worker, JQ_GROUP_NONE, g_iWorkerNumber);

#ifdef ENABLE_SPURS_TRACE
	jqSpursStopWorkerTrace();
#endif

	//tlPrintf("SPU %d worker thread stopped.\n", jqSPUID);
}

} // namespace Scheduler

#endif

#if TL_PLAT_PS3

#include <stdio.h>
#include <sys/event.h>

#include <sys/timer.h>

#include <cell/spurs.h>

#define SPURS_CHECK(v) { int ret = v; if (ret!=CELL_OK && ret!=ETIMEDOUT) { LOGFATAL( Scheduler::g_Log, #v " failed: 0x%08x\n", ret); DebugBreak(); } }

namespace Scheduler
{

extern bool jqStopSignal;
extern sys_lwmutex_t jqMutexID[8];
extern sys_lwcond_t jqCondID[8];

extern int jqSleepingWorkers;
extern int jqWorkerAwakeCount;
extern U32 g_iSPUWorkerAlert[6];

extern CellSpurs* jqSpurs;
extern CellSpursWorkloadId jqWorkloadId;

void jqExecuteBatch(Task* Batch)
{
	// Update SPURS ready count.
	U32 OldReadyCount;
	cellSpursReadyCountAdd(jqSpurs, jqWorkloadId, &OldReadyCount, -1);

	Perf::BeginNamedEvent(Batch->Module->Name);
	jqCurBatch = Batch;
	Batch->Module->PPUCode( Batch );
	jqCurBatch = NULL;
	Perf::EndNamedEvent();
}

bool jqSafeToPopBatch(jqWorker* Worker, jqPriority Priority, bool IsLocal)
{
	return true;
};

bool jqWorkerSleep(jqWorker*, int iWorkerID)
{
	SPURS_CHECK(sys_lwmutex_lock(&jqMutexID[iWorkerID], 100000));
	SPURS_CHECK(sys_lwcond_wait(&jqCondID[iWorkerID], 100000));
	SPURS_CHECK(sys_lwmutex_unlock(&jqMutexID[iWorkerID]));

	MT::MemoryFence();

	return jqStopSignal == 0;
}

bool jqCanBatchExecute( Task* Batch, jqWorker* Worker )
{
	return Batch->Module->PPUCode != NULL;
}

void jqWorkerLoop(jqWorker* Worker, int GroupID, int iWorkerID);

__thread int g_iWorkerNumber = 0;

int GetWorkerID()
{
	return g_iWorkerNumber;
}

void jqSecondaryThreadEntry(uint64_t val)
{
	// this loop stops PS3 SDK270 from crashing the game ;P
	for (int i=0; i<2; i++)
		MT::Thread::Sleep(5000);

	g_iWorkerNumber = 1;

	jqWorkerLoop(&jqWorkers[0], JQ_GROUP_NONE, g_iWorkerNumber);
	
	sys_ppu_thread_exit(0);
}

} // namespace Scheduler

#endif

