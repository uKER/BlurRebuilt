#include "CorePch.h"
#include <core/utility/coredebug.h>

#ifdef COMPILER_VC
#include <crtdbg.h>
#endif
#ifdef PLATFORM_PS3
#include <libsn.h>
#endif

#ifndef CFG_SPEED
	#ifdef COMPILER_VC
		#ifdef PLATFORM_PC
			#define WIN32_LEAN_AND_MEAN
			#define _WIN32_WINNT 0x0501 
			#include "windows.h"
			void DoErrorString(const char* string, const char* file, const U32 line)
			{
				char buffer[512];
				_snprintf(buffer, 512, "Fatal Error at File: %s \nLine: %d \nMessage: %s", file, line, string);
				OutputDebugString(buffer);
				if(IsDebuggerPresent())
				{              
					DebugBreak();
				}            
				FatalAppExit(0, buffer);
			}

		#endif
		void CORE_PRINT(const char* format, ...)
		{
			char buffer[ 1024 ];
		    
			va_list arglist;
			va_start( arglist, format );
			vsprintf_s( buffer, 1024, format, arglist );
			va_end( arglist );

			OutputDebugString(buffer);
		}
	#else
		void CORE_PRINT(const char* format, ...)
		{
			char buffer[ 512 ];
		    
			va_list arglist;
			va_start( arglist, format );
			vsprintf_s( buffer, 512, format, arglist );
			va_end( arglist );
			printf(buffer);
		}

	#endif
#endif

namespace CoreDebug
{

//int curFlags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
//curFlags |= _CRTDBG_CHECK_ALWAYS_DF;
//_CrtSetDbgFlag(curFlags);

CheckMemoryIntegrityFunc *gpMemoryIntegrityFunction = CheckMemoryIntegrity;

bool CheckMemoryIntegrity(void)
{
#if defined(CFG_DEBUG) && defined(PLATFORM_PC)
	if (!_CrtCheckMemory())
		return false;

#ifdef PLATFORM_PC
	HANDLE heaps[1025];
	DWORD nheaps = GetProcessHeaps(1024, heaps);

	for (DWORD i = 0; i < nheaps; i++)
	{
		if (!HeapValidate(heaps[i], 0, NULL))
			return false;
	}
#endif

#endif

	return true;
}

}