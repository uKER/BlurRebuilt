#include "SystemPch.h"
#include <core/Core.h>
#include <core/Maths/MathsUtil.h>
#include <system/DebugLog.h>
#include "system/scheduler/jobqueue.h"
#include "alertbits.h"

#if TL_PLAT_PS3
#include <cell/spurs.h>
#endif

namespace Scheduler
{

Debug::Logger &g_Log = Debug::GetLogger("system.scheduler");

TaskPool jqPool;

int jqNWorkers = 0;
jqWorker* jqWorkers = NULL;
jqWorker* jqTempWorkers = NULL;
int jqNextAvailTempWorker = 0;
int jqNTempWorkers = 0;

int jqMaxWorkers = 0; // Set by _Init
int TaskPoolExternallyLockedCount = 0;
int jqSleepingWorkers = 0;
U32 jqProcessorsMask = 0xfffffffe;

int jqWorkerAwakeCount = 0;

tlThreadId jqMainThreadID = 0;
PolicyList gPolicyList = {0};
U16 ALIGN128 g_iCPUWorkerAlert[8] = {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000};
U32 g_iPolicyUsage[JQ_PRIORITY_MAX] = {0};

tl_threadlocal Task* jqCurBatch = NULL;

// Implementation dependent function prototypes.
void _Init();
void _Shutdown();
void _Start(U32 processorsMask);
void _jqStop();
void _AddTask(const Task* Batch, int iPolicy);

void jqLockBatchPoolInternal();
void jqUnlockBatchPoolInternal();
void DeleteWorkerPolicyOrders();

void jqTempWorkerLoop(jqWorker* worker, DoneAssistingWithJobsFn callback, void* context);

void Init()
{
	jqSleepingWorkers = 0;

	jqPool.ThisPtr = &jqPool;
	jqPool.BatchQueue.ThisPtr = &jqPool.BatchQueue;

	for (int i = 0; i < JQ_PRIORITY_MAX; i++)
	{
#if JQ_USE_ARRAY_QUEUE
		jqPool.BatchQueue.Queue[i].Init(256);
#else
		jqPool.BatchQueue.Queue[i].Init(i == 0 ? NULL : &jqPool.BatchQueue.Queue[0]);
		jqPool.BatchQueue.Queue[i].AllocateNodeBlock(128);
#endif
		jqPool.BatchQueue.QueuedBatchCount[i] = 0;
	}

	jqPool.QueuedBatchCount = 0;
	jqPool.ExecutingBatchCount = 0;

	jqMainThreadID = tlGetCurrentThreadId();

	_Init();

	jqPool.pTaskFinishedEvent = new MT::SpuEvent;
}

void Shutdown()
{
	assert(jqGetCurrentThreadID() == jqGetMainThreadID()); // Shutdown() can only be called from the main thread.

	jqStop();

	SetGroupPoolSize(0);

	for (int i = 0; i < JQ_PRIORITY_MAX; i++)
		jqPool.BatchQueue.Queue[i].Shutdown();

#if PLATFORM_PS3
	delete jqPool.pTaskFinishedEvent;
#endif

	_Shutdown();

	jqPool.ThisPtr = 0;

#if TL_PLAT_PS3
	extern CellSpurs* jqSpurs;
	jqSpurs = NULL;
#endif

#if !PLATFORM_PS3
	delete jqPool.pTaskFinishedEvent;
#endif

	jqSleepingWorkers = 0;
}

bool Active()
{
	return jqPool.ThisPtr != NULL;
}

void jqEnableWorkers(U32 processorsMask)
{
	jqProcessorsMask = processorsMask & 0xfffffffe;	//	avoid creating a worker for main thread
}

int jqGetMaxWorkers()
{
	return jqMaxWorkers;
}

int GetNumWorkers()
{
	return jqNWorkers;
}

tlThreadId jqGetCurrentThreadID()
{
	return tlGetCurrentThreadId();
}

tlThreadId jqGetMainThreadID()
{
	return jqMainThreadID;
}

Task* jqGetBatchForCurrentThread()
{
	return jqCurBatch;
}

void InitWorker(jqWorker* Worker)
{
	Worker->ThisPtr = Worker;
#ifdef ENABLE_LOCAL_BATCH_QUEUE
	Worker->LocalBatchQueue.ThisPtr = &Worker->LocalBatchQueue;
	for (int i = 0; i < JQ_PRIORITY_MAX; i++)
	{
		Worker->LocalBatchQueue.Queue[i].Init(&jqPool.BatchQueue.Queue[0]);
		Worker->LocalBatchQueue.QueuedBatchCount[i] = 0;
	}
#endif
	Worker->LockCount = 0;
	Worker->pAlert = g_iCPUWorkerAlert;
}

void ShutdownWorker(jqWorker* Worker)
{
#ifdef ENABLE_LOCAL_BATCH_QUEUE
	for (int i = 0; i < JQ_PRIORITY_MAX; i++)
		Worker->LocalBatchQueue.Queue[i].Shutdown();
#endif
}

bool Poll( GroupHandle GroupID )
{
	volatile int* BatchCount;
	volatile int* ExecutingBatchCount;
	if (GroupID == JQ_GROUP_NONE)
	{
		BatchCount          = &jqPool.QueuedBatchCount;
		ExecutingBatchCount = &jqPool.ExecutingBatchCount;
	}
	else
	{
		BatchCount          = &jqPool.BatchGroupPool[GroupID].QueuedBatchCount;
		ExecutingBatchCount = &jqPool.BatchGroupPool[GroupID].ExecutingBatchCount;
	}

	int bc = *BatchCount;
	MT::MemoryFence();
	int ec = *ExecutingBatchCount;

	return ((bc + ec) == 0);
}

void Start()
{
	assert(jqGetCurrentThreadID() == jqGetMainThreadID()); // Start can only be called from the main thread.

	jqStop();

	jqNWorkers = Maths::Min(tlCountOnes(jqProcessorsMask), jqMaxWorkers);
	assert(jqNWorkers >= 0); // Not enough workers to use Jobqueue
	jqWorkers = (jqWorker*)BizAlignedMalloc(sizeof(jqWorker)*jqNWorkers, tl_alignof(jqWorker));

	memset(jqWorkers, 0, sizeof(jqWorker)*jqNWorkers);
	for (int i = 0; i < jqNWorkers; i++)
		InitWorker(&jqWorkers[i]);

	//	create temp workers to be used by non-JQ threads if they call jqAssistWithJobs 
	jqNTempWorkers = jqMaxWorkers - jqNWorkers;
	if( jqNTempWorkers > 0 )
	{
		jqTempWorkers = (jqWorker*)BizAlignedMalloc(sizeof(jqWorker)*jqNTempWorkers, tl_alignof(jqWorker));
		memset(jqTempWorkers, 0, sizeof(jqWorker)*jqNTempWorkers);
		for (int i = 0; i < jqNTempWorkers ; i++)
			InitWorker(&jqTempWorkers[i]);
	}

	GeneratePolicySPUUsageBits(g_iPolicyUsage);

	_Start(jqProcessorsMask);
}

void jqStop()
{
	assert(jqGetCurrentThreadID() == jqGetMainThreadID()); // jqStop can only be called from the main thread.
	if ( !jqWorkers )
		return;

	Flush();
	assert(jqPool.QueuedBatchCount == 0); // Stop failed to stop all batches.

	_jqStop();
}

int jqGetQueuedBatchCount(int GroupID)
{
	if (GroupID == JQ_GROUP_NONE)
		return jqPool.QueuedBatchCount;
	else
		return jqPool.BatchGroupPool[GroupID].QueuedBatchCount;
}

int jqGetExecutingBatchCount(int GroupID)
{
	if (GroupID == JQ_GROUP_NONE)
		return jqPool.ExecutingBatchCount;
	else
		return jqPool.BatchGroupPool[GroupID].ExecutingBatchCount;
}

inline void AddTaskToQueue(const Task& Batch, PolicyHandle Priority, TaskQueue* Queue)
{
	_AddTask(&Batch, Priority);

	if (Batch.GroupID != JQ_GROUP_NONE)
		MT::AtomicIncrement(&jqPool.BatchGroupPool[Batch.GroupID].QueuedBatchCount);
	MT::AtomicIncrement(&jqPool.QueuedBatchCount);
	MT::AtomicIncrement(&Queue->QueuedBatchCount[Priority]);

	// increase worker alert counters
	Batch.iCPUBits = g_iPolicyUsage[Priority];
#if PLATFORM_PS3
	if (!Batch._Batch.Code)
		Batch.iCPUBits &= 3;	// if it can't run on the SPU, remove all SPU bits
	if (!Batch.Module->PPUCode)
		Batch.iCPUBits &= ~3;	// if it can't run on the PPU, remove all PPU bits
#endif
	BIZ_ASSERT(Batch.iCPUBits != 0);	// a task was added which can't be run on any CPU

	Queue->Queue[Priority].Push(Batch);

	U64 a,b;
	AlertBits(Batch.iCPUBits, a, b);

#if !PLATFORM_PC
	U64 orga = MT::AtomicAdd((U64*)&g_iCPUWorkerAlert[0], a);
	U64 orgb = MT::AtomicAdd((U64*)&g_iCPUWorkerAlert[4], b);
#else
	U64 orga = (U64)MT::AtomicAdd((U32*)&g_iCPUWorkerAlert[0], a&0xffffffff);
	orga |= (U64)MT::AtomicAdd((U32*)&g_iCPUWorkerAlert[2], (a>>32)) << 32;
	U64 orgb = (U64)MT::AtomicAdd((U32*)&g_iCPUWorkerAlert[4], b&0xffffffff);
	orgb |= (U64)MT::AtomicAdd((U32*)&g_iCPUWorkerAlert[6], (b>>32)) << 32;
#endif

	// main thread doesn't use events
//	if ((orga&(0xffffULL<<48)) == (0x8000ULL<<48) && Batch.iCPUBits&1)	jqAlertWorker(0);
	if ((orga&(0xffffULL<<32)) == (0x8000ULL<<32) && Batch.iCPUBits&2)	jqAlertWorker(1);
#if !PLATFORM_PS3	// SPUs don't use events to wake up
	if ((orga&(0xffffULL<<16)) == (0x8000ULL<<16) && Batch.iCPUBits&4)	jqAlertWorker(2);
	if ((orga&(0xffffULL<< 0)) == (0x8000ULL<< 0) && Batch.iCPUBits&8)	jqAlertWorker(3);
	if ((orgb&(0xffffULL<<48)) == (0x8000ULL<<48) && Batch.iCPUBits&16)	jqAlertWorker(4);
	if ((orgb&(0xffffULL<<32)) == (0x8000ULL<<32) && Batch.iCPUBits&32)	jqAlertWorker(5);
#if !PLATFORM_360
	if ((orgb&(0xffffULL<<16)) == (0x8000ULL<<16) && Batch.iCPUBits&64) jqAlertWorker(6);
	if ((orgb&(0xffffULL<< 0)) == (0x8000ULL<< 0) && Batch.iCPUBits&128) jqAlertWorker(7);
#endif
#endif

	// Update the SPURS ready count to wait up the SPUs.
#if TL_PLAT_PS3
	extern CellSpurs* jqSpurs;
	extern CellSpursWorkloadId jqWorkloadId;
	U32 OldReadyCount;
	cellSpursReadyCountAdd(jqSpurs, jqWorkloadId, &OldReadyCount, 1);
#endif
}

void AddTask(const Task& Data, PolicyHandle Priority)
{
	AddTaskToQueue(Data, Priority, &jqPool.BatchQueue);
}

//void AddTask(const Task& Data)
//{
//	AddTaskToQueue(Data, JQ_PRIORITY_NORMAL, &jqPool.BatchQueue);
//}

void AddTask(const jqModule& Module, void* Input, void* Output, int GroupID, int Priority, void* ParamData, int ParamSize)
{
	assert(Priority >= 0 && Priority < gPolicyList.nPolicies);
	Task Batch;
	Batch.Module = (jqModule*)&Module;
	Batch.Input = Input;
	Batch.Output = Output;
	Batch.GroupID = GroupID;
	if (ParamData && ParamSize)
	{
		BIZ_ASSERT(ParamSize <= (int)sizeof(Batch.ParamData)); // ParamSize is to large to fit in Task structure.
		memcpy(Batch.ParamData, ParamData, ParamSize);
	}
#if TL_DEBUG
	// Clear parameter data if not passed in.
	memset((U8*)Batch.ParamData+ParamSize, 0xCD, sizeof(Batch.ParamData)-ParamSize);
#endif
	AddTask(Batch, (jqPriority)Priority);
}

jqWorker* jqFindWorkerForProcessor(jqProcessor Processor)
{
	//for (int i = 0; i < jqNWorkers; i++)
	//	if (jqWorkers[i].Processor == Processor)
	//		return &jqWorkers[i];
	return NULL;
}

bool jqLockProcessor(int Processor)
{
	jqWorker* Worker = jqFindWorkerForProcessor((jqProcessor)Processor);
	if (Worker)
	{
		MT::AtomicIncrement(&Worker->LockCount);
		return Worker->LockCount > 0;
	}
	return false;
}

void jqUnLockProcessor(int Processor)
{
	jqWorker* Worker = jqFindWorkerForProcessor((jqProcessor)Processor);
	assert(Worker == NULL || Worker->LockCount > 0); // jqUnlockProcessor underflow.
	if (Worker != NULL)
		MT::AtomicDecrement(&Worker->LockCount);
}

#ifdef ENABLE_LOCAL_BATCH_QUEUE
void AddTaskToProcessor(Task& Data, jqPriority Priority, int Processor)
{
	jqWorker* Worker = jqFindWorkerForProcessor((jqProcessor)Processor);
	assert(Worker == NULL || Worker->LockCount > 0); // AddTaskToProcessor must lock processor first.
	if (Worker != NULL)
		AddTaskToQueue(Data, Priority, &Worker->LocalBatchQueue);
	else
		AddTaskToQueue(Data, Priority, &jqPool.BatchQueue);
}
#endif

void jqAssistWithJobs(DoneAssistingWithJobsFn callback, void* context)
{
	assert(callback); // You must supply a non-null callback function
	assert(jqNextAvailTempWorker < jqNTempWorkers); // ran out of temp workers - worker thread calling jqAssistWithJobs, or jqAssistWithJobs being called recursively?
	int tempWorkerIndex = MT::AtomicIncrement(&jqNextAvailTempWorker);
	jqWorker* tempWorker = jqTempWorkers + tempWorkerIndex;
#if TL_PLAT_PC
	tempWorker->Processor = -1;
#elif TL_PLAT_PS3
	tempWorker->Processor = -1;
#endif
	jqTempWorkerLoop(tempWorker, callback, context);
	MT::AtomicDecrement(&jqNextAvailTempWorker);
}

void SetGroupPoolSize(int Size)
{
	Flush();

	jqLockBatchPoolInternal();

	if (jqPool.BatchGroupPool)
	{
		BizAlignedFree(jqPool.BatchGroupPool);
		jqPool.BatchGroupPool = NULL;
	}
	if (Size)
		jqPool.BatchGroupPool = (TaskGroup*)BizAlignedMalloc(sizeof(TaskGroup) * Size, 128);
	jqPool.BatchGroupPoolSize = Size;

	// Link the free list.
	for (int i = 0; i < Size-1; i++)
		jqPool.BatchGroupPool[i].Next = i+1;
	if (Size)
	jqPool.BatchGroupPool[Size-1].Next = JQ_LIST_END;
	jqPool.BatchGroupPoolHead = 0;

	jqUnlockBatchPoolInternal();
}

/// Adds a dependency on another group to a batch group.
void AddGroupDependency(int GroupID, int DependencyGroupID, void *pDependencyObject)
{
	if (DependencyGroupID == JQ_GROUP_NONE || GroupID == JQ_GROUP_NONE)
		return;
	jqLockBatchPoolInternal();
	TaskGroup* Group = &jqPool.BatchGroupPool[GroupID];
	int x = Group->NDependencies++;
	Group->Dependencies[x] = DependencyGroupID;
	Group->DependencyObject[x] = pDependencyObject;
	assert(Group->NDependencies <= (int)tl_elementsof(Group->Dependencies)); // JobQueue group exceeded maximum dependency count!
	jqUnlockBatchPoolInternal();
}

/// Removes a dependency on another group to a batch group.
void jqRemoveBatchGroupDependency(int GroupID, int DependencyGroupID)
{
	if (DependencyGroupID == JQ_GROUP_NONE || GroupID == JQ_GROUP_NONE)
		return;
	jqLockBatchPoolInternal();
	TaskGroup* Group = &jqPool.BatchGroupPool[GroupID];

	//BUG!!! Removing a dependency adds in more dependencies pointing to duff data because they all
	//haven't been initialised!
	const U32 NDep = Group->NDependencies;
	Group->NDependencies = 0;
	for (size_t i = 0; i < NDep; ++i)//tl_elementsof(Group->Dependencies); i++)
	{
		if ( Group->Dependencies[i] != DependencyGroupID )
		{
			int x = Group->NDependencies++;
			Group->Dependencies[x] = Group->Dependencies[i];
			Group->DependencyObject[x] = Group->DependencyObject[i];
		}
	}
	jqUnlockBatchPoolInternal();
}

int CreateGroup(bool bEvent)
{
	jqLockBatchPoolInternal();

	// Allocate one batch from the pool.
	if ( jqPool.BatchGroupPoolHead == JQ_LIST_END )
	{
		LOGFATAL( Scheduler::g_Log, "Out of batch group pool slots (use SetGroupPoolSize to increase)." );
		BIZ_ASSERT_FAIL( "Out of back group pool slots (use SetGroupPoolSize to increase and check for missing calls to Scheduler::DestroyGroup())" );
		DebugBreak(); // make sure the game doesn't continue, because returning -1 for the group number can cause weird crashes later on
		jqUnlockBatchPoolInternal();
		return -1;
	}
	TaskGroup* Group = &jqPool.BatchGroupPool[jqPool.BatchGroupPoolHead];
	jqPool.BatchGroupPoolHead = Group->Next;

	Group->NDependencies = 0;
	Group->QueuedBatchCount = 0;
	Group->ExecutingBatchCount = 0;

	jqUnlockBatchPoolInternal();

	if (bEvent)
	{
		new (&Group->taskFinishedEvent) MT::SpuEvent();
		Group->bUsingEvent = 1;
	}
	else
		Group->bUsingEvent = 0;

	return Group - jqPool.BatchGroupPool;
}

PolicyHandle CreatePolicy(const Policy *pPolicy)
{
	BIZ_ASSERT(jqPool.ThisPtr == NULL); // call CreatePolicy *before* Scheduler::Init(), not after
	BIZ_ASSERT(gPolicyList.nPolicies < JQ_PRIORITY_MAX); // can't create more than 16 policies
	int iPolicy = gPolicyList.nPolicies++;
	gPolicyList.Policies[iPolicy] = pPolicy;
	return iPolicy;
}

static int _giSortByWorkerNo;
int ComparePolicies(const void *A, const void *B)
{
	return gPolicyList.Policies[*(int*)B][_giSortByWorkerNo] - gPolicyList.Policies[*(int*)A][_giSortByWorkerNo];
}

void GenerateWorkerPolicyOrder(WorkerPolicyOrder *pPO, int iCPU)
{
	int nPolicies = gPolicyList.nPolicies;

	// initialise
	for (int j=0; j<nPolicies; j++)
		pPO->iPolicy[j] = j;
	for (int j=nPolicies; j<JQ_PRIORITY_MAX; j++)
		pPO->iPolicy[j] = -1;

	// sort by n'th priority
	_giSortByWorkerNo = iCPU;
	qsort(&pPO->iPolicy, nPolicies, sizeof(int), ComparePolicies);

	// disable any policies with pri 0
	for (int j=0; j<nPolicies; j++)
	{
		Policy x = gPolicyList.Policies[pPO->iPolicy[j]][_giSortByWorkerNo];
		if (x == (Policy)0)
			pPO->iPolicy[j] = -1;
	}
/*
	// dump new WPOs
	char buffer[1024];
	char buffer2[1024];
	sprintf(buffer, "WPO %d : ", i);
	for (int j=0; j<nPolicies; j++)
	{
		sprintf(buffer2, "%d ", pPO->iPolicy[j]);
		strcat(buffer, buffer2);
	}
	LOGINFO(g_Log, "%s\n", buffer);
*/
}

void GeneratePolicySPUUsageBits(U32 *pBits)
{
	for (int i=0; i<gPolicyList.nPolicies; i++)
	{
		*pBits = 0;
		for (int iSPU=0; iSPU<8; iSPU++)
		{
			if (gPolicyList.Policies[i][iSPU] > 0)
				(*pBits) |= 1<<iSPU;
		}
		pBits++;
	}
}

void DestroyGroup(int& GroupID)
{
	using namespace MT;

	if (GroupID == JQ_GROUP_NONE)
	{
		LOGERROR(g_Log, "DestroyGroup: GroupID == JQ_GROUP_NONE - Do not call DestroyGroup with NONE group ID!");
		return;
	}

	Flush(GroupID);

	TaskGroup *pGroup = &jqPool.BatchGroupPool[GroupID];

	jqLockBatchPoolInternal();
	pGroup->Next = jqPool.BatchGroupPoolHead;
	jqPool.BatchGroupPoolHead = GroupID;
	jqUnlockBatchPoolInternal();

	if (pGroup->bUsingEvent)
		jqPool.BatchGroupPool[GroupID].taskFinishedEvent.SpuEvent::~SpuEvent();

	GroupID=JQ_GROUP_NONE;
}

} // namespace Scheduler
