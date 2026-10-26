#ifndef __SYSTEM_ALLOCATORFUNCTIONS_H__
#define __SYSTEM_ALLOCATORFUNCTIONS_H__

// TODO: Add #define to allow for setting new allocations to 0xcd
// TODO: Check for buffer under/over runs
// TODO: Add code to dump memory information when we run out of memory

// ENABLE_SMALL_BLOCK_ALLOCATOR
// ENABLE_MEMORY_TRACKER

// ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE

// ENABLE_GUARD_BLOCKS

// example usage:
//
//#include "system\memory\AllocatorFunctions.h"
//

/*
	_CrtSetBreakAlloc( allocIndex)

	or

	(int*){,,msvcr80d.dll}__crtBreakAlloc	(VS2005)
*/

#ifdef ENABLE_GUARD_BLOCKS
#include "GuardFunctions.inl"
#endif

#ifdef ENABLE_MEMORY_TRACKER
#pragma message("WARNING: Memory Tracking - ENABLED!!!")
#if MEMORY_TRACKER_CALLSTACK
#pragma message("WARNING: Memory Tracking + CALLSTACKS - ENABLED!!!")
#endif
#include <system\memory\MemoryTrackerDefines.h>
#include <system/memory/MemoryTracker.h>
#endif

/*
	Support for the small block allocator
*/
#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR

#include <system/memory/BlockAllocator.h>

namespace Memory
{
	extern BlockAllocator* g_smallBlockAllocatorPtr;
}

#ifndef OVERRIDE_SMALL_BLOCK_ALLOCATOR_SIZES

static int g_SmallBlockAllocatorSizes[] = 
{
	16,32,48,64,80,96,112,128,144,160,176,192,208,224,240,256,
	0
};

#endif

const int g_SmallBlockNumberOfSizes = (sizeof(g_SmallBlockAllocatorSizes) / sizeof(g_SmallBlockAllocatorSizes[0]));

// Global instance of the small block allocator
Memory::BlockAllocator g_SmallBlockAllocator;

#endif

#ifdef ENABLE_MEMORY_TRACKER

size_t MemoryTrackerStructSize;
size_t AlignedMemoryTrackerStructSize;
U32 preStaticInitMemoryUsage;

#endif

