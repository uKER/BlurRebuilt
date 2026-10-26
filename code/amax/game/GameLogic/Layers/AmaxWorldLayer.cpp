#include "stdafx.h"

#include <system/io/async.h>
#include <system/time/simpletimer.h>
#include <OverlayFs/OverlayFs.h>
#include <GameFs/GameHelpers.h>
#include <PakFs/PakFsManager.h>

#include <Game/Game.h>
#include <GameSystems.h>

#include <framework/application/ApplicationBase.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceRules/RaceRulesManager.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include <gravity/Solver2/ConstraintGraphManager.h>
#include <gravity/sharedphysics/Material.h>

#include <Render/Render/GridRender.h>
#include <camera2.h>
#include <SceneGraph/SceneGraph.h>

#include <Render/Utils/Helper.h>

#include <world_render\RenderLevel.h>

#include <animated_model/TaskManager.h>

// temp
#include <Game/GameActors/SystemActors.h>
#include <Game/GameControls/GameControls.h>

#include <Game/Messaging/GameflowMessages.h>
#include <Game/Messaging/GameMessages.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <GameApp.h>

#include <DebugSystems/DebugHUDMenu.h>

#include <Game/GameActors/TestActors.h>
#include <Game/GameActors/Effects/EffectTestActor.h>

#include <Game/GameDef/Polyflags.h>
#include <Game/GameDef/Materials.h>

#include "Ui/UiLayer.h"
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/SimpleTimer.h>
#include <Ui/Game/Multiplayer/MultiplayerCache.h>

#include <Game/GameActors/Drivers/DriverManager.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Vehicle/VehicleModelManager.h>

#include <game/AmaxCam/CameraSystem.h>
#include <game/AmaxCam/CameraSwitcher.h>
#include <shared_actors/XtActorCreation.h>

#include "../RaceLogicCore/Core/RaceLogicCore.h"
#include "AI/AI_Amax.h"
#include <network/Core/NetServices.h>
#include "gravity/narrowphase/Ray.h"

#include <GameLogic/GameData/GameData.h>
#include <GameLogic/GameData/GameEventData.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>

#include <system/Memory.h>

#include <system/automation/automationmessages.h>

#include <GameLogic\InGameCarInfoManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <ui\Graphics\Simple2d.h>
#include <game_resource/PhysicsLevel.h>
#include <game_resource/LevelSceneManager.h>
#include <game_resource/EffectVariations.h>

#include <GameLogic\RaceIntroOutro\RaceIntroOutro.h>

#include <Vehicle/VehicleModel.h>
#include <UI/UiMessages.h>

#include <GameLogic/GameData/GameEventData.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <game_debug/GameDebug.h>

#include <crowd_system/CrowdSystem.h>

#include <input/Device.h>

#include <Game\GameUtil\LoadingPerf.h>
#include "Advertising/Advertising.h"

#include <actor_studio_msg_game/ActorStudioMessaging.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include "Game\PerfStats\PerfStats.h"

#include "Lod/LodManager.h"

#include <GameLogic\ContentServer\KeyArchiveManager.h>
#include <GameLogic\ContentServer\RecordEventManager.h>
#include <GameLogic\ContentServer\LinkCodeManager.h>

#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <Game\GameWorld\RenderPass.h>
#include <Game\GameWorld\ViewType.h>
#include <Game\GameWorld\ViewManager.h>

#define RENDERINGDATA_ALLOW_INCLUDE

#include "renderingdata/renderingdata.h"

#include "XtAudio.h"
#ifdef AUDIO_DEFERRED_LOADING
#include "GameAudio/GameAudioLayer.h"
#endif

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

bool g_bPhysicsMaterialsReload = false;

Debug::Bool g_DebugDisableAllActorStudioActors("RLC.DisableAllActorStudioActors", false);
Debug::Bool g_DebugForceLoadMultiplayerActorLists("Multiplayer.ForceLoadMultiplayerActorLists", false);

// TEMPORARY
Debug::Bool g_SpawnParticleEventTestActor("Test.Particle.SpawnParticleEventTestActor", false);
Debug::Int  g_ParticleEventIndex("Test.Particle.ParticleEventIndex", 0);
//Debug::Bool g_debugCutScenesUseSetupList("CutScene.General.UseSetupActorList", true);

Debug::Bool g_enableLutRamPak("Game.EnableLutRamPak", true);


#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif


/******************************************************************************\

			Quick Hack for Designers to Play with Grav Gun and Perks

 			 (it will be moved to Sorcery if they like it this way)

\******************************************************************************/

Debug::Int g_PerkGravGunSlot("Amax.Perks.GravGunSlot", 1, Debug::Range(1, 5), Debug::Step(1), Debug::Persist());
Debug::Bool g_PerkGravGunSlot_Record("Amax.Perks.RecordGravGun", false);
Debug::Int g_PerkGravGunSlot_PlayBack_VehicleIndex("Amax.Perks.PlayBack_VehicleIndex", 0, Debug::Range(0, 19), Debug::Step(1), Debug::Persist());

