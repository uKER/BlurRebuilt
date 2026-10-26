#include "CorePch.h"
#include <core/allocators/CoreMemoryFunctions.h>

void* BizAlignedMalloc(size_t size, size_t alignment)
{
#if !defined( COMPILER_VC )
	return memalign(alignment, size);
#else
	return _aligned_malloc(size, alignment);
#endif
}

void BizAlignedFree(void* p)
{
#if !defined( COMPILER_VC )
	free(p);
#else
	_aligned_free(p);
#endif
}

void* BizHeapAlignedMalloc(size_t size, size_t alignment)
{
#if !defined( COMPILER_VC )
	return memalign(alignment, size);
#else
	return _aligned_malloc(size, alignment);
#endif
}

void BizHeapAlignedFree(void* p)
{
#if !defined( COMPILER_VC )
	free(p);
#else
	_aligned_free(p);
#endif
}
