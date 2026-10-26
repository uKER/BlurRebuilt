#include "Stdafx.h"

#include <System/Memory/MemoryTracker.h>
#include <System/Memory/MemoryTrackerDefines.h>
#include "OverloadNew.h"

void *LargeBlockAlloc( size_t size )
{	
	return BizNonTrackedMalloc( size );
}

void LargeBlockFree( void* ptr )
{
	BizNonTrackedFree(ptr);
}

size_t LargeBlockSize( void* ptr )
{
	return BizNonTrackedSize( ptr );
}

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

// extern void AssertOutOfMemory(const U32 size);

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
//		gOtherAllocCount++;

		p = LargeBlockAlloc(size);

		iWaste = 16;

		if (!p)
		{
			#if defined(PLATFORM_XENON) || defined(PLATFORM_PS3)
//			AssertOutOfMemory(size);
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
#ifndef PLATFORM_PC
	if ( (int)p & 0xf ) DebugBreak();
#endif
	#endif
	return p;
}

void operator delete[]( void* ptr )
{
	if ( ptr )
	{
		
		int iActual;
		int iWaste = 0;
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
