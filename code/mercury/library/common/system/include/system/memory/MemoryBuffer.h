#ifndef DISPLAY_MEMORY_MEMORY_BUFFER_H
#define DISPLAY_MEMORY_MEMORY_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/vector.h"
#include "btl/containers/list.h"
//#include "btl/containers/map.h"
#include "btl/allocators/pool.h"
#include "btl/containers/skiplist.h"

//#ifndef CFG_SPEED
#ifndef CFG_GOLD
#define VERIFY_MEMORY_BUFFER_INTEGRITY
#endif

namespace Sys
{

struct MemoryBlock
{
	U32 size;
	const U8* address;
	//bool quick;
};

////////////////////////////////////////////////////////////

/// Allocates arrays of bytes from a larger pre-allocated buffer. @n
/// A block size must be specified when initialising the buffer.
/// Although any size 
/// may be allocated, the actual memory used will be a multiple of the block size and aligned 
/// to the block size.
/// Adjacent free blocks are merged together to reduce fragmentation. @n
class MemoryBuffer
{
public:
	MemoryBuffer();
	~MemoryBuffer();

	/// Create the memory buffer.
	bool Initialise(U32 sizeInBytes, U32 blockSize, U32 numQuickLists = 0);

	// Initialise and pass in a buffer to use.
	bool Initialise(U8* pBuffer, U32 sizeInBytes, U32 blockSize, U32 numQuickLists = 0);
	
	/// Free all the memory. Initialise() may be called again to use the allocator after this.
	// Shutdown() will only free memory that was allocated internally.
	void Shutdown();

	// Allocate and free memory.
	U8* Allocate(U32 sizeInBytes, U32 alignment = 1); ///< Allocate memory. The alignment must be a power of 2.
	void Free(U8* p, U32 sizeInBytes); ///< Free memory. The size parameter must be the size that was originally allocated.

	// information
	U32 GetTotalSize() const; ///< The total size of the memory buffer.
	U32 GetTotalAllocated() const; ///< The total amount allocated, including any alignment padding.
	U32 GetTotalFree() const; ///< The total amount free.
	U32 GetFailedCount() const; ///< The number of allocations which have failed since initialisation.
	U32 GetTotalRequested() const; ///< The total amount requested, not including any alignment padding.

	const U8* BufferBegin() const;
	const U8* BufferEnd() const;
	U32 BlockSize() const;

	// debug
	void PrintDebug(Debug::Logger& log) const;
	void GetStats(U32& count, U32& largest, U32& totalFree, U32& totalQuick) const;
	void GetFreeList(Btl::Vector<MemoryBlock>& blocks) const;

private:
	typedef Btl::SkipListMap<U8*, U32, Btl::DynamicPool<Btl::NullType> > MemoryMap;

	void MergeWithPrevious(MemoryMap::Iterator entry);
	void MergeWithNext(MemoryMap::Iterator entry);
	U8* AllocateFrom(MemoryMap::Iterator i, U32 size, U32 alignment);
	//bool ClearQuickLists();
	
	U8* m_pBuffer;
	U8* m_pMemoryAllocated;
	MemoryMap m_freeList;
#ifdef VERIFY_MEMORY_BUFFER_INTEGRITY
	MemoryMap m_allocatedList;
#endif
	U32 m_totalSize;
	U32 m_totalRequested;
	U32 m_totalAllocated;
	U32 m_allocationCount;
	U32 m_blockSize;
	U32 m_failedCount;

	// debug
	/*struct AllocationCount
	{
		int fast;
		int slow;
		int current;
	};

	Btl::Vector<AllocationCount> m_allocationCountList;
	int m_slowAllocations;
	int m_quickAllocations;*/

	// quick fit
	//typedef Btl::List<U8*, Btl::ListParameters_Pool<> > QuickList;
	//Btl::Vector<QuickList> m_quickFitLists;
};

}

#endif
