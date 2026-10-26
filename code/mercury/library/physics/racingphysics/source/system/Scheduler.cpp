//--------------------------------------------------------------------------------------
// Scheduler.cpp
//
// Contains an implementation of a task scheduler, consisting of the TaskScheduler,
// ThreadPool and WorkerThread classes.
//
// XNA Developer Connection.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "stdafx.h"

#ifdef MULTI_THREAD_VEHICLE_PHYSICS

#include <xtl.h>
#include <xboxmath.h>
#include <xmcore.h>
#include "Scheduler.h"
//#include "AtgUtil.h"

//#pragma optimize ("", off)

namespace ATG
{

// Structure used to name threads
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType;     // must be 0x1000
    LPCSTR szName;    // pointer to name (in user address space)
    DWORD dwThreadID; // thread ID (-1 = caller thread)
    DWORD dwFlags;    // reserved for future use, must be zero
} THREADNAME_INFO;

//--------------------------------------------------------------------------------------
// Name: SetThreadName()
// Desc: Set the name of the given thread so that it will show up in the Threads Window
//       in Visual Studio and in PIX timing captures.
//--------------------------------------------------------------------------------------
VOID SetThreadName( DWORD dwThreadID, LPCSTR strThreadName )
{
    THREADNAME_INFO info;
    info.dwType     = 0x1000;
    info.szName     = strThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags    = 0;

    __try
    {
        RaiseException( 0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (const ULONG_PTR*)&info );	// [Blur2PC][x64] ULONG_PTR arguments (DWORD on x86)
    }
    __except( GetExceptionCode()== 0x406D1388 ? 
                EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER ) 
    {
        __noop;
    }
}

