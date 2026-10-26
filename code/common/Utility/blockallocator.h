#pragma once

#include "utility\vector.h"
#include "utility\threads.h"
#include "utility\sortedbinarytree.h"
//#include <set>

namespace Ut
{

/// @addtogroup Allocators
/// @{

template <U32 sysMemTrackID>
class PhysicalAllocator
{
public:
	/// Allocate objects.
	static U8* Allocate(int n) 
	{
		return (U8*) PhyMalloc(sysMemTrackID, n, 64 * 1024, PAGE_READWRITE); // 64k page size
	}

	/// Free objects.
	static void Free(U8* p, int n)
	{
		if (p)
			PhyFree(p);
	}

	//template <typename U> struct Rebind {typedef PhysicalAllocator<sysMemTrackID> Other;};
};

////////////////////////////////////////////////////////////

/// Block Allocator.
/// Allocates arrays of bytes from a larger pre-allocated buffer. @n
/// A block size must be specified when initialising the buffer.
/// Although any size 
/// may be allocated, the actual memory used will be a multiple of the block size and aligned 
/// to the block size.
/// Adjacent free blocks are merged together to reduce fragmentation. @n
/// @param Allocator The allocator used to create the buffer. Must be an array allocator of bytes
/// @param ThreadingModel See @ref Threading_Models.
template<typename Allocator = ArrayAllocator<U8>, typename ThreadingModel = SingleThreaded>
class BlockAllocator
{
public:
	BlockAllocator() : m_totalSize(0) {}
	~BlockAllocator() {Shutdown();}

	/// Allocate the memory buffer.
	void Initialise(U32 blockSize, U32 totalSizeInBytes); 
	
	/// Free all the memory. Initialise() should be called again to use the allocator after this.
	void Shutdown();

	// Allocate and free memory.
	U8* Allocate(U32 sizeInBytes); ///< Allocate memory.
	void Free(U8* p, U32 sizeInBytes); ///< Free memory. The size parameter must be the size that was originally allocated.

	// information
	U32 GetTotalSize() const {return m_totalSize;} ///< The total size of the memory buffer.
	U32 GetTotalAllocated() const {return m_totalAllocated;} ///< The total amount allocated, including any alignment padding.
	U32 GetTotalFree() const {return m_totalSize - m_totalAllocated;} ///< The total amount free.
	U32 GetFailedCount() const {return m_failedCount;} ///< The number of allocations which have failed since initialisation.
	U32 GetTotalRequested() const {return m_totalRequested;} ///< The total amount requested, not including any alignment padding.

	const U8* BufferBegin() const {return &m_buffer[0];}
	const U8* BufferEnd() const {return &m_buffer[0] + m_totalSize;}

	// debug
	void PrintDebug() const;

private:
	ThreadingModel m_threadData;
	DynamicArrayData<U8, Allocator> m_buffer;
	typedef Ut::UtMap<U8*, U32> MemoryMap;
	MemoryMap m_freeList;
	//MemoryMap m_usedList; // debug
	U32 m_totalSize;
	U32 m_totalRequested;
	U32 m_totalAllocated;
	U32 m_allocationCount;
	U32 m_blockSize;
	U32 m_failedCount;
	//MemoryMap::iterator m_current;

	void MergeWithPrevious(MemoryMap::Iterator entry);
	void MergeWithNext(MemoryMap::Iterator entry);
	U32 AllocationSize(U32 size) const;
	//void IncrementCurrent();
	void RemoveFromFreeList(MemoryMap::Iterator i);
	MemoryMap::Iterator AddToFreeList(U8* p, U32 size);
	U8* AllocateFrom(MemoryMap::Iterator i, U32 size);
};

/// A memory allocator which has two buffers.
/// The two buffers can have different block sizes.
/// The advantages are:
/// @li Small allocations have less alignment overhead as they come from a buffer with a smaller block size.
/// @li The larger buffer is not fragmented by small allocations.
/// If the small buffer becomes full, all allocations will be made from the main buffer.
/// See BlockAllocator.
template<typename Allocator = ArrayAllocator<U8>, typename ThreadingModel = SingleThreaded>
class SplitAllocator
{
public:
	SplitAllocator() : m_splitSize(0) {}

	void InitialiseMain(U32 blockSize, U32 sizeInBytes) {mainAllocator.Initialise(blockSize, sizeInBytes);} ///< @a
	void InitialiseSmall(U32 blockSize, U32 sizeInBytes) {smallAllocator.Initialise(blockSize, sizeInBytes);} ///< @a
	void SetSplitSize(U32 sizeInBytes) {m_splitSize = sizeInBytes;} ///< Allocations smaller than the split size are preferentially made from the small buffer.
	
	/// Free all the memory.
	void Shutdown() {mainAllocator.Shutdown(); smallAllocator.Shutdown()}

	/// Allocate memory.
	U8* Allocate(U32 sizeInBytes)
	{
		if (sizeInBytes < m_splitSize)
		{
			U8* p = smallAllocator.Allocate(sizeInBytes);
			if (p) return p;
		}

		return mainAllocator.Allocate(sizeInBytes);
	}

	/// Free memory. The size parameter must be the size that was originally allocated.
	void Free(U8* p, U32 sizeInBytes)
	{
		if (p >= smallAllocator.BufferBegin() && p < smallAllocator.BufferEnd()) smallAllocator.Free(p, sizeInBytes);
		else mainAllocator.Free(p, sizeInBytes);
	}

	// information
	U32 GetTotalSize() const {return mainAllocator.GetTotalSize() + smallAllocator.GetTotalSize();} ///< @a
	U32 GetTotalAllocated() const {return mainAllocator.GetTotalAllocated() + smallAllocator.GetTotalAllocated();} ///< @a 
	U32 GetTotalFree() const {return mainAllocator.GetTotalFree() + smallAllocator.GetTotalFree();} ///< @a
	U32 GetFailedCount() const {return mainAllocator.GetFailedCount();} ///< @a
	U32 GetTotalRequested() const {return mainAllocator.GetTotalRequested() + smallAllocator.GetTotalRequested();} ///< @a

	// debug
	void PrintDebugMain() const {mainAllocator.PrintDebug();}
	void PrintDebugSmall() const {smallAllocator.PrintDebug();}

private:
	U32 m_splitSize;
	BlockAllocator<Allocator, ThreadingModel> mainAllocator;
	BlockAllocator<Allocator, ThreadingModel> smallAllocator;
};

/// @}

}