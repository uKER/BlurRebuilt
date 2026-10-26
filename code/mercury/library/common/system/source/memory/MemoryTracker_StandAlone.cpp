
/*
	This file is designed to be easily included in projects not using the Shared-Tech and to enable memory tracking.

	Will require the code to initialise the network stack and to call an update function, both of which have stubs here

	This code will probably not compile currently.
*/

#include <xtl.h>

#define ENABLE_MEMORY_TRACKING

#include <core\core.h>
#include "Core\Containers\String.h"
#include "..\..\include\system\memory.h"
#include "..\..\include\system\debug\debug.h"
#include "..\..\include\system\debuglog.h"
#include "..\..\include\system\artery\artery.h"

#define MEMORY_TRACKER_TAG_ALLOCATION			0
#define MEMORY_TRACKER_CALLSTACK				1    // Remember the callstack for each allocation. Memory usage: approx 30 bytes per allocation. 
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS	1    // Ability to enumerate all currently allocated memory blocks. Memory usage: 8 bytes.
#define MEMORY_TRACKER_BLOCK_LOCATIONS			1    // Track the size and location of all allocated memory blocks. Memory usage: 8 bytes.

#include "..\..\include\system\memory\MemoryTrackerDefines.h"

size_t MemoryTrackerStructSize;
size_t AlignedMemoryTrackerStructSize;

void InitMemoryNetwork();
void IncrementTag();
void UpdateMemTrack();

Debug::Bool gTest("Debug.Test", false);

void UpdateMemTrack()
{
	Debug::NetworkUpdate();
}

void GameLoadDebugVariables(void)
{
}

void GameSaveDebugVariables(void)
{
}

void InitMemoryNetwork()
{

	bool bypassSecurity = true;

		// startup Xnet first
		XNetStartupParams xnsp;
		ZeroMemory(&xnsp, sizeof(xnsp));
		xnsp.cfgSizeOfStruct = sizeof(xnsp);
#ifndef CFG_GOLD
		// flags are ignored when linking with secure Xenon libraries..
		xnsp.cfgFlags = bypassSecurity ? XNET_STARTUP_BYPASS_SECURITY : 0; 
#endif
		HRESULT hr = XNetStartup(&xnsp);

		assert(hr == NO_ERROR);

	if (ArteryStart(true, "BoomBoom"))
	{
		Debug::NetworkStart();
		//Debug::GetRootLogger().AddSubscriber( new Debug::DebugViewSubscriber() );
	}
	else
	{
		assert(false);
		/*LOGERROR(g_Logger, "ArteryStart returned FALSE");*/
	}

	Memory::InitialiseNetworkConnection();
}

void IncrementTag()
{
	Memory::TrackNodes::Instance().IncrementTag();
	Memory::Tracker::Instance().DumpNodeStats();
}

void TrackAllocation(void *ptr, DWORD origSize, DWORD allocSize, DWORD waste, Memory::E_MEMORY_TYPES memoryType)
{
	if (!ptr)
		return;

	if (memoryType == Memory::kMemType_System)
	{
		const bool needToKeepMemoryAligned = ((origSize & 15) == 0);

		const size_t iMemTrackSize = (needToKeepMemoryAligned) ? AlignedMemoryTrackerStructSize : MemoryTrackerStructSize;

		Memory::AllocationNode* pNode = (Memory::AllocationNode*)(((U8*)ptr) + allocSize - MemoryTrackerStructSize);

		// Store whether we needed to keep the allocation aligned to 16 bytes
		pNode->m_iAligned = needToKeepMemoryAligned;

		Memory::Tracker::Instance().TrackAllocation(ptr,											// allocation address
			origSize,																				// original size
			allocSize - iMemTrackSize + waste,											// actual size (the difference is used to calculate memory wastage)
			memoryType,																				// physical memory flag
			(Memory::AllocationNode*)(((U8*)ptr) + allocSize - MemoryTrackerStructSize),			// address of tracking struct
			MEMORY_TRACKER_FLAGS);
	}
	else
	{
		assert(false);
		//Ut::OpenHashMap<void*, Memory::AllocationNode*, Ut::HashKeyOpPtr >::InsertionReturnType result;

		//MEMORY_TRACKER_PUSH("debug.memtrack");

		//result = g_HashPhysical.Insert(ptr);

		//MEMORY_TRACKER_POP();

		//assert(result.second == true);

		//Memory::AllocationNode* allocNode = (Memory::AllocationNode*) DmAllocatePool(MemoryTrackerStructSize);

		//// TODO: Need to track the memory used by these node's

		//Memory::Tracker::Instance().TrackAllocation(ptr,			// allocation address
		//	origSize,												// original size
		//	allocSize + waste,										// actual size (the difference is used to calculate memory wastage)
		//	memoryType,												// physical memory flag
		//	allocNode,												// address of tracking struct
		//	MEMORY_TRACKER_FLAGS);

		//result.first->GetValue() = allocNode;
	}
}