Debug::Float g_PerkGravGunSlot1_ForceX("Amax.Perks.Slot1_ShuntRight.ForceX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot1_ForceY("Amax.Perks.Slot1_ShuntRight.ForceY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot1_ForceZ("Amax.Perks.Slot1_ShuntRight.ForceZ", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot1_PosX("Amax.Perks.Slot1_ShuntRight.PosX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot1_PosY("Amax.Perks.Slot1_ShuntRight.PosY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot1_PosZ("Amax.Perks.Slot1_ShuntRight.PosZ", 0.f, Debug::Persist());
Debug::Bool g_PerkGravGunSlot1_PlayBack("Amax.Perks.Slot1_ShuntRight.PlayBack", false);

Debug::Float g_PerkGravGunSlot2_ForceX("Amax.Perks.Slot2_ShuntLeft.ForceX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot2_ForceY("Amax.Perks.Slot2_ShuntLeft.ForceY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot2_ForceZ("Amax.Perks.Slot2_ShuntLeft.ForceZ", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot2_PosX("Amax.Perks.Slot2_ShuntLeft.PosX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot2_PosY("Amax.Perks.Slot2_ShuntLeft.PosY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot2_PosZ("Amax.Perks.Slot2_ShuntLeft.PosZ", 0.f, Debug::Persist());
Debug::Bool g_PerkGravGunSlot2_PlayBack("Amax.Perks.Slot2_ShuntLeft.PlayBack", false);

Debug::Float g_PerkGravGunSlot3_ForceX("Amax.Perks.Slot3_ShockRight.ForceX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot3_ForceY("Amax.Perks.Slot3_ShockRight.ForceY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot3_ForceZ("Amax.Perks.Slot3_ShockRight.ForceZ", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot3_PosX("Amax.Perks.Slot3_ShockRight.PosX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot3_PosY("Amax.Perks.Slot3_ShockRight.PosY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot3_PosZ("Amax.Perks.Slot3_ShockRight.PosZ", 0.f, Debug::Persist());
Debug::Bool g_PerkGravGunSlot3_PlayBack("Amax.Perks.Slot3_ShockRight.PlayBack", false);

Debug::Float g_PerkGravGunSlot4_ForceX("Amax.Perks.Slot4_ShockLeft.ForceX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot4_ForceY("Amax.Perks.Slot4_ShockLeft.ForceY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot4_ForceZ("Amax.Perks.Slot4_ShockLeft.ForceZ", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot4_PosX("Amax.Perks.Slot4_ShockLeft.PosX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot4_PosY("Amax.Perks.Slot4_ShockLeft.PosY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot4_PosZ("Amax.Perks.Slot4_ShockLeft.PosZ", 0.f, Debug::Persist());
Debug::Bool g_PerkGravGunSlot4_PlayBack("Amax.Perks.Slot4_ShockLeft.PlayBack", false);

Debug::Float g_PerkGravGunSlot5_ForceX("Amax.Perks.Slot5_Test.ForceX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot5_ForceY("Amax.Perks.Slot5_Test.ForceY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot5_ForceZ("Amax.Perks.Slot5_Test.ForceZ", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot5_PosX("Amax.Perks.Slot5_Test.PosX", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot5_PosY("Amax.Perks.Slot5_Test.PosY", 0.f, Debug::Persist());
Debug::Float g_PerkGravGunSlot5_PosZ("Amax.Perks.Slot5_Test.PosZ", 0.f, Debug::Persist());
Debug::Bool g_PerkGravGunSlot5_PlayBack("Amax.Perks.Slot5_Test.PlayBack", false);
bool g_GameDebugRender2d = true;

/*                                  END                                               */

namespace GameDebug
{
	extern Debug::Bool g_debugMiniMonitorEnable;
}


namespace AmaxWorldLayer_Impl
{

/// Wrapper class for loading level actors.
/// When the level actors finish loading and get created, the wrapper object
/// is destroyed. It's destructor informs the game world layer that the actor
/// studio data is ready for use.
class C_AmaxLevelActorsCreate : public C_LevelActorCreationDataList
{
public:
	virtual ~C_AmaxLevelActorsCreate()
	{
		C_GameApp* gameApp = C_GameApp::GetInstance();
		if (gameApp)
		{
			Game::C_RaceBookLayer* rbl = gameApp->GetRaceBookLayer();
			if (rbl)
			{
				Game::C_AmaxWorldLayer* worldLayer = rbl->GetAmaxWorldLayer();
				if (worldLayer)
				{
					worldLayer->NotifyActorStudioDataCreated();
				}
			}
		}
	}
};

} // namespace AmaxWorldLayer_Impl

namespace Game
{

void AmaxWorldPackOpenCallback(XtPackOpenStatus* status, void* context)
{
	LOGWARN( g_Log, "AmaxWorldPackOpenCallback: %s", status->message);
}

#define LUT_RAMPAK_SOURCE_FILEPATH "cache/lut_rampak.pak"

C_AmaxWorldLayer::C_AmaxWorldLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
: Framework::C_Layer("C_AmaxWorldLayer"),
	 m_pGameWorld(NULL)
	, m_pLevelInstance(NULL)
	, m_pLevelResource(NULL)
	, m_pRLC(NULL)
	, mRLCReady(false)
	, mAIReady(false)
	, mCanRender(false)
	, mActorsLoaded(false)
	, m_WorldState(E_WorldState_Uninitialised)
	, m_CutSceneState(E_CutSceneState_Uninitialised)
	, m_LevelActorCreationPack(NULL)
	, m_pActorStudioMessaging(NULL)
	, m_pXtDb(NULL)
	#ifndef CFG_GOLD
	, m_pGameDebug(NULL)
	#endif
{
	SetSubscriberSystem( pSubscriberSystem );

	AddSubscriber( Game::E_GameflowMessage_RLCReadyToRock );
	AddSubscriber( Game::E_AIMessage_AllLoaded );
	AddSubscriber( Game::E_AIMessage_LoadFailed );
	
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );

	AddSubscriber( E_GameflowMessage_Pause );
	AddSubscriber( E_GameflowMessage_UnPause );
	AddSubscriber( E_GameflowMessage_QuitRace );

	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );
	AddSubscriber( Game::E_GameflowMessage_RestartRace );
	AddSubscriber( Game::E_GameflowMessage_RestartLevel );

	AddSubscriber( E_GameflowMessage_StartGameRendering );
	AddSubscriber( E_GameflowMessage_StopGameRendering );
	AddSubscriber( E_GameflowMessage_StopGameUpdate );	
	
	AddSubscriber( E_GameflowMessage_PrepareOutro );

	AddSubscriber( Game::E_VehicleMessage_Respray );

	AddSubscriber( Game::E_GameflowMessage_LoadSPProfileCar );
	AddSubscriber( Game::E_GameflowMessage_LoadMPProfileCar );

	AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );		

	#ifndef CFG_GOLD
	m_pGameDebug = new GameDebug::C_GameDebug;
	m_pGameDebug->SetAlbedoGamma(true);
	m_pGameDebug->SetSpecularGamma(true);

	m_pGameDebug->SetPerfLimits(33.f, 33.f, 12.f, 14.f, 14.f);
	//m_pGameDebug->SetEmissiveGamma(true);
	#endif

	m_lutRamPakMounted = false;
}

C_AmaxWorldLayer::~C_AmaxWorldLayer()
{
	#ifndef CFG_GOLD
	SAFE_DELETE_PTR( m_pGameDebug );
	#endif

	if (m_LevelActorCreationPack)
		XtPackClose(C_GameApp::GetInstance()->GetGlobalXtDatabase(), m_LevelActorCreationPack);
}

HRESULT C_AmaxWorldLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	HRESULT result = S_OK;

	switch( message )
	{
		case Framework::E_LayerMessage_Create:
		{
			result = Create();
			break;
		}
		case Framework::E_LayerMessage_Destroy:
		{
			result = Destroy();
			break;
		}
		case Framework::E_LayerMessage_Update:
			result = Update();
			break;

		case Framework::E_LayerMessage_GlobalUpdate:
			result = GlobalUpdate();
			break;	

		case Framework::E_LayerMessage_RenderFullscreen:
			result = RenderFullscreen();
			break;

		case Framework::E_LayerMessage_EndFrame:
		{
			result = PostRender();
			break;
		}
		case Framework::E_LayerMessage_Resize:
		{
			FX::RestoreDefaultData(); // Clear the fx states when window resized
			m_pGameWorld->Resize();
			break;
		}
		case Game::E_GameflowMessage_RLCResourcesLoaded:
		{
			mRLCReady = true;
			break;
		}
	}

	if ( mCanRender )
	{
		switch( message )
		{
			case Framework::E_LayerMessage_BeginFrame:
				m_pGameWorld->BeginFrame();
				break;

			case Framework::E_LayerMessage_ScheduleOnce:
				m_pGameWorld->ScheduleOnce();
				break;

			case Framework::E_LayerMessage_RenderOnce:
				m_pGameWorld->RenderOnce();
				break;

			case Framework::E_LayerMessage_BeginSplitscreen:
				m_pGameWorld->BeginSplitscreen();
				break;

			case Framework::E_LayerMessage_ScheduleSplitscreen:
				m_pGameWorld->ScheduleSplitscreen();
				break;

			case Framework::E_LayerMessage_PreRenderSplitscreen:
				m_pGameWorld->PreRenderSplitscreen();
				break;

			case Framework::E_LayerMessage_BeginRenderSplitscreenWithoutAA:
				m_pGameWorld->BeginRenderSplitscreenWithoutAA();
				break;

			case Framework::E_LayerMessage_EndRenderSplitscreenWithoutAA:
				m_pGameWorld->EndRenderSplitscreenWithoutAA();
				break;

			case Framework::E_LayerMessage_RenderSplitscreenWithAA:
				result = RenderSplitscreenWithAA();
				break;

			case Framework::E_LayerMessage_RenderSplitscreenWithoutAA:
				result = RenderSplitscreenWithoutAA();
				break;

			case Framework::E_LayerMessage_RenderFullscreen:
				m_pGameWorld->RenderFullscreen();
				break;

		}
	}
	return result;
}

void C_AmaxWorldLayer::LoadUILutRamPak()
{
	if (g_enableLutRamPak && !m_lutRamPakMounted)
	{
		Sys::C_FilePath ramPak = LogicalFs::GameHelpers::GetPotentiallyInstalledFilePath(LUT_RAMPAK_SOURCE_FILEPATH);
#ifndef CFG_GOLD
		Time::C_SimpleTimer simpleTimer;
		Sys::C_FileInfo info;
		LogicalFs::GameHelpers::GetGameFs()->FileInfo(ramPak, &info);
#endif

		if (LogicalFs::GameHelpers::MountRamPakLayer(
			Sys::C_FileSystem::Instance(),
			*C_GameSystems::Instance().GetMemoryFs(),
			*LogicalFs::GameHelpers::GetGameFs(),
			ramPak,
			LUT_RAMPAK_SOURCE_FILEPATH,
			"/",
			"/"))
		{
			m_lutRamPakMounted = true;
	#ifndef CFG_GOLD
			F32 sizeMb = (F32)info.m_FileSize / (1024.f*1024.f);
			F32 deltaSecs = simpleTimer.Stop() / 1000.f;

			LOGINFO(g_Log, "%s read in %.2f secs @ %.2fmb/s", ramPak.Chars(), deltaSecs, sizeMb / deltaSecs);
	#endif
		}
	}
}

void C_AmaxWorldLayer::UnloadUILutRamPak()
{
	if (m_lutRamPakMounted)
	{
		LogicalFs::GameHelpers::UnmountRamPakLayer(
			*C_GameSystems::Instance().GetMemoryFs(),
			*LogicalFs::GameHelpers::GetGameFs(),
			LUT_RAMPAK_SOURCE_FILEPATH);

		m_lutRamPakMounted = false;
	}
}

HRESULT C_AmaxWorldLayer::Create(bool quickRestart)
{

	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() == E_LoadState_IntoRace)
	{
		Resource::C_Manager::GetInstance()->SetBackgroundLoadingMode(Resource::E_BackgroundLoading_Disabled);
	}
	
	//Once only on boot
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() == E_LoadState_IntoUI)
	{
		LoadUILutRamPak();
	}

	#ifdef ENABLE_LOAD_PERF
	// Handle case where we go into RaceBook from the start screen
	if (levelState.mInUI && !LoadPerf::ProfilingStarted())
	{
		LoadPerf::Push("Loading");
	}

	LoadPerf::Push("C_AmaxWorldLayer::Create()");
	#endif

	//LOADING_PERF("C_AmaxWorldLayer::Create");

	RESOURCE_MEMORY_ID("AmaxWorldLayer");
	MEMORY_TRACKER_PUSH("application.amaxworldlayer");

	if (!quickRestart)
	{
		m_WorldState = E_WorldState_LoadingRace;
		m_RandomGenerator.Seed((U32)Time::GetCurrentTime());
		m_RandomNumberTracker.Clear();
	}
	else
	{
		m_WorldState = E_WorldState_RestartingRace;
		m_RandomNumberTracker.Reset();
	}

	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() == E_LoadState_IntoRace)
	{
		if(!quickRestart)
		{
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == E_GameMode_SinglePlayer)
			{
				//Game::C_InGameDialogueManager::Instance().InitRaceInThread();
			}
		}
	}

	Advertising::EnterZone(quickRestart,	C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity]->tag.string, 
											levelState.mRouteNumber,
											levelState.mNumViewports == 1);

	{
		LOADING_PERF("DestroyPostProcessBuffer");

		//[PK] this stores how many viewports we need
		
		mRumbleEnded=true;
	}

#ifndef CFG_SPEED
	// capture actor info before we wipe them
	Actor::C_ActorInfos actorInfos;	
#endif

	if (!quickRestart)
	{
		{
			LOADING_PERF("LoadLevelData");

			const Sys::C_FilePath levelFilename = C_Game::GetInstance()->GetLevelState().m_LevelFilename;

			m_LevelHandle.Load( levelFilename );	
			m_pLevelResource = m_LevelHandle;

			{
				LOADING_PERF("WaitForLevelResource");
				// Waiting for resource data is not ideal
				Resource::WaitForResourceData(m_pLevelResource);
			}
		}

		{
			LOADING_PERF("CreateGameWorld");

			m_pGameWorld = new C_GameWorld();
			m_pGameWorld->Create( Game::C_Game::GetInstance()->GetActorFactory(), levelState.mNumViewports );
		}
	}
	else
	{
		m_pGameWorld->Reset();
#ifndef CFG_SPEED
		::Actors::C_ActorUtils::Instance().CheckForLeftOverPhysicsBodies(m_pGameWorld->GetPhysicsScene(), actorInfos, *C_Game::GetInstance()->GetActorFactory());
#endif
		//m_pGameWorld->AddSubscribers();
		//AIAmax::AmaxAIR::Instance().Initialise(&C_Game::GetInstance()->GetSubscriberSystem());

		// Free up old Pack file
		if (m_LevelActorCreationPack)
		{
			XtPackClose(C_GameApp::GetInstance()->GetGlobalXtDatabase(), m_LevelActorCreationPack);
			m_LevelActorCreationPack = NULL;
		}
	}

	for (U32 i = 0; i < levelState.mNumViewports; i++)		
		AmaxCam::CameraSystem().DebugFeedShow(i, AmaxCam::E_DebugCam_GameFeed, i);

	#ifndef CFG_GOLD
	m_pGameDebug->Init( GetGameWorld()->GetLevelSceneManager(), &GetGameWorld()->GetPhysicsExternalConstants() );
	#endif

	{
		LOADING_PERF("InitOuttro");

		RaceIntroOutro::Instance().Init();
	}

	if (!quickRestart)
	{
		MEMORY_TRACKER_PUSH("application.amaxworldlayer.renderers");
		LOADING_PERF("CreateRenderers");
		Display::Device()->CreateRenderers();
		Display::Device()->CreateRenderersVolatile();
		MEMORY_TRACKER_POP("application.amaxworldlayer.renderers");
	}

	if (!quickRestart)
		GameResource::EffectVariations::Instance().SetVariation("_LN");


	{
		LOADING_PERF("LoadAIData");

		// intialise the AI
		//AIAmax::AmaxAIR::Instance().Initialise(&C_Game::GetInstance()->GetSubscriberSystem());
		
		// load AI track resources
		AIAmax::AmaxAIR::Instance().LoadAICityData(C_Game::GetInstance()->GetLevelState().m_LevelAIFilename);
		AIAmax::AmaxAIR::Instance().LoadAIEventData(C_Game::GetInstance()->GetLevelState().m_LevelEventAIFilename);

	}

	if (!quickRestart)
	{
		{
			LOADING_PERF("WaitForFxResourceBag");
//			m_pGameWorld->WaitForFxResourceBag();
		}

		{
			LOADING_PERF("InstantiateLevel");
			InstantiateLevel();
		}

#ifndef CFG_GOLD
		if (m_pLevelResource->GetRenderLevel() == NULL)
		{
			Automation::SendProblemFoundMsg("LEVEL", "NORENMESH");
		}

		if ((m_pLevelResource->GetPhysicsLevel() == NULL) || (m_pLevelResource->GetPhysicsLevel()->GetMainImportScene() == NULL))
		{
			Automation::SendProblemFoundMsg("LEVEL", "NOCOLMSH");
		}
#endif
	
		MEMORY_TRACKER_PUSH("graphics.vehicle");
		Vehicle::C_VehicleModelManager::Instance().Create();
		Vehicle::C_VehicleModelManager::Instance().Initialise();
		MEMORY_TRACKER_POP("graphics.vehicle");
	}
	else
	{
		const U32 routeNumber = C_Game::GetInstance()->GetLevelState().mRouteNumber;
		Collision::C_CollisionFilter filter; // filter(0) when fix through from trunk...
		filter.SetBodyFlags(Game::PolyFlags::kbWorld);
		m_pLevelInstance = new GameResource::C_LevelInstance( GetGameWorld()->GetSceneGraph(), GetGameWorld()->GetLevelSceneManager(), m_LevelHandle, 1 << routeNumber, filter);
		SceneGraph::Insert(GetGameWorld()->GetSceneGraph(), m_pLevelInstance);
		::Actors::C_ActorUtils::Instance().SetLevelInstancePtr( m_pLevelInstance );

		if (GetGameWorld()->GetCrowdSystem() && GetGameWorld()->GetCrowdSystem()->Deactivated())
			GetGameWorld()->GetCrowdSystem()->Reactivate(GetGameWorld()->GetActorList());
	}

	Resource::C_Manager::GetInstance()->PauseLoadingThread();
	
	// tell audio system to only create one audio object per frame so the audio doesnt frame out and cause glitches
	if(C_GameApp::GetInstance()->GetGameAudioLayer() && C_GameApp::GetInstance()->GetGameAudioLayer()->GetSystem())
		C_GameApp::GetInstance()->GetGameAudioLayer()->GetSystem()->SetStaggeredInstanceCreation(true);

	#ifdef AUDIO_DEFERRED_LOADING
	Audio::C_DeferredLoadManager::Get().SetDeferredLoading(true);
	#endif

	if (!quickRestart)
		gAudioDynObjLevelEntities.Clear();

	{
		LOADING_PERF("CreateRLC");

		//Create the RaceLogicCore actor
		mRLCReady = false;
		Actor::C_ActorCreationDataFactoryHelper<Actors::RaceLogicCoreActorCreate> pRLCActorCreate;
		pRLCActorCreate->m_pGameWorld = m_pGameWorld;

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->Spawn(pRLCActorCreate.Get());
		const Actor::C_Actor* pRLCActor = m_pGameWorld->GetActorList()->GetActorPtr(actorHandle);
		//CA - I know I'm casting a const pointer to a non-const pointer...aren't I bad? Mwah hahahahaha!!! Ooooo spank me!
		m_pRLC = (Game::Actors::RaceLogicCore*)Actor::ActorCast< Game::Actors::RaceLogicCore> (pRLCActor);


		#ifdef AUDIO_DEFERRED_LOADING
		// build list of all audio resources needed for vehicles
		if(C_GameApp::GetInstance()->GetGameAudioLayer() && C_GameApp::GetInstance()->GetGameAudioLayer()->GetSystem())
			C_GameApp::GetInstance()->GetGameAudioLayer()->VehicleResourcePrePass();	

		// load all audio resources for vehicles in alphabetical order (in 2 passes. bafs first then dats)
		Audio::C_DeferredLoadManager::Get().Load();
		Audio::C_DeferredLoadManager::Get().SetDeferredLoading(false);
		#endif

		if (m_pRLC)
		{
			if (!levelState.mInUI)
				m_pRLC->SpawnVehicleActors(true);		
			else if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() == Game::E_LoadState_BackToUI)
				m_pRLC->StartVehicleLoad(NULL);
		}

		#ifdef AUDIO_DEFERRED_LOADING
		Audio::C_DeferredLoadManager::Get().DecrementRefCounts();
		#endif
	}

	{
		LOADING_PERF("LoadActors");
		MEMORY_TRACKER_PUSH("game.actorstudio");
		//Load the actors created in actor studio
		LoadActorStudioActors();					
		MEMORY_TRACKER_POP("game.actorstudio");
	}

	{
		LOADING_PERF("LoadResources");

		//Tells all the systems to load their data, should be done at the end of the create
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_LoadResources );
		GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_LoadResources );
	}

	Resource::C_Manager::GetInstance()->ResumeLoadingThread();
	Resource::C_Manager::GetInstance()->WaitForAllResourceData();

	// block until all actors are created
	{
		LOADING_PERF("BlockUntilActorsCreated");
		while( !m_pGameWorld->GetActorList()->UpdatePendingActors() )		
		{
			Resource::C_Manager::GetInstance()->PartialUpdate();
		}
	}

	Resource::C_Manager::GetInstance()->WaitForAllResourceData();

	if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer() ||
		(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer() && !C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->IsQuickRestart()))
		C_GameApp::GetInstance()->CacheResources();
	/*
	//this must be called at the end of the load
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceLoaded );
	_UI_MESSAGE(E_GameflowMessage_RaceLoaded);
	GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceLoaded );
	*/

#ifndef CFG_GOLD
	C_ReplayDirector::Instance().GameInit(&GetGameWorld()->GetActorList()->GetSubscriberSystem());
#endif

	C_RaceRulesManager::Instance().Init();

	if (levelState.mInUI)
	{
		LOADING_PERF("FillStreamer");

		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_FillStreamer );
	}
	else
	{
		mCanRender = false;
	}	

	MEMORY_TRACKER_POP("application.amaxworldlayer");

	#ifdef ENABLE_LOAD_PERF
	LoadPerf::Pop("C_AmaxWorldLayer::Create()");
	#endif


	#ifdef ENABLE_LOAD_PERF
	if (!levelState.mInUI)
	{
		LoadPerf::Push("LoadDuringUpdate");
	}
	else
	{
		if (LoadPerf::ProfilingStarted())
		{
			LoadPerf::Pop();
		}
	}
	#endif

	RenderingData::Target::StreamManager::Instance().SetMemoryLimit(INT_MAX);

	if (!quickRestart)
		gAudioDynObjLevelEntities.IncrementRefCounts();

	if(C_GameApp::GetInstance()->GetGameAudioLayer() && C_GameApp::GetInstance()->GetGameAudioLayer()->GetSystem())
		C_GameApp::GetInstance()->GetGameAudioLayer()->GetSystem()->SetStaggeredInstanceCreation(false);


#ifdef DISABLE_RENDERMUTEX_TO_PREVENT_UI_STALLS
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() == Game::E_LoadState_IntoRace)
	{
		C_GameApp::GetInstance()->GetSystemLayer()->m_UseRenderMutex = true;
	}
	else
	{
		C_GameApp::GetInstance()->GetSystemLayer()->m_UseRenderMutex = false;
	}
#endif

	return S_OK;
}

void C_AmaxWorldLayer::LoadAllIntoStreamer()
{
	MEMORY_TRACKER_PUSH("application.streamer");
	m_pGameWorld->GetLevelSceneManager()->LoadAllRequested();
	MEMORY_TRACKER_POP("application.streamer");
}

void C_AmaxWorldLayer::LoadActorStudioActors()
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	//Get the city we're loading
	AmaxCity *pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mCity];
	//...and the route
	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	//...and the event
	AmaxEvent *pEvent  = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mEvent];

	assert(pCity);
	assert(pRoute);

	if (!pCity || !pRoute)
		return;

	Util::C_String fullPath = C_Game::GetInstance()->GetLevelState().m_LevelActorStudioFilename;

	C_LevelActorCreationDataList* pLevelActorCreationDataList = new AmaxWorldLayer_Impl::C_AmaxLevelActorsCreate();

	Util::C_String pakFileName = fullPath + Util::C_String("\\actorpack.xt");	// new pack filename

	//Load Pak file
	if (fs.FileExists(pakFileName.Chars()))
	{
		Sys::I_File* pFile = fs.Open( pakFileName.Chars() );
		if ( pFile )
		{
			U32 length = (U32)pFile->Length();
			U8* pData = new U8[ length ];

			if ( pData )
			{
				if ( pFile->Read( pData, length ) )
				{
					XtSchema* pXtSchema = XtGetApplicationSchema();

					if ( pXtSchema )
					{
						XtPackOpenParams oo;

						oo.callback = AmaxWorldPackOpenCallback;
						oo.context = 0;

						m_LevelActorCreationPack = XtPackOpen( C_GameApp::GetInstance()->GetGlobalXtDatabase(), pData, length, &oo );
					}				
				}

				delete [] pData;
			}

			delete pFile;
		}
	}

	::Actors::C_ActorUtils::Instance().SanityCheck_Initialise();		// Prepare to check dynamic object positions etc

	bool isMultiplayer = false;
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())
		isMultiplayer = (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer() || g_DebugForceLoadMultiplayerActorLists);

	if (!g_DebugDisableAllActorStudioActors)							// Option to view a level with no actors
	{		
		// loop through city def actor list array

		//Now go through the city file and load all of the actor lists for this city
		//Load multiplayer actor lists
		if (isMultiplayer && pCity->multiplayerActorLists.count > 0)
		{
			for(int i = 0; i < pCity->multiplayerActorLists.count; i++)
				pLevelActorCreationDataList->AddActorListFromRecord(pCity->multiplayerActorLists[i]);
		}
		else
		{
			for(int i = 0; i < pCity->actorLists.count; i++)
				pLevelActorCreationDataList->AddActorListFromRecord(pCity->actorLists[i]);
		}

		// Only add route actor lists if we haven't overriden them in the event.
		if( pEvent == NULL || (pEvent && ( !pEvent->actorLists.count || pEvent->actorLists[0] == NULL ) ) )
		{			
			//Load multiplayer actor lists
			if (isMultiplayer && pRoute->multiplayerActorLists.count > 0)
			{
				for(int i = 0; i < pRoute->multiplayerActorLists.count; i++)
					pLevelActorCreationDataList->AddActorListFromRecord(pRoute->multiplayerActorLists[i]);
			}
			else
			{
				if (isMultiplayer)
				{
					LOGWARN(g_Log, "############################################################################");
					LOGWARN(g_Log, "# Route has no multiplayer actor lists specified! Using singleplayer setup #");
					LOGWARN(g_Log, "############################################################################");					
				}
				if (pRoute->multiplayerActorLists.count == 0)
					Automation::SendProblemFoundMsg("LEVEL", "NoMpActorLists");

#ifndef CFG_GOLD
				if ((g_debugSuperSpecialCutSceneVideoMode) && (pRoute->cutsceneActorLists.count > 0))
				{
					for(int i = 0; i < pRoute->cutsceneActorLists.count; i++)
						pLevelActorCreationDataList->AddActorListFromRecord(pRoute->cutsceneActorLists[i]);
				}
				else
				{
#endif
					//Loop through route def actor list array
					for(int j = 0; j < pRoute->actorLists.count; j++)
						pLevelActorCreationDataList->AddActorListFromRecord(pRoute->actorLists[j]);
#ifndef CFG_GOLD
				}
#endif
			}
		}



		//Now go through the event file and load all of the actor lists for this event
		if(pEvent)
		{
			for(int i = 0; i < pEvent->actorLists.count; i++)			// Used for event specific stuff like special pickups
				pLevelActorCreationDataList->AddActorListFromRecord(pEvent->actorLists[i]);
		}
	}

	m_pGameWorld->GetActorList()->CreateActors(pLevelActorCreationDataList);

	if (isMultiplayer && !g_DebugDisableAllActorStudioActors)
	{	
		// Only for playlists
		if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetPlayMode() == E_PlayMode_Playlist )
		{
			const MultiplayerEvent* pMPEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
			C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
			if (pMPEvent && levelState.mMpRaceIndex >= 0 && levelState.mMpRaceIndex < pMPEvent->races.count)
			{
				const MultiplayerRace* pMPRace = pMPEvent->races[levelState.mMpRaceIndex];
				if (pMPRace && pMPRace->powerupActorListOverride)
				{
					m_pGameWorld->GetActorList()->BroadcastMessage(Game::E_PickupMessage_KillAllPickups);
					C_LevelActorCreationDataList* pLevelActorCreationDataListOverride = new AmaxWorldLayer_Impl::C_AmaxLevelActorsCreate();
					pLevelActorCreationDataListOverride->AddActorListFromRecord(pMPRace->powerupActorListOverride);
					m_pGameWorld->GetActorList()->CreateActors(pLevelActorCreationDataListOverride);
				}
			}
		}
	}

	m_pActorStudioMessaging = &ActorStudioMsgGame::C_ActorStudioMessaging::Instance();
	m_pActorStudioMessaging->SetActorList(m_pGameWorld->GetActorList());
	m_pActorStudioMessaging->SetActorPack(m_LevelActorCreationPack);
	m_pActorStudioMessaging->Init();

	::Actors::C_ActorUtils::Instance().SanityCheck_DynamicObjects();	// Check if any dynamic objects are in stupid positions etc
}

