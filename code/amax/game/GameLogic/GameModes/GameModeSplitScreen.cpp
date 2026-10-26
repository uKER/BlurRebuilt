#include "stdafx.h"

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameModes/GameModeSplitScreen.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <Game/Game.h>
#include <GameLogic/Gamedata/GameData.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic\profile\GameProfileManager.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <GameLogic/Structures/LevelResult.h>
#include <GameLogic/AchievementsManager.h>

#include <ui/Graphics/UIColourClasses.h>
#include <Ui/game/Multiplayer/MultiplayerCache.h>

#include <UI/UIHelpers.h>
#include <ui/Game/Profiles/ProfileInterface.h>

#include <Game/AmaxCam/CameraSystem.h>
#include <Game/AmaxCam/VehicleFeed.h>

#include <AI/AI_Amax.h>
#include <ui/Game/UILUA.h>


Debug::Logger &g_GameModeSplitScreenLogger( Debug::GetLogger("GameMode.SplitScreen") );

Debug::Bool g_ForceShieldsTiedToPickups("GameMode.SplitScreen.ForceShieldsTiedToPickups", false);

Debug::Bool	g_OverrideAIPadding("Multiplayer.Splitscreen.OverrideAIPadding", false);
Debug::Int g_OverrideNumberOfAIToAdd("Multiplayer.Splitscreen.OverrideNumberOfAIToAdd", 0, Debug::Range(0, 19));

namespace Game
{

int gSplitScreenAIPaddingNumbers[Game::E_NUM_SPLITSCREEN_PLAYERS_MAX] = 
{
	20,
	10,
	5,
	4,
};

C_GameModeSplitScreen::C_GameModeSplitScreen()
	: C_GameModeMultiPlayer()
{
	SetInternalNameAndID( "SplitScreen", E_GameMode_SplitScreen );
}

void C_GameModeSplitScreen::OnSetupRace( lua_State* pState )
{
	BIZ_ASSERT( pState );
	if ( !pState || lua_type( pState, 1 ) != LUA_TTABLE )
		return;

	m_NumLocalPlayers = 0;

	// Setup persistent data for the game mode
	if ( Lua::DataMemberType( pState, "num_players" ) == LUA_TNUMBER )
	{
		Lua::GetDataMember( pState, "num_players", m_NumLocalPlayers );
	}

	for( S32 i = 0; i < m_NumLocalPlayers; i++)
	{
		AmaxCam::CameraSystem().VehicleFeed(i)->SetChangeViewAllowed(false);
	}

	m_Scoring.SetNoViewports( m_NumLocalPlayers );
}

void C_GameModeSplitScreen::OnSetupLevelState( lua_State* pState, C_LevelState& levelState )
{
	BIZ_ASSERT( pState );
	if ( !pState || lua_type( pState, 1 ) != LUA_TTABLE )
		return;

	// Setup each local player
	if ( Lua::DataMemberType( pState, "players" ) == LUA_TTABLE )
	{
		lua_pushstring( pState, "players" );
		lua_gettable( pState, -2 );

		// Setup num cars and viewports based on players
		levelState.mNumCars = m_NumLocalPlayers;
		levelState.mNumViewports = m_NumLocalPlayers;
		levelState.mNumAdditionalCars = 0;

		// this is local players only - i.e. players that have signed in or blagged a profile
		// player 1-4, pads 0-3
		for ( S32 i = 1; i <= m_NumLocalPlayers; i++ )
		{
			lua_rawgeti( pState, -1, i );

			S32 sPadIndex = Lua::GetDataMember<S32>( pState, "pad" );
		
			// New Car ID Stuff //
			GAMEDATA_ID carId = (GAMEDATA_ID)Lua::GetDataMember<U32>( pState, "car_id" );
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ carId ];

			if ( pVehicle )
			{
				levelState.mCars[i-1].mCarID = pVehicle->ID;
				levelState.mCars[i-1].mCharacterID = GetRandomCharacterID();
				levelState.mCars[i-1].mController = sPadIndex;
				levelState.mCars[i-1].mControlType = E_VEHICLE_CONTROL_TYPE_PLAYER;
				levelState.mCars[i-1].mHuman = true;
				levelState.mCars[i-1].mAutomatedHuman = false;
				levelState.mCars[i-1].mAutoGears = true;
				levelState.mCars[i-1].mStartPosition = i-1;
				levelState.mCars[i-1].mUpgrades.OfflineValues(); //values for human player
				// If mResprayID is set to 0 then we use the default manufacturers paint job
				// anything else is considered a respray
				levelState.mCars[i-1].mResprayID = Lua::GetDataMember<U32>( pState, "colour_id" );

				// PR - set name above car from profile
				levelState.mCars[i-1].mNameAboveCar = UI::C_ProfileInterface::Instance().GetPadProfile( sPadIndex ).m_Name;
			}

			lua_pop( pState, 1 ); // index i
		}

		lua_pop( pState, 1 ); // players table
	}

