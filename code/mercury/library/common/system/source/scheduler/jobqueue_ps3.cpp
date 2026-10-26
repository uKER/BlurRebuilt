#include <core/Core.h>

#include <system/debug/Variables.h>

#include <system/DebugLog.h>
#include <system/Perf/Profiler.h>
#include "system/scheduler/jobqueue.h"

#include <cell/spurs.h>
#include <cell/sheap.h>
#undef assert
#include <core/utility/CoreDebug.h>

#include <sys/spu_utility.h>
#include <spu_printf.h>

#include <sys/event.h>
#include <sys/timer.h>

#include <cell/cell_fs.h>
#include <sys/paths.h>

#include <cell/spurs/lv2_event_queue.h>
#include <sys/gpio.h>

#include <stdlib.h>
#include <stdio.h>

#include <libsn.h>

#include <system/time/Time.h>

namespace Scheduler
{

CellSpurs* jqSpurs = NULL;
CellSpursWorkloadId jqWorkloadId;

// By default the JQ SPURS policy module is assigned medium priority on all SPUs.
uint8_t jqPriorityTable[8] = {7, 7, 7, 7, 7, 7, 0, 0};

jqWorker jqMainThreadWorker;

int jqSecondaryThreadStackSize = 256 * 1024;
int jqSecondaryThreadPriority = 1010;
extern U32 jqProcessorsMask;

sys_ppu_thread_t jqSecondaryThread = 0;

bool jqStopSignal = false;
sys_lwcond_t jqCondID[8];
sys_lwmutex_t jqMutexID[8];
U32 g_iFirstSpuWorker;
int jqPoolLock = 0;

void* TaskDataHeap = NULL;

#if !CFG_SPEED
#define MAX_MODULES 64
Debug::Bool *g_BreakIntoDebuggerSwitches[MAX_MODULES] = {0};
#endif

// Dummy SPU module fields for PPU-only modules.
extern "C" {
	char* _binary_none_spu_elf_bin_start = 0;
	char* _binary_none_spu_elf_bin_size = 0;
}

void none(Task*)
{
}

#define SPURS_CHECK(v) { int ret = v; if (ret!=CELL_OK) { LOGFATAL( Scheduler::g_Log, #v " failed: 0x%08x\n", ret); DebugBreak(); } }

extern TaskPool jqPool;

extern int jqNWorkers;
extern jqWorker* jqWorkers;

extern int jqMaxWorkers;

bool jqCanBatchExecute( Task* Batch, jqWorker* Worker );
void jqWorkerLoop(jqWorker* Worker, int GroupID, int iWorkerID);
void InitWorker(jqWorker* Worker);
void ShutdownWorker(jqWorker* Worker);

#include <sys/spu_thread.h>
#include <cell/spurs/types.h>

#define	SPU_PORT_PRINTF	1
#define	JQ_STOP_PRINT_SERVICE 0xFEE1DEAD

sys_ppu_thread_t jqPrintServiceThread;
static sys_event_queue_t jqPrintServiceEventQueue;
static sys_event_port_t	jqPrintServiceTerminatePort;

void jqPrintServiceHandler(uint64_t)
{
	for (;;) 
	{
		// Wait for an event.
		sys_event_t	event;
		sys_event_queue_receive(jqPrintServiceEventQueue, &event, 0);

		// If the event is from the PPU, stop the thread.
		if (event.source == JQ_STOP_PRINT_SERVICE) 
			sys_ppu_thread_exit(0);

		sys_spu_thread_t spu = event.data1;

		// Get the message.
		char work[1024];
		char *psz = work;
		int sret = spu_thread_snprintf(work, sizeof(work), spu, event.data3);
		// if the message was 0xff, get the PPU address of the static string
		if (psz[0] == (char)0xff)
		{
			U64 tmp;
			sys_spu_thread_read_ls(spu, JQ_KERNEL_WORKER+tl_offsetof(jqWorker,ppustringdata), (U64*)&tmp, 8);
			sprintf(work, (char*)tmp, (int)(tmp>>32));
		}

		// get the debug logger for this worker
		uint64_t logvars;
		sys_spu_thread_read_ls(spu, JQ_KERNEL_WORKER+tl_offsetof(jqWorker,pLogger), &logvars, 8);
		Debug::Logger *pLogger = &Debug::GetLogger("SPU");
		int logger = logvars&0xffffffff;
		int level = logvars>>32;

		// jkc- if the job has not come from our scheduler (eg, has come from spurs), we'll end up retrieving an invalid
		// logger pointer and crashing. To prevent this- check level, which if it's come from our scheduler, should always
		// be between 0 and 6
		if (level < 0 || level > 6)
			pLogger = &(Debug::GetRootLogger());
		else if (logger != NULL)
			pLogger = (Debug::Logger*)logger;

		// get the debug level for this message if specified
		DebugLevel::Level debugLevel = DebugLevel::Debug;
		if (level < 6 && level > 0)
			debugLevel = (DebugLevel::Level)level;

		// handle assert messages
		if (level == 6)
		{
			char buffer[512];
			sprintf(buffer, "SPU %d Assert", sret);
			(*::g_pCoreAssertFunction)(buffer, "", psz, 0);
		}
		else
		// submit the message to the logging system
		if (Debug::GetEffectiveLevel(*pLogger) >= debugLevel)
			Debug::_dbgMessageProxy::LogS( *pLogger, debugLevel, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), psz);

		// let the SPU continue
		sys_spu_thread_write_spu_mb(spu, sret);
	}
	sys_ppu_thread_exit(0);
}

