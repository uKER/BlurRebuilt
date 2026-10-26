#ifndef SAMPLES_MEMORY_H
#define SAMPLES_MEMORY_H

#ifdef _MSC_VER
#pragma once
#endif
/*
INLINE void* operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}
 
INLINE void operator delete(void* ptr)
{
	_aligned_free(ptr);
}

INLINE void* operator new[](size_t size)
{
	return _aligned_malloc(size, 16);
}

INLINE void operator delete[](void* ptr)
{
	_aligned_free(ptr);
}
*/
#endif // SAMPLES_MEMORY_H
