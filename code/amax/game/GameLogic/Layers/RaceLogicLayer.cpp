 #include "stdafx.h"
#include <GameLogic/Profile/GameProfileManager.h>	// [Blur2PC]

#include <GameLogic/Layers/RaceLogicLayer.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>
#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>
#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>
#include <GameLogic\ActorStudioActors\PickupPositionActorDecoy.h>
#include <GameLogic/Pickups/PickupMineBase.h>
#include <GameLogic/Pickups/PickupProjectileBase.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameDef/UserData.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game\Messaging\PickupActorMessages.h>
#include <Game\GameControls\GameControls.h>
#include <Game/Messaging/GameflowMessages.h>

#include <ui/game/Multiplayer/MultiplayerCache.h>

#include <shared_actors/SharedActorUtils.h>
#include <GameLogic\GameData\GameXtLoader.h>

#include "GameAudio/GameAudioLayer.h"
#include <ui/UiMessages.h>

#include <ui/Game/Profiles/ProfileInterface.h>
#include "System/DataLogging/DataLogging.h"
#include <Automation/GameDataLogging.h>
#include <Game/AmaxCam/CameraSystem.h>
#include <vehicle/VehicleModel.h>
#include "system/automation/Automation.h"

#include <ai/AI_AmaxSetup.h>
#include <ai/AI_Amax.h>

#include "system/debug/metadebug/metadebug.h"
#include "system/debug/metadebug/metadebugattributelist.h"

#include <GameLogic\ActorStudioActors\CutSceneMessages.h>

#include "Game/AmaxCam/VehicleFeed.h"

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <GameLogic/AchievementsManager.h>

#include <GameLogic\InGameStartLineInfoManager.h>
#include <GameLogic\MultiplayerUtilities.h>

#include <shared_actors/Utility/VisibilityDetection.h>
#include <Game/GameWorld/ViewManager.h>
#include <scenegraph/view.h>
#include <Game/GameWorld/View.h>

#include <world_render\ModelInstance.h>
#include <GameLogic\Pickups\PickupsManager.h>

#include <GameLogic\ActorStudioActors\DynamicObjectMessages.h>
#include <GameLogic\StickerManager.h>

#include <algorithm>

#include <GameLogic\ActorStudioActors\TriggerRegionActor.h>

static Debug::Logger &g_AmaxLoadingLog = Debug::GetLogger("amax.loading");

extern Debug::Float	gDebugPickupInitialDisableTime;
extern Debug::Bool  gDebugFixRandomPerks;
extern Debug::Float gDebugShuffleTime;

extern Debug::Float g_debugPerkStartDelay;
extern Debug::Float g_debugShockTargetRange;
extern Debug::Float g_debugShockTargetMinRange;
extern Debug::Float g_DebugDraftingBoost;

extern Debug::Bool g_bDisableSPUDamage;

Debug::Bool g_debugEnableCollisionDamping("Gravity.Vehicle.CollisionDamping.EnablePersist", false, Debug::Persist());
Debug::Float g_debugCollisionDampingImpulseThreshold("Gravity.Vehicle.CollisionDamping.ImpulseThreshold", 40000.0f, Debug::Range(0.0f, 100000.0f), Debug::Step(100.0f));
Debug::Float g_debugCollisionDampingImpulseMax("Gravity.Vehicle.CollisionDamping.ImpulseMax", 100000.0f, Debug::Range(0.0f, 100000.0f), Debug::Step(100.0f));
Debug::Float g_dbDampingCoefficientMin("Gravity.Vehicle.CollisionDamping.CoefficientMin", 1000.f, Debug::Range(0.f, 10000.f));
Debug::Float g_dbDampingCoefficientMax("Gravity.Vehicle.CollisionDamping.CoefficientMax", 10000.f, Debug::Range(0.f, 10000.f));
Debug::Float g_dbMaxObjectsMessages("Gravity.Vehicle.Message.NumOfLimitObjectMessages", 20.f, Debug::Range(10.f, 50.f));
Debug::Float g_dbMaxVehicleMessages("Gravity.Vehicle.Message.NumOfLimitVehicleMessages", 20.f, Debug::Range(10.f, 50.f));
Debug::Bool g_debugThrowAwayMessages("Gravity.Vehicle.Message.ThrowAwayMessages", true);
Debug::Float g_dbObjectNormalDotY("Gravity.Vehicle.ObjectNormalDotY", 0.5f);
Debug::Float g_dbCarTunnelSpeed("Vehicle.GamePhysicsMechanics.CarToCar.TunnelThresholdSpeed", 30.f, Debug::Range(0.f, 200.f));

Debug::Int g_debugMaxPickupsOnTrack("Multiplayer.MaxPickupsOnTrack", 170, Debug::Range(1, 250));
Debug::Float g_debugDroppedPickupsTimer("Multiplayer.DroppedPickupsTimer", 10.f, Debug::Range(0.f, 1000.f));
Debug::Float g_debugDroppedDecoysTimer("Multiplayer.Upgrades.DecoyDrop.DroppedDecoysTimer", 15.f, Debug::Range(0.f, 1000.f), Debug::Persist() );

Debug::Float g_debugDefaultPickupSpawnTime("Perks.DefaultTimeToSpawn", 10.0f);
Debug::Float g_debugMotorMashPickupSpawnTime("Perks.MotorMashATimeToSpawn", 1.5f);

Debug::Float g_dbLightToHeavyCarScale("Vehicle.LightToHeavyCarScale", 1.f);
Debug::Float g_dbLightToMediumCarScale("Vehicle.LightToMediumCarScale", 1.1f);  
Debug::Float g_dbMediumToHeavyCarScale("Vehicle.MediumToHeavyCarScale", 1.25f);
Debug::Float g_dbLightToUltraCarScale("Vehicle.LightToUltraCarScale", 1.3f);
Debug::Float g_dbMediumToUltraCarScale("Vehicle.MediumToUltraCarScale", 1.3f);  
Debug::Float g_dbHeavyToUltraCarScale("Vehicle.HeavyToUltraCarScale", 1.1f);


#ifndef CFG_GOLD
namespace AmaxDataLogging
{
	extern bool g_bDoInGameDataLogging;
}
#endif

namespace Game
{

C_RaceLogicLayer::C_RaceLogicLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem ): Framework::C_Layer("C_RaceLogicLayer")
{
	SetSubscriberSystem( pSubscriberSystem );

	AddSubscriber( Game::E_VehicleMessage_VehicleWorldPosition );

	AddSubscriber( Game::E_GameLogicMessage_LapIncremented );
	AddSubscriber( Game::E_GameLogicMessage_SplitCrossed );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_QuitRace );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_RestartRace );
	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );	
	AddSubscriber( GameMessaging::E_GameMsg_Update );

	AddSubscriber( ActorMsg::E_CutSceneMsg_Start321 );
	AddSubscriber( ActorMsg::E_CutSceneMsg_Start_Cutscene );
	AddSubscriber( ActorMsg::E_CutSceneMsg_Start_PassedCutscene );

	// These need to be here so they are set before the pickup actors - Create() is called after.
	mTotalLivePickups = mNumFanPickups = 0;

	for( int i = 0; i < RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes.count; i++ )
		mLivePickups.PushBack(0);


	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	mPickupSpawnInfo.InitNormalValues(levelState.mPerksRespawnRandomType);

	m_RaceLoaded = false;

	mPickupsActivated = false;
	mStarted321=false;
	mMidGamePauseTimer=-1.f;
	mNumVehiclesDisabled = 0;
	mCurrentVehicleEvaluationCheck = 0;
	
	mFanDemandActive=false;
#ifndef CFG_GOLD
	AmaxDataLogging::g_bDoInGameDataLogging = false;
#endif
	C_InGameStartLineInfoManager::Instance().Init();

	m_RaceState = E_RaceState_None;

	mCountObjectsHit=false;
	mObjectsHit.Clear();
	mCountObjectsHandle = Actor::C_ActorHandle();

	m_pRLDamageSystem	= NULL;
	mpWallRidingSystem	= NULL;

	m_IsQuickRestart = false;
	m_PickupPositionManager.Init(this);

	for( int i = 0; i < mWallRidingScalingFactorList.Size(); i++ )
		mWallRidingScalingFactorList[i] = 1.f;

	C_PickupsManager::Instance().Init(this, &levelState);

	//Hopefully the name says it all
	gRaceModeStore.CachePointerToTheRaceLogicLayer(this);

	StopPlayerOvertakeCheck();				// Resets overtake logic
}

C_RaceLogicLayer::~C_RaceLogicLayer()
{
	C_PickupsManager::Instance().Clear();

	//Hopefully the name says it all
	gRaceModeStore.CachePointerToTheRaceLogicLayer((Game::C_RaceLogicLayer*)NULL);
}

void C_RaceLogicLayer::Reset()
{		
	// These need to be here so they are set before the pickup actors - Create() is called after.
	mTotalLivePickups = mNumFanPickups = 0;

	mLivePickups.Clear();
	for( int i = 0; i < RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes.count; i++ )
		mLivePickups.PushBack(0);

	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	mPickupSpawnInfo.InitNormalValues(levelState.mPerksRespawnRandomType);

	mStarted321=false;
	mMidGamePauseTimer=-1.f;
	mNumVehiclesDisabled = 0;
	mCurrentVehicleEvaluationCheck = 0;

#ifndef CFG_GOLD
	AmaxDataLogging::g_bDoInGameDataLogging = false;
#endif
	m_RaceState = E_RaceState_None;

	mCountObjectsHit=false;
	mObjectsHit.Clear();
	mCountObjectsHandle = Actor::C_ActorHandle();

	m_pRLDamageSystem	= NULL;
	m_PickupPositionManager.Reset();

	C_PickupsManager::Instance().Init(this, &levelState);
}

HRESULT C_RaceLogicLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
			break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
			break;

		case Framework::E_LayerMessage_Update:
			return m_restartInProgress ? S_OK : Update();
			break;

		case Framework::E_LayerMessage_GlobalUpdate:
			return m_restartInProgress ? S_OK : GlobalUpdate();
			break;
	}

	return S_OK;
}

HRESULT C_RaceLogicLayer::Create(bool quickRestart)
{
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	BIZ_ASSERT(pRLC);
	if (pRLC)
		m_pRLDamageSystem = (const RLDamageSystem*)pRLC->GetRLSystem()->mDamageSystem.Object();

	BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer());
	BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld());
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() && 
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld())
		m_pGameActorList = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();

	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	m_IsQuickRestart = quickRestart;

	mRandGenerator.Seed((U32)Time::GetCurrentTime());

	mVehicleStarted.Clear();

	for(U32 vehicleIndex = 0; vehicleIndex < levelState.mNumCars; vehicleIndex++)
	{
		Actor::C_ActorCreationDataFactoryHelper<Actors::C_VehicleLogicActorCreate> pVehicleLogicActorCreate;

		pVehicleLogicActorCreate->mVehicleIndex=vehicleIndex;
		pVehicleLogicActorCreate->mRaceLogicLayerPtr=this;
		pVehicleLogicActorCreate->mPickupsAllowed = levelState.mCars[vehicleIndex].mPickupsAllowed;

		Actor::C_ActorHandle handle = m_pGameActorList->Spawn(pVehicleLogicActorCreate.Get());
		mVehicleHandles.PushBack(handle);
		mVehicleStarted.PushBack(false);
	}

	// if (!quickRestart)
	//	UI::g_pUILua->LoadCallFile("Logic\\IngameScoring.lua");

	mTimer_PerkPlayerHitAIDisable = 0.f;

	mBrakeApplied = mHandbrakeApplied = false;
	mNumPushes_Y = mNumPushes_X = mNumPushes_B = mNumPushes_A = mNumLeftStick = mNumRightStick = mNumRB = mNumLB = mNumLT = mNumRT = 0;
	mActiveLeftStick = mActiveRightStick = mActiveRT = mActiveLT = false;

	//if (!quickRestart)
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetPhysicsScene()->RegisterCollisionCallback(this);
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetPhysicsScene()->RegisterEventCallback(this);

	Util::C_String levelExportTime("?");

	if (const MetaDebug::C_MetaNode* pLevelMetaNode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetLevelResource()->GetRootMetaNode())
	{
		if (const MetaDebug::C_AttributeList* attributeList = (MetaDebug::C_AttributeList*) pLevelMetaNode->GetChildByName("Info"))
		{
			if (const MetaDebug::C_Attribute* timeAttribute = attributeList->GetAttributeByName("TimeExported"))
			{
				Time::C_DecomposedTime dt(timeAttribute->GetValueTime(), true);
				levelExportTime = Time::FormatLongDateString(dt) + " - " + Time::FormatTimeString(dt) + " GMT";
			}
		}
	}

	LOGINFO(g_AmaxLoadingLog, "Loading level...");
	LOGINFO(g_AmaxLoadingLog, "   GameMode: %s", C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetName());
	LOGINFO(g_AmaxLoadingLog, "   City    : %s (%x) - Exported(%s)", C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity]->tag.string, levelState.mCity, levelExportTime.Chars());
	LOGINFO(g_AmaxLoadingLog, "   Route   : %s (%x)", C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute]->tag.string, levelState.mRoute);
	LOGINFO(g_AmaxLoadingLog, "   Event   : %x", levelState.mEvent);
	LOGINFO(g_AmaxLoadingLog, "   NumCars : %d", levelState.mNumCars);
	for (unsigned int iCar=0; iCar < levelState.mNumCars; iCar++)
	{
		U32 carId = levelState.mCars[iCar].mCarID;
		LOGINFO(g_AmaxLoadingLog, "      Car  : %s (%x) (Pos %d)", C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)carId]->tag.string, carId, levelState.mCars[iCar].mStartPosition);
	}

	Game::C_StickerManager::Instance().InitForNewRace();
#ifndef CFG_GOLD
	CreateDataLogging();
#endif
	m_RaceState = E_RaceState_Loading;	
	
	m_restartInProgress = false;

	mPerksActivatedIn = 0.0f;

	mPickupsActivated = false;

	
	mpWallRidingSystem = RLDatabase::Instance().mRLSystems[0]->mWallRidingSystem;

	for( int i = 0; i <levelState.mNumCars; i++ )
	{
		GAMEDATA_ID vehId		=( GAMEDATA_ID)levelState.mCars[i].mCarID;
		AmaxVehicle* pAmaxVeh	= C_GameVehicleData::Instance().mVehicles[vehId];

		if(mpWallRidingSystem && pAmaxVeh)
		{
			switch(pAmaxVeh->carClass)
			{
			case AmaxCarClass_A:
				mWallRidingScalingFactorList[i] = mpWallRidingSystem->mClassAScale;
				break;

			case AmaxCarClass_B:
				mWallRidingScalingFactorList[i] = mpWallRidingSystem->mClassBScale;
				break;

			case AmaxCarClass_C:
				mWallRidingScalingFactorList[i] = mpWallRidingSystem->mClassCScale;
				break;

			case AmaxCarClass_D:
				mWallRidingScalingFactorList[i] = mpWallRidingSystem->mClassDScale;
				break;

			default:
				mWallRidingScalingFactorList[i] = 1.f;
				break;
			}
		}	
		else
			mWallRidingScalingFactorList[i] = 1.f;
	}

	return S_OK;
}

#ifndef CFG_GOLD
void C_RaceLogicLayer::CreateDataLogging()
{
	m_dataLog_PositionUpdateTime = mDataLogVehicleDistancesTime = 0.f;

	//Cameras
	{
		m_dataLog_CameraDurations[AmaxCam::E_VFCamera_Hollywood].first		= 0.0f;
		m_dataLog_CameraDurations[AmaxCam::E_VFCamera_Hollywood].second		= "Hollywood";
		m_dataLog_CameraDurations[AmaxCam::E_VFCamera_Bumper].first			= 0.0f;
		m_dataLog_CameraDurations[AmaxCam::E_VFCamera_Bumper].second		= "Bumper";		
	}

	U32 padNo = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	SysProfile::FullProfile& profile = UI::C_ProfileInterface::Instance().GetPadProfile( padNo );

	Util::C_String gamerTag;
	gamerTag.Printf( "%S", profile.m_Name );

	RacerNet::NetRaceSettings raceSettings;
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

	Game::C_GameModeBase* pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	UI::C_WideString32 wEventStr;

	bool singlePlayer = false;
	AmaxEvent *ev = 0;
	if(pGameModeBase->IsRanked())
	{
		const MultiplayerEvent* pEv = UI::C_MultiplayerCache::Instance().CurrentEvent();
		if(pEv)
			wEventStr.Format(UI::C_TextStringID( pEv->name.id ).String() );
	}
	else
	{
		if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == E_GameMode_SinglePlayer)
		{
			ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();
			if(ev)
			{
				singlePlayer = true;
			}
		}
	}

	Util::C_String eventName				= wEventStr.AsString();
	const Game::C_LevelState& levelState	= C_GameApp::GetInstance()->GetGame()->GetLevelState();

	AmaxDataLogging::LogRaceStart(
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetName(),
		&levelState,
		gamerTag.Chars(),
		singlePlayer ? ev->tag.Text() : eventName.Chars(),
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr()->GetName());
}

void C_RaceLogicLayer::EndDataLogging()
{
	if(!AmaxDataLogging::g_bDoInGameDataLogging)
		return;

	for (S32 iCam = 0; iCam < m_dataLog_CameraDurations.Size(); iCam++)
	{
		AmaxDataLogging::LogCameraDuration(m_dataLog_CameraDurations[iCam].second.Chars(), m_dataLog_CameraDurations[iCam].first);
		m_dataLog_CameraDurations[iCam].first = 0.0f;
	}
	
	AmaxDataLogging::g_bDoInGameDataLogging = false;	
}
#endif //CFG_GOLD

HRESULT C_RaceLogicLayer::Destroy(bool quickRestart)
{
	m_IsQuickRestart = quickRestart;

	mVehicleStarted.Clear();

	if (quickRestart)
	{
		mVehicleHandles.Clear();
		Reset();
	}

	C_InGameStartLineInfoManager::Instance().Clear();

	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetPhysicsScene())
	{
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetPhysicsScene()->RegisterCollisionCallback(NULL);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetPhysicsScene()->RegisterEventCallback(NULL);
	}

	m_CachedObjectCollisions.Clear();
	m_CachedImpactMessages.Clear();
	m_PhysicsBodyEvents.Clear();

	m_pRLDamageSystem	= NULL;
	m_pGameActorList	= NULL;

	C_StickerManager::Instance().Shutdown();

	return S_OK;
}

Debug::Bool g_debugDoVehicleFreeze("Vehicle.Test.DoFreeze", false);
Debug::Int g_debugVehicleFreezeIndex("Vehicle.Test.FreezeIndex", 0, Debug::Range(0, 19), Debug::Step(1));
Debug::Bool g_debugFreezeVehicle("Vehicle.Test.Freeze", true);

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
Debug::Bool						g_debugResetIntegrationType("RacingPhysics.ResetIntegrationType", false);
Debug::Int						g_debugDynamicsLodDefaultSteps("RacingPhysics.DyanmicsLodDefaultSteps", 4);
#endif

Debug::Bool						g_debugSmartSetIntegrationType			("Vehicle.LOD.Dynamcis.SmartSetIntegrationType", true);
Debug::Int						g_debugDynamicsLodMaxHighRes			("Vehicle.LOD.Dynamcis.DynamicsLodThreshold", 1);
Debug::Float					g_debugDynamicsDistanceThreshold		("Vehicle.LOD.Dynamcis.DynamicsLodDistanceThreshold", 20.0f);
Debug::Int						g_debugDynamicsLodLowSteps				("Vehicle.LOD.Dynamcis.DyanmicsLodLowSteps", 2);
Debug::Bool						g_debugDynamicsLodScaleWithNumCars		("Vehicle.LOD.Dynamcis.ScaleNumHighResBasedOnNumVehicles", true);
Debug::Int						g_debugDynamicsLodNumVehiclesThreshold	("Vehicle.LOD.Dynamcis.NumVehicleThreshold", 8);

/*Debug::Bool						g_debugWheelParticleLodEnable					("Vehicle.LOD.WheelParticles.Enable", true);
Debug::Int						g_debugWheelParticleLodMaxFullFront				("Vehicle.LOD.WheelParticles.MaxFullFront", 3);
Debug::Float					g_debugWheelParticleFullFrontDistanceThreshold	("Vehicle.LOD.WheelParticles.FullFrontDistanceThreshold", 100.0f);

Debug::Int						g_debugWheelParticleLodMaxHalfFront				("Vehicle.LOD.WheelParticles.MaxHalfFront", 6);
Debug::Float					g_debugWheelParticleHalfFrontDistanceThreshold	("Vehicle.LOD.WheelParticles.HalfFrontDistanceThreshold", 200.0f);

Debug::Int						g_debugWheelParticleLodMaxHalfMirror			("Vehicle.LOD.WheelParticles.MaxHalfMirror", 4);
Debug::Float					g_debugWheelParticleHalfMirrorDistanceThreshold	("Vehicle.LOD.WheelParticles.HalfMirrorDistanceThreshold", 50.0f);*/

#ifndef CFG_GOLD
Debug::Bool g_debugOverrideAiNetworkOptims("AiNetOptim.Override", false);
Debug::Bool g_debugAiTakeControl("AiNetOptim.RunFullAi", true);
#endif

HRESULT C_RaceLogicLayer::Update()
{
	// Update timers
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	float frameTime =  C_GameApp::GetInstance()->GetTimeStep();

	if( mTimer_PerkPlayerHitAIDisable > 0 )
		mTimer_PerkPlayerHitAIDisable -= frameTime;
#ifndef CFG_GOLD
	UpdateDataLogging();

	bool doVehicleFreeze = (bool)g_debugDoVehicleFreeze;
	if (doVehicleFreeze)
	{
		int vehicleIndex = (int)g_debugVehicleFreezeIndex;
		if (GetLogicVehicleActor(vehicleIndex))
		{
			Game::Actors::C_VehicleFreezeMsg freezeMsg(vehicleIndex, (bool)g_debugFreezeVehicle);
			GetLogicVehicleActor(vehicleIndex)->GetGraphicalActor(vehicleIndex)->SendMessageToActor(GetLogicVehicleActor(vehicleIndex)->GetGraphicalActor(vehicleIndex)->GetHandle(), &freezeMsg);
		}
		g_debugDoVehicleFreeze = false;
	}
	
	if (g_debugOverrideAiNetworkOptims)
	{
		for (S32 iCar = 1; iCar < GetNumVehicles(); iCar++)
		{
			if (g_debugAiTakeControl)
				GetLogicVehicleActor(iCar)->GetGraphicalActor(iCar)->ForceAIControl(false);
			else
				GetLogicVehicleActor(iCar)->GetGraphicalActor(iCar)->ForcePlayerControl(false);
		}

		g_debugOverrideAiNetworkOptims = false;
	}
#endif

	if(!C_GameApp::GetInstance()->IsPaused())
	{
		if(levelState.mPickupDisableTime >= 0 && mPerksActivatedIn)
		{
			mPerksActivatedIn-=frameTime;

			if(mPerksActivatedIn < 0.f)
			{
				mPerksActivatedIn=0.f;
				mPickupsActivated = true;
				Game::Actors::PickupMessage_ActivatePickups msg(0.0f);
				if (m_pGameActorList)
					m_pGameActorList->BroadcastMessage(&msg);
				//C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);				

				::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_PickupMessage_UpdateFanDemandState );
			}
		}

		//	[GavinY] Probably a better place to put this but safer than having it where it was (RaceBookLayer::Update())
		Game::C_StickerManager::Instance().Update(frameTime);
	}

	if (levelState.mNumViewports == 1)
		SortVehiclesAndLodBehaviour();

	return S_OK;
}

