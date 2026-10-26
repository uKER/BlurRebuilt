#include "StdAfx.h"

#include <Main.h>
#include <GameApp.h>
#include <GameSystems.h>

#include "BizAssertScreen.h"

#define MYAPPLICATIONMAIN MercuryGameApplicationMain

#define ENABLE_SMALL_BLOCK_ALLOCATOR

//#define ENABLE_MEMORY_TRACKER	// comment this in to enable memory tracking

#ifdef C_APPLICATION
#undef C_APPLICATION
#endif
#define C_APPLICATION C_GameApp

#ifdef C_CORESYSTEMS
#undef C_CORESYSTEMS
#endif
#define C_CORESYSTEMS Game::C_GameSystems

#ifdef PLATFORM_PS3
SYS_PROCESS_PARAM (1001, 128 * 1024) // (priority, stack size)
#endif

#include <Framework/Application/ApplicationMain.inl>

int MercuryGameApplicationMain(Framework::C_CoreSystemsParams& coreSystemsParams, Framework::C_ApplicationParams& applicationParams)
{
	helper_screens::InitialiseVisualAssertScreen(true);

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
		
#ifdef PLATFORM_PC
		applicationParams.m_InputUserCount				= 1;
#else
		// Hmmm apparently this crashes the PC build :-(
		applicationParams.m_InputUserCount				= 4;
#endif

		hr = pApplication->Initialise(applicationParams);
		if(SUCCEEDED(hr))
		{
			hr = pApplication->Run();
		}

		pApplication->Shutdown();
		SAFE_DELETE_PTR(pApplication);

		MEMORY_TRACKER_POP("application");
	}

	helper_screens::ShutdownVisualAssertScreen();

	pCoreSystems->Shutdown();
	SAFE_DELETE_PTR(pCoreSystems);

	MEMORY_TRACKER_POP("coresystems");

	return SUCCEEDED(hr) ? 0 : 1;
}