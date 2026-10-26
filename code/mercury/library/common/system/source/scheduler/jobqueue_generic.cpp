#include "system/scheduler/jobqueue.h"

#include <limits.h>

//-----------------------------------------------------------------------------
//! statics
static size_t   TaskPoolSize;
static Task* TaskPool;
static Task* TaskPoolHead;
static Task* TaskQueueHead;
//-----------------------------------------------------------------------------

void Init()
{
	TaskPool = NULL;
	TaskPoolHead = NULL;
	TaskQueueHead = NULL;
	TaskPoolSize = 0;
}

void Shutdown()
{
	jqStop();

	if ( TaskPool )
		tlMemFree( TaskPool );

	TaskPool = NULL;
	TaskPoolHead = NULL;
	TaskQueueHead = NULL;
}

void Start( size_t TL_UNUSED(NumWorkers) )
{
}

void jqStop()
{
	Flush();
}

void jqSetBatchPoolSize( size_t size )
{
	TL_ASSERT( size > 0, "Invalid batch pool size." );

	Flush();

	// Free existing batches.
	if ( TaskPool )
		tlMemFree( TaskPool );

	// Allocate batch memory.
	TaskPoolSize = size;
	TaskPool = (Task*)tlMemAlloc( sizeof(Task)*TaskPoolSize );

	// Link the free list.
	for ( size_t i = 0; i < TaskPoolSize-1; i++ )
		TaskPool[i].Next = &TaskPool[i+1];
	TaskPool[TaskPoolSize-1].Next = NULL;
	TaskPoolHead = &TaskPool[0];
}

TaskHandle AddTask( const Task& Data )
{
	// Allocate one batch from the pool.
	TL_ASSERT( TaskPoolHead, "Out of batch pool slots (use SetBatchPoolSize to increase)." );
	Task* Batch = TaskPoolHead;
	TaskPoolHead = TaskPoolHead->Next;

	// Copy the batch data.
	*Batch = Data;

	// Link the new batch into the queue.
	Batch->Next = TaskQueueHead;
	TaskQueueHead = Batch;

	return Batch;
}

bool jqRemoveBatch( TaskHandle Handle )
{
	// Walks the list using the address of the Next pointer that points to the deleted node.
	for ( Task* volatile * Next = &TaskQueueHead; *Next; Next = &(*Next)->Next )
	{
		if ( Handle == *Next )
		{
			*Next = Handle->Next;
			Handle->Next = TaskPoolHead;
			TaskPoolHead = Handle;
			return true;
		}
	}

	return false;
}

void Flush(int /*GroupID*/)
{
	while ( TaskQueueHead ) 
	{
		// Pop a batch from the queue.
		Task* Batch = TaskQueueHead;
		TaskQueueHead = Batch->Next;

		// Run the code.
		Batch->Module->Code( Batch );

		// Release this batch, and pop another batch from the queue.
		Batch->Next = TaskPoolHead;
		TaskPoolHead = Batch;
	}
}

void SetGroupPoolSize( size_t /*Size*/ ) { }

void AddGroupDependency( GroupHandle /*GroupID*/, GroupHandle /*DependencyGroupID*/) { }

void jqRemoveBatchGroupDependency( GroupHandle /*GroupID*/, GroupHandle /*DependencyGroupID*/) { }

int CreateGroup() { return 1; }

void DestroyGroup( int /*GroupID*/ ) { }
