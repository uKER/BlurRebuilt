#include "jobqueue.h"

#include <limits.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void jqValidate()
{
}

void Start( size_t TL_UNUSED(NumWorkers) )
{
}

void jqSetBatchPoolSize( size_t size )
{
}

void _AddTask( const Task* Batch, int iPolicy )
{
	Batch->Module->Code( (Task*)Batch );
}

void jqAlertWorker(int iWorkerID)
{
}

void jqLockBatchPoolInternal()
{
}

void jqUnlockBatchPoolInternal()
{
}

void Flush(int /*GroupID*/, bool /*bRunJobs*/)
{
}

void SetGroupPoolSize( size_t /*Size*/ ) { }

void _Init() { }
void _Shutdown() { }
void _Start( int NWorkers ) { }
void _jqStop() { }

