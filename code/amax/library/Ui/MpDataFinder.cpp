
#include "Stdafx.h"
#include "MpDataFinder.h"
#include "UiHelpers.h"

#include <ui/Game/UILocaliser.h>
#include "UI/Game/Multiplayer/MultiplayerCache.h"
#include <Ui/UiInterface.h>
#include "Ui/SoundEffects.h"

#include "Game/Game.h"
#include "GameLogic/Structures/LevelState.h"
#include "GameLogic/Structures/MpResults.h"
#include "GameLogic/Structures/MultiplayerAwards.h"
#include "GameLogic/GameData/MultiplayerData.h"
#include "GameLogic/GameModes/GameModeMultiplayer.h"

#include "GameLogic/MultiplayerChallengeSystem.h"
#include <Game\GameActors\Components\GameComponents.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Player/NetPlayerConnection.h>
#include <Network/Player/NetPlayerStates.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Damage.h>
#include "GameApp.h"
#include "GameLogic\Layers\RaceBookLayer.h"
#include "Game\GameActors\VehicleLogic\VehicleLogicActor.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

using namespace RaceBook;

namespace UI
{

C_MpDataFinder C_MpDataFinder::m_sInstance;

void C_MpDataFinder::Init()
{
	//
}

const WCHAR* C_MpDataFinder::GetVariableText( const char* pVariableName ) 
{
	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	m_VariableText.Clear();

	//Precache some stuff
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	if ( !strcmp( pVariableName, "HUD_GOAL" ) )
	{
		switch ( levelState.mRaceMode )
		{
			case Game::E_RaceMode_Multiplayer_Racing:
				m_VariableText.Format( C_TextStringID( "MP_HUD_TARGET_RACE" ).String() );
				break;

			case Game::E_RaceMode_Multiplayer_Elimination:
				m_VariableText.Format( C_TextStringID( "MP_HUD_TARGET_ELIMINATION" ).String() );
				break;

			case Game::E_RaceMode_Multiplayer_Points:
			{
				if ( levelState.GetMultiplayerSettings().mScoreLimit > 0 )
					m_VariableText.Format( C_TextStringID( "MP_HUD_TARGET_POINTS_LIMIT" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( levelState.GetMultiplayerSettings().mScoreLimit ) );
				else
					m_VariableText.Format( C_TextStringID( "MP_HUD_TARGET_POINTS_MOST" ).String() );

				break;
			}

			case Game::E_RaceMode_Multiplayer_Destruction:
				m_VariableText.Format( C_TextStringID( "MP_HUD_TARGET_DESTRUCTION" ).String() );
				break;

			default:
				m_VariableText.Format( L"" );
				break;
		}
	}
	else
	if ( strncmp( pVariableName, "POB_", 4 ) == 0 )
	{
		pVariableName += 4;
		if ( strncmp( pVariableName, "RANKUP_TEXT", 11 ) == 0 )
		{
			pVariableName += 11;

			S32 Rank = (S32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( C_TextStringID( "POB_UNLOCKS_RANK_TEXT_1" ).String(), Rank );
		}
		else 
		if ( strncmp( pVariableName, "UPCOMING_RANKUP_TEXT", 20 ) == 0 )
		{
			pVariableName += 20;

			S32 Rank = (S32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( C_TextStringID( "POB_UPCOMING_UNLOCKS_RANK_TEXT_1" ).String(), Rank );
		}

	}
	else
	if ( strncmp( pVariableName, "STATS", 5 ) == 0 )
	{
		pVariableName += 5;

		if ( strncmp( pVariableName, "_RANK_TITLE", 11 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( "MP_RANK_AND_TITLE" ).String(), pMultiplayerProfile->GetCurrentLevel() + 1 );
		}
		else
		if ( strncmp( pVariableName, "_RANK", 5 ) == 0 )
		{
			m_VariableText.Format( L"%d", pMultiplayerProfile->GetCurrentLevel() + 1 );
		}
		else
		if ( strncmp( pVariableName, "_FANS_NEEDED", 12 ) == 0)
		{
			if ( pMultiplayerProfile->GetCurrentLevel() > multiplayerConfig->m_LevelingData.count )
				return L"INVALID RANK";

			const S32 nextLevel = pMultiplayerProfile->GetCurrentLevel() + 1; // we start at 0

			if ( nextLevel < multiplayerConfig->m_LevelingData.count )
			{
				const S32 fansNeeded = multiplayerConfig->m_LevelingData[nextLevel].mFansNeeded;

				m_VariableText.Format( C_TextStringID( "MP_FANS_NEEDED" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( fansNeeded - pMultiplayerProfile->NumFans() ), nextLevel + 1 );
			}
			else
			{
				m_VariableText.Format( C_TextStringID( "MP_MAX_RANK" ).String() );
			}

		}
		else
		if ( strncmp( pVariableName, "_FANS", 5 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( "MP_NUM_FANS" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->NumFans() ) );
		}
		else
		if ( strncmp( pVariableName, "_RATING", 7 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->GetMultiplayerStatistics().mNumRacePoints ) );
		}
		else
		if ( strncmp( pVariableName, "_TOTAL_WINS", 11 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->GetMultiplayerStatistics().mNumEventsWon ) );
		}
		else
		if ( strncmp( pVariableName, "_TOTAL_RACES", 12 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->GetMultiplayerStatistics().mNumEventsPlayed ) );
		}
		else
		if ( strncmp( pVariableName, "_POWERUP_RATIO", 14 ) == 0 )
		{
			F32 fRatio = 0.0f;

			if(pMultiplayerProfile->GetMultiplayerStatistics().mTotalPickupsFired > 0)
				fRatio = ((float)pMultiplayerProfile->GetMultiplayerStatistics().mTotalPickupsHit / (float)pMultiplayerProfile->GetMultiplayerStatistics().mTotalPickupsFired) * 100.0f;

			m_VariableText.Format( C_TextStringID( "MP_STAT_POWERUP_RATIO" ).String(), fRatio );
		}
		else
		if ( strncmp( pVariableName, "_TOTAL_PLAYTIME", 15 ) == 0 )
		{
			F32 time = ( (F32)pMultiplayerProfile->GetMultiplayerStatistics().mTotalPlaytime / SYS_USEC_PER_SEC );
			m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTimeDHM( time ) );
		}
		else
		if ( strncmp( pVariableName, "_TOTAL_FANS", 11 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->GetMultiplayerStatistics().mNumLifetimeFans ) );
		}
		else
		if( strncmp( pVariableName, "_ON_TRACK_FANS", 14 ) == 0 )
		{
			pVariableName += 14;
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

			const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( raceSummary.m_OnTrackFans ) );
		}
		else
		if( strncmp( pVariableName, "_FINISH_BONUS", 13 ) == 0 )
		{
			pVariableName += 13;
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

			const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( raceSummary.m_FinishBonus ) );
		}
		else
		if( strncmp( pVariableName, "_TEAM_BONUS", 13 ) == 0 )
		{
			pVariableName += 13;
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

			const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( raceSummary.m_TeamBonus ) );
		}
		else
		if( strncmp( pVariableName, "_POSITION_FINISH_BONUS", 22 ) == 0 )
		{
			pVariableName += 22;
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

			const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();

			m_VariableText.Format( C_TextStringID( "MP_PLACE_RESULTS" ).String(), raceSummary.m_Position, C_UILocaliser::Instance().ConvertNumberToPosition( raceSummary.m_Position, false, NULL, NULL, false, true ) );

		}
	}
	else
	if ( strncmp( pVariableName, "PLAYERS_GATHERED", 16 ) == 0 )
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if ( !services.IsInitialised() )
			return L"ERROR";

		RacerNet::NetRaceSettings raceSettings;
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetNetRaceSettings( raceSettings );

		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		const S32 min = ( mgr.Size(false) == 0 ) ? 1 : mgr.Size(false);
		m_VariableText.Format( C_TextStringID( "POP_SEARCHING_FOR_PLAYERS" ).String(), min, raceSettings.m_MaxDrivers );
	}
	else
	if ( strncmp( pVariableName, "RESULT", 6 ) == 0 ) 
	{
		pVariableName = GetRaceResults(pVariableName, levelState, pMultiplayerProfile);
	}
	else
	if ( strncmp( pVariableName, "CHAMPIONSHIP_STANDINGS", 22 ) == 0 ) 
	{
		pVariableName = GetChampionshipStandings(pVariableName, levelState, pMultiplayerProfile);
	}
	else
	if ( strncmp( pVariableName, "RACE_SUMMARY", 12 ) == 0 )
	{
		pVariableName += 12;

		const Game::MpRaceSummary& summary = pMultiplayerProfile->GetRaceSummary();

		const S8 padIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();

		if ( strncmp( pVariableName, "_TRACK", 6 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( summary.m_OnTrackFans ) );
		}
		else
		if ( strncmp( pVariableName, "_POSITION", 9 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( "MP_FINISH_POSITION" ).String(), C_UILocaliser::Instance().ConvertNumberToPosition( summary.m_Position ) );
		}
		else
		if ( strncmp( pVariableName, "_FINISH", 7 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( summary.m_FinishBonus ) );
		}
		else
		if ( strncmp( pVariableName, "_TEAM_BONUS", 7 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( summary.m_TeamBonus ) );
		}
		else
		if ( strncmp( pVariableName, "_CAR_REWARD_FAN_BONUS", 21 ) == 0 ) 
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( summary.m_CarRewardFanBonus ) );
		}
		else
		if ( strncmp( pVariableName, "_TOTAL", 6 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( summary.m_TotalNewFans ) );
		}
		else
		if ( strncmp( pVariableName, "_CHALLENGE", 10 ) == 0 )
		{
			pVariableName += 10;
			const S32 index = (S32)ParseIndex( pVariableName, "" ) - 1;

			const Game::C_MultiplayerChallengeTracker::TChallengeList list = pMultiplayerProfile->GetChallengeSystem().GetTracker().GetChallengesComplete();
			const Game::C_MultiplayerChallenge& challenge = pMultiplayerProfile->GetChallengeSystem().GetChallenge( list[index].type );

			C_WideString8 numeral;
			for ( S32 i = 0; i <= list[index].tier; i++ )
				numeral.Add( L"I" );

			m_VariableText.Format( L"%s %s +%s", C_TextStringID( challenge.GetName().id ).String(), numeral.Chars(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetFansReward( list[index].tier ) ) );
		}
		else
		if ( !strcmp( pVariableName, "_SHOWY_FLOURISH" ) )
		{
			m_VariableText.Format( C_TextStringID( "MP_RESULT_SHOWY_FLOURISH" ).String(), summary.m_ShowyFlourishContribution );
		}
		else
		if ( !strcmp( pVariableName, "_FAN_FAVOURITE" ) )
		{
			m_VariableText.Format( C_TextStringID( "MP_RESULT_FAN_FAVOURITE" ).String(), summary.m_FanDemandContribution );
		}
	}
	else
	if ( strncmp( pVariableName, "CAR_UPGRADE", 11 ) == 0 )
	{
		pVariableName += 11;

		if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
		{
			pVariableName += 5;

			const UI::E_MULTIPLAYER_CHALLENGE_TYPE type = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)ParseIndexAdvance( pVariableName, "" );
			const S32 tier = (S32)ParseIndex( pVariableName, "_" );

			const Game::C_MultiplayerChallenge& challenge = pMultiplayerProfile->GetChallengeSystem().GetChallenge( type );
			m_VariableText.Format( L"%s ", C_TextStringID( challenge.GetName().id ).String() );
			
			// add numeral. Should proabaly make a localiser function to do this
			for ( S32 i = 0; i <= tier; i++ )
				m_VariableText.Add( L"I" );
		}
		else
		if ( strncmp( pVariableName, "_DESC", 5 ) == 0 )
		{
			pVariableName += 5;

			const UI::E_MULTIPLAYER_CHALLENGE_TYPE type = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)ParseIndexAdvance( pVariableName, "" );
			const S32 tier = (S32)ParseIndex( pVariableName, "_" );

			const Game::C_MultiplayerChallenge& challenge = pMultiplayerProfile->GetChallengeSystem().GetChallenge( type );

			if ( challenge.GetTargetAmount( tier ) == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount() ), C_TextStringID( "CMN_SWAP_RACE" ).String() );
			}
			else
				m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount() ), C_TextStringID( "CMN_SWAP_RACES" ).String() );
		}
		else
		if ( strncmp( pVariableName, "_TYPE", 5 ) == 0 )
		{
			pVariableName += 5;

			const UI::E_MULTIPLAYER_CHALLENGE_TYPE type = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)ParseIndexAdvance( pVariableName, "" );
			const S32 tier = (S32)ParseIndex( pVariableName, "_" );

			const Game::C_MultiplayerChallenge& challenge = pMultiplayerProfile->GetChallengeSystem().GetChallenge( type );

			switch( challenge.GetReward( tier ) )
			{
				case E_MultiplayerVehicleUpgrades_HealthSlot	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_HEALTH_SLOT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_FanBonus		: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_FAN_BONUS" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_SpeedBoost	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_SPEED_BOOST" ).String() );	break;

				case E_MultiplayerVehicleUpgrades_Pearlescent	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_PEARLESCENT_PAINT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_Metallic		: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_METALLIC_PAINT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_Candy			: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_CANDY" ).String() );	break;
			}
		}
	}
	else
	if ( strncmp( pVariableName, "FINISHED_NAME", 13 ) == 0 ) 
	{
		pVariableName += 13;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index >= 0 )
		{
			const S8 localIndex = Game::C_MpResults::Instance().m_FinishedOrder[index].m_Index;

			if ( localIndex > -1 )
				m_VariableText.Format( L"%s", levelState.mCars[localIndex].mNameAboveCar.Chars() );
		}
	}
	else
	if ( strncmp( pVariableName, "PLAYLIST_RACE", 13 ) == 0 )
	{
		pVariableName += 13;
		const S32 raceIndex = (S32)ParseIndexAdvance( pVariableName, "" );

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pLayer->GetGameModePtr()->IsMultiplayer() )
			return L"";

		Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());

		// [NorbertSz] #37034 - to be on the safe side
		if (!UI::C_MultiplayerCache::Instance().IsCurrentEventValid())
			return L"";

		// [WillB] #35638 - Swapped call to use safer version incase the game mode shuts down and we have to render for a frame.
		const MultiplayerRace* pRace = pGameMode->GetPlaylist().GetRaceData( raceIndex );
		if ( !pRace )
			return L"";

		if ( strncmp( pVariableName, "_CITY", 5 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( pRace->route->city->loc_string.id ).String() );
		}
		else
		if ( strncmp( pVariableName, "_ROUTE", 6 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( pRace->route->loc_string.id ).String() );
		}
		else
		if ( strncmp( pVariableName, "_LAP_TEXT", 9 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( ( pRace->laps == 1 ) ? "CMN_LAP" : "CMN_LAPS" ).String() );		
		}
		else
		if ( strncmp( pVariableName, "_LAPS", 5 ) == 0 )
		{
			m_VariableText.Format( L"%d", pRace->laps );
		}
		else
		if ( strncmp( pVariableName, "_NUM_LAPS", 9 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( ( pRace->laps == 1 ) ? "MP_RACE_END_LAP" : "MP_RACE_END_LAPS" ).String(), pRace->laps );		
		}
		else
		if ( strncmp( pVariableName, "_TIME", 5 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( (F32)pRace->time ) );
		}
		else
		if ( strncmp( pVariableName, "_CLASS", 6 ) == 0 )
		{
			switch( pGameMode->GetPlaylist().GetCarClass( raceIndex ) )
			{
				case E_VehicleClass_A : m_VariableText.Format( C_TextStringID( "MP_CLASS_A" ).String() );	break;
				case E_VehicleClass_B : m_VariableText.Format( C_TextStringID( "MP_CLASS_B" ).String() );	break;
				case E_VehicleClass_C : m_VariableText.Format( C_TextStringID( "MP_CLASS_C" ).String() );	break;
				case E_VehicleClass_D : m_VariableText.Format( C_TextStringID( "MP_CLASS_D" ).String() );	break;
				case E_VehicleClass_Any : m_VariableText.Format( C_TextStringID( "MP_CLASS_ANY" ).String() );	break;
			}
		}
		else
		if ( strncmp( pVariableName, "_TYPE", 5 ) == 0 )
		{
			switch( pRace->type )
			{
				case E_RaceType_Racing			: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_RACING" ).String() );			break;
				case E_RaceType_Elimination		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_ELIMINATION" ).String() );		break;
				case E_RaceType_Points			: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_POINTS" ).String() );			break;
				case E_RaceType_Destruction		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_DESTRUCTION" ).String() );		break;
				case E_RaceType_Hardcore		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_HARDCORE" ).String() );			break;
			}
		}
	}
	else
	if ( strncmp( pVariableName, "CUSTOM_RACE", 11 ) == 0 ) 
	{
		pVariableName += 11;

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pLayer->GetGameModePtr()->IsMultiplayer() )
			return L"";

		Game::C_GameModeMultiPlayer* pGameMode = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr());
		const Game::C_CustomRace& customRace = pGameMode->GetCustomRace();

		if ( strncmp( pVariableName, "_CITY", 5 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( C_GameData::Instance().mCities[(GAMEDATA_ID)customRace.GetCity() ]->loc_string.id ).String() );
		}
		else
		if ( strncmp( pVariableName, "_ROUTE", 6 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( C_GameData::Instance().mRoutes[(GAMEDATA_ID)customRace.GetRoute() ]->loc_string.id ).String() );
		}
		else
		if ( strncmp( pVariableName, "_LAP_TEXT", 9 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( ( customRace.GetLapLimit() == 1 ) ? "CMN_LAP" : "CMN_LAPS" ).String() );		
		}
		else
		if ( strncmp( pVariableName, "_LAPS", 5 ) == 0 )
		{
			m_VariableText.Format( L"%d", customRace.GetLapLimit() );
		}
		else
		if ( strncmp( pVariableName, "_NUM_LAPS", 9 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( ( customRace.GetLapLimit() == 1 ) ? "MP_RACE_END_LAP" : "MP_RACE_END_LAPS" ).String(), customRace.GetLapLimit() );		
		}
		else
		if ( strncmp( pVariableName, "_TIME", 5 ) == 0 )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( customRace.GetTimeLimit() ) );
		}
		else
		if ( strncmp( pVariableName, "_CLASS", 6 ) == 0 )
		{
			switch( customRace.GetCarClass() )
			{
				case E_VehicleClass_A : m_VariableText.Format( C_TextStringID( "MP_CLASS_A" ).String() );	break;
				case E_VehicleClass_B : m_VariableText.Format( C_TextStringID( "MP_CLASS_B" ).String() );	break;
				case E_VehicleClass_C : m_VariableText.Format( C_TextStringID( "MP_CLASS_C" ).String() );	break;
				case E_VehicleClass_D : m_VariableText.Format( C_TextStringID( "MP_CLASS_D" ).String() );	break;
				case E_VehicleClass_Any : m_VariableText.Format( C_TextStringID( "MP_CLASS_ANY" ).String() );	break;
			}
		}
		else
		if ( strncmp( pVariableName, "_TYPE", 5 ) == 0 )
		{
			switch( customRace.GetRaceMode() )
			{
				case E_RaceType_Racing			: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_RACING" ).String() );			break;
				case E_RaceType_Elimination		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_ELIMINATION" ).String() );		break;
				case E_RaceType_Points			: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_POINTS" ).String() );			break;
				case E_RaceType_Destruction		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_DESTRUCTION" ).String() );		break;
				case E_RaceType_Hardcore		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_HARDCORE" ).String() );			break;
			}
		}
	}
	else
	if ( strncmp( pVariableName, "PLAYLIST", 8 ) == 0 ) 
	{
		pVariableName += 8;
		const U32 index = (U32)ParseIndex( pVariableName, "", true );

		if ( index < 0 || index >= C_MultiplayerCache::Instance().NumEvents() )
			return L"";

		const MultiplayerEvent* pEvent = C_MultiplayerCache::Instance().Event( index );
		if ( !pEvent )
			return L"";

		if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( pEvent->name.id ).String() );
		}
		else
		if ( strncmp( pVariableName, "_DESC", 5 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( pEvent->desc.id ).String() );
		}
	}
	else
	if ( strncmp( pVariableName, "TIMER", 5 ) == 0 )
	{
		pVariableName += 5;

		if ( strncmp( pVariableName, "_FIND_PLAYERS", 13 ) == 0 )
		{
			pVariableName += 13;

			const S32 time = (S32)ParseIndex( pVariableName, "" );
			m_VariableText.Format( C_TextStringID( "MP_TIMER_FIND_PLAYERS" ).String(), time );
		}
		else
		if ( strncmp( pVariableName, "_VOTE_START", 11 ) == 0 )
		{
			pVariableName += 11;

			const S32 time = (S32)ParseIndex( pVariableName, "" );
			m_VariableText.Format( C_TextStringID( "MP_TIMER_VOTE_START" ).String(), time );
		}
		else
		if ( strncmp( pVariableName, "_VOTE_TIME", 10 ) == 0 )
		{
			pVariableName += 10;

			const S32 time = (S32)ParseIndex( pVariableName, "" );
			m_VariableText.Format( L"%i", time );
		}
		else
		if ( strncmp( pVariableName, "_COUNTDOWN", 10 ) == 0 )
		{
			pVariableName += 10;

			const S32 time = (S32)ParseIndex( pVariableName, "" );
			m_VariableText.Format( C_TextStringID( "MP_TIMER_COUNTDOWN" ).String(), time );
		}
		else
		if ( strncmp( pVariableName, "_RIVALS_COUNTDOWN", 15 ) == 0 )
		{
			Game::C_RaceModeMultiplayer_Destruction* pRaceModeDestruction = (Game::C_RaceModeMultiplayer_Destruction*)(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr());

			if ( !pRaceModeDestruction->GetRivalsCountdownDelayActive() )
				return L"";

			const F32 time = pRaceModeDestruction->GetRivalsCountdownDelayTimer();
			m_VariableText.Format( L"%d", (S32)( time + 1 ) );
		}
		if ( strncmp( pVariableName, "_GASP_COUNTDOWN", 15 ) == 0 )
		{
			pVariableName += 15;

			const S32 time = (S32)ParseIndex( pVariableName, "" );
			m_VariableText.Format( L"%d", time );
		}
		
	}
	else
	if ( strncmp( pVariableName, "TEAM_A_RESULT", 13 ) == 0 )
	{
		pVariableName += 13;

		S32 score = Game::C_MpResults::Instance().m_Teams[0].m_TeamScore;
		m_VariableText.Format( C_TextStringID( ( score == 1 ) ? "MP_RESULT_PT" : "MP_RESULT_PTS" ).String(), score );
	}
	else
	if ( strncmp( pVariableName, "TEAM_B_RESULT", 13 ) == 0 )
	{
		pVariableName += 13;

		S32 score = Game::C_MpResults::Instance().m_Teams[1].m_TeamScore;
		m_VariableText.Format( C_TextStringID( ( score == 1 ) ? "MP_RESULT_PT" : "MP_RESULT_PTS" ).String(), score );
	}
	else
	if ( strncmp( pVariableName, "TOTAL_EVENTS", 12 ) == 0 )
	{
		pVariableName += 12;

		// TODO link this up
		m_VariableText.Format( L"%d", 0 );
	}
	else
	if ( strncmp( pVariableName, "TOTAL_ONLINE_PLAYERS", 20 ) == 0 )
	{
		pVariableName += 20;

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( services.GetOnlinePlayerCount(false) ) );
	}
	else
	if ( strncmp( pVariableName, "TOTAL_RANKED_PLAYERS", 19 ) == 0 )
	{
		pVariableName += 19;
		
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( services.GetOnlinePlayerCount() ) );
	}
	else
	if ( strncmp( pVariableName, "ACTIVE_PLAYERS", 14 ) == 0 )
	{
		pVariableName += 14;

		const S32 menuIndex = (S32)ParseIndex( pVariableName, "" );
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( services.GetRankedEventPlayerCount(menuIndex) ) );
	}
	else
	if ( strncmp( pVariableName, "RETURN_TO_LOBBY", 15 ) == 0 )
	{
		pVariableName += 15;

		const S32 time = (S32)ParseIndex( pVariableName, "" );
		m_VariableText.Format( C_TextStringID( "MP_RETURNING_TO_LOBBY" ).String(), time );
	}
	else
	if ( strncmp( pVariableName, "LEAVING_GAME_IN", 15 ) == 0 )
	{
		pVariableName += 15;

		const S32 time = (S32)ParseIndex( pVariableName, "" );
		m_VariableText.Format( C_TextStringID( "MP_LEAVING_GAME_IN" ).String(), time );
	}
	else
	if ( strncmp( pVariableName, "NEXT_EVENT_IN", 13 ) == 0 )
	{
		pVariableName += 13;

		const S32 time = (S32)ParseIndex( pVariableName, "" );
		m_VariableText.Format( C_TextStringID( "MP_NEXT_EVENT_IN" ).String(), time );
	}

	else
	if ( strncmp( pVariableName, "PLAYER_CONTROL_IN", 17 ) == 0 )
	{
		pVariableName += 17;

		const S32 time = (S32)ParseIndex( pVariableName, "" );
		m_VariableText.Format( C_TextStringID( "MP_E3_COUNTDOWN" ).String(), time );
	}
	else
	if ( strncmp( pVariableName, "VOTE_TIMER", 10 ) == 0 )
	{
		pVariableName += 10;

		const S32 time = (S32)ParseIndex( pVariableName, "" );
		m_VariableText.Format( C_TextStringID( "MP_VOTE_TIMER" ).String(), time );
	}
	else
	if ( strncmp( pVariableName, "VOTES", 5 ) == 0 )
	{
		static S32 skipCountPrev = 0;

		pVariableName += 5;

		U32 skipCount = 0;
		U32 skipTotal = 0;
	
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning())
			{
				RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
				for (U8 i=0; i<mgr.TotalSize(); i++)
				{
					RacerNet::C_NetPlayer* p = mgr.Index(i);
					if (p && p->m_RaceConnection.IsActive())
					{
						// only players who raced this current event will be considered for skipping the next race.
						switch (p->m_RaceConnection.m_PlayerState)
						{
						case RacerNet::eNetPlayerStateIdle:
							skipTotal += 1;
							if (p->m_RaceData.m_BitArray.GetBitValue(RacerNet::NetPlayerRaceGameData::eBitArrayData_WantsToSkipNextRace))
								skipCount += 1;
							break;
						}
					}
				}
			}
		}

		if ( skipCount != skipCountPrev )
		{
			skipCountPrev = skipCount;

			if ( skipCount != 0 )	// for resetting itself
				UiInterface()->PlaySound( E_UiSoundEffect_LobbyVoteCountActionIn );
		}

		m_VariableText.Format( C_TextStringID( "MP_VOTES" ).String(), skipCount, skipTotal );
	}
	else
	if ( strncmp( pVariableName, "LEFT_VOTES", 10 ) == 0 )
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pLayer->GetGameModePtr()->IsMultiplayer() )
			return L"";

		const Game::VoteInfo& info = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr())->GetVoteInfo();
		m_VariableText.Format( C_TextStringID( "MP_VOTES_CAST" ).String(), info.countA );
	}
	else
	if ( strncmp( pVariableName, "RIGHT_VOTES", 11 ) == 0 )
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pLayer->GetGameModePtr()->IsMultiplayer() )
			return L"";

		const Game::VoteInfo& info = ((Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr())->GetVoteInfo();
		m_VariableText.Format( C_TextStringID( "MP_VOTES_CAST" ).String(), info.countB );
	}
	else
	if ( strncmp( pVariableName, "SPECTATE", 8 ) == 0 )
	{
		pVariableName += 8;

		const U32 index = (U32)ParseIndex( pVariableName, "", true );
		const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( index );

		if ( !pLogicActor )
			return L"<MPL_ERROR>";

		if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
		{
			Game::C_LevelState& levelState = GetLevelState();
			m_VariableText.Format( L"%s", levelState.mCars[ index ].mNameAboveCar.Chars() );
		}
		else
		if ( strncmp( pVariableName, "_POSITION", 9 ) == 0 )
		{
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition ) );
		}
		else
		if ( strncmp( pVariableName, "_RANK", 5 ) == 0 )
		{
			Game::C_LevelState& levelState = GetLevelState();
			m_VariableText.Format( L"%d", levelState.mCars[ index ].mMultiplayerRank + 1 );
		}
		else
		if ( strncmp( pVariableName, "_LAP", 4 ) == 0 )
		{
			int currentLap = pLogicActor->GetLapInfo().mCurrentLap;
			int numberOfLaps = GetLevelState().mLaps;

			//Bug 25999 - We don't want to be showing 4/3 laps
			if(currentLap > numberOfLaps)
				currentLap = numberOfLaps;

			m_VariableText.Format( C_TextStringID( "HUD_CURRENT_LAP" ).String(), currentLap, numberOfLaps );
		}
	}
	else
	if ( !strncmp( pVariableName, "SPLITSCREEN", 11 ) )
	{
		pVariableName += 11;

		//Get the game mode
		if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() != Game::E_GameMode_SplitScreen )
			return L"";

		Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

		if ( !strncmp( pVariableName, "_TOTAL_PLAYTIME", 15 ) )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTimeDHM( pGameMode->GetTotalRaceTime() ) );
		}
		else
		if ( !strncmp( pVariableName, "_TOTAL_GAMES", 12 ) )
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalRaces() ) );
		}
		else
		if ( !strncmp( pVariableName, "_FINISHED_POSITION", 18 ) )
		{
			pVariableName += 18;

			S32 sPosition = (S32)ParseIndexAdvance( pVariableName, "_" );
			m_VariableText.Format( C_TextStringID( "MP_PLACE_RESULTS" ).String(), sPosition, C_UILocaliser::Instance().ConvertNumberToPosition( sPosition, false, NULL, NULL, false, true ) );
		}
		else
		{
			// player specific
			S32 sViewportIndex = (S32)ParseIndexAdvance( pVariableName, "" );
			if ( sViewportIndex == -1 )
				return L"";

			static C_TextStringID idPoints( "CMN_SPLIT_SCREEN_POINTS" );
			
			if ( !strcmp( pVariableName, "_POINTS_TOTAL" ) )
			{
				S32 sTotalPoints = pGameMode->GetTotalScore(sViewportIndex);
				m_VariableText.Format( idPoints.String(), C_UILocaliser::Instance().ConvertNumberToCommas( sTotalPoints ) );
			}
			else
			if ( !strcmp( pVariableName, "_POINTS_RACE" ) )
			{
				S32 sRacePoints = pGameMode->GetScoreForRace(sViewportIndex);
				m_VariableText.Format( idPoints.String(), C_UILocaliser::Instance().ConvertNumberToCommas( sRacePoints ) );
			}
			else
			if ( !strcmp( pVariableName, "_FANS_TOTAL" ) )
			{
				S32 sTotalFans = pGameMode->GetTotalFans(sViewportIndex);
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( sTotalFans ) );
			}
			else
			if ( !strcmp( pVariableName, "_FANS_RACE" ) )
			{
				S32 sRaceFans = pGameMode->GetFansForRace(sViewportIndex);
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( sRaceFans ) );
			}
			else
			if ( !strcmp( pVariableName, "_WINS_TOTAL" ) )
			{
				m_VariableText.Format( C_TextStringID( "MP_SPLITSCREEN_TOTAL_WINS" ).String(), pGameMode->GetTotalWins(sViewportIndex) );
			}
			else
			if ( !strcmp( pVariableName, "_STANDINGS_POINTS" ) )
			{
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalScore(sViewportIndex) ) );
			}
			else
			if ( !strcmp( pVariableName, "_STANDING_WINS" ) )
			{
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalWins(sViewportIndex) ) );
			}
		}
	}
	else
	if ( strncmp( pVariableName, "LEVEL", 5 ) == 0 )
	{
		pVariableName += 5;

		if ( strncmp( pVariableName, "_FANS_NEEDED", 12 ) == 0 )
		{
			pVariableName += 12;
			const S32 fans = (S32)ParseIndexAdvance( pVariableName, "" );
			const S32 rank = (S32)ParseIndex( pVariableName, "_" );

			const S32 maxRank = multiplayerConfig->m_LevelingData.count;

			// check we havent reached the max rank
			if ( rank >= maxRank && fans == 0 )
			{
				m_VariableText.Format( C_TextStringID( "MP_MAX_RANK" ).String() );
			}	
			else
			{
				m_VariableText.Format( C_TextStringID( "MP_FANS_NEEDED" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( fans ), rank );
			}
		}
		else
		if ( strncmp( pVariableName, "_NUM_FANS", 9 ) == 0 )
		{
			pVariableName += 9;
			const U32 fans = (U32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( C_TextStringID( "MP_NUM_FANS" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( fans ) );
		}
		else
		if ( strncmp( pVariableName, "_RANK", 5 ) == 0 )
		{
			pVariableName += 5;
			const U32 rank = (U32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( C_TextStringID( "MP_RANK_AND_TITLE" ).String(), rank );
		}
		else
		if ( strncmp( pVariableName, "_UNLOCK_ABILITY", 15 ) == 0 )
		{
			pVariableName += 15;

			m_VariableText.Format( L"Unlocked Some Ability" );
		}
	}
	else
	if ( strncmp( pVariableName, "CHALLENGE_PACK", 14 ) == 0 )
	{
		pVariableName += 14;

		const S32 packIndex = (S32)ParseIndexAdvance( pVariableName, "" ) - 1;
		const MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[packIndex];

		// Get the challenge system associated with the active profile
		const Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();

		if ( strncmp( pVariableName, "_PROGRESS", 9 ) == 0 )
		{
			// Special case for this
			if ( pPack->mType == E_ChallengePack_Car )
			{
				m_VariableText.Format( L"%d/%d", challengeSystem.GetCarPackCounter(), challengeSystem.GetCarPackTotal() );
			}
			else
				m_VariableText.Format( L"%d/%d", challengeSystem.GetPackProgressCounter( pPack->mType ), pPack->mChallenges.count );
		}
	}
	else
	if ( strncmp( pVariableName, "CHALLENGE_CLASS_PROGRESS", 24 ) == 0 )
	{
		pVariableName += 24;

		// Get the challenge system associated with the active profile
		const Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();

		S32 amount = 0, max = 0;
		challengeSystem.GetCarClassProgress( (E_VehicleClass)(S32)ParseIndex( pVariableName, "" ), amount, max );

		m_VariableText.Format( L"%d/%d", amount, max );
	}
	else
	if ( strncmp( pVariableName, "CAR_CHALLENGE", 13 ) == 0 )
	{
		pVariableName += 13;

		const U32 carID = (U32)ParseIndexAdvance( pVariableName, "" );
		const UI::E_MULTIPLAYER_CHALLENGE_TYPE type = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)(S32)ParseIndexAdvance( pVariableName, "_" );

		// Get the challenge system associated with the active profile
		Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();
		Game::C_MultiplayerChallenge& challenge = challengeSystem.GetCarChallenge( carID, type );

		if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
		{
			if ( challenge.GetMaxTiers() == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetName().id ).String() );
			}
			else
			{
				m_VariableText.Format( L"%s ", C_TextStringID( challenge.GetName().id ).String() );

				for ( S32 i = 0; i <= challenge.GetTier(); i++ )
					m_VariableText.Add( L"I" );
			}
		}
		else
		if ( strncmp( pVariableName, "_DESC", 5 ) == 0 )
		{
			if ( challenge.GetTargetAmount() == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount() ), C_TextStringID( "CMN_SWAP_RACE" ).String() );
			}
			else
				m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount() ), C_TextStringID( "CMN_SWAP_RACES" ).String() );
		}
		else
		if ( strncmp( pVariableName, "_PROGRESS", 9 ) == 0 )
		{
			m_VariableText.Clear();
			m_VariableText.Format( L"%d / %d", challenge.GetAmount(), challenge.GetTargetAmount() );
		}
		else
		if ( strncmp( pVariableName, "_REWARD", 7 ) == 0 )
		{
			pVariableName += 7;

			switch( challenge.GetReward( challenge.GetTier() ) )
			{
				case E_MultiplayerVehicleUpgrades_HealthSlot	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_HEALTH_SLOT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_FanBonus		: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_FAN_BONUS" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_SpeedBoost	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_SPEED_BOOST" ).String() );	break;

				case E_MultiplayerVehicleUpgrades_Pearlescent	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_PEARLESCENT_PAINT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_Metallic		: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_METALLIC_PAINT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_Candy			: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_CANDY" ).String() );	break;

				default : C_TextStringID( "MP_CHALLENGE_REWARD_NONE" ).String(); break;
			}
		}
		else if ( strncmp( pVariableName, "_PREV_NAME", 10 ) == 0 )
		{
			if ( challenge.GetMaxTiers() == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetName().id ).String() );
			}
			else
			{
				m_VariableText.Format( L"%s ", C_TextStringID( challenge.GetName().id ).String() );

				for ( S32 i = 0; i <= (challenge.GetTier()-1); i++ )
					m_VariableText.Add( L"I" );
			}
		}
		else
		if ( strncmp( pVariableName, "_PREV_DESC", 10 ) == 0 )
		{
			if ( challenge.GetTargetAmount() == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount(true) ), C_TextStringID( "CMN_SWAP_RACE" ).String() );
			}
			else
				m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount(true) ), C_TextStringID( "CMN_SWAP_RACES" ).String() );
		}
		else
		if ( strncmp( pVariableName, "_PREV_PROGRESS", 14 ) == 0 )
		{
			m_VariableText.Clear();
			m_VariableText.Format( L"%d / %d", challenge.GetTargetAmount(true), challenge.GetTargetAmount(true) );
		}
		else
		if ( strncmp( pVariableName, "_PREV_REWARD", 12 ) == 0 )
		{
			pVariableName += 12;

			switch( challenge.GetReward( challenge.GetTier()-1 ) )
			{
				case E_MultiplayerVehicleUpgrades_HealthSlot	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_HEALTH_SLOT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_FanBonus		: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_FAN_BONUS" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_SpeedBoost	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_SPEED_BOOST" ).String() );	break;

				case E_MultiplayerVehicleUpgrades_Pearlescent	: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_PEARLESCENT_PAINT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_Metallic		: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_METALLIC_PAINT" ).String() );	break;
				case E_MultiplayerVehicleUpgrades_Candy			: m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_REWARD_CANDY" ).String() );	break;

				default : C_TextStringID( "MP_CHALLENGE_REWARD_NONE" ).String(); break;
			}
		}
	}
	else
	if ( strncmp( pVariableName, "CHALLENGE", 9 ) == 0 )
	{
		pVariableName += 9;

		const UI::E_MULTIPLAYER_CHALLENGE_TYPE type = (UI::E_MULTIPLAYER_CHALLENGE_TYPE)(S32)ParseIndexAdvance( pVariableName, "" );

		// Get the challenge system associated with the active profile
		const Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();
		const Game::C_MultiplayerChallenge& challenge = challengeSystem.GetChallenge( type );

		pVariableName++;	// the "_"

		S32 sTier = (S32)ParseIndexAdvance( pVariableName, "" );

		if ( sTier == -1 )	// we're not overriding tier
			sTier = challenge.GetTier();
		else
			pVariableName++;	// the other "_"
		
		if ( strncmp( pVariableName, "NAME", 4 ) == 0 )
		{
			if ( challenge.GetMaxTiers() == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetName().id ).String() );
			}
			else
			{
				m_VariableText.Format( L"%s ", C_TextStringID( challenge.GetName().id ).String() );

				for ( S32 i = 0; i <= sTier; i++ )
					m_VariableText.Add( L"I" );
			}
		}
		else
		if ( strncmp( pVariableName, "DESC", 4 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount( sTier ) ) , challenge.GetData( sTier ) );
		}
		else
		if ( strncmp( pVariableName, "PROGRESS", 8 ) == 0 )
		{
			m_VariableText.Clear();

			if ( challenge.HideProgress() )
			{
				m_VariableText.Format( ( challenge.IsCompleted() ? C_TextStringID( "CMN_COMPLETE" ).String() : C_TextStringID( "CMN_INCOMPLETE" ).String() ) );
			}
			else
			{
				m_VariableText.Format( L"%d / %d", challenge.GetAmount(), challenge.GetTargetAmount() );
			}
		}
		else
		if ( strncmp( pVariableName, "FANS", 4 ) == 0 )
		{
			m_VariableText.Format( L"%d", challenge.GetFansReward( sTier ) );
		}
		else
		if ( strncmp( pVariableName, "FAN_REWARD", 10 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( "MP_CHALLENGE_FAN_REWARD" ).String(), challenge.GetFansReward( sTier ) );
		}
		else
		if ( strncmp( pVariableName, "PREV_NAME", 4 ) == 0 )
		{
			if ( challenge.GetMaxTiers() == 1 )
			{
				m_VariableText.Format( C_TextStringID( challenge.GetName().id ).String() );
			}
			else
			{
				m_VariableText.Format( L"%s ", C_TextStringID( challenge.GetName().id ).String() );

				for ( S32 i = 0; i <= (sTier-1); i++ )
					m_VariableText.Add( L"I" );
			}
		}
		else
		if ( strncmp( pVariableName, "PREV_DESC", 4 ) == 0 )
		{
			m_VariableText.Format( C_TextStringID( challenge.GetDesc().id ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount( (sTier-1) ) ) , challenge.GetData( (sTier-1) ) );
		}
		else
		if ( strncmp( pVariableName, "PREV_PROGRESS", 8 ) == 0 )
		{
			m_VariableText.Clear();

			if ( challenge.HideProgress() )
			{
				m_VariableText.Format( ( challenge.IsCompleted() ? C_TextStringID( "CMN_COMPLETE" ).String() : C_TextStringID( "CMN_INCOMPLETE" ).String() ) );
			}
			else
			{
				m_VariableText.Format( L"%d / %d", challenge.GetTargetAmount(true), challenge.GetTargetAmount(true) );
			}
		}
	}
	else
	if ( strncmp( pVariableName, "LOADOUT", 7 ) == 0 )
	{
		pVariableName += 7;

		if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
		{
			pVariableName +=5;

			bool inLobby = strncmp( pVariableName, "_LOBBY", 6 ) == 0;
			if(inLobby)
			{
				pVariableName +=6;
			}

			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			bool isRanked = !inLobby || (!pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked());

			const S32 index = (S32)ParseIndex( pVariableName, "" );

			const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( (UI::E_MULTIPLAYER_LOADOUT_TYPE)index);

			if(isRanked)
			{
				// Is a default loadout
				if ( index < E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM1 )
				{
					// If they have renamed it
					if ( loadOut.mLoadOutName.Length() )
					{
						m_VariableText.Format( loadOut.mLoadOutName.Chars() );
					}
					else
					{
						m_VariableText.Format( C_TextStringID( "MP_DEFAULT_LOADOUT" ).String() );
					}
				}
				else
				{
					// If they have renamed it
					if ( loadOut.mLoadOutName.Length() )
					{
						m_VariableText.Format( loadOut.mLoadOutName.Chars() );
					}
					else
					{
						switch(index)
						{
							case 1:
							{
								m_VariableText.Format( C_TextStringID( "MP_CUSTOM_LOADOUT_1" ).String() );
								break;
							}
							case 2:
							{
								m_VariableText.Format( C_TextStringID( "MP_CUSTOM_LOADOUT_2" ).String() );
								break;
							}
							case 3:
							{
								m_VariableText.Format( C_TextStringID( "MP_CUSTOM_LOADOUT_3" ).String() );
								break;
							}
							default:
							{
								m_VariableText.Format( C_TextStringID( "MP_CUSTOM_LOADOUT_1" ).String() );
								break;
							}
						}
					}
				}
			}
			else
			{
				m_VariableText.Format( C_TextStringID( "MP_CUTOM_LOADOUT_UNRANKED" ).String(), index+1);
			}
		}
		else
		if ( strncmp( pVariableName, "_INDEX", 6 ) == 0 )
		{
			pVariableName += 6;
			const S32 index = (S32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( L"%i.", index + 1 );
		}
	}
	else
	if ( strncmp( pVariableName, "AWARDS", 6 ) == 0 )
	{
		pVariableName += 6;

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

		const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
		const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

		if ( strncmp( pVariableName, "_OWNER", 6 ) == 0 )
		{
			pVariableName += 6;
			const S32 index = (S32)ParseIndex( pVariableName, "" ) - 1;

			const U8 vehicleIndex = awardSelection.m_Owners[index];
			m_VariableText.Format( levelState.mCars[vehicleIndex].mNameAboveCar.Chars() );
		}
		else
		if ( strncmp( pVariableName, "_RANK", 5 ) == 0 )
		{
			pVariableName += 5;
			const S32 index = (S32)ParseIndex( pVariableName, "" ) - 1;

			const U8 vehicleIndex = awardSelection.m_Owners[index];
			m_VariableText.Format( L"%d", levelState.mCars[vehicleIndex].mMultiplayerRank + 1 );
		}
		else
		if ( strncmp( pVariableName, "_AMOUNT", 7 ) == 0 )
		{
			pVariableName += 7;
			const S32 index = (S32)ParseIndex( pVariableName, "" ) - 1;
			const MultiplayerAwardData* pAward = awards.GetAward(awardSelection.m_Awards[index]).GetData();

			switch( pAward->mAwardStat )
			{
				case E_MultiplayerStat_Accuracy:
				{
					m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_PERCENTRAGE" ).String(), awardSelection.m_AwardValues[index] );
					break;
				}
				case E_MultiplayerStat_TopSpeed:
				{
					m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_SPEED" ).String(), (S32)awardSelection.m_AwardValues[index] );
					break;
				}
				case E_MultiplayerStat_DistanceTravelled:
				case E_MultiplayerStat_LongestDrift:
				case E_MultiplayerStat_LongestJump:
				{
					m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_DISTANCE" ).String(), (S32)awardSelection.m_AwardValues[index] );
					break;
				}
				case E_MultiplayerStat_KillStreak :
				case E_MultiplayerStat_DeathStreak :
				{
					m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_STREAK" ).String(), (S32)awardSelection.m_AwardValues[index] );
					break;
				}
				case E_MultiplayerStat_RaceTime:
				case E_MultiplayerStat_FastestLap:
				case E_MultiplayerStat_SlowestLap:
				case E_MultiplayerStat_TimeInvisibleWithSilentRunning:
				case E_MultiplayerStat_LongestTimeNotDamaged:
				case E_MultiplayerStat_LongestTimeWithoutBeingWrecked:
				case E_MultiplayerStat_TimeInFirstPlace:
				{
					m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( awardSelection.m_AwardValues[index] ) );
					break;
				}
				default :
				{
					// Default is numbers with commas
					m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( (S32)awardSelection.m_AwardValues[index] ) );
					break;
				}
			}
		}
	}
	else
	if ( strncmp( pVariableName, "ABILITY", 7 ) == 0 )
	{
		pVariableName += 7;

		if ( strncmp( pVariableName, "_LOCKED", 7 ) == 0 )
		{
			pVariableName += 7;

			m_VariableText.Format( C_TextStringID( "MP_UNLOCK_RANK" ).String(), (S32)ParseIndex( pVariableName, "" ) );
		}
	}
	else
	if ( strncmp( pVariableName, "UNLOCK_RANK", 11 ) == 0 )
	{
		pVariableName += 11;

		m_VariableText.Format( C_TextStringID( "MP_UNLOCK_RANK" ).String(), (S32)ParseIndex( pVariableName, "" ) );
	}
	else
	if ( strncmp( pVariableName, "UNLOCK_AT_RANK", 14 ) == 0 )
	{
		pVariableName += 14;

		m_VariableText.Format( C_TextStringID( "MP_UNLOCK_AT_RANK" ).String(), (S32)ParseIndex( pVariableName, "" ) );
	}
	else
	if( strncmp( pVariableName, "WRECKED_BY", 10 ) == 0)
	{
		pVariableName += 10;

		const U32 index = (U32)ParseIndex( pVariableName, "", true );
		const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( index );
		
		if(pLogicActor)
		{
			// Cache these
			Components::C_VehicleComponent_NewScoring* pScoringComponent = (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
			Components::C_VehicleComponent_Damage* pDamageComponent = (Components::C_VehicleComponent_Damage*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Damage);

			if(pScoringComponent)
			{
				// [JBunner] BUG: 37448 - Jump The Gun 'wrecked by' text switches to 'self destruct' on player wrecked sequence, as car respawns before wrecked sequence finished. 
				bool displayWreckedByAnotherPlayer = pScoringComponent->DisplayWreckedByAnotherPlayer();

				if(displayWreckedByAnotherPlayer)
				{
					// PR - only want to show names when we have names above the cars too
					if ( !ShowAiNames() )
						return L"";

					S32 otherVehIndex = -1;
						
					// [JBunner] BUG: 37448 
					otherVehIndex = pScoringComponent->GetIndexOfVehicleThatWreckedUs();
					
					if(otherVehIndex != -1 && otherVehIndex != Game::SYSTEM_FIRING_INDEX)
					{
						// If we are in a multiplayer motor mash.. then tell the player that this person is now their rival
						Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
						if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction  &&
							!levelState.mTeams )
						{
							// So we need to display rival text.. 
							// TODO: should probably check if this is the first time player has been wrecked by this person or if they are their existing rival
							m_VariableText.Format(UI::C_TextStringID("MP_RIVAL_NEW").String(), levelState.mCars[otherVehIndex].mNameAboveCar.Chars());
						}
						else
						{
							// Default
							// Single player, or Multiplayer racing == just say 'XXXX wrecked you!'. 
							m_VariableText.Format( C_TextStringID( "MP_WRECKED_BY" ).String(), levelState.mCars[otherVehIndex].mNameAboveCar.Chars());

							// Special cases 
						
							// 1) did we get last-gasped?
							if(pDamageComponent && pDamageComponent->WasLastDamageCausedByLastGasp())
							{
								m_VariableText.Format( C_TextStringID( "MP_LASTGASPED_BY" ).String(), levelState.mCars[otherVehIndex].mNameAboveCar.Chars());
							}
							
							// 2) Elimination
							if(levelState.mWreckedBehaviour == Game::E_AMAXWRECKED_ELIMINATED)
							{
								// If custom event and in elimination...we use 'Eliminated by gamertag'
								m_VariableText.Format( C_TextStringID( "MP_ELIMINATED_BY" ).String(), levelState.mCars[otherVehIndex].mNameAboveCar.Chars());
							}
						}
					}
				}
				else
				{
					if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
					{
						m_VariableText.Format( C_TextStringID( "MP_SELF_DESTRUCT" ).String(),  C_MultiplayerData::Instance().m_pConfig->m_MotormashScoringValues.m_SuicideScore);
					}
					else
					{
						m_VariableText.Format( C_TextStringID( "MP_SUICIDE" ).String() );
					}
				}
			}
		}
	}
	
	return m_VariableText;
}

