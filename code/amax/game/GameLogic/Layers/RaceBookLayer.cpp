#include "stdafx.h"
#include <GameLogic/ContentServer/KeyArchiveManager.h>	// [Blur2PC]

#include <PakFs/PakFsManager.h>

#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameApp.h>
#include <framework/application/ApplicationBase.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include <Ui/UiLayer.h>
#include <Ui/UIManager.h>
#include <ui/Game/Profiles/ProfileInterfaceMessages.h>
#include <GameApp.h>

#include <GameLogic/GameModes/GameModes.h>

#include <GameLogic\Lua\LuaVariableHelper.h>

#include <racingphysics\racingphysics.h>
#include "Network/Core/NetServices.h"

#include <RaceBook/RaceBookManager.h>
#include "../RaceLogicCore/Core/RaceLogicCore.h"

#include <Network/Core.h>

#include <GameLogic/GameData/LocData.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic\InGameCarInfoManager.h>
#include <GameLogic\InGameDialogueManager\InGameDialogueManager.h>
#include <GameLogic\AchievementsManager.h>
#include <GameLogic\EnvironmentSettingsManager.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <Game\Messaging\VehicleActorMessages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <btl/containers/vector.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game\GameActors\Vehicles\VehicleActor.h>

#include <Ui\GameDataFinder.h>
#include <UI/UiXtConfig.h>
#include <ui/game/UIScreenControl.h>

#include <GameLogic\GameData\GameXtLoader.h>

#include <Game\Achievements\AchievementSystem.h>
#include <Game\Achievements\AvatarAwardSystem.h>
#include <Game\Achievements\PictureAwardSystem.h>

#include <Game\GameUtil\LoadingPerf.h>

#include <GameLogic\FriendsChallenges\MessagingManager.h>
#include <GameLogic\Twitter\TwitterManager.h>
#include <GameLogic\Facebook\FacebookManager.h>

#include <Automation/GameDataLogging.h>

#include <GameLogic\InGameStartLineInfoManager.h>

#include <ui/Game/Gadgets/ButtonMessages.h>

#include <Advertising/Advertising.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include <Gamelogic\SocialNetworking\FriendsManager.h>
#include <Gamelogic\SocialNetworking\EventOwnerManager.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // CFG_GOLD

#include <GameLogic\GameLoggingManager.h>

#include <GameLogic/SinglePlayer/SP_BossDemands.h>


#include "Xlast/Amax.spa.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static Debug::Logger &g_GeneralAmaxLayerLog = Debug::GetLogger("Amax.GeneralLog");

Debug::Bool g_DebugLoadOldRaceBook("UI.LoadOldRaceBook", false, Debug::Persist() );

Debug::Bool g_ForceSplitScreenGameMode("SplitScreenMode.ForceSplitScreenMode", false, Debug::Persist() );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Debug::Int g_debugUIFocusCar("UI.ShowRoom.CurrentFocusCar", -1);
Debug::Bool g_debugTestLoadNewCar("UI.ShowRoom.TestLoadNewCar", false);

Debug::Bool g_debugGiveRandomAchievement("Cheats.GiveRandomAchievement", false);

Debug::Bool g_debugGiveAchievement("Cheats.GiveAchievement", false);
Debug::Int g_debugGiveAchievementNumber("Cheats.GiveAchievementNumber", 0);

Debug::Bool g_debugGiveAvatarAward("Cheats.GiveAvatarAward", false);
Debug::Int g_debugGiveAvatarAwardIndex("Cheats.GiveAvatarAwardIndex", 3);

Debug::Bool g_debugGiveGamerPic("Cheats.GiveGamerPic", false);
Debug::Int g_debugGiveGamerPicIndex("Cheats.GiveGamerPicIndex", 0);

Debug::Bool g_debugUpdateUICar("UI.UpdateCar", false);

Debug::Bool g_debugExportEnums("Amax.ExportEnumList", false);

#ifndef CFG_GOLD
Debug::Bool g_debugSuperSpecialCutSceneVideoMode("CutScene.EnableCutSceneVideoRenderMode", false, Debug::Persist());

