
#ifndef __ALLOCATORS_CORE_MEMORY_ALIGNED_H__
#define __ALLOCATORS_CORE_MEMORY_ALIGNED_H__

#ifdef _MSC_VER
#pragma once
#endif

void* BizAlignedMalloc(size_t size, size_t alignment);
void BizAlignedFree(void* p);

void* BizPhysicalAlloc(size_t dwSize, U32 ulPhysicalAddress, U32 ulAlignment, U32 flProtect);
void BizPhysicalFree(void* lpAddress);

/*
	Used by memory tracking system and heaps to split memory usage out - don't use (See Roger)
*/
void* BizHeapAlignedMalloc(size_t size, size_t alignment);
void BizHeapAlignedFree(void* p);
void* BizHeapPhysicalAlloc(size_t dwSize, U32 ulPhysicalAddress, U32 ulAlignment, U32 flProtect);
void BizHeapPhysicalFree(void* lpAddress);

// There's no reason to use these except inside the memory tracking system
void* BizNonTrackedMalloc(	size_t dwSize);
void BizNonTrackedFree(		void* lpAddress);
size_t BizNonTrackedSize(	void* lpAddress);

void* BizNonTrackedAlignedMalloc(size_t dwSize, size_t dwAlignment);
void BizNonTrackedAlignedFree(void* lpAddress);
size_t BizNonTrackedAlignedSize(void* lpAddress);

#ifdef PLATFORM_PS3
int BizContainerCreate(sys_memory_container_t* cid, size_t dwSize);
int BizContainerDestroy(sys_memory_container_t cid);
#endif

#endif
