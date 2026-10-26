#include "StdAfx.h"

#include <system/serialise/FsPerfLogger.h>

#include "GameApp.h"

#define RENDERINGDATA_ALLOW_INCLUDE // just for GameApp.cpp because it deals directly with the renderingdata library

#include <Framework/Framework.h>
#include <Framework/Application/CaptureService.h>
#include <Framework/shared_layers/DebugLayer.h>
#include <Game/Game.h>
#include <ui/Game/SaveSystem/SaveSystem.h>

//#include <Game/SimpleUi/SimpleUiLayer.h>
#include "Ui/UiLayer.h"
#include <ui/Graphics/SimpleTimer.h>
#include <Game/GameWorld/PostProcessLayer.h>
#include <Game/SystemLayer.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <Game/GameWorld/GameWorld.h>	// [Blur2PC] ApplyGraphicalDetail
#include <GameAudio/GameAudioLayer.h>
#include <GameLogic/Layers/BootLayer.h>

#include <Display/Occlusion/OcclusionQuery.h>
#include <Graphics/NormalBuffer.h>

#include <fx/I_Effect.h>
#include <fx/fx.h>

#include <Xt/XtGame.h>

#include <Vehicle/VehicleModelManager.h>

#include <Game/GameControls/GameControls.h>
#include <Network/Core.h>
#include <network/CommunicatorManager.h>
#include <SystemProfile/SystemProfileManager.h>

#include "PlatformCallbacks/PlatformCallbacks.h"
#include "Network/Core/NetServices.h"

#include "Automation\GameAutomationMessageHandler.h"
#include "Automation\GamePerfLogging.h"

#include <game_resource/ModelResource.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>

#include "renderingdata/renderingdata.h"
#include <SchedulerConfig.h>

#include "Graphics/GpuMarkers.h"
#include "AppPhotoCapture.h"

#include <Display/Context/Context.h>
#include <game_resource/ModelTypeManager.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/RenderModel.h>

#include <Game/GameUtil/ResourceBag.h>
#include "Advertising/Advertising.h"
#include <game/AmaxCam/CameraSystem.h>

#include <fx/RenderMutex.h>

#include <dynamic/renderer.h>

#include <System/Io/Async.h>

#include <GameSystems.h>

#include <SpecialFX/AnimatedMesh.h>

#if defined (PLATFORM_PS3)
#include <cell/sysmodule.h>
#include <np.h>
#include <PS3Installer/PS3InstallData.h>
#include <sysutil/sysutil_bgmplayback.h>
#endif

#if PLATFORM_PS3
#include "vehicle\damage\SPU\SPUDamageContext.h"
#endif

#if PLATFORM_PS3
#include <Display/Buffer/GCM/GCMDebugCommandBuffer.h>
#endif

#include <ui/Graphics/Movie.h>
#include <PakFs/PakFsManager.h>

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

namespace Audio
{
	extern bool g_bQuitAllTasks;
#ifdef PLATFORM_PC
	void ShutdownOutputEngine();	// [Blur2PC] T144 (engine_pc.cpp)
#endif
};

Debug::Bool g_DebugUiBloomUpdateEnabled( "Test.BloomUpdateEnabled", true, Debug::Persist() );

#ifndef CFG_GOLD
Debug::Bool g_DebugNoticeGameUpdate("Test.DoNotUse.Dangerous.Notice", false);
Debug::Bool g_DebugWarningGameUpdate("Test.DoNotUse.Dangerous.Warning", false);
Debug::Bool g_DebugPanicGameUpdateMsg("Test.DoNotUse.Dangerous.Panic", false);

Debug::Bool g_DebugCoreAssertGameUpdate("Test.DoNotUse.Dangerous.CoreAssertInUpdate", false);

Debug::Bool g_DebugAssertGameUpdate("Test.DoNotUse.Dangerous.AssertInUpdate", false);
Debug::Bool g_DebugAssertGameUpdateMsg("Test.DoNotUse.Dangerous.AssertInUpdateWithMsg", false);

Debug::Bool g_DebugAssertGameRender("Test.DoNotUse.Dangerous.AssertInRender", false);

Debug::Bool g_DebugCrashGame("Test.DoNotUse.Dangerous.CrashMe", false);
#endif

#if DEBUG_COMMAND_BUFFER
Debug::Bool g_DumpUsedRingBufferMemory("Graphics.Render.DumpUsedRingBufferMemory", false);
#endif

#if !CFG_GOLD
Debug::Bool g_UpdatePresentRender("Graphics.Render.UpdatePresentRender", false);
#endif

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

Debug::Bool g_updateStreamerOnLoadingScreen("Game.UpdateStreamerOnLoadingScreen", true);

using namespace Game;

//Amax Model Type callback
class C_AmaxModelType : public GameResource::I_ModelType
{
public:
	C_AmaxModelType(){}
	void OnLoad(GameResource::C_ModelResource* p_pModelResource);
public:
};

