#include "stdafx.h"

#include <GameLogic/Structures/LevelState.h>
#include <GameLogic/GameData/GameData.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameSystems.h>

#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <Game/GameActors/Vehicles/VehicleControlType.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_HandOfGod.h>
#include <GameLogic/RaceRules/RaceRulesManager.h>

#include <Vehicle/VehicleModelManager.h>
#include <RaceBook/RaceBookManager.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <RaceBook\RaceBookManager.h>

#include <System/Automation/AutomationMessages.h>

#include <ui/Game/Profiles/ProfileInterface.h>

#include <SinglePlayer/SinglePlayer.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>

#include <Game/Game.h>

Debug::Bool g_debugVehicleModel("Vehicle.DebugModel", false, Debug::Persist());

#ifndef CFG_GOLD

Debug::Bool g_debugShopOveride("Shop.Overide", false);

Debug::Int g_debugShopAdditionalPerkSlot("Shop.AdditionalPerkSlot", 5, Debug::Persist());
Debug::Bool g_debugShopPickupDifferentPerks("Shop.PickupDifferentPerks", true, Debug::Persist());
Debug::Bool g_debugShopPickupSelector("Shop.PickupSelector", true, Debug::Persist());

Debug::Bool g_debugShopDoubleTapNitrous("Shop.DoubleTapNitrous", true, Debug::Persist());
Debug::Bool g_debugShopDoubleTapShunt("Shop.DoubleTapShunt", true, Debug::Persist());
Debug::Bool g_debugShopDoubleTapBarge("Shop.DoubleTapBarge", true, Debug::Persist());
Debug::Bool g_debugShopDoubleTapShock("Shop.DoubleTapShock", true, Debug::Persist());
Debug::Bool g_debugShopDoubleTapMine("Shop.DoubleTapMine", true, Debug::Persist());

Debug::Bool g_debugShopAdvSlipstream("Shop.AdvSlipstream", true, Debug::Persist());
Debug::Bool g_debugShopAdvConvertToHealth("Shop.AdvConvertToHealth", true, Debug::Persist());
Debug::Bool g_debugShopAdvCloneFirst("Shop.AdvCloneFirst", true, Debug::Persist());

Debug::Bool g_debugShopShield("Shop.Shield", true, Debug::Persist());
Debug::Int g_debugShopShieldSlot("Shop.ShieldSlot", 10);

Debug::Int g_debugShopAdditionalHealthSlot("Shop.AdditionalHealthSlot", 5);
Debug::Bool g_debugShopRepairBoost1("Shop.RepairBoost1", true, Debug::Persist());
Debug::Bool g_debugShopRepairBoost2("Shop.RepairBoost2", true, Debug::Persist());

Debug::Bool g_debugShopFanSearcher("Shop.FanSearcher", true, Debug::Persist());
Debug::Bool g_debugShopFanViral("Shop.FanViral", true, Debug::Persist());
Debug::Bool g_debugShopFanForumite("Shop.FanForumite", true, Debug::Persist());
Debug::Bool g_debugShopFanBlogger("Shop.FanBlogger", true, Debug::Persist());
Debug::Bool g_debugShopFanSabotage("Shop.FanSabotage", true, Debug::Persist());
Debug::Bool g_debugShopFanBribe("Shop.FanBribe", true, Debug::Persist());

Debug::Bool g_debugShopUpgradeNitrous("Shop.UpgradeNitrous", true, Debug::Persist());
Debug::Bool g_debugShopUpgradeShunt("Shop.UpgradeShunt", true, Debug::Persist());
Debug::Bool g_debugShopUpgradeBarge("Shop.UpgradeBarge", true, Debug::Persist());
Debug::Bool g_debugShopUpgradeShock("Shop.UpgradeShock", true, Debug::Persist());
Debug::Bool g_debugShopUpgradeMine("Shop.UpgradeMine", true, Debug::Persist());
Debug::Float g_debugShopUpgradeNitroMultiplier("Shop.UpgradeNitroMultiplier", 1.0f, Debug::Persist()); 
Debug::Bool g_debugShopUpgradeCloakingDevice("Shop.UpgradeCloakingDevice", true, Debug::Persist()); 
Debug::Bool g_debugShopUpgradeAdvancedRadarTracking("Shop.UpgradeAdvancedRadarTracking", true, Debug::Persist()); 
Debug::Bool g_debugShopUpgradeAdvancedWarningSystem("Shop.UpgradeAdvancedWarningSystem", true, Debug::Persist()); 
Debug::Bool	g_debugShopUpgradeDrifter("Shop.UpgradeDrifter", true, Debug::Persist());
Debug::Float g_debugShopUpgradeDrifterMinDistance("Shop.UpgradeDrifterMinDistance", 20.0, Debug::Persist());
Debug::Bool	g_debugShopUpgradeDraftDodger("Shop.UpgradeDraftDodger", true, Debug::Persist());
Debug::Float g_debugShopUpgradeDraftMultiplier("Shop.UpgradeDraftMultiplier", 0.5, Debug::Persist());
Debug::Bool	g_debugShopUpgradeRetaliation("Shop.UpgradeRetaliation", true, Debug::Persist());

extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;

#endif

// Brought over from RaceBookSorceryEvent - maybe safe to kill off..
Debug::Int g_debugForceVehicleTextureDetail("Vehicle.LOD.SinglePlayerForceTextureDetail", 0, Debug::Range(0, 10), Debug::Step(1));
Debug::Int g_debugSPNonFocusDetail("Vehicle.LOD.SinglePlayerNonFocusDetail", 7, Debug::Range(1, 10), Debug::Step(1));

Debug::Bool g_debugEnableVehicleTextureLods("Vehicle.LOD.EnableTextureLod", true);

