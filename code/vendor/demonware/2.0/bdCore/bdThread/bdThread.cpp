// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdThread/bdThread.h>
#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

bdThread::bdThread(bdRunnable *runnable, bdInt priority, bdUInt stackSize)
: m_runnable(runnable),
  m_isRunning(false),
  m_stackSize(stackSize),
  m_priority(priority)
#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP || defined BD_PLATFORM_WII)
 ,m_stackPointer(BD_NULL)
#endif
#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP)
 ,m_joiningThread(-1)
#endif
{
	m_threadArgs.m_runnable = m_runnable;
	m_threadArgs.m_thread = this;
}

bdThread::~bdThread()
{
	if(m_threadArgs.m_args != BD_NULL)
	{
		bdMemory::deallocate(m_threadArgs.m_args);
	}

#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP)
	if (m_stackPointer)
	{
		bdMemory::alignedDeallocate(m_stackPointer);
	}
#endif

#if (defined BD_PLATFORM_WII)
	if (m_stackPointer)
	{
		bdMemory::deallocate(m_stackPointer);
	}
#endif

	bdPlatformThread::deleteThread(m_handle);

	m_runnable = BD_NULL;
}

bdBool bdThread::start(const void *args,
					   const bdUWord size)
{
	bdBool ok = (m_runnable != BD_NULL) && (m_isRunning == false);

	// copy any thread arguments onto the heap
	m_threadArgs.m_args = BD_NULL;
	if(ok && (size > 0))
	{
		m_threadArgs.m_args = bdMemory::allocate(size);	
		bdMemcpy(m_threadArgs.m_args, args, size); 
	}

	// create and start the thread
	if(ok)
	{
		m_isRunning = true;
		m_runnable->start();

#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP)
		if(m_stackSize > 0)
		{
			m_stackPointer = bdMemory::alignedReallocate(m_stackPointer, m_stackSize, 16);
		}
		ok = ok && bdPlatformThread::createThread((bdThreadProc)threadProc, &m_threadArgs, m_handle, m_priority, m_stackSize, m_stackPointer);
		
#elif (defined BD_PLATFORM_WII)
		if(m_stackSize > 0)
		{
			m_stackPointer = bdMemory::reallocate(m_stackPointer, m_stackSize);
		}
		ok = ok && bdPlatformThread::createThread((bdThreadProc)threadProc, &m_threadArgs, m_handle, m_priority, m_stackSize, m_stackPointer);
#else
		ok = ok && bdPlatformThread::createThread((bdThreadProc)threadProc, &m_threadArgs, m_handle, m_priority, m_stackSize);
#endif
		ok = ok && bdPlatformThread::startThread(m_handle, &m_threadArgs, sizeof(m_threadArgs));
	}

	return ok;
}

void bdThread::stop()
{
	m_runnable->stop();
}

void bdThread::join()
{
#if (defined BD_PLATFORM_PS2 )
	if (m_isRunning)
	{
		m_joiningThread = bdPlatformThread::getThreadHandle();
		SleepThread();
	}
#else
	bdPlatformThread::joinThread(m_handle);
#endif
	if (m_threadArgs.m_args)
	{
		bdMemory::deallocate(m_threadArgs.m_args);
		m_threadArgs.m_args = BD_NULL;
	}
}

void bdThread::cleanup()
{
	if (m_threadArgs.m_args)
	{
		bdMemory::deallocate(m_threadArgs.m_args);
		m_threadArgs.m_args = BD_NULL;
	}
	delete this;
}

void bdThread::setPriority(bdInt priority)
{
	m_priority = priority;
	bdPlatformThread::setPriority(m_handle, priority);
}

bdInt bdThread::getPriority()
{
	return m_priority;
}

bdBool bdThread::isRunning() const
{
	return m_isRunning;
}

bdRunnable *bdThread::getRunnable()
{
	return m_runnable;
}

bdThreadHandle bdThread::getThreadHandle()
{
	return m_handle;
}

void bdThread::wakeupJoiningThread()
{
#if (defined BD_PLATFORM_PS2 )
	if (m_joiningThread != -1)
	{
		WakeupThread(m_joiningThread);
	}
#endif
	m_isRunning = false;
}


#if !defined BD_PLATFORM_PSP

bdInt BD_CALL bdThread::threadProc(bdThreadArgs *args)
{
	bdUInt ret;
	bdRunnable *runnable = args->m_runnable;
	ret = runnable->run(args->m_args);
	args->m_thread->wakeupJoiningThread();
#if defined BD_PLATFORM_PS3
	sys_ppu_thread_exit(ret);
#endif
	return ret;
}

#else

bdInt BD_CALL bdThread::threadProc(bdUInt argSize, bdThreadArgs *args)
{
	bdUInt ret;
	bdRunnable *runnable = args->m_runnable;
	ret = runnable->run(args->m_args);
	args->m_thread->wakeupJoiningThread();
	return ret;
}

#endif 
