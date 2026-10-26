#include "CorePch.h"
#include <core/allocators/CoreMemoryFunctions.h>

#ifdef PLATFORM_PS3

int BizContainerCreate(sys_memory_container_t* cid, size_t dwSize)
{
	return sys_memory_container_create(cid, dwSize);	
}

int BizContainerDestroy(sys_memory_container_t cid)
{
	return sys_memory_container_destroy(cid);
}

#endif
