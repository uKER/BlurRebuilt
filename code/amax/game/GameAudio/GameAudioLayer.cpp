#include "stdafx.h"
#include <Xt/XtGame.h>
#include <GameAudio/GameAudioLayer.h>
#include "DefaultSubsystem.h"
#include "MusicSubsystem.h"
#include "VehicleSubsystem.h"
#include "UISubsystem.h"
#include "DialogueSubsystem.h"
#include "DynamicObjSubsystem.h"
#include "SystemSubsystem.h"
#include "WheelSubsystem.h"
#include "ListenerSubsystem.h"
#include "PickupSubsystem.h"
#include "CollisionSubsystem.h"
#include <Network/CommunicatorManager.h>
#include <SchedulerConfig.h>
#include <GameApp.h>
#include <Game/Messaging/GameflowMessages.h>
#include "Framework/Framework.h"


#ifdef PLATFORM_PS3
#include <sysutil/sysutil_bgmplayback.h>
#endif

#include <System/Memory.h>

#include "ui/Game/Gadgets/ButtonMessages.h"
#include "UI/Game/UiLocaliser.h"
#include "Ui\SoundEffects.h"
#include "Ui\UiLayer.h"

#include <Game/Game.h>

#include "XtAudio.h"

//#include "common\3dmaths.h"

namespace Game
{
	extern Debug::Bool g_bootResourceManagerPause;
}

#define USE_NEW_UI_GAMEFLOW

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

#if defined GAME_AUDIO_BUILD
#define HALT_ON_ERRORS true
#else
#define HALT_ON_ERRORS false
#endif

Debug::Logger &g_Log = Debug::GetLogger("audio.gamelayer");

Debug::Bool g_bDisableAudioPersist("Audio.Disable_Persist", false, Debug::Persist());
#ifdef PLATFORM_PS3
Debug::Bool g_bDisableAudio("Audio.Disable_NonPersist", false);
#else
Debug::Bool g_bDisableAudio("Audio.Disable_NonPersist", false);
#endif
Debug::Bool g_bAudioHaltOnErrors("Audio.System.HaltOnErrors", HALT_ON_ERRORS);

Debug::Bool g_bDrawPerfbarsAndStats("Audio.System.Performance.RenderPerformanceBarsAndStats", false);
Debug::Bool g_bUseAveragePerfbars("Audio.System.Performance.UseAverageCpuTimeForPerfBar", true);
Debug::Bool g_bAudioResetPeakMarker("Audio.System.Performance.ResetCpuBarPeakMarker", false);
Debug::Bool g_bAudioHoldPeakMarker("Audio.System.Performance.HoldCpuBarPeakMarker", false);
Debug::Bool g_bAudioPrintPeakCPUTime("Audio.System.Performance.PrintPeakCpuTime", false);
Debug::Bool g_bAudioResetPeakCPUTime("Audio.System.Performance.ResetPeakCpuTime", false);
Debug::Bool g_bAudioShowNamedPerfBar("Audio.System.Performance.ShowNamedPerfBar", false);
Debug::Bool g_bAudioShowPerfBarKey("Audio.System.Performance.ShowPerfBarKey", false);
Debug::Int g_bAudioPerfBarKeyLeft("Audio.System.Performance.PerfBarKeyLeft", 100, Debug::Range(0, 1000));
Debug::Int g_bAudioPerfBarKeyTop("Audio.System.Performance.PerfBarKeyTop", 100, Debug::Range(-800, 800));
Debug::Int g_bAudioPerfBarKeyStep("Audio.System.Performance.PerfBarKeyStep", 20, Debug::Range(0, 100));
Debug::Bool gbRenderWaveBankDebug("Audio.System.WaveBanks.DebugRender", false);
Debug::Bool gbDebugDumpWaveBanks("Audio.System.Wavebanks.DumpInfo", false);

Debug::Bool gbDebugPrintGameState("Audio.System.LogGamestate", false, Debug::Persist());

Debug::Bool gbAudioCacheCommonResources("Audio.CacheCommonResources", true, Debug::Persist());

Debug::Float g_debugMovieVolumeScale("Audio.Bink.VolumeScale", 0.7f, Debug::Range(0.0f, 2.f));
Debug::Int g_PS3DBVolumeScale("Audio.Bink.Ps3dbVolumeScale", -2, Debug::Range(-60, 60));

C_AudioEntityTypeList gAudioDynObjLevelEntities;

void AudioPrintString(F32 fX, F32 fY, U32 u32RGBA, F32 f32Size, char* strText);
void AudioDrawBox(F32 fXPos, F32 fYPos, F32 fXSize, F32 fYSize, U32 u32RGBA);

extern bool g_bNamedCountersEnabled;
extern Debug::Bool g_debugMovieDisableAudio;

const int MESSAGE_QUEUE_SIZE = 512 * 1024;

const F32 fLog10          = 2.302585093f;

namespace Audio { void SetDebugLevels( const ::DebugLevel::Level core, const ::DebugLevel::Level sys ); }

#ifndef CFG_GOLD
static Render::Font::C_FontRenderer*	g_pAudioFontRenderer = 0;

void PrintAtDebugCallback(F32 f32X, F32 f32Y, U32 u32RGBA, F32 f32Size, char* strText)
{
	if (g_pAudioFontRenderer)
	{
		g_pAudioFontRenderer->SetColourTop(Maths::RGBA(u32RGBA));
		g_pAudioFontRenderer->SetColourBottom(Maths::RGBA(u32RGBA));
		g_pAudioFontRenderer->SetScaleX(f32Size);
		g_pAudioFontRenderer->SetScaleY(f32Size);

		g_pAudioFontRenderer->Print(Maths::HVec2(f32X, f32Y), strText);
	}
}


void DrawBoxDebugCallback(F32 f32XPos, F32 f32YPos, F32 f32XSize, F32 f32YSize, U32 u32RGBA)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Immediate();

	if (pDebugRenderer)
		pDebugRenderer->Draw2dBar(Maths::HVec2(f32XPos, f32YPos), Maths::HVec2(f32XSize, f32YSize), Maths::RGBA(u32RGBA));
}
#endif

#ifndef CFG_GOLD
const char* pchAudioHackLanguages[ UI::E_UILanguage_Count ] =
{
	"en",	// E_UILanguage_Other
	"en",	// E_UILanguage_English
	"fr",	// E_UILanguage_French
	"de",	// E_UILanguage_German
	"it",	// E_UILanguage_Italian
	"es",	// E_UILanguage_Spanish
	"hu",	// E_UILanguage_Hungarian
	"ru",	// E_UILanguage_Russian
	"pl",	// E_UILanguage_Polish
	"cs",	// E_UILanguage_Czech
	"sc",	// E_UILanguage_SimplifiedChinese
	"tc",	// E_UILanguage_TraditionalChinese
	"ja",	// E_UILanguage_Japanese
	"ko",	// E_UILanguage_Korean
};

Debug::Bool gbAudioHackCurrentLanguage("Audio.HackCurrentLanguage", false);
Debug::Enum<UI::E_UILanguage>	gAudioCurrentHackedLanguage("Audio.HackCurrentLanguage", UI::E_UILanguage_English);
#endif

namespace Game
{

Sys::C_FilePath C_GameAudioLayer::m_strPath;

Btl::String	C_GameAudioLayer::WbMapFilenameFunc(const Btl::String& strIn)
{
	Btl::String searchTok("_[LNG]");

	if (strIn.ISearch(searchTok) != -1)
	{
		Btl::String strRet = strIn;
		
		Btl::String repToken("_");
		#ifndef CFG_GOLD
		if (gbAudioHackCurrentLanguage)
			repToken += pchAudioHackLanguages[gAudioCurrentHackedLanguage];
		else
		#endif
			repToken += Btl::String(UI::C_UILocaliser::Instance().GetLanguageCode());

		strRet = strRet.IReplace(searchTok, repToken);
		return strRet; 
	}

	return Btl::String("");
}

F32 C_GameAudioLayer::FormatBinkMovieVolume(S32 movieVol)
{
		//cian- set the volumes for bink movies
		//let sliders affect volume

	//	F32 newVolScale = (F32)m_profileSFXVolume / 9.f; // bone's volume slider max, get into 0-1 range

		//newVolScale *= newVolScale; // square it to get a curve

		S32 newMovieVolume = S32((F32)movieVol/* * newVolScale*/ * g_debugMovieVolumeScale); 

#ifdef PLATFORM_PS3
		F32 normalisedVolume = (F32)newMovieVolume/65536;
		F32 DbScaledFloat = 20.f * log10f( normalisedVolume );  
		DbScaledFloat += g_PS3DBVolumeScale;
		F32 temp = fLog10 * DbScaledFloat * 0.05f;
		temp = 	expf( temp );

		newMovieVolume = temp * 65536;
#endif

		newMovieVolume = Maths::Clamp(newMovieVolume,0,65535);

		//printf("\nMovie Vol %d",newMovieVolume);
	//	LOGINFO(Game::g_Log, "\nMovie Vol %d",newMovieVolume);

		return newMovieVolume;
}

// C_GameAudioLayer
C_GameAudioLayer::C_GameAudioLayer(Messaging::C_MessageSubscriberSystem* pSubscriberSystem) : Framework::C_Layer("C_GameAudioLayer")
{
	m_pSystem = NULL;
	m_pWaveManager = NULL;

	for(int i = 0;i < TotalNumSubsystems;i++)
		m_hActorEntities[i] = 0;

	for(int i = 0;i < MAX_ENTITIES;i++)
		pEntDb[i] = new AudioEntityDebug;

	SetSubscriberSystem( pSubscriberSystem );
	m_pSubscriberSystem = pSubscriberSystem;
	
	AddSubscriber( E_GameflowMessage_Pause );
	AddSubscriber( E_GameflowMessage_UnPause );

	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );
	AddSubscriber( E_GameflowMessage_VehicleFinished );
	AddSubscriber( E_GameflowMessage_LevelDumped );