void C_AmaxWorldLayer::PrepareOutroCutScene()
{
/*
	if (!GetRaceLogicCore())
		return;

	AmaxCity *pCity		= C_GameData::Instance().mCities[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mCity];
	AmaxRoute *pRoute	= C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	AmaxEvent *pEvent	= C_GameEventData::Instance().mEvents[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mEvent];
	
	BIZ_ASSERT(pCity);
	BIZ_ASSERT(pRoute);
	BIZ_ASSERT(pEvent);

	if (!pCity || !pRoute || !pEvent)
		return;

	BIZ_ASSERT(pEvent->m_PassedCutScene);

	AmaxCutScene* pCutScene = pEvent->m_PassedCutScene;

	if (!pCutScene)
		return;

	//All events have a dummy outro cut-scene, so using this to identify if it's actually a valid outro cut-scene
	if (pCutScene->m_Radius == 0.0f)
	{
		LoadOutroCutsceneActors();
		return;
	}

	//Get budgets
#if defined(PLATFORM_XENON)
	S32 sysMemBudget			= (S32) (pCutScene->m_SystemMemorySize.platform360 * 1024*1024);
	S32 gfxMemBudget			= (S32) (pCutScene->m_GraphicsMemorySize.platform360 * 1024*1024);
#elif defined(PLATFORM_PS3)
	S32 sysMemBudget			= (S32) (pCutScene->m_SystemMemorySize.platformPS3 * 1024*1024);
	S32 gfxMemBudget			= (S32) (pCutScene->m_GraphicsMemorySize.platformPS3 * 1024*1024);
#elif defined(PLATFORM_PC)	
	S32 sysMemBudget			= (S32) (pCutScene->m_SystemMemorySize.platformPC * 1024*1024);
	S32 gfxMemBudget			= (S32) (pCutScene->m_GraphicsMemorySize.platformPC * 1024*1024);
#endif

	//Initiate a streamer purge to get back the memory we need
	StartStreamerPurge(gfxMemBudget);

	//Kill off any dynamic/physical objects that are not needed for the outro
	Game::C_GameActorList* pActorList = GetGameWorld()->GetActorList();
	
	Game::C_KillForOutroMsg killMsg(Maths::HVec3(pCutScene->m_Origin.vx, pCutScene->m_Origin.vy, pCutScene->m_Origin.vz), pCutScene->m_Radius);
	pActorList->BroadcastMessage(&killMsg);

	//Kill of any vehicles that aren't in the cut-scene
	for (S32 iVehicle = 0; iVehicle < pCutScene->m_VehiclesToUnload.count; iVehicle++)
	{
		Game::Actors::C_VehicleActor* pVehicleActor = GetRaceLogicCore()->GetVehicleActor(pCutScene->m_VehiclesToUnload[iVehicle]);
		if (!pVehicleActor)
			continue;
		pVehicleActor->OnlyUnloadResources();
		C_DriverManager::Instance()->DestroyDriver(iVehicle);
		pActorList->SendMessageToActor(pVehicleActor->GetHandle(), ActorMsg::E_ResourceMsg_UnloadAll);
	}

	m_CutSceneState = E_CutSceneState_OutroPrepping;
*/
}



