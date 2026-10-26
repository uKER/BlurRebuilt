#include "StdAfx.h"

#include <Main.h>
#include <GameApp.h>
#include <GameSystems.h>
#include <Game/GameControls/GameControls.h>

#include "Framework/Framework.h"
#include <Framework/shared_layers/DebugLayer.h>

#include "Framework/Application/ApplicationBase.h"
#include "Framework/Application/PerformanceLogging.h"

#include "Automation\GameDataLogging.h"
#include "Automation\GamePerfLogging.h"

#include "DebugSystems/DebugSystemsCrashInfo.h"
#include "Ui/Util/UiCrashInfo.h"
#include "Game/AmaxCam/CameraSystem.h"
#include <Game/Game.h>
#include <System/Io/Async.h>
#include <system/time/time.h>
#include <system/serialise/FsPerfLogger.h>

#include "BizAssertScreen.h"

#include "Advertising\Advertising.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#ifndef CFG_GOLD
#include "Ui\UiManager.h"
extern Debug::Enum<UI::C_UiManager::E_LaunchMode> g_UiDebugLaunchMode;
#endif

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

namespace SonyNetworkPlatform
{
	// defined in PlatformCallbacks_ps3.cpp
	extern const char* amax_network_platform_title_id;
}
#endif

#define MYAPPLICATIONMAIN AmaxApplicationMain

#define ENABLE_SMALL_BLOCK_ALLOCATOR

//#define ENABLE_GUARD_BLOCKS

//#define ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE

#if defined(CFG_RELEASE)
#define ENABLE_MEMORY_TRACKER	// comment this in to enable memory tracking
#endif

#ifdef ENABLE_MEMORY_TRACKER
// Please don't check this in set to 1
#define MEMORY_TRACKER_CALLSTACK 0		// 1 to record callstacks too, 0 to disable
#endif

#define ENABLE_OUT_OF_MEMORY_DEBUG_BREAK_IN_SPEED 1 // So out of memory warnings aren't ignored in speed

#include <framework/Application/ApplicationMain.inl>

#ifndef CFG_GOLD

void AdvertJumpToZoneCallback(const char* zoneName);

struct CrashInfo_s
{
	Game::C_LevelState*			levelState;
	DebugSystems::CrashInfo_s*	debugSystems;
	UI::CrashInfo_s*			ui;
	Framework::C_Application*	application;
	AmaxCam::C_CameraSystem*	cameraSystem;
} CrashInfo = { 0 };

#endif

void AmaxOutOfMemoryCallback(void)
{
	C_GameApp::GetInstance()->GetDebugLayer()->GetMemoryBudgets()->Dump();
}

const char* fileCachePriorityList[] =
{
	"GAME:\\cache\\gamedata.sig",
	"GAME:\\cache\\gamedata_boot.sig",
	"GAME:\\cache\\lut_rampak.pak",
	"GAME:\\cache\\shaders-speed.pak",
	"GAME:\\cache\\gamedata_boot.pak",
	"GAME:\\cache\\boot_rampak.pak",
	"GAME:\\cache\\gamedata.pak",
	"GAME:\\levels\\showroom\\level.level",
	"GAME:\\levels\\showroom\\level.levelstream",
	"GAME:\\levels\\showroom\\level.levelphysics",
	"GAME:\\levels\\showroom\\level.levelrender",
};

U64 g_globalTime(0);

U64 GetAppStartTime()
{
	return g_globalTime;
}

F32 GetAppTimeSeconds()
{
	U32 delta = (U32)(Time::GetPerformanceCounter() - GetAppStartTime());
	U32 durationMs = (U32)(delta / 1000);
	return durationMs / 1000.f;
}

void PrintTimeSinceAppStart()
{
	char buf[512];
	sprintf(buf, "Time since app start %.2f\n", GetAppTimeSeconds());
	LOGINFO(Framework::g_BootLog, buf);
	//OutputDebugString(buf);
}

