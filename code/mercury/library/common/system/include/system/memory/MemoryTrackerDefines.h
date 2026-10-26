#ifndef __SYSTEM_MEMORY_TRACKER_DEFINES_H__
#define __SYSTEM_MEMORY_TRACKER_DEFINES_H__

#ifdef _MSC_VER
#pragma once
#endif

// put a tag at the end of the allocation to identify that tracking is on this block
#ifndef MEMORY_TRACKER_TAG_ALLOCATION
#define MEMORY_TRACKER_TAG_ALLOCATION 1
#endif

// enable storing the call-stack for each allocation (uses 4 bytes per allocation)
#ifndef MEMORY_TRACKER_CALLSTACK
#define MEMORY_TRACKER_CALLSTACK 1
#endif

// enable enumerating individual allocations (uses 8 bytes per allocation)
#ifndef MEMORY_TRACKER_ENUMERATE_ALLOCATIONS
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 1
#endif

// enable tracking allocation block locations and sizes (uses 8 bytes per allocation)
#ifndef MEMORY_TRACKER_BLOCK_LOCATIONS
#define MEMORY_TRACKER_BLOCK_LOCATIONS 1
#endif

// define flags that allow us to check what features are enabled
#if MEMORY_TRACKER_CALLSTACK
#define MEMORY_TRACKER_CALLSTACK_FLAG 1
#else
#define MEMORY_TRACKER_CALLSTACK_FLAG 0
#endif

#if MEMORY_TRACKER_ENUMERATE_ALLOCATIONS
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG 2
#else
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG 0
#endif

#if MEMORY_TRACKER_BLOCK_LOCATIONS
#define MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG 4
#else
#define MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG 0
#endif

// macro which identifies the currently used memory tracker flags
#define MEMORY_TRACKER_FLAGS (MEMORY_TRACKER_CALLSTACK_FLAG + MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG + MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)

#endif