namespace Memory
{

typedef void (*BizOutOfMemoryCallbackHandler)(void);

bool g_OutOfMemoryHasOccured(false);
static Debug::Logger &g_DebugLoggerMemory = Debug::GetLogger("mercury.memory");

static BizOutOfMemoryCallbackHandler g_BizOutOfMemoryCallbackHandler = NULL;

void SetOutOfMemoryCallback(BizOutOfMemoryCallbackHandler handler)
{
	g_BizOutOfMemoryCallbackHandler = handler;
}

NOINLINE void AssertOutOfMemory(DWORD size)
{
	g_OutOfMemoryHasOccured = true;

#ifdef ENABLE_MEMORY_TRACKER
	Memory::Tracker::Instance().GetCriticalSection().Enter();

#ifdef PLATFORM_PC
	Memory::Tracker::Instance().DumpMemoryRaw("C:\\OutOfMemory");
#endif

	Memory::Tracker::Instance().DumpNodeStats(false, false);
	Memory::Tracker::Instance().DumpTagStats(0, NULL, true);
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	g_SmallBlockAllocator.DumpStats();
#endif

	if (g_BizOutOfMemoryCallbackHandler)
	{
		g_BizOutOfMemoryCallbackHandler();
	}

#ifdef ENABLE_MEMORY_TRACKER
	Memory::Tracker::Instance().GetCriticalSection().Exit();
#endif

	LOGFATAL(g_DebugLoggerMemory, "AssertOutOfMemory: Game could now potentially crash (%d bytes)\n", size);

	#if !CFG_GOLD && CFG_SPEED && ENABLE_OUT_OF_MEMORY_DEBUG_BREAK_IN_SPEED
	DebugBreak(); // So out of memory warnings aren't ignored in speed
	#endif
}

NOINLINE void AssertOutOfMemory(void)
{
	AssertOutOfMemory(0);
}

/*
	Work out the size of an aligned allocation
*/
#define PTR_SZ sizeof(void *)
size_t  _aligned_msize(void *memblock)
{
#if !defined( COMPILER_VC )

	return malloc_usable_size(memblock);

#else

    uintptr_t ptr;

    if (memblock == NULL)
        return 0;

    ptr = (uintptr_t)memblock;

    /* ptr points to the pointer to starting of the memory block */
    ptr = (ptr & ~(PTR_SZ -1)) - PTR_SZ;

#ifdef CFG_DEBUG
	ptr -= sizeof(void*);	// Take into account the 4-byte under-run buffer-check stuff [Blur2PC][x64] the CRT header pads it to pointer size
#endif

    /* ptr is the pointer to the start of memory block*/
    ptr = *((uintptr_t *)ptr);

	return _msize((void *)ptr) - ((uintptr_t)memblock - ptr);	// [Blur2PC][x64] was long(memblock) - long(ptr)
#endif
}

#ifdef ENABLE_MEMORY_TRACKER

void TrackMyAllocation(void *ptr, DWORD originalSize, DWORD allocSize, DWORD allocatorOverheadSize, Memory::E_MEMORY_TYPES memoryType, bool hadToKeepAlignment, int callstackOverhead, void** manualAllocNode=NULL, bool heapAllocation=false)
{
	if (!ptr)
		return;

	if (memoryType == Memory::kMemType_System)
	{
		const size_t iMemTrackSize = hadToKeepAlignment ? AlignedMemoryTrackerStructSize : MemoryTrackerStructSize;

		Memory::AllocationNode* allocNode = (Memory::AllocationNode*)(((U8*)ptr) + allocSize - MemoryTrackerStructSize);

		// Store whether we needed to keep the allocation aligned to 16 bytes
		allocNode->m_iAligned = hadToKeepAlignment;

		Memory::Tracker::Instance().TrackAllocation(ptr,		// allocation address
			originalSize,										// original size
			allocSize - iMemTrackSize + allocatorOverheadSize,	// actual size (the difference is used to calculate memory wastage)
			memoryType,											// physical memory flag
			allocNode,											// address of tracking struct
#ifdef CFG_DEBUG
			callstackOverhead
#else
			0
#endif
			);
	}
	else
	{
		if (!heapAllocation)
		{
			Memory::AllocationNode* allocNode = (Memory::AllocationNode*) malloc(MemoryTrackerStructSize + 4);

			if (!manualAllocNode)
				Memory::Tracker::Instance().RememberAllocNodeForVirPhys(allocNode, ptr);
			else
				*manualAllocNode = allocNode;

			Memory::Tracker::Instance().TrackAllocation(ptr,	// allocation address
				originalSize,									// original size
				allocSize + allocatorOverheadSize,				// actual size (the difference is used to calculate memory wastage)
				memoryType,										// physical memory flag
				allocNode,										// address of tracking struct
#ifdef CFG_DEBUG
				callstackOverhead
#else
				0
#endif
				);
		}
		else
		{
			Memory::Tracker::Instance().TrackHeapAllocation(ptr, originalSize, memoryType);
		}
	}
}

void TrackMyDeallocation(void *ptr, long iActualSize, long waste, Memory::E_MEMORY_TYPES memoryType, void** manualAllocNode=NULL, bool heapAllocation=false)
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
		if (!heapAllocation)
		{
			Memory::AllocationNode* allocNode;
			
			if (!manualAllocNode)
				allocNode = Memory::Tracker::Instance().ForgetAllocNodeForVirPhys(ptr);
			else
				allocNode = (Memory::AllocationNode*)*manualAllocNode;

			if (allocNode)
			{
				Memory::Tracker::Instance().TrackDeallocation(iActualSize, memoryType, allocNode);

				free(allocNode);
			}
		}
		else
		{
			Memory::Tracker::Instance().TrackHeapDeallocation(ptr, iActualSize, memoryType);
		}
	}
}

