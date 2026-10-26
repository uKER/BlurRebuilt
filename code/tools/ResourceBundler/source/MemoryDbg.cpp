#include "stdafx.h"

#ifdef _DEBUG

#define CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#endif


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

#ifdef _DEBUG

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

//	if (nAllocType!=1 || !lRequest) return TRUE;

	if (lRequest == 257 )// && nSize == 6)
	{
		Log("blah\n");
	}
	

	if (lRequest == 334 && nSize == 1764)
	{
//		int x = 5;
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
//	char *p1, *p2;
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
	va_list	argptr;
	char	text[1024];
	va_start(argptr,error);
	vsprintf(text,error,argptr);
	va_end(argptr);

	OutputDebugString(text);
}


#endif
