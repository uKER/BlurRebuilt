#include <vis_app/stdafx.h>

#include <mfc_extras/BaseThread.h>

BaseThread::BaseThread()
{
    Thread = NULL;
}

BaseThread::~BaseThread()
{
    DWORD exitCode;

    if (Thread != NULL && ::GetExitCodeThread(Thread->m_hThread, &exitCode) && exitCode == STILL_ACTIVE)
	{
        Destroy();
	}
}

// Mode argument specifies whether thread is created in a suspended state or not.
// = 0 to start thread immediatelly or = CREATE_SUSPENDED to create a thread in suspended state.
void BaseThread::Create(int mode /* = 0 */)
{
    if (!Thread) 
	{
        // Auto reset, initially reset

        EKillThread = ::CreateEvent(NULL,FALSE,FALSE,NULL);
        EThreadIsDead = ::CreateEvent(NULL,FALSE,FALSE,NULL);

        // Create the thread.
        Thread = ::AfxBeginThread(ThreadFunc,this,THREAD_PRIORITY_NORMAL,0,mode);
    }
}

void BaseThread::Destroy(void)
{
    if (Thread)
	{
        // Tell thread to commit suicide
        BOOL retcode = ::SetEvent(EKillThread);
        if (!retcode)
		{
            TRACE0("SetEvent(EKillThread) in BaseThread::Destroy() returned error\n");
		}

        // Wait for info that thread is killed

        if (::WaitForSingleObject(EThreadIsDead,10000L) == WAIT_TIMEOUT) 
		{
            // Safe guard - if something is wrong then brutally kill thread
            ::TerminateThread(Thread->m_hThread,1);
        }

        // Wait a bit
        ::Sleep(10);

        // Maybe not necessary, but just in case
        Thread = NULL;
    }

    ::CloseHandle(EKillThread);
    ::CloseHandle(EThreadIsDead);
}

// Thread handler
UINT BaseThread::ThreadFunc(LPVOID arg)
{
    BaseThread *thread = (BaseThread*)arg;
    thread->DoExecute();
    return 0;
}

void BaseThread::DoExecute(void)
{
    Execute();        // Abstract virtual function
    ::SetEvent(EThreadIsDead);
}

void BaseThread::Suspend(void)
{
    if (Thread)
        Thread->SuspendThread();

}

void BaseThread::Resume(void)
{
    if (Thread)
        Thread->ResumeThread();
}
