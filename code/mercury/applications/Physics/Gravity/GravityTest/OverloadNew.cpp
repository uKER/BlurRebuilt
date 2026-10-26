#include "stdafx.h"
#include <Common/Collections.h>
#include <core/Maths.h>
#include <Common/System/Memman.h>
#include <Common/Memory/Memory.h>
#include <Common/Memory/Memory.inl>
#include <Common/Util/PerfStats.h>
#include <Common/Util/PerfStats.inl>

#include <System\Memory.h>

#include <common/memory/memheap.h>
#include <common/memory/memheap.inl>

#include "OverloadNew.h"

//#include "MemoryManager.h"
#undef USE_NEW_MEMORY_TRACKER

#define USE_SMALL_BLOCK_ALLOCATOR

int gSmallAllocCount = 0;
int gOtherAllocCount = 0;

////////////////////////////////////////////////
// Final calls to malloc, free and _msize
////////////////////////////////////////////////

// These are called when the allocation is too big for the small block allocator

#ifdef USE_BACKEND_HEAP

MemHeap::C_Heap* GetLargeBlockHeap()
{
	//static MemHeap::C_Heap heap("LargeHeap", 70 * 1024 * 1024); // didnt load bunker
	#ifdef PLATFORM_XENON
	static MemHeap::C_Heap heap("LargeHeap", 60 * 1024 * 1024); 
	#else
	static MemHeap::C_Heap heap("LargeHeap", 150 * 1024 * 1024); 
	#endif
	return &heap;
}

void *LargeBlockAlloc( size_t size )
{	
#ifdef USER_PAULM
	if( size> 5 * 1024 * 1024)
	{
		DebugBreak();
	}
#endif

	if( size < 1024 * 1024)
	{
		void* ptr = GetLargeBlockHeap()->Alloc( size );
		//GetHeap().Verify();
		return ptr;
	}

	void* ptr = GetLargeBlockHeap()->TrashAlloc( size );
	//GetHeap().Verify();
	return ptr;
}

void LargeBlockFree( void* ptr )
{
	GetLargeBlockHeap()->Free( ptr );
	//GetHeap().Verify();
}

size_t LargeBlockSize( void* ptr )
{
	return (size_t)MemHeap::MSize( ptr );
}

#else

void *LargeBlockAlloc( size_t size )
{
#ifdef PLATFORM_PS3
		return memalign( 16, size );
#else
		return malloc( size );
#endif
}

void LargeBlockFree( void* ptr )
{
	return free(ptr);
}

size_t LargeBlockSize( void* ptr )
{
	#if !defined( COMPILER_VC )
	return malloc_usable_size(ptr);
	#else
	return _msize(ptr);
	#endif
}

#endif

#define MEMSET_ALLOCATIONS
#define MEMSET_VALUE	0x00

#ifdef MEMORY_ALLOCATION_FRAME_DEBUG
#include <Common/Util/PerfStats.h>

Debug::Bool g_TrackNewOperator("System.Performance.TrackNewandDeleteOperators", false);
#endif

#ifdef USE_SMALL_BLOCK_ALLOCATOR
static int g_BlockAllocatorSizes[] = 
{
	16,32,48,64,80,128,144,160,208,224,240,256,
	0
};
const int g_BlockNumberOfSizes = (sizeof(g_BlockAllocatorSizes) / sizeof(g_BlockAllocatorSizes[0]));
Memory::BlockAllocator g_Allocator(g_BlockAllocatorSizes);
#endif

void* operator new( size_t size )
{	
	U8* ptr = new U8[ size ];
	return ptr;
}


void operator delete( void* ptr )
{
	U8* p = (U8*)ptr;
	delete [] p; 
}

void AssertOutOfMemory(const U32 size)
{
	// help
}

void* operator new[]( size_t size )
{
	const size_t iOrigSize = size;

#ifdef USE_NEW_MEMORY_TRACKER
	size += Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);
#endif

	int iActual;
	int iWaste;
	void *p;

#ifdef USE_SMALL_BLOCK_ALLOCATOR
	if (g_Allocator.Initialized() && ((p = g_Allocator.Allocate((U32)size)) != 0))
	{
		iActual = g_Allocator.AlocationSize(p);
		gSmallAllocCount++;
		iWaste = 0;
	}
	else