bool C_RaceLogicLayer::CanEvaluateVehicle(U8 idx) 
{ 
	return (mCurrentVehicleEvaluationCheck == idx); 
};

void C_RaceLogicLayer::IncrementEvaluationVehicle() 
{ 
	if(++mCurrentVehicleEvaluationCheck >= GetNumVehicles())
	{
		mCurrentVehicleEvaluationCheck = 0;
	}
};

void C_RaceLogicLayer::SortVehiclesAndLodBehaviour()
{
	if (!m_RaceLoaded)
		return;

	m_SortedVehicles.Clear();
	
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	if (!pRLC)
		return;

	S32 focusIndex = pRLC->GetViewportFocusVehicleIndex(0);
	Game::Actors::C_VehicleActor* pFocusActor = pRLC->GetVehicleActor(focusIndex);	
	if (pFocusActor)
	{
		m_SortedVehicles.PushBack();
		m_SortedVehicles.Back().m_Index		= focusIndex;
		m_SortedVehicles.Back().m_Distance	= 0.0f;

		for (U32 iCar = 0; iCar < pRLC->GetNumVehicles(); iCar++)
		{
			if (iCar == focusIndex)
				continue;

			Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iCar);
			float fDist = (pFocusActor->GetMatrix().AW() - pActor->GetMatrix().AW()).Length2();
			m_SortedVehicles.PushBack();
			m_SortedVehicles.Back().m_Index		= iCar;
			m_SortedVehicles.Back().m_Distance	= fDist;
		}
		std::sort(m_SortedVehicles.Begin(), m_SortedVehicles.End());
	}
	
	if (g_debugSmartSetIntegrationType)
	{
		S32 numHigh		= 0;
		S32 maxHighRes	= g_debugDynamicsLodMaxHighRes;

		/*S32 numWheelParticlesFullFront = 0;
		S32 numWheelParticlesHalfFront = 0;
		S32 maxWheelParticlesFullFront = g_debugWheelParticleLodMaxFullFront;
		S32 maxWheelParticlesHalfFront = g_debugWheelParticleLodMaxHalfFront;

		S32 numWheelParticlesHalfMirror = 0;
		S32 maxWheelParticlesHalfMirror = g_debugWheelParticleLodMaxHalfMirror;*/

		if (g_debugDynamicsLodScaleWithNumCars)
		{
			F32 fNumCarsScale = Maths::Clamp((20.0f - (float)pRLC->GetNumVehicles()) / (20.0f - (float)g_debugDynamicsLodNumVehiclesThreshold), 0.0f, 1.0f);
			maxHighRes = (S32)Maths::Lerp(1.0f, 20.0f, fNumCarsScale);
		}

		for (S32 iCar = 0; iCar < m_SortedVehicles.Size(); iCar++)
		{
			S32 iIndex = m_SortedVehicles[iCar].m_Index;
			Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iIndex);
			
			if (!pActor->GetVisibleDetect())
				continue;

			F32 fDistance = m_SortedVehicles[iCar].m_Distance;
			bool visibleInFront = pActor->GetVisibleDetect()->IsVisible(Game::E_ViewTypeFlag_Main) != 0;
			//bool visibleInMirror = pActor->GetVisibleDetect()->IsVisible(Game::E_ViewTypeFlag_Mirror) != 0;
			
			// DYNAMICS LOD

			if (pActor->GetControlType() == E_VEHICLE_CONTROL_TYPE_PLAYER || (numHigh < maxHighRes && (visibleInFront && fDistance <= (g_debugDynamicsDistanceThreshold*g_debugDynamicsDistanceThreshold))))
			{
				pActor->GetVehiclePhysics()->OverrideNumIntegrationSteps(4);
				numHigh++;
			}
			else
			{
				pActor->GetVehiclePhysics()->OverrideNumIntegrationSteps((S32)g_debugDynamicsLodLowSteps);
			}

			// WHEEL PARTICLE LOD

			// this is now done in vehicle actor across all splits
			/*if(g_debugWheelParticleLodEnable)
			{
				if (visibleInFront)
				{
					if (numWheelParticlesFullFront < maxWheelParticlesFullFront && fDistance <= (g_debugWheelParticleFullFrontDistanceThreshold*g_debugWheelParticleFullFrontDistanceThreshold))
					{
						pActor->SetWheelParticleLod(true, true, true, true, 1.0f, 1.0f, 1.0f, 1.0f);
						numWheelParticlesFullFront++;
					}
					else if (numWheelParticlesHalfFront < maxWheelParticlesHalfFront && fDistance <= (g_debugWheelParticleHalfFrontDistanceThreshold*g_debugWheelParticleHalfFrontDistanceThreshold))
					{
						pActor->SetWheelParticleLod(false, false, true, true, 0.0f, 0.0f, 0.5f, 0.5f);
						numWheelParticlesHalfFront++;
					}
					else
					{
						pActor->SetWheelParticleLod(false, false, false, false, 0.0f, 0.0f, 0.0f, 0.0f);
					}
				}
				else if (visibleInMirror)
				{
					if (numWheelParticlesHalfMirror < maxWheelParticlesHalfMirror && fDistance <= (g_debugWheelParticleHalfMirrorDistanceThreshold*g_debugWheelParticleHalfMirrorDistanceThreshold))
					{	
						pActor->SetWheelParticleLod(true, true, false, false, 0.5f, 0.5f, 0.0f, 0.0f);
						numWheelParticlesHalfMirror++;
					}
					else
					{
						pActor->SetWheelParticleLod(false, false, false, false, 0.0f, 0.0f, 0.0f, 0.0f);
					}
				}
			}
			else
				pActor->SetWheelParticleLod(true, true, true, true, 1.0f, 1.0f, 1.0f, 1.0f);*/
		}		
	}

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	if (g_debugResetIntegrationType)
	{
		for (U32 iCar = 0; iCar < pRLC->GetNumVehicles(); iCar++)
		{
			Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iCar);
			pActor->GetVehiclePhysics()->OverrideNumIntegrationSteps((S32)g_debugDynamicsLodDefaultSteps);			
		}		
		g_debugResetIntegrationType = false;
	}
#endif
}

const int C_RaceLogicLayer::GetTotalLivePickups() const
{
	return mTotalLivePickups;
}

void C_RaceLogicLayer::IncrementTotalLivePickups(const Actor::C_ActorHandle& actorHandle)
{
	mTotalLivePickups++;
}

void C_RaceLogicLayer::DecrementTotalLivePickups(const Actor::C_ActorHandle& actorHandle)
{
	mTotalLivePickups--;

	//Bug#34788 - Tell the AI when a pickup is removed from the world
	Actor::C_ActorKilledMsg msg(actorHandle);
	AIAmax::AmaxAIR::Instance().MessageHandler(&msg);
}

const char* C_RaceLogicLayer::ConvertPickupTypeToString(E_PICKUP_TYPE type)
{
	switch (type)
	{
	case E_PICKUP_TYPE_NITROUS:
		return "Nitrous";
	case E_PICKUP_TYPE_SHOCK:
		return "Shock";
	case E_PICKUP_TYPE_SHUNT:
		return "Shunt";
	case E_PICKUP_TYPE_BARGE:
		return "Barge";
	case E_PICKUP_TYPE_BOMB:
		return "Bomb";
	case E_PICKUP_TYPE_REPAIR:
		return "Repair";
	case E_PICKUP_TYPE_BLINDFIRE:
		return "Blindfire";
	case E_PICKUP_TYPE_SUPER_SHIELD:
		return "Supershield";
	case E_PICKUP_TYPE_SHIELD:
		return "Shield";
	case E_PICKUP_TYPE_FAN:
		return "Fan";
	case E_PICKUP_TYPE_STOPWATCH:
		return "Stopwatch";
	default:
		return "Unknown";
	}
}

void C_RaceLogicLayer::CalculateBiggestOnScreenVehicles()
{
	mBiggestOnScreenVehicles.Clear();

	for(int a=0;a<this->GetNumVehicles();a++)
	{
		Actors::C_VehicleLogicActor* pLogicActor = GetLogicVehicleActor(a);
		if(pLogicActor)
		{
			Game::Actors::C_VehicleActor* pActor = pLogicActor->GetGraphicalActor(a);
			if(pActor)
			{
				if(pActor->GetVisibleDetect()->IsVisible())
				{
					const SceneGraph::C_View& view = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetViewManager()->GetView(0)->GetMainView();
					F32 sizeOnScreen = view.GetViewSettings3D().GetScreenSize(pActor->GetVehicleModel()->GetModelInstance(0)->GetWorldSphere());

					mBiggestOnScreenVehicles.PushBack(sizeOnScreen);
				}
				else
				{
					mBiggestOnScreenVehicles.PushBack(0.f);
				}
			}
		}
	}
}

#ifndef CFG_GOLD
void C_RaceLogicLayer::UpdateDataLogging()
{
	if (!AmaxDataLogging::g_bDoInGameDataLogging || C_GameApp::GetInstance()->IsPaused())
		return;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	float fTimeStep = C_GameApp::GetInstance()->GetTimeStepRaw();
	m_dataLog_PositionUpdateTime -= fTimeStep;
	mDataLogVehicleDistancesTime -= fTimeStep;

	if (m_dataLog_PositionUpdateTime <= 0.f)
	{
		// Reset timer to 1 second
		m_dataLog_PositionUpdateTime = 1.f;

		// Log data about car position
		if (levelState.mNumCars > 0)
		{
			const RaceLogicCarInfo& ci						= mVehiclePositions[0];
			Actors::C_VehicleLogicActor* pLogicActor		= GetLogicVehicleActor(0);
			Game::Actors::C_VehicleActor* pActor			= pLogicActor->GetGraphicalActor(0);
			class RacingPhysics::Car* pCar					= pActor->GetVehiclePhysics();

			bool wheelsOnGround = pCar->WheelOnGround(0) && pCar->WheelOnGround(1) && pCar->WheelOnGround(2) && pCar->WheelOnGround(3);

			AmaxDataLogging::LogPlayerInfo(ci.mPosition, pLogicActor->GetPositionInfo().mSpeedInMph, pLogicActor->GetLapInfo().mCurrentLap, mNumPushes_A, mNumPushes_B,
											mNumPushes_X, mNumPushes_Y, mNumLeftStick, mNumRightStick, mNumRB, mNumLB, mNumLT, mNumRT, pRLC->GetRaceTime(), wheelsOnGround,
											C_GameApp::GetInstance()->GetTimer().GetFrameRate());
		}

		mNumPushes_Y = mNumPushes_X = mNumPushes_B = mNumPushes_A = mNumLeftStick = mNumRightStick = mNumRB = mNumLB = mNumLT = mNumRT = 0;
	}

	if( mDataLogVehicleDistancesTime <= 0.f && levelState.mNumCars > 0)
	{
		mDataLogVehicleDistancesTime = 10.0f;

		float playerDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(0);

		for( int i = 1; i < GetNumVehicles(); i++ )
		{
#if 0
			float dist=GetLogicVehicleActor(0)->mVehicleProximities[i].mDistanceSquared;
			int player_race_pos=GetLogicVehicleActor(0)->GetPositionInfo().mRacePosition;
			dist=Maths::Sqrt(dist);

			int ai_race_pos=GetLogicVehicleActor(i)->GetPositionInfo().mRacePosition;

			if(ai_race_pos > player_race_pos)
			{
				dist*=-1;
			}
#else 
			
			float otherDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(i);

			float dist = otherDist - playerDist;
			if (Maths::Abs(dist) > 500.0f)
			{
				
				int player_race_pos=GetLogicVehicleActor(0)->GetPositionInfo().mRacePosition;
				int ai_race_pos=GetLogicVehicleActor(i)->GetPositionInfo().mRacePosition;
				int stophereplease = 1;
			}

#endif

			AmaxDataLogging::LogAIInfo(i, dist, pRLC->GetRaceTime());
		}
	}

	Btl::List<AmaxDataLogging::PerkGainInfo>::Iterator it = AmaxDataLogging::g_PerkUsedList.Begin();

	for (; it != AmaxDataLogging::g_PerkUsedList.End();)
	{
		AmaxDataLogging::PerkGainInfo* gainInfo = &(*it);

		if (gainInfo->mTime == 10.f)
		{
			float playerDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(gainInfo->mPlayer);

			float dist = 0.f;
			for (int i=0; i<GetNumVehicles(); i++)
			{
				if (i != gainInfo->mPlayer)
				{
					float otherDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(i);
					dist += otherDist - playerDist; // abs?
				}
			}

			dist /= GetNumVehicles()-1;
			gainInfo->mDist = dist;

			gainInfo->mTime = 5.f;	// 5 seconds
		}
		else
		{
			gainInfo->mTime -= fTimeStep;

			if (gainInfo->mTime <= 0.f)
			{
				float playerDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(gainInfo->mPlayer);

				float dist = 0.f;
				for (int i=0; i<GetNumVehicles(); i++)
				{
					if (i != gainInfo->mPlayer)
					{
						float otherDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(i);
						dist += otherDist - playerDist;
					}
				}

				if (GetNumVehicles() > 1)
				{
					dist /= GetNumVehicles()-1;

					AmaxDataLogging::LogPickupUsageGain(gainInfo, dist);
				}

				// We move the iterator on to the next before we delete as it gets invalidated, so we can't carry on iterating
				Btl::List<AmaxDataLogging::PerkGainInfo>::Iterator itDelete = it;
				++it; // This iterator is here for a very good reason, please don't move me!

				AmaxDataLogging::g_PerkUsedList.Erase(itDelete);
				continue;
			}
		}

		++it;
	}

	m_dataLog_CameraDurations[AmaxCam::CameraSystem().VehicleFeed()->CurrentType()].first += fTimeStep;		

	S8 viewportIndex	= 0;
	U8 controllerIndex	= pRLC->GetViewportControllerIndex(viewportIndex);
	int vehicleIndex	= pRLC->GetPlayerVehicleIndex(controllerIndex);

	if (controllerIndex < INVALID_CONTROLLER && vehicleIndex >= 0)
	{
		// ---- Button presses per push ----
		if( Input::GetGameInput(controllerIndex)->GetButtonPressed(E_Button_Handbrake) )
		{
			if(!mHandbrakeApplied)
				AmaxDataLogging::LogBrake(controllerIndex, pRLC->GetRaceTime(), GetLogicVehicleActor(vehicleIndex)->GetPositionInfo().mWorldPosition, true);

			mHandbrakeApplied = true;
		}
		else
			mHandbrakeApplied = false;

		if( Input::GetGameInput(controllerIndex)->GetButtonValue(E_Button_Brake) > 0.5f )
		{
			if(!mBrakeApplied)
				AmaxDataLogging::LogBrake(controllerIndex, pRLC->GetRaceTime(), GetLogicVehicleActor(vehicleIndex)->GetPositionInfo().mWorldPosition, false);

			mBrakeApplied = true;
		}
		else
			mBrakeApplied = false;

		//if( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_ChangeCamera) )
		//	AmaxDataLogging::LogCameraChanged(controllerIndex, pRLC->GetRaceTime());

		// ---- Button presses every X seconds ----
		if ( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonX) )
			mNumPushes_X++;

		if ( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA) )
			mNumPushes_A++;

		if ( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonY) )
			mNumPushes_Y++;

		if ( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB) )
			mNumPushes_B++;

		if ( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonLB) )
			mNumLB++;

		if ( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonRB) )
			mNumRB++;

		float leftStick = Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Framework::E_Axis_Debug_LeftStick_X);
		if( !mActiveLeftStick && ( leftStick >= 0.5f || leftStick < -0.5f ) )
		{
			mNumLeftStick++;
			mActiveLeftStick = true;
		}
		else if( leftStick >= -0.5f && leftStick <= 0.5f )
			mActiveLeftStick = false;

		float rightStick = Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Framework::E_Axis_Debug_RightStick_X);
		if( !mActiveRightStick && ( rightStick >= 0.5f || rightStick < -0.5f ) )
		{
			mNumRightStick++;
			mActiveRightStick = true;
		}
		else if( rightStick >= -0.5f && rightStick <= 0.5f )
			mActiveRightStick = false;

		float leftTrigger =Input::GetGameInput(controllerIndex)->GetButtonValue(E_Button_Throttle);
		if( !mActiveRT && ( leftTrigger >= 0.5f || leftTrigger < -0.5f ) )
		{
			mNumLT++;
			mActiveLT = true;
		}
		else if( leftTrigger >= -0.5f && leftTrigger <= 0.5f )
			mActiveLT = false;

		float rightTrigger =Input::GetGameInput(controllerIndex)->GetButtonValue(E_Button_Throttle);
		if( !mActiveRT && ( rightTrigger >= 0.5f || rightTrigger < -0.5f ) )
		{
			mNumRT++;
			mActiveRT = true;
		}
		else if( rightTrigger >= -0.5f && rightTrigger <= 0.5f )
			mActiveRT = false;
	}
}
#endif

Debug::Bool g_debugEnableWallRidingFix2("Vehicle.GamePhysicsMechanics.WallRiding.EnableFix2", true);
Debug::Float g_debugWallRidingNormalDotUpThreshold2("Vehicle.GamePhysicsMechanics.WallRiding.NormalDotUpThreshold2", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Bool g_debugEnableWorldCollisionStore("Vehicle.GamePhysicsMechanics.CollsionStore.StoreCollisions", false, Debug::Persist());
Debug::Bool g_debugRenderRecordedCollisions("Vehicle.GamePhysicsMechanics.CollsionStore.Render", false, Debug::Persist());
Debug::Int g_debugRecordedCollisionsVehicleIndex("Vehicle.GamePhysicsMechanics.CollsionStore.Vehicle", 0, Debug::Range(0, 19), Debug::Step(1));
Debug::Float g_dbBaumgarteOverride("Vehicle.GamePhysicsMechanics.WallRiding.BaumgarteOverride", 3.5f, Debug::Range(0.f, 10.f));
Debug::Float g_dbMinPenetration("Vehicle.GamePhysicsMechanics.WallRiding.MinimumPenetration", 0.05f, Debug::Range(0.f, 10.f));
Debug::Float g_dbWallRiddingCosAngle("Vehicle.GamePhysicsMechanics.WallRiding.CosAngle", 0.5f, Debug::Range(0.f, 1.f));
Debug::Float g_dbWallRidingUpAngle("Vehicle.GamePhysicsMechanics.WallRiding.VehicleUpAngle", 0.866f, Debug::Range(0.f, 1.f));

HRESULT C_RaceLogicLayer::GlobalUpdate()
{
	if (g_debugRenderRecordedCollisions)
	{
		m_debugCollisionEvents.Render((int)g_debugRecordedCollisionsVehicleIndex);
	}

	if(mMidGamePauseTimer > 0.f)
	{
		mMidGamePauseTimer-=C_GameApp::GetInstance()->GetTimeStepRaw();

		if(mMidGamePauseTimer < 5.f)
		{
			if(!C_GameApp::GetInstance()->IsPaused())
			{
				Util::C_String32 buffer;
				buffer.Printf( "Flash_fade_%d", 0 );
				UI::C_UiTimelineMessage msg( buffer.Chars(), true, 0.0f );
				UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );
			}

			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(E_GameflowMessage_Pause);
		}

		if(mMidGamePauseTimer < 0.f)
		{
			if(C_GameApp::GetInstance()->IsPaused())
			{
				Util::C_String32 buffer;
				buffer.Printf( "Flash_fade_%d", 0 );
				UI::C_UiTimelineMessage msg( buffer.Chars(), true, 0.0f );
				UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );
			}

			mMidGamePauseTimer=0.f;
		
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(E_GameflowMessage_UnPause);

			AmaxCam::CameraSystem().GameFeedShow(0,AmaxCam::E_GameCam_VehicleFeed);
		}
	}

	
	return S_OK;
}

Actors::C_VehicleLogicActor *C_RaceLogicLayer::GetLogicVehicleActor(int i)
{
	if (i < 0 || i >= mVehicleHandles.Size())
		return NULL;

	const Actor::C_Actor *pActor = m_pGameActorList->GetActorPtr(mVehicleHandles[i]);

	// Actually, I want to edit the object: have at you, ye const!
	return const_cast<Actors::C_VehicleLogicActor*>((Actors::C_VehicleLogicActor*)pActor);
}

Actors::C_VehicleActor *C_RaceLogicLayer::GetVehicleActor(int i)
{
	if (i < 0 || i >= mVehicleHandles.Size())
		return NULL;

	const Actor::C_Actor *pActor = m_pGameActorList->GetActorPtr(mVehicleHandles[i]);

	// Actually, I want to edit the object: have at you, ye const!
	return const_cast<Actors::C_VehicleActor*>((Actors::C_VehicleActor*)pActor);
}





::Actors::PickupPositionActor* C_RaceLogicLayer::GetPickupPositionActor(U32 id)
{
	::Actors::PickupPositionActor* pActor = NULL;

	Btl::Map<U32, Actor::C_ActorHandle>::Iterator it = mPickupPositionActorMap.Find(id);

	if( it != mPickupPositionActorMap.End() )
	{
		Actor::C_ActorHandle handle = (*it).GetValue();
		pActor = (::Actors::PickupPositionActor*)m_pGameActorList->GetActorPtr(handle);
	}

	return pActor;
}

void C_RaceLogicLayer::DestroyDroppedPickupPositionActor(U32 id)
{
	::Actors::PickupPositionActor* pActor = NULL;

	Btl::Map<U32, Actor::C_ActorHandle>::Iterator it = mDroppedPickupPositionActorMap.Find(id);

	if( it != mDroppedPickupPositionActorMap.End() )
	{
		Actor::C_ActorHandle handle = (*it).GetValue();
		pActor = (::Actors::PickupPositionActor*)m_pGameActorList->GetActorPtr(handle);
		if(pActor)
		{
			pActor->Destroy();
		}
	}
}


int C_RaceLogicLayer::GetNumLiveVehicles()
{
	//	Special case for Points mode - even wrecked vehicles are in the running for winner (only disconnected vehicles aren't included in the points ranking).
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr()->GetID() == Game::E_RaceMode_Multiplayer_Points)
		return GetNumConnected();
	
	return (GetNumVehicles() - mNumVehiclesDisabled);
}