void C_AmaxModelType::OnLoad( GameResource::C_ModelResource* p_pModelResource )
{
	GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
	WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

	// for each model inside the file
	for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
	{
		GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
		WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

		pWRModel->ExposeParameter("g_SHNode", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
		pWRModel->ExposeParameter("g_tEnvironment", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
		pWRModel->ExposeParameter("g_vShockIntensity", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
	}
}

const int MAX_GLOBAL_XT_DATABASE_RECORDS=15000;

C_GameApp::C_GameApp() 
	: m_pGame(NULL)
#ifdef _SIMPLE_UI_
	, m_pSimpleUiLayer(NULL)
#endif
#ifndef CFG_GOLD
	, m_pGameAutomationMessageController(NULL)
	, m_pGamePerformanceMessageController(NULL)
#endif
	, m_pUiLayer(NULL)
	, m_pDebugLayer(NULL)
	, m_pSystemLayer(NULL)
	, m_NetworkStartedByMercury(false)
	, m_pRaceBookLayer(NULL)
	, m_pBootLayer(NULL)
	, m_LoadingScreenTimeStep( 1.0f / 30.0f )
	, m_cachePostBootFxResourceBagStarted(false)
	, m_cachePostBootAnimationsStarted(false)
	, m_setBootPreCachingComplete(false)
	, m_installerScreenEnabled(false)
	, m_loadingSegEnabled(true)
{
#ifdef PLATFORM_PC
	m_Class.m_WndClassName = "Blur Class";
	m_Window.m_Name = "Blur";
#endif
	m_pGame = new C_Game();
	m_hSpecialFXResourceBag = NULL;
	m_TimeStep			= 0.0f;
	m_TimeScale			= 1.0f;
	m_TimeScaleTimeOut	= 0.0f;
	m_ElapsedRaceTime	= 0.0f;
#ifdef PLATFORM_PS3	
	m_SystemMenuActive	= false;
	m_QuitApplication	= false;
#endif
#ifndef CFG_GOLD
	Framework::g_CaptureService.m_AppAudioCapture = AppAudioCapture;
#endif
}

C_GameApp::~C_GameApp()
{
	SAFE_DELETE_PTR(m_pGame);
}

struct UIXtCallbackContext : public XtCallbackContext
{
	inline UIXtCallbackContext() : XtCallbackContext( 'GAME' )	{  }
};

Debug::Logger &g_GameAppLog = Debug::GetLogger("Amax.GameApp");

void C_GameApp::CreateGlobalXtDatabase()
{
	if (!XtInitialise())
	{
		LOGFATAL(g_GameAppLog, "XtInitialse() failed - check output for schema errors, a type not registered maybe?");
	}

	XtSchema* pXtSchema = XtGetApplicationSchema();
	mXtCC = new UIXtCallbackContext;

	mGlobalXtDatabase = XtDbCreate( pXtSchema, MAX_GLOBAL_XT_DATABASE_RECORDS, mXtCC );

	XtNetworkStart( mGlobalXtDatabase );
}

void C_GameApp::SetupUserControls( )
{
	assert(m_pGameInputManager);
	m_pGameInputManager->Resize(m_InputUserCount, m_InputButtonCount, m_InputAxisCount);
	
	C_ApplicationBase::SetupUserControls();
	
	for(S32 i=0; i<m_InputUserCount; i++)
	{
		// Call the dirty global function
		Game::SetupAmaxUserControls( Input::GetGameInput(i), m_pKeyboardList[i], m_pMouseList[i], m_pPadList[i] );
	}
}

#ifndef CFG_GOLD
Debug::Int g_debugEmergencySystemMemoryClawBack("Amax.BuildSetup.LargeHeapReduction", 0);
#endif

void C_GameApp::SetupDeviceMemoryParams( )
{
	// Use the defaults
	C_ApplicationBase::SetupDeviceMemoryParams();

	// Override for game 
#if PLATFORM_PS3
	m_DisplayDeviceParams.m_SystemMemorySize	= 1024*1024*52;			// How much main memory the device can use (tweaked so a minimum of 10mb of actual main memory is available to the game on worst event UL5)
	#ifndef CFG_SPEED
	m_DisplayDeviceParams.m_SystemMemorySize	-= 1024*1024*12;		// Difference between available main memory in a Speed vs Release build loading into LA River Shunt & Run in Single Player
	//m_DisplayDeviceParams.m_SystemMemorySize	-= 1024*1024*6;			// Difference between static allocations in a Release and a Speed version (memory tracking)
	#endif
	#ifndef CFG_GOLD
	m_DisplayDeviceParams.m_SystemMemorySize	-= 1024*1024*g_debugEmergencySystemMemoryClawBack;
	#endif
#endif
}

HRESULT C_GameApp::PlatformInitialise(const Framework::C_ApplicationParams& params)
{
	if (Framework::C_Application::PlatformInitialise(params) != S_OK)
		return E_FAIL;

	// debug connection may have already started the networking as part of the mercury startup..
	m_NetworkStartedByMercury = params.m_NetworkStackAlreadyStarted;

#if defined (PLATFORM_PS3)
	
	if (!m_NetworkStartedByMercury)
		cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	cellSysmoduleLoadModule(CELL_SYSMODULE_L10N);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME); // for dlc npdrm access.
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);

	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP);	// for retrieving gamerpictures
	cellSysmoduleLoadModule(CELL_SYSMODULE_PNGDEC);		// for decoding gamerpictures

#if CELL_SDK_VERSION >= 0x250001
	cellSysmoduleLoadModule(CELL_SYSMODULE_VOICE);
#endif

#endif

	bool ok = true;
	NetPlay::C_Core::Instance().Initialise(true, m_NetworkStartedByMercury);
	if (NetPlay::C_Core::Instance().IsInitialised())
	{
		PlatformCallbacks::Register();
		ok = ok && SysProfile::C_Manager::Instance().Initialise(); // no age requirement.
	}
	else
		ok = false;

	return ok ? S_OK : E_FAIL;
}