namespace GameDebug
{
	extern Debug::Float g_debugMiniMonitorPosX;
	extern Debug::Float g_debugMiniMonitorPosY;
}

#endif

namespace Framework { extern Debug::Bool g_DebugKeyCombos; }

namespace UI { extern Debug::Bool g_EnableLoadingScreen; }

namespace Game
{
	extern Debug::Bool g_bootResourceManagerPause;
}

namespace Game
{

XtPack* OpenSpecialFxPack();
void CloseSpecialFxPack();

extern bool g_bEnvironmentSettingsChangedInSorcery;


bool		g_SorceryRecordsModified = false;


C_RaceBookLayer::C_RaceBookLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem ): Framework::C_Layer("C_RaceBookLayer"),
	m_DataGathering(C_RecordEventManager::Instance())
{
	SetSubscriberSystem( pSubscriberSystem );

	m_CurrentGameMode=E_GameMode_Nothing;	//no game mode i.e. Start Screen
	m_CurrentRaceMode=E_RaceMode_Nothing;

	AddSubscriber( E_GameflowMessage_EnteredDebugMenu );
	AddSubscriber( E_GameflowMessage_StartRaceLoad );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );
	AddSubscriber( E_GameflowMessage_StartUILoad );
	AddSubscriber( E_GameflowMessage_UILoaded );
	AddSubscriber( E_GameflowMessage_RaceLoaded );
	AddSubscriber( E_GameflowMessage_ResultsCalculated );
	AddSubscriber( E_GameflowMessage_QuitRace );
	AddSubscriber( E_GameflowMessage_SaveGameLoaded );
	AddSubscriber (E_GameflowMessage_RestartLevel );
	AddSubscriber( Game::E_GameflowMessage_DumpLevel );	
	AddSubscriber( Game::E_GameflowMessage_DumpUI );	
	AddSubscriber( Game::E_GameflowMessage_LoadUIScene );

	m_pUIWorldLayer = 0;
	
	m_pRaceLogicLayer = 0 ;
	m_pAmaxWorldLayer = 0;

	m_pAmaxCurrentWorldLayer = NULL;

	m_UtilityDirectoryCreated = false;

	m_UseMpUICar=false;
	m_LoadingFrontEnd			= false;

	m_LoadState					= E_LoadState_IntoUI;

	m_MpTitleStorageDataActive = false;

#ifndef CFG_GOLD
	//in the E3 build don't allow any button combos
	if(g_UiDebugLaunchMode==UI::C_UiManager::E_LaunchMode_Master)
	{
		Framework::g_DebugKeyCombos=false;
	}
	else
	{
		Framework::g_DebugKeyCombos=true;
	}
#endif
}

void C_RaceBookLayer::UpdateCurrentWorldLayer()
{
	m_updateStreamerOnLoadingScreenMutex.Wait();

	if(m_pAmaxWorldLayer)
		m_pAmaxCurrentWorldLayer = m_pAmaxWorldLayer;
	else if(m_pUIWorldLayer)
		m_pAmaxCurrentWorldLayer = m_pUIWorldLayer;
	else
		m_pAmaxCurrentWorldLayer = NULL;

	m_updateStreamerOnLoadingScreenMutex.Release();
}

C_RaceBookLayer::~C_RaceBookLayer()
{
}

HRESULT C_RaceBookLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;

		// [WillB] - Put update back in
		case Framework::E_LayerMessage_Update:
			return Update();
		break;
		
		case Framework::E_LayerMessage_GlobalUpdate:
			return GlobalUpdate();
		break;

		case Framework::E_LayerMessage_RenderFullscreen:
			return RenderFullscreen();
		break;
	}

	return S_OK;
}