const char* C_MpDataFinder::GetRaceResults( const char* pVariableName, Game::C_LevelState &levelState, Game::C_MultiplayerProfile* pMultiplayerProfile )
{
	pVariableName += 6;

	if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
	{
		pVariableName += 5;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		const S8 localIndex = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_LocalIndex;

		if ( localIndex > -1 )
			m_VariableText.Format( L"%s", levelState.mCars[localIndex].mNameAboveCar.Chars() );
	}
	else
	if ( strncmp( pVariableName, "_RANK", 5 ) == 0 )
	{
		pVariableName += 5;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		// Check to see if this is the primary player
		const bool isPlayer = ( Game::C_MpResults::Instance().m_PlayerSortedIndices[SysProfile::C_Manager::Instance().GetPrimaryPadNo()] == index );
		const S8 localIndex = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_LocalIndex;

		// Local player hasnt ranked up yet
		if ( isPlayer )
		{
			m_VariableText.Format( L"%i", pMultiplayerProfile->GetPreviousLevel() + 1 );
		}
		else
			m_VariableText.Format( L"%i", levelState.mCars[localIndex].mMultiplayerRank + 1 );
	}
	else
	if ( strncmp( pVariableName, "_STREAK", 7 ) == 0 )
	{
		pVariableName += 7;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		const S8 localIndex = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_LocalIndex;

		// only show this if its greater than 1
		if ( levelState.mCars[localIndex].mMultiplayerWinningStreak > 1 )
			m_VariableText.Format( C_TextStringID( "MP_WINNING_STREAK" ).String(), levelState.mCars[localIndex].mMultiplayerWinningStreak );
	}
	else
	if ( strncmp( pVariableName, "_POSITION", 9 ) == 0 )
	{
		pVariableName += 9;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index < levelState.mNumCars )
		{
			const U8 position = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Position;
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToPosition( position ) );
		}
		else
			m_VariableText.Format( L"--" );
	}
	else
	if ( strncmp( pVariableName, "_POS_WHOLE", 10 ) == 0 )
	{
		pVariableName += 10;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index < levelState.mNumCars )
		{
			const U8 position = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Position;
			m_VariableText.Format( C_TextStringID( "MP_PLACE_RESULTS" ).String(), position, C_UILocaliser::Instance().ConvertNumberToPosition( position, false, NULL, NULL, false, true ) );
		}
	}
	else
	if ( strncmp( pVariableName, "_POS_NUMBER", 11 ) == 0 )
	{
		pVariableName += 11;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index < levelState.mNumCars )
		{
			const U8 position = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Position;
			m_VariableText.Format( L"%d", position );
		}
	}
	else
	if ( strncmp( pVariableName, "_POS_SUFFIX", 11 ) == 0 )
	{
		pVariableName += 11;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index < levelState.mNumCars )
		{
			const U8 position = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Position;
			m_VariableText.Format( L"%s",  C_UILocaliser::Instance().ConvertNumberToPosition( position, false, NULL, NULL, false, true ) );
		}
	}
	else
	if ( strncmp( pVariableName, "_SCORE", 6 ) == 0 )
	{
		pVariableName += 6;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index < levelState.mNumCars )
		{
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

			switch( levelState.mRaceMode )
			{
				case Game::E_RaceMode_Multiplayer_Racing :
				{
					S32 score = Game::GetDriverRacePoints(Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Position, levelState.mNumCars);
					m_VariableText.Format( C_TextStringID( ( score == 1 ) ? "MP_RESULT_PT" : "MP_RESULT_PTS" ).String(), score );
					break;
				}
				case Game::E_RaceMode_Multiplayer_Elimination :
				case Game::E_RaceMode_Multiplayer_Destruction :
				{
					S32 score = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
					m_VariableText.Format( C_TextStringID( ( score == 1 ) ? "MP_RESULT_PT" : "MP_RESULT_PTS" ).String(), score );
					break;
				}
			}
		}
	}
	else
	if ( strncmp( pVariableName, "_RESULT", 7 ) == 0 )
	{
		pVariableName += 7;
		const U32 index = (U32)ParseIndex( pVariableName, "" );

		if ( index < levelState.mNumCars )
		{
			switch( Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_State )
			{
			case Game::E_VEHICLE_STATE_FINISHED :
				{
					Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

					switch( levelState.mRaceMode )
					{
					case Game::E_RaceMode_Multiplayer_Racing :
						{
							if(levelState.mTeams)
							{
								S32 score = Game::GetDriverRacePoints(Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Position, levelState.mNumCars);
								m_VariableText.Format(  C_TextStringID( "MP_RESULT_PTS" ).String(), score);
							}
							else
							{
								m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_RaceTime] ) );
							}
							break;
						}
					case Game::E_RaceMode_Multiplayer_Elimination :
					case Game::E_RaceMode_Multiplayer_Destruction :
						{
							S32 score = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
							m_VariableText.Format(  C_TextStringID( "MP_RESULT_PTS" ).String(), score);
							break;
						}
					case Game::E_RaceMode_Multiplayer_Points :
						{
							m_VariableText.Format( C_TextStringID( "MP_RACE_END_SCORE" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_Fans] ) );
							break;
						}
					}
					break;
				}
			case Game::E_VEHICLE_STATE_TIMED_OUT :
				{
					if(levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
					{
						S32 score = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
						m_VariableText.Format(  C_TextStringID( "MP_RESULT_PTS" ).String(), score);
					}
					else
					{
						m_VariableText.Format( C_TextStringID( "MP_TIMED_OUT" ).String());
					}
					break;
				}
			case Game::E_VEHICLE_STATE_FATALLY_DAMAGED :
				{
					if ( Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Points )
					{
						m_VariableText.Format(  C_TextStringID( "MP_RESULT_WRECKED_POINTS" ).String(), Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_Fans] );
					}
					else if(Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
					{
						S32 score = Game::C_MpResults::Instance().m_SortedVehicleResults[index].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
						m_VariableText.Format(  C_TextStringID( "MP_RESULT_PTS" ).String(), score);
						break;
					}
					else
					{
						m_VariableText.Format( L"%s", C_TextStringID( "MP_WRECKED" ).String() );
					}
					break;
				}
			case Game::E_VEHICLE_STATE_LEFT_RACE :
				{
					m_VariableText.Format( L"%s", C_TextStringID( "MP_DISCONNECTED" ).String() );
					break;
				}
			case Game::E_VEHICLE_STATE_ELIMINATED :
				{
					m_VariableText.Format( L"%s", C_TextStringID( "MP_ELIMINATED" ).String() );
					break;
				}
			default :
				{
					m_VariableText.Format( L"%s", C_TextStringID( "MP_UNKNOWN" ).String() );
					break;
				}
			}
		}
		else
			m_VariableText.Format( L"--" );
	}

	return pVariableName;
}