void C_GameApp::PlatformShutdown()
{
	PlatformCallbacks::UnRegister();

#if defined (PLATFORM_PS3)

#if CELL_SDK_VERSION >= 0x250001
	cellSysmoduleUnloadModule(CELL_SYSMODULE_VOICE);
#endif

	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_GAME);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_L10N);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NETCTL);
	if (!m_NetworkStartedByMercury)
		cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);

	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);

	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_PNGDEC);

#endif

	Framework::C_Application::PlatformShutdown();
}

#if PLATFORM_PC
// [Blur2PC] Retail PC graphical detail hook (Blur.exe 1.2 0x0042e1c0, C_Application vtable +0xe0): hands the
// level to the game world when one exists; a world created later applies it at the end of its Create.
void C_GameApp::ApplyGraphicalDetail(const U32 graphicalDetail)
{
	if(m_pRaceBookLayer && m_pRaceBookLayer->GetAmaxWorldLayer())
		m_pRaceBookLayer->GetAmaxWorldLayer()->GetGameWorld()->SetGraphicalDetail(graphicalDetail);
}
#endif

void C_GameApp::ExitChecks()
{
	//Ensure we've written save data before continuing.
	if( UI::C_SaveSystem::Instance().GetAsyncSaveThread().IsRunning() )
		UI::C_SaveSystem::Instance().GetAsyncSaveThread().Join();
}

HRESULT C_GameApp::OnInitialise()
{
	LOGDEBUG(Framework::g_BootLog, "C_GameApp::OnInitialise() - BEGIN");
	#ifdef COMPILER_VC
		LOGINFO(Framework::g_BootLog, "C_GameApp::OnInitialise() - Main Thread Id %I64u", MT::GetThreadId());
	#else
		LOGINFO(Framework::g_BootLog, "C_GameApp::OnInitialise() - Main Thread Id %llu", MT::GetThreadId());
	#endif

#if PLATFORM_PS3
	U32 cLocalSize					= 1024*1024*4;
	U32 cMainSize					= 1024*1024*8;
	U32 cDefaultCommandBufferSize	= 1024*32;
	Display::RingBuffer()->Initialise(cLocalSize, cMainSize, cDefaultCommandBufferSize);
#else
	Display::RingBuffer()->Initialise(5*1024*1024);
#endif

	FX::Initialise();

	RenderingData::Target::StreamManager::Instance().SetJobQueuePolicy( Game::C_SchedulerConfig::GetInstance()->GetStreamerPolicy() );

#if PLATFORM_PS3
	SpuDamage::C_ContextManager::SetSchedulerPolicy( Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy() );
#endif

	Game::Actors::C_VehicleActor::SetDeferredRaytracePolicy( Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy() );



	//Create resource bag
	m_hSpecialFXResourceBag = C_ResourceBag::Create();

	//Create particle textures
	m_pParticleTextures = new DynamicEffect::Textures;

	//create the global Xt database for the whole game
	CreateGlobalXtDatabase();

	// create occlusion query renderer
	Display::g_pOcclusionQueryRender = new Display::C_OcclusionQueryRender();

#ifndef CFG_GOLD
	m_pDebugLayer = new Framework::C_DebugLayer();

	InsertLayer( m_pDebugLayer );
#endif

	m_pSystemLayer = new Game::C_SystemLayer();
	InsertLayer( m_pSystemLayer );

	// construct audio layer here to init audio system
	m_pGameAudioLayer = new Game::C_GameAudioLayer(&C_Game::GetInstance()->GetSubscriberSystem());
	m_pGameAudioLayer->DisableLayer();

	m_pBootLayer = new Game::C_BootLayer(&C_Game::GetInstance()->GetSubscriberSystem());
	InsertLayer( m_pBootLayer );

	//CreateMainLayers();

	GetGame()->Initialise();

	RacerNet::C_NetServices::Instance().Initialise();

#ifndef CFG_GOLD
	BIZ_ASSERT(m_pGameAutomationMessageController == NULL);
	m_pGameAutomationMessageController = new GameAutomationMessageController(&Game::C_Game::GetInstance()->GetSubscriberSystem());
	
	BIZ_ASSERT(m_pGamePerformanceMessageController == NULL);
	m_pGamePerformanceMessageController = new AmaxPerfLogging::MessageController(&Game::C_Game::GetInstance()->GetSubscriberSystem());
#endif

	Vehicle::C_VehicleModelManager::Instance().RegisterModelTypes();
	SpecialFX::C_AnimatedMeshSpecialFX::RegisterModelType();

	//Register default amax model type callback function
	GameResource::g_ModelTypeManager.RegisterModelType("", new C_AmaxModelType);

	PhotoMode::AppPhotoCapture().Init();

	m_TimeStep	= GetTimer().GetFrameTime();
	m_TimeScale = 1.0f;

	LOGDEBUG(Framework::g_BootLog, "C_GameApp::OnInitialise() - END");

	return S_OK;
}

