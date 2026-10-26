#include "stdafx.h"

#include "tools_shared\MemoryDbg.h"

#ifdef COMPILER_VC
#include <crtdbg.h>

namespace Tools
{
/// custom allocation hook
/// use this hook to intercept a particular request number. this can be used as a method to track memory leaks
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
		return( 1 );

	if (nAllocType!=1 || !lRequest) return 1;

	if (lRequest == 1366)// && nSize == 471)	// change this line to trap a specific memory allocation
//	if (lRequest > 2000 && nSize == 65607)
	{
		int j = 10;
	}

	return( 1 ); 	   // Allow the memory operation to proceed
}

void InitialiseMemoryDbg()
{
	// set up our memory allocation hook
	_CrtSetAllocHook(MyAllocHook);

	// turn on memory leak reporting
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
}

};

#else

namespace Tools
{

void InitialiseMemoryDbg()
{
}

};

#endif


