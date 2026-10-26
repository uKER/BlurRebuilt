// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Plugable memory management.
#ifndef BD_MEMORY_WII_H
#define BD_MEMORY_WII_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// A pluggable memory manager.
/// All memory allocation and deallocation goes through this class.
/// The function pointers contained in this class should be assigned
/// to appropriate memory allocation functions. In this way you can
/// assign memory using your preferred strategy.
class bdMemory
{
	public:

		//
		// Typedefs
		//

		/// Defines a function pointer type for allocating unaligned memory.
		/// A bdMemory::bdMallocFunc function should return a pointer
		/// to a new block of memory of at least \a size bytes.
		/// \param size[in] The size in bytes of the required memory block.
		/// \return A pointer to the allocated memory block.
		typedef void* (BD_CALL *bdAllocateFunc)(const bdUWord size);

		/// Defines a function pointer type for deallocating unaligned memory.
		/// A bdMemory::bdFreeFunc should deallocate a memory block previously allocated
		/// using a bdMemory::bdMallocFunc or bdMemory::bdReallocFunc type function.
		/// \param p[in] A pointer to the memory block to be deallocated.
		typedef void (BD_CALL *bdDeallocateFunc)(void *p);

		/// Defines a function pointer type for resizing unaligned memory blocks.
		/// The bdMemory::bdReallocFunc is used to resize memory blocks allocated using a
		/// bdMemory::bdMallocFunc or bdMemory::bdReallocFunc type function. Returns a pointer to a
		/// memory block of at least \a size bytes. The pointer returned may
		/// have the same value as \a p, and the first \a size bytes should be
		/// identical to those of \a p.
		/// \param p[in] A pointer to the memory block to be resized.
		/// \param size[in] The new size in bytes of the memory block.
		/// \return A pointer to the allocated memory block.
		typedef void* (BD_CALL *bdReallocateFunc)(void *p,
												  const bdUWord size);

		/// Defines a function pointer type for allocating aligned memory.
		/// A bdMemory::bdMallocFunc function should return a pointer, aligned on an \a align
		/// byte boundary, to a new block of memory of at least \a size bytes.
		/// \param size[in] The size in bytes of the required memory block.
		/// \param align[in] The required memory alignment, must be a power of 2.
		/// \return A pointer to the allocated memory block.
		typedef void* (BD_CALL *bdAlignedAllocateFunc)(const bdUWord size,
													   const bdUWord align);

		/// Defines a function pointer type for deallocating aligned memory.
		/// A bdMemory::bdAlignedFreeFunc should deallocate a memory block previously
		/// allocated using a bdMemory::bdAlignedMallocFunc or bdMemory::bdAlignedReallocFunc
		/// type function.
		/// \param p[in] A pointer to the memory block to be deallocated.
		typedef void (BD_CALL *bdAlignedDeallocateFunc)(void *p);

		/// Defines a function pointer type for resizing aligned memory blocks.
		/// The bdMemory::bdAlignedReallocFunc is used to resize memory blocks allocated
		/// using a bdMemory::bdAlignedMallocFunc or bdMemory::bdAlignedReallocFunc type function.
		/// Returns a pointer to a memory block of at least \a size bytes, which
		/// is aligned to an \a align byte boundary. The pointer returned may
		/// have the same value as \a p, and the first \a size bytes should be
		/// identical to those of \a p.
		/// \param p[in] A pointer to the memory block to be resized.
		/// \param size[in] The new size in bytes of the memory block.
		/// \param align[in] The required memory alignment, must be a power of 2.
		/// \return A pointer to the allocated memory block.
		typedef void* (BD_CALL *bdAlignedReallocateFunc)(void *p,
														 const bdUWord size,
														 const bdUWord align);

		//
		// Set methods
		//

		/// Sets the function through which unaligned memory will be allocated.
		/// \param allocator[in] A pointer to the function to be called
		///						 to allocate unaligned memory blocks.
		static void BD_CALL setAllocateFunc(const bdAllocateFunc allocator);

		static void BD_CALL setAllocateFunc2(const bdAllocateFunc allocator);

		/// Sets the function via which unaligned memory will be freed.
		/// \param deallocator[in] A pointer to the function to be called to
		///						   deallocate unaligned memory blocks.
		static void BD_CALL setDeallocateFunc(const bdDeallocateFunc deallocator);

		static void BD_CALL setDeallocateFunc2(const bdDeallocateFunc deallocator);

		/// Sets the function via which unaligned memory will be reallocated.
		/// \param reallocator[in] A pointer to the function to be called to
		///						   resize unaligned memory blocks.
		static void BD_CALL setReallocateFunc(const bdReallocateFunc reallocator);

		static void BD_CALL setReallocateFunc2(const bdReallocateFunc reallocator);

		/// Sets the function through which aligned memory will be allocated.
		/// \param allocator[in] A pointer to the function to be called
		///						 to allocate aligned memory blocks.
		static void BD_CALL setAlignedAllocateFunc(const bdAlignedAllocateFunc allocator);

		static void BD_CALL setAlignedAllocateFunc2(const bdAlignedAllocateFunc allocator);