void C_AmaxWorldLayer::LoadOutroCutsceneActors()
{
/*
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	//Get the city we're loading
	AmaxCity *pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mCity];
	//...and the route
	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	//...and the event
	AmaxEvent *pEvent  = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mEvent];

	assert(pCity);
	assert(pRoute);

	if (!pCity || !pRoute)
		return;

	C_LevelActorCreationDataList* pLevelActorCreationDataList = new AmaxWorldLayer_Impl::C_AmaxLevelActorsCreate();

	if (!g_DebugDisableAllActorStudioActors)							// Option to view a level with no actors
	{
		if(pEvent)
		{
			AddCutsceneActors(pLevelActorCreationDataList, pEvent->m_PassedCutScene);
			AddCutsceneActors(pLevelActorCreationDataList, pEvent->m_PassedScriptedCamera);
			AddCutsceneActors(pLevelActorCreationDataList, pEvent->m_FailedScriptedCamera);
		}
	}

	m_pGameWorld->GetActorList()->CreateActors(pLevelActorCreationDataList);

	LOADING_PERF("BlockUntilActorsCreated");
	while( !m_pGameWorld->GetActorList()->UpdatePendingActors() )
	{
		Resource::C_Manager::GetInstance()->PartialUpdate();
	}

	m_CutSceneState = E_CutSceneState_OutroPrepped;

	RenderingData::Target::StreamManager::Instance().SetMemoryLimit(INT_MAX);
*/
}