HRESULT C_RaceBookLayer::Create()
{
	MEMORY_TRACKER_PUSH("game");

	// Set up directory for platform-specific runtime saving/loading.
	// Used for replay system and probably some other things down the line

#ifdef PLATFORM_PC

#endif
#if (defined(PLATFORM_PS3))

#endif


	Game::LuaVariableHelper::Instance().CreateLuaTables();

	RacingPhysics::CreateXt( C_GameApp::GetInstance()->GetGlobalXtDatabase() );
		
	GameXtLoader::Instance().AddPack(E_XTPACK_GAMEDATA,						"gamedata\\Xt\\Amax.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_RACELOGICDATABASE,			"gamedata\\Xt\\RLDatabase.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_AIR,							"gamedata\\Xt\\AIR.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_VEHICLE,						"gamedata\\Xt\\Vehicles.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_DRIVER,						"gamedata\\Xt\\Drivers.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_DRIVERANIMATION_ANIMTREES,	"gamedata\\Xt\\DriverAnimation.AnimTrees.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_DRIVERANIMATION_CYCLES,		"gamedata\\Xt\\DriverAnimation.Cycles.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_DRIVERANIMATION_CYCLEGROUPS,	"gamedata\\Xt\\DriverAnimation.CycleGroups.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_PICKUPS,						"gamedata\\Xt\\Pickups.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_CROWD,						"gamedata\\Xt\\Crowd.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_SPLITSCREEN,					"gamedata\\Xt\\SplitScreen.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_SCORING,						"gamedata\\Xt\\Scoring.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_ACHIEVEMENTS,					"gamedata\\Xt\\Achievements.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_SOCIAL_NETWORKING,			"gamedata\\Xt\\SocialNetworking.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_DLC_OFFER_E,					"gamedata\\Xt\\DLC Pack Offer E Deployed.bin");
	GameXtLoader::Instance().AddPack(E_XTPACK_SINGLEPLAYER,					"gamedata\\Xt\\SinglePlayer.bin");	

	//GameXtLoader::Instance().AddPack(E_XTPACK_GLOBAL_ACTOR_LIST,"gamedata\\Xt\\_AS_Global.bin");
	//[PK] moved to be more actor studio friendly
	GameXtLoader::Instance().AddPack(E_XTPACK_GLOBAL_ACTOR_LIST,"gamedata\\levels\\global\\actorpack.xt");

	GameXtLoader::Instance().AddPack(E_XTPACK_RACEBOOK,			"gamedata\\Xt\\FinalRaceBook.bin");

	if (g_bootResourceManagerPause) Resource::C_Manager::GetInstance()->PauseLoadingThread();

	GameXtLoader::Instance().LoadXt();

	C_Game::GetInstance()->GetLevelState().Clear();

	C_MultiplayerData::Instance().LoadMultiplayerPackFile( "gamedata\\Xt\\Multiplayer.bin" );

	if (g_bootResourceManagerPause) Resource::C_Manager::GetInstance()->ResumeLoadingThread();

	//Initialise the game data
	C_GameData::Instance().Init();

	C_GameApp::GetInstance()->CachePostBootAnimations();
	C_GameApp::GetInstance()->CachePostBootFxResourceBag();
	//C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_CachePostBootFxResourceBag );

	InitialiseNetServicesTitleStorage();

	C_GameVehicleData::Instance().Init();
	C_GameEventData::Instance().Init();
	C_RaceBookData::Instance().Init();
	C_AchievementData::Instance().Init();
	C_AchievementsManager::Instance().Init();
	//C_InGameDialogueManager::Instance().Initialise();
	
	C_FriendChallenges::Instance().Init();
	
	C_TwitterManager::Instance().Init();
	C_FacebookManager::Instance().Init();

	C_FriendsManager::Instance().Init();
	C_EventOwnerManager::Instance().Init();

	for(int a=0;a<E_GameMode_MAX;a++)
	{
		gGameModeStore((E_GameMode)a)->Initialise();
	}

	//Get a pointer to the level state
	Game::C_LevelState *pLevelState = &C_Game::GetInstance()->GetLevelState();
	for(int a=0;a<E_RaceMode_MAX;a++)
	{
		gRaceModeStore((E_RaceMode)a)->Initialise(pLevelState);
	}

	RaceBook::C_RaceBookManager::Instance().Init();

	C_GameProfileManager::Instance().Init();

	C_EnvironmentSettingsManager::Instance().Init();
	C_InGameSpecialFXManager::Instance().Init();

	if (g_bootResourceManagerPause) Resource::C_Manager::GetInstance()->PauseLoadingThread();
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_XtInitialised );
	if (g_bootResourceManagerPause) Resource::C_Manager::GetInstance()->ResumeLoadingThread();