Debug::Bool g_RemoveMinorCharactersFromRaces("Game.RemoveMinorCharactersFromRaces", false, Debug::Persist());
Debug::Int g_debugSetEventLaps("Cheats.NumEventLaps",0,Debug::Persist());

Debug::Logger &g_LevelStateLogger( Debug::GetLogger("EventCreation") );

extern Debug::Int g_debugShieldsEarnedRepairSinglePlayer;
extern Debug::Int g_debugShieldsEarnedHitByPerkSinglePlayer;
extern Debug::Int g_debugShieldsEarnedFansSinglePlayer;
extern Debug::Int g_debugShieldsEarnedFansRequiredSinglePlayer;
extern Debug::Int g_debugShieldsInitialSinglePlayer;


//#pragma optimize("", off)					// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP

Debug::Float g_debugPerkActivateTime("Perks.DefaultActivateTime", 4.0f);

namespace Game
{

void C_LevelState::Clear()
{
	SysProfile::UserIdentifier emptyId;

	m_LevelFilename = "";
	m_LevelActorStudioFilename = "";
	mCity = 0;
	mRoute = 0;
	mLaps = 1;
	mTimeLimit = 0;
	mTeams = false;
	mFanMultiplier = 1.0f;
	
	mPunitiveMeasuresInPlace = false;		//Is this player being penalised for n00b pwning? ie Go into a beginners lobby with a huge rank
	mPunitiveFanMultiplier = 1.f;

	mStartOpponentsDisabled = false;

	mUsePerks = true;
	mPerksRespawn = true;
	mPerksRespawnRandomType = false;
	mNetworkRace = false;
	mOriginalNetworkHost = false;
	m_RandomSeed = 0;
	mInUI = false;
	mRaceMode = E_RaceMode_Nothing;
	mUiRaceType = UI::E_RaceType_Racing;
	mDamageMode = E_DAMAGE_MODE_NORMAL;
	
	mDamageMultiplier = 1.0f;
	mPowerUpDamageMultiplier = 1.0f;

	mStartType = AmaxStartType_Grid;
	mReplayMode = E_ReplayMode_Record;
	mLeaderboardType = E_LEADERBOARDTYPE_POSITION;
	mAIController = NULL;
	mOverrideNonRandomPickups = false;
	mShieldsTiedToPickups = false;
	mPerkDroppingWhenUsed = false;
	mAllowPickupsOnLapZero = false;
	m_ScriptedCameraWavBank = m_CutSceneWavBank = false;
	mWorldHUDPositionInfoRequired = true;
	mDisableRacingContextFanRewards = false; 
	mDisableFanDemands = false;
	mWreckedBehaviour = E_AMAXWRECKED_RESPAWN;

	mPickupsEnabled = true;
	mPickupProbabilities.Clear();
	mPickupDisableTime = g_debugPerkActivateTime;
	mPickupSpawnLevel = 0;
	mPickupRespawnType = E_PICKUP_RESPAWN_TYPE_NORMAL;

	mNumViewports=1;
	mEnvironmentOveride = 0;
	
	mPlayerIsAI = false;
	mDebugEvent	= false;
	m_AIGameplayBalancing = 0;
	m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Default_Hard.csv");
	m_AIRetryInterval = 0.0f;
	m_AIRetryMinTrackSpeed = 0.0f;
	m_RetryCount = 0.0f;
	m_AIDontPickupPickups = false;
	mHoGResetCriteria = Components::C_VehicleComponent_HandOfGod::E_HandOfGodResetCriteria_All;
	mHoGNoSplineResetSpeedInMPH = 0;
	m_overrideAISurface = false;
	m_overrideAISurfaceValue = 0.9f;

	// Multiplayer stuff
	mMultiplayerSettings.Clear();

	//CWright - these were not getting initialised/cleared
	m_difficulty = GAME_DIFFICULTY_NORMAL;
	mEvent = 0;
	mSpEventId = -1;
	mRouteNumber = 0;
	mSpecificAudioLinesOnly = false;
	mRandomCars = 0;
	mQualify = false;
	mPointsBased = false;
	mMpRaceIndex = -1;
	mGhostCarSettings.Clear();
	mDamageSettings.Clear();
	mCarClassList.Clear();

	mDefaultPickupInfo.Init();

	mNumCars=0;
	mNumAdditionalCars=0;
	for (S32 i=0; i<mCars.Size(); i++)
	{
		LevelStateCar& car = mCars[i];

		car.mUpgrades.Clear();
		car.mPickupInfo.Init();

		car.mControlType = E_VEHICLE_CONTROL_TYPE_UNKNOWN;
		car.mCarID = 0;
		car.mCarLocID = 0;
		car.mCarStyle = 0;
		car.mManufacturerID = 0;
		car.mController = INVALID_CONTROLLER;
		car.mStartPosition = 0;
		car.mCharacterID = 0;
		car.mAIDriver = 0;
		car.mAISteering = 0;
		car.mResprayID = 0;
		car.mResprayColour = 0;
		car.mTextureDetail = INT_MAX;
		car.mTeam = i;	//	Ensure all vehicles are on different teams by default
		
		car.mGrantTimeMin = 0.f;
		car.mGrantTimeMax = 0.f;

		car.mSystemPlayerId = emptyId;
		car.mMultiplayerRank = 0;
		car.mMultiplayerLegendLevel = 0;
		car.mMultiplayerWinningStreak = 0;
		car.mBeatenBizarre = false;
		car.mNetJoinId = 0;
		car.mOwnerNetJoinId = 0;

		car.mCharacter = NULL;
		car.mNameAboveCar.Clear();

		car.mAutoGears = true;
		car.mRightHandDrive = false;
		car.mHuman = false;
		car.mAutomatedHuman = false;
		car.mStartDisabled = false;
		car.mCutSceneOnly = false;
		car.mPickupsAllowed = true;

		car.mFilename.Clear();
		car.mWheelFilename.Clear();
		car.mDynamicsFilename.Clear();
		car.mAICharacterName.Clear();

		car.mState = E_VEHICLE_STATE_UNKNOWN;
		car.mNetMessageNos[0] = 0;
		car.mNetMessageNos[1] = 0;
		car.mNetUpdateTimer = 0;

		car.mHealthOverride = 0;
		car.mPerkToGrantAI = E_PICKUP_TYPE_NONE;
		car.maxPerkLevel = 3;		

		car.mPerkGrantingLaps = false;
		car.mPerkGrantingTimer = 0;
		car.mPerksToGrant.Clear();
		car.mPreferredPickups.Clear();

		car.mShieldsEarnedRepair = 0;
		car.mShieldsEarnedHitByPerk = 0;
		car.mShieldsEarnedFans = 0;
		car.mShieldsEarnedFansRequired = 0;
		car.mShieldsInitial = 0;

		//CWright - these were not getting initialised/cleared
		car.mPerkGrantingInitialTimer = 0.0f;
		car.mCharactersNotTargetted.Clear();
		car.mAiAvatarId = 0;
		car.m_InitialWorldMatrix.SetIdentity();
		car.mLoadOuts.Clear();
		car.mUpgrades.GetMultiplayerAbilities()->Reset();
	}
}

void C_LevelState::Finished()
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	C_GameApp::GetInstance()->GetRaceBookLayer()->SetRaceMode(mRaceMode);

