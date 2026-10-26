#include <Framework/Application/CoreSystems.h>
#include <Framework/Application/UserControls.h>

#include "System/Memory/MemoryTrackerDefines.h"
#include "System/Memory/MemoryTracker.h"

#include "System/Memory/AllocatorFunctions.inl"

#include "Framework/Application/PerformanceLogging.h"

#ifndef C_CORESYSTEMS
#pragma message("*****")
#pragma message("***** Please define C_CORESYSTEMS in your main.cpp e.g. #define C_CORESYSTEMS Framework::C_CoreSystems")
#pragma message("*****")
#endif

#ifndef C_APPLICATION
#pragma message("*****")
#pragma message("***** Please define C_APPLICATION in your main.cpp e.g. #define C_APPLICATION C_TestApp")
#pragma message("*****")
#endif

#ifndef MYAPPLICATIONMAIN
#define MYAPPLICATIONMAIN ApplicationMain
#define DEFAULT_APPLICATION_MAIN
#endif

// Code to catch a PS3 abort() before it actually gets there and makes debugging hard
#ifndef PS3_DISABLE_ABORT_OVERRIDE
#if defined(PLATFORM_PS3) && !defined(CFG_GOLD)
void std::abort(void)
{
     printf("Abort handler called!");
	 snPause();
	 while (1) ;
}
#endif
#endif

int MYAPPLICATIONMAIN(Framework::C_CoreSystemsParams& coreSystemsParams, Framework::C_ApplicationParams& applicationParams);

#ifdef DEFAULT_APPLICATION_MAIN

int ApplicationMain(Framework::C_CoreSystemsParams& coreSystemsParams, Framework::C_ApplicationParams& applicationParams)
{
	HRESULT hr = S_OK;
		
	MEMORY_TRACKER_PUSH("coresystems");
	
	C_CORESYSTEMS* pCoreSystems = new C_CORESYSTEMS();

	// TODO: Initialise cross-platform core sytems params here.

	hr = pCoreSystems->Initialise(coreSystemsParams);
	if(SUCCEEDED(hr))
	{
		MEMORY_TRACKER_PUSH("application");

		C_APPLICATION* pApplication = new C_APPLICATION();

		// TODO: Initialise cross-platform application params here.
		applicationParams.m_NetworkStackAlreadyStarted	= pCoreSystems->NetworkInitialised();
		applicationParams.m_InputButtonCount			= Framework::E_Button_Count;
		applicationParams.m_InputAxisCount				= Framework::E_Axis_Count;
		applicationParams.m_InputUserCount				= 1;

		hr = pApplication->Initialise(applicationParams);
		if(SUCCEEDED(hr))
		{
			hr = pApplication->Run();
		}

		pApplication->Shutdown();
		SAFE_DELETE_PTR(pApplication);

		MEMORY_TRACKER_POP("application");
	}

	pCoreSystems->Shutdown();
	SAFE_DELETE_PTR(pCoreSystems);

	MEMORY_TRACKER_POP("coresystems");

	return SUCCEEDED(hr) ? 0 : 1;
}

#endif

#ifdef ENABLE_MEMORY_TRACKER

Memory::AllocationNode staticMemoryAllocationNode;

#ifdef PLATFORM_PS3
Memory::AllocationNode ps3SystemMemoryAllocationNode;
Memory::AllocationNode ps3LocalMemoryAllocationNode;
#endif

#endif

#if defined(PLATFORM_PS3)

