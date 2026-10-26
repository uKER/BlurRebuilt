#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include "system/scheduler/core/tl_defs.h"
#include "system/scheduler/core/tl_thread.h"
#include <core/Maths.h>

namespace Debug { class Logger; }
namespace Scheduler
{
extern Debug::Logger &g_Log;
}
namespace MT
{
class SpuRwLock;
}

/// Indicates that this version of the JobQueue uses a simplified API, replacing the various data pointers and sizes
//   in Task with a generic parameter area.
#define JQ_SIMPLIFIED 1

namespace Scheduler
{

enum
{
	JQ_GROUP_NONE = -1 ,
	JQ_GROUP_SPARE = 0,		// Special group used to soak up spare time on the main CPU.
	JQ_LIST_END = -1,
	JQ_INVALID_HANDLE = -1,

	JQ_GROUP_SPURWLOCK = -2,// indicates a dependency on a MT::SpuRwLock
	JQ_GROUP_LABEL = -3,	// indicates a dependency on a GPU label
};

#if PLATFORM_PPU_SPU
struct __attribute__(( aligned( 16 ) )) Label
#else
struct Label
#endif
{
	U32 *pLabel;
	U32	iCmpValue;
};

enum jqPriority
{
	JQ_PRIORITY_MAX = 10
};

typedef int GroupHandle;
typedef int PolicyHandle;

struct Task;
struct TaskGroup;
struct TaskPool;
struct jqWorker;

}

#if TL_PLAT_PC
#include "jobqueue_xenon.h"
#elif TL_PLAT_PS3 || TL_PLAT_PS3_SPU
#include "jobqueue_ps3.h"
#else
#include "system/scheduler/jobqueue_generic.h"
#endif

#define JQ_USE_ARRAY_QUEUE 0
#define JQ_USER_LF_QUEUE 1

#include "system/scheduler/jobqueue_mem.h"
#include <core/threads/SpuEvent.h>

namespace Scheduler
{

//! Parameters passed to the job processing code.
struct Task
{
	Task()
	{
		GroupID = JQ_GROUP_NONE;
	}

	//! Input and output data.
	void* Input;
	void* Output;

	//! Pointer to job module containing functor info
	jqModule* Module;

	//! ID of the group this batch belongs to.
	int GroupID;

	//! Arbitrary parameter storage for the batch.
#if defined(_WIN64)
	// [Blur2PC][x64] Jobs copy structs of pointers in here (AddTask's ParamData); those
	// structs double in size and read pointers at their natural 8-byte alignment.
	__declspec(align(8)) U32 ParamData[24];
#else
	U32 ParamData[12];
#endif

	mutable U32 iCPUBits; // a bit for each CPU, on if that CPU can run this task or 0 if it can't

	// Implementation specific parts.  Not to be set by user.
	_Task _Batch; // 16 bytes on PS3, 0 on Xenon
} JQ_ALIGNED;

}

#if JQ_USER_LF_QUEUE
#include "system/scheduler/jobqueue_task_queue_lf.h"
#elif JQ_USE_ARRAY_QUEUE
#include "system/scheduler/jobqueue_atomic_queue_array.h"
#else
#include "system/scheduler/jobqueue_atomic_queue_mutex.h"
#endif

namespace Scheduler
{

struct TaskGroup
{
	MT::SpuEvent taskFinishedEvent;
	U32 bUsingEvent;

	/// Number of dependencies in the Dependencies array.
	int NDependencies;

	//! Array of up to 4 groups that this group depends on.  
	/** 
		Processing of batches in this group will not begin until all batches in the groups in the Dependencies
		array have been completed.
	*/
	int Dependencies[4];
	void *DependencyObject[4];

	//! Next group in the batch group pool.
	int Next;

	/// Number of unprocessed batches in the batch pool with this group ID.
	int QueuedBatchCount; 

	//! Number of batches currently executing.
	int ExecutingBatchCount;
} JQ_ALIGNED;

struct TaskQueue
{
	TaskQueue* ThisPtr;
	jqAtomicQueue Queue[JQ_PRIORITY_MAX];
	int QueuedBatchCount[JQ_PRIORITY_MAX];
} JQ_ALIGNED;

// contains an int specifying the priority for each worker
typedef int Policy;

struct WorkerPolicyOrder
{
	int iPolicy[JQ_PRIORITY_MAX];
};

struct PolicyList
{
	// list of policies
	int nPolicies;
	const Policy *Policies[JQ_PRIORITY_MAX];
};

extern PolicyList gPolicyList;

struct TaskPool
{
	TaskPool* ThisPtr;

	TaskQueue BatchQueue;

	TaskGroup* BatchGroupPool;

	int BatchGroupPoolSize;
	int BatchGroupPoolHead;