#ifndef CFG_GOLD
	C_ReplayDirector::Instance().Init();
#endif // CFG_GOLD

	Game::C_AchievementSystem::Instance().Init();
	Game::C_AchievementSystem::Instance().InitialiseSubscription(&C_Game::GetInstance()->GetSubscriberSystem());
	Game::C_AvatarAwardSystem::Instance().Init();
	Game::C_PictureAwardSystem::Instance().Init();

#ifndef CFG_GOLD
	for( int i = 0; i < m_UiScreensActiveTime.Size(); i++ )
		m_UiScreensActiveTime[i] = 0;
#endif

	MEMORY_TRACKER_POP("game");

	//Open and precache specialfx pack and data.
	//C_GameApp::GetInstance()->CachePostBootFxResourceBag();

	return S_OK;
}

void C_RaceBookLayer::InitialiseNetServicesTitleStorage()
{
	Btl::Vector<Btl::String> files;

	if (C_GameData::Instance().mTitleManagedStorageSettings!=0)
	{
		TitleManagedStorageSettings* settings=C_GameData::Instance().mTitleManagedStorageSettings;
		files.PushBack(settings->mMultiplayerPackFilename);
		files.PushBack(settings->mMultiplayerLocFilename);
	}
	else
	{
		LOGERROR(g_GeneralAmaxLayerLog, "Missing Title Managed Storage filename definitions - setting values to defaults.");
		files.PushBack(Btl::String("Multiplayer_bin.ts"));
		files.PushBack(Btl::String("MultiplayerTS-%s_loc.ts"));
	}

	RacerNet::C_NetServices::Instance().InitialiseTitleStorage(files);
}


HRESULT C_RaceBookLayer::Destroy()
{	
	RaceBook::C_RaceBookManager::Instance().Destroy();
	//Get rid of the Amax World Layer


	// These layers have already been deleted in C_LayerManager::DestroyPendingLayers()
	// Just set member pointers to NULL

 	//delete m_pAmaxWorldLayer;
	m_pAmaxWorldLayer = 0;

	//delete m_pUIWorldLayer;
	m_pUIWorldLayer = 0;

	//delete m_pRaceLogicLayer;
	m_pRaceLogicLayer = 0;

	m_pAmaxCurrentWorldLayer = NULL;

	CloseSpecialFxPack();

	return S_OK;
}