int main(int argc, char *argv[])
{
	PerformanceLogging::UpdateCounters(PerformanceLogging::kGameSubSessionStarted);

#ifdef ENABLE_MEMORY_TRACKER
	{
		sys_memory_info_t memInfo;
		sys_memory_get_user_memory_size(&memInfo);
		std::malloc_managed_size mms;
		malloc_stats(&mms);

		// This is memory used by on the exe and static allocations - before global allocations
		preStaticInitMemoryUsage = memInfo.total_user_memory - memInfo.available_user_memory;

		// What's used up minus the HUD stuff and any other stuff we already allocated and tracked
		DWORD staticMemUsage = memInfo.total_user_memory - memInfo.available_user_memory;

		DWORD diff = staticMemUsage - preStaticInitMemoryUsage + mms.current_inuse_size;

		int tracked = Memory::Tracker::Instance().TotalMemory();

		// Real static is the difference between static allocations and the memory we tracked since the game started up
		int staticMemorySize = preStaticInitMemoryUsage + (diff - tracked);

		{
			size_t sysMainSize = 43 * 1024 * 1024;
			size_t sysLocalSize = 7 * 1024 * 1024;

			MEMORY_TRACKER_PUSH_DYNAMIC("console.systemmain")
			Memory::Tracker::Instance().TrackAllocation(
				NULL, sysMainSize, sysMainSize, Memory::kMemType_System, &ps3SystemMemoryAllocationNode, MEMORY_TRACKER_FLAGS);
			MEMORY_TRACKER_POP("console.systemmain");
			
			MEMORY_TRACKER_PUSH_DYNAMIC("console.systemlocal")
			Memory::Tracker::Instance().TrackAllocation(
				NULL, sysLocalSize, sysLocalSize, Memory::kMemType_Local, &ps3LocalMemoryAllocationNode, MEMORY_TRACKER_FLAGS);
			MEMORY_TRACKER_POP("console.systemlocal");
		}

		MEMORY_TRACKER_PUSH_DYNAMIC("console.static")
		Memory::Tracker::Instance().TrackAllocation(
			NULL, staticMemorySize, staticMemorySize, Memory::kMemType_System, &staticMemoryAllocationNode, MEMORY_TRACKER_FLAGS);
		MEMORY_TRACKER_POP("console.static");
	}
#endif

#ifdef ENABLE_MEMORY_TRACKER
	Debug::Logger &g_Logger(Debug::GetLogger("application"));
	LOGWARN(g_Logger, "********************************************************************\n");
	LOGWARN(g_Logger, "WARNING: Memory Tracking is ENABLED\n");
#if MEMORY_TRACKER_CALLSTACK
	LOGWARN(g_Logger, "WARNING: Memory Tracking + CALLSTACKS is ENABLED - This eats memory!\n");
#endif
	LOGWARN(g_Logger, "********************************************************************\n");
#endif

	MEMORY_TRACKER_PUSH("main");
	
	Framework::C_CoreSystemsParams coreSystemsParams;
	Framework::C_ApplicationParams applicationParams;

	int errorCode = 0;
	
	errorCode = MYAPPLICATIONMAIN(coreSystemsParams, applicationParams);

	MEMORY_TRACKER_POP("main");

	Memory::Tracker::Instance().DumpNodeStats(false, true);

// Termination Processing For PS3
	sys_process_exit(0);

	return errorCode;
}

#elif defined(PLATFORM_PC)

#if defined(CFG_DEBUG)
#include <crtdbg.h>
#endif

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// [Blur2PC] Retail names this mutex "BlurMutex" (found via Ghidra on Blur.exe 1.2) and checks
	// for it before anything else runs; a second instance without this guard starts to a black
	// screen. Keep the mutex handle open for the process lifetime; the OS closes it on exit.
	// A plain return (retail's own mechanism: verified against Blur.exe, exit code 0) needed the
	// Game::C_ReplayDirector::Destroy null-guard fix alongside this; ExitProcess was a stand-in
	// for that missing fix and skipped DLL_PROCESS_DETACH for every loaded DLL, unlike retail.
	HANDLE hSingleInstanceMutex = CreateMutexA(NULL, FALSE, "BlurMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	PerformanceLogging::UpdateCounters(PerformanceLogging::kGameSubSessionStarted);

	#ifdef CFG_DEBUG
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
	#endif

	//TODO: Add this to prestatic init of some description to catch static mem leaks
	#ifdef ENABLE_MEMORY_TRACKER
		Memory::Tracker::Instance().DumpBlocksOnExit(true);
	#endif

#ifdef ENABLE_MEMORY_TRACKER
	Debug::Logger &g_Logger(Debug::GetLogger("application"));
	LOGWARN(g_Logger, "********************************************************************\n");
	LOGWARN(g_Logger, "WARNING: Memory Tracking is ENABLED\n");
#if MEMORY_TRACKER_CALLSTACK
	LOGWARN(g_Logger, "WARNING: Memory Tracking + CALLSTACKS is ENABLED - This eats memory!\n");
#endif
	LOGWARN(g_Logger, "********************************************************************\n");
#endif

	MEMORY_TRACKER_PUSH("main");
	
	Framework::C_CoreSystemsParams coreSystemsParams;
	Framework::C_ApplicationParams applicationParams;
	applicationParams.m_PC.m_hInstance = hInstance;
		
	int errorCode = 0;
	
	errorCode = MYAPPLICATIONMAIN(coreSystemsParams, applicationParams);

	Memory::Tracker::Instance().DumpNodeStats(false, true);

	MEMORY_TRACKER_POP("main");

	return errorCode;
}

#endif