	int QueuedBatchCount;
	int ExecutingBatchCount;
	MT::SpuEvent *pTaskFinishedEvent;
} JQ_DMA_ALIGNED;

struct jqWorker : public _jqWorker
{
	jqWorker* ThisPtr;

#ifdef ENABLE_LOCAL_BATCH_QUEUE
	TaskQueue LocalBatchQueue;
#endif

	WorkerPolicyOrder PolicyOrder;

	int LockCount;
	U16 *pAlert;
} JQ_ALIGNED;

// internal function for generating worker policy order data for a specified CPU
void GenerateWorkerPolicyOrder(WorkerPolicyOrder *pPO, int iCPU);

// calculates which policies can run on which SPU
void GeneratePolicySPUUsageBits(U32 *pBits);

//! Initialises the job queue manager.
void Init();

//! Causes the job queue manager to shutdown, after making sure the manager has stopped.
void Shutdown();

//! Returns whether the job queue manager is currently active
bool Active();

//! Specify the cores that should have workers created by or-ing jqProcessor values
//! Defaults to all
void jqEnableWorkers(U32 processorsMask);

//! Returns the maximum number of workers available
int jqGetMaxWorkers();

//! Returns the number of workers currently active.
int GetNumWorkers();

//! Returns the ID of the current thread.
tlThreadId jqGetCurrentThreadID();

//! Returns the ID of the thread that called Init.
tlThreadId jqGetMainThreadID();

//! Returns the batch currently executing on this thread, or NULL
Task* jqGetBatchForCurrentThread();

//! Returns the group ID or JQ_GROUP_NONE for whatever work the current thread is doing
inline int jqGetGroupIDForCurrentThread()
{
	Task* curBatch = jqGetBatchForCurrentThread();
	return curBatch ? curBatch->GroupID : JQ_GROUP_NONE;
}

//! Starts the manager, which is responsible for processing batches.
void Start();

//! Stops the manager after all jobs have finished, ending all worker threads and releasing hardware resources.
void jqStop();

//! Returns the number of batches remaining in the queue (not being executed) for the given group.
int jqGetQueuedBatchCount(int GroupID = JQ_GROUP_NONE);

inline int jqGetBatchPoolCount(int GroupID = JQ_GROUP_NONE) { return jqGetQueuedBatchCount(GroupID); }

inline int jqGetRemainingBatchPoolSize() { return 32; }

//! Returns the number of batches currently being executed for the given group.
int jqGetExecutingBatchCount(int GroupID = JQ_GROUP_NONE);

//! Locks the batch pool for access by the current thread.
/**
	Call this function when adding many batches together, to reduce synchronization overhead.  
	However, you should not keep the batch pool locked for a long time as it will prevent workers from retrieving new jobs.
*/
void jqLockBatchPool();

//! Unlocks the batch pool.
void jqUnlockBatchPool();

void jqLetWorkersSleep();
void jqKeepWorkersAwake();
void jqAlertWorker(int iWorkerID);

//! Adds a new batch to be processed.  This is the main entry point for most game code.
/**
	\param Module    The module that will be called.
	\param Input     Arbitrary Input pointer, passed on to the module.
	\param Output    Arbitrary Output pointer, passed on to the module.
	\param GroupID   Group ID number the batch will be assigned to.
	\param Priority  Priority level assigned to the batch.
	\param ParamData Optional pointer to up to 40 bytes of extra parameter data.  The data will be copied into the batch structure.
	\param ParamSize Size of optional parameter data.
*/
void AddTask( 
	const jqModule& Module, void* Input = NULL, void* Output = NULL, 
	int GroupID = JQ_GROUP_NONE, int Policy = 0,
	void* ParamData = NULL, int ParamSize = 0);

//! Adds a new batch to be processed.
/**
	This version allows the batch structure to be constructed manually and passed in.
	This can be faster when many similar batch structures will be created and added together.

	\param Data      The data structure describing the batch.
	\param Priority  Priority level for the batch.
*/
void AddTask( const Task& Data, PolicyHandle Priority );

//! Advanced batch usage. 
bool jqLockProcessor( int processor );
void jqUnLockProcessor( int processor );
void AddTaskToProcessor( Task& Data, jqPriority Priority, int Processor );

//! Waits for queued batches to finish executing.
/**
	This function will stall the main thread until the batches with the specified group have all completed.

	\param GroupID Group ID of batches to wait for.  When JQ_GROUP_NONE, flushes all batches regardless of group.
*/
void Flush( GroupHandle GroupID, bool bRunJobs, U32 timeout = 0xfffffff );
void Flush( GroupHandle GroupID = JQ_GROUP_NONE );

//! Returns true if the batches in the specified group have finished executing.
/**
	\param GroupID Group ID of batches to wait for.  When JQ_GROUP_NONE, flushes all batches regardless of group.
*/
bool Poll( GroupHandle GroupID = JQ_GROUP_NONE );

//! Become a temporary worker
/**
	A thread that is not owned by the JQ system can help with JQ jobs by calling this function.
	You can do this to help process jobs instead of sleeping or waiting the thread.
	The function will not return until the caller-supplied DoneAssistingWithJobsFn returns true.

	\param callback pointer to caller-supplied function which returns true when control of the thread should be returned.
	\param context This pointer value will be passed into the callback to help supply data to it as a convenience
*/
typedef bool (*DoneAssistingWithJobsFn)(void* context);
void jqAssistWithJobs(DoneAssistingWithJobsFn callback, void* context);


//! Sets the size of the batch group pool.  Called before Start.
/** 
	\param Size The maximum number of batch groups that may be created at one time.
*/
void SetGroupPoolSize( int Size );

//! Adds a dependency on another group to a batch group.
/** 
	Batches in the group will not execute until all batches in the dependency group have finished.
	A group can have up to 4 dependencies.

	\param GroupID The group to add the dependency to.
	\param DependencyGroupID The dependency to add.
*/
void AddGroupDependency( GroupHandle GroupID, GroupHandle DependencyGroupID, void *pDependencyObject = NULL );

/// Adds a dependency on another group to an MT::SpuRwLock
inline void AddGroupDependency( GroupHandle GroupID, MT::SpuRwLock *pRwLock ) { AddGroupDependency(GroupID, JQ_GROUP_SPURWLOCK, (void*)pRwLock); }

/// Adds a dependency on another group to a GPU label
inline void AddGroupDependency( GroupHandle GroupID, U32 *pLabel ) { AddGroupDependency(GroupID, JQ_GROUP_LABEL, (void*)pLabel); }

//! Removes a dependency on another group to a batch group.
/** 
	\param GroupID The group to remove the dependency from.
	\param DependencyGroupID The dependency to remove.
*/
void jqRemoveBatchGroupDependency( GroupHandle GroupID, GroupHandle DependencyGroupID );

//! Allocates a new batch group ID.
/** 
	Batch groups are used to control the order in which batches are executed.  

	You can wait for all batches with a given group to finish using Flush, or you can add batches that 
	will not begin processing until all batches with a given Group ID have finished first.
*/
GroupHandle CreateGroup(bool bEvent = false);

//! Allocates a new scheduling policy.
/** 
	Scheduling policies are used to control the order in which batches are executed.  

	Each scheduling policy specifies a task priority per worker thread.
*/
PolicyHandle CreatePolicy(const Policy *pPolicy);

//! Destroys a previously created batch group.
/**
	\param GroupID The ID of the group to be destroyed.
	
	\warning Calling DestroyGroup calls Flush(JQ_FLUSH_ALL) to ensure that no batches in the queue
	are waiting on the destroyed batch, which could lead to hangs.
*/
void DestroyGroup( GroupHandle& GroupID );

//! Sets the size of the batch data heap.
/**
	This function can only be called when the JobQueue is stopped.

	\param Size Size of the heap, in bytes.
*/
void jqSetBatchDataHeapSize( size_t Size );

//! Allocates memory from the batch data heap.
/**
	These allocations are intended to be attached to individual batches, to store extra data for the module to use.  Typically, the module will 
	free this data when it is done processing.

	Note that the allocator used is a binary buddy allocator, so allocation and free time is extremely quick, and blocks are always allocated 
	at an address that is a multiple of their size.

	\param Size Size of the allocation, in bytes.
*/
void* jqAllocBatchData( size_t Size );

//! Releases memory that was allocated from the batch data heap.
/**
	These allocations are intended to be attached to individual batches, to store extra data about the work to be done.  Typically, the
	module will free this data when it is done processing.

	Note that the allocator used is a binary buddy allocator, so allocation and free time is extremely short, and blocks are always allocated 
	at an address that is a multiple of their size.

	\param Ptr Pointer to memory block to free.
*/
void jqFreeBatchData( void* Ptr );

//! Reloads SPU modules from disk, if present.
/**
	SPU modules are embedded into the game executable by converting their ELF files into binary object files.

	However, it is desirable during development to modify and reload SPU modules to quickly test changes without relinking the application.
	When called, this function will scan the target file server root directory for SPU modules whose names match the currently linked modules.
	If files are found, they will be loaded and used in place of the linked modules in all future AddTask calls.

	\warning No attempt is made to verify the modification date or validity of the loaded SPU modules, they are simply matched by name 
	         and loaded if they exist.
*/
void jqReloadSPUModules();

//! Reloads a single SPU module.
/** 
	\param Module - Module object to reload.
	\return Returns true if the module was loaded.
*/
bool jqReloadSPUModule(jqModule* Module);

int GetWorkerID();

#if PLATFORM_SPU
inline int GetWorkerID()
{
	return Scheduler::_jqThisWorker->Processor;
}
#endif

}

#endif