void StartPrintService(int prio)
{
	// Create an event queue for the handler thread.
	sys_event_queue_attribute_t	attr;
	sys_event_queue_attribute_initialize(attr);
	SPURS_CHECK(sys_event_queue_create(&jqPrintServiceEventQueue, &attr, SYS_EVENT_QUEUE_LOCAL, 8));

	// Start the listening PPU thread and create a terminating port.
	SPURS_CHECK(sys_ppu_thread_create(&jqPrintServiceThread, 
		jqPrintServiceHandler, (uint64_t)0, prio, 64*1024, SYS_PPU_THREAD_CREATE_JOINABLE, "jqPrintService"));
	SPURS_CHECK(sys_event_port_create(&jqPrintServiceTerminatePort, SYS_EVENT_PORT_LOCAL, JQ_STOP_PRINT_SERVICE));
	SPURS_CHECK(sys_event_port_connect_local(jqPrintServiceTerminatePort, jqPrintServiceEventQueue));

	// Attach the event queue.
	uint8_t	port = SPU_PORT_PRINTF;
	SPURS_CHECK(cellSpursAttachLv2EventQueue(jqSpurs, jqPrintServiceEventQueue, &port, 0));
}

void jqStopPrintService()
{
	SPURS_CHECK(cellSpursDetachLv2EventQueue(jqSpurs, SPU_PORT_PRINTF));

	// Signal the thread to stop and wait for it to exit.
	uint64_t exit_code;
	SPURS_CHECK(sys_event_port_send(jqPrintServiceTerminatePort, 0, 0, 0));
	SPURS_CHECK(sys_ppu_thread_join(jqPrintServiceThread, &exit_code));

	// Disconnect and destroy the terminating port.
	SPURS_CHECK(sys_event_port_disconnect(jqPrintServiceTerminatePort));
	SPURS_CHECK(sys_event_port_destroy(jqPrintServiceTerminatePort));

	// Clean up event queue.
	SPURS_CHECK(sys_event_queue_destroy(jqPrintServiceEventQueue, 0));
}

void jqPolicyModuleExceptionHandler(CellSpurs* spurs, const CellSpursExceptionInfo* exception_info, void* arg)
{
#ifndef CFG_GOLD
	printf("JQ: SPU exception!\n");
#endif
}