#endif

__forceinline size_t BizInternalSize(void *lpAddress)
{
#ifdef ENABLE_GUARD_BLOCKS
	lpAddress = (void*) (((size_t)lpAddress) - 16);
	return *((DWORD*)lpAddress);
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	if (g_SmallBlockAllocator.MemoryOwnedByAllocator(lpAddress))
	{
		return g_SmallBlockAllocator.AlocationSize(lpAddress);
	}
#endif

#if !defined( COMPILER_VC )
	return malloc_usable_size(lpAddress);
#else
	return _msize(lpAddress);
#endif
}

__forceinline size_t BizInternalSize(void *lpAddress, size_t& allocatorOverhead)
{
#ifdef ENABLE_GUARD_BLOCKS
	allocatorOverhead = 0;
	lpAddress = (void*) (((size_t)lpAddress) - 16);
	return *((DWORD*)lpAddress);
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	if (g_SmallBlockAllocator.MemoryOwnedByAllocator(lpAddress))
	{
		allocatorOverhead = 0;
		return g_SmallBlockAllocator.AlocationSize(lpAddress);
	}
#endif

	allocatorOverhead = 16;
#if !defined( COMPILER_VC )
	return malloc_usable_size(lpAddress);
#else
	return _msize(lpAddress);
#endif
}

// JASOND: SSE requires 16 byte aligned allocations ?

__forceinline void* BizInternalMalloc(size_t allocSize, size_t& allocatorOverhead)
{
	void *pAllocatedMemory;

#ifdef ENABLE_GUARD_BLOCKS
	size_t preGuardSize = allocSize;
	allocSize += 16 + GUARD_BLOCK_SIZE;
#endif

#if defined(ENABLE_SMALL_BLOCK_ALLOCATOR) && !defined(ENABLE_GUARD_BLOCKS)
	if (g_SmallBlockAllocator.Initialized() && ((pAllocatedMemory = g_SmallBlockAllocator.Allocate((U32)allocSize)) != 0))
	{
		allocatorOverhead = 0;
	}
	else
#endif
	{
		// Default allocation if Small Block Allocator fails or isn't used
		// "malloc" has a minimum overhead of 16 bytes internally to track allocation (more in debug)
		pAllocatedMemory = malloc(allocSize);
		allocatorOverhead = 16;
	}

#ifndef CFG_GOLD
	if (pAllocatedMemory == NULL)
	{
		LOGDEBUG(g_DebugLoggerMemory, "BizInternalMalloc: Failed To Allocate (%d - %.1fk)\n", allocSize, float(allocSize)/1024.f);
		AssertOutOfMemory(allocSize);
	}
#ifdef ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE
	else
	{
		memset(pAllocatedMemory, 0x9e, allocSize);
	}
#endif
#endif

#ifdef ENABLE_GUARD_BLOCKS
	pAllocatedMemory = (void*) (((size_t)pAllocatedMemory) + 16);

	AddMemoryGuards(pAllocatedMemory, 16, preGuardSize);
#endif

	return pAllocatedMemory;
}

__forceinline void BizInternalFree(void* lpAddress)
{
	if (lpAddress)
	{
		size_t memSize = BizInternalSize(lpAddress);

#ifdef ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE
		memset(lpAddress, 0x9f, memSize);
#endif

#ifdef ENABLE_GUARD_BLOCKS
		size_t guardAlignment = RemoveMemoryGuards(lpAddress);

		lpAddress = (void*) (((size_t)lpAddress) - guardAlignment);
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
		if (g_SmallBlockAllocator.MemoryOwnedByAllocator(lpAddress))
		{ 
			g_SmallBlockAllocator.Free(lpAddress); 	
		}
		else
#endif
		{
			free(lpAddress);
		}
	}
}

