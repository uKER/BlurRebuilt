// MemoryDbg.c
#include "stdafx.h"
#include "Core\Core.h"
#include "Core\Containers.h"
#include "lux_common\MemoryTrackerFlags.h"
#include "System\Memory.h"
#include "System\DebugLog.h"
#include <system\memory\MemoryTrackerDefines.h>
#include <core\allocators\CoreMemoryFunctions.h>

#ifdef _DEBUG

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <crtdbg.h>
#include <windows.h>

// ---- public variables ----

unsigned short	gMemClass;
unsigned short	gMemUser;

void Log(char *message, ...);

#ifdef _DEBUG

// This routine place comments at the head of a section of debug output
void OutputHeading( const char * explanation )
{
_RPT1( _CRT_WARN, "\n\n%s:\n**************************************\
	  ************************************\n", explanation );
}

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
_CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
_CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#else

#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)

#endif

static _CrtMemState s1, s2, s3;
static bool bOn=true;

void PrintMemCheck(void){};

void GoMemCheck(void)
{
}

void MemMirrorClear(void)
{
	if (!bOn) return;
	
	_CrtMemCheckpoint( &s1 );
}

void MemMirrorShow(void)
{
	if (!bOn) return;

	_CrtMemDumpAllObjectsSince(&s1);
}

void MemProtect(void *pMemory, int cSize, bool bOn)
{
	DWORD dummy;

	VirtualProtect(pMemory,cSize,bOn?PAGE_READONLY:PAGE_READWRITE,&dummy);
}

/* ALLOCATION HOOK FUNCTION
   -------------------------
   An allocation hook function can have many, many different
   uses. This one simply logs each allocation operation in a file.
*/
int __cdecl MyAllocHook(
						int 	nAllocType,
						void   * pvData,
						size_t	nSize,
						int 	nBlockUse,
						long	lRequest,
						const unsigned char * szFileName,
						int 	nLine
						)
{
	char *operation[] = { "", "allocating", "re-allocating", "freeing" };
	char *blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };

	if ( nBlockUse == _CRT_BLOCK )	// Ignore internal C runtime library allocations
		return( TRUE );

	if (nAllocType!=1 || !lRequest) return TRUE;

	if (lRequest == 419)// && nSize == 471)
	{
		int j = 10;
	}

/*

	if (nSize == 20)
	{
		if ( nBlockUse>=0 && nBlockUse<5 )
			Log(
				"Memory operation in %s, line %d: %s a %d-byte '%s' block (# %ld)\n",
				szFileName, nLine, operation[nAllocType], nSize, 
				blockType[nBlockUse], lRequest );
	}
//*/
//	_ASSERT( ( nAllocType > 0 ) && ( nAllocType < 4 ) );
//	_ASSERT( ( nBlockUse >= 0 ) && ( nBlockUse < 5 ) );
/*	
	Log(
		"Memory operation in %s, line %d: %s a %d-byte '%s' block (# %ld)\n",
		szFileName, nLine, operation[nAllocType], nSize, 
		blockType[nBlockUse], lRequest );
*/	
	return( TRUE ); 	   // Allow the memory operation to proceed
}

void MemMirrorOn(void)
{
	char *p1, *p2;
	/*
	// Send all reports to STDOUT
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
	*/
	
	_CrtSetAllocHook(MyAllocHook);

   	bOn=TRUE;
}

void MemMirrorOff(void)
{
	bOn=FALSE;
}


#else
void PrintMemCheck(void){};
void GoMemCheck(void){};
void MemMirrorClear(void){};
void MemMirrorShow(void){};
void MemMirrorOn(void){};
void MemMirrorOff(void){};
#endif

#endif

void Log(char *error,...)
{
#ifdef WIN32
	va_list	argptr;
	char	text[1024];
	va_start(argptr,error);
	vsprintf(text,error,argptr);
	va_end(argptr);

	OutputDebugString(text);
#endif
}

#include <system\memory.h>
 
void MemoryTrackerMessage(char *psz)
{
	OutputDebugString(psz);
}

static Debug::Logger &g_Log = Debug::GetLogger("tools.lux.memorytracker");