extern "C" {
int	cellSpursAttributeEnableSystemWorkload(
			CellSpursAttribute* attr,
			const uint8_t priority[8],
			unsigned int maxSpu,
			const bool preemptionVictim[8]
		);
}

void _Init()
{
	// 4MB profiler buffer
	U64 dip = 0;
	sys_gpio_get(SYS_GPIO_DIP_SWITCH_DEVICE_ID, &dip);

	// make sure the profiler has been initialised
	Perf::ProfilerCommon::Instance();

	//	6 SPUS + secondary PPU thread
	jqMaxWorkers = 6 + 1;

	sys_ppu_thread_t my_ppu_thread_id;
	sys_ppu_thread_get_id(&my_ppu_thread_id);
	int ppu_thr_prio;
	sys_ppu_thread_get_priority(my_ppu_thread_id, &ppu_thr_prio);
	ppu_thr_prio--;

	const U32 numSpus = 6;
	const U32 spuPriority = 250;
	CellSpursAttribute attr;
	SPURS_CHECK(cellSpursAttributeInitialize(&attr, numSpus, spuPriority, ppu_thr_prio, false));

#define JQ_USE_SYSTEM_WORKLOAD

#ifdef JQ_USE_SYSTEM_WORKLOAD
	// Set system workload to use 6th SPU by assigning it a non-zero priority.
	U8 sysWorkloadPriority[8] = { 0, 0, 0, 0, 0, 1, 0, 0 };

	// Identify which SPU(s) should ideally be preempted if system workload is not executed by deadline.
	// NB. this is just a hint and in fact any SPU could be preempted.
	bool sysWorkloadPreemptHint[8] = { false, false, false, false, false, true, false, false };
		
	const U32 sysWorkloadMaxSpus = 1;
	SPURS_CHECK(cellSpursAttributeEnableSystemWorkload(&attr, sysWorkloadPriority, sysWorkloadMaxSpus, sysWorkloadPreemptHint));
#endif

	// Initialise SPURS.
	jqSpurs = (CellSpurs*)BizAlignedMalloc(sizeof(CellSpurs), 128);
	SPURS_CHECK(cellSpursInitializeWithAttribute(jqSpurs, &attr));


	StartPrintService(ppu_thr_prio);
}

void _Shutdown()
{
	jqStopPrintService();
	SPURS_CHECK(cellSpursFinalize(jqSpurs));
	BizAlignedFree(jqSpurs);
#if !CFG_SPEED
	for (int i=0; i<MAX_MODULES; i++)
		delete g_BreakIntoDebuggerSwitches[i];
#endif
}

void jqSecondaryThreadEntry(uint64_t val);

extern "C" {
	extern char JQ_MODULE_START_NAME(jobqueue_kernel_spu)[];
	extern char JQ_MODULE_SIZE_NAME(jobqueue_kernel_spu)[];
};