__forceinline size_t BizInternalAlignedSize(void *lpAddress)
{
#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	if (g_SmallBlockAllocator.MemoryOwnedByAllocator(lpAddress))
	{
		return g_SmallBlockAllocator.AlocationSize(lpAddress);
	}
#endif

	return _aligned_msize(lpAddress);
}

__forceinline size_t BizInternalAlignedSize(void *lpAddress, size_t& allocatorOverhead)
{
#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	if (g_SmallBlockAllocator.MemoryOwnedByAllocator(lpAddress))
	{
		allocatorOverhead = 0;
		return g_SmallBlockAllocator.AlocationSize(lpAddress);
	}
#endif

	allocatorOverhead = 16;
	return _aligned_msize(lpAddress);
}

__forceinline void* BizInternalAlignedMalloc(size_t allocSize, size_t alignment, size_t& allocatorOverhead)
{
	void *pAllocatedMemory;

#ifdef ENABLE_GUARD_BLOCKS
	size_t amountToAlign = alignment > 16 ? alignment : 16;
	size_t preGuardSize = allocSize;
	allocSize += amountToAlign + GUARD_BLOCK_SIZE;
#endif

#if defined(ENABLE_SMALL_BLOCK_ALLOCATOR) && !defined(ENABLE_GUARD_BLOCKS)
	if ((alignment == 16) && g_SmallBlockAllocator.Initialized() && ((pAllocatedMemory = g_SmallBlockAllocator.Allocate((U32)allocSize)) != 0))
	{
		allocatorOverhead = 0;
	}
	else
#endif
	{
		// Default allocation if Small Block Allocator fails or isn't used
		// "malloc" has a minimum overhead of 16 bytes internally to track allocation (more in debug)
#if !defined( COMPILER_VC )
		pAllocatedMemory = memalign(alignment, allocSize);
#else
		pAllocatedMemory = _aligned_malloc(allocSize, alignment);
#endif
		allocatorOverhead = 16;
	}

#ifndef CFG_GOLD
	if (pAllocatedMemory == NULL)
	{
		LOGDEBUG(g_DebugLoggerMemory, "BizInternalAlignedMalloc: Failed To Allocate (%d - %.1fk)\n", allocSize, float(allocSize)/1024.f);
		AssertOutOfMemory(allocSize);
	}
#ifdef ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE
	else
	{
		memset(pAllocatedMemory, 0x9e, allocSize);
	}
#endif
#endif

#ifdef ENABLE_GUARD_BLOCKS
	pAllocatedMemory = (void*) (((size_t)pAllocatedMemory) + amountToAlign);

	AddMemoryGuards(pAllocatedMemory, amountToAlign, preGuardSize);
#endif

	return pAllocatedMemory;
}

__forceinline void BizInternalAlignedFree(void* lpAddress)
{
	if (lpAddress)
	{
#ifdef ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE
		memset(lpAddress, 0x9f, BizInternalAlignedSize(lpAddress));
#endif

#ifdef ENABLE_GUARD_BLOCKS
		size_t guardAlignment = RemoveMemoryGuards(lpAddress);
		
		lpAddress = (void*) (((size_t)lpAddress) - guardAlignment);
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
		if (g_SmallBlockAllocator.MemoryOwnedByAllocator(lpAddress))
		{ 
			g_SmallBlockAllocator.Free(lpAddress); 	
		}
		else
#endif
		{
#if !defined( COMPILER_VC )
			free(lpAddress);
#else
			_aligned_free(lpAddress);
#endif
		}
	}
}

