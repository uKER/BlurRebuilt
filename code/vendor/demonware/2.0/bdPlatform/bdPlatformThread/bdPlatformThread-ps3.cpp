// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformThread.h>

#include <sys/ppu_thread.h>
#include <netex/net.h>

bdBool BD_CALL bdPlatformThread::createThread(bdThreadProc threadProc, 
											  void *args,
											  bdThreadHandle &handle,
											  bdInt priority ,
											  bdUInt32 stackSize,
											  void*)
{
	// This also starts the thread.
	const bdInt result = sys_ppu_thread_create(&handle,
											   threadProc,
											   reinterpret_cast<bdUInt64>(args),
											   priority,
											   stackSize,
											   SYS_PPU_THREAD_CREATE_JOINABLE,
											   "DW Thread");

	if (result == CELL_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bdBool BD_CALL bdPlatformThread::startThread(bdThreadHandle &handle, 
											 void *args, 
											 bdUInt argSize )
{
	return true;
}

void BD_CALL bdPlatformThread::joinThread(bdThreadHandle &handle)
{
	bdUInt64 vptr;
	sys_ppu_thread_join(handle, &vptr);
}

void BD_CALL bdPlatformThread::deleteThread(bdThreadHandle &handle)
{
	// Threads making use of libnet socket functions allocate 3K of context
	// information that needs to be freed up explicitely
	sys_net_free_thread_context(handle, 0);
}

void BD_CALL bdPlatformThread::setPriority(bdThreadHandle &handle, 
										   bdInt32 priority)
{
	sys_ppu_thread_set_priority(handle, priority);
}

bdInt32 BD_CALL bdPlatformThread::getPriority(bdThreadHandle &handle)
{
	bdInt prio = -1;
	if (sys_ppu_thread_get_priority(handle, &prio) == CELL_OK)
	{
		return prio;
	}
	return -1;
}

bdInt32 BD_CALL bdPlatformThread::getPriority()
{
	bdThreadHandle current = getThreadHandle();
	if (current != 0)
	{
		bdInt prio = -1;
		if (sys_ppu_thread_get_priority(current, &prio) == CELL_OK)
		{
			return prio;
		}
	}
	return -1;
}

bdThreadHandle BD_CALL bdPlatformThread::getThreadHandle()
{
	bdThreadHandle current;
	if (sys_ppu_thread_get_id(&current) == CELL_OK)
	{
		return current;
	}

	return 0;
}
