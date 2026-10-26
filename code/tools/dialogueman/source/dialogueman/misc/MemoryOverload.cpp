#include "stdafx.h"

/*
#ifndef _DEBUG

#include "system\memory\blockallocator.h"
#include <System\Memory.h>

static int g_BlockAllocatorSizes[] = {
		4,8,12,16,20,24,28,32,36,40,44,48,56,60,64,76,80,92,96,116,128,136,160,232,256,0
	};

Memory::BlockAllocator g_Allocator(g_BlockAllocatorSizes);

void* operator new[]( size_t size )
{
	void *p;
	if (g_Allocator.Initialized() && ((p = g_Allocator.Allocate((U32)size)) != 0))
		return p;

	return malloc(size);
}

void operator delete[]( void* mem )
{
	if (g_Allocator.MemoryOwnedByAllocator(mem))
                g_Allocator.Free(mem);
        else
		free(mem);
}

void *operator new (size_t size)
{
	void *p;
	if (g_Allocator.Initialized() && ((p = g_Allocator.Allocate((U32)size)) != 0))
		return p;
 
	return malloc(size);
}
 
void operator delete(void *mem)
{
	if (g_Allocator.MemoryOwnedByAllocator(mem))
                g_Allocator.Free(mem);
        else
		free(mem);
}

#endif
*/