int AmaxApplicationMain(Framework::C_CoreSystemsParams& coreSystemsParams, Framework::C_ApplicationParams& applicationParams)
{
	g_globalTime = Time::GetPerformanceCounter();
	FSPERF_SETMAINTHREADID(MT::GetThreadId());

	LOGDEBUG( Framework::g_BootLog, "AmaxApplicationMain()");

	Memory::SetOutOfMemoryCallback(AmaxOutOfMemoryCallback);

#ifndef PLATFORM_PS3
	extern MT::Event AudioStreamingEvent;
	AudioStreamingEvent.Signal();
#endif

	helper_screens::InitialiseVisualAssertScreen(true);

	HRESULT hr = S_OK;
		
	MEMORY_TRACKER_PUSH("coresystems");
	
	LOGDEBUG( Framework::g_BootLog, "Init CoreSystems - BEGIN");

	C_CORESYSTEMS* pCoreSystems = new C_CORESYSTEMS();

	// TODO: Initialise cross-platform core sytems params here.

#ifdef PLATFORM_PS3
	IoSetTitleIdPs3(SonyNetworkPlatform::amax_network_platform_title_id);
#endif

	IoCachePrioritise(fileCachePriorityList, lengthof(fileCachePriorityList));

	hr = pCoreSystems->Initialise(coreSystemsParams);

	LOGDEBUG( Framework::g_BootLog, "Init CoreSystems - END");

	if(SUCCEEDED(hr))
	{
		MEMORY_TRACKER_PUSH("application");

		C_APPLICATION* pApplication = new C_APPLICATION();

		// This is for the Virtual Tester to ensure we always play in Master mode
		// We also disable debug variable writing so we don't really mess up the end user's environment

		// TODO: Initialise cross-platform application params here.
		applicationParams.m_NetworkStackAlreadyStarted	= pCoreSystems->NetworkInitialised();
		applicationParams.m_InputButtonCount			= Game::E_UiButton_Count;
		applicationParams.m_InputAxisCount				= Game::E_UiAxis_Count;
		// [Blur2PC] Retail PC processes four input slots in C_UiManager::ProcessController
		// (Blur.exe 1.2, 0x00c348c0), including the second split-screen keyboard.
		applicationParams.m_InputUserCount				= 4;

		hr = pApplication->Initialise(applicationParams);


		Framework::g_debugPresentInterval = 2;

		AmaxPerfLogging::Initialise();
#ifndef CFG_GOLD
		AmaxDataLogging::Create();
#endif

		LOGDEBUG( Framework::g_BootLog, "Creating FS Layers - BEGIN");

		Game::C_GameSystems::Instance().CreateFsLayers();

		LOGDEBUG( Framework::g_BootLog, "Creating FS Layers - END");

		//LOGDEBUG( Framework::g_BootLog, "Caching Global Resources");
		
		//pApplication->CacheGlobalResources();

		LOGDEBUG( Framework::g_BootLog, "Advertising::Initialise - BEGIN");

#if defined(PLATFORM_PC)
		Advertising::Initialise("atvi_blur_pc", "0.0");
#endif

		LOGDEBUG( Framework::g_BootLog, "Advertising::Initialise - END");

#ifndef CFG_GOLD
		Advertising::SetJumpToZoneCallback(AdvertJumpToZoneCallback);

		// Helper pointers to aid with debugging
		{
			CrashInfo.debugSystems		= &DebugSystems::CrashInfo;
			CrashInfo.ui				= &UI::CrashInfo;
			CrashInfo.application		= Framework::C_Application::GetInstance();
			CrashInfo.cameraSystem		= &AmaxCam::CameraSystem();
			CrashInfo.levelState		= &Game::C_Game::GetInstance()->GetLevelState();
		}
#endif

		LOGDEBUG( Framework::g_BootLog, "Running Application");

		if(SUCCEEDED(hr))
		{
			hr = pApplication->Run();
		}

		pApplication->ReleaseAllGlobalResources();

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

void AdvertJumpToZoneCallback(const char* szZoneName)
{
	// Convert szZoneName back into "circuit" and "route"
	char circuitN[40];
	int lenCN = strlen(szZoneName);
	strncpy_s(circuitN, lengthof(circuitN), szZoneName, lenCN - 3);
	int routeN = atoi(szZoneName + lenCN - 2);

	// Now we need to get into the selected level - if we're already in one, need to quit and go in

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer() == NULL)
	{
		const char* uiScreen = UI::gUIScreenControl.GetNextSreen();

		if (!stricmp(uiScreen, "debug\\debugscreen.lua"))
		{
			bool foundRoute(false);

			for (int i=0; i < C_GameData::Instance().mRoutes.Number(); i++)
			{
				const AmaxRoute& r = *(C_GameData::Instance().mRoutes[i]);
				const AmaxCity& c = *r.city;

				if ((r.route_number == routeN) && (!stricmp(circuitN, c.tag.string)))
				{
					levelState.mCity = c.ID;
					levelState.mRoute = r.ID;
					foundRoute = true;
					break;
				}
			}

			if (!foundRoute)
			{
				//LOGERROR(g_Log, "CallbackTeleportToZone: Couldn't find zone '%s'", szZoneName);
				return;
			}

			levelState.mNumCars = 1;

			{
				Game::LevelStateCar& vehicle = levelState.mCars[0];

				vehicle.mCarID = 0xAA0FC396;
				vehicle.mCharacterID = C_RaceBookData::Instance().mCharacters[0]->ID;
				vehicle.mControlType = E_VEHICLE_CONTROL_TYPE_PLAYER;
				vehicle.mStartPosition = 0;

				vehicle.mUpgrades.DebugValues();
			}

			levelState.mDebugEvent = true;
			levelState.Finished();

			const S32 mask = UI::C_ScreenControlManager::Instance().MessageMask();
			_UI_MESSAGE2_MASK( UI::E_UiMsg_AutomationLoadGame, 0, 1, mask );
		}
	}
	else
	{
		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_QuitRace);
		UI::gUIScreenControl.SetNextScreen("Loading\\LoadingUI.lua");
	}
}
