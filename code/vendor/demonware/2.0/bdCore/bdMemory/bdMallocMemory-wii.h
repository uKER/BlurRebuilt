// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A non-pooled memory manager, which records all allocations and
//			deallocations so memory leaks can be detected.

#ifndef BD_MALLOC_MEMORY_WII_H
#define BD_MALLOC_MEMORY_WII_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdCore/bdThread/bdMutex.h>

#include <revolution/os.h>
#include <revolution/mem.h>

/// A non-pooled memory manager.
/// This set of memory functions implement a memory manager that allocates,
/// deallocates and resizes memory blocks on demand using the platforms native
/// memory routines. No attempt is made to preallocate memory so each call to
/// one of the allocation, deallocation or resizing functions will result in a
/// call to one of the platforms native memory management routines.
///
/// Each allocation is increased in size to accommodate a
/// bdMemoryChainElement that is used to record allocations and
/// detect memory overwriting. Allocations are prepended to the
/// bdMallocMemory::m_memoryChain linked list, and deallocations are removed
/// from the list. Hence if memory leaks are detected it is still possible to
/// deallocate all memory by calling releaseAllMemory.
///
/// The current number of allocations and number of
/// currently allocated bytes are also recorded.
class bdMallocMemory
{
	public:

		/// Initializes MEM1 and MEM2 memory heaps (wii only)
		static void BD_CALL init();
		
		/// Allocates an unaligned memory block.
		/// Unaligned memory is allocated via the bdPlatformMalloc function
		/// pointer. The allocation is also recorded.
		/// \param size[in] The size in bytes of the required memory block.
		/// \return
		/// - A pointer to the allocated memory block.
		/// - BD_NULL, if the memory could not be allocated.
		static void* BD_CALL allocate(const bdUWord size);

		static void* BD_CALL allocate2(const bdUWord size);

		/// Deallocates an unaligned memory block.
		/// Unaligned memory is deallocated via the bdPlatformFree function
		/// pointer. The allocation record is also removed.
		/// \param p[in] A pointer to the memory block to be deallocated.
		static void BD_CALL deallocate(void *p);

		/// Resizes an unaligned memory block.
		/// Unaligned memory is resized via the bdPlatformRealloc function
		/// pointer. The resize is also recorded.
		/// \param p[in] A pointer to the memory block to be resized.
		/// \param size[in] The new size in bytes of the memory block.
		/// \return
		/// - A pointer to the allocated memory block.
		/// - BD_NULL, if the memory could not be resized.
		static void* BD_CALL reallocate(void *p,
										const bdUWord size);
											   
		static void* BD_CALL reallocate2(void *p,
										 const bdUWord size);

		/// Allocates an aligned memory block.
		/// Aligned memory is allocated via the bdPlatformAlignedMalloc function
		/// pointer. The allocation is also recorded.
		/// \param size[in] The size in bytes of the required memory block.
		/// \param align[in] The required memory alignment, must be a power of 2.
		/// \return
		/// - A pointer to the allocated memory block.
		/// - BD_NULL, if the memory could not be allocated.
		static void* BD_CALL alignedAllocate(const bdUWord size,
											 const bdUWord align);

		static void* BD_CALL alignedAllocate2(const bdUWord size,
											  const bdUWord align);

		/// Deallocates an aligned memory block.
		/// Aligned memory is deallocated via the bdPlatformAlignedFree function
		/// pointer. The allocation record is also removed.
		/// \param p[in] A pointer to the memory block to be deallocated.
		static void BD_CALL alignedDeallocate(void *p);
		
        /// Resizes an aligned memory block.
		/// Aligned memory is resized via the bdPlatformAlignedRealloc function
		/// pointer. The resize is also recorded.
		/// \param p[in] A pointer to the memory block to be resized.
		/// \param size[in] The new size in bytes of the memory block.
		/// \param align[in] The required memory alignment, must be a power of 2.
		/// \return
		/// - A pointer to the allocated memory block.
		/// - BD_NULL, if the memory could not be resized.
		static void* BD_CALL alignedReallocate(void *p,
											   const bdUWord size,
											   const bdUWord align);

		static void* BD_CALL alignedReallocate2(void *p,
											    const bdUWord size,
											    const bdUWord align);

		/// Prints details of currently allocated memory blocks.
		/// This function is called automatically from bdCore::quit to warn
		/// of any memory leaks.
		static void BD_CALL leakCheck();
		

		/// Deallocates all memory allocated through bdMallocMemory.
		static void BD_CALL releaseAllMemory();
		

	private:

		/// Memory block record.
		/// This struct is prepended each memory allocation so that it
		/// can be added to and removed from the linked list of allocated
		/// memory blocks (bdMallocMemory::m_memoryChain).
		struct bdMemoryChainElement
		{
			/// The BD_MEMORY_MAGIC bit pattern
			bdUInt16 m_magic;

			/// The size of the memory block.
			/// \note This does not include the size of a prepended
			/// bdMemoryChainElement.
			bdUWord m_size;

			/// Indicates if the memory block was allocated with specific
			/// alignment requirements.
			bdBool m_aligned;

			/// Indicates which memory heap the block was allocated from: MEM1 or MEM2
			bdUByte8 m_heapNum;
			
			/// A pointer to the previous memory block record.
			bdMemoryChainElement *m_prev;

			/// A pointer to the next memory block record.
			bdMemoryChainElement *m_next;
		};

	private:

		/// Record a memory block.
		/// Initializes and adds a bdMemoryChainElement to the linked list
		/// of allocated memory blocks. Increments the allocation counters.
		/// \param link[in] A pointer to the start of the newly allocated
		///					memory block plus bdMemoryChainElement header.
		/// \param size[in] The size of the memory block pointer to by \a link,
		///					not including the size of the bdMemoryChainElement
		///					header.
		/// \param aligned[in] Should be true if the memory being recorded was
		///					   allocated with specific alignment.
		/// \return A pointer to the start of the usable memory block.
		static void* BD_CALL recordMemory(bdMemoryChainElement *link,
										  const bdUWord size,
										  const bdBool aligned,
										  const bdUByte8 heapNum);

		/// Remove a memory block record.
		/// Removes a bdMemoryChainElement record from the linked list
		/// of allocated memory blocks. Decrements the allocation counters.
		/// \param link[in] A pointer to the bdMemoryChainElement of the memory
		///					block to be removed.
		static void BD_CALL eraseMemory(bdMemoryChainElement *link);

		static void* BD_CALL alignedOffsetMalloc(const bdUWord size,
											   	 const bdUWord align,
									             const bdUWord offset,
												 const bdUByte8 heapNum);

		static void BD_CALL alignedOffsetFree(bdMemoryChainElement *link);
		
		static void* BD_CALL alignedOffsetRealloc(bdMemoryChainElement *link,
									 const bdUWord origSize,
									 const bdUWord size,
									 const bdUWord align,
									 const bdUWord offset,
									 const bdUByte8 heapNum);

		/// Private constructor.
		/// There should be no need to new this class as everything is static.
		bdMallocMemory();

	private:

		/// A pointer to the head of a linked list of bdMemoryChainElements.
		/// This list if used to record all currently allocated memory blocks.
		static bdMemoryChainElement *m_memoryChain;
		static MEMHeapHandle m_handles[2];

	public:

		/// The total number of bytes currently allocated.
		static bdUWord m_allocatedBytes;

		/// The total number of memory blocks currently allocated.
		static bdUInt m_numAllocations;

		static bdMutex m_mutex;
};

#endif // BD_MALLOC_MEMORY_WII_H