	LevelStateMultiplayerSettings& mpSettings = levelState.GetMultiplayerSettings();
	if(mpSettings.mAIMode != UI::E_AiOff && levelState.mRaceMode != E_RaceMode_Multiplayer_Destruction)
	{
		//Add some AI vehicles
		int noAIToAdd = CalculateNumberOfAIToAdd(levelState);
		if(noAIToAdd)
		{
			PadAI(levelState, noAIToAdd);
		}

		//Setup ai data
		switch (mpSettings.mAIMode)
		{
		case UI::E_AiEasy:
			levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\splitscreen_easy.csv");
			break;
		case UI::E_AiMedium:
			levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\splitscreen_medium.csv");
			break;
		case UI::E_AiHard:
			levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\splitscreen_hard.csv");
			break;
		default:
			//levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Default_Medium.csv");
			levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\SplitScreen_Default.csv");
			break;
		}
	}

	if( !levelState.mAIController )
	{
		levelState.mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[0].mController.Object();
	}
}


void C_GameModeSplitScreen::StartRace()
{
	EnableChangeViewOnAllViewports();

	C_GameProfileManager::Instance().GetPrimaryProfile()->mRetryingEvent=false;

	// Ensure camera changes not allowed until race properly started
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen)
	{	
		//stop the level being black on the bowl
		if(!( AIAmax::AmaxAIR::Instance().AllOK() ) )
		{
			C_GameApp* pgameApp = C_GameApp::GetInstance();
			BIZ_ASSERT_MSG(pgameApp, "Could Not Get The Game App: GameModeSplitScreen.cpp - StartRace()");

			Game::C_RaceBookLayer* praceBookLayer = pgameApp->GetRaceBookLayer();
			BIZ_ASSERT_MSG(praceBookLayer, "Could Not Get Race Book Layer: GameModeSplitScreen.cpp - StartRace()");

			Game::C_AmaxWorldLayer* pamaxWorldLayer = praceBookLayer->GetAmaxWorldLayer();
			BIZ_ASSERT_MSG(pamaxWorldLayer, "Could Not Get Amax World Layer: GameModeSplitScreen.cpp - StartRace()");

			Game::Actors::RaceLogicCore* prlc = pamaxWorldLayer->GetRaceLogicCore();
			BIZ_ASSERT_MSG(prlc, "Could Not Get The Race Logic Core: GameModeSplitScreen.cpp - StartRace()");

			Game::C_GameWorld* pgameWorld = prlc->GetGameWorld();
			BIZ_ASSERT_MSG(pgameWorld, "Could Not Get The Game World: GameModeSplitScreen.cpp - StartRace()");

			pgameWorld->SetGlobalFade(1.0f);
		}
	}

	// TODO_MP_REFACTOR
	m_RaceAwards.ResetSelection();
}