#endif
	{
		gOtherAllocCount++;

		p = LargeBlockAlloc(size);

		iWaste = 16;

		if (!p)
		{
			#if defined(PLATFORM_XENON) || defined(PLATFORM_PS3)
			AssertOutOfMemory(size);
			#endif
		}

		iActual = (int)LargeBlockSize( p );
	}
	
	if (p)
	{
		// Clear Mem (Need to remove this at a future date)
#ifdef MEMSET_ALLOCATIONS
		memset(p,MEMSET_VALUE,size);	
#endif
#ifdef USE_NEW_MEMORY_TRACKER
		// Add to memory tracker
		Memory::Tracker::Instance().TrackAllocation(p,				// allocation address
										 iOrigSize,					// original size
										 iActual + iWaste - Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS) ,	// actual size (the difference is used to calculate memory wastage)
										 Memory::kMemType_System,   // memory 'type'
										 (Memory::AllocationNode*)(((U8*)p) + iActual - Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS)), // address of tracking struct
										 MEMORY_TRACKER_FLAGS);
#endif
	}

    #ifdef CFG_DEBUG
    if ( !p ) DebugBreak();
	// Check for 16-byte alignment
	if ( (int)p & 0xf ) DebugBreak();
	#endif
	return p;
}

void operator delete[]( void* ptr )
{
	if ( ptr )
	{
		int iActual;
		int iWaste;
#ifdef USE_SMALL_BLOCK_ALLOCATOR
		if (g_Allocator.MemoryOwnedByAllocator(ptr))
		{
			iActual = g_Allocator.AlocationSize(ptr);
			iWaste = 0;
		}
		else
		{
			iWaste = 16;
#endif
			iActual = (int)LargeBlockSize( ptr );
#ifdef USE_SMALL_BLOCK_ALLOCATOR
		}
#endif

		memset(ptr, 0xcd, iActual);

#ifdef USE_NEW_MEMORY_TRACKER

			Memory::Tracker::Instance().TrackDeallocation(iActual + iWaste - Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS), // actual allocation size (used to track memory wastage)
											Memory::kMemType_System,
											(Memory::AllocationNode*)(((U8*)ptr) + iActual - Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS)) // address of tracking struct
										   );
#endif

#ifdef USE_SMALL_BLOCK_ALLOCATOR
		if (g_Allocator.MemoryOwnedByAllocator(ptr))
		{ 
			gSmallAllocCount--;
			g_Allocator.Free(ptr); 	
			return; 
		}
		else
		{
			gOtherAllocCount--;
#endif
			LargeBlockFree( ptr );
#ifdef USE_SMALL_BLOCK_ALLOCATOR
		}
#endif

	}

}

// if we dont want this heap, include source\misc_shared\Lua\LuaAlloc.cpp into the game project
namespace Lua
{

MemHeap::C_Heap* GetLuaHeap()
{
	#ifdef PLATFORM_XENON
	static MemHeap::C_Heap heap("LuaHeap", 12 * 1024 * 1024, false); 
	#else
	
	#ifdef TRACK_MEMORY_USAGE
	static MemHeap::C_Heap heap("LuaHeap", 7 * 1024 * 1024, false); 
	#else
	static MemHeap::C_Heap heap("LuaHeap", 6 * 1024 * 1024, false); 
	#endif
	
	#endif

	return &heap;
}

void* LuaAlloc( const U32 iOriginalSize )
{
#ifdef TRACK_MEMORY_USAGE
	const size_t iAllocSize = iOriginalSize + g_MemoryTrackerStructSize;
#else
	const size_t iAllocSize = iOriginalSize;
#endif

	void* pMem = GetLuaHeap()->Alloc(iAllocSize);

	if (pMem)
	{
		const U32 actualSize = MemHeap::MSize(pMem);

		#ifdef TRACK_MEMORY_USAGE
		TrackAllocation(pMem, iOriginalSize, actualSize, 0, Memory::kMemType_System, false);
		#endif
	}
	else
	{
//		LOGWARN(Game::g_Log, "LuaAlloc: Failed To Allocate %dbytes\n", iOriginalSize);
		//AssertOutOfMemory(iAllocSize);
	}

	return pMem;
}

void LuaFree( void* ptr )
{
	if (ptr)
	{
#ifdef TRACK_MEMORY_USAGE
		TrackDeallocation(ptr, MemHeap::MSize(ptr), 0, Memory::kMemType_System);
#endif
	}

	GetLuaHeap()->Free(ptr);
}

}
