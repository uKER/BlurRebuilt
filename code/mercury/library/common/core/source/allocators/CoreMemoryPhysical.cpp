#include "CorePch.h"
#include <core/allocators/CoreMemoryFunctions.h>

void* BizPhysicalAlloc(size_t dwSize, U32 ulPhysicalAddress, U32 ulAlignment, U32 flProtect)
{
	return NULL;
}

void BizPhysicalFree(void* lpAddress)
{
}

void* BizHeapPhysicalAlloc(size_t dwSize, U32 ulPhysicalAddress, U32 ulAlignment, U32 flProtect)
{
	return NULL;
}

void BizHeapPhysicalFree(void* lpAddress)
{
}
