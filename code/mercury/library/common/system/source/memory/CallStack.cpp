#include "SystemPch.h"

#include <system/serialise.h>
#include <core/utility/endian.h>

#if defined( PLATFORM_PC ) && !defined( _WIN64 )

// requires libraries dbghelp.lib psapi.lib

#include <dbghelp.h>
#include <psapi.h>

namespace Memory
{

int TraceStack(int *pBuffer, int kMaxStoreSymbols, STACKFRAME64 &stackFrame)
{
	int iBufPos = 0;
	HANDLE hProcess = ::GetCurrentThread();

	CONTEXT context;
	memset(&context,0,sizeof(context));

	const int kNumberTosSkip = 1;

	for(int i = 0; i < kMaxStoreSymbols + kNumberTosSkip; i++)
	{
		if( ::StackWalk64(
			IMAGE_FILE_MACHINE_I386,
			GetCurrentProcess(),
			GetCurrentThread(), // this value doesn't matter much if previous one is a real handle
			&stackFrame, 
			&context,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL ) )
		{
			if(i >= kNumberTosSkip)
			{
				pBuffer[iBufPos] = (U32)stackFrame.AddrPC.Offset;
				iBufPos++;
			}
		}
		else
		{
			break;
		}
	}

	return iBufPos;
}

void GetAddressInfo(void *pAddress, char *buffer)
{
	// Then any name for the symbol
	struct tagSymInfo
	{
		IMAGEHLP_SYMBOL64 symInfo;
		char nameBuffer[ 512 ];
	} SymInfo = { { sizeof( IMAGEHLP_SYMBOL ) } };

	IMAGEHLP_SYMBOL64 * pSym = &SymInfo.symInfo;
	pSym->MaxNameLength = 512;

	// Get the function name
	DWORD64 dwDisplacement = 0;
	if ( SymGetSymFromAddr64( ::GetCurrentProcess(), (DWORD)pAddress, &dwDisplacement, pSym) )
	{
		// Finally get any file/line number
		IMAGEHLP_LINE64 lineInfo;
		DWORD dwDisplacement2 = 0;
		lineInfo.Address = (DWORD)pAddress;
		lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		if ( SymGetLineFromAddr64( ::GetCurrentProcess(), (DWORD)pAddress, &dwDisplacement2, &lineInfo ) )
		{
			sprintf(buffer, "%s(%d): %s (%08x)", lineInfo.FileName, lineInfo.LineNumber, pSym->Name, pAddress);
		}
		else
		{
			sprintf(buffer, "???: %s", pSym->Name);
		}
	}
	else
	{
		sprintf(buffer, "0x%08x", pAddress);
	}
}

static int gbInitialized = false;


int CaptureCallStack(void **pCallStack, int cMax)
{
#if !defined(CFG_GOLD) && !defined(__LINT__)
	if (!gbInitialized)
	{
		// Initialize the symbol reading
		DWORD dwOpts	= ::SymGetOptions();

		dwOpts |= SYMOPT_LOAD_LINES;
		dwOpts &= ~SYMOPT_DEFERRED_LOADS;
		SymSetOptions( dwOpts );

		SymSetSearchPath(::GetCurrentProcess(), "c:\\windows\\symbols\\dll");

		SymInitialize(::GetCurrentProcess(), 0, true);

		gbInitialized = true;
	}

	// capture stack frame
	int iPCOffset, iFrameOffset, iSP;
	_asm call $+5
	_asm pop eax
	_asm mov iPCOffset, eax
	_asm mov iFrameOffset, ebp
	_asm mov iSP, esp

	// initialize stack frame
	STACKFRAME64 frame;
	memset(&frame, 0x0, sizeof(frame));
	frame.AddrPC.Offset = iPCOffset;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = iFrameOffset;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = iSP;
	frame.AddrStack.Mode = AddrModeFlat;

	return TraceStack((int*)pCallStack, cMax, frame) * 4;
#else
	return 0;
#endif
}

// 

#pragma warning(disable: 4311)

template <typename Archive>
void LogModule(Archive ar, char *psz, int iBase, int iSize)
{
	int c = (int)strlen(psz);

	ar << (int)strlen(psz);
	ar << iBase;
	ar << iSize;
	ar << c;
	for (int i=0; i<c; i++)
		ar << psz[i];
}

void SaveModuleInfo(Sys::I_BinaryStream *pStream)
{
	HANDLE h = GetCurrentProcess();

	DWORD c;
	int n;
	EnumProcessModules(h, NULL, 0, &c);
	n = c/sizeof(HMODULE);

	HMODULE *p = new HMODULE[n];
	EnumProcessModules(h, p, c, &c);

	SymSetSearchPath(h, "c:\\windows\\symbols\\dll");

	Sys::BinarySerialiser<Ut::NativeToLittleEndian> ar(*pStream);

	ar << (int)1; // platform PC

	for (int i=0; i<n; i++)
	{
		MODULEINFO info;
		GetModuleInformation(h, p[i], &info, sizeof(MODULEINFO));

		char Buffer[_MAX_PATH];
		GetModuleFileName(p[i], Buffer, _MAX_PATH);

		// add this module to the symbol server
		SymUnloadModule64(h, (int)info.lpBaseOfDll);
		if (SymLoadModule64(h,NULL,(PSTR)Buffer,NULL,(int)info.lpBaseOfDll,info.SizeOfImage))
		{
			IMAGEHLP_MODULE64 moduleInfo;
			moduleInfo.SizeOfStruct = sizeof(moduleInfo);
			SymGetModuleInfo64(h, (int)info.lpBaseOfDll, &moduleInfo);
			char *psz;
			if ((moduleInfo.SymType == SymNone) || (moduleInfo.SymType == SymExport))
			{
				psz = "no symbols loaded.";
			}
			else
			{
				psz = "loaded ok.";
			}
			char buffer[512];
			sprintf(buffer, "Module %s: %s (0x%08x).\n", Buffer, psz, (int)info.lpBaseOfDll);
			OutputDebugString(buffer);
		}

		LogModule(ar, Buffer, (int)info.lpBaseOfDll, info.SizeOfImage);
	}

	delete [] p;

	ar << (int)0;
}

}

#elif defined( PLATFORM_PS3 )

namespace Memory
{

int CaptureCallStack(void **pCallStack, int cMax)
{
#ifndef CFG_GOLD
    uint64_t* frameptr;
    asm ("mr %0, %%r1\n" : "=&b"(frameptr));
	int i;
    for (i=0; i<cMax && frameptr; frameptr=(uint64_t*)*frameptr)
    {
        if (frameptr[2])
        {
            pCallStack[i] = (void*)frameptr[2];
            i++;
        }
    }

	return i*sizeof(void*);
#else
	return 0;
#endif
}

}

#else

namespace Memory
{

int CaptureCallStack(void **pCallStack, int cMax)
{
#ifndef CFG_GOLD
	// TODO: implement for other platforms
#endif
	return 0;
}

}

#endif

