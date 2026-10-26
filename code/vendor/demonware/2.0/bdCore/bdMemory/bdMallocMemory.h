// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A non-pooled memory manager, which records all allocations and
//			deallocations so memory leaks can be detected.

#ifndef BD_MALLOC_MEMORY_H
#define BD_MALLOC_MEMORY_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// The BitDemon memory magic pattern.
/// BD_MEMORY_MAGIC is written into the memory header for each allocation.
/// Upon deallocation a BD_ASSERT is thrown if the pattern has changed,
/// indicating that the memory has been overwritten.
#define BD_MEMORY_MAGIC (0xbdbd)

#if defined BD_PLATFORM_WII
#include <bdCore/bdMemory/bdMallocMemory-wii.h>
#else
#include <bdCore/bdMemory/bdMallocMemory-std.h>
#endif

#endif // BD_MALLOC_MEMORY_H