	AddSubscriber( E_GameflowMessage_VideoFinished );
	AddSubscriber( E_GameflowMessage_VideoPresent );
	AddSubscriber( E_GameflowMessage_VideoStarted );

	AddSubscriber( E_GameflowMessage_CutsceneVideoFinished );
	AddSubscriber( E_GameflowMessage_CutsceneVideoStarted );

	AddSubscriber( E_GameflowMessage_LoadFrontendAudio );
	AddSubscriber( E_GameflowMessage_StartFrontendAudio );
	AddSubscriber( E_GameflowMessage_StopFrontendAudio );
	AddSubscriber( E_GameflowMessage_StartInGameAudio );
	AddSubscriber( E_GameflowMessage_StopInGameAudio );

	AddSubscriber( E_GameflowMessage_StartRaceLoad );
	AddSubscriber( E_GameflowMessage_RestartLevel );	


	//AddSubscriber( E_GameflowMessage_DumpLevel );
	//AddSubscriber( E_GameflowMessage_OutroStarted );
	//AddSubscriber( E_GameflowMessage_OutroFinished );
	//AddSubscriber( E_GameflowMessage_SaveGameLoaded );
	//AddSubscriber( E_GameflowMessage_ResultsCalculated );
	//AddSubscriber( E_GameflowMessage_EnteredDebugMenu );

#ifndef USE_NEW_UI_GAMEFLOW
	AddSubscriber( E_GameflowMessage_StartGameRendering );
	AddSubscriber( E_GameflowMessage_StopGameRendering );
	AddSubscriber( E_GameflowMessage_StartRaceLoad );
	AddSubscriber( E_GameflowMessage_StartUILoad );
	AddSubscriber( E_GameflowMessage_RaceLoaded );
	AddSubscriber( E_GameflowMessage_QuitRace );
	AddSubscriber( E_GameflowMessage_RestartLevel );	
	AddSubscriber( E_GameflowMessage_RestartRace );
	AddSubscriber( E_GameflowMessage_QuitLevel );
	AddSubscriber( E_GameflowMessage_QuitAndWinRace );

	AddSubscriber( UI::E_UiMsg_PlaySoundEffect );
#endif
	
	m_videoPresent = false;
	m_bInRace = false;
	m_bFineToCreateEntityInstances = true;
	m_profileSFXVolume = 7;
	m_AudioLevelTransitionState = -1;

	InitSystem();
}


C_GameAudioLayer::~C_GameAudioLayer()
{
#ifndef CFG_SPEED
	if (g_pAudioFontRenderer)
		delete g_pAudioFontRenderer;
#endif
}


void C_GameAudioLayer::GetDatPathFn(const char* pchBuf, U32 uBufLen) 
{
	strncpy_s((char*)pchBuf, uBufLen, (m_strPath+"/dat/").Chars(), uBufLen); 
}

HRESULT C_GameAudioLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	if (g_bDisableAudio || g_bDisableAudioPersist
#ifndef CFG_GOLD
		|| g_debugSuperSpecialCutSceneVideoMode
#endif
		)
	{
		g_debugMovieDisableAudio = true;
		return S_OK;
	}

	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;

		case Framework::E_LayerMessage_Update:
			return Update();
		break;

		case Framework::E_LayerMessage_GlobalUpdate:
			return GlobalUpdate();
		break;

		case Framework::E_LayerMessage_RenderSplitscreenWithAA:
		case Framework::E_LayerMessage_RenderSplitscreenWithoutAA:
			return RenderSplitscreen();
		break;

		case Framework::E_LayerMessage_RenderFullscreen:
			return RenderFullscreen();
		break;
	}

	return S_OK;
}

void C_GameAudioLayer::InitSystem()
{
	//cian temp disable
	if(g_bDisableAudio || g_bDisableAudioPersist
#ifndef CFG_GOLD
		|| g_debugSuperSpecialCutSceneVideoMode
#endif		
		)
		return;

#ifdef PLATFORM_PC
	//
	//	// initialise voice communications for amax.
	//	// max 2 local talkers, 19 remote talkers.
	//	Communicator::C_Core::Instance().Initialise(2, 19);
	//
	//return S_OK;
#endif

	MEMORY_TRACKER_PUSH("Audio");
	//
#ifdef GAME_AUDIO_BUILD
	Audio::SetDebugLevels(DebugLevel::Info, DebugLevel::Info);
#else
	// sam. set debug level for everyone to WARN.
	// we need to know about warnings at this stage in the project.
	//Audio::SetDebugLevels(DebugLevel::Error, DebugLevel::Error); //set everyone
	Audio::SetDebugLevels(DebugLevel::Warn, DebugLevel::Warn);
#endif



#ifndef CFG_GOLD
	if (g_bAudioHaltOnErrors)
	{
		m_pErrorHandler = new Audio::C_ErrorHandler;
		if (m_pErrorHandler)
		{
			Audio::Core::g_Log.AddSubscriber(m_pErrorHandler);
			Audio::g_Log.AddSubscriber(m_pErrorHandler);
			//InterfaceLayerLogger::g_Logger.AddSubscriber(m_pErrorHandler);
		}
	}
#endif // #ifndef CFG_GOLD

	SetupPath();

	MEMORY_TRACKER_PUSH("Audio.CreateSystem");

	//  create audio system
	if (m_pSystem == NULL)
	{
		Audio::ISystem::InitParams params;

		memset(&params, 0, sizeof(Audio::ISystem::InitParams));

		Scheduler::GroupHandle hGroup = Scheduler::CreateGroup(true);

		params.uMessageQueueNumBytes = MESSAGE_QUEUE_SIZE;
		params.uReturnMessageQueueNumBytes = 128 * 1024;
		params.uNumReverbs = 0;
		params.uTempFrameBufferNumBytes = 64 * 256 * sizeof(F32);
		params.pfnGetDatPath = C_GameAudioLayer::GetDatPathFn;
		params.hPolicy = C_SchedulerConfig::GetInstance()->GetAudioPolicy();
		params.hPolicyB = C_SchedulerConfig::GetInstance()->GetAudioPolicy();
		params.uInstanceStateDataBufferSize = 8 * 1024;
		params.hGroup = hGroup;
		//		params.eDistributedUpdate = Audio::ISystem::eDistribUpdate_Disabled;

#if defined (PLATFORM_PC)
		params.hWnd = GetForegroundWindow();
#endif
		m_pSystem = Audio::CreateSystem(&params);
	}

	m_pSystem->Init();

	MEMORY_TRACKER_POP("Audio.CreateSystem");

	MEMORY_TRACKER_POP("Audio");
}

