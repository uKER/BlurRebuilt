#include "CorePch.h"
#include <core/allocators/CoreMemoryFunctions.h>

/*
	These functions are authorised to use Malloc as they are designed for use by
	the memory tracking system (to avoid recursive meltdown).
*/

void* BizNonTrackedMalloc(size_t dwSize)
{
	return malloc(dwSize);
}

void BizNonTrackedFree(void* lpAddress)
{
	free(lpAddress);
}

size_t BizNonTrackedSize(void* lpAddress)
{
	if( lpAddress == NULL )
		return 0;

#if !defined( COMPILER_VC )
	return malloc_usable_size(lpAddress);
#else
	return _msize(lpAddress);
#endif
}

void* BizNonTrackedAlignedMalloc(size_t dwSize, size_t dwAlignment)
{
#if !defined( COMPILER_VC )
	return memalign(dwAlignment, dwSize);
#else
	return _aligned_malloc(dwSize, dwAlignment);
#endif
}

void BizNonTrackedAlignedFree(void* lpAddress)
{
#if !defined( COMPILER_VC )
	free(lpAddress);
#else
	_aligned_free(lpAddress);
#endif
}

#if defined(PLATFORM_PC)

size_t Biz_aligned_msize(void *memblock)
{
	#define PTR_SZ sizeof(void *)

    uintptr_t ptr;

    if (memblock == NULL)
        return 0;

    ptr = (uintptr_t)memblock;

    /* ptr points to the pointer to starting of the memory block */
    ptr = (ptr & ~(PTR_SZ -1)) - PTR_SZ;

#ifdef _DEBUG
	ptr -= sizeof(void*);	// Take into account the 4-byte under-run buffer-check stuff [Blur2PC][x64] the CRT header pads it to pointer size
#endif

    /* ptr is the pointer to the start of memory block*/
    ptr = *((uintptr_t *)ptr);
    
	return _msize((void *)ptr) - ((uintptr_t)memblock - ptr);	// [Blur2PC][x64] was long(memblock) - long(ptr)
}
#endif

size_t BizNonTrackedAlignedSize(void* lpAddress)
{
	if( lpAddress == NULL )
		return 0;

#if !defined( COMPILER_VC )
	return malloc_usable_size(lpAddress);
#else
	return Biz_aligned_msize(lpAddress);
#endif
}