const char* C_MpDataFinder::GetChampionshipStandings( const char* pVariableName, Game::C_LevelState &levelState, Game::C_MultiplayerProfile* pMultiplayerProfile )
{
	pVariableName += 22;

	Game::C_GameModeBase* pBaseMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	BIZ_ASSERT(pBaseMode);
	if (!pBaseMode->IsNetwork())
		return pVariableName;

	Game::C_GameModeNetwork* pGameMode = static_cast<Game::C_GameModeNetwork*>(pBaseMode);

	if ( strncmp( pVariableName, "_NAME", 5 ) == 0 )
	{
		pVariableName += 5;
		const U32 index = (U32)ParseIndex( pVariableName, "" ) - 1;

		m_VariableText.Format( pGameMode->GetSortedChampionshipStandings()[index].mPlayerName );
	}
	else
	if ( strncmp( pVariableName, "_SCORE", 6 ) == 0 )
	{
		pVariableName += 6;
		const U32 index = (U32)ParseIndex( pVariableName, "" ) - 1;

		m_VariableText.Format( L"%d", pGameMode->GetSortedChampionshipStandings()[index].mTotalScore );
	}
	else
	if ( strncmp( pVariableName, "_WINS", 5 ) == 0 )
	{
		pVariableName += 5;
		const U32 index = (U32)ParseIndex( pVariableName, "" ) - 1;

		m_VariableText.Format( L"%d", pGameMode->GetSortedChampionshipStandings()[index].mTotalWins );
	}

	return pVariableName;
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