void C_GameModeSplitScreen::ResetScoring()
{
	m_Scoring.Reset();

	m_TotalPlayTime = 0.0f;
}

const float C_GameModeSplitScreen::GetTotalRaceTime() const
{
	return m_TotalPlayTime;
}

const SplitScreenScoring::Scores& C_GameModeSplitScreen::GetTotalScores() const
{
	return m_Scoring.GetTotalScores();
}

const SplitScreenScoring::Scores& C_GameModeSplitScreen::GetScoresForRace() const
{
	return m_Scoring.GetScoresForRace();
}

const SplitScreenScoring::Scores& C_GameModeSplitScreen::GetResultsForRace() const
{
	return m_Scoring.GetRaceResulsts();
}

const SplitScreenScoring::Scores& C_GameModeSplitScreen::GetTotalFans() const
{
	return m_Scoring.GetTotalFans();
}

const SplitScreenScoring::Scores& C_GameModeSplitScreen::GetFansForRace() const
{
	return m_Scoring.GetFansForRace();
}

const int C_GameModeSplitScreen::GetScoreForRace(const int viewportIndex) const
{
	return (m_Scoring.GetScoresForRace())[viewportIndex];
}

const int C_GameModeSplitScreen::GetTotalScore(const int viewportIndex) const
{
	return (m_Scoring.GetTotalScores())[viewportIndex];
}

const int C_GameModeSplitScreen::GetFansForRace(const int viewportIndex) const
{
	return (m_Scoring.GetFansForRace())[viewportIndex];
}

const int C_GameModeSplitScreen::GetTotalFans(const int viewportIndex) const
{
	return (m_Scoring.GetTotalFans())[viewportIndex];
}

const int C_GameModeSplitScreen::GetTotalWins(const int viewportIndex) const
{
	return (m_Scoring.GetTotalWins())[viewportIndex];
}

const int C_GameModeSplitScreen::GetTotalRaces() const
{
	return m_Scoring.GetTotalRaces();
}


void C_GameModeSplitScreen::InterpretRaceResults()
{
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	bool has_passed_event = true;
	RaceBook::C_RaceBookProfile *prof = NULL;

	// Check if all cars have finished
	for( U32 i = 0; i < levelState.mNumCars; i++ )
	{
		//Check to see if this is a player car
		if(levelState.mCars[i].mController != 99)
		{
			//If any player car has position 99 it hasn't finished
			if(results.mCars[i].mPosition == 99)  // Their magic number is 99!
				return;
		}
	}

	//	Only eligible for this sticker/achievement if there are 4 players
	if(m_NumLocalPlayers == 4)
		C_StickerManager::Instance().CheckBlurParty();

	UpdateScores(results, levelState);
	//C_AchievementsManager::Instance().PostRace(has_passed_event);

	// TODO_MP_REFACTOR
	m_RaceAwards.CalculateAwards((U32)Time::GetCurrentTime());
}

void C_GameModeSplitScreen::UpdateScores(const Game::C_LevelResult& results, const Game::C_LevelState& levelState)
{
	
	SplitScreenScoring::RaceResults racePositionResults;
	SplitScreenScoring::RaceResults fansResults;

	//Blanks our result array out
	for(int i=0; i<E_NUM_SPLITSCREEN_PLAYERS_MAX; i++)
	{
		racePositionResults[i] = -1;
		fansResults[i] = -1;
	}
	

	//Getting player positions
	for(int vehicleIndex=0; vehicleIndex<results.mNumCars; vehicleIndex++)
	{
		//If its a player vehicle
		if (levelState.mCars[vehicleIndex].mControlType==E_VEHICLE_CONTROL_TYPE_PLAYER)
		{
			//Store the race position
			int viewportIndex = UI::GetLocalPlayerIndex(vehicleIndex);

			racePositionResults[viewportIndex] = results.mCars[vehicleIndex].mPosition;
			fansResults[viewportIndex] = results.mCars[vehicleIndex].mScore;
		}
	}

	//Get race time
	const float raceTime = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetRaceTime();
	m_TotalPlayTime += raceTime;

	// Sending it to the Scoring system
	m_Scoring.RaceFinished(racePositionResults, fansResults);
}