void C_GameApp::DestroyBootLayer()
{
	if(m_pBootLayer)
	{
		DestroyLayer( m_pBootLayer );
		m_pBootLayer = NULL;
	}
}

void C_GameApp::CreateMainLayers()
{
	LOGDEBUG(Framework::g_BootLog, "C_GameApp::CreateMainLayers() - BEGIN");

	m_pUiLayer = new UI::C_UiLayer();
	m_pUiLayer->DisableLayer();

	m_pRaceBookLayer = new Game::C_RaceBookLayer(&C_Game::GetInstance()->GetSubscriberSystem());
	m_pRaceBookLayer->DisableLayer();

	InsertLayerAsync(m_pUiLayer);
	//This needs to happen before all the other layers as this builds the sorcery database
	InsertLayerAsync(m_pRaceBookLayer);
	InsertLayerAsync(m_pGameAudioLayer);

	LOGDEBUG(Framework::g_BootLog, "C_GameApp::CreateMainLayers() - END");
}

bool C_GameApp::MainLayersReady()
{
	if(!m_pRaceBookLayer)
		return false;

	if(m_pRaceBookLayer->GetLayerState() != Framework::E_LayerState_Running)
	{
		return false;
	}

	if(!m_pUiLayer)
		return false;

	if(m_pUiLayer->GetLayerState() != Framework::E_LayerState_Running)
	{
		return false;
	}

	if(!m_pGameAudioLayer)
		return false;

	if(m_pGameAudioLayer->GetLayerState() != Framework::E_LayerState_Running)
	{
		return false;
	}

	return true;
}

void C_GameApp::BootUiScene()
{
	if(m_pRaceBookLayer->GetUIWorldLayer() != 0)
		return;

	m_pRaceBookLayer->LoadUIScene( true );
}

bool C_GameApp::UiSceneReady()
{
#ifndef PLATFORM_PS3
	//On the PS3 the presence of the installer means this assertion is inappropriate.
	//Actually, this is invalid if we're not in the 'master' launch mode too. So commenting out the assertion
	//BIZ_ASSERT_SPEED( m_pRaceBookLayer->GetUIWorldLayer() );
#endif

	if(	m_pRaceBookLayer &&
		m_pRaceBookLayer->GetUIWorldLayer() &&
		m_pRaceBookLayer->GetUIWorldLayer()->GetLayerState() == Framework::E_LayerState_Running)
		return true;

	return false;
}

void C_GameApp::EnableMainLayers()
{
	BIZ_ASSERT(MainLayersReady());

	m_pRaceBookLayer->EnableLayer();
	if (m_pRaceBookLayer->GetUIWorldLayer())
		m_pRaceBookLayer->GetUIWorldLayer()->EnableLayer();
	m_pUiLayer->EnableLayer();
	m_pGameAudioLayer->EnableLayer();
}

//#define SHOW_PRINT_SHUTDOWN_PROCESS
#ifdef SHOW_PRINT_SHUTDOWN_PROCESS
#define PRINT_SHUTDOWN_PROCESS printf("Shutdown Process %d\n",__LINE__)
#else
#define PRINT_SHUTDOWN_PROCESS 
#endif

class cStopReads : public MT::Thread
{
private:
	Game::C_BootLayer* m_pBootLayer;
	
	void ThreadMain()
	{
		LogicalFs::C_PakFsManager::Instance().StopReads();
		while(1)
		{
			Sleep(10000);
		}
	}
};