	const AmaxCity *city = C_GameData::Instance().mCities[(GAMEDATA_ID)mCity];
	
	if (city)
	{
		const AmaxRoute *route = C_GameData::Instance().mRoutes[(GAMEDATA_ID)mRoute];

		const char* cityName = static_cast<const char*>(city->tag);

		m_LevelFilename = Util::C_String::Format(".\\levels\\%s\\level.level", cityName);
		m_LevelActorStudioFilename = Util::C_String::Format(".\\gamedata\\levels\\%s", cityName);

		if (route)
		{
			const char* routeName = static_cast<const char*>(route->tag);
			m_LevelAIFilename = Util::C_String::Format(".\\gamedata\\levels\\%s\\%s.aid", cityName, routeName);
			mRouteNumber=route->route_number;

			mHoGNoSplineResetSpeedInMPH = route->handOfGodNoSplineResetSpeedInMPH;
		}
	}

	for(unsigned int a=0;a<mNumCars+mNumAdditionalCars;a++)
	{
		//RaceBook::C_RaceBookVehicle *vehicle = RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)mCars[a].mCarID);

		GAMEDATA_ID gdvID = (GAMEDATA_ID)mCars[a].mCarID;
		AmaxVehicle *veh=C_GameVehicleData::Instance().mVehicles[gdvID];
		BIZ_ASSERT_MSG_SPEED(veh, "Couldn't match Vehicle(%x) when setting up race\n%d(%d)", gdvID.id, a, mNumCars+mNumAdditionalCars);

		GAMEDATA_ID gdcID = (GAMEDATA_ID)mCars[a].mCharacterID;
		RaceBookCharacter *pCharacter = C_RaceBookData::Instance().mCharacters[gdcID];
		BIZ_ASSERT_MSG(pCharacter, "Couldn't match Character(%x) when setting up race\n%d(%d)", gdcID.id, a, mNumCars+mNumAdditionalCars);

		if (pCharacter)
		{
			mCars[a].mAICharacterName = static_cast<const char*>(pCharacter->tag);
			mCars[a].mAIDriver = pCharacter->ai_template->ID;
			mCars[a].mAISteering = veh->aiSteering->ID;
		}
			
		RaceBook::C_RaceBookProfile *pRBProfile=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)mCars[a].mCharacterID);
		if(pRBProfile)
		{
			if(!mCars[a].mHuman)
				mCars[a].mNameAboveCar.Format(UI::C_TextStringID(pRBProfile->GetName().id).String());
		}

		if(veh)
		{	
			//Set the vehicle model file name
			mCars[a].mFilename = Util::C_String::Format("models/vehicles/%s.model", static_cast<const char*>(veh->tag));

			//Check file exists6
			Sys::C_FilePath filePath = fs.GetWorkingDirectory() + "/" + mCars[a].mFilename;
			
			if (!fs.FileExists(filePath) || !veh->use)
			{
				// Report that the car model could not be found
				Automation::SendProblemFoundMsg("VEHICLE", a, "NOFILE");

				mCars[a].mFilename = "models/vehicles/shelby_mustanggt500_stock.model";
			}

			if (Vehicle::C_VehicleModelManager::Instance().m_DEBUG_RegisterCarModelType)
			{
				mCars[a].mFilename += "|car";
				if (g_debugEnableVehicleTextureLods)
				{
					mCars[a].mFilename += "|";
					mCars[a].mFilename += Util::unsignedIntegerToCString(mCars[a].mTextureDetail).Chars();
				}
			}

			//Set the wheel model file name
			if (veh->wheelModel)
			{
				mCars[a].mWheelFilename = Util::C_String::Format("models/vehicles/wheels/%s.model", static_cast<const char*>(veh->wheelModel->fileName));
			}
			else
				mCars[a].mWheelFilename = "models/vehicles/wheels/default.model";

			//Check file exists6
			filePath = fs.GetWorkingDirectory() + "/" + mCars[a].mWheelFilename;
			if (!fs.FileExists(filePath))
			{
				// Report that the car model could not be found
				Automation::SendProblemFoundMsg("WHEEL", 0, "NOFILE");
				mCars[a].mWheelFilename = "models/vehicles/wheels/default.model";
			}

			if (Vehicle::C_VehicleModelManager::Instance().m_DEBUG_RegisterWheelModelType)
			{
				mCars[a].mWheelFilename += "|wheel";
				if (g_debugEnableVehicleTextureLods)
				{
					mCars[a].mWheelFilename += "|";
					mCars[a].mWheelFilename += Util::unsignedIntegerToCString(mCars[a].mTextureDetail).Chars();
				}
			}
			
			if (g_debugVehicleModel)
			{
				mCars[a].mWheelFilename = "models/vehicles/wheels/default.model|car";
				mCars[a].mFilename = "models/vehicles/default.model|wheel";
			}

			//Set dynamics file name
			if (veh->dynamics)
			{
				if(!mCars[a].mHuman)
				{
					if (veh->UseAIDynamics)
						mCars[a].mDynamicsFilename = static_cast<const char*>(veh->aiDynamics->name);
					else
						mCars[a].mDynamicsFilename = static_cast<const char*>(veh->dynamics->name);
				}
				else
				{
					mCars[a].mDynamicsFilename = static_cast<const char*>(veh->dynamics->name);
				}

				
			}
			else
				mCars[a].mDynamicsFilename = "DefaultCar";
				
			//Set right hand drive flag.
			mCars[a].mRightHandDrive = veh->rightHandDrive;
		}
	}