__forceinline void* BizNonAlignedMalloc(size_t originalSize)
{
	// If we're tracking memory usage we need to tack our tracking information onto the end of the allocation
	// We also try and preserve alignment on allocations that are multiples of 16 bytes (for stuff like VMX vectors)
#ifdef ENABLE_MEMORY_TRACKER
	bool hadToKeepAlignment = ((originalSize & 15) == 0);

	size_t allocSize = originalSize + ((!hadToKeepAlignment) ? MemoryTrackerStructSize : AlignedMemoryTrackerStructSize);
#else
	size_t allocSize = originalSize;
#endif

	size_t allocatorOverheadSize;
	void *pAllocatedMemory = BizInternalMalloc(allocSize, allocatorOverheadSize);

#ifdef ENABLE_MEMORY_TRACKER
	if (pAllocatedMemory != NULL)
	{
		// Calculate how much memory was actually allocated for us (will include any rounding up that might occur in the allocation)
		size_t actualSize = BizInternalSize(pAllocatedMemory);

		TrackMyAllocation(pAllocatedMemory, originalSize, actualSize, allocatorOverheadSize, kMemType_System, hadToKeepAlignment, 8);
	}
#endif

	// TrackAlloc - originalSize, allocatedSize, allocOverhead, Type, Tracking Struct

	return pAllocatedMemory;
}

__forceinline void BizNonAlignedFree(void* ptr)
{
#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	const bool isSmallBlockAllocator = g_SmallBlockAllocator.MemoryOwnedByAllocator(ptr);
#endif

#ifdef ENABLE_MEMORY_TRACKER
	if (ptr != NULL)
	{
		size_t allocatorOverheadSize;
		size_t actualSize = BizInternalSize(ptr, allocatorOverheadSize);
		if (actualSize < MemoryTrackerStructSize)
			return;
		
		Memory::AllocationNode* pNode = (Memory::AllocationNode*)(((U8*)ptr) + actualSize - MemoryTrackerStructSize);

		const size_t memTrackOverhead = (pNode->m_iAligned) ? AlignedMemoryTrackerStructSize : MemoryTrackerStructSize;

		Memory::Tracker::Instance().TrackDeallocation(
			actualSize - memTrackOverhead + allocatorOverheadSize,
			Memory::kMemType_System,
			pNode);
	}
#endif

	BizInternalFree(ptr);
}


}

#ifndef DONT_OVERRIDE_NEW_AND_DELETE

void * operator new(size_t size)
{
#ifndef PLATFORM_PC
	return Memory::BizNonAlignedMalloc(size);
#else
	return BizAlignedMalloc(size, 16);
#endif
}

void* operator new[](size_t size)
{
#ifndef PLATFORM_PC
	return Memory::BizNonAlignedMalloc(size);
#else
	return BizAlignedMalloc(size, 16);
#endif
}

#ifdef PLATFORM_PS3

void * operator new(size_t size, std::size_t alignment)
{
	return BizAlignedMalloc(size, alignment);
}

void* operator new[]( size_t size, std::size_t alignment )
{
	return BizAlignedMalloc(size, alignment);
}

#endif

void operator delete( void *ptr )
{
#ifndef PLATFORM_PC
	Memory::BizNonAlignedFree(ptr);
#else
	BizAlignedFree(ptr);
#endif
}

void operator delete[](void* ptr)
{
#ifndef PLATFORM_PC
	Memory::BizNonAlignedFree(ptr);
#else
	BizAlignedFree(ptr);
#endif
}

#endif

void* BizAlignedMalloc(size_t originalSize, size_t alignment)
{
	// If we're tracking memory usage we need to tack our tracking information onto the end of the allocation
#ifdef ENABLE_MEMORY_TRACKER
	size_t allocSize = originalSize + MemoryTrackerStructSize;
#else
	size_t allocSize = originalSize;
#endif

	size_t allocatorOverheadSize;
	void *pAllocatedMemory = Memory::BizInternalAlignedMalloc(allocSize, alignment, allocatorOverheadSize);

#ifdef ENABLE_MEMORY_TRACKER
	if (pAllocatedMemory != NULL)
	{
		// Calculate how much memory was actually allocated for us (will include any rounding up that might occur in the allocation)
		size_t actualSize = Memory::BizInternalAlignedSize(pAllocatedMemory);

		Memory::TrackMyAllocation(pAllocatedMemory, originalSize, actualSize, allocatorOverheadSize, Memory::kMemType_System, false, 8);
	}
#endif

	return pAllocatedMemory;
}