void C_RaceLogicLayer::ForceRandomisePickups( U32 p_Seed )
{
	mRandGenerator.Seed(p_Seed);

	Btl::Map<U32, Actor::C_ActorHandle>::Iterator it = mPickupPositionActorMap.Begin();
	while( it != mPickupPositionActorMap.End())
	{
		::Actors::PickupPositionActor* pActor = NULL;

		Actor::C_ActorHandle handle = (*it).GetValue();
		pActor = (::Actors::PickupPositionActor*)m_pGameActorList->GetActorPtr(handle);
		if(pActor)
		{
			pActor->SetRandomPickupTypeMpSetup();
		}
		it++;
	}
}

bool C_RaceLogicLayer::ShowMidPointHelp()
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(levelState.mEvent == 0xAFA00028 && mMidGamePauseTimer < 5.f && mMidGamePauseTimer != -1.f && mMidGamePauseTimer != 0.f)
		return true;
	else
		return false;
}

void C_RaceLogicLayer::MidGamePause()
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(levelState.mEvent == 0xAFA00028 && mMidGamePauseTimer==-1.f)
	{
		mMidGamePauseTimer=3.5f + 5.f;
		
		// Tell Audio
		InGameDialogueEvent dialogueMsg(155, 16);
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&dialogueMsg);

		// Tell UI
		UI::C_UiGameMsg_Dialogue uiDialogueMsg( (GAMEDATA_ID)0xA926986F, UI::C_TextStringID("RaceDialogue_3348") );
		_UI_MESSAGE_FULL( uiDialogueMsg );

		mPerksActivatedIn=3.5f;
	}
}

void C_RaceLogicLayer::StartObjectHitCounting(Actor::C_ActorHandle handle)
{
	mCountObjectsHit=true;
	mObjectsHit.Clear();
	mCountObjectsHandle = handle;
}

int C_RaceLogicLayer::GetObjectHitCounting()
{
	int num=mObjectsHit.Size();
	return num;
}

void C_RaceLogicLayer::EndObjectHitCounting()
{
	mCountObjectsHit=false;
	mObjectsHit.Clear();
	mCountObjectsHandle = Actor::C_ActorHandle();
}

U32 C_RaceLogicLayer::DropPickup(U32 uniqueId, S32 dropperIndex, E_PICKUP_TYPE type, Maths::Mat43 mat)
{
	Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();

	if(uniqueId == -1)
	{
		uniqueId = Game::C_PickupsManager::Instance().AllocateUniqueId(dropperIndex);
	}

	//mostly copied from PickupShock code
	Actor::C_ActorCreationDataFactoryHelper< ::Actors::PickupPositionCreate> pPickupActorCreate;
	pPickupActorCreate->m_pPickupType = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupHandle(type);
	pPickupActorCreate->m_iPickupActiveLevel=1;
	pPickupActorCreate->m_OneShot=true;
	pPickupActorCreate->m_bRotateGraphic = true;
	pPickupActorCreate->m_AllowRandomPickupOverride = false;
	pPickupActorCreate->m_bConstantShuffle = false;
	pPickupActorCreate->m_ForceSelectedType = true;
	pPickupActorCreate->m_WorldMatrix=Store(mat);
	pPickupActorCreate->m_UniqueDropID = uniqueId;
	pPickupActorCreate->m_OneShotTime = g_debugDroppedPickupsTimer;
	pPickupActorCreate->m_DroppedFromVehicleIndex = dropperIndex;

	if(pPickupActorCreate->m_pPickupType->meType == E_PICKUP_TYPE_NONE)
		BIZ_ASSERT_FAIL("VLC_Pickups: Dropping a null pickup");

	::Actor::C_ActorHandle actorHandle = pActorList->Spawn(pPickupActorCreate.Get());

	Game::Actors::C_VehicleLogicActor *pLogicActor = GetLogicVehicleActor(dropperIndex);

	::Actors::PickupPositionActor *pickup_pos=(::Actors::PickupPositionActor *)pActorList->GetActorPtr(actorHandle);
	pickup_pos->OnMessageSimple(Game::E_GameflowMessage_RaceStarted);
	pickup_pos->OnMessageSimple(Game::E_PickupMessage_ActivatePickup);
	pickup_pos->SetVehicleCollectedLap(dropperIndex, pLogicActor->GetLapInfo().mCurrentLap, Game::VehicleIsLocal(dropperIndex));

	if(pickup_pos->GetPickupType()->meType == E_PICKUP_TYPE_NONE)
		BIZ_ASSERT_FAIL("VLC_Pickups: Spawned a null pickup");

	//Add it to the dropped pickups map
	mDroppedPickupPositionActorMap.Insert(uniqueId, actorHandle);

	return uniqueId;
}

U32 C_RaceLogicLayer::DropDecoyPickup(U32 uniqueId, S32 dropperIndex, E_PICKUP_TYPE decoyType, Maths::Mat43 mat)
{
	Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();

	if(uniqueId == -1)
	{
		uniqueId = Game::C_PickupsManager::Instance().AllocateUniqueId(dropperIndex);
	}

	//mostly copied from PickupShock code
	Actor::C_ActorCreationDataFactoryHelper< ::Actors::PickupPositionDecoyCreate> pPickupActorCreate;
	pPickupActorCreate->m_pPickupType = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupHandle(decoyType);
	pPickupActorCreate->m_iPickupActiveLevel=1;
	pPickupActorCreate->m_OneShot=true;
	pPickupActorCreate->m_bRotateGraphic = true;
	pPickupActorCreate->m_AllowRandomPickupOverride = false;
	pPickupActorCreate->m_bConstantShuffle = false;
	pPickupActorCreate->m_ForceSelectedType = true;
	pPickupActorCreate->m_WorldMatrix=Store(mat);
	pPickupActorCreate->m_UniqueDropID = uniqueId;
	pPickupActorCreate->m_OneShotTime = g_debugDroppedDecoysTimer;
	pPickupActorCreate->m_DroppedFromVehicleIndex = dropperIndex;

	if(pPickupActorCreate->m_pPickupType->meType == E_PICKUP_TYPE_NONE)
		BIZ_ASSERT_FAIL("VLC_Pickups: Dropping a null decoy pickup");

	::Actor::C_ActorHandle actorHandle = pActorList->Spawn(pPickupActorCreate.Get());

	Game::Actors::C_VehicleLogicActor *pLogicActor = GetLogicVehicleActor(dropperIndex);

	::Actors::PickupPositionActorDecoy *pickup_pos_decoy=(::Actors::PickupPositionActorDecoy *)pActorList->GetActorPtr(actorHandle);
	pickup_pos_decoy->OnMessageSimple(Game::E_GameflowMessage_RaceStarted);
	pickup_pos_decoy->OnMessageSimple(Game::E_PickupMessage_ActivatePickup);
	pickup_pos_decoy->SetVehicleCollectedLap(dropperIndex, GetLogicVehicleActor(dropperIndex)->GetLapInfo().mCurrentLap );

	if(pickup_pos_decoy->GetPickupType()->meType == E_PICKUP_TYPE_NONE)
		BIZ_ASSERT_FAIL("VLC_Pickups: Spawned a null decoy pickup");

	//Add it to the dropped pickups map
	mDroppedPickupPositionActorMap.Insert(uniqueId, actorHandle);

	C_VehicleLogicDecoyPickupDropOnTrackMsg msg = C_VehicleLogicDecoyPickupDropOnTrackMsg(dropperIndex);
	pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Abilities, &msg);
	pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Statistics, &msg);

	return uniqueId;
}

// --------------------------------------------------------------------------------------
// COLLISION HANDLING
// --------------------------------------------------------------------------------------

Debug::Float g_debugObjectToObjectMassScale("PhysicsHax.ObjectToObjectMassScale", 1.0f);

Debug::Bool g_debugCollisionImpulseEnable("Vehicle.CollisionImpulse.Enable", false);
Debug::Float g_debugCollisionImpulse("Vehicle.CollisionImpulse.Power", 75000.f);

Debug::Float g_dbGripLossMinImpluseLight	("Vehicle.GripLoss.MinImpluseToTriggerLight", 13000.f, Debug::Range(0.0f, 1000000.0f), Debug::Step(0.001f)); 
Debug::Float g_dbGripLossMinImpluseMedium	("Vehicle.GripLoss.MinImpluseToTriggerMedium", 15000.f, Debug::Range(0.0f, 1000000.0f), Debug::Step(0.001f));
Debug::Float g_dbGripLossMinImpluseHeavy	("Vehicle.GripLoss.MinImpluseToTriggerHeavy", 17000.f, Debug::Range(0.0f, 1000000.0f), Debug::Step(0.001f));

//bool C_RaceLogicLayer::HandleCollisionLogic(const Actor::C_Actor* p_pActorA, const Actor::C_Actor* p_pActorB,
//											Vehicle::CollisionInfo* p_CollisionInfo, SharedPhysics::C_ContactStreamResult* p_pResult)
//{
//	//PERF_EVENT("C_RaceLogicLayer::HandleCollisionLogic");
//
//	using namespace Maths;
//	bool validCollision = true;
//
//	Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
//
//	bool bVehicles = false;
//	bool bDynamicObjects = false;
//	bool bPhysicalObjects = false;
//
//	// Dynamic Object Collisions - TODO: Need to cache these a process later
//
//	Actor::C_ActorTypeId idA = p_pActorA->GetActorTypeId();
//	Actor::C_ActorTypeId idB = p_pActorB->GetActorTypeId();
//
//	if (idA == E_ActorTypeId_DynamicObject)
//	{
//		float fScale = CacheDynamicObjectCollision((::Actors::DynamicObjectActor*)(p_pActorA), const_cast<Actor::C_Actor*>(p_pActorB), p_CollisionInfo);
//		bool con(fScale < 0.f);
//		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
//		validCollision =!con;//(Select(con, 0, 1)!=0);
//		p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);
//
//		bDynamicObjects = true;
//
//		if(mCountObjectsHit && idB == E_ActorTypeId_Vehicle)
//		{
//			if(p_pActorB->GetHandle() == mCountObjectsHandle)
//			{
//				mObjectsHit.Insert(p_pActorA->GetHandle());
//			}
//		}
//	}
//
//	if (idB == E_ActorTypeId_DynamicObject)
//	{
//		float fScale = CacheDynamicObjectCollision((::Actors::DynamicObjectActor*)(p_pActorB), const_cast<Actor::C_Actor*>(p_pActorA), p_CollisionInfo);
//		bool con(fScale < 0.f);
//		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
//		validCollision =!con; //(Select(con, 0, 1)!=0);
//		p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);
//		
//		bDynamicObjects = true;
//
//		if(mCountObjectsHit && idA == E_ActorTypeId_Vehicle)
//		{
//			if(p_pActorA->GetHandle() == mCountObjectsHandle)
//			{
//				mObjectsHit.Insert(p_pActorB->GetHandle());
//			}
//		}
//	}
//
//
//	// Physical Object Collisions
//	
//	if (idA == E_ActorTypeId_PhysicalObject && idB == E_ActorTypeId_Vehicle)
//	{
//		p_CollisionInfo->m_ImpactVelocity = ((const Game::Actors::C_VehicleActor*)p_pActorB)->GetCollisionBody()->LinearVelocity();
//		float fScale = CachePhysicsObjectCollision((::Actors::C_PhysicalObject*)(p_pActorA), const_cast<Actor::C_Actor*>(p_pActorB), p_CollisionInfo);
//		bool con(fScale < 0.f);
//		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
//		validCollision =!con;//(Select(con, 0, 1)!=0);
//		p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);		
//		bPhysicalObjects = true;
//
//		if(mCountObjectsHit)
//		{
//			if(p_pActorB->GetHandle() == mCountObjectsHandle)
//			{
//				mObjectsHit.Insert(p_pActorA->GetHandle());
//			}
//		}
//	}
//
//	if (idB == E_ActorTypeId_PhysicalObject && idA == E_ActorTypeId_Vehicle)
//	{
//		p_CollisionInfo->m_ImpactVelocity = ((const Game::Actors::C_VehicleActor*)p_pActorA)->GetCollisionBody()->LinearVelocity();
//		float fScale = CachePhysicsObjectCollision((::Actors::C_PhysicalObject*)(p_pActorB), const_cast<Actor::C_Actor*>(p_pActorA), p_CollisionInfo);
//		bool con(fScale < 0.f);
//		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
//		validCollision =!con;//(Select(con, 0, 1)!=0);
//		p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);		
//		bPhysicalObjects = true;
//
//		if(mCountObjectsHit)
//		{
//			if(p_pActorA->GetHandle() == mCountObjectsHandle)
//			{
//				mObjectsHit.Insert(p_pActorB->GetHandle());
//			}
//		}
//	}
//	
//	if(!validCollision)
//		return false;
//
//	// Vehicle Collisions
//	//bVehicles = Select((idA ==E_ActorTypeId_Vehicle) | (idB== E_ActorTypeId_Vehicle), 1, 0);
//	if (idA == E_ActorTypeId_Vehicle || idB == E_ActorTypeId_Vehicle)
//	{
//		bVehicles = true;
//	}
//	
//#ifndef CFG_GOLD
//	if ((bPhysicalObjects || bDynamicObjects) && !bVehicles)
//	{
//		p_pResult->m_ObjectAMassScale		= (float)g_debugObjectToObjectMassScale;
//		p_pResult->m_ObjectBMassScale		= (float)g_debugObjectToObjectMassScale;
//	}
//#endif
//
//	// Inertia scaling
//	
//	if(idA == E_ActorTypeId_Vehicle && idB == E_ActorTypeId_Vehicle)
//	{
//		int collision_impulse=0;
//
//		const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)p_pActorA;
//		const Game::Actors::C_VehicleActor* pVehicleB = (const Game::Actors::C_VehicleActor*)p_pActorB;
//		
//		if (g_debugGripLossEnable)
//		{
//			const HF32 vehicleAMomentum	= pVehicleA->GetCollisionBody()->GetBody().m_LinearMomentum.Length2();
//			const HF32 vehicleBMomentum	= pVehicleB->GetCollisionBody()->GetBody().m_LinearMomentum.Length2();
//
//			const HF32 gripMomentumRatio((float)g_debugGripLossMomentumRaio);
//			const HF32 gripMomentum((float)g_debugGripLossMinMomentum * (float)g_debugGripLossMinMomentum);
//
//			const Scalar_Bool con(vehicleAMomentum.GT(vehicleBMomentum));
//			const HF32 aMomentum = Select(con, vehicleAMomentum, vehicleBMomentum);
//			const HF32 bMomentum = Select(con, vehicleBMomentum, vehicleAMomentum);
//			const Game::Actors::C_VehicleActor* vehicleB = (Game::Actors::C_VehicleActor*)Select((bool)con, (U32)pVehicleB, (U32)pVehicleA);
//			const Game::Actors::C_VehicleActor* vehicleA = (Game::Actors::C_VehicleActor*)Select((bool)con, (U32)pVehicleA, (U32)pVehicleB);
//			HF32 fRatio(bMomentum * aMomentum.Reciprocal());
//
//			if((aMomentum.GT(gripMomentum) & fRatio.LE(gripMomentumRatio))  && !vehicleB->LostGrip() )
//			{
//				vehicleB->SendMessageToActor(vehicleB->GetHandle(), E_VehicleMessage_LoseGrip);
//				vehicleA->SendMessageToActor(vehicleA->GetHandle(), E_VehicleMessage_DontLoseGrip);
//				collision_impulse=2;
//			}
//			if((bMomentum.GT(gripMomentum) & fRatio.LE(gripMomentumRatio))  && !vehicleA->LostGrip() )
//			{
//				vehicleA->SendMessageToActor(vehicleA->GetHandle(), E_VehicleMessage_LoseGrip);
//				vehicleB->SendMessageToActor(vehicleB->GetHandle(), E_VehicleMessage_DontLoseGrip);
//				collision_impulse=1;
//			}
//		}
//
//		if(g_debugCollisionImpulseEnable)
//		{
//			if(collision_impulse == 1)	//vehicle A gets it
//			{
//				const HF32 vehicleAMomentum	= pVehicleB->GetCollisionBody()->GetBody().m_LinearMomentum.Length2();
//				
//				Maths::HVec3 va=pVehicleA->GetPosition();
//				Maths::HVec3 vb=pVehicleB->GetPosition();
//						
//				Maths::HVec3 v = va - vb;
//				v = Maths::Normalise(v);
//
//				v = v * ( g_debugCollisionImpulse ) * C_GameApp::GetInstance()->GetTimeStep();	//* vehicleAMomentum );
//
//				pVehicleA->GetCollisionBody()->GetBody().ApplyImpulseLocalSpace(v, pVehicleA->GetCollisionBody()->GetBody().m_CenterOfMassOffset);
//			}
//			if(collision_impulse == 2)	//vehicle B gets it
//			{
//				const HF32 vehicleAMomentum	= pVehicleA->GetCollisionBody()->GetBody().m_LinearMomentum.Length2();
//
//				Maths::HVec3 va=pVehicleA->GetPosition();
//				Maths::HVec3 vb=pVehicleB->GetPosition();
//
//				Maths::HVec3 v = vb - va;
//				v = Maths::Normalise(v);
//
//				v = v * ( g_debugCollisionImpulse ) * C_GameApp::GetInstance()->GetTimeStep();	// * vehicleAMomentum );
//
//				pVehicleB->GetCollisionBody()->GetBody().ApplyImpulseLocalSpace(v, pVehicleB->GetCollisionBody()->GetBody().m_CenterOfMassOffset);
//			}
//		}
//		
//		/*
//			0 == humanToAiInertiaScaleHuman
//			1 == humanToAiInertiaScaleAi
//			2 == humanToHumanInertiaScale
//			3 == aiToAiInertiaScale
//			4 == humanToAiMassScaleHuman
//			5 == humanToAiMassScaleAi
//		*/
//		if (pRLC->IsVehicleAIControlled(pVehicleA->GetVehicleIndex()))
//		{
//			
//			if (pRLC->IsVehicleAIControlled(pVehicleB->GetVehicleIndex()))
//			{
//				p_pResult->m_ObjectAInertiaScaleX = p_pResult->m_ObjectAInertiaScaleY = p_pResult->m_ObjectAInertiaScaleZ = m_dataList[3];//aiToAiInertiaScale;
//				p_pResult->m_ObjectBInertiaScaleX = p_pResult->m_ObjectBInertiaScaleY = p_pResult->m_ObjectBInertiaScaleZ = m_dataList[3];//aiToAiInertiaScale;
//			}
//			else
//			{
//				p_pResult->m_ObjectAInertiaScaleX	= p_pResult->m_ObjectAInertiaScaleY = p_pResult->m_ObjectAInertiaScaleZ = m_dataList[1];//humanToAiInertiaScaleAi;
//				p_pResult->m_ObjectAMassScale		= m_dataList[5];//humanToAiMassScaleAi;	
//				p_pResult->m_ObjectBInertiaScaleX	= p_pResult->m_ObjectBInertiaScaleY = p_pResult->m_ObjectBInertiaScaleZ = m_dataList[0];//humanToAiInertiaScaleHuman;
//				p_pResult->m_ObjectBMassScale		= m_dataList[4];//humanToAiMassScaleHuman;	
//			}
//		}
//		else
//		{
//			
//			if (pRLC->IsVehicleAIControlled(pVehicleB->GetVehicleIndex()))
//			{
//				p_pResult->m_ObjectAInertiaScaleX	= p_pResult->m_ObjectAInertiaScaleY = p_pResult->m_ObjectAInertiaScaleZ = m_dataList[0];//humanToAiInertiaScaleHuman;				
//				p_pResult->m_ObjectAMassScale		= m_dataList[4];//humanToAiMassScaleHuman;	
//				p_pResult->m_ObjectBInertiaScaleX	= p_pResult->m_ObjectBInertiaScaleY = p_pResult->m_ObjectBInertiaScaleZ = m_dataList[1];//humanToAiInertiaScaleAi;
//				p_pResult->m_ObjectBMassScale		= m_dataList[5];//humanToAiMassScaleAi;	
//			}
//			else
//			{
//				p_pResult->m_ObjectAInertiaScaleX = p_pResult->m_ObjectAInertiaScaleY = p_pResult->m_ObjectAInertiaScaleZ = m_dataList[2];//humanToHumanInertiaScale;
//				p_pResult->m_ObjectBInertiaScaleX = p_pResult->m_ObjectBInertiaScaleY = p_pResult->m_ObjectBInertiaScaleZ = m_dataList[2];//humanToHumanInertiaScale;
//			}
//		}
//
//		// Reset last collision timers
//		Game::Actors::C_VehicleLogicActor* pLogicVehA = (Game::Actors::C_VehicleLogicActor*)GetLogicVehicleActor(pVehicleA->GetVehicleIndex());
//		Game::Actors::C_VehicleLogicActor* pLogicVehB = (Game::Actors::C_VehicleLogicActor*)GetLogicVehicleActor(pVehicleB->GetVehicleIndex());
//		
//		if (pLogicVehA && pLogicVehB)
//		{
//			pLogicVehA->mTimeSinceLastCollision[pLogicVehB->GetVehicleIndex()] = 0.0f;
//			pLogicVehB->mTimeSinceLastCollision[pLogicVehA->GetVehicleIndex()] = 0.0f;
//		}
//	}
//
//	/*
//		6 == carToObjectInertiaScaleCar
//		7 == carToObjectInertiaScaleObject
//		8 == carToObjectMassScaleCar
//		9 == carToObjectMassScaleObject
//	*/
//	if (bVehicles && (bDynamicObjects || bPhysicalObjects))
//	{
//		if(idA == E_ActorTypeId_Vehicle)//idA == E_ActorTypeId_Vehicle && idB != E_ActorTypeId_Vehicle
//		{
//			p_pResult->m_ObjectAInertiaScaleX	= p_pResult->m_ObjectAInertiaScaleY = p_pResult->m_ObjectAInertiaScaleZ = m_dataList[6];//carToObjectInertiaScaleCar;
//			p_pResult->m_ObjectAMassScale		= m_dataList[8];//carToObjectMassScaleCar;
//			p_pResult->m_ObjectBInertiaScaleX	= p_pResult->m_ObjectBInertiaScaleY = p_pResult->m_ObjectBInertiaScaleZ = m_dataList[7];//carToObjectInertiaScaleObject;
//			p_pResult->m_ObjectBMassScale		= m_dataList[9];//carToObjectMassScaleObject;
//		}
//		else //idB == E_ActorTypeId_Vehicle && idA != E_ActorTypeId_Vehicle)
//		{
//			p_pResult->m_ObjectAInertiaScaleX	= p_pResult->m_ObjectAInertiaScaleY = p_pResult->m_ObjectAInertiaScaleZ = m_dataList[7];//carToObjectInertiaScaleObject;
//			p_pResult->m_ObjectAMassScale		= m_dataList[9];//carToObjectMassScaleObject;
//			p_pResult->m_ObjectBInertiaScaleX	= p_pResult->m_ObjectBInertiaScaleY = p_pResult->m_ObjectBInertiaScaleZ = m_dataList[6];//carToObjectInertiaScaleCar;
//			p_pResult->m_ObjectBMassScale		= m_dataList[8];//carToObjectMassScaleCar;
//			
//		}
//	}
//
//	return validCollision;
//}