#ifndef CFG_GOLD
	if(g_debugShopOveride)
	{
		/*mCars[0].mUpgrades.mShop[E_RACEBOOK_SHOP_UPGRADES_PERK_ADDITIONAL_PERK_SLOT]=(int)g_debugShopAdditionalPerkSlot;
		mCars[0].mUpgrades.mShop[E_RACEBOOK_SHOP_UPGRADES_PERK_PICK_UP_DIFFERENT_PERKS]=g_debugShopPickupDifferentPerks;
		mCars[0].mUpgrades.mShop[E_RACEBOOK_SHOP_UPGRADES_PERK_PICK_UP_SELECTOR]=g_debugShopPickupSelector;*/
						   
		mCars[0].mUpgrades.DoubleNitro(g_debugShopDoubleTapNitrous);
		mCars[0].mUpgrades.DoubleShunt(g_debugShopDoubleTapShunt);
		mCars[0].mUpgrades.DoubleBarge(g_debugShopDoubleTapBarge);
		mCars[0].mUpgrades.DoubleShock(g_debugShopDoubleTapShock);
		mCars[0].mUpgrades.DoubleBomb(g_debugShopDoubleTapMine);
						   
		mCars[0].mUpgrades.AdvancedSlipstream(g_debugShopAdvSlipstream);
		mCars[0].mUpgrades.AdvancedConvertToHealth(g_debugShopAdvConvertToHealth);
		mCars[0].mUpgrades.AdvancedCloneFirst(g_debugShopAdvCloneFirst);
						   
		mCars[0].mUpgrades.RepairBoostLevel2(g_debugShopRepairBoost1);
		mCars[0].mUpgrades.RepairBoostLevel3(g_debugShopRepairBoost2);
				
		mCars[0].mUpgrades.FansSearcher(g_debugShopFanSearcher);
		mCars[0].mUpgrades.FansViral(g_debugShopFanViral);
		mCars[0].mUpgrades.FansSabotage(g_debugShopFanForumite);
		mCars[0].mUpgrades.FansForumite(g_debugShopFanBlogger);
		mCars[0].mUpgrades.FansSabotage(g_debugShopFanSabotage);
		mCars[0].mUpgrades.FansBribe(g_debugShopFanBribe);
						   
		mCars[0].mUpgrades.NitroUpgrade(g_debugShopUpgradeNitrous);
		mCars[0].mUpgrades.ShuntUpgrade(g_debugShopUpgradeShunt);
		mCars[0].mUpgrades.BargeUpgrade(g_debugShopUpgradeBarge);
		mCars[0].mUpgrades.ShockUpgrade(g_debugShopUpgradeShock);
		mCars[0].mUpgrades.BombUpgrade(g_debugShopUpgradeMine);

		mCars[0].mUpgrades.CollectAnyPerkType(g_debugShopPickupDifferentPerks);
		mCars[0].mUpgrades.ManualPerkChange(g_debugShopPickupSelector);
		mCars[0].mUpgrades.NumPickupSlots(g_debugShopAdditionalPerkSlot);
		mCars[0].mUpgrades.Shield(g_debugShopShield);
		mCars[0].mUpgrades.NumShieldSlots(g_debugShopShieldSlot);

		mCars[0].mUpgrades.NumAdditionalHealthSlots((int)g_debugShopAdditionalHealthSlot);
	}
#endif
}

