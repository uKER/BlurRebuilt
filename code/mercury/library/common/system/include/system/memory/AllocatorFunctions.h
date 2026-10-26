#ifndef __SYSTEM_ALLOCATORFUNCTIONS_H__
#define __SYSTEM_ALLOCATORFUNCTIONS_H__

// example usage:
//
//#include "system\memory\MemoryTrackerDefines.h"
//#include "system\memory\MemoryTracker.h"
//#include "system\memory\AllocatorFunctions.h"
//
//void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
//void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
//void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
//void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }
//void* BizAlignedMalloc(size_t size, size_t alignment) { return Memory::TrackedAlignedAlloc(size,alignment); }
//void BizAlignedFree(void *p) { Memory::TrackedAlignedFree(p); }


namespace Memory
{

//#if _MSC_FULL_VER <= 140050727
//#pragma warning("You need at least Visual Studio 2005 SP1 to compile this code.")
//#endif

/// malloc and free functions with support for alignment and memory tracking
/// these functions are meant to be used for overloading operator new and delete

/// the reason these function are inline is that they need access to MEMORY_TRACKER_FLAGS which is defined by the app
#ifdef PLATFORM_PC

// Aligned Memory Allocation Functions
void *TrackedAlignedAlloc(size_t size, size_t alignment)
{
#if 0//((defined _WIN64) || (defined WIN64))
	return tracked_alloc(size);
#else
	const int c = Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);

	int orgsize = (int)size;

	size += c;
	if (size < alignment)
		size = alignment;

	void *p = (orgsize > 0) ? _aligned_offset_malloc(size, alignment, c) : _aligned_malloc(size, alignment);
	if (!p)
	{
		throw std::bad_alloc();
		return NULL;
	}

#ifdef COMPILER_VC
    int iActual = (int)_aligned_msize(p,alignment,c);
#else
    int iActual = (int)malloc_usable_size();
#endif
 
	Memory::Tracker::Instance().TrackAllocation((char*)p + c,	// allocation address
                                     orgsize,    					    // original size
                                     iActual-c, 						// actual size (the difference is used to calculate memory wastage)
									 Memory::kMemType_System,			// memory 'type'
									 (Memory::AllocationNode*)p,		// address of tracking struct
                                     MEMORY_TRACKER_FLAGS);

	return (char*)p + c;
#endif
}

void TrackedAlignedFree(void *ptr)
{
#if 0//((defined _WIN64) || (defined WIN64))
	return tracked_free(ptr);
#else
	const int c = Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);

	if (!ptr)
		return;

	ptr = (char*)ptr - c;

#ifdef COMPILER_VC
    int iActual = (int)_aligned_msize(ptr,16,c);
#else
    int iActual = (int)malloc_usable_size(ptr);
#endif
    Memory::Tracker::Instance().TrackDeallocation(iActual-c, // actual allocation size (used to track memory wastage)
										Memory::kMemType_System,
										(Memory::AllocationNode*)ptr // address of tracking struct
                                       );
    _aligned_free(ptr);
#endif
}

#endif


// TODO: implement for other platforms

}

#endif