		/// Sets the function via which aligned memory will be freed.
		/// \param deallocator[in] A pointer to the function to be called to
		///						   deallocate aligned memory blocks.
		static void BD_CALL setAlignedDeallocateFunc(const bdAlignedDeallocateFunc deallocator);

		static void BD_CALL setAlignedDeallocateFunc2(const bdAlignedDeallocateFunc deallocator);

		/// Sets the function via which aligned memory will be reallocated.
		/// \param reallocator[in] A pointer to the function to be called to
		///						   resize aligned memory blocks.
		static void BD_CALL setAlignedReallocateFunc(const bdAlignedReallocateFunc reallocator);

		static void BD_CALL setAlignedReallocateFunc2(const bdAlignedReallocateFunc reallocator);

		//
		// Get methods
		//

		/// Get the function used for allocating unaligned memory blocks.
		/// \return A pointer to the function used to allocate unaligned
		///			memory blocks.
		static bdAllocateFunc BD_CALL getAllocateFunc();

		static bdAllocateFunc BD_CALL getAllocateFunc2();

		/// Get the function used to free unaligned memory blocks.
		/// \return A pointer to the function used to deallocate unaligned
		///			memory blocks.
		static bdDeallocateFunc BD_CALL getDeallocateFunc();

		static bdDeallocateFunc BD_CALL getDeallocateFunc2();

		/// Get the function used to resize unaligned memory blocks.
		/// \return A pointer to the function used to resize unaligned
		///			memory blocks.
		static bdReallocateFunc BD_CALL getReallocateFunc();

		static bdReallocateFunc BD_CALL getReallocateFunc2();

		/// Get the function used for allocating aligned memory blocks.
		/// \return A pointer to the function used to allocate aligned
		///			memory blocks.
		static bdAlignedAllocateFunc BD_CALL getAlignedAllocateFunc();

		static bdAlignedAllocateFunc BD_CALL getAlignedAllocateFunc2();

		/// Get the function used to free aligned memory blocks.
		/// \return A pointer to the function used to deallocate aligned
		///			memory blocks.
		static bdAlignedDeallocateFunc BD_CALL getAlignedDeallocateFunc();

		static bdAlignedDeallocateFunc BD_CALL getAlignedDeallocateFunc2();

		/// Get the function used to resize aligned memory blocks.
		/// \return A pointer to the function used to resize aligned
		///			memory blocks.
		static bdAlignedReallocateFunc BD_CALL getAlignedReallocateFunc();

		static bdAlignedReallocateFunc BD_CALL getAlignedReallocateFunc2();

		//
		// Memory allocation and deallocation methods
		//

		/// Allocates an unaligned memory block.
		/// Calls the memory allocation function set using bdMemory::setMallocFunc.
		/// \param size[in] The size in bytes of the required memory block.
		/// \return
		/// - The pointer returned by calling the function set using bdMemory::setMallocFunc.
		/// - BD_NULL, if no bdMemory::bdMallocFunc has been set.
		static void* BD_CALL allocate(const bdUWord size);

		static void* BD_CALL allocate2(const bdUWord size);

		/// Deallocates an unaligned memory block.
		/// Calls the memory deallocation function set using bdMemory::setFreeFunc.
		/// If no such function has been set the function returns immediately.
		/// \param p[in] A pointer to the memory block to be deallocated.
		static void BD_CALL deallocate(void *p);

		static void BD_CALL deallocate2(void *p);

		/// Resizes an unaligned memory block.
		/// Calls the memory reallocation function set using bdMemory::setReallocFunc.
		/// \param p[in] A pointer to the memory block to be resized.
		/// \param size[in] The size in bytes of the required memory block.
		/// \return
		/// - The pointer returned by calling the function set using
		/// bdMemory::setReallocFunc.
		/// - BD_NULL, if no bdMemory::bdReallocFunc has been set.
		static void* BD_CALL reallocate(void *p,
									    const bdUWord size);

		static void* BD_CALL reallocate2(void *p,
										 const bdUWord size);

		/// Allocates an aligned memory block.
		/// Calls the memory allocation function set using bdMemory::setAlignedMallocFunc.
		/// \param size[in] The size in bytes of the required memory block.
		/// \param align[in] The required memory alignment, must be a power of 2.
		/// \return
		/// - The pointer returned by calling the function set using
		/// bdMemory::setAlignedMallocFunc.
		/// - BD_NULL, if no bdMemory::bdAlignedMallocFunc has been set.
		static void* BD_CALL alignedAllocate(const bdUWord size,
											 const bdUWord align = 16);

		static void* BD_CALL alignedAllocate2(const bdUWord size,
											  const bdUWord align = 16);

		/// Deallocates an aligned memory block.
		/// Calls the memory deallocation function set using bdMemory::setAlignedFreeFunc.
		/// If no such function has been set the function returns immediately.
		/// \param p[in] A pointer to the memory block to be deallocated.
		static void BD_CALL alignedDeallocate(void *p);

		static void BD_CALL alignedDeallocate2(void *p);