HRESULT C_GameAudioLayer::Create()
{
	LOGDEBUG(Framework::g_BootLog, "C_GameAudioLayer::Create - BEGIN");

	//cian temp disable
	if(g_bDisableAudio || g_bDisableAudioPersist
#ifndef CFG_GOLD
		|| g_debugSuperSpecialCutSceneVideoMode
#endif		
		)
		return S_OK;

	MEMORY_TRACKER_PUSH("Audio");

#ifndef CFG_GOLD
	if (g_pAudioFontRenderer == NULL)
	{
		//Create the font renderer
		g_pAudioFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");

		if (g_pAudioFontRenderer)
		{
			g_pAudioFontRenderer->SetShadowEnable( true );
			g_pAudioFontRenderer->SetColourOutline(Maths::RGBA(0,0,25,255));
			g_pAudioFontRenderer->SetOutlineSize(0.35f);

			//Set Font Size
			g_pAudioFontRenderer->SetScaleX(24);
			g_pAudioFontRenderer->SetScaleY(24);

			m_pSystem->SetPrintAtCallback(PrintAtDebugCallback);
			m_pSystem->SetDrawBoxCallback(DrawBoxDebugCallback);
		}
	}
#endif
	
	//default
	//m_pSystem->SetSpeakerSettings(2,5,5, 240.f / 180.f, 120.f / 180.f, 330.f / 180.f, 30.f / 180.f);
	m_pSystem->SetSpeakerDistance( Audio::ISystem::eSurround, 5.f );

	MEMORY_TRACKER_PUSH("Audio.WaveManager");

	//	create wavebank manager
	if (m_pWaveManager == NULL)
	{
		m_pWaveManager = Audio::CreateWaveManager();
		m_pWaveManager->SetBankFilePath(m_strPath);
		m_pWaveManager->SetMapFilenameFunc(WbMapFilenameFunc);
	}

	MEMORY_TRACKER_POP("Audio.WaveManager");


	MEMORY_TRACKER_PUSH("Audio.InterfaceController");
	Audio::InterfaceController::Get().Init(TotalNumSubsystems, m_pSystem, m_pWaveManager);
	MEMORY_TRACKER_POP("Audio.InterfaceController");
		
	//#if defined USER_SHALL || defined USER_NBYGRAVE || defined USER_COFLANAGAN
	Audio::C_XtAudioDb::Create();

	#ifdef AUDIO_DEFERRED_LOADING
	Audio::C_DeferredLoadManager::Get().SetWaveManager(m_pWaveManager);
	#endif

	if (g_bootResourceManagerPause) Resource::C_Manager::GetInstance()->PauseLoadingThread();

	//#if defined (PLATFORM_XENON)
	Util::C_String audioBin = "gamedata\\xt\\Audio.bin";
	Util::C_String audioPack= "gamedata\\xt\\AudioEntities.bin";

	Audio::C_XtAudioDb::Get()->EntitiesPack().LoadDefs(audioPack,C_GameApp::GetInstance()->GetGlobalXtDatabase());
	Audio::C_XtAudioDb::Get()->AudioPack().LoadDefs(audioBin,C_GameApp::GetInstance()->GetGlobalXtDatabase());
	//#endif

	Audio::C_XtAudioDb::Get()->CreateEntityDefClasses();
	
	#ifdef AUDIO_DEFERRED_LOADING
	Audio::C_XtAudioDb::Get()->SetupEntityResourceLists("audio\\");
	#endif

	Audio::C_XtAudioDb::Get()->EntitiesPack().Flush(C_GameApp::GetInstance()->GetGlobalXtDatabase());

	

	MEMORY_TRACKER_PUSH("Audio.InterfaceController");
	
	Audio::InterfaceController::Get().AddSubsystem(new DefaultSubsystem, eDefaultSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new MusicSubsystem, eMusicSubsystem);
	//Audio::InterfaceController::Get().AddSubsystem(new EffectSubsystem, eEffectSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new VehicleSubsystem, eVehicleSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new UISubsystem, eUISubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new DialogueSubsystem, eDialogueSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new DynamicObjSubsystem, eDynamicObjSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new SystemSubsystem, eSystemSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new WheelSubsystem, eWheelSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new ListenerSubsystem, eListenerSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new PickUpSubsystem, ePickupSubsystem);
	Audio::InterfaceController::Get().AddSubsystem(new CollisionSubsystem, eCollisionSubsystem);

	MEMORY_TRACKER_POP("Audio.InterfaceController");

	{
		#ifdef AUDIO_CACHE_COMMON_RESOURCES
		CacheCommonResources();
		#endif

//		UISubsystem* pSubSystem = (UISubsystem*)Audio::InterfaceController::Get().GetSubsystem( Game::C_GameAudioLayer::eUISubsystem );
//		if (pSubSystem)
//			pSubSystem->StartLoadUIEntity("FrontEndUI");
	}

	// create system entity
	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eInitSystem);

	//this will create all the subsystemEntities
	Audio::InterfaceController::Get().BroadcastMessage(Audio::eInit, 0xffffffff);

	if (g_bootResourceManagerPause) Resource::C_Manager::GetInstance()->ResumeLoadingThread();

	//m_UIActor.Create(m_pSubscriberSystem);
	//m_DialogueActor.Create(&Game::C_Game::GetInstance()->GetSubscriberSystem());
	
	// initialise voice communications for amax. must be done after audio hardware is initialised.
	// max 2 local talkers, 19 remote talkers.
	Communicator::C_Core::Instance().Initialise(2, 19);

	m_peakXmaDecoderCount = 0;

	MEMORY_TRACKER_POP("Audio");

	LOGDEBUG(Framework::g_BootLog, "C_GameAudioLayer::Create - END");

	return S_OK;
}

#ifdef AUDIO_DEFERRED_LOADING
void C_GameAudioLayer::ResourcePreProcessVehicle(S32 p_Slot)
{
	if (!Audio::gbUseDeferredLoading)
		return;

	S32 size = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars.Size();

	if (p_Slot < 0 || p_Slot >= size)
		return;

	GAMEDATA_ID xtId = (GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mCarID;
	XtHandle<AmaxVehicle> hVeh = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), xtId));
	AmaxVehicle* pVehicle = hVeh.Object();

	if (!pVehicle)
	{
		LOGWARN(g_Log, DBSTR" Could not lookup vehicle %d", p_Slot);
		return;
	}

	// player car
	if(p_Slot == 0 && C_Game::GetInstance()->GetLevelState().mNumViewports == 1)
	{
		if (pVehicle->vehicleAudioDef)
		{
			if(pVehicle->dryWheelAudioDef)
				pVehicle->dryWheelAudioDef->SetupEntityType();

			pVehicle->vehicleAudioDef->SetupEntityType();
		}
	}
	else
	{
		#ifndef PLATFORM_PS3
		if (pVehicle->aiVehicleAudioDef)
		#else
		if (pVehicle->aiVehicleAudioDef_PS3)
		#endif
		{
			if(pVehicle->aiDryWheelAudioDef)
				pVehicle->aiDryWheelAudioDef->SetupEntityType();

			#ifndef PLATFORM_PS3
			pVehicle->aiVehicleAudioDef->SetupEntityType(); 
			#else

			//we load a special Bosscar for PS3 boss events
			if(Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_BossBattle && pVehicle->aiBossBattleVehicleAudioDef_PS3)
				pVehicle->aiBossBattleVehicleAudioDef_PS3->SetupEntityType(); 
			else
				pVehicle->aiVehicleAudioDef_PS3->SetupEntityType(); 
			#endif
		}
	}
}

// make a list of which audio resources we need to load
void C_GameAudioLayer::VehicleResourcePrePass()
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	if (levelState.mInUI)
		return;

	int noVehicles = levelState.mNumCars+levelState.mNumAdditionalCars;
	for(int vehicleIndex = 0; vehicleIndex < noVehicles; vehicleIndex++)
	{
		ResourcePreProcessVehicle(vehicleIndex);
	}	
}
#endif // AUDIO_DEFERRED_LOADING

#if defined AUDIO_CACHE_COMMON_RESOURCES
Debug::Bool gbCacheAllDynamicObjects("Audio.CacheDynamicObjects", true, Debug::Persist());