void InitializeMemoryTracker()
{
	Memory::Tracker::Instance().SetOutputCallBackFunction(MemoryTrackerMessage);
	Memory::Tracker::Instance().DumpBlocksOnExit(false);
	Memory::Tracker::Instance().DumpStatsOnExit(false);
}

#if (MEMORY_TRACKER_ENABLE == 1)

void *tracked_alloc( size_t size )
{
	const int c = Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);
	int orgsize = size;

	size += c;
 
    void *p = malloc(size);

#ifdef COMPILER_VC
    int iActual = (int)_msize(p);
#else
    int iActual = (int)malloc_usable_size(p);
#endif

    Memory::Tracker::Instance().TrackAllocation(p,       				// allocation address
                                     orgsize,  						    // original size
                                     iActual-c, 						// actual size (the difference is used to calculate memory wastage)
									 Memory::kMemType_System,			// memory 'type'
                                     (Memory::AllocationNode*)(((U8*)p) + iActual - c), // address of tracking struct
                                     MEMORY_TRACKER_FLAGS);

	return p;
}

void tracked_free( void *mem )
{
	if (!mem)
		return;

	const int c = Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);

#ifdef COMPILER_VC
    int iActual = (int)_msize(mem);
#else
    int iActual = (int)malloc_usable_size(mem);
#endif
	if (iActual >= c)
	{
		Memory::Tracker::Instance().TrackDeallocation(iActual-c, // actual allocation size (used to track memory wastage)
											Memory::kMemType_System,
											(Memory::AllocationNode*)(((U8*)mem) + iActual - c) // address of tracking struct
										   );
	}
    free(mem);
}

// Aligned Memory Allocation Functions
void *tracked_aligned_alloc(size_t size, size_t alignment)
{
#if ((defined _WIN64) || (defined WIN64))
	return tracked_alloc(size);
#else
	const int c = Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);

	int orgsize = size;

	size += c;
	if (size < alignment)
		size = alignment;

	void *p = _aligned_offset_malloc(size, alignment, c);
	if (!p)
		return NULL;

#ifdef COMPILER_VC
    int iActual = (int)_aligned_msize(p,alignment,c);
#else
    int iActual = (int)malloc_usable_size();
#endif
 
	Memory::Tracker::Instance().TrackAllocation((char*)p + c,	// allocation address
                                     orgsize,    					    // original size
                                     iActual-c, 						// actual size (the difference is used to calculate memory wastage)
									 Memory::kMemType_System,			// memory 'type'
									 (Memory::AllocationNode*)p,		// address of tracking struct
                                     MEMORY_TRACKER_FLAGS);

	return (char*)p + c;
#endif
}

void tracked_aligned_free(void *ptr)
{
#if ((defined _WIN64) || (defined WIN64))
	return tracked_free(ptr);
#else
	const int c = Memory::AllocationNode::Size(MEMORY_TRACKER_FLAGS);

	if (!ptr)
		return;

	ptr = (char*)ptr - c;

#ifdef COMPILER_VC
    int iActual = (int)_aligned_msize(ptr,16,c);
#else
    int iActual = (int)malloc_usable_size(ptr);
#endif
    Memory::Tracker::Instance().TrackDeallocation(iActual-c, // actual allocation size (used to track memory wastage)
										Memory::kMemType_System,
										(Memory::AllocationNode*)ptr // address of tracking struct
                                       );
    _aligned_free(ptr);
#endif
}

void *AllocAligned(size_t size)
{
	return tracked_aligned_alloc(size, 128);
}

void FreeAligned(void *ptr)
{
	tracked_aligned_free(ptr);
}


#else

void *tracked_alloc( size_t size )
{
	return malloc(size);
}

void tracked_free(void *p)
{
	free(p);
}

void *tracked_aligned_alloc( size_t size, size_t align)
{
	return _aligned_malloc(size,align);
}

void tracked_aligned_free(void *p)
{
	return _aligned_free(p);
}


// Aligned Memory Allocation Functions
void *AllocAligned(size_t size)
{
#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif
#ifdef GCC
	void *p = memalign(L1_CACHE_LINE_SIZE, size);
#else
	void *p = _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#endif
	if (!p)
		throw std::bad_alloc();

	return p;
}

void FreeAligned(void *ptr)
{
#ifdef GCC
	free(ptr);
#else
	_aligned_free(ptr);
#endif
}

#endif
