// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>

#include <sys/synchronization.h>

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	sys_mutex_attribute_t attr;
	sys_mutex_attribute_initialize(attr);

	attr.attr_protocol = SYS_SYNC_PRIORITY;
	attr.attr_recursive = SYS_SYNC_NOT_RECURSIVE;
	attr.attr_pshared = SYS_SYNC_NOT_PROCESS_SHARED;

	bdMutexHandle handle = 0;
	const bdInt result = sys_mutex_create(&handle, &attr);

	if (result != CELL_OK)
	{
		bdFprintf(BD_STDERR, "sys_mutex_create failed with %i\n", result);
		handle = 0;
	}

	return handle;
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	const bdInt result = sys_mutex_lock(handle, 0);
	if (result != CELL_OK)
	{
		bdFprintf(BD_STDERR, "sys_mutex_lock failed with %i\n", result);
	}
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	const bdInt result = sys_mutex_unlock(handle);
	if (result != CELL_OK)
	{
		bdFprintf(BD_STDERR, "sys_mutex_unlock failed with %i\n", result);
	}
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &handle)
{
	const bdInt result = sys_mutex_destroy(handle);
	if (result != CELL_OK)
	{
		bdFprintf(BD_STDERR, "sys_mutex_destroy failed with %i\n", result);
	}
}