void C_GameAudioLayer::CacheCommonResources()
{
	if (!gbAudioCacheCommonResources)
		return;

	Time::C_Time t = Time::GetPerformanceCounter();

	{
		Btl::Vector<Btl::String> bankList;

		// vehicles common
		bankList.PushBack(Btl::String("Car_Collisions.baf"));
		bankList.PushBack(Btl::String("Car_Common.baf"));
		bankList.PushBack(Btl::String("Wheel_common.baf"));
		bankList.PushBack(Btl::String("Wheel_dry.baf"));
		bankList.PushBack(Btl::String("Wheel_offroad.baf"));

		bankList.PushBack(Btl::String("do_concrete_large.baf"));
		bankList.PushBack(Btl::String("do_vegetation_hedge.baf"));
		bankList.PushBack(Btl::String("do_wood_fence.baf"));
		bankList.PushBack(Btl::String("do_rubber_tyre.baf"));
		bankList.PushBack(Btl::String("do_concrete_fence_chain.baf"));
		bankList.PushBack(Btl::String("do_concrete_fence_plastic.baf"));
		bankList.PushBack(Btl::String("do_metal_hollow_large.baf"));
		bankList.PushBack(Btl::String("do_metal_armco.baf"));
		bankList.PushBack(Btl::String("do_metal_sheet.baf"));
		bankList.PushBack(Btl::String("do_metal_fence_chainlink.baf"));
		bankList.PushBack(Btl::String("do_metal_solid_medium.baf"));
		bankList.PushBack(Btl::String("do_metal_railing.baf"));
		bankList.PushBack(Btl::String("do_metal_post_tall.baf"));

		if (gbCacheAllDynamicObjects)
		{
			// dynamic objects
			bankList.PushBack(Btl::String("do_brk_metal.baf"));
			bankList.PushBack(Btl::String("do_brk_metal_building.baf"));
			bankList.PushBack(Btl::String("do_brk_metal_structure.baf"));
			bankList.PushBack(Btl::String("do_brk_paper_debris.baf"));
			bankList.PushBack(Btl::String("do_brk_plastic.baf"));
			bankList.PushBack(Btl::String("do_brk_plastic_water_barrel.baf"));
			bankList.PushBack(Btl::String("do_brk_rubbish.baf"));
			bankList.PushBack(Btl::String("do_brk_water_spray.baf"));
			bankList.PushBack(Btl::String("do_brk_wood.baf"));
			bankList.PushBack(Btl::String("do_brk_wood_door.baf"));
			bankList.PushBack(Btl::String("do_cardboard_box.baf"));
			bankList.PushBack(Btl::String("do_glass_hit.baf"));
			bankList.PushBack(Btl::String("do_glass_shatter.baf"));
			bankList.PushBack(Btl::String("do_metal_alloyrim.baf"));
			bankList.PushBack(Btl::String("do_metal_hollow_small.baf"));
			bankList.PushBack(Btl::String("do_metal_pipe_thin.baf"));
			bankList.PushBack(Btl::String("do_metal_post_medium.baf"));
			bankList.PushBack(Btl::String("do_metal_post_short.baf"));
			bankList.PushBack(Btl::String("do_metal_scaffold.baf"));
			bankList.PushBack(Btl::String("do_metal_sheet_large.baf"));
			bankList.PushBack(Btl::String("do_metal_sign_chevron.baf"));
			bankList.PushBack(Btl::String("do_metal_solid_large.baf"));
			bankList.PushBack(Btl::String("do_metal_solid_small.baf"));
			bankList.PushBack(Btl::String("do_metal_trolley.baf"));
			bankList.PushBack(Btl::String("do_metal_vehicle_bus.baf"));
			bankList.PushBack(Btl::String("do_metal_vehicle_car.baf"));
			bankList.PushBack(Btl::String("do_metal_vehicle_motorbike.baf"));
			bankList.PushBack(Btl::String("do_plastic_binbag.baf"));
			bankList.PushBack(Btl::String("do_plastic_cone.baf"));
			bankList.PushBack(Btl::String("do_plastic_furniture.baf"));
			bankList.PushBack(Btl::String("do_plastic_post.baf"));
			bankList.PushBack(Btl::String("do_plastic_sign.baf"));
			bankList.PushBack(Btl::String("do_plastic_water_barrel.baf"));
			bankList.PushBack(Btl::String("do_plastic_wheelbin.baf"));
			bankList.PushBack(Btl::String("do_vehicle_alarm.baf"));
			bankList.PushBack(Btl::String("do_wood_door.baf"));
			bankList.PushBack(Btl::String("do_wood_tree.baf"));
		}

		// pickups entity
		bankList.PushBack(Btl::String("Pickups.baf"));

		// listener entity
		bankList.PushBack(Btl::String("Proximity.baf"));

		Audio::C_DeferredLoadManager::Get().AddBafsToQueue(bankList);
	}

	{
		Btl::Vector<Btl::String> datList;

		// system entity
		datList.PushBack(Btl::String("system.dat"));
		//datList.PushBack(Btl::String("vo_upmixer.dat"));// not used anymore
		datList.PushBack(Btl::String("aisubmix.dat"));
		datList.PushBack(Btl::String("finalsubmix.dat"));
		datList.PushBack(Btl::String("reverbcontroller.dat"));
		datList.PushBack(Btl::String("music_upmixer.dat"));
		datList.PushBack(Btl::String("finalperksubmix.dat"));

		// listenener entity
		datList.PushBack(Btl::String("proximity.dat"));
		datList.PushBack(Btl::String("cam_wind.dat"));

		// perks entity
		//datList.PushBack(Btl::String("nitrous.dat"));
		//datList.PushBack(Btl::String("shock.dat"));
		//datList.PushBack(Btl::String("shunt.dat"));
		//datList.PushBack(Btl::String("barge.dat"));
		//datList.PushBack(Btl::String("bomb.dat"));
		datList.PushBack(Btl::String("shockhit.dat"));
		datList.PushBack(Btl::String("shunthit.dat"));
		datList.PushBack(Btl::String("bargehit.dat"));
		datList.PushBack(Btl::String("bombhit.dat"));

		// vehicle entities common

		datList.PushBack(Btl::String("FrontEngine3dMixer.dat"));
		datList.PushBack(Btl::String("LeftExhaust3dMixer.dat"));
		datList.PushBack(Btl::String("MidEngine3dMixer.dat"));
		datList.PushBack(Btl::String("SideExhaust3dMixer.dat"));
		datList.PushBack(Btl::String("TwinExhaust3dMixer.dat"));

		datList.PushBack(Btl::String("AI_Wheel_3dMixer.dat"));
		datList.PushBack(Btl::String("AI_Wheel_Common.dat"));
		datList.PushBack(Btl::String("AI_Wheel_Controller.dat"));
		datList.PushBack(Btl::String("AI_Wheel_Dry.dat"));
		datList.PushBack(Btl::String("AI_Wheel_OffRoad.dat"));
		//datList.PushBack(Btl::String("AICollision3dMixer.dat"));
		//datList.PushBack(Btl::String("Bonnet.dat"));
		//datList.PushBack(Btl::String("Boot.dat"));
		//datList.PushBack(Btl::String("BumperBack.dat"));
		//datList.PushBack(Btl::String("BumperFront.dat"));
		datList.PushBack(Btl::String("Cam_Wind.dat"));
		datList.PushBack(Btl::String("Car_common_race.dat"));
		datList.PushBack(Btl::String("Car_common_road.dat"));
		//datList.PushBack(Btl::String("CarBasedPerks.dat"));
		//datList.PushBack(Btl::String("CarPartController.dat"));
		//datList.PushBack(Btl::String("Collision3dMixer.dat"));
		//datList.PushBack(Btl::String("CollisionController.dat"));
		//datList.PushBack(Btl::String("CollisionGlass.dat"));
		//datList.PushBack(Btl::String("CollisionLFE.dat"));
		//datList.PushBack(Btl::String("CollisionMain.dat"));
		//datList.PushBack(Btl::String("CollisionSurface.dat"));
		datList.PushBack(Btl::String("Common3dMixer.dat"));
		datList.PushBack(Btl::String("DO_concrete_fence_chain.dat"));
		datList.PushBack(Btl::String("DO_concrete_fence_plastic.dat"));
		datList.PushBack(Btl::String("DO_concrete_large.dat"));
		datList.PushBack(Btl::String("DO_metal_armco.dat"));
		datList.PushBack(Btl::String("DO_metal_fence_chainlink.dat"));
		datList.PushBack(Btl::String("DO_metal_hollow_large.dat"));
		datList.PushBack(Btl::String("DO_metal_post_tall.dat"));
		datList.PushBack(Btl::String("DO_metal_railing.dat"));
		datList.PushBack(Btl::String("DO_metal_sheet.dat"));
		datList.PushBack(Btl::String("DO_metal_solid_medium.dat"));
		datList.PushBack(Btl::String("DO_rubber_tyre.dat"));
		datList.PushBack(Btl::String("DO_vegetation_hedge.dat"));
		datList.PushBack(Btl::String("DO_wood_fence.dat"));
		//datList.PushBack(Btl::String("DoorBackLeft.dat"));
		//datList.PushBack(Btl::String("DoorBackRight.dat"));
		//datList.PushBack(Btl::String("DoorLeft.dat"));
		//datList.PushBack(Btl::String("DoorRight.dat"));
		//datList.PushBack(Btl::String("PerkMono3dMixer.dat"));		// nick w said not used any more
		//datList.PushBack(Btl::String("Perks.dat"));				// nick w said not used any more
		//datList.PushBack(Btl::String("Perks3dMixer.dat"));		// nick w said not used any more
		//datList.PushBack(Btl::String("PerkStereo3dMixer.dat"));	// nick w said not used any more
		//datList.PushBack(Btl::String("PerkWeapon3dMixer.dat"));	// nick w said not used any more
		datList.PushBack(Btl::String("Reflection.dat"));
		datList.PushBack(Btl::String("Wheel_3dMixer.dat"));
		datList.PushBack(Btl::String("Wheel_Common.dat"));
		datList.PushBack(Btl::String("Wheel_Controller.dat"));
		datList.PushBack(Btl::String("Wheel_Dry.dat"));
		datList.PushBack(Btl::String("Wheel_OffRoad.dat"));
		//datList.PushBack(Btl::String("WingLeft.dat"));
		//datList.PushBack(Btl::String("WingMirrorLeft.dat"));
		//datList.PushBack(Btl::String("WingMirrorRight.dat"));
		//datList.PushBack(Btl::String("WingRight.dat"));

		if (gbCacheAllDynamicObjects)
		{
			// all other dynamic objects
			datList.PushBack(Btl::String("DO_3dMixer.dat"));
			datList.PushBack(Btl::String("DO_cardboard_box.dat"));
			datList.PushBack(Btl::String("DO_glass_hit.dat"));
			datList.PushBack(Btl::String("DO_glass_shatter.dat"));
			datList.PushBack(Btl::String("DO_metal_alloyrim.dat"));
			datList.PushBack(Btl::String("DO_metal_hollow_small.dat"));
			datList.PushBack(Btl::String("DO_metal_post_medium.dat"));
			datList.PushBack(Btl::String("DO_metal_post_short.dat"));
			datList.PushBack(Btl::String("DO_metal_scaffold.dat"));
			datList.PushBack(Btl::String("DO_metal_sign_chevron.dat"));
			datList.PushBack(Btl::String("DO_metal_solid_large.dat"));
			datList.PushBack(Btl::String("DO_metal_solid_small.dat"));
			datList.PushBack(Btl::String("DO_metal_trolley.dat"));
			datList.PushBack(Btl::String("DO_metal_vehicle_bus.dat"));
			datList.PushBack(Btl::String("DO_metal_vehicle_car.dat"));
			datList.PushBack(Btl::String("DO_metal_vehicle_motorbike.dat"));
			datList.PushBack(Btl::String("DO_plastic_binbag.dat"));
			datList.PushBack(Btl::String("DO_plastic_cone.dat"));
			datList.PushBack(Btl::String("DO_plastic_furniture.dat"));
			datList.PushBack(Btl::String("DO_plastic_post.dat"));
			datList.PushBack(Btl::String("DO_plastic_sign.dat"));
			datList.PushBack(Btl::String("DO_plastic_water_barrel.dat"));
			datList.PushBack(Btl::String("DO_plastic_wheelbin.dat"));
			datList.PushBack(Btl::String("DO_vegetation_hedge.dat"));
			datList.PushBack(Btl::String("DO_vehicle_alarm.dat"));
			datList.PushBack(Btl::String("DO_wood_tree.dat"));

			datList.PushBack(Btl::String("PO_CardboardBox.dat"));
			datList.PushBack(Btl::String("PO_ConcreteLarge.dat"));
			datList.PushBack(Btl::String("PO_GlassLarge.dat"));
			datList.PushBack(Btl::String("PO_MetalAlloyRim.dat"));
			datList.PushBack(Btl::String("PO_MetalFenceChainlink.dat"));
			datList.PushBack(Btl::String("PO_MetalFireHydrant.dat"));
			datList.PushBack(Btl::String("PO_MetalHollowLarge.dat"));
			datList.PushBack(Btl::String("PO_MetalHollowSmall.dat"));
			datList.PushBack(Btl::String("PO_MetalHollowSmallBin.dat"));
			datList.PushBack(Btl::String("PO_MetalHollowSmallNewstand.dat"));
			datList.PushBack(Btl::String("PO_MetalPipesFall.dat"));
			datList.PushBack(Btl::String("PO_MetalPostMedium.dat"));
			datList.PushBack(Btl::String("PO_MetalPostShort.dat"));
			datList.PushBack(Btl::String("PO_MetalPostTall.dat"));
			datList.PushBack(Btl::String("PO_MetalRailing.dat"));
			//datList.PushBack(Btl::String("PO_MetalScaffold.dat"));		// nick w said not used any more
			datList.PushBack(Btl::String("PO_MetalSheet.dat"));
			datList.PushBack(Btl::String("PO_MetalSheetBuildingCollapse.dat"));
			datList.PushBack(Btl::String("PO_MetalSignchevron.dat"));
			datList.PushBack(Btl::String("PO_MetalSolidLarge.dat"));
			datList.PushBack(Btl::String("PO_MetalSolidLargeBusShelter.dat"));
			datList.PushBack(Btl::String("PO_MetalSolidMedium.dat"));
			datList.PushBack(Btl::String("PO_MetalSolidMediumPhonebox.dat"));
			datList.PushBack(Btl::String("PO_MetalSolidMediumPostbox.dat"));
			datList.PushBack(Btl::String("PO_MetalSolidSmall.dat"));
			datList.PushBack(Btl::String("PO_MetalTrolley.dat"));
			datList.PushBack(Btl::String("PO_MetalVehicleBus.dat"));
			datList.PushBack(Btl::String("PO_MetalVehicleCar.dat"));
			datList.PushBack(Btl::String("PO_MetalVehicleMotorbike.dat"));
			//datList.PushBack(Btl::String("PO_PlasticBarrel.dat"));		// nick w said not used any more
			datList.PushBack(Btl::String("PO_PlasticBin.dat"));
			datList.PushBack(Btl::String("PO_PlasticBinbag.dat"));
			datList.PushBack(Btl::String("PO_PlasticCone.dat"));
			datList.PushBack(Btl::String("PO_PlasticFurniture.dat"));
			datList.PushBack(Btl::String("PO_PlasticPost.dat"));
			datList.PushBack(Btl::String("PO_PlasticSign.dat"));
			datList.PushBack(Btl::String("PO_PlasticWaterBarrel.dat"));
			datList.PushBack(Btl::String("PO_PlasticWheelbin.dat"));
			datList.PushBack(Btl::String("PO_RubberTyre.dat"));
			datList.PushBack(Btl::String("PO_VegetationHedge.dat"));
			datList.PushBack(Btl::String("PO_WoodDoorSplinter.dat"));
			datList.PushBack(Btl::String("PO_WoodFence.dat"));
			datList.PushBack(Btl::String("PO_WoodTree.dat"));
		}

		Audio::C_DeferredLoadManager::Get().AddDatsToQueue(datList);
	}					 

	Audio::C_DeferredLoadManager::Get().Load();
	Audio::C_DeferredLoadManager::Get().Clear();

	// increment reference counts for dynamic entity types
	if (gbCacheAllDynamicObjects)
	{
		for (Audio::XtEntityProxyList::Iterator it = Audio::C_XtAudioDb::Get()->XtEntityProxies().Begin(); it != Audio::C_XtAudioDb::Get()->XtEntityProxies().End(); ++it)
		{
			Audio::XtEntityProxy* pEntity = it->Object();
			if (pEntity && pEntity->Group() == Game::C_GameAudioLayer::eDynamicObjSubsystem)
			{
				// entity PO_MetalAlloyRim is not used any more. the baf doesnt exist on the server so dont try and load it
				if (pEntity->id != 0x9be4af2d)
					pEntity->SetupEntityType();
			}
		}
	}

	t = Time::GetPerformanceCounter() - t;

	LOGDEBUG(Framework::g_BootLog, "Loaded common audio resources in %.3f secs", (F32)(t / (F32)Time::GetPerformanceFrequency()));
}
#endif // AUDIO_CACHE_COMMON_RESOURCES