HRESULT C_RaceBookLayer::Update()
{
	PERF_EVENT("RaceBookLayer::Update");
	
	Game::C_AchievementSystem::Instance().Update();
	Game::C_AvatarAwardSystem::Instance().Update();
	Game::C_PictureAwardSystem::Instance().Update();
	
	C_TwitterManager::Instance().Update();
	C_FacebookManager::Instance().Update();
	C_MessagingManager::Instance().Update();

	if(C_GameApp::GetInstance()->IsPaused())
	{
		return S_OK;
	}

	gGameModeStore(m_CurrentGameMode)->Update();

	if(m_CurrentRaceMode != E_RaceMode_Nothing)
	{
		gRaceModeStore(m_CurrentRaceMode)->Update();
		gRaceModeStore(m_CurrentRaceMode)->UpdateHUD();
	}

	RaceBook::C_RaceBookManager::Instance().Update();

	C_EnvironmentSettingsManager::Instance().Update();

	if(g_debugGiveGamerPic)
	{
		g_debugGiveGamerPic=false;
	}

	if(g_debugGiveAvatarAward)
	{
		g_debugGiveAvatarAward=false;
		Game::C_AvatarAwardSystem::Instance().GiveAvatarAward(0,(BLUR_AVATAR_AWARD)((int)g_debugGiveAvatarAwardIndex));
	}

	if(g_debugGiveAchievement)
	{
		g_debugGiveAchievement=false;
		Game::C_AchievementSystem::Instance().GiveAchievement(0,(BLUR_ACHIEVEMENT)((int)g_debugGiveAchievementNumber));
	}

	if(g_debugGiveRandomAchievement)
	{
		g_debugGiveRandomAchievement=false;
		Game::C_AchievementSystem::Instance().GiveAchievement(0,(BLUR_ACHIEVEMENT)(rand()&BLUR_ACHIEVEMENT_MAX));
	}

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	// Update daily challenges 
	float frameTime = C_GameApp::GetInstance()->GetTimeStepRaw();

	// DOnt do any checks until game has actually loaded resources
	if(m_CurrentGameMode != E_GameMode_Nothing)
	{
		m_DailyChallengeManager.Update(frameTime);	
	}
	

#ifndef CFG_GOLD
	if (levelState.mInUI && g_debugUpdateUICar && GetUIWorldLayer() && GetUIWorldLayer()->GetRaceLogicCore())
	{
		UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();
		if (pConfig)
		{
			Maths::HMat43 matrix(Maths::HMat33(Maths::C_Quaternion(Maths::HVec3(0.0f, 1.0f, 0.0f), Maths::DegToRad((float)pConfig->m_CarOrientation))),
								 Maths::HVec3((float)pConfig->m_CarPosition.vx, (float)pConfig->m_CarPosition.vy, (float)pConfig->m_CarPosition.vz));

			for (unsigned int iCar = 0; iCar < GetUIWorldLayer()->GetRaceLogicCore()->GetNumVehicles(); iCar++)
			{
				Game::Actors::C_VehicleActor* pActor = GetUIWorldLayer()->GetRaceLogicCore()->GetVehicleActor(iCar);
				if (pActor)
				{
					Game::Actors::C_VehicleResetAtWorldPositionMsg msg(pActor->GetHandle(), iCar, matrix, Maths::Constants<Maths::HVec3>::Zero());
					pActor->SendMessageToActor(pActor->GetHandle(), &msg);
				}
			}
		}
	}

	if (levelState.mInUI && g_debugTestLoadNewCar && GetUIWorldLayer() && GetUIWorldLayer()->GetRaceLogicCore())
	{
		g_debugTestLoadNewCar = false;

		RaceBook::C_RaceBookVehicle* pVehicle = NULL;

		if (g_debugUIFocusCar == 1)
		{
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			if (pGameProfile && pGameProfile->mRaceBookProfile)
			{
				pVehicle = RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)pGameProfile->mRaceBookProfile->mCurrentCarID);
			}
			g_debugUIFocusCar = 0;
		}
		else
		{
			pVehicle = RaceBook::C_RaceBookManager::Instance().GetVehicle(0);//C_GameVehicleData::Instance().mVehicles["Nissan_350ZNismo_Stock"];
			g_debugUIFocusCar = 1;
		}

		GetUIWorldLayer()->GetRaceLogicCore()->StartVehicleLoad(pVehicle);
	}
#endif

#ifndef CFG_GOLD
	for( int i = 0; i < UI::C_ScreenControlManager::Instance().Count(); i++ )
	{
		if( i >= m_UiScreensActiveTime.Size())
		{
			LOGWARN(g_GeneralAmaxLayerLog, "UI screen number greater than size of time logger - tell SteveB");
			break;
		}

		if(UI::C_ScreenControlManager::Instance().Get(i).Active())
		{
			if(m_UiScreensActiveTime[i] == 0)
				strcpy(m_CurrentUIScreen, UI::C_ScreenControlManager::Instance().Get(i).m_NextScreen);

			m_UiScreensActiveTime[i] += frameTime;
		}
		else if(m_UiScreensActiveTime[i] > 0)
		{
			AmaxDataLogging::LogTimeOnScreen(m_UiScreensActiveTime[i], m_CurrentUIScreen);
			m_UiScreensActiveTime[i] = 0;
		}
	}
#endif
	
	if(g_debugExportEnums)
	{
		C_GameLoggingManager::Instance().DebugExportEnumNames();
		g_debugExportEnums=false;
	}

	return S_OK;
}

HRESULT C_RaceBookLayer::GlobalUpdate()
{
	PERF_EVENT("RaceBookLayer::GlobalUpdate");

	g_SorceryRecordsModified = false;
	if (XtNetworkUpdate())
	{
		g_SorceryRecordsModified = true;
		g_bEnvironmentSettingsChangedInSorcery = true;
		g_bPhysicsMaterialsReload = true;
	}

	// needs constant updates, even when paused.
	RacerNet::C_NetServices::Instance().Poll();
	
#ifndef CFG_GOLD
	C_ReplayDirector::Instance().GlobalUpdate();
#endif // CFG_GOLD
	
	return S_OK;
}