void C_GameApp::OnShutdown()
{
	PRINT_SHUTDOWN_PROCESS;
	ExitChecks();
	PRINT_SHUTDOWN_PROCESS;

	if (m_RunState == Framework::E_RunState_QuitForced)
	{
	#if PLATFORM_PS3
	PRINT_SHUTDOWN_PROCESS;
		sys_timer_sleep( 1 );
	PRINT_SHUTDOWN_PROCESS;
		// TODO: Ensure this is TCR compliant.
		C_PS3InstallData::Instance().SetAbort( true );
	PRINT_SHUTDOWN_PROCESS;
		Advertising::ExitZone(true);
	PRINT_SHUTDOWN_PROCESS;
		Advertising::TidyZone();
	PRINT_SHUTDOWN_PROCESS;
		Advertising::Shutdown();
	PRINT_SHUTDOWN_PROCESS;
		// Exit Loading Thread 
	PRINT_SHUTDOWN_PROCESS;
		C_ApplicationBase::GetInstance()->RequestShutdown(true);
	PRINT_SHUTDOWN_PROCESS;

//		if (m_pRaceBookLayer && m_pRaceBookLayer->GetAmaxWorldLayer())
//		{
//			m_pRaceBookLayer->GetAmaxWorldLayer()->ExitLoadingThread();
//		}

	PRINT_SHUTDOWN_PROCESS;

		// need to shuddown network voice comms. forums says exiting while this is running causes issues.
	PRINT_SHUTDOWN_PROCESS;
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	PRINT_SHUTDOWN_PROCESS;
		if (ns.IsInitialised())
		{
			// enter critical section. loading has another thread that updates this stuff also. (See above, loading thread has been removed 1st, EnterLock Dead-Locks)
			//ns.EnterLock();
			Communicator::C_Core::Instance().Shutdown();
			//ns.Exitlock();
		}
	PRINT_SHUTDOWN_PROCESS;
		// Stop Loading 
	PRINT_SHUTDOWN_PROCESS;
		m_QuitApplication = true;
	PRINT_SHUTDOWN_PROCESS;
		__sync();
	PRINT_SHUTDOWN_PROCESS;
	sys_timer_usleep( 1000 * 500 );
		// Kill Any Bink
	PRINT_SHUTDOWN_PROCESS;
		for(U32 i = 0; i < MAX_MOVIE_SLOTS; i ++)
			UI::gMovie[i].Destroy();

	PRINT_SHUTDOWN_PROCESS;
	// Stop Reads Thread
	// Thread to hold mutex for shutdown 
	cStopReads StopReads;
	StopReads.Start();


	if(m_pBootLayer)
	{
		m_pBootLayer->Destroy(true);
	}
	PRINT_SHUTDOWN_PROCESS;

	sys_timer_usleep( 1000 * 250 );

	PRINT_SHUTDOWN_PROCESS;
		Audio::g_bQuitAllTasks = true;
	sys_timer_usleep( 1000 * 250 );
		// until the game can exit cleanly everywhere, e.g. whilst loading a level etc..
	PRINT_SHUTDOWN_PROCESS;
		IoShutdown();
	PRINT_SHUTDOWN_PROCESS;
		// wait for other threads to exit.
	PRINT_SHUTDOWN_PROCESS;
	sys_timer_usleep( 1000 * 500 );
	PRINT_SHUTDOWN_PROCESS;
		//Display::Device()->BlockUntileIdle();

		//Prevent shutdown order issues.
	PRINT_SHUTDOWN_PROCESS;
		Sys::C_FsPerfLogger::Shutdown();
	PRINT_SHUTDOWN_PROCESS;
	sys_process_exit(0);
		_Exit(EXIT_SUCCESS);
	#elif PLATFORM_PC
		//TODO: Properly implement PC quit routine.

		//Very evil exit process.
		TerminateProcess(GetCurrentProcess(), 0);
	#endif
	}

	PhotoMode::AppPhotoCapture().Exit();
	SAFE_DELETE_PTR(m_pParticleTextures)

	DestroyBootLayer();

#ifdef PLATFORM_PC
	// [Blur2PC] T144: retail PC's OnShutdown (0x0042f620) stops the audio output engine, tells the audio
	// threads to quit and waits 750 ms before destroying the layers
	Audio::ShutdownOutputEngine();
	Audio::g_bQuitAllTasks = true;
	Sleep(750);
#endif

	if(m_pSystemLayer)
	{
		DestroyLayer( m_pSystemLayer );
		m_pSystemLayer = NULL;
	}

#ifndef CFG_GOLD
	if(m_pDebugLayer)
	{
		DestroyLayer( m_pDebugLayer );
		m_pDebugLayer = NULL;
	}
#endif

#ifdef _SIMPLE_UI_
	if(m_pSimpleUiLayer)
	{
		DestroyLayer( m_pSimpleUiLayer );
		m_pSimpleUiLayer = NULL;
	}
#endif

	if(m_pUiLayer)
	{
		DestroyLayer( m_pUiLayer );
		m_pUiLayer = NULL;
	}
	
	if(m_pRaceBookLayer)
	{
		DestroyLayer( m_pRaceBookLayer );
		m_pRaceBookLayer = NULL;
	}

	if(m_pGameAudioLayer)
	{
		DestroyLayer(m_pGameAudioLayer);
		m_pGameAudioLayer = NULL;
	}

#ifdef PLATFORM_PS3
	cellSysutilDisableBgmPlayback();
#endif

	SAFE_DELETE_PTR(Display::g_pOcclusionQueryRender);

	RacerNet::C_NetServices::Instance().Shutdown();

	m_hSpecialFXResourceBag = NULL;

	Display::RingBuffer()->Shutdown();

	FX::Shutdown();

	SysProfile::C_Manager::Instance().Shutdown();
	NetPlay::C_Core::Instance().Shutdown();

	AmaxCam::CameraSystem().Exit();

#ifdef PLATFORM_PC
	// [Blur2PC] T142: retail PC's OnShutdown (Blur.exe 1.2, 0x0042f620) ends here with
	// TerminateProcess (0x0042f826) whatever the run state, so the layer manager never tears
	// down the layers. Without it a window close (not a forced quit) reached
	// C_LayerManager::ShutDown, which destroyed the running world and race-logic layers after
	// m_pRaceBookLayer was cleared above (PickupPositionActor::Destroy, C_RaceLogicLayer::Destroy).
	TerminateProcess(GetCurrentProcess(), 0);
#endif
}