void C_AmaxWorldLayer::AddCutsceneActors(C_LevelActorCreationDataList* list, AmaxCutScene *acs)
{
/*
	if (acs)
	{
		if (acs->m_UseSetupList)
			list->AddActorListFromRecord(acs->m_Setup_ActorList);
		else
			list->AddActorListFromRecord(acs->m_Scene_ActorList);
		list->AddActorListFromRecord(acs->m_Audio_ActorList);
	}
*/
}

void C_AmaxWorldLayer::NotifyActorStudioDataCreated()
{
	// this gets call from the dtor of the wrapper object that we use to create the actor studio data
	GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_GameMsg_PostCreateInit );
	
	mActorsLoaded=true;
}

class C_WaitForEvictListener : public Resource::C_Listener
{
public:
	// inherited from Resource::C_Listener
	void ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
	{
		switch( result )
		{
		case Resource::E_Notify_NoData:
			m_Event.Signal();
			break;
		default:
			break;
		}
	}

	void WaitForEvict()
	{
		while(!m_Event.Wait(100))
		{
			Resource::C_Manager::GetInstance()->PartialUpdate();
		}
	}

private:
	MT::Event m_Event;
};

HRESULT C_AmaxWorldLayer::Destroy(bool quickRestart)
{	
	if (!quickRestart)
	{
		UnloadUILutRamPak();
		Advertising::ExitZone(false);
		Advertising::TidyZone();
		C_GameApp::GetInstance()->ReleaseCachedResources();
		m_RandomNumberTracker.Clear();
	}
	else
	{
		m_RandomNumberTracker.Reset();
		Advertising::RestartZone();
	}

	Display::Device()->AcquireThreadOwnership();

#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif
	Display::Device()->ReleaseThreadOwnership();

	#ifndef CFG_GOLD
	m_pGameDebug->Close();
	#endif

	// get rid of debug menu
	#ifndef CFG_GOLD
	DebugSystems::C_DebugHUDMenu::GetInstance()->Active( false );
	#endif
	
	#ifndef CFG_GOLD
	C_ReplayDirector::Instance().GameEnd();
	#endif

	// Kill off the SFX manager stuff.
	Game::C_InGameSpecialFXManager::Instance().ClearFXStore();

	GetLevelResource()->GetRenderLevel()->GetScene().GetStreamData().ClearRequests();

	SceneGraph::Remove(GetGameWorld()->GetSceneGraph(), m_pLevelInstance);
	SAFE_DELETE_PTR( m_pLevelInstance );

	if (!quickRestart)
	{
		// set up listener to wait for level to be evicted
		C_WaitForEvictListener listener;
		m_pLevelResource->AddListener( &listener, NULL );

		// release level resource
		m_LevelHandle = NULL;
		m_pLevelResource = NULL;

		// wait for resource to be evicted
		listener.WaitForEvict();

		m_pGameWorld->Destroy();
		SAFE_DELETE_PTR( m_pGameWorld );

		// Free up old Pack file
		if (m_LevelActorCreationPack)
		{
			XtPackClose(C_GameApp::GetInstance()->GetGlobalXtDatabase(), m_LevelActorCreationPack);
			m_LevelActorCreationPack = NULL;
		}
	}

	mRLCReady = false;
	mCanRender = false;
	mAIReady = false;
	mActorsLoaded = false;

	if (m_pActorStudioMessaging)
		m_pActorStudioMessaging->Exit();
	m_pActorStudioMessaging = NULL;

	if (!quickRestart)
	{
		Vehicle::C_VehicleModelManager::Instance().Destroy();

		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_LevelDumped );
		_UI_MESSAGE(E_GameflowMessage_LevelDumped);
	}

	#if PLATFORM_PS3
	if(!quickRestart)
		Vehicle::C_VehicleVertexBufferManager::GetInstance().DestroyVertexBuffers();
	#endif

	if (!quickRestart)
	{
		gAudioDynObjLevelEntities.DecrementRefCounts();
		gAudioDynObjLevelEntities.Clear();
	}

	return S_OK;
}

Debug::Bool g_debugStreamerBlockLoad("Amax.StreamerBlockLoad", false);

void C_AmaxWorldLayer::InstantiateLevel()
{
	const Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	const U32 routeNumber = levelState.mRouteNumber;
	if (GetGameWorld() && GetGameWorld()->GetCrowdSystem() && m_pLevelResource->GetRenderLevel())
	{
		const AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
		const AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];

		GetGameWorld()->GetCrowdSystem()->Create(GetGameWorld()->GetSceneGraph(), GetGameWorld()->GetActorList(), &m_pLevelResource->GetRenderLevel()->GetCrowdData(),
			1 << routeNumber, ((pRoute != NULL) ? &*pRoute->CrowdPopulation : NULL), (Game::E_ViewTypeFlag_Main | Game::E_ViewTypeFlag_Mirror));

		if (pCity)
			GetGameWorld()->GetCrowdSystem()->AddTextureList(&*pCity->crowdFlagTextures);
	}
	
	if (!m_pLevelResource->GetRenderLevel()) return;
	
	m_pLevelInstance = new GameResource::C_LevelInstance( GetGameWorld()->GetSceneGraph(), GetGameWorld()->GetLevelSceneManager(), m_LevelHandle, 1 << routeNumber );
	SceneGraph::Insert(GetGameWorld()->GetSceneGraph(), m_pLevelInstance);
	m_pLevelResource = m_LevelHandle;
	GetGameWorld()->SetSunDirection( m_pLevelResource->GetRenderLevel()->GetLightList().GetSunDirection() );			

	::Actors::C_ActorUtils::Instance().SetLevelInstancePtr( m_pLevelInstance );