void _Start(U32 processorsMask)
{
	U32 spuCoresMask = processorsMask & JQ_CORE_ALL_SPU;
	int numSpus = tlCountOnes(spuCoresMask);
	int firstSpuWorker = ((processorsMask & JQ_CORE_PPU_SECONDARY) == JQ_CORE_PPU_SECONDARY) ? 1 : 0;
	g_iFirstSpuWorker = firstSpuWorker;

	for (int i = 0; i < numSpus ; i++)
	{
		jqWorker* Worker = &jqWorkers[i + firstSpuWorker];
		GenerateWorkerPolicyOrder(&Worker->PolicyOrder, i + 2);

		Worker->PoolPtr = &jqPool;

		Worker->BatchDataHeap = TaskDataHeap;

		Worker->MemBase = 0;
		Worker->MemLimit = 0;

		Worker->Processor = i;

		Perf::SetSPUData(&Worker->profilerInfo);

		Worker->bIsDebuggerAttached = snIsDebuggerPresent();
		BIZ_ASSERT(i>=0 && i<6);
	}

	// Module has to be a multiple of 16 bytes, and 128 byte aligned in memory.
	size_t size = tl_align((size_t)JQ_MODULE_SIZE(jobqueue_kernel_spu), 16);
	BIZ_ASSERT(((size_t)JQ_MODULE_START(jobqueue_kernel_spu) & 127) == 0); // Misaligned jobqueue module.

	BIZ_ASSERT(size < 0x2c00);

	// Set up the SPURS workload for the JobQueue.  The number of workers requested by the client simply becomes
	//   the maximum contention (e.g. maximum number of simultaneous policy modules) passed to SPURS.
	CellSpursWorkloadAttribute attr;
	SPURS_CHECK(cellSpursWorkloadAttributeInitialize(&attr, JQ_MODULE_START(jobqueue_kernel_spu), size, (uint64_t)(jqWorkers + firstSpuWorker), jqPriorityTable, 1, numSpus));
	SPURS_CHECK(cellSpursAddWorkloadWithAttribute(jqSpurs, &jqWorkloadId, &attr));

	SPURS_CHECK(cellSpursSetExceptionEventHandler(jqSpurs, jqWorkloadId, jqPolicyModuleExceptionHandler, NULL));
	SPURS_CHECK(cellSpursEnableExceptionEventHandler(jqSpurs, true));

	// Option A: Force the ready count to always remain positive, keeping our workers active.
//	SPURS_CHECK(cellSpursReadyCountStore(jqSpurs, jqWorkloadId, numSpus));
	// Option B: Allow SPURS to page out our workers, but request any available idle time.
	SPURS_CHECK(cellSpursRequestIdleSpu(jqSpurs, jqWorkloadId, numSpus));

	// Initialize the PPU workers.  Both PPU workers can only take batches whose modules have a PPU implementation defined.
	// The main thread worker is used by Flush to execute batches on the PPU.
	InitWorker(&jqMainThreadWorker);
	GenerateWorkerPolicyOrder(&jqMainThreadWorker.PolicyOrder, 0);
	jqMainThreadWorker.Processor = JQ_CORE_PPU_PRIMARY;

	for (int i=0; i<8; i++)
	{
		sys_lwmutex_attribute_t jqMutexAttr;
		sys_lwmutex_attribute_initialize(jqMutexAttr);
		SPURS_CHECK(sys_lwmutex_create(&jqMutexID[i], &jqMutexAttr));

		sys_lwcond_attribute_t jqCondAttr;
		sys_lwcond_attribute_initialize(jqCondAttr);
		SPURS_CHECK(sys_lwcond_create(&jqCondID[i], &jqMutexID[i], &jqCondAttr));
	}

	jqStopSignal = false;

	// The secondary thread worker runs as a PPU thread, operating much like workers on the Xenon.
	if( processorsMask & JQ_CORE_PPU_SECONDARY )
	{
		InitWorker(&jqWorkers[0]);
		jqWorkers[0].Processor = JQ_CORE_PPU_SECONDARY;
		GenerateWorkerPolicyOrder(&jqWorkers[0].PolicyOrder, 1);
		SPURS_CHECK(sys_ppu_thread_create(&jqSecondaryThread, jqSecondaryThreadEntry, (uint64_t)0, jqSecondaryThreadPriority, jqSecondaryThreadStackSize, SYS_PPU_THREAD_CREATE_JOINABLE, "JobQueue PPU Worker"));
#ifdef USE_P3X
		p3x_prof_set_thread_name(jqSecondaryThread, "PPU Secondary");
#endif
	}

#ifdef USE_P3X
	p3x_prof_set_thread_name(tlGetCurrentThreadId(), "PPU Primary");
#endif
}