const int C_GameModeSplitScreen::CalculateNumberOfAIToAdd(const C_LevelState& levelState) const
{
	if(levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
	{
		return 0;
	}

	int noPlayers = levelState.mNumViewports;
	MultiplayerConfig* multiplayerConfig = C_MultiplayerData::Instance().m_pConfig;

	if(g_OverrideAIPadding)
	{
		return g_OverrideNumberOfAIToAdd;
	}

	if(multiplayerConfig->m_NumberOfSplitScreenAIPlayers.count >= noPlayers)
	{
		return multiplayerConfig->m_NumberOfSplitScreenAIPlayers[noPlayers-1];
	}

	return gSplitScreenAIPaddingNumbers[noPlayers - 1] - noPlayers;
}

void C_GameModeSplitScreen::PadAI( C_LevelState& levelState, const int noAIToAdd )
{
	//Check we're actually asking for some AI's to be added
	if(noAIToAdd > 0)
	{
		//Make a list of valid vehicles
		VehicleList validVehicles;
		GetValidVehicles(validVehicles);

		//Make sure we're not going  break the bank
		int newTotalCars = (levelState.mNumCars + noAIToAdd);
		int AIIndex = levelState.mNumCars;
		if( newTotalCars > MAX_LEVELSTATE_CARS )
		{
			BIZ_ASSERT_MSG(false, "C_GameModeMultiPlayer::PadAI() > If you pad %d more vehicles to the levelState you'll have too many vehicles in total (%d)", noAIToAdd, newTotalCars);
		}
		levelState.mNumCars = newTotalCars;

		//Now we can start to add the AIs
		for (int i = 0; i < noAIToAdd; i++ )
		{
			//Fill in the level state for the AI vehicles
			CreateAIPaddingVehicle(validVehicles, levelState, AIIndex++);
		}
	}
}

void C_GameModeSplitScreen::CreateAIPaddingVehicle( const VehicleList& validVehicles, C_LevelState& levelState, const int levelStateCarIndex )
{
	const AmaxVehicle *pVehicle = GetRandomVehicle(validVehicles);
	BIZ_ASSERT_MSG(pVehicle, "C_GameModeMultiPlayer::CreateAIPaddingVehicle() > Failed to find a suitable AI vehicle to pad");

	MultiplayerConfig* mpConfig = C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(mpConfig, "Couldn't get MP config");

	//	Should always be safe as we have 20 AI character records in data for system link :)
	const RaceBookCharacter* pChar = C_MultiplayerData::Instance().m_pConfig->m_AiCharacters[levelStateCarIndex];

	//Set ai level state car
	LevelStateCar& aiCar = levelState.mCars[levelStateCarIndex];

	aiCar.mCharacterID	= pChar->ID;	//	Setting this gives us the driver model and the name above the car
	aiCar.mAiAvatarId	= pChar->avatarID;	//	Matching AI avatar (gamerpic) for results screen
	aiCar.mCarID		= pVehicle->ID;
	aiCar.mControlType = E_VEHICLE_CONTROL_TYPE_AI;
	aiCar.mController = Game::INVALID_CONTROLLER;
	aiCar.mHuman = false;
	aiCar.mAutomatedHuman = false;
	aiCar.mAutoGears = true;
	aiCar.mStartPosition = levelStateCarIndex;
	aiCar.mResprayID = 0; //Owing to manufacturers and licensing we can't randomly respray AI vehicles :( respray ID = 0 is the default colour
	aiCar.mUpgrades.OfflineValues();
}

} // namespace Game