bool C_RaceLogicLayer::HandleCollisionLogic(const Actor::C_Actor* p_pActorA, const Actor::C_Actor* p_pActorB,
											C_ImpactMsg* p_pImpactMsg, SharedPhysics::C_ContactStreamResult* p_pResult, F32& gameImpulseScale)
{
	if (m_RaceState == E_RaceState_Loading)
		return false;

	//PERF_EVENT("C_RaceLogicLayer::HandleCollisionLogic");

	 Vehicle::CollisionInfo* p_CollisionInfo = &p_pImpactMsg->mCollisionInfo;

	using namespace Maths;
	bool validCollision = true;

	Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	bool bVehicles = false;
	bool bDynamicObjects = false;
	bool bPhysicalObjects = false;

	// Dynamic Object Collisions - TODO: Need to cache these a process later

	Actor::C_ActorTypeId idA = p_pActorA->GetActorTypeId();
	Actor::C_ActorTypeId idB = p_pActorB->GetActorTypeId();
	
	//if((idA == E_ActorTypeId_DynamicObject || idA == E_ActorTypeId_PhysicalObject) && 
	//	(idB == E_ActorTypeId_DynamicObject || idB == E_ActorTypeId_PhysicalObject))
	//	{
	//		//Both dynamics, so check to see if a is light and b is static with a static activation threshold
	//		Gravity::C_Body* pBodyA = p_pResult->m_pNodeA->m_pBody;
	//		SharedPhysics::C_GraphNode* pNodeB = p_pResult->m_pNodeB;
	//		if(pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static) &&			// If any are static then it's going to be B
	//		pNodeB->m_Deactivator.m_StaticActivateThresholdSq >= 0.f)
	//		{
	//			//Dynamic object hitting off a static activate body...need to reject this one perhaps?
	//			p_pResult->Invalidate();
	//			return false;				
	//		}
	//	}

	if (idA == E_ActorTypeId_DynamicObject)
	{
		float fScale= CacheDynamicObjectCollision((::Actors::DynamicObjectActor*)(p_pActorA), const_cast<Actor::C_Actor*>(p_pActorB), p_CollisionInfo);
		bool con(fScale < 0.f);
		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
		validCollision =!con;//(Select(con, 0, 1)!=0);
		gameImpulseScale = Select(con, 1.f, fScale);
		//p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);

		bDynamicObjects = true;

		if(mCountObjectsHit && idB == E_ActorTypeId_Vehicle)
		{
			if(p_pActorB->GetHandle() == mCountObjectsHandle)
			{
				mObjectsHit.Insert(p_pActorA->GetHandle());
			}
		}
	}

	if (idB == E_ActorTypeId_DynamicObject)
	{
		float fScale = CacheDynamicObjectCollision((::Actors::DynamicObjectActor*)(p_pActorB), const_cast<Actor::C_Actor*>(p_pActorA), p_CollisionInfo);
		bool con(fScale < 0.f);
		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
		validCollision =!con; //(Select(con, 0, 1)!=0);
		gameImpulseScale = Select(con, 1.f, fScale);
		//p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);
		
		bDynamicObjects = true;

		if(mCountObjectsHit && idA == E_ActorTypeId_Vehicle)
		{
			if(p_pActorA->GetHandle() == mCountObjectsHandle)
			{
				mObjectsHit.Insert(p_pActorB->GetHandle());
			}
		}
	}


	// Physical Object Collisions
	
	if (idA == E_ActorTypeId_PhysicalObject && idB == E_ActorTypeId_Vehicle)
	{
		p_CollisionInfo->m_ImpactVelocity = ((const Game::Actors::C_VehicleActor*)p_pActorB)->GetCollisionBody()->LinearVelocity();
		float fScale = CachePhysicsObjectCollision((::Actors::C_PhysicalObject*)(p_pActorA), const_cast<Actor::C_Actor*>(p_pActorB), p_CollisionInfo);
		bool con(fScale < 0.f);
		validCollision =!con;//(Select(con, 0, 1)!=0);
		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
		gameImpulseScale = Select(con, 1.f, fScale);
		
		//p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);		
		bPhysicalObjects = true;

		if(mCountObjectsHit)
		{
			if(p_pActorB->GetHandle() == mCountObjectsHandle)
			{
				mObjectsHit.Insert(p_pActorA->GetHandle());
			}
		}
	}

	if (idB == E_ActorTypeId_PhysicalObject && idA == E_ActorTypeId_Vehicle)
	{
		p_CollisionInfo->m_ImpactVelocity = ((const Game::Actors::C_VehicleActor*)p_pActorA)->GetCollisionBody()->LinearVelocity();
		float fScale = CachePhysicsObjectCollision((::Actors::C_PhysicalObject*)(p_pActorB), const_cast<Actor::C_Actor*>(p_pActorA), p_CollisionInfo);
		bool con(fScale < 0.f);
		p_pResult->m_ValidData = (SharedPhysics::E_ContactStreamDataValidity)Select(con, SharedPhysics::E_DeletedData, p_pResult->m_ValidData);
		validCollision =!con;//(Select(con, 0, 1)!=0);
		gameImpulseScale = Select(con, 1.f, fScale);
		//p_CollisionInfo->m_GameplayImpulse = Select(con,p_CollisionInfo->m_GameplayImpulse,  p_CollisionInfo->m_GameplayImpulse * fScale);		
		bPhysicalObjects = true;

		if(mCountObjectsHit)
		{
			if(p_pActorA->GetHandle() == mCountObjectsHandle)
			{
				mObjectsHit.Insert(p_pActorB->GetHandle());
			}
		}
	}
	
	if(!validCollision)
		return false;

	// Vehicle Collisions
	//bVehicles = Select((idA ==E_ActorTypeId_Vehicle) | (idB== E_ActorTypeId_Vehicle), 1, 0);
	if (idA == E_ActorTypeId_Vehicle || idB == E_ActorTypeId_Vehicle)
	{
		gameImpulseScale = 1.f;
		bVehicles = true;
	}
	
#if 0 //ndef CFG_GOLD
	if ((bPhysicalObjects || bDynamicObjects) && !bVehicles)
	{
		p_pResult->m_ObjectAMassScale		= (float)g_debugObjectToObjectMassScale;
		p_pResult->m_ObjectBMassScale		= (float)g_debugObjectToObjectMassScale;
	}
#endif

	// Inertia scaling
	
	if(idA == E_ActorTypeId_Vehicle && idB == E_ActorTypeId_Vehicle)
	{
		int collision_impulse=0;

		const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)p_pActorA;
		const Game::Actors::C_VehicleActor* pVehicleB = (const Game::Actors::C_VehicleActor*)p_pActorB;


		const Maths::HF32 lightToHeavyCarScale((F32)g_dbLightToHeavyCarScale);
		const Maths::HF32 mediumToHeavyCarScale((F32)g_dbMediumToHeavyCarScale);
		const Maths::HF32 lightToMediumCarScale((F32)g_dbLightToMediumCarScale);
		const Maths::HF32 lightToUltraCarScale((F32)g_dbLightToUltraCarScale);
		const Maths::HF32 mediumToUltraCarScale((F32)g_dbMediumToUltraCarScale);
		const Maths::HF32 heavyToUltraCarScale((F32)g_dbHeavyToUltraCarScale);

		p_pImpactMsg->mVehicleVelocityA = pVehicleA->GetCollisionBody()->GetBody().m_LinearVelocity;
		p_pImpactMsg->mVehicleVelocityB = pVehicleB->GetCollisionBody()->GetBody().m_LinearVelocity;

		
		/*
			0 == humanToAiInertiaScaleHuman
			1 == humanToAiInertiaScaleAi
			2 == humanToHumanInertiaScale
			3 == aiToAiInertiaScale
			4 == humanToAiMassScaleHuman
			5 == humanToAiMassScaleAi
		*/
	
		if (pRLC->IsVehicleAIControlled(pVehicleA->GetVehicleIndex()))
		{
			if (pRLC->IsVehicleAIControlled(pVehicleB->GetVehicleIndex()))
			{
				p_pResult->m_ObjectAInertiaScale = m_dataList[3];//aiToAiInertiaScale;
				p_pResult->m_ObjectBInertiaScale = m_dataList[3];//aiToAiInertiaScale;
			}
			else
			{
				p_pResult->m_ObjectAInertiaScale	= m_dataList[1];//humanToAiInertiaScaleAi;
				p_pResult->m_ObjectAMassScale		= m_dataList[5];//humanToAiMassScaleAi;	
				p_pResult->m_ObjectBInertiaScale	= m_dataList[0];//humanToAiInertiaScaleHuman;
				p_pResult->m_ObjectBMassScale		= m_dataList[4];//humanToAiMassScaleHuman;

			}
		}
		else
		{
			
			if (pRLC->IsVehicleAIControlled(pVehicleB->GetVehicleIndex()))
			{
				p_pResult->m_ObjectAInertiaScale    = m_dataList[0];//humanToAiInertiaScaleHuman;				
				p_pResult->m_ObjectAMassScale		= m_dataList[4];//humanToAiMassScaleHuman;	
				p_pResult->m_ObjectBInertiaScale	= m_dataList[1];//humanToAiInertiaScaleAi;
				p_pResult->m_ObjectBMassScale		= m_dataList[5];//humanToAiMassScaleAi;

			}
			else
			{
				p_pResult->m_ObjectAInertiaScale	= m_dataList[2];//humanToHumanInertiaScale;
				p_pResult->m_ObjectBInertiaScale	= m_dataList[2];//humanToHumanInertiaScale;
			}
		}

		AmaxVehicleWeight vehicleAWeight = pVehicleA->GetWeightClass();
		AmaxVehicleWeight vehicleBWeight = pVehicleB->GetWeightClass();

		Actors::C_VehicleLogicActor* pLogicActorA = GetLogicVehicleActor(pVehicleA->GetVehicleIndex());
		Actors::C_VehicleLogicActor* pLogicActorB = GetLogicVehicleActor(pVehicleB->GetVehicleIndex());

		if((pLogicActorA->mDoubleShield || pLogicActorA->mSuperShield) && pLogicActorA->mShieldTime > 0.f)
			vehicleAWeight = AmaxVehicleWeight_Ultra;

		if((pLogicActorB->mDoubleShield || pLogicActorB->mSuperShield) && pLogicActorB->mShieldTime > 0.f)
			vehicleBWeight = AmaxVehicleWeight_Ultra;

		if(vehicleAWeight == AmaxVehicleWeight_Light)
		{
			if(vehicleBWeight == AmaxVehicleWeight_Ultra)
			{
				p_pResult->m_ObjectAMassScale *= lightToUltraCarScale;
				p_pResult->m_ObjectAInertiaScale *=lightToUltraCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Medium)
			{
				p_pResult->m_ObjectAMassScale *= lightToMediumCarScale;
				p_pResult->m_ObjectAInertiaScale *=lightToMediumCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Heavy)
			{
				p_pResult->m_ObjectAMassScale *= lightToHeavyCarScale;
				p_pResult->m_ObjectAInertiaScale *=lightToHeavyCarScale;
			}
		}
		else if(vehicleAWeight == AmaxVehicleWeight_Medium)
		{
			if(vehicleBWeight == AmaxVehicleWeight_Ultra)
			{
				p_pResult->m_ObjectAMassScale *= mediumToUltraCarScale;
				p_pResult->m_ObjectAInertiaScale *= mediumToUltraCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Light)
			{
				p_pResult->m_ObjectBMassScale *= lightToMediumCarScale;
				p_pResult->m_ObjectBInertiaScale *= lightToMediumCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Heavy)
			{
				p_pResult->m_ObjectAMassScale *= mediumToHeavyCarScale;
				p_pResult->m_ObjectAInertiaScale *=mediumToHeavyCarScale;
			}
		}
		else if(vehicleAWeight == AmaxVehicleWeight_Heavy)
		{
			if(vehicleBWeight == AmaxVehicleWeight_Ultra)
			{
				p_pResult->m_ObjectAMassScale *= heavyToUltraCarScale;
				p_pResult->m_ObjectAInertiaScale *= heavyToUltraCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Light)
			{
				p_pResult->m_ObjectBMassScale *= lightToHeavyCarScale;
				p_pResult->m_ObjectBInertiaScale *= lightToHeavyCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Medium)
			{
				p_pResult->m_ObjectBMassScale *= mediumToHeavyCarScale;
				p_pResult->m_ObjectBInertiaScale *= mediumToHeavyCarScale;
			}
		}
		else
		{
			if(vehicleBWeight == AmaxVehicleWeight_Light)
			{
				p_pResult->m_ObjectBMassScale *= lightToUltraCarScale;
				p_pResult->m_ObjectBInertiaScale *= lightToUltraCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Medium)
			{
				p_pResult->m_ObjectBMassScale *= mediumToUltraCarScale;
				p_pResult->m_ObjectBInertiaScale *= mediumToUltraCarScale;
			}
			else if(vehicleBWeight == AmaxVehicleWeight_Heavy)
			{
				p_pResult->m_ObjectBMassScale *= heavyToUltraCarScale;
				p_pResult->m_ObjectBInertiaScale *= heavyToUltraCarScale;
			}
		}

	
		/*if(p_pResult->m_pNodeA->m_pRigidBody->Mass() > p_pResult->m_pNodeB->m_pRigidBody->Mass())
		{
			p_pResult->m_ObjectBMassScale *= heavyCarScale;
			p_pResult->m_ObjectBInertiaScale *= heavyCarScale;
		}
		else
		{
			p_pResult->m_ObjectAMassScale *= heavyCarScale;
			p_pResult->m_ObjectAInertiaScale *= heavyCarScale;
		}*/

	}

	/*
		6 == carToObjectInertiaScaleCar
		7 == carToObjectInertiaScaleObject
		8 == carToObjectMassScaleCar
		9 == carToObjectMassScaleObject
		10 == ScaledImpactMaxSpeed
		11 == ScaledImpactMinSpeed
		12 == MinCarToObjectMassValue
	*/

	if (bVehicles && (bDynamicObjects || bPhysicalObjects))
	{
		//LAST BIT - SCALING INERTIA TENSOR/MASS RATIO OF OBJECTS


		F32		inertia_scale_A = 1.f, inertia_scale_B = 1.f;
		F32		mass_scale_A = 1.f, mass_scale_B = 1.f;
		F32		scale = 1.f;
		const Gravity::C_Body* pVehicleBody(NULL);
		const Gravity::C_Body* pDynamicObjectBody(NULL);
		const Maths::HVec3 normal(p_pImpactMsg->mCollisionInfo.m_ContactNormal);


		if(idA == E_ActorTypeId_Vehicle)
		{
			const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)p_pActorA;
			pVehicleBody = pVehicleA->GetCollisionBody();
			const Maths::HF32 vehiclespeed = pVehicleBody->LinearVelocity().Length();
			inertia_scale_A = m_dataList[8];
			mass_scale_A = m_dataList[8];
			inertia_scale_B = m_dataList[9];
			mass_scale_B = m_dataList[9];
			if(idB == E_ActorTypeId_DynamicObject)
			{
				const ::Actors::DynamicObjectActor* pDynamicActor = (const ::Actors::DynamicObjectActor*)p_pActorB;
				pDynamicObjectBody = pDynamicActor->m_pBody;
			}
			else if (idB == E_ActorTypeId_PhysicalObject)
			{
				const ::Actors::C_PhysicalObject* pPhysicalObject = (const ::Actors::C_PhysicalObject*)p_pActorB;
				pDynamicObjectBody = pPhysicalObject->PhysicsBodyComponent()->CollideBody();
			}

			if((!pDynamicObjectBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject)) && 
				pDynamicObjectBody->Mass().LE(m_dataList[12]))
			{
				if (normal.Y().LT(-m_dataList[13]) | vehiclespeed.LT(m_dataList[11]))
				{
					scale = 0.0f;				// Infinate mass if over max speed
				}
				else
				{
					if (vehiclespeed > m_dataList[10])
					{
						scale = 1.0f;			// Normal mass if less than min speed
					}
					else
					{
						scale = 1.0f - (vehiclespeed - m_dataList[11]) / (m_dataList[10] - m_dataList[11]);		// min speed = 1.0f,  max speed = 0.0f
					}
				}
				inertia_scale_A = scale;
				mass_scale_A = scale;
			}
		}
		else
		{
			inertia_scale_B = m_dataList[8];
			mass_scale_B = m_dataList[8];
			inertia_scale_A = m_dataList[9];
			mass_scale_A = m_dataList[9];

			const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)p_pActorB;
			pVehicleBody = pVehicleA->GetCollisionBody();
			const Maths::HF32 vehiclespeed = pVehicleBody->LinearVelocity().Length();
			if(idA == E_ActorTypeId_DynamicObject)
			{
				const ::Actors::DynamicObjectActor* pDynamicActor = (const ::Actors::DynamicObjectActor*)p_pActorA;
				pDynamicObjectBody = pDynamicActor->m_pBody;
			}
			else if (idA == E_ActorTypeId_PhysicalObject)
			{
				const ::Actors::C_PhysicalObject* pPhysicalObject = (const ::Actors::C_PhysicalObject*)p_pActorA;
				pDynamicObjectBody = pPhysicalObject->PhysicsBodyComponent()->CollideBody();
			}
			

			if(!(pDynamicObjectBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject) || 
				pDynamicObjectBody->Mass().GT(m_dataList[12])))
			{
				if (normal.Y().GT(m_dataList[13]) | vehiclespeed.LT(m_dataList[11]))
				{
					scale = 0.0f;				// Infinate mass if over max speed
				}
				else
				{
					if (vehiclespeed > m_dataList[10])
					{
						scale = 1.0f;			// Normal mass if less than min speed
					}
					else
					{
						scale = 1.0f - (vehiclespeed - m_dataList[11]) / (m_dataList[10] - m_dataList[11]);		// min speed = 1.0f,  max speed = 0.0f
					}
				}
				inertia_scale_B = scale;
				mass_scale_B = scale;
			}
		}


		p_pResult->m_ObjectAInertiaScale	= inertia_scale_A;
		p_pResult->m_ObjectAMassScale		= mass_scale_A;
		p_pResult->m_ObjectBInertiaScale	= inertia_scale_B;
		p_pResult->m_ObjectBMassScale		= mass_scale_B;
	}

	return validCollision;
}


float C_RaceLogicLayer::CacheDynamicObjectCollision(::Actors::DynamicObjectActor* p_pObjActor,
											Actor::C_Actor* p_pColliderActor, 
											Vehicle::CollisionInfo* p_pCollisionInfo)
{
	if (m_RaceState == E_RaceState_Loading)
		return 0.0f;

	float fScale = ((::Actors::DynamicObjectActor*)p_pObjActor)->RegisterCollision(p_pColliderActor, p_pCollisionInfo);

	if(fScale > 0.f && (p_pColliderActor && p_pColliderActor->GetActorTypeId() == E_ActorTypeId_Vehicle))
	{
		m_CachedObjectCollisions.PushBack();
		m_CachedObjectCollisions.Back().m_pObjectActor	= p_pObjActor;
		m_CachedObjectCollisions.Back().m_pColliderActor	= p_pColliderActor;
		m_CachedObjectCollisions.Back().m_CollisionInfo	= *p_pCollisionInfo;
	}

	return fScale;
}

float C_RaceLogicLayer::CachePhysicsObjectCollision(::Actors::C_PhysicalObject* p_pObjActor,
											Actor::C_Actor* p_pColliderActor, 
											Vehicle::CollisionInfo* p_pCollisionInfo)
{
	if (m_RaceState == E_RaceState_Loading)
		return 0.0f;

	float fScale = ((::Actors::C_PhysicalObject*)p_pObjActor)->RegisterCollision(p_pColliderActor, p_pCollisionInfo);

	if(fScale > 0.f && (p_pColliderActor && p_pColliderActor->GetActorTypeId() == E_ActorTypeId_Vehicle))
	{
		m_CachedObjectCollisions.PushBack();
		m_CachedObjectCollisions.Back().m_pObjectActor	= p_pObjActor;
		m_CachedObjectCollisions.Back().m_pColliderActor	= p_pColliderActor;
		m_CachedObjectCollisions.Back().m_CollisionInfo	= *p_pCollisionInfo;
	}

	return fScale;
}

void C_RaceLogicLayer::ProcessObjectCollisions()
{
	PERF_EVENT("C_RaceLogicLayer.ProcessObjectCollisions");

	for (S32 iCollision = 0; iCollision < m_CachedObjectCollisions.Size(); iCollision++)
	{
		Actor::C_Actor* pObjActor = m_CachedObjectCollisions[iCollision].m_pObjectActor;
		Actor::C_Actor* pColliderActor = m_CachedObjectCollisions[iCollision].m_pColliderActor;

		if (pObjActor->GetActorTypeId() == E_ActorTypeId_DynamicObject)
			((::Actors::DynamicObjectActor*)pObjActor)->ProcessCollision(m_CachedObjectCollisions[iCollision].m_pColliderActor, &m_CachedObjectCollisions[iCollision].m_CollisionInfo);
		else if (pObjActor->GetActorTypeId() == E_ActorTypeId_PhysicalObject && (pColliderActor && pColliderActor->GetActorTypeId() == E_ActorTypeId_Vehicle))
			((::Actors::C_PhysicalObject*)pObjActor)->ProcessCollision(m_CachedObjectCollisions[iCollision].m_pColliderActor, &m_CachedObjectCollisions[iCollision].m_CollisionInfo);
	}

	m_CachedObjectCollisions.Clear();
}

void C_RaceLogicLayer::ProcessFluidEvents(Gravity::C_FluidEvent* m_pFluidEvents, U32 m_NumFluidEvents)
{
	if (!m_pFluidEvents)
		return;

	for (U32 iEvent = 0; iEvent < m_NumFluidEvents; iEvent++)
	{
		Gravity::C_FluidEvent* pEvent = &m_pFluidEvents[iEvent];
		if (pEvent && pEvent->m_pBody && pEvent->m_pBody->m_pBody)
		{
			C_BodyUserDataBase* pUserData = (C_BodyUserDataBase*)pEvent->m_pBody->m_pBody->GetUserData<void>();
			if (pUserData && pUserData->m_eType == E_BodyUserDataType_Actor)
			{
				const Actor::C_Actor* pActor = m_pGameActorList->GetActorPtr(((C_BodyUserDataActor*)pUserData)->m_hActorHandle);
				if (pActor && pActor->GetActorTypeId() == E_ActorTypeId_Vehicle)
				{					
					Game::Actors::C_VehicleActor* pVehicle = const_cast<Game::Actors::C_VehicleActor*>((Game::Actors::C_VehicleActor*)pActor);
					pVehicle->RegisterWaterCollision(pEvent->m_SurfacePoint);
				}
			}
		}
	}
}

