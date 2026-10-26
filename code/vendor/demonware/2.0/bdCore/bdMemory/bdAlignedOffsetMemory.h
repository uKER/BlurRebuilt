// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines functions pointers for default memory management operations.

#ifndef BD_ALIGNED_OFFSET_MEMORY_H
#define BD_ALIGNED_OFFSET_MEMORY_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// Allocates an aligned memory block.
/// The alignment is such that the \a offset'th byte is aligned on a \a align
/// byte boundary.
/// \param size[in] The size in bytes of the required memory block.
/// \param align[in] The required memory alignment, must be a power of 2.
/// \param offset[in] The offset into the memory block that should line of a
///					  \a align byte boundary.
/// \return
/// - A pointer to the allocated memory block.
/// - BD_NULL, if the memory could not be allocated.
void* BD_CALL bdAlignedOffsetMalloc(const bdUWord size,
									const bdUWord align,
									const bdUWord offset);

/// Deallocates an aligned offset memory block.
/// The memory block pointed to by \a p must have been allocated using
/// bdAlignedOffsetMalloc.
/// \param p[in] A pointer to the memory block to deallocate.
void BD_CALL bdAlignedOffsetFree(void *p);

/// Resizes an aligned offset memory block.
/// The alignment is such that the \a offset'th byte is aligned on a \a align
/// byte boundary.
/// The memory block pointed to by \a p must have been allocated using
/// bdAlignedOffsetMalloc.
/// \param p[in] A pointer to the memory block to be resized.
/// \param origSize[in] The size of the memory block pointed to by \a p.
/// \param size[in] The size in bytes of the required memory block.
/// \param align[in] The required memory alignment, must be a power of 2.
/// \param offset[in] The offset into the memory block that should line of a
///					  \a align byte boundary.
/// \return
/// - A pointer to the allocated memory block.
/// - BD_NULL, if the memory could not be allocated.
void* BD_CALL bdAlignedOffsetRealloc(void *p,
									 const bdUWord origSize,
									 const bdUWord size,
									 const bdUWord align,
									 const bdUWord offset);

#endif // BD_ALIGNED_OFFSET_MEMORY_H
