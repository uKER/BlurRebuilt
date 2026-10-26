#include "StdAfx.h"
#include "CoreSystems.h"
#include <System/Io/Async.h>
#include <System/Artery/Artery.h>

#if PLATFORM_PS3
#include <cell/sysmodule.h>
#include <sys/paths.h>
#include <netex/net.h>
#include <netex/libnetctl.h>
#endif

bool C_CoreSystems::Initialise()
{
	#if PLATFORM_PC
	bool success = Sys::C_FileSystem::Instance().SetWorkingDirectory("");
	#elif PLATFORM_XENON
	bool success = Sys::C_FileSystem::Instance().SetWorkingDirectory("GAME:");
	#elif PLATFORM_PS3
	bool success = Sys::C_FileSystem::Instance().SetWorkingDirectory(SYS_APP_HOME);
	#endif
	
	//BIZ_ASSERT(success);
	
	Sys::SetDefaultFileSystem(&Sys::C_FileSystem::Instance());

	IoInitialise();

	success = StartupArtery();
	BIZ_ASSERT(success);

	Memory::InitialiseNetworkConnection();
	
	Debug::NetworkStart();

	return true;
}

void C_CoreSystems::Shutdown()
{
	StartupArtery();

	IoShutdown();
}

void C_CoreSystems::Update()
{
	Memory::Update();

	Debug::NetworkUpdate();
}

bool C_CoreSystems::StartupArtery()
{
	U32 hr;

	#ifdef PLATFORM_PS3
	if((hr = cellSysmoduleLoadModule(CELL_SYSMODULE_NET)) != 0)
	{
		LOGERROR(Display::g_Log, "Failed calling cellSysmoduleLoadModule with error code %i\n", hr);
		return false;
	}
	if((hr = sys_net_initialize_network()) != 0)
	{
		LOGERROR(Display::g_Log, "Failed calling sys_net_initialize_network with error code %i\n", hr);
		return false;
	}
	if((hr = cellNetCtlInit()) != 0)
	{
		LOGERROR(Display::g_Log, "Failed calling cellNetCtlInit with error code %i\n", hr);
		sys_net_finalize_network();
		return false;
	}
	#endif

	#ifdef PLATFORM_XENON
	XNetStartupParams xnsp;
	ZeroMemory(&xnsp, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(xnsp);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY; 
	xnsp.cfgQosDataLimitDiv4 = 512/4;
	xnsp.cfgSecRegMax  = 64;
	if((hr = XNetStartup(&xnsp)) != 0)
	{
		LOGERROR(Display::g_Log, "Failed calling XNetStartup with error code %i\n", hr);
		return false;
	}
	#endif
	
	#if defined(PLATFORM_PC) || defined(PLATFORM_XENON)
	WSADATA wsaData;
	if((hr = WSAStartup(WINSOCK_VERSION, &wsaData)) != 0)
	{
		LOGERROR(Display::g_Log, "Failed calling WSAStartup with error code %i\n", hr);
		return false;
	}
	#endif
	
	if(!ArteryStartup(true, "llwright-TestDisplay"))
	{
		LOGERROR(Display::g_Log, "Failed calling ArteryStartup\n");
		return false;
	}

	return true;
}

void C_CoreSystems::ShutdownArtery()
{
	ArteryCleanup();

	#ifdef PLATFORM_PS3
	cellNetCtlTerm();
	sys_net_finalize_network();
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
	#endif

	#if defined(PLATFORM_PC) || defined(PLATFORM_XENON)
	WSACleanup();
	#endif
}
