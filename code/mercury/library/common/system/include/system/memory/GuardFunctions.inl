
// Guard Blocks requires the Small Block Allocator to function (but disables their use for the game code)

// Not currently compatible with the Memory Tracker (so it disables it)

// Don't include this header directly in your code (it might work - but is designed to come in through AllocatorFunctions.inl)

#pragma message("****************************************")
#pragma message("WARNING: Allocation Guard's - ENABLED!!!")
#pragma message("****************************************")

#include <system/memory/CallStack.h>

// Doesn't support both running right now
#undef ENABLE_MEMORY_TRACKER

// Currently can't change this without dire results!
#define GUARD_BLOCK_SIZE 4

#ifndef GUARD_STACK_DEPTH
#define GUARD_STACK_DEPTH 4
#endif

extern Memory::BlockAllocator g_SmallBlockAllocator;

namespace Memory
{

	static Debug::Logger &g_GuardLog = Debug::GetLogger("mercury.system.guard");

	static Debug::Int g_GuardCheckFrequency("System.Memory.GuardCheckFrequency", 30, Debug::Range(-1, 10000), Debug::Persist());

	bool CheckGuards();

	struct GuardTrackHeader
	{
		GuardTrackHeader() : memory(NULL), prev(NULL), next(NULL)
		{
			CoreDebug::gpMemoryIntegrityFunction = CheckGuards;
		}
		
		GuardTrackHeader(void* m, GuardTrackHeader* p, GuardTrackHeader* n, size_t s) : memory(m), prev(p), next(n), size(s)
		{
		}

		void*				memory;
		GuardTrackHeader*	prev;
		GuardTrackHeader*	next;
		size_t				size;
#if GUARD_STACK_DEPTH > 0
		void*				miniCallstack[GUARD_STACK_DEPTH];
#endif

		static bool			initialised;

		static MT::Lock& Lock()
		{
			static MT::Lock lock;

			return lock;
		}
	};

	bool GuardTrackHeader::initialised = false;