void C_LevelState::SetupRaceFromEvent(U32 eventId, S32 spEventId)
{
	Game::C_LevelResult& levelResult = Game::C_Game::GetInstance()->GetLevelResult();

	Clear();
	levelResult.Clear();

	mAIController = NULL;
	for( int i = 0; i < RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers.count; i++ )		// Setup AI Controller
	{
		if( RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[i].mDifficulty == Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetDifficulty() )
		{
			mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[i].mController.Object();
			break;
		}
	}
	if( !mAIController )
	{
		LOGWARN(g_LevelStateLogger, "No controller found for difficulty setting; defaulting to first in list.");
		mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[0].mController.Object();
	}


	for( int i = 0; i < RaceBook::C_RaceBookManager::Instance().NumProfiles(); i++ )				// Setup Defaults
		RaceBook::C_RaceBookManager::Instance().GetProfileByIndex(i)->mCurrentCarIdx = -1;			// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!


	AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)eventId];						// Setup Level State
	if(ev)
	{
		AmaxRoute* route = C_GameData::Instance().mRoutes[(GAMEDATA_ID)ev->route->ID];

		mRoute = route->ID;
		mCity = route->city->ID;
		mRaceMode = Game::E_RaceMode_StreetRace;
		mSpEventId = spEventId;
		
		if (ev->raceMode)
			mRaceMode = ev->raceMode->raceMode;

		SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), mSpEventId );

		if (!pEvent)
		{
			pEvent = SP_GetEventByAmaxEventId(SP_GetCurrentSaveGame(), eventId);
			if (pEvent)
				mSpEventId = pEvent->id;
		}

		if (pEvent)
		{
			switch(pEvent->kind)
			{
			case kSP_RaceEvent:
				BIZ_ASSERT(mRaceMode == Game::E_RaceMode_StreetRace);
				break;
			case kSP_FanEvent:
				BIZ_ASSERT(mRaceMode == Game::E_RaceMode_FanDemand);
				break;
			case kSP_CheckpointEvent:
				BIZ_ASSERT(mRaceMode == Game::E_RaceMode_Checkpoint);
				break;
			case kSP_DestructionEvent:
				BIZ_ASSERT(mRaceMode == Game::E_RaceMode_Destruction);
				break;
			case kSP_BossEvent:
				BIZ_ASSERT(mRaceMode == Game::E_RaceMode_BossBattle);
				break;
			}
		}

		Game::C_GameProfile* game_profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

		if(Game::C_FriendChallenges::Instance().IsAttemptingFriendDemand())
		{
			if(mRaceMode == E_RaceMode_StreetRace)
				mRaceMode=E_RaceMode_FriendDemand_StreetRace;

			if(mRaceMode == E_RaceMode_FanDemand)
				mRaceMode=E_RaceMode_FriendDemand_FanDemand;

			if(mRaceMode == E_RaceMode_Checkpoint)
				mRaceMode=E_RaceMode_FriendDemand_Checkpoint;

			if(mRaceMode == E_RaceMode_Destruction)
				mRaceMode=E_RaceMode_FriendDemand_Destruction;

			if(mRaceMode == E_RaceMode_BossBattle)
				mRaceMode=E_RaceMode_FriendDemand_BossBattle;

			m_difficulty = (Game::GAME_DIFFICULTY)Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mDifficulty;
		}
		else
			m_difficulty = (Game::GAME_DIFFICULTY)game_profile->GetDifficulty();

		gRaceModeStore(mRaceMode)->SetupRace();

		if(g_debugSetEventLaps <= 0)
			mLaps = ev->laps;
		else
			mLaps = g_debugSetEventLaps;

		Game::C_RaceRulesManager::Instance().ClearAllRules();

		game_profile->mNeedsAIUpdate = true;					// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!

		m_RetryCount=0;									// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!
		if(eventId == game_profile->mCurrentRetryingEvent)
			m_RetryCount=(float)game_profile->mNumberOfRetrys;


		mLeaderboardType = Game::E_LEADERBOARDTYPE_NULL;
		mTimeLimit = 0;
		DifficultySettings* difficultySettings = NULL;
		switch(m_difficulty)
		{
			case Game::GAME_DIFFICULTY_VERY_EASY:
			case Game::GAME_DIFFICULTY_EASY:
				mLeaderboardType = ev->easyVersion.leaderboardType;			// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!
				mTimeLimit = (float)ev->easyVersion.timeLimit;
				difficultySettings = &ev->easyVersion;
				break;

			case Game::GAME_DIFFICULTY_NORMAL:
				mLeaderboardType = ev->normalVersion.leaderboardType;			// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!
				mTimeLimit = (float)ev->normalVersion.timeLimit;
				difficultySettings = &ev->normalVersion;
				break;

			case Game::GAME_DIFFICULTY_HARD:
			case Game::GAME_DIFFICULTY_HARDCORE:
				mLeaderboardType = ev->hardVersion.leaderboardType;			// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!
				mTimeLimit = (float)ev->hardVersion.timeLimit;
				difficultySettings = &ev->hardVersion;
				break;
		}

		BIZ_ASSERT(difficultySettings);

		if (difficultySettings->aiBalancingFileName.Text() != "")
		{
			Util::C_String temp = Util::C_String::Format("%s",difficultySettings->aiBalancingFileName.Text());
			if (temp.Length() < 5)
				m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Default_Easy.csv");
			else
				m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\%s", difficultySettings->aiBalancingFileName.Text());
		}
		else
		{
			m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Default_Easy.csv");
		}
		

		

		// Retry count needs populating from somewhere, ie stats manager....
		m_AIRetryInterval = difficultySettings->aiRetryInterval * m_RetryCount;
		m_AIRetryMinTrackSpeed = difficultySettings->aiRetryMinTrackSpeed;
		m_AIDontPickupPickups = ev->aiDontPickupPickups;
		mSpecificAudioLinesOnly = ev->specificAudioLinesOnly;
		m_overrideAISurface = ev->overrideAISurface;
		m_overrideAISurfaceValue = ev->overrideAISurfaceValue;

		if(ev->pickupProbabilities)							// NOT USED AT THE MOMENT BUT MAY BE USED IN THE FUTURE!!!!!!!!!!!!!!!!!!!!!!!
		{
			for( int i = 0; i < ev->pickupProbabilities->pickupProbabilities.count; i++ )
			{
				Game::LevelPickupProbability p;
				p.pickupType = ev->pickupProbabilities->pickupProbabilities[i].pickupType;
				p.probability = ev->pickupProbabilities->pickupProbabilities[i].probability;
				mPickupProbabilities.PushBack(p);
			}
		}

		if (ev->pickupDisableTime > 0.0f)
			mPickupDisableTime = ev->pickupDisableTime;
		mPickupSpawnLevel = ev->pickupSpawnLevel;
		if (ev->pickupsDoNotRespawn)
			mPickupRespawnType = E_PICKUP_RESPAWN_TYPE_NONE;
		else
			mPickupRespawnType = E_PICKUP_RESPAWN_TYPE_NORMAL;


		mEvent = ev->ID;
		
		mNumAdditionalCars = 0;

		// Allow the racemode to choose if it wants to setup the opponents for this race
		if( gRaceModeStore(mRaceMode)->SetupOpponents() == false )
		{
			mNumCars = ev->opponents.count;

			int car_index=0;
			for(unsigned int a=0;a<mNumCars + mNumAdditionalCars;a++)
			{

				AmaxOpponentSetup opponentSetupXt = ev->opponents[a];

				LevelSetupOpponentData opponentSetup;

				opponentSetup.startPositionIndex	= a;
				opponentSetup.ev					= ev;
				opponentSetup.difficultySettings	= difficultySettings;
				if (a == 0)
					opponentSetup.startDisabled		= false;
				else
					opponentSetup.startDisabled		= mStartOpponentsDisabled;				// w false
				opponentSetup.opponent				= opponentSetupXt.opponent.Object();
				opponentSetup.vehicleOverride		= opponentSetupXt.vehicleOverride.Object();
				opponentSetup.detailOverride		= opponentSetupXt.detailOverride;
				opponentSetup.alwaysShields			= opponentSetupXt.alwaysShields;			
				opponentSetup.onlyAttacksPlayer		= opponentSetupXt.onlyAttacksPlayer;		
				opponentSetup.healthOverride		= opponentSetupXt.healthOverride;		
				opponentSetup.perkSlotsOverride		= opponentSetupXt.perkSlotsOverride;	
				opponentSetup.perkGiftTimer			= opponentSetupXt.perkGiftTimer;			
				opponentSetup.perkGiftTimerInitial	= opponentSetupXt.perkGiftTimerInitial;	
				opponentSetup.perkGiftLaps			= opponentSetupXt.perkGiftLaps;			
				opponentSetup.perkGiftTypes			= opponentSetupXt.perkGiftTypes;	

				if( SetupOpponent(car_index,opponentSetup))
				{			  
					car_index++;
				}
			}
		}


		for( int i = 0; i < difficultySettings->aiPerkUse.count; i++ )
		{
			int opponentIdx = difficultySettings->aiPerkUse[i].opponentIdx;
			if( opponentIdx >= 0 && opponentIdx <= 20 )
			{
				mCars[opponentIdx].mGrantTimeMin		= difficultySettings->aiPerkUse[i].grantTimeMin;
				mCars[opponentIdx].mGrantTimeMax		= difficultySettings->aiPerkUse[i].grantTimeMax;
				mCars[opponentIdx].mPerkToGrantAI	= difficultySettings->aiPerkUse[i].perkToGrant;
			}
		}

		Finished();									// Setup models etc

		Game::C_RaceRulesManager::Instance().InitRulesFromEvent(mNumCars, levelResult, ev);			// CAN PROBABLY REMOVE!!!!!!!!!!!!!!!!!!!!!!!
	}
}


