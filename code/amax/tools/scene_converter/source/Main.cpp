#include "stdafx.h"
#include <Application.h>

//#ifdef CFG_DEBUG
//#define ENABLE_MEMORY_TRACKING
//#endif

#if defined( ENABLE_MEMORY_TRACKING )

#define MEMORY_TRACKER_TAG_ALLOCATION 1
#define MEMORY_TRACKER_CALLSTACK 1
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 1
#define MEMORY_TRACKER_BLOCK_LOCATIONS 1

#else

#define MEMORY_TRACKER_TAG_ALLOCATION 0
#define MEMORY_TRACKER_CALLSTACK 0
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 0
#define MEMORY_TRACKER_BLOCK_LOCATIONS 0

#endif

#include <System/Memory/MemoryTracker.h>

#include <System/Memory/MemoryTrackerDefines.h>

#include <System/Memory/AllocatorFunctions.h>

 

//void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }

//void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }

//void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }

//void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }

void* BizAlignedMalloc(size_t size, size_t alignment) { return Memory::TrackedAlignedAlloc(size,alignment); }

void BizAlignedFree(void *p) { Memory::TrackedAlignedFree(p); }

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	if (!IsDebuggerPresent())
		MessageBox(NULL, "Example A3D Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

    Memory::Tracker::Instance().DumpBlocksOnExit(true);

	C_Application app(
		"Scene Converter",
		"Scene Converter",
		"tools.scene_converter",
		"",
		true);
		
	return app.Run(argc, argv);
}