#ifndef CFG_GOLD
	m_pGameDebug->InitPlatformDebug( m_pLevelResource->GetModelData(), m_pLevelResource->GetRenderModelData() );
#endif

	if (GetGameWorld())
	{
		GetGameWorld()->m_waitForStreamer = !g_debugStreamerBlockLoad;
		if (C_Game::GetInstance()->GetLevelState().mInUI)
		{
			if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetFrontEndLoading() &&
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() != Game::E_LoadState_IntoRace)
			{				
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StartGameRendering );
				C_GameApp::GetInstance()->DisableLoadingScreen();
			}
//			GetGameWorld()->GetActorList()->BroadcastMessage(ActorMsg::E_CutSceneMsg_Start_ScriptedCamera);
//			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_Start_ScriptedCamera );			
		}
	}
}

/******************************************************************************\

			Quick Hack for Designers to Play with Grav Gun and Perks

 			 (it will be moved to Sorcery if they like it this way)

\******************************************************************************/

void PlayBackGravGunSetting()
{
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if(pRLC->GetVehicleActor(g_PerkGravGunSlot_PlayBack_VehicleIndex) == NULL)
		return;

	Gravity::C_Body* pBody = pRLC->GetVehicleActor(g_PerkGravGunSlot_PlayBack_VehicleIndex)->GetCollisionBody();

	if(g_PerkGravGunSlot1_PlayBack)
	{
		g_PerkGravGunSlot1_PlayBack=false;

		pBody->ApplyForce(	Maths::HVec3((float)g_PerkGravGunSlot1_ForceX,(float)g_PerkGravGunSlot1_ForceY,(float)g_PerkGravGunSlot1_ForceZ) * pBody->ElementMatrix().Rotation(), 
							Maths::HVec3((float)g_PerkGravGunSlot1_PosX,(float)g_PerkGravGunSlot1_PosY,(float)g_PerkGravGunSlot1_PosZ) * pBody->ElementMatrix());
	}

	if(g_PerkGravGunSlot2_PlayBack)
	{
		g_PerkGravGunSlot2_PlayBack=false;
		pBody->ApplyForce( Maths::HVec3((float)g_PerkGravGunSlot2_ForceX,(float)g_PerkGravGunSlot2_ForceY,(float)g_PerkGravGunSlot2_ForceZ) * pBody->ElementMatrix().Rotation(), 
							Maths::HVec3((float)g_PerkGravGunSlot2_PosX,(float)g_PerkGravGunSlot2_PosY,(float)g_PerkGravGunSlot2_PosZ) * pBody->ElementMatrix());
	}

	if(g_PerkGravGunSlot3_PlayBack)
	{
		g_PerkGravGunSlot3_PlayBack=false;
		pBody->ApplyForce( Maths::HVec3((float)g_PerkGravGunSlot3_ForceX,(float)g_PerkGravGunSlot3_ForceY,(float)g_PerkGravGunSlot3_ForceZ) * pBody->ElementMatrix().Rotation(), 
							Maths::HVec3((float)g_PerkGravGunSlot3_PosX,(float)g_PerkGravGunSlot3_PosY,(float)g_PerkGravGunSlot3_PosZ) * pBody->ElementMatrix());
	}

	if(g_PerkGravGunSlot4_PlayBack)
	{
		g_PerkGravGunSlot4_PlayBack=false;
		pBody->ApplyForce( Maths::HVec3((float)g_PerkGravGunSlot4_ForceX,(float)g_PerkGravGunSlot4_ForceY,(float)g_PerkGravGunSlot4_ForceZ) * pBody->ElementMatrix().Rotation(), 
							Maths::HVec3((float)g_PerkGravGunSlot4_PosX,(float)g_PerkGravGunSlot4_PosY,(float)g_PerkGravGunSlot4_PosZ) * pBody->ElementMatrix());
	}

	if(g_PerkGravGunSlot5_PlayBack)
	{
		g_PerkGravGunSlot5_PlayBack=false;
		pBody->ApplyForce( Maths::HVec3((float)g_PerkGravGunSlot5_ForceX,(float)g_PerkGravGunSlot5_ForceY,(float)g_PerkGravGunSlot5_ForceZ) * pBody->ElementMatrix().Rotation(), 
							Maths::HVec3((float)g_PerkGravGunSlot5_PosX,(float)g_PerkGravGunSlot5_PosY,(float)g_PerkGravGunSlot5_PosZ) * pBody->ElementMatrix());
	}
}

void StoreGravGunSetting(Maths::HVec3 force,Maths::HVec3 pos)
{
	if(!g_PerkGravGunSlot_Record)
		return;

	switch(g_PerkGravGunSlot)
	{
		case 1:
			g_PerkGravGunSlot1_ForceX=force.X();
			g_PerkGravGunSlot1_ForceY=force.Y();
			g_PerkGravGunSlot1_ForceZ=force.Z();

			g_PerkGravGunSlot1_PosX=pos.X();
			g_PerkGravGunSlot1_PosY=pos.Y();
			g_PerkGravGunSlot1_PosZ=pos.Z();
			break;
		case 2:
			g_PerkGravGunSlot2_ForceX=force.X();
			g_PerkGravGunSlot2_ForceY=force.Y();
			g_PerkGravGunSlot2_ForceZ=force.Z();

			g_PerkGravGunSlot2_PosX=pos.X();
			g_PerkGravGunSlot2_PosY=pos.Y();
			g_PerkGravGunSlot2_PosZ=pos.Z();
			break;
		case 3:
			g_PerkGravGunSlot3_ForceX=force.X();
			g_PerkGravGunSlot3_ForceY=force.Y();
			g_PerkGravGunSlot3_ForceZ=force.Z();

			g_PerkGravGunSlot3_PosX=pos.X();
			g_PerkGravGunSlot3_PosY=pos.Y();
			g_PerkGravGunSlot3_PosZ=pos.Z();
			break;
		case 4:
			g_PerkGravGunSlot4_ForceX=force.X();
			g_PerkGravGunSlot4_ForceY=force.Y();
			g_PerkGravGunSlot4_ForceZ=force.Z();

			g_PerkGravGunSlot4_PosX=pos.X();
			g_PerkGravGunSlot4_PosY=pos.Y();
			g_PerkGravGunSlot4_PosZ=pos.Z();
			break;
		case 5:
			g_PerkGravGunSlot5_ForceX=force.X();
			g_PerkGravGunSlot5_ForceY=force.Y();
			g_PerkGravGunSlot5_ForceZ=force.Z();

			g_PerkGravGunSlot5_PosX=pos.X();
			g_PerkGravGunSlot5_PosY=pos.Y();
			g_PerkGravGunSlot5_PosZ=pos.Z();
			break;
		default:
			break;
	}
}

/******************************************************************************\


									END


\******************************************************************************/





Debug::Float g_DebugPushForceMagAmax("Amax.PushForceMag", 100000.0f);

Debug::Bool g_CollideRaytraceAmax("Amax.Test.CollideRaytrace", true);
Debug::Bool g_CollideRaytraceVsShootableAmax("Amax.Test.CollideRaytraceVsShootable", true);
Debug::Bool g_CollideRaytraceVsAllAmax("Amax.Test.CollideRaytraceVsAll", true);
Debug::Bool g_CollideRaytraceResultsAmax("Amax.Test.CollideRaytraceResults", false, Debug::Persist());