void BizAlignedFree(void *ptr)
{
#ifdef ENABLE_MEMORY_TRACKER
	if (ptr != NULL)
	{
		size_t allocatorOverheadSize;
		size_t actualSize = Memory::BizInternalAlignedSize(ptr, allocatorOverheadSize);

		if (actualSize >= MemoryTrackerStructSize)
		{
			Memory::AllocationNode* pNode = (Memory::AllocationNode*)(((U8*)ptr) + actualSize - MemoryTrackerStructSize);

			const size_t memTrackOverhead = (pNode->m_iAligned) ? AlignedMemoryTrackerStructSize : MemoryTrackerStructSize;

			Memory::Tracker::Instance().TrackDeallocation(
				actualSize - memTrackOverhead + allocatorOverheadSize,
				Memory::kMemType_System,
				pNode);
		}
	}
#endif

	Memory::BizInternalAlignedFree(ptr);
}

void* BizPhysicalAlloc(size_t dwSize, U32 ulPhysicalAddress, U32 ulAlignment, U32 flProtect)
{
	return NULL;
}

void BizPhysicalFree(void* lpAddress)
{
}

/*
	Heap Functions
*/
void* BizHeapAlignedMalloc(size_t originalSize, size_t alignment)
{
	size_t allocatorOverheadSize;
	void *pAllocatedMemory = Memory::BizInternalAlignedMalloc(originalSize, alignment, allocatorOverheadSize);

#ifdef ENABLE_MEMORY_TRACKER
	if (pAllocatedMemory != NULL)
	{
		// Calculate how much memory was actually allocated for us (will include any rounding up that might occur in the allocation)
		size_t actualSize = Memory::BizInternalAlignedSize(pAllocatedMemory);

		Memory::Tracker::Instance().TrackHeapAllocation(pAllocatedMemory, actualSize, Memory::kMemType_System);
	}
#endif

	return pAllocatedMemory;
}

void BizHeapAlignedFree(void *ptr)
{
#ifdef ENABLE_MEMORY_TRACKER
	if (ptr != NULL)
	{
		size_t actualSize = Memory::BizInternalAlignedSize(ptr);

		Memory::Tracker::Instance().TrackHeapDeallocation(ptr, actualSize, Memory::kMemType_System);
	}
#endif

	Memory::BizInternalAlignedFree(ptr);
}

void* BizHeapPhysicalAlloc(size_t dwSize, U32 ulPhysicalAddress, U32 ulAlignment, U32 flProtect)
{
	return NULL;
}

void BizHeapPhysicalFree(void* lpAddress)
{
}


/*
	This code runs before any global initialisation occurs, which allows us to set up some of the memory tracking and
	small block allocator systems first
*/
#if defined(PLATFORM_PC) && !defined(WIN64)


int PreStaticInit (void);

typedef int (__cdecl *PIFV)(void);

#pragma data_seg(".CRT$XIB")
PIFV g_initFn = PreStaticInit;
#pragma data_seg()

int PreStaticInit(void)
{
	if (g_initFn == NULL)
		return 0;

	#if OUTPUT_EXE_SIZE
	{
	}
	#endif

#ifdef ENABLE_MEMORY_TRACKER
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);

	// What's used up minus the HUD stuff and any other stuff we already allocated and tracked
	preStaticInitMemoryUsage = (stat.dwTotalPhys - stat.dwAvailPhys) - (32 * 1024 * 1024);

	//char buffer[256];
	//sprintf(buffer, "Pre-Static memory usage by EXE: %d (%.1fk)\n", preStaticInitMemoryUsage, float(preStaticInitMemoryUsage)/1024.f);
	//OutputDebugString(buffer);

	MemoryTrackerStructSize			= Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);
	AlignedMemoryTrackerStructSize	= (MemoryTrackerStructSize + 15) & ~15;

	Memory::Tracker::Instance().Initialize("Main", 2, MEMORY_TRACKER_FLAGS, true);
#endif