void C_RaceBookLayer::LoadUIScene( bool bCreateAsync )
{
	C_Game::GetInstance()->GetLevelState().Clear();

	SetFrontEndLoading(true);

	AmaxCity* pCity					= C_GameData::Instance().mCities["ShowRoom"];
	AmaxRoute* pRoute				= C_GameData::Instance().mRoutes["ShowRoom"];

	if (!pCity || !pRoute)
		return;

	C_Game::GetInstance()->GetLevelState().mCity=pCity->ID;
	C_Game::GetInstance()->GetLevelState().mRoute=pRoute->ID;
	C_Game::GetInstance()->GetLevelState().mEvent						= 0;
	C_Game::GetInstance()->GetLevelState().mNumCars						= 0;
	C_Game::GetInstance()->GetLevelState().mInUI						= true;
	C_Game::GetInstance()->GetLevelState().Finished();

	if(m_pUIWorldLayer == 0)
	{
		m_pUIWorldLayer = new C_AmaxWorldLayer( &GetSubscriberSystem() );

		//If we're coming in from the boot layer...
		if( bCreateAsync )
		{
			C_GameApp::GetInstance()->InsertLayerAsync( m_pUIWorldLayer );
			//Disable initially, still allows creation to occur.
			//This UI world layer will be enabled within C_GameApp::EnableMainLayers()
			m_pUIWorldLayer->DisableLayer();
		}
		else
		{
			C_GameApp::GetInstance()->InsertLayer( m_pUIWorldLayer );
		}
		UpdateCurrentWorldLayer();
	}
}