HRESULT C_GameAudioLayer::Destroy()
{
	// release voice communications.
	Communicator::C_Core::Instance().Shutdown();

	if (m_pSystem)
	{
		Audio::ReleaseSystem();
		m_pSystem = NULL;
	}

	if (m_pWaveManager)
	{
		#ifdef AUDIO_DEFERRED_LOADING
		Audio::C_DeferredLoadManager::Get().SetWaveManager(NULL);
		#endif

		ReleaseWaveManager(m_pWaveManager);
		m_pWaveManager = NULL;
	}

	Audio::InterfaceController::Get().Release();

//	m_UIActor.Kill();


	return S_OK;
}

HRESULT C_GameAudioLayer::Update()
{
	PERF_EVENT("C_GameAudioLayer::Update");

	#ifndef CFG_GOLD
	if (gbDebugDumpWaveBanks)
	{
		if (m_pWaveManager)
			m_pWaveManager->LogLoadedBanks();
		gbDebugDumpWaveBanks = false;
	}
	#endif

	Audio::InterfaceController::Get().Update();
	if(m_pSystem)
		m_pSystem->DoWork();

	return S_OK;

}

bool C_GameAudioLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	MEMORY_TRACKER_PUSH("audio");

	PERF_EVENT("C_GameAudioLayer::MessageHandler");

	//printf("msg %s\n", pMessage->GetDebugName());

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
#ifdef USE_NEW_UI_GAMEFLOW
		case E_GameflowMessage_LoadFrontendAudio:
		{
			//Audio::InterfaceController::Get().BroadcastMessage(Audio::ePreLoadUI, 0xffffffff);

			//if(gbDebugPrintGameState)
			//	LOGINFO(Game::g_Log, "\nLOAD FRONTEND AUDIO\n");

			break;
		}

		case E_GameflowMessage_StartFrontendAudio:
		{
			//check if message has been previously sent
			if(m_AudioLevelTransitionState == (S32)E_GameflowMessage_StartFrontendAudio)
				break;

			//set current transition state
			m_AudioLevelTransitionState = (S32)E_GameflowMessage_StartFrontendAudio;


			Audio::InterfaceController::Get().BroadcastMessage(Audio::eLoadUI, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nSTART FRONTEND AUDIO\n");

			break;
		}

		case E_GameflowMessage_StopFrontendAudio:
		{

			//check if message has been previously sent
			if(m_AudioLevelTransitionState == (S32)E_GameflowMessage_StopFrontendAudio)
				break;

			//set current transition state
			m_AudioLevelTransitionState = (S32)E_GameflowMessage_StopFrontendAudio;

			Audio::InterfaceController::Get().BroadcastMessage(Audio::eLoadLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nSTOP FRONTEND AUDIO\n");

			break;
		}

		case E_GameflowMessage_StartInGameAudio:
		{

			//check if message has been previously sent
			if(m_AudioLevelTransitionState == (S32)E_GameflowMessage_StartInGameAudio)
				break;

			//set current transition state
			m_AudioLevelTransitionState = (S32)E_GameflowMessage_StartInGameAudio;

			m_bInRace = true;

			Audio::InterfaceController::Get().BroadcastMessage(Audio::eBeginLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nGAME FLOW START LEVEL AUDIO\n");

			break;
		}

		case E_GameflowMessage_StopInGameAudio:
		{

			//check if message has been previously sent
			if(m_AudioLevelTransitionState == (S32)E_GameflowMessage_StopInGameAudio)
				break;

			//set current transition state
			m_AudioLevelTransitionState = (S32)E_GameflowMessage_StopInGameAudio;

			m_bInRace = false;

			// special flag to fix the problem that we can get this message and actor updates can happen after this point.
			// if this flag is set then entity instance creation will be disabled
			m_bFineToCreateEntityInstances = false;

			Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nGAME FLOW STOP LEVEL AUDIO\n");

			break;
		}

		case E_GameflowMessage_RestartLevel:
		case E_GameflowMessage_StartRaceLoad:
			m_bFineToCreateEntityInstances = true;
			break;
#endif

		//case E_GameflowMessage_VehicleFinished:
		case E_GameflowMessage_RaceFinished:
		{
			//crossed the finished line
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eRaceFinished, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\neGAME FLOW RACE FINSISHED\n");

			break;
		}
		case E_GameflowMessage_LevelDumped:
		{
			//level has finished and has dumped all its data, used as a final death knell for dialogue
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eDumpLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nGAMEFLOW LEVEL DUMPED FOR DIALOGUE: dumpLevel\n");

			break; 
		}

		case E_GameflowMessage_VideoFinished:
			//if(!C_GameApp::GetInstance()->GetUiLayer()->IsFrontend())
			//	Audio::InterfaceController::Get().BroadcastMessage(Audio::eBeginLevel, 0xffffffff);
			//m_videoPresent = false;
			//Audio::InterfaceController::Get().BroadcastMessage(Audio::eUnmute, 0xffffffff);

			//raise music level after video finished
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eRaiseMusicLevel);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nE_GAMEFLOWMESSAGE_VIDEOFINSHED\n");

			break;

		case E_GameflowMessage_VideoPresent:
			//m_videoPresent = true;
			break;
		
		case E_GameflowMessage_VideoStarted:
			//a cutscene is playing or a video is playing in frontend

			//Audio::InterfaceController::Get().BroadcastMessage(Audio::eMute, 0xffffffff);

			//duck the music level
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eDuckMusicLevel);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nE_GAMEFLOWMESSAGE_VIDEOSTARTED\n");

			break;

		case E_GameflowMessage_CutsceneVideoFinished:

			m_videoPresent = false;

			Audio::InterfaceController::Get().BroadcastMessage(Audio::eUnmute, 0xffffffff);

			if(gbDebugPrintGameState)
			LOGINFO(Game::g_Log, "\nE_GAMEFLOWMESSAGE_CUTSCENEVIDEOFINSHED\n");

#ifdef PLATFORM_PS3
			cellSysutilEnableBgmPlayback();
#endif

			//for music on videos
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eRaiseMusicLevel);
			break;

	
		case E_GameflowMessage_CutsceneVideoStarted:
			//a small window is playing in friontend, I think E_GameflowMessage_VideoStarted is getting called instead
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eMute, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nE_GAMEFLOWMESSAGE_CUTSCENEVIDEOSTARTED\n");

