#include "wrap32/com/stdafx.h"
#include <core/core.h>
#include <system/debuglog.h>
#include <core/containers/string.h>
#include <system/debug/log/autoConfigure.h>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#include "tools_shared\ToolsApplication.h"
#include "tools_shared\MiniDumper.h"
#include "tools_shared\VersionStrings.h"
#include "tools_shared\ToolsAssert.h"
#include "tools_shared\MemoryDbg.h"
#include <tools_shared\VersionStrings.inl>

bool g_created = false;

static Debug::Logger& g_log = Debug::GetLogger("tools.wrap32_dll");

void myinvalid_param_handler( wchar_t const* expr, wchar_t const* func, const wchar_t* file, unsigned int line, uintptr_t ) ;
using namespace Tools;

void InitialiseToolsShared()
{
	// turn on memory leak reporting and tracking
	InitialiseMemoryDbg();

#ifdef COMPILER_VC
	// initialise network
	WSADATA	wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	// hook in tools assert
	::g_pCoreAssertFunction = ToolsAssert;

	// initialise logging system
	Debug::AutoConfigureDebugLog();

	// display header
	LOGINFO( g_log, "Wrap32 DLL Initialised");

	// initialise the mdump.cpp exception handler
	MiniDumper::InitialiseExceptionHandler();

#ifdef COMPILER_VC
	// put in a hook that intercepts crt asserts that would normally call _invoke_watson
	_set_invalid_parameter_handler(myinvalid_param_handler);
#endif
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (!g_created)
	{
#ifdef _DEBUG
//		MessageBox(NULL, TEXT("Wrap32 DllMain"), TEXT("Status"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif
		InitialiseToolsShared();
		g_created = true;
	}

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