Debug::Bool g_debugThrowAwayNegativeImpulses("Vehicle.Damage.ThrowAwayNegativeImpulses", false);
Debug::Float g_dbYVelThreshold("Vehicle.GamePhysicsMechanics.WallRiding.YVelocityThreshold", 2.f, Debug::Range(-10.f, 10.f));
Debug::Float g_dbWallDragCoefficient("Vehicle.GamePhysicsMechanics.WallDragCoefficient", 450.f, Debug::Range(0.f, 1000.f));

void C_RaceLogicLayer::ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter, SharedPhysics::TCollisionCallbackPhase phase)
{
	if (!m_pRLDamageSystem)
		return;

	PERF_EVENT("C_RaceLogicLayer.CollisionCallback");
	using namespace Maths;

	const RLInertiaScale& inertiaScale	= m_pRLDamageSystem->mParameters->mIntraVehicularCollisionInertiaScale;

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 eps = Constants<HF32>::Epsilon();
	const HVec3 positiveY(Constants<HVec3>::UnitAxisPositiveY());

	const HF32 cosThirty((float)g_dbWallRidingUpAngle);
	const HF32 peturbThreshold((float)g_dbWallRiddingCosAngle); 
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

	const HF32 maxStaticImpulse(m_pRLDamageSystem->mParameters->mMaxStaticImpulse);
	const HF32 maxDynamicImpulse(m_pRLDamageSystem->mParameters->mMaxDynamicImpulse);

	const HF32 maxWorldDamageImpulse(m_pRLDamageSystem->mParameters->mMaxWorldDamageImpulse);
	const HF32 maxWorldEffectImpulse(m_pRLDamageSystem->mParameters->mMaxWorldEffectImpulse);
	const HF32 maxVehicleDamageImpulse(m_pRLDamageSystem->mParameters->mMaxVehicleDamageImpulse);
	const HF32 maxVehicleEffectImpulse(m_pRLDamageSystem->mParameters->mMaxVehicleEffectImpulse);
	const HF32 maxObjectDamageImpulse(m_pRLDamageSystem->mParameters->mMaxObjectDamageImpulse);
	const HF32 maxObjectEffectImpulse(m_pRLDamageSystem->mParameters->mMaxObjectEffectImpulse);

	//const Maths::HF32 dampingCoefficient((F32)g_dbDampingCoefficient);
	
	const HF32 dampingImpulseThreshold((F32)g_debugCollisionDampingImpulseThreshold);
	const HF32 dampingImpulseMax((F32)g_debugCollisionDampingImpulseMax);
	const HF32 dampingCoeffMin((F32)g_dbDampingCoefficientMin);
	const HF32 dampingCoeffMax((F32)g_dbDampingCoefficientMax);
	const HF32 dampingImpulseRange(dampingImpulseMax - dampingImpulseThreshold);
	const HF32 reciprocalDampingImpulseRange(dampingImpulseRange.Reciprocal());
	const HF32 dampingCoeffRange(dampingCoeffMax - dampingCoeffMin);

	const HF32 thumanToAiInertiaScaleHuman(Load<HF32>(&inertiaScale.mHumanToAiInertiaScaleHuman));
	const HF32 thumanToAiInertiaScaleAi(Load<HF32>(&inertiaScale.mHumanToAiInertiaScaleAi));
	const HF32 thumanToHumanInertiaScale(Load<HF32>(&inertiaScale.mHumanToHumanInertiaScale));
	const HF32 taiToAiInertiaScale(Load<HF32>(&inertiaScale.mAiToAiInertiaScale));
	const HF32 thumanToAiMassScaleHuman(Load<HF32>(&inertiaScale.mHumanToAiMassScaleHuman));
	const HF32 thumanToAiMassScaleAi(Load<HF32>(&inertiaScale.mHumanToAiMassScaleAi));

	const HF32 tcarToObjectInertiaScaleCar(Load<HF32>(&inertiaScale.mCarToObjectInertiaScaleCar));
	const HF32 tcarToObjectInertiaScaleObject(Load<HF32>(&inertiaScale.mCarToObjectInertiaScaleObject));
	const HF32 tcarToObjectMassScaleCar(Load<HF32>(&inertiaScale.mCarToObjectMassScaleCar));
	const HF32 tcarToObjectMassScaleObject(Load<HF32>(&inertiaScale.mCarToObjectMassScaleObject));

	const HF32 tScaledImpactMaxSpeed(Load<HF32>(&inertiaScale.mScaledImpactMaxSpeed));
	const HF32 tScaledImpactMinSpeed(Load<HF32>(&inertiaScale.mScaledImpactMinSpeed));
	const HF32 tMinCarToObjectMassValue(Load<HF32>(&inertiaScale.mMinCarToObjectMassValue));

	const HF32 humanToAiInertiaScaleHuman = Select(thumanToAiInertiaScaleHuman == zero, one, thumanToAiInertiaScaleHuman);
	const HF32 humanToAiInertiaScaleAi = Select(thumanToAiInertiaScaleAi == zero, one, thumanToAiInertiaScaleAi);
	const HF32 humanToHumanInertiaScale = Select(thumanToHumanInertiaScale == zero, one, thumanToHumanInertiaScale);
	const HF32 aiToAiInertiaScale = Select(taiToAiInertiaScale == zero, one, taiToAiInertiaScale);
	const HF32 humanToAiMassScaleHuman = Select(thumanToAiMassScaleHuman == zero, one, thumanToAiMassScaleHuman);
	const HF32 humanToAiMassScaleAi = Select(thumanToAiMassScaleAi == zero, one, thumanToAiMassScaleAi);

	const HF32 carToObjectInertiaScaleCar = Select(tcarToObjectInertiaScaleCar == zero, one,  tcarToObjectInertiaScaleCar);
	const HF32 carToObjectInertiaScaleObject = Select(tcarToObjectInertiaScaleObject == zero, one, tcarToObjectInertiaScaleObject);
	const HF32 carToObjectMassScaleCar = Select(tcarToObjectMassScaleCar == zero, one, tcarToObjectMassScaleCar);
	const HF32 carToObjectMassScaleObject = Select(tcarToObjectMassScaleObject == zero, one,  tcarToObjectMassScaleObject);
	const HF32 objectNormalDotY((F32)g_dbObjectNormalDotY);

	const HF32 thresholdSpeedSq((F32)g_dbCarTunnelSpeed * (F32)g_dbCarTunnelSpeed);

	const HF32 yVelThreshold((F32)g_dbYVelThreshold);


	//C_SoceryDataList dataList;
	m_dataList.m_Parameters[0]=humanToAiInertiaScaleHuman.Reciprocal();
	m_dataList.m_Parameters[1]=humanToAiInertiaScaleAi.Reciprocal();
	m_dataList.m_Parameters[2]=humanToHumanInertiaScale.Reciprocal();
	m_dataList.m_Parameters[3]=aiToAiInertiaScale.Reciprocal();
	m_dataList.m_Parameters[4]=humanToAiMassScaleHuman.Reciprocal();
	m_dataList.m_Parameters[5]=humanToAiMassScaleAi.Reciprocal();
	m_dataList.m_Parameters[6]=carToObjectInertiaScaleCar.Reciprocal();
	m_dataList.m_Parameters[7]=carToObjectInertiaScaleObject.Reciprocal();
	m_dataList.m_Parameters[8]=carToObjectMassScaleCar.Reciprocal();
	m_dataList.m_Parameters[9]=carToObjectMassScaleObject.Reciprocal();

	m_dataList.m_Parameters[10]=tScaledImpactMaxSpeed * 0.446f;			// Convert mph to m/s	Does nothing if this is <= 0.0f
	m_dataList.m_Parameters[11]=tScaledImpactMinSpeed * 0.446f;			// Convert mph to m/s
	m_dataList.m_Parameters[12]=tMinCarToObjectMassValue;
	m_dataList.m_Parameters[13]=objectNormalDotY;

	F32 bumgartOverride = (float)g_dbBaumgarteOverride;
	F32 minPen = (float)g_dbMinPenetration;

	U32 MaxObjectMessages = (U32)g_dbMaxObjectsMessages;
	U32 MaxVehicleMessages = (U32)g_dbMaxVehicleMessages;
	bool throwAwayMessage = (bool)g_debugThrowAwayMessages;
	bool enableWallRiding = (bool)g_debugEnableWallRidingFix2;
	const HF32 wallDragCoefficient((float)g_dbWallDragCoefficient);

	HF32 impulse(UNDEFINED);
	HVec3 friction(UNDEFINED);


	C_ImpactMsg impactMsg;

	U32 numObjectAndObjectCollisions =0;
	Actor::C_ActorHandle nullHandle;

	while(iter.hasNextContact())
	{
		SharedPhysics::C_ContactStreamResult* result(iter.nextContact());
		// We have to do both bodies at once, because otherwise the iterator continues and loses track.
		Gravity::C_Body* pBodyA(result->m_pNodeA->m_pBody);
		Gravity::C_Body* pBodyB(result->m_pNodeB->m_pBody);
		
	
		impactMsg.mActorHandleA = nullHandle;
		impactMsg.mActorHandleB = nullHandle;
		impactMsg.mCollisionInfo.Clear();
		impactMsg.mDamageFilter.Reset();

		HF32 maxImpulse(zero);

		const HF32 massA(pBodyA->Mass());
		const HF32 massB(pBodyB->Mass());


		U32 bodyAStatic = Select((bool)(massA).LT(eps), 1, 0);
		U32 bodyBStatic = Select((bool)(massB).LT(eps), 1, 0);

		HF32 maxImpulseRange_Damage = maxStaticImpulse;
		HF32 maxImpulseRange_Effect = maxStaticImpulse;


		const Scalar_Bool con1(!bodyAStatic && !bodyBStatic);
		const HF32 maxImpulseRange = Select(con1, maxDynamicImpulse, maxStaticImpulse);
		const HF32 maxImpulseRangeReciprocal(maxImpulseRange.Reciprocal());

		const Actor::C_Actor* pActorA		= NULL;
		const Actor::C_Actor* pActorB		= NULL;

		C_BodyUserDataBase* pUserDataA = (C_BodyUserDataBase*)pBodyA->GetUserData<void>();
		if (pUserDataA && pUserDataA->m_eType == E_BodyUserDataType_Actor)
		{
			impactMsg.mActorHandleA = ((C_BodyUserDataActor*)pUserDataA)->m_hActorHandle;
			pActorA = m_pGameActorList->GetActorPtr(impactMsg.mActorHandleA);
		}
		
		C_BodyUserDataBase* pUserDataB = (C_BodyUserDataBase*)pBodyB->GetUserData<void>();
		if (pUserDataB && pUserDataB->m_eType == E_BodyUserDataType_Actor)
		{
			impactMsg.mActorHandleB = ((C_BodyUserDataActor*)pUserDataB)->m_hActorHandle;
			pActorB = m_pGameActorList->GetActorPtr(impactMsg.mActorHandleB);
		}


		bool vehicleActorA(pActorA && pActorA->GetActorTypeId() == E_ActorTypeId_Vehicle);
		bool vehicleActorB(pActorB && pActorB->GetActorTypeId() == E_ActorTypeId_Vehicle);
		bool bothActorExist(pActorA && pActorB);

		
		if( throwAwayMessage & (!vehicleActorA) & (!vehicleActorB))
		{
			if(numObjectAndObjectCollisions > MaxObjectMessages)
				continue;
			numObjectAndObjectCollisions++;
		}
		
		const Scalar_Bool con4(vehicleActorA);
		const Scalar_Bool con5(vehicleActorB);

		maxImpulseRange_Damage = Select(con4 |con5, maxVehicleDamageImpulse,  maxObjectDamageImpulse);
		maxImpulseRange_Effect = Select(con4 |con5, maxVehicleEffectImpulse,  maxObjectEffectImpulse);

		const HF32 maxImpulseRange_DamageReciprocal = maxImpulseRange_Damage.Reciprocal();
		const HF32 maxImpulseRange_EffectReciprocal = maxImpulseRange_Effect.Reciprocal();

		// ----------------- HANDLE PROJECTILES ----------------- 
		if(HandleProjectiles(pBodyA, pBodyB))
		{
			//We've dealt with the collision so we don't need to carry on with this result
			result->Invalidate();
			continue;
		}
		// ------------------------------------------------


		impactMsg.mMatrixA		= pBodyA->ElementMatrix();
		impactMsg.mMatrixB		= pBodyB->ElementMatrix();

		bool validCollision = true;
		bool invalidateForDamping = false;

		F32 gameimpulseScale =1.f;

		const HF32 gripLossMinImpulseLight((F32)g_dbGripLossMinImpluseLight);
		const HF32 gripLossMinImpulseMedium((F32)g_dbGripLossMinImpluseMedium);
		const HF32 gripLossMinImpulseHeavy((F32)g_dbGripLossMinImpluseHeavy);

		bool bSkipTraversal = false;
		if(vehicleActorA && vehicleActorB)
		{

			SharedPhysics::C_RigidBody& bodyA = pBodyA->GetBody();
			SharedPhysics::C_RigidBody& bodyB = pBodyB->GetBody();

			const Maths::HVec3 linearVelA(bodyA.m_LinearVelocity);
			const Maths::HVec3 linearVelB(bodyB.m_LinearVelocity);
			const Maths::HVec3 relVel(linearVelB - linearVelA);
			const Maths::HF32 relSpeedSq(relVel | relVel);
			if(relSpeedSq > thresholdSpeedSq)
			{
				//OK. We have a high-speed relative collision.
				//We need to work out if the cars tunneled and fix it up!
				//We also need to clear out the existing contacts and make just one continuous contact!
				//(1) Do a continuous test!
				Collision::C_Shape* pShapeA = pBodyA->GetShape();
				Collision::C_Shape* pShapeB = pBodyB->GetShape();
				Gravity::C_Scene* pScene = pBodyA->GetScene();

				const HVec3 transA(linearVelA * iter.timeStep());
				const HVec3 transB(linearVelB * iter.timeStep());
				Maths::HF32 lambda;
				Maths::HVec3 normal(Maths::UNDEFINED), contactPoint(Maths::UNDEFINED);

				if(pScene->ContinuousCheckBoundingBox(pShapeA, pShapeB, transA, transB, 
					lambda, normal, contactPoint))
				{
					//lambda should be the t-value for the time of impact...
					if(lambda < 1.f)
					{
						bSkipTraversal = true;
						const Maths::HVec3 posA(bodyA.m_Position - (transA * (one - lambda)));
						const Maths::HVec3 posB(bodyB.m_Position - (transB * (one - lambda)));

						bodyA.m_Position = posA;
						bodyB.m_Position = posB;

						const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)pActorA;
						const Game::Actors::C_VehicleActor* pVehicleB = (const Game::Actors::C_VehicleActor*)pActorB;

						pVehicleA->SendMessageToActor(pVehicleA->GetHandle(), E_VehicleMessage_KillNitro);
						pVehicleB->SendMessageToActor(pVehicleB->GetHandle(), E_VehicleMessage_KillNitro);

						//Now - we load up the contacts...

						if(iter.hasNextPatch())
						{
							SharedPhysics::C_ContactStreamPatch* pPatch(iter.nextPatch());
							Maths::Store(normal, &pPatch->m_Normal);

							if(iter.hasNextPoint())
							{
								SharedPhysics::C_ContactStreamPoint* pPoint(iter.nextPoint());
								Maths::Store(contactPoint, &pPoint->m_ContactPoint);
								pPoint->m_PenetrationDepth = 0.f;

								iter.GetImpulseAndFriction(impulse, friction, true);

								const HF32 frictionLength(friction.Length());

								maxImpulse = Max(impulse, maxImpulse);
								
								impactMsg.mCollisionInfo.m_ContactPoints.PushBack();
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_ContactType		= (U8)iter.GetContactType();
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_Friction			= friction;
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint		= contactPoint;
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PenetrationDepth	= 0.f;			
								impactMsg.mCollisionInfo.m_GameplayFriction							= frictionLength;
								impactMsg.mCollisionInfo.m_AudioFriction							= frictionLength * massA.Reciprocal();

								float fImpulse = (F32)Clamp(impulse*maxImpulseRangeReciprocal, zero, one);

								//float fImpulse														= Maths::Clamp((float)(impulse / maxImpulseRange), 0.0f, 1.0f);
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_Impulse			= normal * impulse;
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PtVelA			= pBodyA->PointVelocity(contactPoint);
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PtVelB			= pBodyB->PointVelocity(contactPoint);
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_MassA				= pBodyA->Mass();
								impactMsg.mCollisionInfo.m_ContactPoints.Back().m_MassB				= pBodyB->Mass();
								impactMsg.mCollisionInfo.m_GameplayImpulse							= Max(fImpulse, impactMsg.mCollisionInfo.m_GameplayImpulse);

								float fDamageImpulse												= (F32)Clamp((impulse*maxImpulseRange_DamageReciprocal), zero, one);
								impactMsg.mCollisionInfo.m_DamageImpulse							= Max(fDamageImpulse, impactMsg.mCollisionInfo.m_DamageImpulse);
								float fEffectImpulse												= (F32)Clamp((impulse*maxImpulseRange_EffectReciprocal), zero, one);
								impactMsg.mCollisionInfo.m_EffectImpulse							= Max(fEffectImpulse, impactMsg.mCollisionInfo.m_EffectImpulse);

								impactMsg.mHashValue												= iter.currentPoint()->m_HashValue;
								impactMsg.mCollisionInfo.m_PhysicsMaterialA							= iter.currentPoint()->m_PhysicsMaterialIndices & 0xffff;
								impactMsg.mCollisionInfo.m_PhysicsMaterialB							= iter.currentPoint()->m_PhysicsMaterialIndices >> 16;
								impactMsg.mCollisionInfo.m_EffectMaterialA							= iter.currentPoint()->m_EffectMaterialIndices & 0xffff;
								impactMsg.mCollisionInfo.m_EffectMaterialB							= iter.currentPoint()->m_EffectMaterialIndices >> 16;

								HandleCollisionLogic(pActorA, pActorB, &impactMsg, result, gameimpulseScale);
								
								impactMsg.mCollisionInfo.m_GameplayImpulse = impactMsg.mCollisionInfo.m_GameplayImpulse * gameimpulseScale;

							}
							while(iter.hasNextPoint())
							{
								SharedPhysics::C_ContactStreamPoint* pPoint(iter.nextPoint());
								pPoint->Invalidate();
							}
						}
						while(iter.hasNextPatch())
						{
							SharedPhysics::C_ContactStreamPatch* pPatch(iter.nextPatch());
							pPatch->Invalidate();
						}
					}
				}
			}
		}

		if(!bSkipTraversal)
		{
			HF32 scale(Maths::Constants<Maths::HF32>::Zero());
			while( iter.hasNextPatch() ) 
			{
				SharedPhysics::C_ContactStreamPatch* patch(iter.nextPatch());

				const HVec3 norm(Load<HVec3>(&patch->m_Normal));
				const HVec3 nNorm(-norm);
				impactMsg.mCollisionInfo.m_ContactNormal	= norm;
				impactMsg.mCollisionInfo.m_InvalidatePhysicsReaction = false;
				F32 minPenetration = 0.f;
				bool wallCollision = false;
				bool air = false;

				//if (g_debugEnableWallRidingFix2 && pActorA && pActorA->GetActorTypeId() == E_ActorTypeId_Vehicle && !pActorB)
				if (enableWallRiding & vehicleActorA & (pActorB == NULL))
				{		
					const Game::Actors::C_VehicleActor* pVehicle = (const Game::Actors::C_VehicleActor*)pActorA;
					class RacingPhysics::Car* pCar					= pVehicle->GetVehiclePhysics();

					const HF32 wallRidingNormalDotUpThreshold(g_debugWallRidingNormalDotUpThreshold2);
					const HF32 fNdotUp(Maths::Abs(nNorm | positiveY));
					

					if (fNdotUp.LT(wallRidingNormalDotUpThreshold))// && !pCar->Air())
					{
						//New version - project onto the local X and Z axes...test to see if its over the
						//angle limit, in which case, keep original...Could also be done by setting Y component of above formula to 0
						//bScalePenetration = !pCar->Air();
						//result->m_BaumgarteOverride = (float)g_dbBaumgarteOverride;
						result->m_BaumgarteOverride = bumgartOverride;
						const HMat33 rot(pBodyA->Orientation());
						air = pCar->Air();
						if(rot.AY().Y() > cosThirty)
						{
							//minPenetration = Maths::Select(!pCar->Air(), (F32)g_dbMinPenetration, minPenetration);
							minPenetration = Maths::Select(!air, minPen, minPenetration);
							const HF32 xScale(rot.AX() | norm);
							const HF32 zScale(rot.AZ() | norm);
							HVec3 worldNormal(rot.AX() * xScale + rot.AZ() * zScale);
							worldNormal.Normalise();
							Store(Maths::Select((worldNormal | norm).GT(peturbThreshold), worldNormal, norm), &patch->m_Normal);
						}
						wallCollision = true;

						SharedPhysics::C_RigidBody& body = pBodyA->GetBody();
						const C_Normal dirZ(body.m_OrientationMatrix.AZ());
						const HF32 scale0(Abs(dirZ | norm));
						BIZ_ASSERT(scale0 <= one);
						scale = Maths::Max(scale, one - scale0);
					}
				}
				
				{
					while( iter.hasNextPoint() )
					{
						SharedPhysics::C_ContactStreamPoint* point(iter.nextPoint());

						if(wallCollision)
						{
							point->m_PenetrationDepth = Maths::Max(minPenetration, point->m_PenetrationDepth);
							if(!air || pBodyA->GetBody().m_LinearVelocity.Y() > yVelThreshold)
							{
								point->m_PhysicsMaterialIndices = Game::E_PhysicsMaterial_Vehicle | (Game::E_PhysicsMaterial_Anisotropic_Wall << 16);
							}
							else
							{
								point->m_PhysicsMaterialIndices = Game::E_PhysicsMaterial_Vehicle | (Game::E_PhysicsMaterial_Anisotropic_ZeroY << 16);
							}
						}	

						/*if(vehicleActorA && pActorB == NULL)
						{
							DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(Maths::Load<Maths::HVec3>(&point->m_ContactPoint), 0.1f, Maths::RGBA(255, 0, 0));
						}*/

						

						iter.GetImpulseAndFriction(impulse, friction, true);

						maxImpulse = Max(impulse, maxImpulse);

						if(impactMsg.mCollisionInfo.m_ContactPoints.Size() < 4)
						{

							const HF32 frictionLength(friction.Length());
							const HVec3 contactPoint(Load<HVec3>(&point->m_ContactPoint));

							impactMsg.mCollisionInfo.m_ContactPoints.PushBack();
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_ContactType		= (U8)iter.GetContactType();
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_Friction			= friction;
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint		= contactPoint;
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PenetrationDepth	= point->m_PenetrationDepth;			
							impactMsg.mCollisionInfo.m_PenerationDepth							+= point->m_PenetrationDepth;
							impactMsg.mCollisionInfo.m_GameplayFriction							= frictionLength;
							impactMsg.mCollisionInfo.m_AudioFriction							= frictionLength * massA.Reciprocal();

							float fImpulse = (F32)Clamp(impulse*maxImpulseRangeReciprocal, zero, one);

							//float fImpulse														= Maths::Clamp((float)(impulse / maxImpulseRange), 0.0f, 1.0f);
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_Impulse			= norm * impulse;
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PtVelA			= pBodyA->PointVelocity(contactPoint);
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PtVelB			= pBodyB->PointVelocity(contactPoint);
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_MassA				= pBodyA->Mass();
							impactMsg.mCollisionInfo.m_ContactPoints.Back().m_MassB				= pBodyB->Mass();
							impactMsg.mCollisionInfo.m_GameplayImpulse							= Max(fImpulse, impactMsg.mCollisionInfo.m_GameplayImpulse);

							float fDamageImpulse												= (F32)Clamp((impulse*maxImpulseRange_DamageReciprocal), zero, one);
							impactMsg.mCollisionInfo.m_DamageImpulse							= Max(fDamageImpulse, impactMsg.mCollisionInfo.m_DamageImpulse);
							float fEffectImpulse												= (F32)Clamp((impulse*maxImpulseRange_EffectReciprocal), zero, one);
							impactMsg.mCollisionInfo.m_EffectImpulse							= Max(fEffectImpulse, impactMsg.mCollisionInfo.m_EffectImpulse);

							impactMsg.mHashValue												= iter.currentPoint()->m_HashValue;
							impactMsg.mCollisionInfo.m_PhysicsMaterialA							= iter.currentPoint()->m_PhysicsMaterialIndices & 0xffff;
							impactMsg.mCollisionInfo.m_PhysicsMaterialB							= iter.currentPoint()->m_PhysicsMaterialIndices >> 16;
							impactMsg.mCollisionInfo.m_EffectMaterialA							= iter.currentPoint()->m_EffectMaterialIndices & 0xffff;
							impactMsg.mCollisionInfo.m_EffectMaterialB							= iter.currentPoint()->m_EffectMaterialIndices >> 16;
						}

						
						if(vehicleActorA && vehicleActorB)
						{
							const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)pActorA;
							const Game::Actors::C_VehicleActor* pVehicleB = (const Game::Actors::C_VehicleActor*)pActorB;
							const HVec3 actorAXAxis = pBodyA->GetBody().m_OrientationMatrix.AX();
							const HVec3 actorBXAxis = pBodyB->GetBody().m_OrientationMatrix.AX();
							const HF32 impluseA = maxImpulse * Abs((norm | actorAXAxis));
							const HF32 impluseB = maxImpulse * Abs((norm | actorBXAxis));
							const Scalar_Bool con(impluseA > impluseB);
							const HF32 finalImpluse = Select(con, impluseA, impluseB);
							const Game::Actors::C_VehicleActor* pVehicle = (bool)con ? pVehicleA : pVehicleB;	// [Blur2PC][x64] was a U32 Select of the two pointers
							
							const AmaxVehicleWeight vehicleWeightClass = (bool)con ? pVehicleA->GetWeightClass() : pVehicleB->GetWeightClass();

							const HF32 gripLossMinImpulse((vehicleWeightClass == AmaxVehicleWeight_Light) ? gripLossMinImpulseLight : (vehicleWeightClass == AmaxVehicleWeight_Medium) ? gripLossMinImpulseMedium : gripLossMinImpulseHeavy);
							
							
							if(finalImpluse > gripLossMinImpulse)
							{
								//Fudge materials here to get a more punchy contact for Ged! treat these contacts as bouncy contacts!
								//point->m_PhysicsMaterialIndices = (Game::E_PhysicsMaterial_ObjectBouncy << 16) | Game::E_PhysicsMaterial_Vehicle;

								if(!pVehicle->LostGrip())
								{
									pVehicle->SendMessageToActor(pVehicle->GetHandle(), E_VehicleMessage_LoseGrip);
								}
							}
						}

					}
					impactMsg.mCollisionInfo.m_GameplayImpulse = impactMsg.mCollisionInfo.m_GameplayImpulse * gameimpulseScale;
					
					//if((!g_debugGripLossEnable) & (!g_debugGripLossEnable2) & bothActorExist)
				}

				if(bothActorExist)
					validCollision = HandleCollisionLogic(pActorA, pActorB, &impactMsg, result, gameimpulseScale);
				if(!validCollision)
				{
					break;
				}
			}

			SharedPhysics::C_RigidBody& body = pBodyA->GetBody();

			float dragSpeedScale = 1.f;
			if(mpWallRidingSystem && vehicleActorA)
			{
				Actors::C_VehicleActor* pVehActor = (Actors::C_VehicleActor*)m_pGameActorList->GetActorPtr(((C_BodyUserDataActor*)pUserDataA)->m_hActorHandle);
				int vehIdx = pVehActor->GetVehicleIndex();

				float classModifier = 1.f;
				if(vehIdx >= 0 && vehIdx < mWallRidingScalingFactorList.Size())
					classModifier = mWallRidingScalingFactorList[vehIdx];

				float speedInMph = body.m_LinearVelocity.Length() * 2.2374f * classModifier;
				dragSpeedScale = mpWallRidingSystem->mWallRidingCurve.Evaluate(speedInMph);
			}

			const HVec3 drag = body.m_LinearVelocity * wallDragCoefficient * scale * dragSpeedScale;
			body.AddForce(-drag);
		}

			
		if(validCollision && impactMsg.mCollisionInfo.m_ContactPoints.Size() > 0)
		{
			impactMsg.mCollisionInfo.m_PhysicsMaterial = impactMsg.mCollisionInfo.m_PhysicsMaterialB;
			impactMsg.mCollisionInfo.m_EffectMaterial = (U16)Select(impactMsg.mCollisionInfo.m_PhysicsMaterialB == Game::E_PhysicsMaterial_Vehicle, Game::E_EffectMaterial_Vehicle, impactMsg.mCollisionInfo.m_EffectMaterialB);
			impactMsg.mCollisionInfo.m_PrimaryVehicle = (impactMsg.mCollisionInfo.m_PhysicsMaterialA == Game::E_PhysicsMaterial_Vehicle && impactMsg.mCollisionInfo.m_PhysicsMaterialB == Game::E_PhysicsMaterial_Vehicle) ||
														(impactMsg.mCollisionInfo.m_PhysicsMaterialA == Game::E_PhysicsMaterial_Vehicle && impactMsg.mCollisionInfo.m_PhysicsMaterialB != Game::E_PhysicsMaterial_Vehicle);
			if (!CacheImpactMessage(pBodyA, impactMsg, maxImpulse))
				SendImpactMessagePatch(pBodyA, impactMsg, maxImpulse);


			impactMsg.mCollisionInfo.m_PhysicsMaterial = impactMsg.mCollisionInfo.m_PhysicsMaterialA;
			impactMsg.mCollisionInfo.m_EffectMaterial = (U16)Select(impactMsg.mCollisionInfo.m_PhysicsMaterialA == Game::E_PhysicsMaterial_Vehicle, Game::E_EffectMaterial_Vehicle, impactMsg.mCollisionInfo.m_EffectMaterialA);
			impactMsg.mCollisionInfo.m_PrimaryVehicle = (impactMsg.mCollisionInfo.m_PhysicsMaterialA != Game::E_PhysicsMaterial_Vehicle && impactMsg.mCollisionInfo.m_PhysicsMaterialB == Game::E_PhysicsMaterial_Vehicle);
			impactMsg.mCollisionInfo.m_ContactNormal = -impactMsg.mCollisionInfo.m_ContactNormal;
			if (!CacheImpactMessage(pBodyB, impactMsg, maxImpulse))
				SendImpactMessagePatch(pBodyB, impactMsg, maxImpulse);
		}
	}
}