#ifdef PLATFORM_PS3
			cellSysutilDisableBgmPlayback();
#endif
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eDuckMusicLevel);
			break;

		case E_GameflowMessage_Pause:
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eMute, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nEMUTE\n");
			break;

		case E_GameflowMessage_UnPause:
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eUnmute, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nUNMUTE\n");
			break;
			
		case E_GameflowMessage_RaceStarted:
			//when crossing the start line, used for music
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eRaceStarted, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\neGAME FLOW RACE STARTED\n");
			break;

#ifndef USE_NEW_UI_GAMEFLOW
		case UI::E_UiMsg_PlaySoundEffect:
			{
				const UI::C_UiSoundEffectMessage* msg = Messaging::MessageCast<UI::C_UiSoundEffectMessage>(pMessage);
				switch(msg->m_EffectNum)
				{
					//start the UI Music, we are entering the frontEnd
					case UI::E_UiSoundEffect_Start:
						haveReceivedLoadUiMsg = true;
						Audio::InterfaceController::Get().BroadcastMessage(Audio::eLoadUI, 0xffffffff);

						if(gbDebugPrintGameState)
							LOGINFO(Game::g_Log, "\nPLAYSOUNDEFFECT LOADUI\n");
					break;

					//leaving the frontEnd
					case UI::E_UiSoundEffect_Stop:		
						if(haveReceivedLoadUiMsg)
							Audio::InterfaceController::Get().BroadcastMessage(Audio::eLoadLevel, 0xffffffff);
						haveReceivedLoadUiMsg = false;

						if(gbDebugPrintGameState)
							LOGINFO(Game::g_Log, "\nPLAYSOUNDEFFECT LOADLEVEL\n");
					break;
				}
			}
			break;

		case E_GameflowMessage_StartGameRendering:
			//this start all level audio bar the music
			if (!C_GameApp::GetInstance()->GetUiLayer()->IsFrontend()/* && !m_videoPresent*/)
			{
				Audio::InterfaceController::Get().BroadcastMessage(Audio::eBeginLevel, 0xffffffff);

				if(gbDebugPrintGameState)
					LOGINFO(Game::g_Log, "\nGAME FLOW START RENDERING BEGIN LEVEL\n");

			}
			break;

		case E_GameflowMessage_StopGameRendering:	
			//release all level audio
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nSTOPGAMERENDERING: eEndLevel\n");
			break;

		case E_GameflowMessage_QuitLevel:	
			//release all level audio
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nQUIT LEVEL: eEndLevel\n");
			break;

		case E_GameflowMessage_QuitAndWinRace:	
			//release all level audio
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nQUIT AND WIN RACE: eEndLevel\n");
			break;


	/*	case E_GameflowMessage_DumpLevel:
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eDumpLevel, 0xffffffff);
			break;*/

		case E_GameflowMessage_RestartLevel:
			//release all level audio
			m_bInRace = false;
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nGAMEFLOW RESTARTLEVEL: endLevel\n");
			break;
		case E_GameflowMessage_QuitRace:
			Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nGAMEFLOW QUIT RACE: endLevel\n");
			//Audio::InterfaceController::Get().BroadcastMessage(Audio::eDumpLevel, 0xffffffff);
			break;

		case E_GameflowMessage_RestartRace:
			//release all level audio
			m_bInRace = false;
		Audio::InterfaceController::Get().BroadcastMessage(Audio::eEndLevel, 0xffffffff);

			if(gbDebugPrintGameState)
				LOGINFO(Game::g_Log, "\nGAMEFLOW RESTART RACE: endLevel\n");

		break;

		case E_GameflowMessage_StartUILoad:

			m_bInRace = false;
		//	Audio::InterfaceController::Get().BroadcastMessage(Audio::eLoadUI, 0xffffffff);
				break;

		case E_GameflowMessage_StartRaceLoad:
			m_bInRace = false;
			m_bInUi = false;
			//Audio::InterfaceController::Get().BroadcastMessage(Audio::eLoadLevel, 0xffffffff);
				break;

		case E_GameflowMessage_RaceLoaded:
			m_bInRace = true;
			//if(!C_Game::GetInstance()->GetLevelState().mInUI && !m_videoPresent)
			//	Audio::InterfaceController::Get().BroadcastMessage(Audio::eBeginLevel, 0xffffffff);
			break;
#endif

		default:
			handledMessage = false;
		break;
	}

	MEMORY_TRACKER_POP("audio");

	return handledMessage;
}

bool C_GameAudioLayer::TitleHasBGMusicControl()
{
	if(m_pSystem == NULL)
		return false;

	return m_pSystem->TitleHasControl();		
}

HRESULT C_GameAudioLayer::GlobalUpdate()
{
	return S_OK;
}

#ifndef CFG_GOLD
const U32 ncNumRgbs = 16;
Maths::RGBA nc_rgbas[ncNumRgbs] =
{
Maths::RGBA(0x00, 0x00, 0xff, 0xff), // blue 
Maths::RGBA(0xff, 0x00, 0x00, 0xff), // red
Maths::RGBA(0xff, 0x00, 0xff, 0xff), // magenta
Maths::RGBA(0x00, 0xff, 0x00, 0xff), // green
Maths::RGBA(0xff, 0xff, 0x00, 0xff), // yellow
Maths::RGBA(0x00, 0xff, 0xff, 0xff), // cyan

Maths::RGBA(0xff, 0xa0, 0xff, 0xff), // pink
Maths::RGBA(0xff, 0x80, 0x00, 0xff), // orange
Maths::RGBA(0xc0, 0xc0, 0xc0, 0xff), // grey
Maths::RGBA(0x80, 0xff, 0x80, 0xff), // pale green
Maths::RGBA(0x80, 0x80, 0x00, 0xff), // brown
Maths::RGBA(0x00, 0x00, 0xa0, 0xff), // dark blue 
Maths::RGBA(0xa0, 0x00, 0x00, 0xff), // dark red
Maths::RGBA(0xa0, 0x00, 0xa0, 0xff), // dark magenta
Maths::RGBA(0x00, 0xa0, 0x00, 0xff), // dark green
Maths::RGBA(0x00, 0xa0, 0xa0, 0xff), // dark cyan
};
#endif
	