#ifdef ENABLE_GUARD_BLOCKS
	// Force the Guard system to initialise before we have multiple threads running amok
	Memory::GuardTrackHeader::Lock().Enter();
	Memory::GuardTrackHeader::Lock().Exit();
	Memory::GuardTrackHeader::initialised = true;
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	g_SmallBlockAllocator.PreStatic_Initialise(g_SmallBlockAllocatorSizes);

	Memory::g_smallBlockAllocatorPtr = &g_SmallBlockAllocator;
#endif

#if !defined(PLATFORM_PC)
	g_initFn = NULL;
#endif

	return 0;
}

#elif defined(PLATFORM_PC) && defined(WIN64)

class CPreStaticInit
{
public:
	CPreStaticInit()
	{
	#if OUTPUT_EXE_SIZE
	{
	}
	#endif

#ifdef ENABLE_MEMORY_TRACKER
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);

	// What's used up minus the HUD stuff and any other stuff we already allocated and tracked
	preStaticInitMemoryUsage = (stat.dwTotalPhys - stat.dwAvailPhys) - (32 * 1024 * 1024);

	//char buffer[256];
	//sprintf(buffer, "Pre-Static memory usage by EXE: %d (%.1fk)\n", preStaticInitMemoryUsage, float(preStaticInitMemoryUsage)/1024.f);
	//OutputDebugString(buffer);

	MemoryTrackerStructSize			= Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);
	AlignedMemoryTrackerStructSize	= (MemoryTrackerStructSize + 15) & ~15;

	Memory::Tracker::Instance().Initialize("Main", 2, MEMORY_TRACKER_FLAGS, true);
#endif

#ifdef ENABLE_GUARD_BLOCKS
	// Force the Guard system to initialise before we have multiple threads running amok
	Memory::GuardTrackHeader::Lock().Enter();
	Memory::GuardTrackHeader::Lock().Exit();
	Memory::GuardTrackHeader::initialised = true;
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
	g_SmallBlockAllocator.PreStatic_Initialise(g_SmallBlockAllocatorSizes);

	Memory::g_smallBlockAllocatorPtr = &g_SmallBlockAllocator;
#endif

	}
};

#pragma init_seg(compiler)

CPreStaticInit g_preStaticInit;

#else

class CPreStaticInit
{
public:
	CPreStaticInit()
	{
#ifdef ENABLE_MEMORY_TRACKER
		sys_memory_info_t memInfo;
		sys_memory_get_user_memory_size(&memInfo);

		std::malloc_managed_size mms;
		malloc_stats(&mms);

		// This is memory used by on the exe and static allocations - before global allocations
		preStaticInitMemoryUsage = memInfo.total_user_memory - memInfo.available_user_memory;

		//char buffer[256];
		//sprintf(buffer, "Pre-Static memory usage by EXE: %d (%.1fk)\n", preStaticInitMemoryUsage, float(preStaticInitMemoryUsage)/1024.f);
		//OutputDebugString(buffer);

		MemoryTrackerStructSize			= Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);
		AlignedMemoryTrackerStructSize	= (MemoryTrackerStructSize + 15) & ~15;

		Memory::Tracker::Instance().Initialize("Main", 2, MEMORY_TRACKER_FLAGS, true);
#endif

#ifdef ENABLE_GUARD_BLOCKS
	// Force the Guard system to initialise before we have multiple threads running amok
	Memory::GuardTrackHeader::Lock().Enter();
	Memory::GuardTrackHeader::Lock().Exit();
	Memory::GuardTrackHeader::initialised = true;
#endif

#ifdef ENABLE_SMALL_BLOCK_ALLOCATOR
		g_SmallBlockAllocator.PreStatic_Initialise(g_SmallBlockAllocatorSizes);

		Memory::g_smallBlockAllocatorPtr = &g_SmallBlockAllocator;
#endif
	}
};

// This will force this class to be initialised before all others on the PS3
CPreStaticInit g_preStaticInit __attribute__ ((init_priority (101)));

#endif

#endif