void C_GameApp::SetTimeScale(F32 p_TimeScale) 
{ 
	m_TimeScale = p_TimeScale; 
	UI::theFETimer.SetTimeScale(m_TimeScale);
}

#ifdef PLATFORM_PS3

void C_GameApp::SystemMenuOpen()
{
	m_SystemMenuActive = true;
}

void C_GameApp::SystemMenuClose()
{
	m_SystemMenuActive = false;
}

#endif

void C_GameApp::OnUpdate()
{

#ifdef PLATFORM_PS3
	if(m_pBootLayer == NULL)
	{
		if (!C_PS3InstallData::Instance().GetXMBActiveState())
		{
			C_PS3InstallData::Instance().Update();
		}
	}
	if (m_SystemMenuActive) sys_timer_usleep( 1000 * 1 );
#endif

	CommonUpdate();

	m_TimeStep = GetTimer().GetFrameTime() * m_TimeScale;

	if (!IsPaused())
		m_ElapsedRaceTime += m_TimeStep;

	if (m_TimeScale != 1.0f && !IsPaused())
	{
		m_TimeScaleTimeOut -= GetTimeStepRaw();
		if (m_TimeScaleTimeOut <= 0.0f)
		{
			SetTimeScale(1.0f);
			if (m_pRaceBookLayer && m_pRaceBookLayer->GetAmaxWorldLayer())
				m_pRaceBookLayer->GetAmaxWorldLayer()->ExitSloMo();
			m_TimeScaleTimeOut = 1000.0f;
		}
	}

	if (!LoadingScreenEnabledState())
	{
		Advertising::Update(false);
	}

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (C_GameApp::OnUpdate, 0x0042e930, T146): the start screen's quit popup
	// (UISystem.SystemQuitGame) closes the window
	if (m_pUiLayer && m_pUiLayer->GetUiManager() && m_pUiLayer->GetUiManager()->Blur2PCQuitGameRequested())
	{
		Blur2PCCloseWindow();
	}
#endif

#ifndef CFG_GOLD
	if (g_DebugCrashGame)
		*((DWORD*)0) = NULL;

	// Notice
	if (g_DebugNoticeGameUpdate)
	{
		BIZ_NOTICE_MSG(g_DebugNoticeGameUpdate == false, "Test Notice Update");
		//BIZ_NOTICE_FAIL("Test Notice Update");
		g_DebugNoticeGameUpdate = false;
	}

	// Warning
	if (g_DebugWarningGameUpdate)
	{
		BIZ_WARNING_MSG(g_DebugWarningGameUpdate == false, "Test Warning Update");
		g_DebugWarningGameUpdate = false;
	}

	if (g_DebugCoreAssertGameUpdate)
	{
		assert(g_DebugCoreAssertGameUpdate == false);
		g_DebugCoreAssertGameUpdate = false;
	}
	
	if (g_DebugAssertGameUpdateMsg)
	{
		BIZ_ASSERT_MSG(g_DebugAssertGameUpdateMsg == false, "Test Assert Update");
		g_DebugAssertGameUpdateMsg = false;
	}
	
	if (g_DebugAssertGameUpdate)
	{
		BIZ_ASSERT(g_DebugAssertGameUpdate == false);
		g_DebugAssertGameUpdate = false;
	}
	
	if (g_DebugPanicGameUpdateMsg)
	{
		//BIZ_PANIC_MSG(g_DebugPanicGameUpdateMsg == false, "Test Panic Update");
		BIZ_PANIC_FAIL("Test Panic Update");
		g_DebugPanicGameUpdateMsg = false;
	}
#endif
}

void C_GameApp::OnRender()
{
	bool useRenderMutex = true;
	if(m_pSystemLayer)
	{
		if(!m_pSystemLayer->m_UseRenderMutex)
		{
			useRenderMutex = false;
		}
	}

	if(useRenderMutex)
	{
		FX::LockRenderMutex();
	}

	PhotoMode::C_AppPhotoCapture& photoCapture = PhotoMode::AppPhotoCapture();
	photoCapture.SetWidescreen(Display::Device()->GetWideScreen());

	Framework::C_CaptureService& captureService = Framework::g_CaptureService;

	if (captureService.TakePhoto())
		photoCapture.TakePhoto(0, false, false);

	if (captureService.PhotoMovie())
		photoCapture.TakePhoto(0, false, true);

	bool captured = photoCapture.OnAppRender();

	if (!captured)
		RenderNow();

	// Update screenshot capture service:
	captureService.Update();

	if(useRenderMutex)
	{
		FX::ReleaseRenderMutex();
	}
}

void C_GameApp::RenderNow()
{
	#if !CFG_GOLD
	if(GetUpdatePresentRender() != g_UpdatePresentRender)
		SetUpdatePresentRender(g_UpdatePresentRender);
	#endif


	if(BeginRender())
	{
#ifndef CFG_GOLD
		if (g_DebugAssertGameRender)
		{
			BIZ_ASSERT(g_DebugAssertGameRender == false);
			g_DebugAssertGameRender = false;
		}
#endif

		CommonRender();

		EndRender();

		Display::RingBuffer()->EndFrame();

		#if DEBUG_COMMAND_BUFFER
		if(g_DumpUsedRingBufferMemory)
		{
			LOGINFO(g_Log, "RingBufferUsage : Local %8i Main %8i\n", Display::RingBuffer()->GetUsedSize(), Display::RingBuffer()->GetMainUsedSize());

			Display::g_MainDebugCommandBuffer.Dump("MainDebugCommandBuffer");
			Display::g_UserDebugCommandBuffer.Dump("UserDebugCommandBuffer");
		}

		Display::g_MainDebugCommandBuffer.Update();
		Display::g_UserDebugCommandBuffer.Update();
		#endif
	}
}