void _jqStop()
{
	// Tell SPURS to stop the workers.
	SPURS_CHECK(cellSpursReadyCountStore(jqSpurs, jqWorkloadId, 0));
	SPURS_CHECK(cellSpursShutdownWorkload(jqSpurs, jqWorkloadId));
	SPURS_CHECK(cellSpursWaitForWorkloadShutdown(jqSpurs, jqWorkloadId));
	SPURS_CHECK(cellSpursRemoveWorkload(jqSpurs, jqWorkloadId));

	jqStopSignal = true;

	for (int i=0; i<8; i++)
		SPURS_CHECK(sys_lwcond_signal_all(&jqCondID[i]));

	uint64_t res;
	if( jqProcessorsMask & JQ_CORE_PPU_SECONDARY )
		SPURS_CHECK(sys_ppu_thread_join(jqSecondaryThread, &res));

	for (int i=0; i<8; i++)
	{
		SPURS_CHECK(sys_lwcond_destroy(&jqCondID[i]));
		SPURS_CHECK(sys_lwmutex_destroy(&jqMutexID[i]));
	}

	ShutdownWorker(&jqMainThreadWorker);

	for (int i = 0; i < jqNWorkers; i++)
		ShutdownWorker(&jqWorkers[i]);

	jqNWorkers = 0;
	BizAlignedFree( jqWorkers );
	jqWorkers = NULL;
}

#if !CFG_SPEED
bool GetBreakIntoDebuggerSwitchValue(const char *pszName)
{
	Util::C_String str;
	str.Printf("System.Scheduler.Break%s", pszName);

	int i;
	int iFirstEmpty = -1;
	for (i=0; i<MAX_MODULES; i++)
	{
		if (g_BreakIntoDebuggerSwitches[i])
		{
			if (!stricmp(g_BreakIntoDebuggerSwitches[i]->Name(), str.Chars()))
			{
				if (g_BreakIntoDebuggerSwitches[i]->Value())
				{
					g_BreakIntoDebuggerSwitches[i]->SetValue(false);
					return true;
				}
				return false;
			}
		}
		else
			if (iFirstEmpty == -1)
				iFirstEmpty = i;
	}

	if (iFirstEmpty != -1)
		g_BreakIntoDebuggerSwitches[iFirstEmpty] = new Debug::Bool(str.Chars(), false, Debug::Dynamic());

	return false;
}
#endif

void _AddTask(const Task* _Batch, int iPolicy)
{
	Task* Batch = (Task*)_Batch;
	if (Batch->Module->Code == NULL || Batch->Module->Code == &_binary_none_spu_elf_bin_start)
	{
		Batch->_Batch.Code = NULL;
		Batch->_Batch.CodeSize = 0;
		Batch->_Batch.pLogger = NULL;
		assert(Batch->Module->PPUCode != NULL); // No code to execute this batch
	}
	else
	{
		Batch->_Batch.Code = Batch->Module->Code;
		Batch->_Batch.CodeSize = Batch->Module->CodeSize;
		Batch->_Batch.pLogger = Batch->Module->pLogger;

		// Check that all pointers are 16 byte aligned for SPU code.
#ifndef CFG_SPEED
		// Input and Output are occasionally used for things other than data pointers.
		unsigned DataAlign = /*(unsigned)Batch->Input | (unsigned)Batch->Output |*/ (unsigned)Batch->_Batch.Code;
		assert((DataAlign & 15) == 0); // Batch data misalignment.  16 byte alignment is required, 128 byte is optimal.

		// Check that sizes all sizes are 16 byte aligned.
		unsigned DataSizeAlign = Batch->_Batch.CodeSize;
		assert((DataSizeAlign & 15) == 0); // Batch data size misalignment.  16 byte alignment is required, 128 byte is optimal.
#endif
	}

#if !CFG_SPEED
	Batch->_Batch.iBreakIntoDebugger = GetBreakIntoDebuggerSwitchValue(Batch->Module->Name);
#endif
	Batch->_Batch.pszFlushString = (U32)"Scheduler.Flush";
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

void jqAlertWorker(int iWorkerID)
{
	SPURS_CHECK(sys_lwcond_signal_all(&jqCondID[iWorkerID]));
}

void Flush( GroupHandle GroupID )
{
	Flush(GroupID, true);
}

void Flush(int GroupID, bool bRunJobs, U32 _timeout)
{
	Perf::BeginNamedEvent("Scheduler.Flush");

	// flush dependencies
	if (GroupID != JQ_GROUP_NONE)
	{
		TaskGroup *pGroup = &jqPool.BatchGroupPool[GroupID];
		for (int i=0; i<pGroup->NDependencies; i++)
		{
			if (pGroup->Dependencies[i] != JQ_GROUP_SPURWLOCK &&
				pGroup->Dependencies[i] != JQ_GROUP_LABEL)				Flush(pGroup->Dependencies[i]);
		}
	}

	// prepare to get number of jobs left
	volatile int* BatchCount;
	volatile int* ExecutingBatchCount;
	MT::SpuEvent *pEvent;
	bool bUsingEvent;
	U64 timeout = Time::GetPerformanceCounter() + (U64)_timeout;
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

	while (1)
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
			pEvent->Wait(1);
		else
			MT::YieldThread();
	}

	Perf::EndNamedEvent();
}