bool C_RaceBookLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	MEMORY_TRACKER_PUSH("game");

	PERF_EVENT("C_RaceBookLayer::MessageHandler");

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case E_GameflowMessage_EnteredDebugMenu:
			C_Game::GetInstance()->GetLevelState().Clear();
			break;
			
		case E_GameflowMessage_LoadUIScene:
			{
//				if (m_CurrentGameMode == E_GameMode_SinglePlayer && !C_Game::GetInstance()->GetLevelState().mDebugEvent)
				if (m_CurrentGameMode != E_GameMode_Debug && !C_Game::GetInstance()->GetLevelState().mDebugEvent)
				{
					//Clear LevelState to avoid problems with the one created for the front end single player menu
					Game::C_LevelState&  levelState  = Game::C_Game::GetInstance()->GetLevelState();
					levelState.Clear();

					if(UI::g_EnableLoadingScreen)
					{
						C_GameApp::GetInstance()->EnableLoadingScreen();
					}

					LoadUIScene();
				}
			}
			break;

		case E_GameflowMessage_StartRaceLoad:
			{
				if(m_pUIWorldLayer)
				{
					C_GameApp::GetInstance()->DestroyLayer( m_pUIWorldLayer );
					m_pUIWorldLayer = 0;
					UpdateCurrentWorldLayer();
				}

				if(m_pRaceLogicLayer)
				{
					C_GameApp::GetInstance()->DestroyLayer( m_pRaceLogicLayer );
					m_pRaceLogicLayer = 0;
				}

				BIZ_ASSERT(m_pAmaxWorldLayer == NULL);

				m_pAmaxWorldLayer = new C_AmaxWorldLayer( &GetSubscriberSystem() );
				C_GameApp::GetInstance()->InsertLayer( m_pAmaxWorldLayer );
				UpdateCurrentWorldLayer();

				BIZ_ASSERT(m_pRaceLogicLayer == NULL);

				//must be created after the amax layer
				m_pRaceLogicLayer = new C_RaceLogicLayer( &GetSubscriberSystem() );

				
				C_GameApp::GetInstance()->InsertLayer( m_pRaceLogicLayer );
				//C_GameApp::GetInstance()->InsertLayerAsync( m_pAmaxWorldLayer );
				//C_GameApp::GetInstance()->InsertLayerAsync( m_pRaceLogicLayer );

				break;
			}

		case E_GameflowMessage_RaceStarted:
			{
				gGameModeStore(m_CurrentGameMode)->StartRace();

				gRaceModeStore(m_CurrentRaceMode)->StartRace();

				//HACK HACK HACK
				//CA - Need to get this message to the AmaxWorldLayer
				m_pAmaxWorldLayer->GetRaceLogicCore()->MessageHandler(pMessage);
				//HACK HACK HACK
			}
			break;

		case E_GameflowMessage_QuitRace:
			gRaceModeStore(m_CurrentRaceMode)->QuitLevel();
			break;

		case E_GameflowMessage_ResultsCalculated:
			{
				if(m_CurrentRaceMode == E_RaceMode_StreetRace ||
					m_CurrentRaceMode == E_RaceMode_Destruction ||
						m_CurrentRaceMode == E_RaceMode_Checkpoint ||
							m_CurrentRaceMode == E_RaceMode_FanDemand ||
								m_CurrentRaceMode == E_RaceMode_BossBattle||
								gGameModeStore(m_CurrentGameMode)->IsMultiplayer())
				{
					gGameModeStore(m_CurrentGameMode)->InterpretRaceResults();
				}
			}
			break;

		case Game::E_GameflowMessage_RestartLevel:
			{
				if (!m_pAmaxWorldLayer || !m_pRaceLogicLayer)
					break;

				LOADING_PERF("RestartLevel");
#ifndef CFG_GOLD
				AmaxDataLogging::LogRaceEnd();
#endif
				m_pRaceLogicLayer->Destroy(true);
				m_pAmaxWorldLayer->Destroy(true);
				gRaceModeStore(m_CurrentRaceMode)->Reset();
				C_InGameStartLineInfoManager::Instance().Init();
				m_pAmaxWorldLayer->Create(true);
				m_pRaceLogicLayer->Create(true);
			}
			break;

		case E_GameflowMessage_StartUILoad:
		case E_GameflowMessage_DumpLevel:
			{
#ifndef CFG_GOLD
				//Delay end of logging in online races as we care about post race information such as awards
				if(!GetGameModePtr() || !GetGameModePtr()->IsOnline())
				{
					AmaxDataLogging::LogRaceEnd();
				}
#endif

				UI::C_GameDataFinder::Instance().RemoveMessaging();

				DumpLevel();
				
				if (m_pRaceLogicLayer)
				{
					C_GameApp::GetInstance()->DestroyLayer( m_pRaceLogicLayer );
					m_pRaceLogicLayer = 0;
				}
			}
			break;
		case E_GameflowMessage_DumpUI:
			{
				UI::C_GameDataFinder::Instance().RemoveMessaging();

				if(m_pUIWorldLayer)
				{
					C_GameApp::GetInstance()->DestroyLayer( m_pUIWorldLayer );
					m_pUIWorldLayer = 0;
					UpdateCurrentWorldLayer();
				}
				else
					_UI_MESSAGE(E_GameflowMessage_LevelDumped);


				if (m_pRaceLogicLayer)
				{
					C_GameApp::GetInstance()->DestroyLayer( m_pRaceLogicLayer );
					m_pRaceLogicLayer = 0;
				}

				if (m_CurrentGameMode == E_GameMode_SinglePlayer && !C_Game::GetInstance()->GetLevelState().mDebugEvent)
				{
					//Clear LevelState to avoid problems with the one created for the front end single player menu
					Game::C_LevelState&  levelState  = Game::C_Game::GetInstance()->GetLevelState();
					levelState.Clear();
				}
			}
			break;
		case E_GameflowMessage_RaceLoaded:
			{
				gRaceModeStore(m_CurrentRaceMode)->StartLevel();

				Advertising::EnterZone_PostLoad(
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetLevelResource() /*->GetRenderLevel()*/,
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetLevelInstance());
			}
			break;

		default:
			break;
	}

	MEMORY_TRACKER_POP("game");

	return handledMessage;
}