void TrackDeallocation(void *ptr, long iActualSize, long waste, Memory::E_MEMORY_TYPES memoryType)
{
	if (memoryType == Memory::kMemType_System)
	{
		Memory::AllocationNode* pNode = (Memory::AllocationNode*)(((U8*)ptr) + iActualSize - MemoryTrackerStructSize);

		const size_t iMemTrackSize = (pNode->m_iAligned) ? AlignedMemoryTrackerStructSize : MemoryTrackerStructSize;

		Memory::Tracker::Instance().TrackDeallocation(
			iActualSize - iMemTrackSize + waste,
			memoryType,
			pNode);
	}
	else	// isPhysicalMemory
	{
		assert(false);
		//Ut::OpenHashMap<void*, Memory::AllocationNode*, Ut::HashKeyOpPtr >::Iterator result = g_HashPhysical.Find(ptr);

		//if (result != g_HashPhysical.End())
		//{
		//	Memory::AllocationNode* allocNode = result->GetValue();

		//	Memory::Tracker::Instance().TrackDeallocation(iActualSize, memoryType, allocNode);

		//	DmFreePool(allocNode);

		//	g_HashPhysical.Erase(result);
		//}
		//else
		//{
		//	assert(false);
		//}
	}
}

#ifdef ENABLE_MEMORY_TRACKING

void * __cdecl operator new(size_t iOriginalSize)
{
	size_t iAllocSize = iOriginalSize + ((iOriginalSize & 15) ? MemoryTrackerStructSize : AlignedMemoryTrackerStructSize);
	
	void *pMem = malloc(iAllocSize);
    
	if (pMem)
	{
		const size_t iActualSize = _msize(pMem);

		TrackAllocation(pMem, iOriginalSize, iActualSize, 16, Memory::kMemType_System);
	}
	else
	{
		int mrf = 10;
		//LOGDEBUG(g_DebugLoggerMemory, "new: Failed To Allocate (%d - %.1fk)\n", iAllocSize, float(iAllocSize)/1024.f);
		//AssertOutOfMemory();
	}

	return pMem;
}

void __cdecl operator delete(void *mem)
{
	if (mem)
	{
		size_t iActualSize = _msize(mem);

		TrackDeallocation(mem, iActualSize, 16, Memory::kMemType_System);
	}

	free(mem);
}

int PreStaticInit (void);

typedef int (__cdecl *PIFV)(void);

#pragma data_seg(".CRT$XIB")
PIFV g_initFn = PreStaticInit;
#pragma data_seg()

int PreStaticInit (void)
{
	if ( g_initFn == NULL )
		return 0;

	MemoryTrackerStructSize			= Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);
	AlignedMemoryTrackerStructSize	= (MemoryTrackerStructSize + 15) & ~15;

	Memory::Tracker::Instance().Initialize(MemoryTrackerStructSize, 3, true);

	g_initFn = NULL;

	return 0;
}

#endif