const bool C_RaceLogicLayer::HandleProjectiles(const Gravity::C_Body *pBodyA, const Gravity::C_Body *pBodyB) const 
{
	C_BodyUserDataBase* pUserDataA = (C_BodyUserDataBase*)pBodyA->GetUserData<void>();
	C_BodyUserDataBase* pUserDataB = (C_BodyUserDataBase*)pBodyB->GetUserData<void>();

	//Check we've got valid user data for both bodies, there's no point in continuing if we can't interogate the user data so we'll early out
	if(!pUserDataA || !pUserDataB)
		return false;

	const Actor::C_Actor* pActorA = NULL;
	const Actor::C_Actor* pActorB = NULL;

	bool bodyAIsVehicle = false;
	bool bodyBIsVehicle = false;

	if(pUserDataA->m_eType == E_BodyUserDataType_Actor)
	{
		pActorA = m_pGameActorList->GetActorPtr(((C_BodyUserDataActor*)pUserDataA)->m_hActorHandle);
		bodyAIsVehicle = (pActorA && pActorA->GetActorTypeId() == E_ActorTypeId_Vehicle);
	}

	if (pUserDataB->m_eType == E_BodyUserDataType_Actor)
	{
		pActorB = m_pGameActorList->GetActorPtr(((C_BodyUserDataActor*)pUserDataB)->m_hActorHandle);
		bodyBIsVehicle = (pActorB && pActorB->GetActorTypeId() == E_ActorTypeId_Vehicle);
	}

	bool isMineA = pUserDataA->m_eType == E_BodyUserDataType_MineInner || pUserDataA->m_eType == E_BodyUserDataType_MineOuter;
	bool isMineB = pUserDataB->m_eType == E_BodyUserDataType_MineInner || pUserDataB->m_eType == E_BodyUserDataType_MineOuter;

	if(isMineA && isMineB)
		return true;

	//Check that either of the bodies is a vehicle, no point in continuing if not
	if(!bodyAIsVehicle && !bodyBIsVehicle)
		return false;

	int vehicleIndex = INVALID_VEHICLE_IDX;
	if(bodyBIsVehicle && isMineA)
	{
		vehicleIndex = ((Actors::C_VehicleActor*)pActorB)->GetVehicleIndex();
		return CheckMineCollision(pUserDataA, vehicleIndex);
	}

	if(bodyAIsVehicle && isMineB)
	{
		vehicleIndex = ((Actors::C_VehicleActor*)pActorA)->GetVehicleIndex();
		return CheckMineCollision(pUserDataB, vehicleIndex);
	}

	//Not handled
	return false;
}

const bool C_RaceLogicLayer::CheckMineCollision(const C_BodyUserDataBase* pUserData, const int victimVehicleIndex) const	// [Blur2PC] was default-int
{
	if( pUserData->m_eType == E_BodyUserDataType_MineInner || pUserData->m_eType == E_BodyUserDataType_MineOuter )
	{
		C_PickupMine* pMine = ((C_BodyUserDataMine*)pUserData)->m_pMineObject;
		if(!pMine)
			return false;

		if(pUserData->m_eType == E_BodyUserDataType_MineInner)
			//'Splode
			pMine->Trigger(victimVehicleIndex);
		else
			//Tense ready to 'splode
			pMine->Prime(victimVehicleIndex);
					
		return true;
	}

	return false;
}


Debug::Int g_debugMaxPhysicsBodyEventsToProcess("Physics.MaxPhysicsBodyEventsToProcess", 8);

void C_RaceLogicLayer::ProcessEvents(SharedPhysics::C_MemoryIterator<Gravity::C_BodyEvent>& eventIter, const U32 numEvents)
{
	PERF_EVENT(" C_RaceLogicLayer.ProcessEvents");

	C_GameWorld* pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();

	for (U32 a = 0; a < numEvents; ++a)
	{
		if (m_PhysicsBodyEvents.Size() >= MAX_CACHED_BODY_EVENTS)
			return;
		
		const Gravity::C_BodyEvent* bodyEvent = eventIter.Get();
		
		//Not interested in other events - enter/exit water + remove constraint!
		if (bodyEvent->GetEventType() == Gravity::C_BodyEvent::E_GoToSleep || bodyEvent->GetEventType() == Gravity::C_BodyEvent::E_WakeUp)
		{
			Gravity::C_Body* pBody = bodyEvent->GetBody();
			if (pBody)
			{
				Actor::C_ActorHandle actorHandle;
				C_BodyUserDataBase* pUserData	= (C_BodyUserDataBase*)pBody->GetUserData<void>();
				if (pUserData && pUserData->m_eType == E_BodyUserDataType_Actor)
				{
					actorHandle = ((C_BodyUserDataActor*)pUserData)->m_hActorHandle;

					if (bodyEvent->GetEventType() == Gravity::C_BodyEvent::E_GoToSleep ||
						(bodyEvent->GetEventType() == Gravity::C_BodyEvent::E_WakeUp && pBody->GetGraphNode()->m_Deactivator.m_StaticActivateThresholdSq <= 0.0f))
					{
						C_PhysicsBodyEventMsg msg(*bodyEvent);
						pGameWorld->GetActorList()->SendMessageToActor(actorHandle, &msg);
					}
					else
					{
						m_PhysicsBodyEvents.PushBack(C_CachedBodyEvent(*bodyEvent, actorHandle));
					}
				}
			}
		}
	}
}

void C_RaceLogicLayer::ProcessCachedEvents()
{
	if (m_PhysicsBodyEvents.Empty())
		return;

	C_GameWorld* pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();

	if (!pGameWorld)
	{
		m_PhysicsBodyEvents.Clear();
		return;
	}

	PERF_EVENT(" C_RaceLogicLayer.ProcessCachedEvents");

	CachedBodyList::Iterator it;
	S32 count = 0;
	for (it = m_PhysicsBodyEvents.Begin(); it != m_PhysicsBodyEvents.End(); ++it, count++)
	{
		if (count > g_debugMaxPhysicsBodyEventsToProcess)
			break;

		Gravity::C_BodyEvent* bodyEvent = &it->m_BodyEvent;
		Actor::C_ActorHandle* actorHandle = &it->m_ActorHandle;
		C_PhysicsBodyEventMsg msg(it->m_BodyEvent);
		pGameWorld->GetActorList()->SendMessageToActor(it->m_ActorHandle, &msg);
	}

	for (S32 i = 0; i < count; i++)
		m_PhysicsBodyEvents.PopFront();
}