HRESULT C_GameAudioLayer::RenderFullscreen()
{
	g_bNamedCountersEnabled = false;

#ifndef CFG_GOLD
	if (m_pSystem)
	{
		m_pSystem->DebugRender();

		if (gbRenderWaveBankDebug)
		{
			if (m_pWaveManager)
				m_pWaveManager->RenderStats();
		}

		if (g_bDrawPerfbarsAndStats)
		{
			Util::C_String str;
			F32 fCpuTime;
			F32 fBarHeight = 0;

			static bool bAverage = true;
			Audio::ISystem::Info info;
			m_pSystem->GetInfo(&info, bAverage);

			
			// draw perf bar
			{
				// work out average cpu time over last few frames
				fCpuTime = info.fTotalCpuTime;
				//fCpuTime = info.fObjUpdateAndGenerateCpuTime;
				if (g_bUseAveragePerfbars)
				{
					static const S32 sNumAverageValues = 8;
					static F32 safAverageCpuTime[sNumAverageValues] = {0};
					static S32 sCurTimeSlot = 0;

					safAverageCpuTime[sCurTimeSlot] = fCpuTime;
					if (++sCurTimeSlot >= sNumAverageValues)
						sCurTimeSlot = 0;

					fCpuTime = 0.f;
					for (S32 i=0; i<sNumAverageValues; i++)
						fCpuTime += safAverageCpuTime[i];
					fCpuTime /= (F32)sNumAverageValues;
				}

				static F32 sfX = 800.f;
				static F32 sfY = 100.f;
				static F32 sfWidth = 100.f;
				static F32 sfHeight = 600.f;
				static F32 sfBarBottomY = sfY + sfHeight;

				F32 fYStart = sfBarBottomY;	// fYStart is the y coord of the bottom of the bar

				// draw bar background
				Display::BasicRender()->DrawColoured2DRect(sfX, sfY, sfWidth, sfHeight, Maths::RGBA(64, 64, 64, 192));

				// draw the bar itself
				F32 fHeightScaled = (fCpuTime/100.f);
				fBarHeight = fHeightScaled * sfHeight;
				fYStart -= fBarHeight;
				Display::BasicRender()->DrawColoured2DRect(sfX, fYStart, sfWidth, fBarHeight, Maths::RGBA(255, 255, 0, 255));
				//if (fHeightScaled > 0.9)
				//	Display::BasicRender()->DrawColoured2DRect(sfX, fYStart, sfWidth, fBarHeight-sfHeight*0.9f, Maths::RGBA(255, 0, 0, 255));

				// draw the max marker
				Display::BasicRender()->DrawColoured2DRect(sfX, sfY, sfWidth, 2.f, Maths::RGBA(255, 255, 255, 255));

				static F32 fPeak = sfBarBottomY;

				if (g_bAudioHoldPeakMarker==false)
				{
					if (fYStart < fPeak)
						fPeak = fYStart;
				}

				if (g_bAudioResetPeakMarker)
				{
					fPeak = sfBarBottomY;
					g_bAudioResetPeakMarker = false;
				}
				static F32 fPeakBarThickness = 2.f;
				Display::BasicRender()->DrawColoured2DRect(sfX, fPeak, sfWidth, fPeakBarThickness, Maths::RGBA(255, 0, 0, 255));
			}

			// named cpu counter display
#ifdef AUDIO_CPU_COUNTER_CODE
			if (g_bAudioShowNamedPerfBar)
			{
				g_bNamedCountersEnabled = true;

				F32 fCpuTimes[MAX_CPU_NAMED_EVENTS];

				// draw perf bar named
				{
					// work out average cpu time over last few frames
					for (U32 i=0; i<MAX_CPU_NAMED_EVENTS; i++)
						fCpuTimes[i] = info.namedCpuEvents[i].fTime;

					if (g_bUseAveragePerfbars)
					{
						static const S32 sNumAverageValues = 8;
						static F32 safAverageCpuTime[MAX_CPU_NAMED_EVENTS][sNumAverageValues] = {0};
						static S32 sCurTimeSlot = 0;

						for (U32 c=0; c<MAX_CPU_NAMED_EVENTS; c++)
						{
							safAverageCpuTime[c][sCurTimeSlot] = fCpuTimes[c];
							
							fCpuTimes[c] = 0.f;
							for (S32 i=0; i<sNumAverageValues; i++)
								fCpuTimes[c] += safAverageCpuTime[c][i];
							fCpuTimes[c] /= (F32)sNumAverageValues;
						}

						if (++sCurTimeSlot >= sNumAverageValues)
							sCurTimeSlot = 0;
					}

					static F32 sfX = 675.f;
					static F32 sfY = 100.f;
					static F32 sfWidth = 100.f;
					static F32 sfHeight = 600.f;
					static F32 sfBarBottomY = sfY + sfHeight;

					F32 fYStart = sfBarBottomY;	// fYStart is the y coord of the bottom of the bar

					// draw bar background
					Display::BasicRender()->DrawColoured2DRect(sfX, sfY, sfWidth, sfHeight, Maths::RGBA(64, 64, 64, 192));

					F32 fCpuTimedNamed;
					F32 fHeightScaled;
					F32 fNamedBarHeight;

					// draw the bar itself
					for (U32 i=0; i<info.uNumNamedCpuEvents; i++)
					{
						fCpuTimedNamed = fCpuTimes[i];
						fHeightScaled = (fCpuTimedNamed/100.f);
						fNamedBarHeight = fHeightScaled * sfHeight;

						fYStart -= fNamedBarHeight;

						//nc_rgbas[i] = Maths::RGBA(0x404040ff | Btl::Hash(info.namedCpuEvents[i].pchName));

						Display::BasicRender()->DrawColoured2DRect(sfX, fYStart, sfWidth, fNamedBarHeight, nc_rgbas[i%ncNumRgbs]);
					}
					// draw the max marker
					//Display::BasicRender()->DrawColoured2DRect(sfX, sfY, sfWidth, 2.f, Maths::RGBA(255, 255, 255, 255));

					static F32 fPeak = sfBarBottomY;

					if (g_bAudioHoldPeakMarker==false)
					{
						if (fYStart < fPeak)
							fPeak = fYStart;
					}

					if (g_bAudioResetPeakMarker)
					{
						fPeak = sfBarBottomY;
						g_bAudioResetPeakMarker = false;
					}
					static F32 fPeakBarThickness = 2.f;
					Display::BasicRender()->DrawColoured2DRect(sfX, fPeak, sfWidth, fPeakBarThickness, Maths::RGBA(255, 0, 0, 255));
				}

				// draw the named counters key
				if (g_bAudioShowPerfBarKey)
				{
					F32 fXCol1 = (F32)g_bAudioPerfBarKeyLeft;
					F32 fXCol2 = (F32)g_bAudioPerfBarKeyLeft + 250.f;
					F32 fY = (F32)g_bAudioPerfBarKeyTop + info.uNumNamedCpuEvents * (F32)g_bAudioPerfBarKeyStep;
					
					for (U32 i=0; i<info.uNumNamedCpuEvents; i++)
					{
						Display::BasicRender()->DrawColoured2DRect(fXCol1, fY-8.f, 200.f, 20.f, Maths::RGBA(0+((i&1)*16), 0+((i&1)*16), 0+((i&1)*16), 255));

						Display::BasicRender()->DrawColoured2DRect(fXCol1, fY-8.f, 16.f, 20.f, nc_rgbas[i%ncNumRgbs]);

						str.Printf("%s", info.namedCpuEvents[i].pchName);
						Display::FontRender()->Draw(str.Chars(), fXCol1+20.f, fY, 16.f, 16.f, Maths::RGBA(255, 255, 255, 255));
						str.Printf("%.2f", fCpuTimes[i]);
						Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 16.f, 16.f, Maths::RGBA(255, 255, 255, 255));
						fY -= (F32)g_bAudioPerfBarKeyStep;
					}
				}
			}
#endif

			// draw stats
			{	
				static F32 fXCol1 = 40.f;
				F32 fXCol2 = 250.f;
				F32 fY = 140.f;
				static F32 fYInc = 20.f;
				/*str.Printf("Generators :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d / %d", info.uActiveGenerators, info.uTotalGenerators);
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("Waves :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d / %d", info.uActiveWaves, info.uTotalWaves);
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;*/

				str.Printf("CPU (percent):");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, fCpuTime>100.f?0:255, fCpuTime>100.f?0:255, 255));
				str.Printf("%d", static_cast<U32>(fCpuTime));
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, fCpuTime>100.f?0:255, fCpuTime>100.f?0:255, 255));
				fY += fYInc;
				str.Printf("Mixers :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d / %d", info.uActiveMixers, info.uTotalMixers);
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("PCM Decoders :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", info.uActivePCMDecoders);
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("XMA Decoders :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", info.uActiveXMADecoders);
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;

				if(info.uActiveXMADecoders > m_peakXmaDecoderCount)
					m_peakXmaDecoderCount = info.uActiveXMADecoders;

				if(g_bAudioResetPeakCPUTime)
					m_peakXmaDecoderCount = 0;

				str.Printf("XMA Decoder Peak :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", m_peakXmaDecoderCount);
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;

				fXCol2 = 300.f;
				str.Printf("Definitions :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", info.uNumObjectDefinitions );
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("Active Instances :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", m_pSystem->GetIObjectManager()->GetNumInstancesActive() );
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("Inactive Instances :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", m_pSystem->GetIObjectManager()->GetNumTotalInstances() - m_pSystem->GetIObjectManager()->GetNumInstancesActive());
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("Muted Instances :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", m_pSystem->GetIObjectManager()->GetNumInstancesMuted());
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("Total Instances :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", m_pSystem->GetIObjectManager()->GetNumTotalInstances() );
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				str.Printf("Mixers for Objects :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%d", m_pSystem->GetIObjectManager()->GetNumMixersActive() );
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fY += fYInc;
				/*str.Printf("Optimizations :");
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%s", gAudioSystemOptimized ? "ON (Fast)" : "OFF (Slow)" );
				Display::FontRender()->Draw(str.Chars(), fXCol2, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));*/
				
				//draw cpu time
				static F32 peakCpuTime = 0;
				if(g_bAudioResetPeakCPUTime)
				{
					peakCpuTime = 0;
					g_bAudioResetPeakCPUTime = false;
				}

				if(fBarHeight > peakCpuTime)
					peakCpuTime = fBarHeight;

				fY += fYInc;
				static F32 cpuMax = 650.f;
				str.Printf("Peak CPU pct : %f", (peakCpuTime/cpuMax) * 100);
				Display::FontRender()->Draw(str.Chars(), fXCol1, fY, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));

				//m_DebugSystem.PrintAt(40.f, 230.f, "Num Active Mixers for Objects = %d", m_pAudioSystem->GetIObjectManager()->GetNumMixersActive());
				//m_DebugSystem.PrintAt(40.f, 254.f, "Num Active Instances = %d", m_pAudioSystem->GetIObjectManager()->GetNumInstancesActive());
				//m_DebugSystem.PrintAt(40.f, 278.f, "Num Inactive Instances = %d", m_pAudioSystem->GetIObjectManager()->GetNumTotalInstancesActive() - m_pAudioSystem->GetIObjectManager()->GetNumInstancesActive());
				//m_DebugSystem.PrintAt(40.f, 292.f, "Total Num Instances = %d", m_pAudioSystem->GetIObjectManager()->GetNumTotalInstancesActive());
			}


			// draw message queue debug
			{
				F32 fMessageQueueFullRatio = (F32)info.uCurrentMessageQueueBytes / (F32)(MESSAGE_QUEUE_SIZE);
				F32 fMessagePeakFullRatio = (F32)info.uPeakMessageQueueBytes / (F32)(MESSAGE_QUEUE_SIZE);

				static F32 fX = 100.f;
				static F32 fHeight = 30.f;
				static F32 fWidth = 400.f;
				static F32 fY = 700.f - fHeight;
				static F32 fYInc = 20.f;
				Display::BasicRender()->DrawColoured2DRect(fX, fY, fWidth, fHeight, Maths::RGBA(0, 0, 0, 255));
				static F32 fZoom = 1.f;
				F32 fTotal = fZoom;
				F32 fYStart = fY;
				F32 fBarWidth = fMessageQueueFullRatio / fTotal;
				Display::BasicRender()->DrawColoured2DRect(fX, fYStart, fWidth * fBarWidth, fHeight, Maths::RGBA(255, 255, 0, 255));
				F32 peakMarker = fMessagePeakFullRatio * fWidth;
				static F32 peakMarkerWidth = 2.f;
				Display::BasicRender()->DrawColoured2DRect(fX+peakMarker-peakMarkerWidth/2, fYStart, peakMarkerWidth, fHeight, Maths::RGBA(255, 0, 0, 255));

				fYStart -= fYInc * 2.5f;
				str.Printf("Message queue :");
				Display::FontRender()->Draw(str.Chars(), fX, fYStart, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%.2f percent full", fMessageQueueFullRatio*100.f);
				Display::FontRender()->Draw(str.Chars(), fX+200.f, fYStart, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				fYStart += fYInc;
				str.Printf("Peak:");
				Display::FontRender()->Draw(str.Chars(), fX, fYStart, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
				str.Printf("%.2f percent full", fMessagePeakFullRatio*100.f);
				Display::FontRender()->Draw(str.Chars(), fX+200.f, fYStart, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
			}
		}
	}
#endif // ndef CFG_GOLD

	return S_OK;
}

HRESULT C_GameAudioLayer::RenderSplitscreen()
{
	return S_OK;
}

////-----------------------------------------------------------------------------
//// AudioSystem::SignalMusicChange
////-----------------------------------------------------------------------------
//void C_GameAudioLayer::SetMusicActionIn(const char* pchTitle)
//{
//	Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(MusicEntityHnd(), pchTitle);
//	Audio::InterfaceController::Get().TriggerEntityActionIn(MusicEntityHnd(), hActionIn);
//}
//
////-----------------------------------------------------------------------------
//// AudioSystem::SetMusicParam
////-----------------------------------------------------------------------------
//void C_GameAudioLayer::SetMusicParam(const char* pchTitle, F32 fValue)
//{
//	Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(MusicEntityHnd(), pchTitle);
//	Audio::InterfaceController::Get().SetEntityValue(MusicEntityHnd(), hParamIn, fValue);
//}
//
////-----------------------------------------------------------------------------
//// AudioSystem::SignalMusicChange
////-----------------------------------------------------------------------------
//void AudioSystem::SignalMusicChange(const char* pchTitle)
//{
//	Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(MusicEntityHnd(), pchTitle);
//	Audio::InterfaceController::Get().TriggerEntityActionIn(MusicEntityHnd(), hActionIn);
//}
//
////-----------------------------------------------------------------------------
//// AudioSystem::SetMusicParam
////-----------------------------------------------------------------------------
//void AudioSystem::SetMusicParam(const char* pchTitle, F32 fValue)
//{
//	Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(MusicEntityHnd(), pchTitle);
//	Audio::InterfaceController::Get().SetEntityValue(MusicEntityHnd(), hParamIn, fValue);
//}

// wrapper function which loads resources for an audio entity, making sure we are not doing so mid-race or in frontend
void CacheEntityResources(Audio::XtEntityProxy& entity, const char* pchName, bool bInRaceWarning, bool bForceLoad)
{
	// dont load any audio resources in in frontend. 
	// this stops loading banks etc for the game level in the background of the UI.
	if (bForceLoad && C_GameApp::GetInstance()->GetGameAudioLayer()->AreWeInRace())
		return;

	bool bLoaded = false;

	// load the entity
	entity.SetupEntityType(bLoaded);

	#ifndef CFG_GOLD
	if (bInRaceWarning)
	{
		if ((C_GameApp::GetInstance()->GetGameAudioLayer()->AreWeInRace() && bLoaded))
			LOGERROR(Game::g_Log, "We are loading audio resources for entity '%s' mid race. Context is '%s'. This should not happen. Please tell the audio team.", entity.Name(), pchName);
		//BIZ_ASSERT_MSG(!(C_GameApp::GetInstance()->GetGameAudioLayer()->AreWeInRace() && bLoaded), "We are loading audio resources for entity '%s' mid race. Context is '%s'. This should not happen.", entity.Name(), pchName);
	}
	#endif
}

//------------------------------------------------------------------------------
//	SetupPath
//-----------------------------------------------------------------------------
void C_GameAudioLayer::SetupPath()
{
	m_strPath = "/audio/";
}

} // namespace Game