void DebugCollideRaytrace( Gravity::C_Scene* pScene, DbUtil::I_DebugRender* pDebugRender  )
{
	if(!g_CollideRaytraceAmax)
		return;

	Collision::C_Ray ray;

	U32 filter = 0;

	if(g_CollideRaytraceVsAllAmax) filter |= 0xffffffff;
	if(g_CollideRaytraceVsShootableAmax) filter |= PolyFlags::kShootable;

	Collision::C_CollisionFilter rayCollisionFilter(0xffffffff, filter);

	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

	Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

	Maths::HVec3 start = Maths::HVec3( 0.2f, -0.2f, 0.0f ) * cameraMatrix;
	Maths::HVec3 end = Maths::HVec3( 0.2f, -0.2f, 30.0f ) * cameraMatrix;

	ray.InitialiseRay( start, end, &rayCollisionFilter );

	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(data, 1024);
	pScene->DoRayTrace( ray, stack );

	if(ray.m_result.m_pShape)
	{
		pDebugRender->Draw3dLine( ray.m_start, ray.m_end, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );

		if(g_CollideRaytraceResultsAmax)
		{
			Util::C_String surfaceString;
			surfaceString.Printf("Material:%s|%s\n0x%08x\n%s", GetEffectMaterialName( (E_EffectMaterial)ray.m_result.GetEffectMaterial() ).Chars(), GetPhysicsMaterialName( (E_PhysicsMaterial)ray.m_result.GetPhysicsMaterial() ).Chars(),ray.m_result.m_CollisionFlags, PolyFlags::GetCollideFlagsName(ray.m_result.m_CollisionFlags).Chars() );
			pDebugRender->Draw3dText( ray.m_end, surfaceString );
		}


		Gravity::C_Body* pBody = NULL;
		
		const SharedPhysics::C_GraphNode* pGraphNode = ray.m_result.GetGraphNode();
		if(pGraphNode)
		{
			pBody = pGraphNode->m_pBody;
		}

		if(pBody)
		{
			pDebugRender->Draw3dArrow( ray.m_end, ray.m_result.m_normal, 0.5f, 0.2f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );
			pDebugRender->Draw3dPlaneStar( ray.m_end + ray.m_result.m_normal * 0.01f, ray.m_result.m_normal, 0.25f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );

			F32 forceMag = Input::GetGameInput(0)->GetButtonValue( Framework::E_Button_Debug_ButtonRightTrigger )
						 - Input::GetGameInput(0)->GetButtonValue( Framework::E_Button_Debug_ButtonLeftTrigger );

			Maths::HVec3 forceDir = (end - start).Normalised().Normal();

			if( fabs(forceMag) > 0.2f )
			{
				forceMag = forceMag * g_DebugPushForceMagAmax;
				if(!pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
				{
					pBody->Activate();
					pBody->ApplyForce( forceDir * forceMag, ray.m_end );
				}
			}

			if (Input::GetGameInput(0)->GetButtonPressedDebounced( Framework::E_Button_Debug_ButtonX ))
			{
				if(!pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
				{
					pBody->Activate();
					pBody->ApplyForce( forceDir * g_DebugPushForceMagAmax, ray.m_end );

					StoreGravGunSetting( (forceDir * g_DebugPushForceMagAmax) * pBody->ElementMatrix().Rotation().Inverse(), ray.m_end * pBody->ElementMatrix().InverseRotTrans());
				}
			}

			if (Input::GetGameInput(0)->GetButtonPressedDebounced( Framework::E_Button_Debug_ButtonB ))
			{
				if (C_GameApp::GetInstance()->GetRaceBookLayer() && 
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld() &&
					pBody)
				{
					C_BodyUserDataBase* pUserData = (C_BodyUserDataBase*)pBody->GetUserData<void>();
					if (pUserData && pUserData->m_eType == E_BodyUserDataType_Actor)
					{
						const Actor::C_Actor* pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(((C_BodyUserDataActor*)pUserData)->m_hActorHandle);
						if (pActor && pActor->GetActorTypeId() == E_ActorTypeId_Vehicle)
						{					
							Game::Actors::C_VehicleActor* pVehicle = const_cast<Game::Actors::C_VehicleActor*>((Game::Actors::C_VehicleActor*)pActor);
							Actors::C_VehicleDebugDamageMsg msg(ray.m_end, forceDir);
							pVehicle->SendMessageToActor(pVehicle->GetHandle(), &msg);
						}
					}
				}
			}
		}
		else
		{
			pDebugRender->Draw3dArrow( ray.m_end, ray.m_result.m_normal, 0.5f, 0.2f, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
			pDebugRender->Draw3dPlaneStar( ray.m_end + ray.m_result.m_normal * 0.01f, ray.m_result.m_normal, 0.25f, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
		}
	}
	else
	{
		pDebugRender->Draw3dLine( ray.m_start, ray.m_end, Maths::RGBA(0xff, 0x00, 0x00, 0xff) );
	}
}

void C_AmaxWorldLayer::ExitSloMo(const int viewportIndex)
{
	AmaxCam::CameraSystem().StopSetPieceCamera();
	AmaxCam::CameraSystem().ResetVehicleFeed(viewportIndex);

	C_GameApp::GetInstance()->SetTimeScale(1.0f);

	ActorMsg::C_VehicleSloMoExitMsg pMsg;
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&pMsg);

	if (GetGameWorld())
	{
		GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&pMsg);
		GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoExit);

	}
}

HRESULT C_AmaxWorldLayer::Update()
{
	PERF_EVENT("AmaxWorldLayer::Update");
	
	GetGameWorld()->Update();
	m_pActorStudioMessaging->Update();

	if(C_GameApp::GetInstance()->IsPaused())
		return S_OK;

	if (m_CutSceneState == E_CutSceneState_OutroPrepping)
	{
		S32 totalUsed = (S32)GetLevelResource()->GetRenderLevel()->GetScene().GetStreamData().GetTotalUsed();
		S32 memLimit = (S32)RenderingData::Target::StreamManager::Instance().GetMemoryLimit();
		if (totalUsed <= memLimit)
		{
			LoadOutroCutsceneActors();
		}
		return S_OK;
	}
	if (m_WorldState == E_WorldState_LoadedRace)
	{
		RaceIntroOutro::Instance().PostUpdate();
	}
	else if (m_WorldState == E_WorldState_LoadingRace || m_WorldState == E_WorldState_RestartingRace)
	{
		if (mRLCReady && mAIReady && mActorsLoaded)
		{
			//GetRaceLogicCore()->m_RaceLoadTime = C_GameApp::GetInstance()->GetElapsedRaceTime();
			GetRaceLogicCore()->m_RaceLoadTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceLoaded );
			_UI_MESSAGE(E_GameflowMessage_RaceLoaded);
			GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceLoaded );

			m_WorldState = E_WorldState_LoadedRace;

			// [Bone] #34942 : Set cameras to vista cam
			C_GameApp* gameApp = C_GameApp::GetInstance();
			Game::C_RaceBookLayer* rbl = gameApp ? gameApp->GetRaceBookLayer() : 0;
			Game::E_GameMode gameMode = rbl ? rbl->GetGameMode() : E_GameMode_Nothing;
			
			if (gameMode != Game::E_GameMode_Debug && !Game::C_Game::GetInstance()->GetLevelState().mInUI )
			{
				// [Bone] #34942 : Set cameras to vista cam
				ActorMsg::C_StartAnimatedCameraMsg msg;
				msg.m_Sequence = ::Actors::E_ANIMCAMERASEQ_ROUTEVISTA;
				msg.m_VehicleIndex = -1;
				msg.m_Viewport = 0;
				msg.m_StartPaused = true;
				msg.m_FutureFeed = false;
				// Game
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
				// Future
				msg.m_FutureFeed = true;
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
			}
		}
	}

#ifndef CFG_GOLD
	// TEMPORARY SPAWN
	if ( g_SpawnParticleEventTestActor )
	{
		g_SpawnParticleEventTestActor = false;
		
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();
		Actor::C_ActorCreationDataFactoryHelper<Actors::C_EffectTestActorCreate> pCreate;
	
		pCreate->m_WorldMatrix = Store(cameraMatrix);
		pCreate->m_ParticleEventIndex = g_ParticleEventIndex;

		Maths::HVec3 vCollisionPosition(0.0f, -10000.0f, 0.0f);
		Maths::C_Normal vCollisionNormal;// = Maths::C_Normal::AxisPositiveY();
		{
			Collision::C_Ray ray;
			Collision::C_CollisionFilter rayCollisionFilter = PolyFlags::kShootable;

			U32 filterFlags = 0;
			filterFlags |= PolyFlags::kVehicleSolid;
			filterFlags |= PolyFlags::kWheelSolid;
			filterFlags |= PolyFlags::kCharacterSolid;
			filterFlags |= PolyFlags::kDynamicObjectSolid;
			filterFlags |= PolyFlags::kRagdollSolid;

			rayCollisionFilter.SetCollisionMask(filterFlags);
			rayCollisionFilter.SetCollisionFlags(0xffffffff);

			Maths::HVec3 start = Maths::HVec3( 0.0f, 0.0f, 0.0f ) * cameraMatrix;
			Maths::HVec3 end = start - Maths::HVec3( 0.0f, 100.0f, 0.0f );
			ray.InitialiseRay( start, end, &rayCollisionFilter );
			m_pGameWorld->GetPhysicsScene()->DoRayTrace( ray, m_pGameWorld->GetPhysicsScene()->m_Stack, false );

			if(ray.GetResult().Hit())
			{
				vCollisionPosition = ray.GetResult().HitPoint();
				vCollisionNormal = ray.GetResult().Normal();
			}
		}

		pCreate->m_vCollisionPosition = vCollisionPosition;
		pCreate->m_vCollisionNormal = vCollisionNormal;

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->Spawn(pCreate.Get());
	}

	if (g_bPhysicsMaterialsReload)
	{
		g_bPhysicsMaterialsReload = false;
		C_PhysicsMaterials::SetupMaterials();
	}
	PlayBackGravGunSetting();