bool C_LevelState::SetupOpponent(int car_index, LevelSetupOpponentData& opponentSetup )
{
	Game::C_GameProfile* game_profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(opponentSetup.opponent, "NULL opponent in event %s", opponentSetup.ev->tag.Text());

	RaceBookCharacter* rbChar = opponentSetup.opponent;
	RaceBook::C_RaceBookProfile *rbc=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)rbChar->ID);
	BIZ_ASSERT_MSG(rbc, "Could not find RaceBook character to add to race");

	//	First thing's first - set this vehicle's upgrades/specs from their RB profile.
	rbc->BuildLevelStateVehicleUpgrades(mCars[car_index].mUpgrades);

	mCars[car_index].mStartDisabled = opponentSetup.startDisabled;

	//	Secondly, we override all vehicles' pickup/shield slots and basic abilities to be the same as the player (don't ask...)
	if(opponentSetup.perkSlotsOverride)
		mCars[car_index].mUpgrades.NumPickupSlots(opponentSetup.perkSlotsOverride);
	else
		mCars[car_index].mUpgrades.NumPickupSlots(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->NumPickupSlots());
		mCars[car_index].mUpgrades.CollectAnyPerkType(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->AllowAnyPerkType());
		mCars[car_index].mUpgrades.ManualPerkChange(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->AllowManualPerkChange());
		mCars[car_index].mUpgrades.Shield(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->AllowShield());
		mCars[car_index].mUpgrades.NumShieldSlots(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->NumShieldSlots());
		mCars[car_index].mUpgrades.NumAdditionalHealthSlots(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->NumAdditionalHealthSlots());

	if(g_RemoveMinorCharactersFromRaces && rbc->mType == E_CHARACTER_TYPE_NORMAL)
		return false;

	rbc->mCurrentCarIdx = car_index;

	if(rbc->mType != E_CHARACTER_TYPE_PLAYER)
	{
		bool foundClass = false;

		for(int i = 0; i < rbc->mGarage.Size(); i++ )
		{
			RaceBook::C_RaceBookVehicle* veh = RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)rbc->mGarage[i].mId);

			if(veh)
			{
				AmaxVehicle* amaxVeh = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)veh->mBaseModelID];

				if( amaxVeh && amaxVeh->carClass == opponentSetup.ev->carClass )
				{
					rbc->mCurrentCarID = veh->mBaseModelID;
					foundClass = true;
					break;
				}
			}
		}

		if(!foundClass)
		{
			LOGWARN(g_LevelStateLogger, "Character %s has no vehicle in the class specified by the event. Using first vehicle in Garage.", UI::C_TextStringID( rbc->GetName().id ).String());

			if(rbc->mGarage.Size())
			{
				RaceBook::C_RaceBookVehicle* veh = RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)rbc->mGarage[0].mId);

				BIZ_ASSERT_MSG(veh, "AI character has a vehicle in his Garage but it can't be found");

				if(veh)
				{
					rbc->mCurrentCarID = veh->mBaseModelID;
				}
			}
			else
			{
				BIZ_ASSERT_MSG(false, "AI character has no vehicle in his Garage");
			}
		}
	}
	else
	{
		// setup profile for leaderboards
		mCars[car_index].mSystemPlayerId = UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
	}

	if(opponentSetup.vehicleOverride)
	{
		RaceBook::C_RaceBookVehicle *rbv=RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)opponentSetup.vehicleOverride->ID);
		rbc->mCurrentCarID = rbv->mBaseModelID;
	}

	AmaxVehicle *av = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)rbc->mCurrentCarID];
	BIZ_ASSERT(av);

	mCars[car_index].mManufacturerID=av->manufacturer->ID;
	mCars[car_index].mCarID=av->ID;

	mCars[car_index].mCharacterID=rbc->GetID();

	//if(car_index)
	mCars[car_index].mStartPosition = opponentSetup.startPositionIndex;			



	for( int i = 0; i < opponentSetup.perkGiftTypes.count; i++ )
		mCars[car_index].mPerksToGrant.PushBack(opponentSetup.perkGiftTypes[i]);

	mCars[car_index].mPerkGrantingLaps			= opponentSetup.perkGiftLaps;
	mCars[car_index].mPerkGrantingTimer			= opponentSetup.perkGiftTimer;
	mCars[car_index].mPerkGrantingInitialTimer	= opponentSetup.perkGiftTimerInitial;
	mCars[car_index].mHealthOverride			= opponentSetup.healthOverride;

