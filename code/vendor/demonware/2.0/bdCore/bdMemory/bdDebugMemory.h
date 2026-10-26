// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DEBUG_MEMORY_H
#define BD_DEBUG_MEMORY_H

// PURPOSE: A non-pooled memory manager, which records all allocations and
//			deallocations so memory leaks can be detected. The record comprises
//			of allocation information and a full stack trace at the point of
//			allocation.

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdCore/bdContainers/bdFastArray.h>

#ifndef BD_PLATFORM_WIN32
#error bdDebugMemory is for win32 platforms only.
#endif // BD_PLATFORM_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dbghelp.h>

/// The BitDemon memory magic pattern.
/// BD_MEMORY_MAGIC is written into the memory header for each allocation.
/// Upon deallocation a BD_ASSERT is thrown if the pattern has changed,
/// indicating that the memory has been overwritten.
#define BD_MEMORY_MAGIC (0xbdbd)

/// A non-pooled debug memory manager.
/// This set of memory functions implement a memory manager that allocates,
/// deallocates and resizes memory blocks on demand using the platforms native
/// memory routines. As no attempt is made to preallocate memory; each call to
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
/// The bdMemoryChainElement stores enough information
/// to enable a full stack trace to be output for each allocation. This
/// can be a big help in tracking the source of memory leaks.
///
/// The current number of allocations and number of
/// currently allocated bytes are also recorded.
class bdDebugMemory
{
	public:

		/// Allocates an unaligned memory block.
		/// Unaligned memory is allocated via the bdPlatformMalloc function
		/// pointer. The allocation is also recorded.
		/// \param size[in] The size in bytes of the required memory block.
		/// \return
		/// - A pointer to the allocated memory block.
		/// - BD_NULL, if the memory could not be allocated.
		static void* BD_CALL allocate(const bdUWord size);

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

		/// Deallocates an aligned memory block.
		/// Aligned memory is deallocated via the bdPlatformAlignedFree function
		/// pointer. The allocation record is also removed.
		/// \param p[in] A pointer to the memory block to deallocated.
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

		/// Outputs the details of all memory block allocations.
		/// A summary of the current memory allocations is output, followed
		/// be a detailed report (including stack trace) of each individual
		/// block allocation.
		/// \param output[in] The output stream to which the report should be
		///					  printed.
		static void BD_CALL dumpMemory();

		/// Cleans up the memory used to produce the stack trace.
		static void BD_CALL cleanup();

		/// Prints details of currently allocated memory blocks.
		/// This function is called automatically from bdCore::quit to warn
		/// of any memory leaks.
		static void BD_CALL leakCheck();

		/// Deallocates all memory allocated through bdDebugMemory.
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
			/// bdMemoryChainElement
			bdUWord m_size;

			/// Indicates if the memory block was allocated with specific
			/// alignment requirements.
			bdBool m_aligned;

			/// A pointer to the previous memory block record.
			bdMemoryChainElement *m_prev;

			/// A pointer to the next memory block record.
			bdMemoryChainElement *m_next;

			/// The stack trace.
			/// An array of line addresses that are later resolved to lines.
			bdFastArray<DWORD64> m_stack;
		};

		/// Record a memory block.
		/// Initializes and adds a bdMemoryChainElement to the linked list
		/// of allocated memory blocks. Increments the allocation counters.
		/// \param link[in]		A pointer to the start of the newly allocated
		///						memory block plus bdMemoryChainElement header.
		/// \param size[in]		The size of the memory block pointer to by \a
		///						link, not including the size of the
		///						bdMemoryChainElement header.
		/// \param aligned[in]	A flag to indicate if the memory is aligned.
		/// \return A pointer to the start of the usable memory block.
		static void* BD_CALL recordMemory(bdMemoryChainElement *link,
										  const bdUWord size,
										  const bdBool aligned);

		/// Remove a memory block record.
		/// Removes a bdMemoryChainElement record from the linked list
		/// of allocated memory blocks. Decrements the allocation counters.
		/// \param link[in] A pointer to the bdMemoryChainElement of the memory
		///					block to be removed.
		static void BD_CALL eraseMemory(bdMemoryChainElement *link);

		/// Attempts to load the imagehlp.dll and initialise the symbol
		/// handler.
		static void BD_CALL initialize();

		/// Outputs a stack trace to an output stream.
		/// \param stack[in] An array of line addresses that constitute a
		///					 call stack.
		/// \param output[in] The stream to which the stack should be printed.
		static void BD_CALL dumpStackTrace(bdFastArray<DWORD64>& stack);

		/// Constructs the stack in a bdMemoryChainElement.
		/// Calls placement new on link->m_stack. This is only called from
		/// recordMemory but must be in a separate function otherwise the compiler
		/// will complain with error C2712: "Cannot use __try in functions that
		/// require object unwinding".
		///	The __try {} __except() {} construct is used to get the current
		/// threads context. If we find a better way to get a valid thread
		/// CONTEXT this can be removed.
		/// \param link[in,out] A pointer to the link whose m_stack member is
		///						to be constructed.
		static void BD_CALL initStack(bdMemoryChainElement *const link);

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdDebugMemory();

	private:

		/// A pointer to the head of a linked list of bdMemoryChainElements.
		/// This list if used to record all currently allocated memory blocks.
		static bdMemoryChainElement *m_memoryChain;

		/// The total number of bytes currently allocated.
		static bdUWord m_allocatedBytes;

		/// The total number of memory blocks currently allocated.
		static bdUInt m_numAllocations;

		/// A flag to indicate that initialize was called successfully.
		static bdBool m_initialized;

		/// A flag to specify if memory allocations should be recorded.
		/// This is set to false anywhere that memory is allocated or
		/// deallocated within that class to avoid recursive recording.
		static bdBool m_recording;
};

#endif // BD_DEBUG_MEMORY_H
