#ifndef __MEMORY_TRACKER_FLAGS_H__
#define __MEMORY_TRACKER_FLAGS_H__

// default memory usage: 4 bytes per allocation
#define MEMORY_TRACKER_ENABLE 0

// Remember the callstack for each allocation. Memory usage: approx 30 bytes per allocation. 
#define MEMORY_TRACKER_CALLSTACK 0

// Ability to enumerate all currently allocated memory blocks. Memory usage: 8 bytes.
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 0

// Track the size and location of all allocated memory blocks. Memory usage: 8 bytes.
#define MEMORY_TRACKER_BLOCK_LOCATIONS 0

// Enable tagging of allocations (for tracking memory leaks when going through game loops)
#define MEMORY_TRACKER_TAG_ALLOCATION 1

#endif