		/// Resizes an aligned memory block.
		/// Calls the memory reallocation function set using bdMemory::setAlignedReallocFunc.
		/// \param p[in] A pointer to the memory block to be resized.
		/// \param size[in] The size in bytes of the required memory block.
		/// \param align[in] The required memory alignment, must be a power of 2.
		/// \return
		/// - The pointer returned by calling the function set using
		/// bdMemory::setAlignedReallocFunc.
		/// - BD_NULL, if no bdMemory::bdAlignedReallocFunc has been set.
		static void* BD_CALL alignedReallocate(void *p,
											   const bdUWord size,
											   const bdUWord align = 16);

		static void* BD_CALL alignedReallocate2(void *p,
											    const bdUWord size,
											    const bdUWord align = 16);


	protected:

		/// A pointer to the function used by bdMemory::malloc to
		/// allocate memory.
		static bdAllocateFunc m_allocateFunc;

		static bdAllocateFunc m_allocateFunc2;

		/// A pointer to the function used by bdMemory::free to
		/// deallocate memory.
		static bdDeallocateFunc m_deallocateFunc;

		static bdDeallocateFunc m_deallocateFunc2;

		/// A pointer to the function used by bdMemory::realloc to
		/// reallocate memory.
		static bdReallocateFunc m_reallocateFunc;

		static bdReallocateFunc m_reallocateFunc2;

		/// A pointer to the function used by bdMemory::alignedMalloc to
		/// allocate aligned memory.
		static bdAlignedAllocateFunc m_alignedAllocateFunc;

		static bdAlignedAllocateFunc m_alignedAllocateFunc2;

		/// A pointer to the function used by bdMemory::alignedFree to
		/// deallocate aligned memory.
		static bdAlignedDeallocateFunc m_alignedDeallocateFunc;

		static bdAlignedDeallocateFunc m_alignedDeallocateFunc2;

		/// A pointer to the function used by bdMemory::realloc to
		/// reallocate aligned memory.
		static bdAlignedReallocateFunc m_alignedReallocateFunc;

		static bdAlignedReallocateFunc m_alignedReallocateFunc2;

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdMemory();
};

/// Allocates an unaligned memory block of at least size n*sizeof(T) bytes.
/// This function forwards to bdMemory::malloc.
/// \param n[in] The number of objects of type T to be allocated.
/// \return
/// - A pointer to the newly allocated memory block.
/// - BD_NULL, if the memory manager failed to allocate the block.
template <typename T> inline
T* bdAllocate(const bdUWord n);

template <typename T> inline
T* bdAllocate2(const bdUWord n);

/// Deallocates memory previously allocated with bdAllocate<T>.
/// This function forwards to bdMemory::free.
/// \param p[in] A pointer to the memory block to be deallocated.
template <typename T> inline
void bdDeallocate(T *p);

template <typename T> inline
void bdDeallocate2(T *p);

/// Resizes an unaligned memory block to at least size n*sizeof(T) bytes.
/// This function forwards bdMemory::reallocate.
/// \param p[in] A pointer to the memory block to be resized.
/// \param n[in] The number of objects of type T to be allocated.
/// \return
/// - A pointer to the newly resized memory block.
/// - BD_NULL, if the memory manager failed to resize the block.
template <typename T> inline
T* bdReallocate(T *p,
				const bdUWord n);

template <typename T> inline
T* bdReallocate2(T *p,
				 const bdUWord n);

/// Allocates an aligned memory block of at least size n*sizeof(T) bytes.
/// This function forwards to bdMemory::alignedMalloc.
/// \param n[in] The number of objects of type T to be allocated.
/// \param align[in] The required memory alignment, must be a power of 2.
/// \return
/// - A pointer to the newly allocated memory block.
/// - BD_NULL, if the memory manager failed to allocate the block.
template <typename T> inline
T* bdAlignedAllocate(const bdUWord n,
					 const bdUWord align = 16);

template <typename T> inline
T* bdAlignedAllocate2(const bdUWord n,
					  const bdUWord align = 16);

/// Deallocates memory previously allocated with bdAlignedAllocate<T>.
/// This function forwards to bdMemory::alignedFree.
/// \param p[in] A pointer to the memory block to be deallocated.
template <typename T> inline
void bdAlignedDeallocate(T *p);

template <typename T> inline
void bdAlignedDeallocate2(T *p);

/// Resizes an aligned memory block to at least size n*sizeof(T) bytes.
/// This function forwards bdMemory::alignedReallocate.
/// \param p[in] A pointer to the memory block to be resized.
/// \param n[in] The number of objects of type T to be allocated.
/// \param align[in] The required memory alignment, must be a power of 2.
/// \return
/// - A pointer to the newly resized memory block.
/// - BD_NULL, if the memory manager failed to resize the block.
template <typename T> inline
T* bdAlignedReallocate(T *p,
					   const bdUWord n,
					   const bdUWord align = 16);

template <typename T> inline
T* bdAlignedReallocate2(T *p,
					    const bdUWord n,
					    const bdUWord align = 16);


#include <bdCore/bdMemory/bdMemory.inl>
#include <bdCore/bdMemory/bdMemory-wii.inl>

#endif // BD_MEMORY_WII_H