void jqSetBatchDataHeapSize( size_t Size )
{
	assert(!jqNWorkers); // This function can only be called when the JobQueue is stopped.
	assert(Size > 0); // Invalid batch pool size.

	if (TaskDataHeap)
		BizAlignedFree(TaskDataHeap);
	TaskDataHeap = BizAlignedMalloc(Size, 128);

	cellSheapInitialize(TaskDataHeap, Size, 4/*JQ_TAG_BATCH_DATA_HEAP*/);
}

void* jqAllocBatchData( size_t Size )
{
	return cellSheapAllocate(TaskDataHeap, Size);
}

void jqFreeBatchData( void* Ptr )
{
	cellSheapFree(TaskDataHeap, Ptr);
}

// jqModule
jqModule* jqModule::Head = NULL;

void jqModule::Init(const char* _Name, jqFunctor _PPUCode, const char* _ELFName, void* _Code, unsigned _CodeSize, const char *pszLogger)
{
	Name = _Name;
	PPUCode = _PPUCode;
	ELFName = _ELFName;
	Code = _Code;
	// pull out the actual code size, minus the overlays
	if (_Code)
	{
		CodeSize = ((int*)_Code)[5];
		assert((CodeSize&0xf) == 0);
	}
	else
		CodeSize = _CodeSize;
	CodeReplaced = false;
	Next = Head;
	Head = this;
	if (pszLogger)
		pLogger = (int)&Debug::GetLogger(pszLogger);
	else
		pLogger = NULL;
}

bool jqReloadSPUModule(jqModule* mod)
{
	char work[1024];
	snprintf(work, sizeof(work), "%s/%s.spu.elf.bin", SYS_APP_HOME, mod->ELFName);
	int fd;
	if (cellFsOpen(work, CELL_FS_O_RDONLY, &fd, NULL, 0) != CELL_OK)
	{
		LOGINFO( Scheduler::g_Log, "Failed to find replacement for JobQueue module '%s' in file '%s'.\n", mod->Name, work);
		return false;
	}

	CellFsStat stat;
	cellFsFstat(fd, &stat);

	void* data = BizAlignedMalloc(stat.st_size, 128);
	if (!data)
	{
		cellFsClose(fd);
		return false;
	}

	cellFsRead(fd, data, stat.st_size, NULL);
	cellFsClose(fd);

	if (mod->CodeReplaced)
		BizAlignedFree(mod->Code);

	mod->Code = data;
	mod->CodeSize = tl_align(stat.st_size, 16);
	mod->CodeReplaced = true;

	LOGINFO( Scheduler::g_Log, "Loaded replacement for JobQueue module '%s' from file '%s'.\n", mod->Name, work);
	return true;
}

void jqReloadSPUModules()
{
	for (jqModule* mod = jqModule::Head; mod; mod = mod->Next)
		jqReloadSPUModule(mod);
}



CellSpurs* jqGetSpurs()
{
	return jqSpurs;
}

} // namespace Scheduler