Debug::Bool gbAudioIncrementDynamicObjectRefCounts("Audio.IncrementDynamicObjectsRefCountsOnLevelLoad", true);

void C_AudioEntityTypeList::Add(Audio::XtEntityProxy* pEntity)
{
	if (!gbAudioIncrementDynamicObjectRefCounts)
		return;

	for (S32 i=0; i<entityList.Size(); i++)
	{
		if (entityList[i] == pEntity)
			return;
	}

	entityList.PushBack(pEntity);

	//LOGINFO(Game::g_Log, DBSTR" Added '%s' to level entity list", pEntity->Name());
}

void C_AudioEntityTypeList::IncrementRefCounts()
{
	for (S32 i=0; i<entityList.Size(); i++)
	{
		//LOGINFO(Game::g_Log, DBSTR" Incrementing '%s'", entityList[i]->Name());
		entityList[i]->SetupEntityType();
	}
}

void C_AudioEntityTypeList::DecrementRefCounts()
{
	for (S32 i=0; i<entityList.Size(); i++)
	{
		//LOGINFO(Game::g_Log, DBSTR" Decrementing '%s'", entityList[i]->Name());

		entityList[i]->ReleaseEntityType();
	}
}

void Audio::C_ErrorHandler::Log(Debug::LogEvent *pEvent) 
{ 
#ifndef CFG_GOLD
	if ((pEvent->level != DebugLevel::Error && pEvent->level != DebugLevel::Fatal) || !g_bAudioHaltOnErrors)
	{
		return;
	}

#ifndef CFG_GOLD
	#if defined( COMPILER_VC )
	OutputDebugString(pEvent->msg.Chars());
	OutputDebugString("\n");
	#else
	printf("%s\n", pEvent->msg.Chars());
	#endif
#endif

	// This is a big can of worms, what to do here.
	//BIZ_ASSERT_FAIL(pEvent->msg.Chars());

	/*#ifdef PLATFORM_PC
	if (IsDebuggerPresent())
		DebugBreak();
	#endif*/
	#ifdef PLATFORM_PS3
	__asm__ volatile ( "tw 31,1,1" );
	#endif
#endif
}



