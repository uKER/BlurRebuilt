#include "SystemPch.h"
#include <core/Core.h>
#include <core/threads/SpuRwLock.h>
#include "system/scheduler/jobqueue.h"
#include "system/perf/PerfEvent.h"
#include <system/debuglog.h>
#include "alertbits.h"

namespace Scheduler
{

extern TaskPool jqPool;
extern int jqWorkerAwakeCount;
extern jqWorker* jqWorkers;

extern tl_threadlocal Task* jqCurBatch;

bool jqWorkerSleep(jqWorker* Worker, int iWorkerID);
void jqExecuteBatch(Task* Batch);
bool jqCanBatchExecute( Task* Batch, jqWorker* Worker );
bool jqSafeToPopBatch(jqWorker* Worker, jqPriority Priority, bool IsLocal);
void jqLockBatchPoolInternal();
void jqUnlockBatchPoolInternal();

//! Returns whether a group's dependencies are finished.
/** 
	GroupID - The ID of the group to check. If JQ_GROUP_NONE, returns true.
	\return Returns true if the group has no dependencies or if all dependencies are finished, else false.
*/

FORCE_INLINE bool jqCheckDependenciesFinished(Task *pTask)
{
	int GroupID = pTask->GroupID;
	if (GroupID == JQ_GROUP_NONE)
		return true;

	TaskGroup* GroupPool = (TaskGroup*)Spu::Get((INTPTR*)&jqPool.ThisPtr->BatchGroupPool);	// [Blur2PC][x64] pointer-wide
	TaskGroup* Group = &GroupPool[GroupID];
	int NDep = Spu::Get(&Group->NDependencies);
	for (int i = 0; i < NDep; i++)
	{
		int Dep = Spu::Get(&Group->Dependencies[i]);
		switch (Dep)
		{
		default:
			// depends on another group - check that all the tasks in that group have finished
			{
			int Queued = Spu::Get(&GroupPool[Dep].QueuedBatchCount);
			MT::MemoryFence();
			int Executing = Spu::Get(&GroupPool[Dep].ExecutingBatchCount);
			if (Queued+Executing > 0)
				return false;
			}
			break;
		case JQ_GROUP_SPURWLOCK:
			// depends on an MT::SpuRwLock - acquire a read-lock on it
			{
				MT::SpuRwLock *pLock = (MT::SpuRwLock*)Spu::Get((INTPTR*)&Group->DependencyObject[i]);	// [Blur2PC][x64] pointer-wide
#if PLATFORM_SPU
				U8 dummy[sizeof(MT::SpuRwLock)];
				Spu::Fetch(&dummy, pLock, sizeof(MT::SpuRwLock));
				pLock = (MT::SpuRwLock*)&dummy;
#endif
				if (!pLock->ReadLock(0))
					return false;
			}
			break;
		case JQ_GROUP_LABEL:
			// depends on a 'Label' structure
			{
				U32 *pLabel = (U32*)Spu::Get((INTPTR*)&Group->DependencyObject[i]);	// [Blur2PC][x64] a pointer
				U32 iCmpValue = (U32)(INTPTR)pTask->Output;	// [Blur2PC][x64] label fence value carried in Output
				U32 fenceValue = MT::AtomicGet(pLabel);
				if ((fenceValue - iCmpValue) >> 31)
					return false;
			}
			break;
		}
	}
	return true;
}

// release synchronisation primitive if neccesary
void jqPostExecBatch(int GroupID)
{
	if (GroupID == JQ_GROUP_NONE)
		return;

	TaskGroup* GroupPool = (TaskGroup*)Spu::Get((INTPTR*)&jqPool.ThisPtr->BatchGroupPool);	// [Blur2PC][x64] pointer-wide
	TaskGroup* Group = &GroupPool[GroupID];
	int NDep = Spu::Get(&Group->NDependencies);
	for (int i = 0; i < NDep; i++)
	{
		int Dep = Spu::Get(&Group->Dependencies[i]);
		switch (Dep)
		{
		default:
			break;
		case JQ_GROUP_SPURWLOCK:
			// depends on an MT::SpuRwLock - release read-lock on it
			{
				MT::SpuRwLock *pLock = (MT::SpuRwLock*)Spu::Get((INTPTR*)&Group->DependencyObject[i]);	// [Blur2PC][x64] pointer-wide
#if PLATFORM_SPU
				U8 dummy[sizeof(MT::SpuRwLock)];
				Spu::Fetch(&dummy, pLock, sizeof(MT::SpuRwLock));
				pLock = (MT::SpuRwLock*)&dummy;
#endif
				pLock->ReleaseReadLock();
			}
			break;
		}
	}
}


//! Returns the next available batch with the given group ID and priority, and removes it from the batch queue.
/** 
	Batches are skipped when their groups have dependencies that are not finished, or when they do not belong
	to the requested group.  Skipped batches are moved to the end of the queue.

	\param Queue       The batch queue to pull from.  Either the global queue or a worker's local one.
	\param Priority    Requested priority level.  Only batches with this priority level will be returned.
	\param GroupID     Requested group ID. If JQ_GROUP_NONE, grabs from any group.
	\param PoppedBatch Pointer to a batch structure to fill with the popped batch.

	\return Returns true if a batch was popped, otherwise false.  Returns the batch data in *PoppedBatch.
*/
static bool jqPopNextBatchFromQueue(jqWorker* Worker, TaskQueue* Queue, int Priority, int GroupID, Task* PoppedBatch)
{
	int CheckedBatches = 0;
	for (;;)
	{
		// Break out of the loop in the case where none of the batches could be processed now.
		int BatchCount = (int)MT::AtomicGet((U32*)&Queue->ThisPtr->QueuedBatchCount[Priority]);
		if (BatchCount == 0 || CheckedBatches > BatchCount)
			break;
		++CheckedBatches;

		// Pop next batch off the current priority queue.
		bool Popped = Queue->Queue[Priority].Pop(PoppedBatch);
		if (!Popped)
			break;

		// Check that this batch is the in correct group, can be executed by this worker (PPU vs SPU),
		//  and is ready to be processed.
		if (GroupID == JQ_GROUP_NONE || GroupID == PoppedBatch->GroupID)
		{
			if (jqCanBatchExecute(PoppedBatch, Worker))
			{
				if (jqCheckDependenciesFinished(PoppedBatch))
				{
					// The batch can be taken.  Update the various counters and return success.
					if (PoppedBatch->GroupID != JQ_GROUP_NONE)
					{
						TaskGroup* GroupPool = (TaskGroup*)Spu::Get((INTPTR*)&jqPool.ThisPtr->BatchGroupPool);	// [Blur2PC][x64] pointer-wide

						MT::AtomicIncrement(&GroupPool[PoppedBatch->GroupID].ExecutingBatchCount);
						MT::AtomicDecrement(&GroupPool[PoppedBatch->GroupID].QueuedBatchCount);
#ifndef TL_PLAT_PS3_SPU
						assert(jqPool.BatchGroupPool[PoppedBatch->GroupID].QueuedBatchCount >= 0); // JobQueue group queued count went below zero!
#endif
					}
					MT::AtomicIncrement(&jqPool.ThisPtr->ExecutingBatchCount);
					MT::AtomicDecrement(&jqPool.ThisPtr->QueuedBatchCount);
					MT::AtomicDecrement(&Queue->ThisPtr->QueuedBatchCount[Priority]);
#ifndef TL_PLAT_PS3_SPU
					assert(jqPool.QueuedBatchCount >= 0); // JobQueue global queued count went below zero!
					assert(Queue->QueuedBatchCount[Priority] >= 0); // JobQueue queue count went below zero!
#endif
					return true;
				}
			}
		}

		// The batch could not be taken for some reason.
		// Push the batch back on the end of the queue and loop to try another one.
		Queue->Queue[Priority].Push(*PoppedBatch);
	}

	// When the entire queue for this priority has been tried and no batch was found, return failure.
	return false;
}

//! Returns the next available batch with the given group ID, and removes it from the batch queue.
/** 
	Batches are skipped when their groups have dependencies that are not finished, or when they do not belong
	to the requested group.  Skipped batches are moved to the end of the queue.

	\param Worker      The worker executing the batch.  The worker has some control over what batches it will accept.
	\param GroupID     Requested group ID. If JQ_GROUP_NONE, grabs from any group.
	\param PoppedBatch Pointer to a batch structure to fill with the popped batch.

	\return Returns true if a batch was popped, otherwise false.  Returns the batch data in *NextBatch.
*/
bool jqPopNextBatch(jqWorker* Worker, int GroupID, Task* PoppedBatch)
{
/*
#if !PLATFORM_SPU && !CFG_CONVERTER
	PERF_EVENT("jqPopNextBatch");
#endif
*/

	// Used when Flush is called from the main thread with a specific group ID.
	// JAN: commented out this bit of code. It doesn't seem that useful and it will potentially
	//      acquire a lock without releasing it later on
	//if (GroupID != JQ_GROUP_NONE)
	//	if (!jqCheckDependenciesFinished(GroupID))
	//		return false;

#ifdef TL_PLAT_PS3_SPU
//	DebugBreak();
#endif

	// Attempt to pull each priority level in order, testing against platform specific checks that allow customization
	// of the scheduler.
	for (int p = 0; p < JQ_PRIORITY_MAX; p++)
	{
		// get policy number in priority order from high to low, -1 means no more policies
		int iPolicy = Worker->PolicyOrder.iPolicy[p];
		if (iPolicy == -1)
			break;
#ifdef ENABLE_LOCAL_BATCH_QUEUE
		// First try the local queue specific to this processor.
		int LocalCount = (int)MT::AtomicGet((U32*)&Worker->ThisPtr->LocalBatchQueue.QueuedBatchCount[iPolicy]);
		if (LocalCount && jqSafeToPopBatch(Worker, (jqPriority)iPolicy, true))
		{
			if (jqPopNextBatchFromQueue(Worker, &Worker->LocalBatchQueue, iPolicy, GroupID, PoppedBatch))
				return true;
		}
#endif
		// Then try the global batch queue.
		int GlobalCount = (int)MT::AtomicGet((U32*)&jqPool.ThisPtr->BatchQueue.QueuedBatchCount[iPolicy]);
		if (GlobalCount && jqSafeToPopBatch(Worker, (jqPriority)iPolicy, false))
		{
			if (jqPopNextBatchFromQueue(Worker, &jqPool.BatchQueue, iPolicy, GroupID, PoppedBatch))
				return true;
		}
	}


	return false;
}

extern U32 g_pszFlushString;

void jqWorkerLoop(jqWorker* Worker, int GroupID, int iWorkerID)
{
	Task CurBatch;
	for (;;)
	{
		{
/*
#if !PLATFORM_SPU#if !CFG_CONVERTER
		PERF_EVENT("Scheduler.Loop");
#endif
#else
		if (g_pszFlushString && Scheduler::_jqThisWorker->PPUModuleAddress)
		{
			Perf::BeginNamedEvent((char*)(g_pszFlushString - (U32)Scheduler::_jqThisWorker->PPUModuleAddress));
			Perf::FlushEventBuffer();
		}
#endif
*/
		// Loop while batches are available, popping batches from the queue.		
		while (1)
		{
#if PLATFORM_SPU
			// give system workload a chance to executer even if there are batches still queued up
//			if (!jqWorkerSleep(Worker, iWorkerID))
//				return;
#endif

#ifdef PLATFORM_PPU
			// stop the PPU main thread from thrashing the jobqueue when there is no work to do anyway
			if (iWorkerID == -1 && Worker->pAlert[0] <= 0x8000)
			{
				sys_timer_usleep(20);
				return;
			}
#endif

			bool b = jqPopNextBatch(Worker, GroupID, &CurBatch);
			if (!b)
				break;

			//	update group id
			jqCurBatch = &CurBatch;

			U64 a0,b0;
			AlertBits(CurBatch.iCPUBits, a0, b0);
#if !PLATFORM_PC
			MT::AtomicAdd((U64*)&Worker->pAlert[0], -a0);
			MT::AtomicAdd((U64*)&Worker->pAlert[4], -b0);
#else
			MT::AtomicAdd((U32*)&Worker->pAlert[0], -a0);
			MT::AtomicAdd((U32*)&Worker->pAlert[2], -(a0>>32));
			MT::AtomicAdd((U32*)&Worker->pAlert[4], -b0);
			MT::AtomicAdd((U32*)&Worker->pAlert[6], -(b0>>32));
#endif

			// Call the batch's module to execute the batch.
			jqExecuteBatch(&CurBatch);

			// release synchronisation primitive if neccesary
			jqPostExecBatch(CurBatch.GroupID);

			//	reset group id
			jqCurBatch = NULL;
			
			// Signal that the batch has finished executing.
			U32 iQBC = 0;
			MT::AtomicDecrement(&jqPool.ThisPtr->ExecutingBatchCount);
			TaskGroup* GroupPool = (TaskGroup*)Spu::Get((INTPTR*)&jqPool.ThisPtr->BatchGroupPool);	// [Blur2PC][x64] pointer-wide
			if (CurBatch.GroupID != JQ_GROUP_NONE)
			{
				MT::AtomicDecrement(&GroupPool[CurBatch.GroupID].ExecutingBatchCount);
				iQBC = MT::AtomicGet(&GroupPool[CurBatch.GroupID].QueuedBatchCount);
			}

			// 
			MT::SpuEvent *pEvent;
			if (CurBatch.GroupID == JQ_GROUP_NONE)
				pEvent = jqPool.pTaskFinishedEvent;
			else
				pEvent = &GroupPool[CurBatch.GroupID].taskFinishedEvent;

			if (MT::AtomicGet((U32*)&GroupPool[CurBatch.GroupID].bUsingEvent) && iQBC==0)
			{
#if PLATFORM_SPU
				U8 x = Spu::Get((U8*)pEvent);
				int rtn = sys_spu_thread_send_event(x, 0, 0);
				if (rtn != CELL_OK && rtn != EBUSY)
					DebugBreak();	// this can happen if Scheduler::DestroyGroup was called on this task's group
#else
				pEvent->Signal();
#endif
			}
		}

/*
#if PLATFORM_SPU			if (g_pszFlushString && Scheduler::_jqThisWorker->PPUModuleAddress)
			{
				Perf::EndNamedEvent();
				Perf::FlushEventBuffer();
			}
#endif
*/
		}
		// No more batches are available.  Break out if requested, or else sleep and continue.
		if (iWorkerID == -1)
			return;

		while (1)
		{
			U16 v;
#if PLATFORM_SPU
			U32 x = MT::AtomicGet((U32*)&Worker->pAlert[iWorkerID&~1]);
			if (iWorkerID&1)
				v = x>>16;
			else
				v = x;
			
#else
			v = Worker->pAlert[iWorkerID];
#endif
			if (v > 0x8000)
				break;
			bool KeepGoing = jqWorkerSleep(Worker, iWorkerID);
			if (KeepGoing == false)
				return;
		}
	}

	// NOTE: It's possible for this function to exit even though not all batches have been executed
	//       this is because another CPU could try to grab that batch, even though it might not be allowed to run it.
	//       In this case queued batch count would temporarily be 0, meaning jqPopNextBatch returns false
}

void jqTempWorkerLoop(jqWorker* worker, DoneAssistingWithJobsFn callback, void* context)
{
	Task CurBatch;
	for( ;; )
	{
		if( callback(context) ) return;
		// Loop while batches are available, popping batches from the queue.
		while (jqPopNextBatch(worker, JQ_GROUP_NONE, &CurBatch) )
		{
			//	update group id
			jqCurBatch = &CurBatch;

			// Call the batch's module to execute the batch.
			jqExecuteBatch(&CurBatch);

			// release synchronisation primitive if neccesary
			jqPostExecBatch(CurBatch.GroupID);

			//	reset group id
			jqCurBatch = NULL;
			
			// Signal that the batch has finished executing.
			MT::AtomicDecrement(&jqPool.ThisPtr->ExecutingBatchCount);
			if (CurBatch.GroupID != JQ_GROUP_NONE)
				MT::AtomicDecrement(&jqPool.BatchGroupPool[CurBatch.GroupID].ExecutingBatchCount);

			if( callback(context) ) return;
		}
	}
}

} // namespace Scheduler

#if TL_PLAT_PS3 || TL_PLAT_PS3_SPU
#include "jobqueue_kernel_ps3.cpp"
#endif
#if TL_PLAT_PC
#include "jobqueue_kernel_xenon.cpp"
#endif
