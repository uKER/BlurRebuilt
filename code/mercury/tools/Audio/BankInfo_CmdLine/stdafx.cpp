// stdafx.cpp : source file that includes just the standard includes
// BankInfo.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

//#include "../SystemHeaders.h"
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// this is so we dont have to include library/audio/source/system.cpp.
// including system.cpp would mean we would need to include a lot of the audio system's source files
namespace Audio
{
	AllocateMemory System::m_pfnAlloc = NULL;
	FreeMemory System::m_pfnFree = NULL;
	AllocateMemory System::m_pfnPhysAlloc = NULL;
	FreeMemory System::m_pfnPhysFree = NULL;

	DbgMessage System::m_DebugMessageCallback = NULL;
	DbgWarning System::m_DebugWarningCallback = NULL;
	DbgError System::m_DebugErrorCallback = NULL;

	void* System::Alloc(size_t uSize, U32 uAlignment, Audio::System::AllocType eType)
	{
		return malloc(uSize);
	}
	void System::Free(void* pAddress, Audio::System::AllocType eType)
	{
		free(pAddress);
	}

	Audio::FileOpenFunc System::m_openFunc = NULL;
	Audio::FileReadFunc System::m_readFunc = NULL;
	Audio::FileWriteFunc System::m_writeFunc = NULL;
	Audio::FileCloseFunc System::m_closeFunc = NULL;
	Audio::FileSizeFunc System::m_sizeFunc = NULL;
}

void Audio::Assert(bool bExp, char* strMessage)
{
	if (!bExp)
	{
		Print(FOREGROUND_RED | FOREGROUND_INTENSITY, strMessage);
		DebugBreak();
	}
}

////////////////////////////////////////////////////////////////////////////////
//	LogMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::LogMessage(const char* pchMessage, ...)
{
	char		out[256];
	va_list		va;

	// if not parameter set, write a new line
	if(pchMessage == NULL) 
		sprintf(out,"\n");
	// process arguments
	else
	{
		va_start(va, pchMessage);
			vsprintf(out, pchMessage, va);
		va_end(va);	
	}		
   
	// write to the console
	Print(0, out);
}

////////////////////////////////////////////////////////////////////////////////
//	LogInfo
////////////////////////////////////////////////////////////////////////////////
void Audio::LogInfo(const char* pchMessage, ...)
{
	char		out[256];
	va_list		va;

	// if not parameter set, write a new line
	if(pchMessage == NULL) 
		sprintf(out,"\n");
	// process arguments
	else
	{
		va_start(va, pchMessage);
			vsprintf(out, pchMessage, va);
		va_end(va);	
	}		
   
	// write to the console
	Print(0, out);
}

////////////////////////////////////////////////////////////////////////////////
//	LogWarning
////////////////////////////////////////////////////////////////////////////////
void Audio::LogWarning(const char* pchMessage, ...)
{
	char		out[256];
	va_list		va;

	// if not parameter set, write a new line
	if(pchMessage == NULL) 
		sprintf(out,"\n");
	// process arguments
	else
	{
		va_start(va, pchMessage);
			vsprintf(out, pchMessage, va);
		va_end(va);	
	}		
   
	// write to the console
	Print(0, out);
}

////////////////////////////////////////////////////////////////////////////////
//	LogError
////////////////////////////////////////////////////////////////////////////////
void Audio::LogError(const char* pchMessage, ...)
{
	char		out[256];
	va_list		va;

	// if not parameter set, write a new line
	if(pchMessage == NULL) 
		sprintf(out,"\n");
	// process arguments
	else
	{
		va_start(va, pchMessage);
			vsprintf(out, pchMessage, va);
		va_end(va);	
	}		
   
	// write to the console
	Print(0, "\n");
	Print(FOREGROUND_RED | FOREGROUND_INTENSITY, out);
}

////////////////////////////////////////////////////////////////////////////////
//	LogFatal
////////////////////////////////////////////////////////////////////////////////
void Audio::LogFatal(const char* pchMessage, ...)
{
	char		out[256];
	va_list		va;

	// if not parameter set, write a new line
	if(pchMessage == NULL) 
		sprintf(out,"\n");
	// process arguments
	else
	{
		va_start(va, pchMessage);
			vsprintf(out, pchMessage, va);
		va_end(va);	
	}		
   
	// write to the console
	Print(FOREGROUND_RED | FOREGROUND_INTENSITY, out);
	DebugBreak();
}

#ifdef I_BROKE_THE_HEDGE_TRIMMER
////////////////////////////////////////////////////////////////////////////////
//	LogMessage
//	including this to save having to include util.cpp
////////////////////////////////////////////////////////////////////////////////
void Audio::LogMessage(const char* pchMessage, ...)
{
	char string[1024];
	va_list arglist;
	
	va_start(arglist, pchMessage);
	vsprintf(string, pchMessage, arglist);
	va_end(arglist);

    strcat(string, "\n");

    OutputDebugString(string);
}

////////////////////////////////////////////////////////////////////////////////
//	LogError
//	including this to save having to include util.cpp
////////////////////////////////////////////////////////////////////////////////
void Audio::LogError(const char* pchError, ...)
{
	char string[1024];
	va_list arglist;
	
	va_start(arglist, pchError);
	vsprintf(string, pchError, arglist);
	va_end(arglist);

    strcat(string, "\n");

    OutputDebugString(string);
}
#endif