#endif

	/* [WillB] Removed Kerbs fix as he going to do it another way
	C_GameApp::GetInstance()->GetRaceBookLayer()->Update();
	*/
	return S_OK;
}

// this update doesn't get paused
HRESULT C_AmaxWorldLayer::GlobalUpdate()
{
	//GetGameWorld()->GlobalUpdate();

	bool gravGun = AmaxCam::Switcher().Choice() == AmaxCam::E_DebugCam_GravGun;
	if (gravGun)
		DebugCollideRaytrace( m_pGameWorld->GetPhysicsScene(), DbUtil::DefaultDebugRenderSys()->Deferred() );

	#ifndef CFG_GOLD
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	Maths::HMat43 mCameraToWorld = view.m_View.GetViewSettings3D().GetCameraToWorld();

	m_pGameDebug->Update( mCameraToWorld, C_GameApp::GetInstance()->IsPaused() );
	#endif

	return S_OK;
}

void C_AmaxWorldLayer::EnableRumble(bool from_paused)
{
	if(!from_paused)
		mRumbleEnded=false;

	if(mRumbleEnded)
		return;

	for(int a=0; a<Framework::C_ApplicationBase::GetInstance()->GetInputUserCount(); a++)
	{
		Input::C_Device *pad=Framework::C_ApplicationBase::GetInstance()->GetPad( a );
		
		pad->EnableRumble();
		pad->SetRumbleMotorSpeed(0,0);
		pad->SetRumbleMotorSpeed(1,0);
	}
}

void C_AmaxWorldLayer::DisableRumble(bool from_paused)
{
	if(!from_paused)
		mRumbleEnded=true;

	for(int a=0; a<Framework::C_ApplicationBase::GetInstance()->GetInputUserCount(); a++)
	{
		Input::C_Device *pad=Framework::C_ApplicationBase::GetInstance()->GetPad( a );
		
		pad->SetRumbleMotorSpeed(0,0);
		pad->SetRumbleMotorSpeed(1,0);
		pad->DisableRumble();
	}
}

extern Debug::Bool g_debugEnableQuickRestart;

bool C_AmaxWorldLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case Game::E_GameflowMessage_ClearStoredActorPtrs:
			{
				m_pRLC = NULL;
			}
			break;

		case E_GameflowMessage_RLCReadyToRock:
			{
				mRLCReady = true;
				break;
			}
		case E_GameflowMessage_Pause:
			DisableRumble(true);
			m_pRLC->SetPaused(true);
			UI::theFETimer.SetTimeScale(1.0f);
			if (!C_Game::GetInstance()->GetLevelState().mNetworkRace)
				Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Game );
			break;

		case E_GameflowMessage_UnPause:
			EnableRumble(true);
			m_pRLC->SetPaused(false);
			UI::theFETimer.SetTimeScale(C_GameApp::GetInstance()->GetTimeScale());
			if (!C_Game::GetInstance()->GetLevelState().mNetworkRace)
				Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Game );
			break;

		case E_AIMessage_AllLoaded:
		case E_AIMessage_LoadFailed:		
			{
				GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );

				mAIReady = true;
				break;
			}
		case Game::E_GameflowMessage_RaceStarted:
			{
				EnableRumble(false);
				GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceStarted );
				break;
			}
		case Game::E_GameflowMessage_StartGameRendering:
				mCanRender = true;
			break;
		case Game::E_GameflowMessage_StopGameRendering:
				mCanRender = false;
			break;
		case Game::E_GameflowMessage_StopGameUpdate:
				GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StopGameUpdate );				
			break;
		

		case Game::E_GameflowMessage_QuitAndWinRace:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_RaceFinished:
				Advertising::SuspendMakingMoney();
				DisableRumble(false);
			break;
		case Game::E_GameflowMessage_RestartRace:
			{
				DisableRumble(false);
				if (g_debugEnableQuickRestart)
				{
					m_pGameWorld->Exit();					
				}
			}
			break;
		case Game::E_GameflowMessage_RestartLevel:
			{
				m_WorldState = E_WorldState_RestartingRace;
			}
			break;		
		case Game::E_GameflowMessage_PrepareOutro:
			{
				PrepareOutroCutScene();
			}
			break;
		case Game::E_VehicleMessage_Respray:
			{
				const Game::Actors::C_VehicleResprayMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResprayMsg>(pMessage);
				GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(pMsg);
			}
			break;
		case Game::E_GameflowMessage_LoadSPProfileCar:
			{
				if (m_pRLC)
					m_pRLC->StartVehicleLoad(NULL);
			}
			break;

		default:
			handledMessage = false;
		break;
	}

	return handledMessage;
}

void C_AmaxWorldLayer::StartStreamerPurge(S32 p_PurgeAmountBytes)
{
	S32 totalUsed = (S32)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetLevelResource()->GetRenderLevel()->GetScene().GetStreamData().GetTotalUsed();	
	RenderingData::Target::StreamManager::Instance().SetMemoryLimit(totalUsed-p_PurgeAmountBytes);	
}

Debug::Float g_debugNameMinScreenSize("Vehicle.Names.MinScreenSize", 25.0f, Debug::Range(0.0f, 1280.0f), Debug::Step(1.0f));
Debug::Float g_debugNameMaxScreenSize("Vehicle.Names.MaxScreenSize", 200.0f, Debug::Range(0.0f, 1280.0f), Debug::Step(1.0f));
Debug::Float g_debugNameMinFontSize("Vehicle.Names.MinFontSize", 4.0f, Debug::Range(0.0f, 30.0f), Debug::Step(0.001f));
Debug::Float g_debugNameMaxFontSize("Vehicle.Names.MaxFontSize", 16.0f, Debug::Range(0.0f, 30.0f), Debug::Step(0.001f));

Debug::Float g_debugNameFadeMinDist("Vehicle.Names.MinFadeDist", 30.0f, Debug::Range(0.0f, 200.0f), Debug::Step(0.001f));
Debug::Float g_debugNameFadeMaxDist("Vehicle.Names.MaxFadeDist", 80.0f, Debug::Range(0.0f, 200.0f), Debug::Step(0.001f));
Debug::Float g_debugNameFadeMin("Vehicle.Names.MinFade", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugNameFadeMax("Vehicle.Names.MaxFade", 0.9f, Debug::Range(0.0f, 10.f), Debug::Step(0.001f));

HRESULT C_AmaxWorldLayer::RenderSplitscreenWithAA()
{
	// reset states in display, since command buffers have probably changed them
	Display::Device()->InvalidateState();

	{
		PERF_EVENT("BroadcastPostRenderAA_A");

		UI::C_UiGameMsg_IngameRenderAA message( false );
		UI::gButtons.BroadcastMessageNow( 1, &message );
	}

	PERF_EVENT("AmaxWorldLayer.RenderSplitscreenWithAA");

#if PLATFORM_PS3
	GCM_NAMESPACE::cellGcmSetClearReport(gCellGcmCurrentContext, CELL_GCM_ZCULL_STATS); // clear old report values
	GCM_NAMESPACE::cellGcmSetZcullStatsEnable(gCellGcmCurrentContext, CELL_GCM_TRUE); // start taking  zcull statistics
#endif

	m_pGameWorld->RenderSplitscreenWithAA();

#if PLATFORM_PS3
	GCM_NAMESPACE::cellGcmSetZcullStatsEnable(gCellGcmCurrentContext, CELL_GCM_FALSE); // stop taking zcull statistics
#endif
	{
		PERF_EVENT("BroadcastPostRenderAA_B");

		UI::C_UiGameMsg_IngameRenderAA message( true );
		UI::gButtons.BroadcastMessageNow( 1, &message );
	}

	return S_OK;
}

HRESULT C_AmaxWorldLayer::RenderSplitscreenWithoutAA()
{
	m_pGameWorld->RenderSplitscreenWithoutAA();

	m_pGameWorld->DebugRender( DbUtil::DefaultDebugRenderSys()->Immediate() );
	
	return S_OK;
}

HRESULT C_AmaxWorldLayer::RenderFullscreen()
{
#ifndef CFG_GOLD
	if (g_GameDebugRender2d)
	{
		m_pGameDebug->Render2d();

		if(GameDebug::g_debugMiniMonitorEnable)
			Lod::C_LodManager::Instance().Render();
	}

	Game::C_PerfStats::Instance().DisplayOnScreenStats();

	if (GetGameWorld() && GetGameWorld()->GetCrowdSystem())
		GetGameWorld()->GetCrowdSystem()->RenderFullscreen();
#endif

	return S_OK;
}

HRESULT C_AmaxWorldLayer::PostRender()
{
	m_pGameWorld->PostRender();
	return S_OK;
}

C_GameWorld* C_AmaxWorldLayer::GetGameWorld() const
{
	return m_pGameWorld;
}

Game::Actors::RaceLogicCore *C_AmaxWorldLayer::GetRaceLogicCore() const
{
	return m_pRLC;
}
} // namespace Game
