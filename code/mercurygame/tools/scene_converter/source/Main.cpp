#include "stdafx.h"
#include <Application.h>

#define ENABLE_SMALL_BLOCK_ALLOCATOR
 
//#ifdef CFG_DEBUG
//#define ENABLE_MEMORY_TRACKER
//#endif

#if defined( ENABLE_MEMORY_TRACKER )

#define MEMORY_TRACKER_TAG_ALLOCATION 1
#define MEMORY_TRACKER_CALLSTACK 1
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 1
#define MEMORY_TRACKER_BLOCK_LOCATIONS 1

#endif

#include <System/Memory/AllocatorFunctions.inl>

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	if (!IsDebuggerPresent())
		MessageBox(NULL, "Example A3D Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

#ifdef ENABLE_MEMORY_TRACKER
    Memory::Tracker::Instance().DumpBlocksOnExit(true);
#endif

	C_Application app(
		"Scene Converter",
		"Scene Converter",
		"tools.scene_converter",
		"",
		true);
		
	return app.Run(argc, argv);
}

