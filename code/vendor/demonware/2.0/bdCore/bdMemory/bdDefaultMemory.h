// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DEFAULT_MEMORY_H
#define BD_DEFAULT_MEMORY_H

#define BD_MALLOC_MEMORY
#include <bdCore/bdMemory/bdMallocMemory.h>
typedef bdMallocMemory bdDefaultMemory;

//#define BD_DEBUG_MEMORY
//#include <bdCore/bdMemory/bdDebugMemory.h>
//typedef bdDebugMemory bdDefaultMemory;

#endif // BD_DEFAULT_MEMORY_H