#ifndef CFG_GOLD
	if (rbc->mType == E_CHARACTER_TYPE_PLAYER && mPlayerIsAI)
	{
		mCars[car_index].mAutomatedHuman=true;
	}
#endif
	if (rbc->mType == E_CHARACTER_TYPE_PLAYER
#ifndef CFG_GOLD
		&& !mPlayerIsAI
#endif
	)
	{
		mCars[car_index].mController=SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		mCars[car_index].mControlType=E_VEHICLE_CONTROL_TYPE_PLAYER;
		mCars[car_index].mHuman=true;
		mCars[car_index].mAutoGears=true;
		mCars[car_index].mResprayID=game_profile->ResprayGet(av->ID);

		// Setup abilities
		C_MultiplayerAbilities& pMultiplayerAbilities = C_Game::GetInstance()->GetMultiplayerAbilities();
		pMultiplayerAbilities.ResetLoadOuts(mCars[car_index]);

		if(Game::C_FriendChallenges::Instance().IsAttemptingFriendDemand())
		{
			if(Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mBossMod != UI::E_MultiplayerVehicleAbilities_None)
				pMultiplayerAbilities.ActivateBossSlot((UI::E_MultiplayerVehicleAbilities)Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mBossMod, mCars[car_index]);
		}
		else
		{
			// determine whether we are allowed to use a boss mod that we may have equipped
			bool useAbilitySP = false;

			SP_SaveGame* pSaveGame = SP_GetCurrentSaveGame();
			SP_Event* pEvent = SP_GetEventById( pSaveGame, mSpEventId );

			if ( pEvent )
			{
				useAbilitySP = ( pEvent->kind == kSP_RaceEvent || pEvent->kind == kSP_BossEvent );
			}

			if(useAbilitySP && game_profile->mSpEquippedAbility != UI::E_MultiplayerVehicleAbilities_None)
			{
				pMultiplayerAbilities.ActivateBossSlot(game_profile->mSpEquippedAbility, mCars[car_index]);
			}
		}
	}
	else
	{
		mCars[car_index].mController=Game::INVALID_CONTROLLER;
		mCars[car_index].mControlType=E_VEHICLE_CONTROL_TYPE_AI;
		mCars[car_index].mHuman=false;
		mCars[car_index].mAutoGears=true;

		C_MultiplayerAbilities& pMultiplayerAbilities = C_Game::GetInstance()->GetMultiplayerAbilities();
		pMultiplayerAbilities.ResetLoadOuts(mCars[car_index]);

		if(av->bossAbilitySlot != UI::E_MultiplayerVehicleAbilities_None)
		{
			pMultiplayerAbilities.ActivateBossSlot(av->bossAbilitySlot, mCars[car_index]);
		}
	
		for( int i = 0; i < opponentSetup.difficultySettings->aiPerkUse.count; i++ )
		{
			if( opponentSetup.difficultySettings->aiPerkUse[i].opponentIdx == car_index )
			{
				if( opponentSetup.difficultySettings->aiPerkUse[i].charactersNeverTargetted.count )
				{
					for( int b = 0; b < opponentSetup.difficultySettings->aiPerkUse[i].charactersNeverTargetted.count; b++ )
						mCars[car_index].mCharactersNotTargetted.PushBack(opponentSetup.difficultySettings->aiPerkUse[i].charactersNeverTargetted[b]->ID);
				}
				else
				{
					for( int b = 0; b < rbChar->charactersNeverTargetted.count; b++ )
						mCars[car_index].mCharactersNotTargetted.PushBack(rbChar->charactersNeverTargetted[b]->ID);
				}
			}

			if( opponentSetup.difficultySettings->aiPerkUse[i].preferredPickups.count )
			{
				for( int b = 0; b < opponentSetup.difficultySettings->aiPerkUse[i].preferredPickups.count; b++ )
					mCars[car_index].mPreferredPickups.PushBack(opponentSetup.difficultySettings->aiPerkUse[i].preferredPickups[b]);
			}
			else
			{
				for( int b = 0; b < rbChar->preferredPickups.count; b++ )
					mCars[car_index].mPreferredPickups.PushBack(rbChar->preferredPickups[b]);
			}

			if( opponentSetup.difficultySettings->aiPerkUse[i].maxPerkLevel )
				mCars[car_index].maxPerkLevel = opponentSetup.difficultySettings->aiPerkUse[i].maxPerkLevel;
		}
	}