	GuardTrackHeader g_GuardRoot;

// Add memory guard and put in linked list
void AddMemoryGuards(void* pAllocatedMemory, size_t alignment, size_t allocationSize)
{
	GuardTrackHeader* trackHeader;

	if (g_SmallBlockAllocator.Initialized())
		trackHeader	= (GuardTrackHeader*)g_SmallBlockAllocator.Allocate(sizeof(GuardTrackHeader));
	else
		trackHeader	= (GuardTrackHeader*)malloc(sizeof(GuardTrackHeader));

	BIZ_ASSERT(trackHeader != NULL);

	trackHeader->prev	= &g_GuardRoot;
	trackHeader->memory = pAllocatedMemory;
	trackHeader->size	= allocationSize;

#if GUARD_STACK_DEPTH > 0
	{
		// store callstack
		void *pCallStack[GUARD_STACK_DEPTH + 4];

		int callstackSize = CaptureCallStack(pCallStack, GUARD_STACK_DEPTH + 4);
			
		if (callstackSize > 0)
		{
			// How much to trim off the callstack to allow for the function call level getting here
			int callstackOverhead = 0;
			int callStackSkip = 4 * sizeof(void*);

			callstackSize -= callStackSkip + callstackOverhead;

			memcpy(&trackHeader->miniCallstack[0], ((U8*)pCallStack) + callStackSkip + callstackOverhead, sizeof(void*) * GUARD_STACK_DEPTH);
		}
		else
		{
			memset(&trackHeader->miniCallstack[0], 0, sizeof(void*) * GUARD_STACK_DEPTH);
		}
	}
#endif

	*((GuardTrackHeader**)(((size_t)pAllocatedMemory) - 16))	= trackHeader;
	*((DWORD*)(((size_t)pAllocatedMemory) - 12))				= alignment;
	*((DWORD*)(((size_t)pAllocatedMemory) - 8))					= allocationSize;

	U8* header	= (U8*)(((size_t)pAllocatedMemory) - 4);
	header[0]	= header[1] = header[2] = header[3] = 0x8a;
	
	U8* footer	= (U8*)(((size_t)pAllocatedMemory) + allocationSize);
	footer[0]	= footer[1] = footer[2] = footer[3] = 0x8a;

	// Insert block into guard tracking double-linked list
	if (GuardTrackHeader::initialised)
		GuardTrackHeader::Lock().Enter();

	trackHeader->next		= g_GuardRoot.next;
	if (g_GuardRoot.next)
		g_GuardRoot.next->prev = trackHeader;
	g_GuardRoot.next		= trackHeader;

	if (GuardTrackHeader::initialised)
		GuardTrackHeader::Lock().Exit();
}

// Remove from memory guard list and check everything is ok
size_t RemoveMemoryGuards(void* lpAddress)
{
	GuardTrackHeader*	trackHeader				= *(GuardTrackHeader**)(((size_t)lpAddress) - 16);
	GuardTrackHeader*	trackHeaderFromAlloc	= *((GuardTrackHeader**)(((size_t)trackHeader->memory) - 16));

	bool somethingBadFound = false;

	if (trackHeaderFromAlloc != trackHeader)
	{
		LOGERROR(g_GuardLog, "CheckGuards: @(0x%x) - (Memory::GuardTrackHeader*)0x%x", (((size_t)trackHeader->memory) - 16), trackHeader);
		somethingBadFound = true;
	}

	DWORD*	alignPtr	= (DWORD*)(((size_t)lpAddress) - 12);
	DWORD*	sizePtr		= (DWORD*)(((size_t)lpAddress) - 8);

	if (*sizePtr & 0x80000000)
	{
		LOGERROR(g_GuardLog, "CheckGuards: @(0x%x) - (Memory::GuardTrackHeader*)0x%x - Memory already deleted", (((size_t)trackHeader->memory) - 16), trackHeader);
		somethingBadFound = true;
		DebugBreak();
	}

	if (*sizePtr != trackHeader->size)
	{
		LOGERROR(g_GuardLog, "CheckGuards: Size difference (%d)-(%d) @(0x%x) - (Memory::GuardTrackHeader*)0x%x", *sizePtr, trackHeader->size, (((size_t)trackHeader->memory) - 8), trackHeader);
		somethingBadFound = true;
	}

	const U8* header = (U8*)(((size_t)lpAddress) - 4);
	if (header[0] != 0x8a || header[1] != 0x8a || header[2] != 0x8a || header[3] != 0x8a)
	{
		LOGERROR(g_GuardLog, "CheckGuards: Header@(0x%x) - (Memory::GuardTrackHeader*)0x%x", header, trackHeader);
		somethingBadFound = true;
	}
	
	const U8* footer = (U8*)(((size_t)lpAddress) + trackHeader->size);
	if (footer[0] != 0x8a || footer[1] != 0x8a || footer[2] != 0x8a || footer[3] != 0x8a)
	{
		LOGERROR(g_GuardLog, "CheckGuards: Footer@(0x%x) - (Memory::GuardTrackHeader*)0x%x", footer, trackHeader);
		somethingBadFound = true;
	}

	if (somethingBadFound)
	{
		DebugBreak();
	}

	GuardTrackHeader::Lock().Enter();
	if (trackHeader->next)
		trackHeader->next->prev	= trackHeader->prev;
	trackHeader->prev->next	= trackHeader->next;
	GuardTrackHeader::Lock().Exit();

	// Flag this as having been deleted already
	*sizePtr |= 0x80000000;

	if (g_SmallBlockAllocator.MemoryOwnedByAllocator(trackHeader))
		g_SmallBlockAllocator.Free(trackHeader);
	else
		free(trackHeader);

	return *alignPtr;
}

size_t g_guardMin;
size_t g_guardMax;

void UpdateGuardMinMax(bool badAlready, size_t ptrValue)
{
	if (badAlready)
	{
		g_guardMin = Maths::Min(g_guardMin, ptrValue);
		g_guardMax = Maths::Max(g_guardMax, ptrValue);
	}
	else
	{
		g_guardMin = g_guardMax = ptrValue;
	}
}

bool CheckGuards(void)
{
	static U32 checkCounter(0);

	if ((g_GuardCheckFrequency == -1) || ((g_GuardCheckFrequency != 0) && (checkCounter++ % g_GuardCheckFrequency != 0)))
		return true;

	GuardTrackHeader::Lock().Enter();

	const GuardTrackHeader* trackHeader = g_GuardRoot.next;

	g_guardMin = g_guardMax = 0;

	bool somethingBadFoundHere = false;

	for (; trackHeader; trackHeader=trackHeader->next)
	{
		GuardTrackHeader* trackHeaderFromAlloc = *((GuardTrackHeader**)(((size_t)trackHeader->memory) - 16));

		bool somethingBadFound = false;

		if (trackHeaderFromAlloc != trackHeader)
		{
			size_t ptrValue = (((size_t)trackHeader->memory) - 16);
			LOGERROR(g_GuardLog, "CheckGuards: @(0x%x) - (Memory::GuardTrackHeader*)0x%x", ptrValue, trackHeader);
			UpdateGuardMinMax(somethingBadFoundHere, ptrValue);
			somethingBadFound = true;
		}

		size_t alignment		= *((DWORD*)(((size_t)trackHeader->memory) - 12));
		size_t allocationSize	= *((DWORD*)(((size_t)trackHeader->memory) - 8));

		if (allocationSize != trackHeader->size)
		{
			size_t ptrValue = (((size_t)trackHeader->memory) - 8);
			LOGERROR(g_GuardLog, "CheckGuards: Size difference (%d)-(%d) @ (0x%x) - (Memory::GuardTrackHeader*)0x%x", allocationSize, trackHeader->size, ptrValue, trackHeader);
			UpdateGuardMinMax(somethingBadFoundHere, ptrValue);
			somethingBadFound = true;
		}
		
		U8* header	= (U8*)(((size_t)trackHeader->memory) - 4);
		
		if (header[0] != 0x8a || header[1] != 0x8a || header[2] != 0x8a || header[3] != 0x8a)
		{
			LOGERROR(g_GuardLog, "CheckGuards: Header@(0x%x) - (Memory::GuardTrackHeader*)0x%x", header, trackHeader);
			UpdateGuardMinMax(somethingBadFoundHere, (size_t)header);
			somethingBadFound = true;
		}
		
		U8* footer	= (U8*)(((size_t)trackHeader->memory) + trackHeader->size);

		if (footer[0] != 0x8a || footer[1] != 0x8a || footer[2] != 0x8a || footer[3] != 0x8a)
		{
			LOGERROR(g_GuardLog, "CheckGuards: Footer@(0x%x) - (Memory::GuardTrackHeader*)0x%x", footer, trackHeader);
			UpdateGuardMinMax(somethingBadFoundHere, (size_t)footer);
			somethingBadFound = true;
		}

		if (somethingBadFound)
		{
			LOGERROR(g_GuardLog, "CheckGuards: ************************");
			somethingBadFoundHere = true;
		}
	}

	if (somethingBadFoundHere)
	{
		LOGERROR(g_GuardLog, "CheckGuards: Range (%x)->(%x) - Size(%d)", g_guardMin, g_guardMax, g_guardMax - g_guardMin);
		DebugBreak();
	}

	GuardTrackHeader::Lock().Exit();

	return true;
}

}