//KierS - old Version
//void C_RaceLogicLayer::ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter)
//{
//	if (!m_pRLDamageSystem)
//		return;
//
//	PERF_EVENT("C_RaceLogicLayer.CollisionCallback");
//	using namespace Maths;
//
//	const RLInertiaScale& inertiaScale	= m_pRLDamageSystem->mParameters->mIntraVehicularCollisionInertiaScale;
//
//	const HF32 zero(Constants<HF32>::Zero());
//	const HF32 one(Constants<HF32>::One());
//	const HF32 eps = Constants<HF32>::Epsilon();
//	const HVec3 positiveY(Constants<HVec3>::UnitAxisPositiveY());
//
//	const HF32 cosThirty(0.866f); //30 degrees
//	const HF32 peturbThreshold((float)g_dbWallRiddingCosAngle); 
//	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
//
//	const HF32 maxStaticImpulse(m_pRLDamageSystem->mParameters->mMaxStaticImpulse);
//	const HF32 maxDynamicImpulse(m_pRLDamageSystem->mParameters->mMaxDynamicImpulse);
//
//	const HF32 maxWorldDamageImpulse(m_pRLDamageSystem->mParameters->mMaxWorldDamageImpulse);
//	const HF32 maxWorldEffectImpulse(m_pRLDamageSystem->mParameters->mMaxWorldEffectImpulse);
//	const HF32 maxVehicleDamageImpulse(m_pRLDamageSystem->mParameters->mMaxVehicleDamageImpulse);
//	const HF32 maxVehicleEffectImpulse(m_pRLDamageSystem->mParameters->mMaxVehicleEffectImpulse);
//	const HF32 maxObjectDamageImpulse(m_pRLDamageSystem->mParameters->mMaxObjectDamageImpulse);
//	const HF32 maxObjectEffectImpulse(m_pRLDamageSystem->mParameters->mMaxObjectEffectImpulse);
//
//	//const Maths::HF32 dampingCoefficient((F32)g_dbDampingCoefficient);
//	
//	const HF32 dampingImpulseThreshold((F32)g_debugCollisionDampingImpulseThreshold);
//	const HF32 dampingImpulseMax((F32)g_debugCollisionDampingImpulseMax);
//	const HF32 dampingCoeffMin((F32)g_dbDampingCoefficientMin);
//	const HF32 dampingCoeffMax((F32)g_dbDampingCoefficientMax);
//	const HF32 dampingImpulseRange(dampingImpulseMax - dampingImpulseThreshold);
//	const HF32 reciprocalDampingImpulseRange(dampingImpulseRange.Reciprocal());
//	const HF32 dampingCoeffRange(dampingCoeffMax - dampingCoeffMin);
//
//	const HF32 thumanToAiInertiaScaleHuman(Load<HF32>(&inertiaScale.mHumanToAiInertiaScaleHuman));
//	const HF32 thumanToAiInertiaScaleAi(Load<HF32>(&inertiaScale.mHumanToAiInertiaScaleAi));
//	const HF32 thumanToHumanInertiaScale(Load<HF32>(&inertiaScale.mHumanToHumanInertiaScale));
//	const HF32 taiToAiInertiaScale(Load<HF32>(&inertiaScale.mAiToAiInertiaScale));
//	const HF32 thumanToAiMassScaleHuman(Load<HF32>(&inertiaScale.mHumanToAiMassScaleHuman));
//	const HF32 thumanToAiMassScaleAi(Load<HF32>(&inertiaScale.mHumanToAiMassScaleAi));
//
//	const HF32 tcarToObjectInertiaScaleCar(Load<HF32>(&inertiaScale.mCarToObjectInertiaScaleCar));
//	const HF32 tcarToObjectInertiaScaleObject(Load<HF32>(&inertiaScale.mCarToObjectInertiaScaleObject));
//	const HF32 tcarToObjectMassScaleCar(Load<HF32>(&inertiaScale.mCarToObjectMassScaleCar));
//	const HF32 tcarToObjectMassScaleObject(Load<HF32>(&inertiaScale.mCarToObjectMassScaleObject));
//
//	const HF32 tScaledImpactMaxSpeed(Load<HF32>(&inertiaScale.mScaledImpactMaxSpeed));
//	const HF32 tScaledImpactMinSpeed(Load<HF32>(&inertiaScale.mScaledImpactMinSpeed));
//	const HF32 tMinCarToObjectMassValue(Load<HF32>(&inertiaScale.mMinCarToObjectMassValue));
//
//	const HF32 humanToAiInertiaScaleHuman = Select(thumanToAiInertiaScaleHuman == zero, one, thumanToAiInertiaScaleHuman);
//	const HF32 humanToAiInertiaScaleAi = Select(thumanToAiInertiaScaleAi == zero, one, thumanToAiInertiaScaleAi);
//	const HF32 humanToHumanInertiaScale = Select(thumanToHumanInertiaScale == zero, one, thumanToHumanInertiaScale);
//	const HF32 aiToAiInertiaScale = Select(taiToAiInertiaScale == zero, one, taiToAiInertiaScale);
//	const HF32 humanToAiMassScaleHuman = Select(thumanToAiMassScaleHuman == zero, one, thumanToAiMassScaleHuman);
//	const HF32 humanToAiMassScaleAi = Select(thumanToAiMassScaleAi == zero, one, thumanToAiMassScaleAi);
//
//	const HF32 carToObjectInertiaScaleCar = Select(tcarToObjectInertiaScaleCar == zero, one,  tcarToObjectInertiaScaleCar);
//	const HF32 carToObjectInertiaScaleObject = Select(tcarToObjectInertiaScaleObject == zero, one, tcarToObjectInertiaScaleObject);
//	const HF32 carToObjectMassScaleCar = Select(tcarToObjectMassScaleCar == zero, one, tcarToObjectMassScaleCar);
//	const HF32 carToObjectMassScaleObject = Select(tcarToObjectMassScaleObject == zero, one,  tcarToObjectMassScaleObject);
//
//
//	//C_SoceryDataList dataList;
//	m_dataList.m_Parameters[0]=humanToAiInertiaScaleHuman.Reciprocal();
//	m_dataList.m_Parameters[1]=humanToAiInertiaScaleAi.Reciprocal();
//	m_dataList.m_Parameters[2]=humanToHumanInertiaScale.Reciprocal();
//	m_dataList.m_Parameters[3]=aiToAiInertiaScale.Reciprocal();
//	m_dataList.m_Parameters[4]=humanToAiMassScaleHuman.Reciprocal();
//	m_dataList.m_Parameters[5]=humanToAiMassScaleAi.Reciprocal();
//	m_dataList.m_Parameters[6]=carToObjectInertiaScaleCar.Reciprocal();
//	m_dataList.m_Parameters[7]=carToObjectInertiaScaleObject.Reciprocal();
//	m_dataList.m_Parameters[8]=carToObjectMassScaleCar.Reciprocal();
//	m_dataList.m_Parameters[9]=carToObjectMassScaleObject.Reciprocal();
//
//	m_dataList.m_Parameters[10]=tScaledImpactMaxSpeed * 0.446f;			// Convert mph to m/s	Does nothing if this is <= 0.0f
//	m_dataList.m_Parameters[11]=tScaledImpactMinSpeed * 0.446f;			// Convert mph to m/s
//	m_dataList.m_Parameters[12]=tMinCarToObjectMassValue;
//
//
//	F32 bumgartOverride = (float)g_dbBaumgarteOverride;
//	F32 minPen = (float)g_dbMinPenetration;
//
//	U32 MaxObjectMessages = (U32)g_dbMaxObjectsMessages;
//	U32 MaxVehicleMessages = (U32)g_dbMaxVehicleMessages;
//	bool throwAwayMessage = (bool)g_debugThrowAwayMessages;
//	bool enableWallRiding = (bool)g_debugEnableWallRidingFix2;
//
//
//	C_ImpactMsg impactMsg;
//	//impactMsg.mCollisionInfo.m_ContactPoints.Reserve(25);
//
//	U32 numObjectAndObjectCollisions =0;
//
//	while(iter.hasNextContact())
//	{
//		SharedPhysics::C_ContactStreamResult* result(iter.nextContact());
//		// We have to do both bodies at once, because otherwise the iterator continues and loses track.
//		Gravity::C_Body* pBodyA(result->m_pNodeA->m_pBody);
//		Gravity::C_Body* pBodyB(result->m_pNodeB->m_pBody);
//		
//	
//		impactMsg.mCollisionInfo.Clear();
//		impactMsg.mDamageFilter.Reset();
//
//		HF32 maxImpulse(zero);
//
//		const HF32 massA(pBodyA->Mass());
//		const HF32 massB(pBodyB->Mass());
//
//
//		U32 bodyAStatic = Select((bool)(massA).LT(eps), 1, 0);
//		U32 bodyBStatic = Select((bool)(massB).LT(eps), 1, 0);
//
//		HF32 maxImpulseRange_Damage = maxStaticImpulse;
//		HF32 maxImpulseRange_Effect = maxStaticImpulse;
//
//
//		const Scalar_Bool con1(!bodyAStatic && !bodyBStatic);
//		const HF32 maxImpulseRange = Select(con1, maxDynamicImpulse, maxStaticImpulse);
//		const HF32 maxImpulseRangeReciprocal(maxImpulseRange.Reciprocal());
//
//		const Actor::C_Actor* pActorA		= NULL;
//		const Actor::C_Actor* pActorB		= NULL;
//
//		C_BodyUserDataBase* pUserDataA = (C_BodyUserDataBase*)pBodyA->GetUserData<void>();
//		if (pUserDataA && pUserDataA->m_eType == E_BodyUserDataType_Actor)
//		{
//			impactMsg.mActorHandleA = ((C_BodyUserDataActor*)pUserDataA)->m_hActorHandle;
//			pActorA = m_pGameActorList->GetActorPtr(impactMsg.mActorHandleA);
//		}
//		
//		C_BodyUserDataBase* pUserDataB = (C_BodyUserDataBase*)pBodyB->GetUserData<void>();
//		if (pUserDataB && pUserDataB->m_eType == E_BodyUserDataType_Actor)
//		{
//			impactMsg.mActorHandleB = ((C_BodyUserDataActor*)pUserDataB)->m_hActorHandle;
//			pActorB = m_pGameActorList->GetActorPtr(impactMsg.mActorHandleB);
//		}
//
//
//		bool vehicleActorA(pActorA && pActorA->GetActorTypeId() == E_ActorTypeId_Vehicle);
//		bool vehicleActorB(pActorB && pActorB->GetActorTypeId() == E_ActorTypeId_Vehicle);
//		bool bothActorExist(pActorA && pActorB);
//
//		
//		if( throwAwayMessage & (!vehicleActorA) & (!vehicleActorB))
//		{
//			if(numObjectAndObjectCollisions > MaxObjectMessages)
//				continue;
//			numObjectAndObjectCollisions++;
//		}
//		
//		const Scalar_Bool con4(vehicleActorA);
//		const Scalar_Bool con5(vehicleActorB);
//
//		maxImpulseRange_Damage = Select(con4 |con5, maxVehicleDamageImpulse,  maxObjectDamageImpulse);
//		maxImpulseRange_Effect = Select(con4 |con5, maxVehicleEffectImpulse,  maxObjectEffectImpulse);
//
//		const HF32 maxImpulseRange_DamageReciprocal = maxImpulseRange_Damage.Reciprocal();
//		const HF32 maxImpulseRange_EffectReciprocal = maxImpulseRange_Effect.Reciprocal();
//
//		// ----------------- HANDLE PROJECTILES ----------------- 
//		if( pUserDataA && pUserDataA->m_eType == E_BodyUserDataType_Mine )
//		{
//			if(vehicleActorB)
//			{
//				C_PickupMine* pMine = ((C_BodyUserDataMine*)pUserDataA)->m_pMineObject;
//				int vehIdx = ((Actors::C_VehicleActor*)pActorB)->GetVehicleIndex();
//
//				if(pMine)
//					pMine->Trigger(vehIdx);
//
//				result->Invalidate();
//			}
//			
//			continue;
//		}
//
//		if( pUserDataB && pUserDataB->m_eType == E_BodyUserDataType_Mine )
//		{
//			if(vehicleActorA)
//			{
//				C_PickupMine* pMine = ((C_BodyUserDataMine*)pUserDataB)->m_pMineObject;
//				int vehIdx = ((Actors::C_VehicleActor*)pActorA)->GetVehicleIndex();
//
//				if(pMine)
//					pMine->Trigger(vehIdx);
//
//				result->Invalidate();
//			}
//				
//			continue;
//		}
//		// ------------------------------------------------
//
//
//		impactMsg.mMatrixA		= pBodyA->ElementMatrix();
//		impactMsg.mMatrixB		= pBodyB->ElementMatrix();
//
//		bool validCollision = true;
//		bool invalidateForDamping = false;
//
//		while( iter.hasNextPatch() ) 
//		{
//			SharedPhysics::C_ContactStreamPatch* patch(iter.nextPatch());
//
//			const HVec3 norm(Load<HVec3>(&patch->m_Normal));
//			const HVec3 nNorm(-norm);
//			impactMsg.mCollisionInfo.m_ContactNormal	= norm;
//			impactMsg.mCollisionInfo.m_InvalidatePhysicsReaction = false;
//			F32 minPenetration = 0.f;
//
//			//if (g_debugEnableWallRidingFix2 && pActorA && pActorA->GetActorTypeId() == E_ActorTypeId_Vehicle && !pActorB)
//			if (enableWallRiding & vehicleActorA & (pActorB == NULL))
//			{		
//				const Game::Actors::C_VehicleActor* pVehicle = (const Game::Actors::C_VehicleActor*)pActorA;
//				class RacingPhysics::Car* pCar					= pVehicle->GetVehiclePhysics();
//
//				const HF32 wallRidingNormalDotUpThreshold(g_debugWallRidingNormalDotUpThreshold2);
//				const HF32 fNdotUp(nNorm | positiveY);
//
//				if ((fNdotUp.LT(wallRidingNormalDotUpThreshold) && fNdotUp.GT(-wallRidingNormalDotUpThreshold)))// && !pCar->Air())
//				{
//					//New version - project onto the local X and Z axes...test to see if its over the
//					//angle limit, in which case, keep original...Could also be done by setting Y component of above formula to 0
//					//bScalePenetration = !pCar->Air();
//					//result->m_BaumgarteOverride = (float)g_dbBaumgarteOverride;
//					result->m_BaumgarteOverride = bumgartOverride;
//					const HMat33 rot(pBodyA->Orientation());
//					//if(rot.AY().Y() > cosThirty)
//					if(rot.AY().Y().GT(cosThirty))
//					{
//						//minPenetration = Maths::Select(!pCar->Air(), (F32)g_dbMinPenetration, minPenetration);
//						minPenetration = Maths::Select(!pCar->Air(), minPen, minPenetration);
//						const HF32 xScale(rot.AX() | norm);
//						const HF32 zScale(rot.AZ() | norm);
//						HVec3 worldNormal(rot.AX() * xScale + rot.AZ() * zScale);
//						worldNormal.Normalise();
//						Store(Maths::Select((worldNormal | norm).GT(peturbThreshold), worldNormal, norm), &patch->m_Normal);
//					}
//				}
//			}
//
//
//			F32 gameimpulseScale =1.f;
//			if(bothActorExist)
//				validCollision = HandleCollisionLogic(pActorA, pActorB, &impactMsg, result, gameimpulseScale);
//
//			
//
//			if(!validCollision)
//			{
//				break;
//			}
//			else
//			{
//				HF32 impulse(UNDEFINED);
//				HVec3 friction(UNDEFINED);
//
//				const HF32 gripLossMinImpulse((float) g_dbGripLossMinImpluse2);
//				while( iter.hasNextPoint() )
//				{
//					SharedPhysics::C_ContactStreamPoint* point(iter.nextPoint());
//
//					point->m_PenetrationDepth = Maths::Max(minPenetration, point->m_PenetrationDepth);
//
//					iter.GetImpulseAndFriction(impulse, friction, true);
//
//					maxImpulse = Max(impulse, maxImpulse);
//
//					const HF32 frictionLength(friction.Length());
//					const HVec3 contactPoint(Load<HVec3>(&point->m_ContactPoint));
//
//					impactMsg.mCollisionInfo.m_ContactPoints.PushBack();
//					impactMsg.mCollisionInfo.m_ContactPoints.Back().m_ContactType		= (U8)iter.GetContactType();
//					impactMsg.mCollisionInfo.m_ContactPoints.Back().m_Friction			= friction;
//					impactMsg.mCollisionInfo.m_ContactPoints.Back().m_ContactPoint		= contactPoint;
//					impactMsg.mCollisionInfo.m_ContactPoints.Back().m_PenetrationDepth	= point->m_PenetrationDepth;			
//					impactMsg.mCollisionInfo.m_PenerationDepth							+= point->m_PenetrationDepth;
//					impactMsg.mCollisionInfo.m_GameplayFriction							= frictionLength;
//					impactMsg.mCollisionInfo.m_AudioFriction							= frictionLength * massA.Reciprocal();
//
//					float fImpulse = (F32)Clamp(impulse*maxImpulseRangeReciprocal, zero, one);
//
//					//float fImpulse														= Maths::Clamp((float)(impulse / maxImpulseRange), 0.0f, 1.0f);
//					impactMsg.mCollisionInfo.m_ContactPoints.Back().m_Impulse			= fImpulse;
//					impactMsg.mCollisionInfo.m_GameplayImpulse							= Max(fImpulse, impactMsg.mCollisionInfo.m_GameplayImpulse);
//
//					float fDamageImpulse												= (F32)Clamp((impulse*maxImpulseRange_DamageReciprocal), zero, one);
//					impactMsg.mCollisionInfo.m_DamageImpulse							= Max(fDamageImpulse, impactMsg.mCollisionInfo.m_DamageImpulse);
//					float fEffectImpulse												= (F32)Clamp((impulse*maxImpulseRange_EffectReciprocal), zero, one);
//					impactMsg.mCollisionInfo.m_EffectImpulse							= Max(fEffectImpulse, impactMsg.mCollisionInfo.m_EffectImpulse);
//
//					impactMsg.mHashValue												= iter.currentPoint()->m_HashValue;
//					impactMsg.mCollisionInfo.m_PhysicsMaterialA							= iter.currentPoint()->m_PhysicsMaterialIndices & 0xffff;
//					impactMsg.mCollisionInfo.m_PhysicsMaterialB							= iter.currentPoint()->m_PhysicsMaterialIndices >> 16;
//					impactMsg.mCollisionInfo.m_EffectMaterialA							= iter.currentPoint()->m_EffectMaterialIndices & 0xffff;
//					impactMsg.mCollisionInfo.m_EffectMaterialB							= iter.currentPoint()->m_EffectMaterialIndices >> 16;
//				}
//				impactMsg.mCollisionInfo.m_GameplayImpulse = impactMsg.mCollisionInfo.m_GameplayImpulse * gameimpulseScale;
//				
//				//if((!g_debugGripLossEnable) & (!g_debugGripLossEnable2) & bothActorExist)
//				if(bothActorExist)
//				{
//					Actor::C_ActorTypeId idA = pActorA->GetActorTypeId();
//					Actor::C_ActorTypeId idB = pActorB->GetActorTypeId();
//					if(idA == E_ActorTypeId_Vehicle && idB == E_ActorTypeId_Vehicle)
//					{
//						const Game::Actors::C_VehicleActor* pVehicleA = (const Game::Actors::C_VehicleActor*)pActorA;
//						const Game::Actors::C_VehicleActor* pVehicleB = (const Game::Actors::C_VehicleActor*)pActorB;
//						const HVec3 actorAXAxis = pBodyA->GetBody().m_OrientationMatrix.AX();
//						const HVec3 actorBXAxis = pBodyB->GetBody().m_OrientationMatrix.AX();
//						const HF32 impluseA = maxImpulse * Abs((norm | actorAXAxis));
//						const HF32 impluseB = maxImpulse * Abs((norm | actorBXAxis));
//						const Scalar_Bool con(impluseA > impluseB);
//						const HF32 finalImpluse = Select(con, impluseA, impluseB);
//						const Game::Actors::C_VehicleActor* pVehicle = (bool)con ? pVehicleA : pVehicleB;	// [Blur2PC][x64] was a U32 Select of the two pointers
//						
//						if(finalImpluse.GT(gripLossMinImpulse) && ! pVehicle->LostGrip())
//						{
//							pVehicle->SendMessageToActor(pVehicle->GetHandle(), E_VehicleMessage_LoseGrip);
//						}
//					}
//				}
//			}
//		}
//			
//		if(validCollision && impactMsg.mCollisionInfo.m_ContactPoints.Size() > 0)
//		{
//			impactMsg.mCollisionInfo.m_PhysicsMaterial = impactMsg.mCollisionInfo.m_PhysicsMaterialB;
//			impactMsg.mCollisionInfo.m_EffectMaterial = (U16)Select(impactMsg.mCollisionInfo.m_PhysicsMaterialB == Game::E_PhysicsMaterial_Vehicle, Game::E_EffectMaterial_Vehicle, impactMsg.mCollisionInfo.m_EffectMaterialB);
//			if (!CacheImpactMessage(pBodyA, impactMsg, maxImpulse))
//				SendImpactMessagePatch(pBodyA, impactMsg, maxImpulse);
//
//
//			impactMsg.mCollisionInfo.m_PhysicsMaterial = impactMsg.mCollisionInfo.m_PhysicsMaterialA;
//			impactMsg.mCollisionInfo.m_EffectMaterial = (U16)Select(impactMsg.mCollisionInfo.m_PhysicsMaterialA == Game::E_PhysicsMaterial_Vehicle, Game::E_EffectMaterial_Vehicle, impactMsg.mCollisionInfo.m_EffectMaterialA);
//			impactMsg.mCollisionInfo.m_ContactNormal = -impactMsg.mCollisionInfo.m_ContactNormal;
//			if (!CacheImpactMessage(pBodyB, impactMsg, maxImpulse))
//				SendImpactMessagePatch(pBodyB, impactMsg, maxImpulse);
//		}
//	}
//}


void C_RaceLogicLayer::SendImpactMessagePoint(Gravity::C_Body* pBody, C_ImpactMsg& impactMsg, float impulse)
{
	return;

	Actor::C_ActorHandle actorHandle;
	const Actor::C_Actor* pActor	= NULL;
	C_BodyUserDataBase* pUserData	= (C_BodyUserDataBase*)pBody->GetUserData<void>();
	if (pUserData && pUserData->m_eType == E_BodyUserDataType_Actor)
	{
		actorHandle = ((C_BodyUserDataActor*)pUserData)->m_hActorHandle;
		pActor = m_pGameActorList->GetActorPtr(actorHandle);
	}
		
	if(pActor && pActor->GetActorTypeId() == E_ActorTypeId_Vehicle)
	{
		//impactMsg.mCollisionInfo.m_AudioImpulse = impulse / pBody->Mass();
		impactMsg.mCollisionInfo.m_AudioImpulse = impulse * pBody->ReciprocalMass();

		m_pGameActorList->SendMessageToActor(actorHandle,&impactMsg);
	}
}

Debug::Int g_debugMaxCollisionsToCache("Amax.Collisions.MaxEventsToCache", 300, Debug::Range(0, 1000), Debug::Step(1));

bool C_RaceLogicLayer::CacheImpactMessage(Gravity::C_Body* pBody, C_ImpactMsg& impactMsg, float impulse)
{
	if (g_bDisableSPUDamage || m_CachedImpactMessages.Size() >= g_debugMaxCollisionsToCache)
		return false;

	if (m_RaceState == E_RaceState_Loading)
		return true;

	m_CachedImpactMessages.PushBack();
	m_CachedImpactMessages.Back().m_pBody			= pBody;
	m_CachedImpactMessages.Back().m_ImpactMessage	= impactMsg;
	m_CachedImpactMessages.Back().m_Impulse			= impulse;	

	return true;
}

void C_RaceLogicLayer::ProcessImpactMessages()
{	
	if (g_bDisableSPUDamage) return;
	
	PERF_EVENT("C_RaceLogicLayer.ProcessImpactMessages");

	for (S32 iMsg = 0; iMsg < m_CachedImpactMessages.Size(); iMsg++)
	{
		SendImpactMessagePatch(	m_CachedImpactMessages[iMsg].m_pBody, 
								m_CachedImpactMessages[iMsg].m_ImpactMessage,
								m_CachedImpactMessages[iMsg].m_Impulse);
	}
}

void C_RaceLogicLayer::SendImpactMessagePatch(Gravity::C_Body* pBody, C_ImpactMsg& impactMsg, float impulse)
{
	Actor::C_ActorHandle actorHandle;
	const Actor::C_Actor* pActor	= NULL;
	C_BodyUserDataBase* pUserData	= (C_BodyUserDataBase*)pBody->GetUserData<void>();
	if (pUserData && pUserData->m_eType == E_BodyUserDataType_Actor)
	{
		actorHandle = ((C_BodyUserDataActor*)pUserData)->m_hActorHandle;
		pActor = m_pGameActorList->GetActorPtr(actorHandle);
	}

	if(pActor && pActor->GetActorTypeId() == E_ActorTypeId_Vehicle)
	{
		Actor::C_ActorHandle vehLogicHandle = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleHandle(((const Actors::C_VehicleActor*)pActor)->GetVehicleIndex());
		const Actor::C_Actor* pLogicActor	= m_pGameActorList->GetActorPtr(vehLogicHandle);
		if (pLogicActor)
		{
			//impactMsg.mCollisionInfo.m_AudioImpulse = impulse / pBody->Mass(); 
			impactMsg.mCollisionInfo.m_AudioImpulse = impulse * pBody->ReciprocalMass();
			impactMsg.mMatrix						= pBody->ElementMatrix();
			impactMsg.mCollisionInfo.m_MaxImpulse	= impulse;
		
			if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Damage) )
				m_pGameActorList->SendMessageToActorComponent(vehLogicHandle,Components::E_IF_VehicleComponent_Damage,&impactMsg);

			if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
				m_pGameActorList->SendMessageToActorComponent(vehLogicHandle,Components::E_IF_VehicleComponent_HandOfGod,&impactMsg);

			if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Sensor) )
				m_pGameActorList->SendMessageToActorComponent(vehLogicHandle,Components::E_IF_VehicleComponent_Sensor,&impactMsg);

			// Need to send these messages to the AI System....can either do this or call AI system directly...
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&impactMsg);

			Game::C_AchievementsManager::Instance().MessageHandler(&impactMsg);  
		}
	}
	//[Ciano] send impact messages to the audio component
	else if(pActor && (pActor->GetActorTypeId() == E_ActorTypeId_DynamicObject || pActor->GetActorTypeId() == E_ActorTypeId_PhysicalObject ))
	{
		impactMsg.mMatrixA						= pBody->ElementMatrix();
		//impactMsg.mCollisionInfo.m_AudioImpulse = impulse / pBody->Mass();
		impactMsg.mCollisionInfo.m_AudioImpulse = impulse * pBody->ReciprocalMass();
		impactMsg.mCollisionInfo.m_MaxImpulse = impulse;

		//actorList->SendMessageToActor(actorHandle,&impactMsg);
		//cian, temp disable until i figure why this is spamming
		if(pActor->QueryInterface(Components::E_AudioEntity_Spawned)) 
		m_pGameActorList->SendMessageToActorComponent(actorHandle,Components::E_AudioEntity_Spawned,&impactMsg);
	}
}

// --------------------------------------------------------------------------------------
// Not directly used by collision handling in RaceLogicLayer, but by others.
// --------------------------------------------------------------------------------------

const E_IMPACT_LEVEL C_RaceLogicLayer::CalculateImpactLevel(const float gameplayImpulse, bool isSideImpact) const
{
	const RLImpactType* pImpactType;

	for(int impactLevelIndex = 0; impactLevelIndex < m_pRLDamageSystem->mImpactTypes.count; impactLevelIndex++)
	{
		pImpactType = m_pRLDamageSystem->mImpactTypes[impactLevelIndex];

		if (isSideImpact)
		{
			if(gameplayImpulse <= pImpactType->mSideImpulseBounds.mMax && gameplayImpulse > pImpactType->mSideImpulseBounds.mMin)
				return m_pRLDamageSystem->mImpactTypes[impactLevelIndex]->meImpactLevel;
		}
		else
		{
			if(gameplayImpulse <= pImpactType->mFrontImpulseBounds.mMax && gameplayImpulse > pImpactType->mFrontImpulseBounds.mMin)
				return m_pRLDamageSystem->mImpactTypes[impactLevelIndex]->meImpactLevel;
		}
	}

	//If we're here then the impact wasn't valid
	return E_IMPACT_LEVEL_NONE;
}

const RLImpactType* C_RaceLogicLayer::CalculateImpactType(const float gameplayImpulse, bool isSideImpact)
{
	const RLImpactType* pImpactType;

	for(int impactLevelIndex = 0; impactLevelIndex < m_pRLDamageSystem->mImpactTypes.count; impactLevelIndex++)
	{
		pImpactType = m_pRLDamageSystem->mImpactTypes[impactLevelIndex];

		if (isSideImpact)
		{
			if(gameplayImpulse <= pImpactType->mSideImpulseBounds.mMax && gameplayImpulse > pImpactType->mSideImpulseBounds.mMin)
				return m_pRLDamageSystem->mImpactTypes[impactLevelIndex];
		}
		else
		{
				if(gameplayImpulse <= pImpactType->mFrontImpulseBounds.mMax && gameplayImpulse > pImpactType->mFrontImpulseBounds.mMin)
				return m_pRLDamageSystem->mImpactTypes[impactLevelIndex];
		}
	}

	//If we're here then the impact wasn't valid
	return NULL;
}

const RLLandingType* C_RaceLogicLayer::CalculateLandingType(const float gameplayImpulse)
{
	const RLLandingType* pLandingType;

	for(int landingLevelIndex = 0; landingLevelIndex < m_pRLDamageSystem->mLandingTypes.count; landingLevelIndex++)
	{
		pLandingType = m_pRLDamageSystem->mLandingTypes[landingLevelIndex];

		if(gameplayImpulse <= pLandingType->mImpulseBounds.mMax && gameplayImpulse > pLandingType->mImpulseBounds.mMin)
			return pLandingType;
	}

	//If we're here then the impact wasn't valid
	return NULL;
}

// --------------------------------------------------------------------------------------

extern Debug::Bool g_debugEnableQuickRestart;