#ifndef CFG_GOLD
	if (g_debugSuperSpecialCutSceneVideoMode)
		mCars[car_index].mTextureDetail = INT_MAX;
	else
#endif
	{		
#ifndef CFG_GOLD
		if (g_debugForceVehicleTextureDetail > 0)
			mCars[car_index].mTextureDetail = g_debugForceVehicleTextureDetail;
		else
#endif
			if (mCars[car_index].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
				mCars[car_index].mTextureDetail = INT_MAX;
			else
				mCars[car_index].mTextureDetail = opponentSetup.detailOverride == 0 ? g_debugSPNonFocusDetail : opponentSetup.detailOverride;			
	}

	switch(mRaceMode)
	{
	case E_RaceMode_Checkpoint:
	case E_RaceMode_FriendDemand_Checkpoint:
		{
			mCars[car_index].mPickupInfo.InitCheckpoint();
			break;
		}

	case E_RaceMode_Destruction:
	case E_RaceMode_FriendDemand_Destruction:
		{
			mCars[car_index].mPickupInfo.InitDestruction();
			break;
		}
	}		

	return true;
}

void C_LevelState::ClearMultiplayerAbilities()
{
	for (S32 i=0; i<mCars.Size(); i++)
		mCars[i].mUpgrades.GetMultiplayerAbilities()->Reset();
}

void C_LevelState::Dump()
{
#if !defined(CFG_GOLD)
/*
	printf("### level state ###\n");
	printf("city : %u\n", mCity);
	printf("route : %u\n", mRoute);
	printf("laps : %u\n", mLaps);
	printf("win by : %u\n", mWinBy);
	printf("use perks : %u\n", mUsePerks);
	printf("num vehicles : %u\n", mNumCars);

	for (U32 i=0; i<mNumCars; i++)
	{
		const LevelStateCar& car = mCars[i];
		printf("\n");
		printf("  name : %S\n", car.mPlayer.Name());
		printf("  join id : %u\n", car.mNetJoinId);
		printf("  car id : %u\n", car.mCarID);
		printf("  character id : %d\n", car.mCharacterID);
		printf("  start pos : %u\n", car.mStartPosition);
		printf("  control type : %u\n", car.mControlType);
	}

	printf("\n");
	printf("### end level state ###\n");
*/
#endif
}

LevelStateMultiplayerSettings& C_LevelState::GetMultiplayerSettings()
{
	return mMultiplayerSettings;
}


LevelStateVehiclePickupInfo& C_LevelState::GetPickupInfo( U8 vehicleIndex )
{
	if(vehicleIndex != SYSTEM_FIRING_INDEX)
		return mCars[vehicleIndex].mPickupInfo;
	else
		return mDefaultPickupInfo;
}

void C_LevelState::ResetMPAbilityActivations()
{
	for(U8 i=0; i<mNumCars; i++)
	{
		mCars[i].mUpgrades.GetMultiplayerAbilities()->ResetInstantAbilityFiringBits();
	}
}

void C_LevelState::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );

	out << mEvent << mCity << mRoute << mLaps << mTimeLimit << (int)mUsePerks << (int)mNetworkRace << (int)mQualify << (int)mPointsBased << 
		  (int)mRaceMode << (int)mStartType << mNumCars << (int)mGhostCarSettings.ghostCarWhen << mGhostCarSettings.numGhostCars << (int)mLeaderboardType <<
		  mGhostCarSettings.respawnLength << (int)mDamageSettings.fatalDamage;

	out << mPickupProbabilities.Size();
	for( int i = 0; i < mDamageSettings.startingDamage.Size(); i++ )
	{
		out << (S32&)mPickupProbabilities[i].pickupType;
		out << mPickupProbabilities[i].probability;
	}

	out << mDamageSettings.startingDamage.Size();
	for( int i = 0; i < mDamageSettings.startingDamage.Size(); i++ )
	{
		out << mDamageSettings.startingDamage[i];
	}
}
void C_LevelState::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{	
	Sys::BinaryDeserialiserAutoEndian in( stream );
	int numElements;

	in >> mEvent >> mCity >> mRoute >> mLaps >> mTimeLimit >> (S32&)mUsePerks >> (S32&)mNetworkRace >> (S32&)mQualify >> (S32&)mPointsBased >> 
		(S32&)mRaceMode >> (S32&)mStartType >> mNumCars >> (S32&)mGhostCarSettings.ghostCarWhen >> mGhostCarSettings.numGhostCars >> (S32&)mLeaderboardType >>
		mGhostCarSettings.respawnLength >> (S32&)mDamageSettings.fatalDamage;

	in >> numElements;
	for( int i = 0; i < numElements; i++ )
	{
		LevelPickupProbability p;

		in >> (S32&)p.pickupType;
		in >> p.probability;
		mPickupProbabilities.PushBack(p);
	}

	in >> numElements;
	for( int i = 0; i < numElements; i++ )
	{
		int damage;
		in >> damage;
		mDamageSettings.startingDamage.PushBack(damage);
	}
}

void AmaxRaceTeam::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );

	out << colour.R() << colour.G() << colour.B() << numDrivers << delay;
	out << carClasses.Size();
	for( int i = 0; i < carClasses.Size(); i++ )
	{
		out << (int)carClasses[i];
	}

}
void AmaxRaceTeam::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{	
	Sys::BinaryDeserialiserAutoEndian in( stream );

	F32 colR, colG, colB;
	in >> colR >> colG >> colB >> numDrivers >> delay;
	Maths::C_RGBA_F32 col(colR, colG, colB, 0);

	colour = col;

	int numElements;
	in >> numElements;
	for( int i = 0; i < carClasses.Size(); i++ )
	{
		AmaxCarClass carClass;
		in >> (S32&)carClass;
		carClasses.PushBack(carClass);
	}
}

} // namespace Game