//#define NULL_LOADING_SCREEN
//#define TEST_LOADING_SCREEN

void C_GameApp::OnLoadingScreenUpdate()
{
#ifdef PLATFORM_PS3
	if ( m_QuitApplication ) return;
#endif

	Time::C_Time loadingScreenTime = Time::GetPerformanceCounter();
	Time::C_Time elapsedTime = loadingScreenTime - m_LoadingScreenPrevTime;
	m_LoadingScreenPrevTime = loadingScreenTime;
	m_LoadingScreenTimeStep = (F32)elapsedTime / (F32)Time::GetPerformanceFrequency();
	if(m_LoadingScreenTimeStep > 4.0f / 30.0f)
	{
		m_LoadingScreenTimeStep = 4.0f / 30.0f;
	}

	if(m_LoadingScreenTimeStep < 1.0f / 120.0f)
	{
		m_LoadingScreenTimeStep = 1.0f / 120.0f;
	}

	// don't update game systems, just core systems
	// we are qualifying with the Base class name to avoid calling the virtual Update.
	Framework::C_CoreSystems::Instance().C_CoreSystems::Update();

	C_GameApp::GetInstance()->UpdateInputManager();

	Advertising::Update(true);

#ifdef NULL_LOADING_SCREEN
	for(S32 i=0; i<100; i++)
	{
		MT::YieldThread();
	}
#endif
	//C_GameApp::GetInstance()->GetDebugLayer()->GlobalUpdate();
	C_GameApp::GetInstance()->GetUiLayer()->GlobalUpdate();

	if (g_updateStreamerOnLoadingScreen)
	{
		Game::C_RaceBookLayer* rb=C_GameApp::GetInstance()->GetRaceBookLayer();
		if (rb)
		{
			MT::Mutex& mutex=rb->GetUpdateStreamerOnLoadingScreenMutex();
			mutex.Wait();
			if (rb->GetAmaxWorldLayer() && rb->GetAmaxWorldLayer()->GetGameWorld() )
			{
				if (FX::TrySerialiseMutex())
				{
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->LevelSceneManagerUpdate();
					FX::ReleaseSerialiseMutex();
				}
			}
			mutex.Release();
		}
	}
}

void C_GameApp::OnLoadingScreenRender()
{
#ifdef NULL_LOADING_SCREEN
	Display::RingBuffer()->EndFrame();	
	return;
#else

	//RenderNow();

	
#ifdef TEST_LOADING_SCREEN
	BeginRender();

	Display::Device()->SetColourDepthStencilSurface();

	static F32 t = 0;
	t++;

	F32 r = (sinf( t * 0.12312f ) * 0.5f + 0.5f) * 0.5f;
	F32 g = (sinf( t * 0.23423f ) * 0.5f + 0.5f) * 0.5f;
	F32 b = (sinf( t * 0.43523f ) * 0.5f + 0.5f) * 0.5f;	

	Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0.5f,0.0f,0.5f,1.0f));

	Render::Font::C_FontRenderer* pFontRenderer = C_GameApp::GetInstance()->GetDebugLayer()->GetFontRenderer();

	for(S32 i=0; i<10; i++)
	{
		Maths::HVec2 pos(512, 384);
		F32 t2 = t - pow(0.5f, (F32)i) * 10.0f;
		pos = pos + Maths::HVec2( cosf(t2 * 0.1f), sinf(t2 * 0.123423f) ) * 256;

		S32 shade = (i * 255 / 10);
		Maths::RGBA colour(shade, shade, shade, 255);

		pFontRenderer->SetColourTop(colour);
		pFontRenderer->SetColourBottom(colour);
		pFontRenderer->Print(pos, "Loading");
	}	
	
	EndRender();
	Display::RingBuffer()->EndFrame();

#else
	PlatformCallbacks::Poll();

	BeginRender();

	bool bBloomEnabled = g_DebugUiBloomUpdateEnabled;
	if(bBloomEnabled)
	{
		C_GameApp::GetInstance()->GetUiLayer()->RenderOnce();
	}

	Framework::C_View& fullscreenView = GetFullscreenView();
	Display::Device()->SetColourDepthStencilSurface();
	Display::Device()->SetupView(&fullscreenView.m_View, fullscreenView.m_ViewSetup);
	Display::Device()->SetView(&fullscreenView.m_View);
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil);

	C_GameApp::GetInstance()->GetUiLayer()->RenderFullscreen();
#ifndef CFG_GOLD
	if(C_GameApp::GetInstance()->GetDebugLayer())
	{
		C_GameApp::GetInstance()->GetDebugLayer()->RenderFullScreenDebug();
	}
