// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Used for accessing default memory management operations for a platform.

#ifndef BD_PLATFORM_MEMORY_H
#define BD_PLATFORM_MEMORY_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// Platform specific, unaligned memory allocation.
/// Forwards to the platforms standard unaligned memory allocation
/// function. In most cases this is ::malloc.
/// \param size[in] The size in bytes of the required memory block.
/// \return
/// - The pointer returned by calling the function set using bdMemory::setMallocFunc.
/// - BD_NULL, if no bdMemory::bdMallocFunc has been set.
 inline void* BD_CALL bdMalloc(bdUWord size);

/// Platform specific, unaligned memory dellocation.
/// Forwards to the platforms standard unaligned memory deallocation
/// function. In most cases this is ::free.
/// \param p[in] A pointer to the memory block to be deallocated.
 inline void BD_CALL bdFree(void *p);

/// Platform specific, unaligned memory resizing.
/// Forwards to the platforms standard unaligned memory resizing
/// function. In most cases this is ::realloc.
/// \param p[in] A pointer to the memory block to be resized.
/// \param size[in] The size in bytes of the required memory block.
/// \return
/// - The pointer returned by calling the function set using
/// bdMemory::setReallocFunc.
/// - BD_NULL, if no bdMemory::bdReallocFunc has been set.
 inline void* BD_CALL bdRealloc(void *p,
								bdUWord size);

//
// Memory buffer manipulation routines.
//

/// Finds the first occurrence of \a c in the first \a len bytes of \a s.
/// \param s[in] A pointer to the area of memory to search.
/// \param c[in] The char/int to find.
/// \param len[in] The maximum number of bytes to search.
/// \return If found returns a pointer to the occurrence, else \a BD_NULL.
/// \note This is a non const version of memchr().
inline void* BD_CALL bdMemchr(void *const s,
									const bdInt c,
									const bdUWord len);

/// Copies \a len bytes from \a src to \a dest.
/// \param dest[in] A pointer to the area of memory to write the copied
///  data to.
/// \param src[in] A pointer to the area of memory to copy the data
///  from.
/// \param len[in] The number of bytes to copy.
/// \return A pointer to the \a dest is returned.
/// \note Take care that the \a dest buffer is equal to or larger
///  than the \a src buffer.
/// \remark This function does not handle overlapping regions, if
///  required use memmove().
inline void* BD_CALL bdMemcpy(void *const dest,
									const void *const src,
									const bdUWord len);

/// Compares the two sections of memory by looking at the
/// first \a len bytes  (each interpreted as an unsigned char)
/// \param s1[in] A pointer to the first section of memory.
/// \param s2[in] A pointer to the second section of memory.
/// \param len[in] The number of bytes to compare.
/// \return
///  - an integer less than zero, if \a s1 is lexicographically less
///    than \a s2, when taken to be unsigned characters.
///  - an integer equal to zero, if \a s1 is lexicographically equal
///    to \a s2, when taken to be unsigned characters.
///  - an integer greater than zero, if \a s1 is lexicographically
///    greater than \a s2, when taken to be unsigned characters.
inline bdInt BD_CALL bdMemcmp(const void *const s1,
										const void *const s2,
										const bdUWord len);

/// Copies \a len bytes from \a src to \a dest.
/// \param dest[in] A pointer to the area of memory to write the copied
///  data to.
/// \param src[in] A pointer to the area of memory to copy the data
///  from.
/// \param len[in] The number of bytes to copy.
/// \return A pointer to the \a dest is returned.
/// \note Take care that the \a dest buffer is larger than the \a src buffer.
/// \remark This function handles overlapping regions, if not required
///  use memcpy().
inline void* BD_CALL bdMemmove(void *const dest,
									const void *const src,
									const bdUWord len);

/// Sets the initial \a len bytes of \a s to \a c.
/// \param s[in] A pointer to the section of memory to copy the data
///  from.
/// \param c[in] A pointer to the section of memory to copy the data
///  to.
/// \param len[in] The number of bytes to set.
/// \return A pointer to \a s, the area of memory that has been set.
inline void* BD_CALL bdMemset(void *const s,
									const bdInt c,
									const bdUWord len);


#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.inl>
/*
// This is messy but it's the only way to reap the rewards of inlining.
#if defined (BD_PLATFORM_WIN32)
#	include <bdPlatform/bdPlatformMemory/bdPlatformMemory-win32.inl>
#elif defined (BD_PLATFORM_PS2) || defined (BD_PLATFORM_PSP) || defined (BD_PLATFORM_PS3)
#	include <bdPlatform/bdPlatformMemory/bdPlatformMemory-ps2.inl>
#elif defined (BD_PLATFORM_UNIX)
#	include <bdPlatform/bdPlatformMemory/bdPlatformMemory-unix.inl>
#elif defined (BD_PLATFORM_XBOX)
#	include <bdPlatform/bdPlatformMemory/bdPlatformMemory-xbox.inl>
#else
#	error Unknown platform, not supported.
#endif
*/
#endif // BD_PLATFORM_MEMORY_H