//-----------------------------------------------------------------------------
// Name: SetThreadProcessor
// Desc: Wrapper function to set the hardware thread index.
//-----------------------------------------------------------------------------
__inline BOOL SetThreadProcessor( HANDLE handle,
                                  int threadIndex )
{
    if( XSetThreadProcessor( handle, threadIndex % 6 ) == -1 )
    {
        return FALSE;
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: TaskScheduler
// Desc: Class that manages task scheduling.
//-----------------------------------------------------------------------------
TaskScheduler::TaskScheduler()
    : work( NULL ),
      threadPool( NULL ) 
{
	for (int i = 0; i < NUM_RESULTS_QUEUES; i++)
		results[i] = 0;
}

TaskScheduler::~TaskScheduler()
{
    Shutdown();
}

//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Initialize the task scheduler.
//-----------------------------------------------------------------------------
HRESULT TaskScheduler::Initialize( XLOCKFREE_LOG l )
{
    HRESULT             hr = S_OK;

	wakeUpEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Setup the log
    errorLog = l;

    // 1. We do not want to block on the work queue but the thread pool will.
    //    The thread pool will wait on the queue util it gets a special task
    //    from the scheduler telling it to quit.
    // 2. We will have three priorities. High, medium and low. The hash table
    //    will have three buckets and the ArrayFunction simply maps key to
    //    bucket. The key will be the priority of the task.

    XLOCKFREE_CREATE    info =
    {
        0
    };
    info.attributes = XLOCKFREE_REMOVE_WAIT;
    info.removeWaitTime = INFINITE;
    info.allocationLength = 128;

    work = new WorkQueue();
    hr = work->Initialize( &info );
    if( FAILED( hr ) ) return hr;

    //
    // The result queue is XLockFreeQueue used to pass results
    // back to the scheduler from the thread-pool and the tasks
    // run at the pool.
    //
    // Neither the scheduler or thread-pool will block
    // on the result queue. 
    ///
    info.attributes = XLOCKFREE_NO_ATTRIBUTES;
    info.removeWaitTime = 0;
    info.allocationLength = 128;

	for (int i = 0; i < NUM_RESULTS_QUEUES; i++)
	{
		results[i] = new ResultQueue();
		hr = results[i]->Initialize( &info );
		if( FAILED( hr ) ) return hr;
	}

    //
    // Start up the ThreadPool, it will create a number of threads
    // to run. 
    // 
    threadPool = new ThreadPool( work, results, wakeUpEvent );
    hr = threadPool->Initialize( errorLog );
    if( FAILED( hr ) ) return hr;

    return hr;
}

//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Complete all jobs and release memory.
//-----------------------------------------------------------------------------
HRESULT TaskScheduler::Shutdown()
{
    if( work != NULL )
    {
        /*completeMessage.SetCommand( TERMINATE_OPERATION );
        if( SUCCEEDED( work->Add( 0, &completeMessage ) ) )
        {
            BOOL wait = TRUE;
            while( wait )
            {
                Task* job = result->Remove();
                if( job != NULL )
                {
                    switch( job->GetCommand() )
                    {
                    case PERFORM_TASK:
                        job->Complete();
                        break;
                    case TERMINATE_COMPLETE:
                        wait = FALSE;
                        break;
                    case TERMINATE_OPERATION:
                        // Unexpected
                        break;
                    default:
                        // Unexpected
                        break;
                    }
                }
            }
        }*/

        delete work;
        work = NULL;

		for (int i = 0; i < NUM_RESULTS_QUEUES; i++)
		{
			delete results[i];
			results[i] = NULL;
		}

        delete threadPool;
        threadPool = NULL;
    }
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ScheduleTask
// Desc: Schedule a task with standard priority.
//-----------------------------------------------------------------------------
HRESULT TaskScheduler::ScheduleTask(DWORD priority,  Task* task )
{
	task->SetCommand(PERFORM_TASK);
    return work->Add( priority, task );
}


//-----------------------------------------------------------------------------
// Name: ProcessResults
// Desc: Process recieves task results by checkng the queue. Once it recieves 
// then number of tasks it states it returns. To keep the example simple a count
// is used. A more complex criteria for continuing is possible.
//-----------------------------------------------------------------------------
HRESULT TaskScheduler::ProcessResults( int requiredJobs, int resultType )
{
    HRESULT hr = S_OK;
    __int64 missed = 0;
    int     count = 0;
    Task*   task;
    while( count != requiredJobs )
    {
        task = results[resultType]->Remove();
        if( task != NULL )
        {
            switch( task->GetCommand() )
            {
            case PERFORM_TASK:
                hr = task->Complete();
                if( FAILED( hr ) )
                {
                    //XLFLogPrint( errorLog, "Failed Task %x (%d)\n", hr, hr );
                    return hr;
                }
                count++;
                break;
            case TERMINATE_OPERATION:
                // unexpected
                break;
            case TERMINATE_COMPLETE:
                // unexpected
                break;
            default:
                // unexpected
                break;
            }
        }
        else
        {
			WaitForSingleObject(wakeUpEvent, INFINITE);

            missed++;
            if( missed > 0x45000 )
            {
                //XLFLogPrint( errorLog, "Did not receive all the required jobs, got %d, needed %d\n", count,
                //             requiredJobs );
                //return S_OK;  // for now we do not error out.
            }
        }
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Name: ThreadPool
// Desc: ThreadPool class constructor.
//-----------------------------------------------------------------------------
ThreadPool::ThreadPool( WorkQueue* incomingJobs,
                        ResultQueue** jobResults,
						HANDLE wue)
    : lastError( S_OK ),
      numberOfWorkers( 0 ),
      hardwareCore( 0 ),
      firstWorkerCore( 0 ),

      workers( NULL ),
      workHandles( NULL ),
      work( incomingJobs ),
      threadWork( NULL ),
	  wakeUpEvent( wue )
{
	for (int i = 0; i < TaskScheduler::NUM_RESULTS_QUEUES; i++)
		results[i] = jobResults[i];
}

//-----------------------------------------------------------------------------
// Name: ~ThreadPool
// Desc: ThreadPool class destructor.
//-----------------------------------------------------------------------------
ThreadPool::~ThreadPool()
{
    if( workers != NULL )
    {
        for( UINT i = 0; i < numberOfWorkers; i++ )
        {
            delete workers[i];
            delete threadWork[i];
            CloseHandle( workHandles[i] );
        }

        delete [] workers;
        workers = NULL;

        delete [] threadWork;
        threadWork = NULL;

        delete [] workHandles;
        workHandles = NULL;
    }
}


//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Create a thread on pools hardware core to monitor tasks.
//-----------------------------------------------------------------------------
HRESULT ThreadPool::Initialize( XLOCKFREE_LOG l )
{
	// pool
	// number of worker threads
	// starting hw thread for workers
	// number of hw threads for workers
    return Initialize( l, 0, 4, 1, 3 );
}


//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Create a thread on pools hardware core to monitor tasks.
//-----------------------------------------------------------------------------
HRESULT ThreadPool::Initialize( XLOCKFREE_LOG l,
                                UINT coreForPool,
                                UINT workerThreads,
                                UINT startingCoreForWorkers,
                                UINT coresForWorkers )
{
    HRESULT             hr = S_OK;
    //
    // Set up the log and default values.
    //
    errorLog = l;
    hardwareCore = coreForPool;
    numberOfWorkers = workerThreads;
    firstWorkerCore = startingCoreForWorkers;
    numberOfWorkerCores = coresForWorkers;

    //
    // 1. Each bucket will be assigned to specific worker. Therefore, the key is acutally the thread worker ID.
    // 2. The ArrayFunctions classes hash function maps key directly to bucket.
    // 3. The worker threads will block on removing tasks from their bucket.
    //
    XLOCKFREE_CREATE    info =
    {
        0
    };

    info.attributes = XLOCKFREE_REMOVE_WAIT;
    info.removeWaitTime = INFINITE;
    info.allocationLength = 128 / numberOfWorkers;

    threadWork = new WorkQueue*[numberOfWorkers];

    for( UINT i = 0; i < numberOfWorkers; i++ )
    {
        threadWork[i] = new WorkQueue();
        hr = threadWork[i]->Initialize( &info );
        if( FAILED( hr ) ) return hr;
    }

    //
    // We now have all the queues initialized before we have started any threads. We do it this way so there is no worry about race
    // conditions. After the lock-free data structures are created they can be safely used from multiple threads.
    //
    // Create the thread for the ThreadPool on the specified hardware core.
    //
    DWORD               threadId;
    HANDLE              threadPoolHandle = CreateThread( 0, 0, &ThreadPool::Start, this, CREATE_SUSPENDED, &threadId );
    if( threadPoolHandle == NULL )
        return E_FAIL;

    SetThreadName( threadId, "ThreadPool" );

    if( !SetThreadProcessor( threadPoolHandle, hardwareCore ) )
        return E_FAIL;

    if( ResumeThread( threadPoolHandle ) == -1 )
        return E_FAIL;

    // It is no longer safe to directly access the threadPool until we have coordinated a shutown.
    // Only communicate with the thread pool the work hash-table!
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Start
// Desc: This function gets run on the hardware thread that will be responsible
// for the hardware queue.
//-----------------------------------------------------------------------------
DWORD ThreadPool::Start( void* information )
{
    ThreadPool* me = ( ThreadPool* )information;
    return me->Process();
}


//-----------------------------------------------------------------------------
// Name: Process
// Desc: Main threadpool process loop.
//-----------------------------------------------------------------------------
DWORD ThreadPool::Process()
{
    HRESULT hr = S_OK;
    int     hardwareThread = 0;

    //
    // First start up the worker threads
    //
    workers = new ThreadWorker*[numberOfWorkers];
    ZeroMemory( workers, sizeof( ThreadWorker* ) * numberOfWorkers );

    workHandles = new HANDLE[numberOfWorkers];
    ZeroMemory( workHandles, sizeof( HANDLE ) * numberOfWorkers );

    for( UINT i = 0; i < numberOfWorkers; i++ )
    {
        workers[i] = new ThreadWorker( i, 
										work,//threadWork[i], 
										results, 
										wakeUpEvent );
        if( workers[i] == NULL )
        {
            lastError = E_FAIL;
            return 1;
        }

        hardwareThread = i % numberOfWorkerCores + firstWorkerCore;
        hr = workers[i]->Initialize( hardwareThread, errorLog, &( workHandles[i] ) );
        if( FAILED( hr ) )
        {
            lastError = hr;
            return 1;
        }
    }

	return 0;

    //
    // We are just going to round-robin and process the work
    //
    int     nextWorker = 0;
    Task*   job;
    BOOL    keepWorking = TRUE;
    do
    {
        // This is blocking so we will wait until there is an entry.
        hr = work->RemoveFirst( &job );
        if( FAILED( hr ) )
        {
            lastError = hr;
            //XLFLogPrint( errorLog, "Thread pool got unexpected error removing task, error = %x (%d)\n", hr, hr );
            return 1;
        }

        switch( job->GetCommand() )
        {
        case PERFORM_TASK:
            threadWork[nextWorker]->Add( 1, job );
            nextWorker = ( nextWorker + 1 ) % numberOfWorkers;
            break;
        case TERMINATE_OPERATION:
            Shutdown();
            keepWorking = FALSE;
            break;
        case TERMINATE_COMPLETE:
            //XLFLogPrint( errorLog, "Thread pool got unexpected command\n" );
            break;
        default:
            break;
        }
    } while( keepWorking );

    return 0;
}

//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Complete all worker threads.
//-----------------------------------------------------------------------------
HRESULT ThreadPool::Shutdown()
{
    completeMessage.SetCommand( TERMINATE_OPERATION );
    for( UINT i = 0; i < numberOfWorkers; i++ )
    {
        threadWork[i]->Add( 0, &completeMessage );
    }

    DWORD waitResult = WaitForMultipleObjects( numberOfWorkers, workHandles, TRUE, INFINITE );
    if( waitResult == WAIT_FAILED )
    {
        // unexpected
    }

    completeMessage.SetCommand( TERMINATE_COMPLETE );
    results[0]->Add( &completeMessage );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ThreadWorker
// Desc: Worker thread constructor.
//-----------------------------------------------------------------------------
ThreadWorker::ThreadWorker( int id,
                            WorkQueue* jobQueue,
                            ResultQueue** completedJobs,
							HANDLE wue)
    : workerId( id ),
      work( jobQueue ),
	  wakeUpEvent( wue ),
      lastError( S_OK ) 
{
	for (int i = 0; i < TaskScheduler::NUM_RESULTS_QUEUES; i++)
		results[i] = completedJobs[i];
}


ThreadWorker::~ThreadWorker() {}


//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Initialize worker thread.
//-----------------------------------------------------------------------------
HRESULT ThreadWorker::Initialize( int hardwareThread,
                                  XLOCKFREE_LOG l,
                                  HANDLE* handle )
{
    char    threadName[32];
    sprintf_s( threadName, sizeof( threadName ) / sizeof( threadName[0] ), "WorkerThread%d", workerId );

    //
    // Register the log
    //
    errorLog = l;

    // 
    // Start up the worker thread
    //
    DWORD   threadId;
    HANDLE  worker = CreateThread( 0, 0, &ThreadWorker::Start, this, CREATE_SUSPENDED, &threadId );

    if( worker == NULL )
        return E_FAIL;

    SetThreadName( threadId, threadName );

    if( !SetThreadProcessor( worker, hardwareThread ) )
        return E_FAIL;

    if( ResumeThread( worker ) == -1 )
        return E_FAIL;

    *handle = worker;
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Start
// Desc: Start worker thread.
//-----------------------------------------------------------------------------
DWORD ThreadWorker::Start( void* information )
{
    ThreadWorker* me = ( ThreadWorker* )information;
    return me->Process();
}


//-----------------------------------------------------------------------------
// Name: Process
// Desc: Main worker thread process loop.
//-----------------------------------------------------------------------------
DWORD ThreadWorker::Process()
{
    HRESULT hr = S_OK;

    Task*   job;
    BOOL    keepWorking = TRUE;
    do
    {
        //
        // This is blocking so we will wait until there is an entry in our bucket
        //
        hr = work->RemoveFirst( &job );
        if( FAILED( hr ) )
        {
            lastError = hr;
            //XLFLogPrint( errorLog, "Removing task failed, %x (%d)\n", hr, hr );
            return 1;
        }

        switch( job->GetCommand() )
        {
        case PERFORM_TASK:
            hr = job->Process();
            if( FAILED( hr ) )
            {
                //XLFLogPrint( errorLog, "Thread task failed, %x (%d)\n", hr, hr );
                lastError = hr;
                return 1;
            }
            results[job->GetResultType()]->Add( job );
			SetEvent(wakeUpEvent);
            break;
        case TERMINATE_OPERATION:
            keepWorking = FALSE;
            break;
        case TERMINATE_COMPLETE:
            //XLFLogPrint( errorLog, "Told to complete, not expected\n" );
            break;
        default:
            break;
        }
    } while( keepWorking );

    return 0;
}

}

#endif