#endif

	Display::Device()->SetView(NULL);

	EndRender();
	Display::RingBuffer()->EndFrame();
#endif

#endif
}

Debug::Bool g_CacheResourcesDuringRestart("Test.CacheResourcesDuringRestart", true);
Debug::Bool g_CacheLevelDuringRestart("Test.CacheLevelDuringRestart", false);

void C_GameApp::CacheResources()
{	
	// everything except levels
	Resource::C_HandleList handleList;
	Resource::C_Manager::GetInstance()->GetHandleList(handleList);
	for(S32 i=0; i<handleList.Size(); i++)
	{
		Resource::C_ResourceBase* pResource = handleList[i];

		if(pResource)
		{
			if(pResource->GetResourceTypeId() == GameResource::C_LevelResource::StaticGetResourceTypeId())
			{
				if(g_CacheLevelDuringRestart)
				{
					m_CachedResourceHandleList.PushBack(handleList[i]);
				}
			}
			else
			{
				if(g_CacheResourcesDuringRestart)
				{
					m_CachedResourceHandleList.PushBack(handleList[i]);
				}
			}
		}
	}

	DumpCachedResources();
}

void C_GameApp::ReleaseCachedResources()
{
	if(g_CacheResourcesDuringRestart)
	{
		m_CachedResourceHandleList.Resize(0);
	}
}

void C_GameApp::DumpCachedResources()
{
	for(S32 i=0; i<m_CachedResourceHandleList.Size(); i++)
	{
		Resource::C_HandleBase hResource = m_CachedResourceHandleList[i];

		Resource::C_ResourceBase* pResource = (Resource::C_ResourceBase*)hResource;
		if(pResource)
		{
			LOGDEBUG(g_Log, "Cached %s", pResource->GetResourceFilename().Chars() );
		}
	}
}

void C_GameApp::PollPlatformCallbacks()
{
	PlatformCallbacks::Poll();
}

#ifndef CFG_GOLD
void C_GameApp::AppAudioCapture(Cam2::C_CaptureService* self, bool start)
{
	if (start)
		Audio::CaptureFrames(AudioCallback, 1.f / 30.f);
	else
		Audio::CaptureStop();
}

S32 C_GameApp::AudioCallback(F32 *audio, S32 channels, S32 len)
{
	Framework::g_CaptureService.AudioCallback(audio, channels, len);
	return 0;
}
#endif

bool C_GameApp_IsBootLayerActive()
{
	return C_GameApp::GetInstance()->IsBootLayerActive();
}

XT_APPLICATION
	XT_DEPENDS_ON(ComponentCreation)
	XT_DEPENDS_ON( UI )
	XT_DEPENDS_ON( GameUI )
	XT_DEPENDS_ON( GAMELOGIC )
	XT_DEPENDS_ON( GAMEDATA )
	XT_DEPENDS_ON( GAMEVEHICLEDATA )
	XT_DEPENDS_ON( GAMEEVENTDATA )
	XT_DEPENDS_ON( GAMEEFFECTDATA )
	XT_DEPENDS_ON( RACEBOOKDATA )
	XT_DEPENDS_ON( MULTIPLAYERDATA )
	XT_DEPENDS_ON( ACHIEVEMENTDATA )
	XT_DEPENDS_ON( AIR )
	XT_DEPENDS_ON( RLDATABASE )
	XT_DEPENDS_ON( AudioXtLib )
	XT_DEPENDS_ON( CrowdXtLib )
	XT_DEPENDS_ON( AudioActorMessages )
	XT_DEPENDS_ON(SinglePlayer)

	XT_DEPENDS_ON(RacingPhysics_Surfaces)
	XT_DEPENDS_ON(RacingPhysics_DynamicsParameters)
	XT_DEPENDS_ON(RacingPhysics_MotorbikeDynamics)
	XT_DEPENDS_ON(RacingPhysics_CarDynamics)

	// dynamic effect
	XT_DEPENDS_ON(GraphicsDynamic)
	XT_DEPENDS_ON(GraphicsDynamicLight)
	XT_DEPENDS_ON(GraphicsDynamicParticle)
	XT_DEPENDS_ON(GraphicsDynamicTracer)

	XT_DEPENDS_ON(Vehicle)
	
	XT_DEPENDS_ON(AnimationTree)

	XT_DEPENDS_ON(EnvironmentSettings)
	XT_DEPENDS_ON(AmaxActorStudioActors)
	
	XT_DEPENDS_ON(AmaxResourceTypes)
	XT_DEPENDS_ON(ResourceUtils)

	XT_DEPENDS_ON(BasePhysicalObjects)

	XT_DEPENDS_ON(SpecialFX)
	XT_DEPENDS_ON(AmaxSpecialFX)
	XT_DEPENDS_ON(AmaxCameraSpecialFX)
	XT_DEPENDS_ON(AmaxCameraDollyZoomSpecialFX)
	XT_DEPENDS_ON(AmaxCameraNoiseSpecialFX)
	XT_DEPENDS_ON(AmaxCam)

	XT_DEPENDS_ON(ConstraintActorCreate)
	XT_DEPENDS_ON(VideoConfig)
XT_END_APPLICATION