bool C_RaceLogicLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_RaceLogicLayer.MessageHandler");

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case ActorMsg::E_CutSceneMsg_Start321:
			{
				mStarted321=true;
				m_RaceState = E_RaceState_Starting;				
			}
			break;

		case Game::E_VehicleMessage_VehicleWorldPosition:
			{
				const Game::Actors::C_VehicleWorldPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleWorldPositionMsg>(pMessage);

				mVehiclePositions[pMsg->mVehicleIndex].mLastPosition=mVehiclePositions[pMsg->mVehicleIndex].mPosition;

				mVehiclePositions[pMsg->mVehicleIndex].mPosition=pMsg->mWorldPosition;
				mVehiclePositions[pMsg->mVehicleIndex].mMatrix=pMsg->mWorldMatrix;
			}
			break;

		case Game::E_GameLogicMessage_SplitCrossed:
			{
				const Game::C_SplitCrossedMsg* pMsg = Messaging::MessageCast<Game::C_SplitCrossedMsg>(pMessage);

				if (pMsg->mVehicleIndex == 0)
				{
					Util::C_String32 notifyLap;
					notifyLap.Printf("SplitCrossed:%d:%d:%d", pMsg->mVehicleIndex, pMsg->mLap, pMsg->mSplit);
					Automation::Notify(notifyLap.Chars());
				}
			}
			break;

		case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);

				C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				
				if (pMsg->mLap == 1)
				{
					S32 numStarted = 0;
					for (S32 iVehicle = 0; iVehicle < GetNumVehicles(); iVehicle++)
					{
						Actors::C_VehicleLogicActor* pActor = GetLogicVehicleActor(iVehicle);
						if (!pActor)
							continue;

						if (iVehicle == pMsg->mVehicleIndex && !mVehicleStarted[iVehicle])	
							mVehicleStarted[iVehicle] = true;							
						
						if (mVehicleStarted[iVehicle])
							numStarted++;
					}

					//
					if(!mPickupsActivated)
					{
						if (numStarted == 1 && mPerksActivatedIn <= 0.0f)
						{							
							Game::Actors::PickupMessage_ActivatePickups msg(0.5f);
							C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);
							mPerksActivatedIn = levelState.mPickupDisableTime;
						}
					}
				}
				
				if(pMsg->mLap > (S32) levelState.mLaps && levelState.mLaps != 0)
				{
					ActorMsg::C_SetVehicleLogicMsg logicMsg;
					logicMsg.m_Ghosting = true; // This is for the out of range ghosting, and we want this to keep happening.

					Actors::C_VehicleLogicActor* pLogicActor = GetLogicVehicleActor(pMsg->mVehicleIndex);

					//[PK] apologies for the hard coded number, will sort out post Alpha
					if(levelState.mCity == 0x6D38AA95)
					{
						Game::Actors::C_VehicleActor* pActor = pLogicActor->GetGraphicalActor(pMsg->mVehicleIndex);

						Game::Actors::C_VehicleControlsDisableMsg disableMsg(pMsg->mVehicleIndex);
						pActor->SendMessageToActor(pActor->GetHandle(),&disableMsg);
					}
					else
					{
						logicMsg.m_HandOfGod = true;
						logicMsg.m_Drafting = true;
					}

					pLogicActor->SendMessageToActor(pLogicActor->GetHandle(),&logicMsg);
				}
				if (pMsg->mVehicleIndex == 0)
				{
#ifndef CFG_GOLD
					const Actors::C_VehicleLogicActor*	pLogicActor = GetLogicVehicleActor(0);

					Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

					//AmaxDataLogging::LogPlayerInfo(ci.mPosition, GetLogicVehicleActor(0)->GetPositionInfo().mSpeedInMph, pLogicActor->GetLapInfo().mCurrentLap);

					unsigned int	currentLap		= pMsg->mLap-1;
					int				racePosition	= pLogicActor->GetPositionInfo().mRacePosition;
					float			distance		= AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(0);

					// Distance to closest other vehicle (front +ve or behind -ve) and leader
					float distanceToLeader		= 0.f;
					float distanceToNearest		= 0.f;

					for( int i = 0; i < GetNumVehicles(); i++ )
					{
						if( i == 0 )
							continue;

						float currDistance = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(i) - distance;

						const Actors::C_VehicleLogicActor*	pOtherActor = GetLogicVehicleActor(i);
						if(pOtherActor->GetPositionInfo().mRacePosition == 1)
							distanceToLeader = currDistance;

						if(abs(currDistance) < abs(distanceToNearest) || distanceToNearest == 0.f)
							distanceToNearest = currDistance;
					}

					AmaxDataLogging::LogRaceLap(
						pLogicActor->GetCurrentHealth(),
						pMsg->mTime,
						currentLap,
						racePosition,
						distanceToLeader,
						distanceToNearest, pRLC->GetRaceTime());
#endif
					handledMessage = true;
				}
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
				m_RaceState = E_RaceState_Racing;
				m_restartInProgress = false;
#ifndef CFG_GOLD
				AmaxDataLogging::g_bDoInGameDataLogging = true;
#endif
				m_debugCollisionEvents.Reset();

				if (Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Destruction ||
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_Debug)
				{
					Game::Actors::PickupMessage_ActivatePickups msg((float)g_debugMotorMashPickupSpawnTime);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);
					mPerksActivatedIn = (float)g_debugMotorMashPickupSpawnTime;
					mPickupsActivated = true;
				}
				//else
				//	mPerksActivatedIn = (float)g_debugDefaultPickupSpawnTime;

				InitialisePlayerOvertakeCheck();
			}
			break;

		case ActorMsg::E_CutSceneMsg_Start_Cutscene:
			{
				m_RaceState = E_RaceState_Intro;
			}
			break;
		case ActorMsg::E_CutSceneMsg_Start_PassedCutscene:
			{				
				m_RaceState = E_RaceState_Outro;
			}
			break;
		case Game::E_GameflowMessage_RestartRace:
		case Game::E_GameflowMessage_QuitAndWinRace:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_RaceFinished:
			{
				StopPlayerOvertakeCheck();

				//if (pMessage->Type() != Game::E_GameflowMessage_RestartRace)
				//	C_GameApp::GetInstance()->ReleaseCachedResources();
#ifndef CFG_GOLD
				//Output anything that's been accumulated during the race
				EndDataLogging();

				const Actors::C_VehicleLogicActor* pLogicActor = GetLogicVehicleActor(0);

				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				unsigned int racePosition(0);
				int health(0);
				bool fatallyDamaged = false;
				U8 stopwatchesCollected = 0;
				U8 nitrousCollected = 0;
				U8 nitrousFired = 0;
				
				if (pLogicActor)
				{
					racePosition	= pLogicActor->GetPositionInfo().mRacePosition;
					health			= pLogicActor->GetCurrentHealth();
					fatallyDamaged	= pLogicActor->IsFatallyDamaged();
					stopwatchesCollected = pLogicActor->GetStatistics().mStopwatchPickupsCollected;
					nitrousCollected = pLogicActor->GetStatistics().mNitrousPickupsCollected;
					if(pLogicActor->GetStatistics().mNumPerksFired.Size() > E_PICKUP_TYPE_NITROUS)
						nitrousFired = pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_NITROUS];
				}

				// Distance to closest other vehicle (front +ve or behind -ve) and leader
				float distance				= 0.f;
				float distanceToLeader		= 0.f;
				float distanceToNearest		= 0.f;

				for( int i = 0; i < GetNumVehicles(); i++ )
				{
					if( i == 0 )
					{
						distance = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(0);
						continue;
					}

					float currDistance = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(i) - distance;

					const Actors::C_VehicleLogicActor*	pOtherActor = GetLogicVehicleActor(i);
					if(pOtherActor->GetPositionInfo().mRacePosition == 1)
						distanceToLeader = currDistance;

					if(abs(currDistance) < abs(distanceToNearest) || distanceToNearest == 0.f)
						distanceToNearest = currDistance;
				}

				bool didQuit		= pMessage->Type() == Game::E_GameflowMessage_QuitRace;
				bool didRestart		= m_restartInProgress;
				bool didQuitAndWin	= pMessage->Type() == E_GameflowMessage_QuitAndWinRace;

				const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
				bool passedRace = true;

				for( int i = 0; i < results.mCars[0].mPassedRule.Size(); i++ )
				{
					if(!results.mCars[0].mPassedRule[i])
						passedRace = false;
				}

				for( int i = 0; i < results.mCars[0].mFailedRule.Size(); i++ )
				{
					if(results.mCars[0].mFailedRule[i])
						passedRace = false;
				}

				// Please don't add anyhere with pLogicActor direct - it'll crash with Zero cars
				AmaxDataLogging::LogRaceFinished(
					health,
					racePosition,
					distanceToLeader,
					distanceToNearest,
					passedRace,
					fatallyDamaged,
					didQuit,
					didRestart,
					didQuitAndWin, 
					pRLC->GetRaceTime(),
					stopwatchesCollected,
					nitrousCollected,
					nitrousFired
					);
#endif
				if (pMessage->Type() == Game::E_GameflowMessage_RestartRace)
				{
					m_RaceLoaded = false;
					m_RaceState = E_RaceState_Restarting;
					m_restartInProgress = true;
					if (!g_debugEnableQuickRestart)
						break;
				}
				else
					m_RaceState = E_RaceState_Finished;

				handledMessage = true;
			}
			break;
		
		case Game::E_GameflowMessage_RaceLoaded:
		{
			m_RaceLoaded = true;
			/*if (!m_IsQuickRestart)
				C_GameApp::GetInstance()->CacheResources();*/
		}
		break;

		case GameMessaging::E_GameMsg_Update:
			{
				ProcessObjectCollisions();
				UpdatePlayerOvertakeCheck();

				C_RaceBookLayer* pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				if(pRaceBookLayer->GetRaceMode() == E_RaceMode_FanDemand || pRaceBookLayer->GetRaceMode() == E_RaceMode_FriendDemand_FanDemand)
				{
					UpdateFanRacePositions();
				}
			}
			break;

		default:
			handledMessage = false;
		break;
	}

	return handledMessage;
}

void C_RaceLogicLayer::UpdateFanRacePositions()
{
	// This could be optimised if we need to, and probably isn't needed every frame, but it's only used in fan races
	// so it hopefully doesn't matter. We need this to make sure shunt/shock targetting works how we expect with spawned vehicles.

	PERF_EVENT("C_RaceLogicLayer.UpdateFanRacePositions");
	for( int i = 0; i < mFanRacePositions.Size(); i++)
		mFanRacePositions[i].mVehIdx = -1;

	Game::Actors::C_VehicleLogicActor* pPlayerActor	= GetLogicVehicleActor(0); // Don't look at me like I'm the only one doing this.

	float playerLogicPos = pPlayerActor->GetPositionInfo().mLogicPosition;
	float playerCheckPos = playerLogicPos + 0.5f;
	if(playerCheckPos > 1.f)
		playerCheckPos -= 1.f;

	mFanRacePositions[0].mVehIdx = 0;
	mFanRacePositions[0].mDistFromPlayer = 0.f;

	for( int i = 1; i < GetNumVehicles(); i++ )
	{
		Game::Actors::C_VehicleLogicActor* pOtherActor = GetLogicVehicleActor(i);
		float otherLogicPos = pOtherActor->GetPositionInfo().mLogicPosition;

		float relativePos = 0.f;
		if(otherLogicPos < 0.5f && otherLogicPos > playerCheckPos)
			relativePos = 1.f - (otherLogicPos - playerLogicPos);
		else
			relativePos = otherLogicPos - playerLogicPos;

		mFanRacePositions[i].mVehIdx = i;
		mFanRacePositions[i].mDistFromPlayer = relativePos;
	}

	std::sort( mFanRacePositions.Begin(), mFanRacePositions.End(), FanRacePlayerPositionSort() );

	for( int i = 0; i < mFanRacePositions.Size(); i++ )
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor	= GetLogicVehicleActor(mFanRacePositions[i].mVehIdx);
		pLogicActor->GetPositionInfo().mRacePosition = i + 1;
	}
}

bool C_RaceLogicLayer::CanSpawnPickupOnTrack()
{
	return(mTotalLivePickups <= g_debugMaxPickupsOnTrack);
}

void C_TriggerEventCallback::ProcessTriggerEvents(SharedPhysics::C_MemoryIterator<Gravity::C_TriggerEvent>& iter, const U32 numEvents)
{
	Actor::C_ActorList* pActorList		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();

	for (U32 iEvent = 0; iEvent < numEvents; iEvent++)
	{
		Gravity::C_TriggerEvent* pEvent = iter.Get();

		if(pEvent->ValidTrigger())		
		{
			//Gravity::C_Trigger* pTrigger		= pEvent->GetTrigger();
			//const void* p = pTrigger->GetUserData();
			//Actor::C_ActorHandle triggerObject	= SharedActorUtils::HandleFromUserData(p);
			//p = pEvent->GetTriggerActor()->GetUserData();
			//Actor::C_ActorHandle triggerActor	= SharedActorUtils::HandleFromUserData(p);

			Gravity::C_Trigger* pTrigger		= pEvent->GetTrigger();
			const void* p = pTrigger->GetUserData();
			Actor::C_ActorHandle triggerObject	= SharedActorUtils::HandleFromUserData(p);
			p = pEvent->GetTriggerActor()->GetUserData();
			Actor::C_ActorHandle triggerActor;

			C_BodyUserDataBase* pUserData = (C_BodyUserDataBase*)p;
			if (pUserData && pUserData->m_eType == E_BodyUserDataType_Actor)
				triggerActor = ((C_BodyUserDataActor*)pUserData)->m_hActorHandle;

	#ifndef CFG_GOLD
			bool fail = false;
			if (!pActorList->ActorExists(triggerObject))
			{
				LOGERROR(g_AmaxLoadingLog, "Trigger event - triggerObject handle is INVALID");
				fail = true;
			}
			if (!pActorList->ActorExists(triggerActor))
			{
				LOGERROR(g_AmaxLoadingLog, "Trigger event - triggerActor handle is INVALID");
				fail = true;
			}
			if (fail)
				continue;
	#endif
			const Actor::C_Actor* pTriggerObject		= pActorList->GetActorPtr(triggerObject);
			const Actor::C_Actor* pTriggerActor			= pActorList->GetActorPtr(triggerActor);

			if (pTriggerActor && pTriggerActor->GetActorTypeId() == E_ActorTypeId_Vehicle)
			{
				if(pTriggerObject)
				{
					if(pTriggerObject->GetActorTypeId() == E_ActorTypeId_PickupPosition || pTriggerObject->GetActorTypeId() == E_ActorTypeId_PickupPositionDecoy || pTriggerObject->GetActorTypeId() == E_ActorTypeId_FanDemandPosition)
					{
						if (pEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
						{
							Game::Actors::C_EnteredPickupMsg pickupMsg(triggerActor);
							pActorList->SendMessageToActor(triggerObject,&pickupMsg);
						}
						else if(pEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
						{
							Game::Actors::C_ExitedPickupMsg pickupMsg(triggerActor);
							pActorList->SendMessageToActor(triggerObject,&pickupMsg);
						}
					}
					else if(pTriggerObject->GetActorTypeId() == E_ActorTypeId_TriggerRegion)
					{
						Game::Actors::C_VehicleCheckTriggerRegionMsg triggerMsg(triggerActor, (S32)pEvent->GetEventType());
						pActorList->SendMessageToActor(triggerObject,&triggerMsg);

						Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(((const Actors::C_VehicleActor*)pTriggerActor)->GetVehicleIndex());
						if(pLogicActor && pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand))
						{
							Actor::C_ActorHandle vehLogicHandle = pLogicActor->GetHandle();

							Game::Actors::C_VehicleCheckTriggerActorEvent triggerMsg1(triggerActor, pEvent, pTriggerObject, pTriggerActor);
							pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Fan_Demand, &triggerMsg1);
						}
					}
				}
			}

			if (pTriggerActor && pTriggerActor->GetActorTypeId() == E_ActorTypeId_ListenerAudioActor)
			{
				if(pTriggerObject && (pTriggerObject->GetActorTypeId() == E_ActorTypeId_DynamicObject || pTriggerObject->GetActorTypeId() == E_ActorTypeId_PhysicalObject))
				{
					//Game::Actors::C_VehicleCheckTriggerRegionMsg triggerMsg(triggerActor, (S32)pEvent->GetEventType());
					Game::Actors::C_VehicleCheckTriggerActorEvent triggerMsg(triggerActor, pEvent, pTriggerObject, pTriggerActor);
					//Game::Actors::C_VehicleAudioImpactMsg triggerMsg;
					//triggerMsg.m_PartCode == -1;
					Actor::C_ActorHandle hnd = C_GameApp::GetInstance()->GetGameAudioLayer()->GetAudioActorHnd(Game::C_GameAudioLayer::eListenerSubsystem);
					pActorList->SendMessageToActor(hnd,&triggerMsg);

				}
				else
				if(pTriggerObject && (pTriggerObject->GetActorTypeId() == E_ActorTypeId_PickupPosition || pTriggerObject->GetActorTypeId() == E_ActorTypeId_PickupPositionDecoy || pTriggerObject->GetActorTypeId() == E_ActorTypeId_FanDemandPosition))
				{
					//DebugBreak();
					Game::Actors::C_VehicleCheckTriggerActorEvent triggerMsg(triggerActor, pEvent, pTriggerObject, pTriggerActor);

					//Actor::C_ActorHandle hnd = C_GameApp::GetInstance()->GetGameAudioLayer()->GetAudioActorHnd(Game::C_GameAudioLayer::eListenerSubsystem);
					pActorList->SendMessageToActor(pTriggerObject->GetHandle(),&triggerMsg);


				}
				else //the listener is intersecting a triggerRegion (overhead)
				if(pTriggerObject && (pTriggerObject->GetActorTypeId() == E_ActorTypeId_TriggerRegion))
				{
						if(pEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
						{
							const ::Actors::TriggerRegionActor* pRegionTriggerActor = (const ::Actors::TriggerRegionActor*)pTriggerObject;

							if(pRegionTriggerActor->GetCreationData() && pRegionTriggerActor->GetCreationData()->m_VehicleMessage)
							{
								const ActorMsg::C_VehicleEnteredTriggerMsg* pEnterMsg = Messaging::MessageCast<ActorMsg::C_VehicleEnteredTriggerMsg>(pRegionTriggerActor->GetCreationData()->m_VehicleMessage->GetMessage(pRegionTriggerActor));

								//if it's one of the actorStudio placed tunnels...
								if(pEnterMsg->m_TriggerType == Game::E_TRIGGER_TYPE_TUNNEL_ENTER)
								{
									Game::Actors::C_VehicleCheckTriggerActorEvent triggerMsg(triggerActor, pEvent, pTriggerObject, pTriggerActor);

									Actor::C_ActorHandle hnd = C_GameApp::GetInstance()->GetGameAudioLayer()->GetAudioActorHnd(Game::C_GameAudioLayer::eListenerSubsystem);
									pActorList->SendMessageToActor(hnd,&triggerMsg);
								}
							}
						}
						else
						if(pEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
						{
							const ::Actors::TriggerRegionActor* pRegionTriggerActor = (const ::Actors::TriggerRegionActor*)pTriggerObject;

							if(pRegionTriggerActor->GetCreationData() && pRegionTriggerActor->GetCreationData()->m_VehicleMessageOnExit)
							{
								const ActorMsg::C_VehicleExitedTriggerMsg* pExitMsg = Messaging::MessageCast<ActorMsg::C_VehicleExitedTriggerMsg>(pRegionTriggerActor->GetCreationData()->m_VehicleMessageOnExit->GetMessage(pRegionTriggerActor));

								//if it's one of the actorStudio placed tunnels...
								if(pExitMsg->m_TriggerType == Game::E_TRIGGER_TYPE_TUNNEL_EXIT)
								{
									Game::Actors::C_VehicleCheckTriggerActorEvent triggerMsg(triggerActor, pEvent, pTriggerObject, pTriggerActor);

									Actor::C_ActorHandle hnd = C_GameApp::GetInstance()->GetGameAudioLayer()->GetAudioActorHnd(Game::C_GameAudioLayer::eListenerSubsystem);
									pActorList->SendMessageToActor(hnd,&triggerMsg);
								}
							}
						}

					
				}

			}
		}
	}
}

void C_DebugCollisionEventRecorder::Render(S32 p_VehicleIndex)
{
	if (p_VehicleIndex < 0 || p_VehicleIndex >= m_CollisionEventStream.Size())
		return;

	for (S32 i = 0; i < m_CollisionEventStream[p_VehicleIndex].m_Collisions.Size(); i++)
	{
		C_DebugCollisionEventRecorder::CollisionEventStream::CollisionEvent* pEvent = &m_CollisionEventStream[p_VehicleIndex].m_Collisions[i];
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pEvent->m_Position, pEvent->m_Normal, 1.0f, 0.1f, Maths::RGBA(0, 255, 0, 255));
	}
}

// --------------------------------------------------------------------------

void PickupEffectInfo::InitNormalValues(bool shuffleOnRespawn)
{
	shuffleTime = gDebugShuffleTime;
	shuffleOnPickedUp = !gDebugFixRandomPerks || shuffleOnRespawn;
	respawnTime =  C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mPickupDeactivateTimer;
}

//============================================================================

void C_RaceLogicLayer::InitialisePlayerOvertakeCheck()
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	m_overtake_number = 0;

	if (levelState.mNumCars <= 0)
	{
		LOGINFO(g_AmaxLoadingLog, "Player overtake logic disabled because there are no cars");
		return;
	}

	F32 playerpos = GetLogicVehicleActor(0)->GetPositionInfo().mLogicPosition;
	for(U32 a=1; a<levelState.mNumCars; a++)
		m_overtake_relative_pos[a] = GetPlayerRelativePos(playerpos, GetLogicVehicleActor(a)->GetPositionInfo().mLogicPosition);

	m_overtake_initialised = true;
}

//============================================================================

void C_RaceLogicLayer::StopPlayerOvertakeCheck()
{
	m_overtake_number = 0;
	m_overtake_initialised = false;
}

//============================================================================

void C_RaceLogicLayer::UpdatePlayerOvertakeCheck()
{
	PERF_EVENT("C_RaceLogicLayer.UpdatePlayerOvertakeCheck");
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	if (!m_overtake_initialised)
		return;

	m_overtake_number = 0;

	F32 playerpos = GetLogicVehicleActor(0)->GetPositionInfo().mLogicPosition;
	for(U32 a=1; a<levelState.mNumCars; a++)
	{
		F32 newval = GetPlayerRelativePos(playerpos, GetLogicVehicleActor(a)->GetPositionInfo().mLogicPosition);
		if ((m_overtake_relative_pos[a] < 0.25f) && (newval > 0.75f))			// If it was less than 1/4 of a lap ahead and it's now almost 1 lap ahead
		{
			m_overtake_number++;												// then the player must have overtaken it
		}
		else if ((m_overtake_relative_pos[a] > 0.75f) && (newval < 0.25f))		// Same again but this time the player was overtaken by the AI
		{
			if ((levelState.mRaceMode != Game::E_RaceMode_FanDemand) && (levelState.mRaceMode != Game::E_RaceMode_Destruction))	// Ignore vehicles spawning in front of player
				m_overtake_number--;
		}
		m_overtake_relative_pos[a] = newval;
	}
}

//============================================================================

F32 C_RaceLogicLayer::GetPlayerRelativePos(F32 playerpos, F32 vehpos)
{
	F32 dist = (vehpos - playerpos) + 1.0f;				// Make sure it's positive

	dist -= (F32)((S32)dist);								// Get mod

	return(dist);
}


} // namespace Game