bool C_RaceBookLayer::DumpLevel()
{
	C_Game::GetInstance()->GetLevelState().ClearMultiplayerAbilities();

	//CWright - this is actually valid, as in MP the level gets dumped before 
	//the next-event intermission lobby and the player can quit out back to UI
	//if (!m_pUIWorldLayer && !m_pAmaxWorldLayer)
	//	BIZ_ASSERT_FAIL("m_pAmaxWorldLayer == NULL and m_pUIWorldLayer == NULL");
	if (m_pAmaxWorldLayer && m_pUIWorldLayer)
		BIZ_ASSERT_FAIL("m_pUIWorldLayer != NULL but m_pAmaxWorldLayer != NULL");
	
	if (!m_pUIWorldLayer && !m_pAmaxWorldLayer)
	{
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_LevelDumped );
		_UI_MESSAGE(E_GameflowMessage_LevelDumped);
	}

	if (m_pAmaxWorldLayer)
	{
		C_GameApp::GetInstance()->DestroyLayer( m_pAmaxWorldLayer );
		m_pAmaxWorldLayer = NULL;
	}
	
	if (m_pUIWorldLayer)
	{
		C_GameApp::GetInstance()->DestroyLayer( m_pUIWorldLayer );
		m_pUIWorldLayer = NULL;
	}

	//	[GavinY] Bug: #38264 - Safety check. We don't want mRaceInProgress to EVER be true once we've dumped the level
	if(GetRaceModePtr() && GetRaceModePtr()->IsRaceInProgress())
		GetRaceModePtr()->QuitLevel();

	UpdateCurrentWorldLayer();

	return true;
}

HRESULT C_RaceBookLayer::RenderFullscreen()
{	
	return S_OK;
}

void C_RaceBookLayer::SetGameMode(E_GameMode gm)
{
	m_CurrentGameMode=gm;

	// Allow for testing of split screen mode (we have no UI option yet)
	if(g_ForceSplitScreenGameMode)
	{
		m_CurrentGameMode= E_GameMode_SplitScreen;
	}

	//If we are using the title storage data
	if (m_MpTitleStorageDataActive)
	{
		//And we need to deactivate it
		if(!gGameModeStore(m_CurrentGameMode)->IsOnline() && m_CurrentGameMode != E_GameMode_Nothing) //Wait for an actual game mode to be set before deciding what to do
		{
			C_MultiplayerData::Instance().LoadMultiplayerPackFile( "gamedata\\Xt\\Multiplayer.bin" );
		}
	}

	gGameModeStore(m_CurrentGameMode)->Start();
}

void C_RaceBookLayer::SetupRace(void *data)
{
	gGameModeStore(m_CurrentGameMode)->SetupRace(data);
}

void C_RaceBookLayer::SetRaceMode(E_RaceMode rm)
{
	m_CurrentRaceMode=rm;
}

const E_RaceMode C_RaceBookLayer::GetRaceMode() const
{
	return m_CurrentRaceMode;
}

void C_RaceBookLayer::SetupResults()
{
	gRaceModeStore(m_CurrentRaceMode)->SetupResults();
	SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_EVENT_FINISHED);
}

void C_RaceBookLayer::SetupQuitAndWinResults(lua_State *pState)
{
	gRaceModeStore(m_CurrentRaceMode)->SetupQuitAndWinResults(pState);
}

void C_RaceBookLayer::SetupFakeResults()
{
	gRaceModeStore(E_RaceMode_StreetRace)->SetupFakeResults();
}

DataGathering& C_RaceBookLayer::GetDataGathering()
{
	return m_DataGathering;
}

void C_RaceBookLayer::SetTitleStorageDataActive(bool active) 
{
	m_MpTitleStorageDataActive = active; 
}

MT::Mutex& C_RaceBookLayer::GetUpdateStreamerOnLoadingScreenMutex()
{
	return m_updateStreamerOnLoadingScreenMutex;
}

void C_RaceBookLayer::EnterNetworkLoad()
{
	// need to kill any active demonware tasks since these can't run while demonware is being updated via the loading thread.
	Game::C_KeyArchiveManager::Instance().KillCurrentTask();
	Game::C_RecordEventManager::Instance().KillCurrentTask();

	m_NetLoadingThread.Enter();
}

void C_RaceBookLayer::ExitNetworkLoad()
{
	m_NetLoadingThread.Leave();
}

} // namespace Game
