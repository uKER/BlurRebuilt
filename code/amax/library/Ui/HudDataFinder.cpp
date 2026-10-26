#include "Stdafx.h"
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>	// [Blur2PC]

#include "HudDataFinder.h"
#include "UiHelpers.h"

#include <ui/Game/UILocaliser.h>
#include <lua_shared/LuaHelper.h>
#include <Ui/UiInterface.h>

#include "Game/Game.h"
#include "Game/GameWorld/GameWorld.h"

#include "Ui/SoundEffects.h"

#include "GameLogic/RaceLogicCore/Database/RLDatabase.h"
#include "GameLogic/RaceModes/RaceModeDefines.h"
#include "GameLogic/RaceRules/RaceRule_FinishInXPosition.h"
#include "GameLogic/RaceRules/RaceRule_DistanceFromDriver.h"
#include "GameLogic/RaceRules/RaceRule_TimerExpired.h"
#include "GameLogic/RaceRules/RaceRulesManager.h"
#include "GameLogic/Structures/LevelResult.h"
#include "GameLogic/Structures/LevelState.h"
#include "GameLogic/RaceModes/RaceModeMultiplayer.h"
#include "GameLogic/Structures/MpResults.h"

#include "Ui/Game/Multiplayer/MultiplayerDefines.h"

#include <GameLogic/RaceModes/RaceModeCheckpoint.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

Debug::Bool g_debugPeteTimeHack("Amax.Tool.TimeHack", false, Debug::Persist());
Debug::Bool g_debugDistanceHack("Amax.Tool.DistanceHack", false, Debug::Persist());

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// ---------------
// C_HudDataFinder
// ---------------
C_HudDataFinder C_HudDataFinder::m_sInstance;

void C_HudDataFinder::Init()
{
	m_bDistanceFromDriverFailing = false;
	m_TriggerRegionActors.Clear();
	m_sNumActiveRules = 1;
	m_bTlmSentMpStampTimer = false;
	m_bTlmSentMpShowTimer = false;

	for ( S32 i = 0; i < m_LocalPlayerPosition.Size(); i++ )
		m_LocalPlayerPosition[ i ] = -1;
}

const WCHAR* C_HudDataFinder::GetVariableText( const char* pVariableName )
{
	m_VariableText.Clear();

	if ( !strncmp( pVariableName, "SP_", 3 ) )
	{
		pVariableName += 3;
		return GetVariableTextSP( pVariableName );
	}

	if ( !strcmp( pVariableName, "BONUS_STRING" ) )
	{
		m_VariableText.Format( L"%S", m_BonusString.Chars() );
		return m_VariableText.Chars();
	}
	else if ( !strcmp( pVariableName, "SCORE_STRING" ) )
	{
		m_VariableText.Format( L"%S", m_ScoreString.Chars() );
		return m_VariableText.Chars();
	}

	// local player index
	S32 sLocalPlayerIndex = (S32)ParseIndexAdvance( pVariableName, "" );

	if ( sLocalPlayerIndex == -1 )
		return L"";

	S32 sVehicleIndex = GetPlayerVehicleIndex( sLocalPlayerIndex );
	const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( sVehicleIndex );

	if ( !pLogicActor )
		return L"";

	pVariableName++;	// passed the '_'

	Game::Actors::RaceLogicCore* pRLC = GetRLC();

	if ( !pRLC )
		return L"";

	if ( !strcmp( pVariableName, "TARGET" ) )
	{
		Game::RaceModeRuleBase* pRulePass = Game::C_RaceRulesManager::Instance().GetRaceRulePass( 0 );

		m_VariableText.Format( L"--" );

		C_WideString512 str;
		Game::C_RaceRulesManager::Instance().GetRuleStringOverride(str);

		if ( pRulePass && str.Length() == 0 )
			pRulePass->GetRuleString( (U32)sVehicleIndex, str, Game::RACE_MODE_RULE_STRING_TYPE_HUD_TARGET, true );

		m_VariableText.Format( L"%s", str.Chars() );

	}
	else if ( !strcmp( pVariableName, "POS" ) )
	{
		static C_TextStringID id( "HUD_CURRENT_POS" );
		m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition ), pRLC->GetNumVehicles() );
	}
	else if ( !strcmp( pVariableName, "POS_NUM" ) )
	{
		S32 sPosition = -1;

		if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_SplitScreen )
			sPosition = pLogicActor->GetPositionInfo().mRacePosition;
		else
		{
			// If in splitscreen mode
			// If Vehicle is not finished then get its position as normal
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			if(levelState.mCars[pLogicActor->GetVehicleIndex()].mState == Game::E_VEHICLE_STATE_FINISHED)
			{
				// But If Vehicle has finished race, then use the position that was cached for that vehicle (a vehicle should not change its HUD position once it has finished!)
				S8 vehicleCrossedFinishLinePos = Game::C_MpResults::Instance().GetCrossedFinishLinePosition(pLogicActor->GetVehicleIndex()); 

				// NOTE - vehicle may have finished by default (i.e. all other players wrecked) in which case will not have crossed finish line!
				if( vehicleCrossedFinishLinePos != -1)
					sPosition = vehicleCrossedFinishLinePos;
				else
					sPosition = pLogicActor->GetPositionInfo().mRacePosition;
			}
			else
				sPosition = pLogicActor->GetPositionInfo().mRacePosition;
		}

		if ( sPosition != -1 )
		{
			static C_TextStringID id( "HUD_CURRENT_POS_NUM" );
			m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertNumberToPosition( sPosition, false, NULL, NULL, true, false ) );
		}

		if ( sPosition != m_LocalPlayerPosition[ sLocalPlayerIndex ] )
		{
			Util::C_String32 tmp;
			tmp.Printf( "position_changed_%d", sLocalPlayerIndex );
			UI::C_UiTimelineMessage msg( tmp.Chars(), true, 0.0f );
			_UI_MESSAGE_FULL( msg );

			if ( sPosition < m_LocalPlayerPosition[ sLocalPlayerIndex ] )
				UiInterface()->PlaySound( E_UiSoundEffect_HudPositionIncrementActionIn );

			m_LocalPlayerPosition[ sLocalPlayerIndex ] = sPosition;
		}
	}
	else if ( !strcmp( pVariableName, "POS_NUM_OF" ) )
	{
		static C_TextStringID id( "HUD_CURRENT_POS_OF" );
		Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if(pLayer)
			m_VariableText.Format( id.String(), pLayer->GetNumLiveVehicles());
	}
	else if ( !strcmp( pVariableName, "POS_SUFFIX" ) )
	{
		
		if(!(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen))
		{
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition, false, NULL, NULL, false, true ) );
		}
		else
		{
			// If in splitscreen mode
			// If Vehicle is not finished then get its position as normal
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			if(levelState.mCars[pLogicActor->GetVehicleIndex()].mState == Game::E_VEHICLE_STATE_FINISHED)
			{
				// But If Vehicle has finished race, then use the position that was cached for that vehicle (a vehicle should not change its HUD position once it has finished!)
				S8 vehicleCrossedFinishLinePos = Game::C_MpResults::Instance().GetCrossedFinishLinePosition(pLogicActor->GetVehicleIndex()); 

				// Check we found the info we wanted
				if( vehicleCrossedFinishLinePos != -1)
				{
					m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( vehicleCrossedFinishLinePos, false, NULL, NULL, false, true ) );
				}
				else
				{
					m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition, false, NULL, NULL, false, true ) );
				}
			}
			else
			{
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition, false, NULL, NULL, false, true ) );
			}
		}
	}
	else if ( !strcmp( pVariableName, "LAP" ) )
	{
		if(g_debugPeteTimeHack)
		{
			F32 fTime = pLogicActor->GetLapInfo().mBestLapTime;

			if( fTime < 0 )
				fTime = 0;

			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime ) );
		}
		else if(g_debugDistanceHack)
		{
			F32 fDistance = pLogicActor->GetPositionInfo().mDistanceFromStart;
			m_VariableText.Format( L"%0.2fm", fDistance );
		}
		else
		{
			static C_TextStringID id( "HUD_CURRENT_LAP" );
			static C_TextStringID idInf( "HUD_CURRENT_LAP_INF" );

			U8 lap = pLogicActor->GetLapInfo().mCurrentLap;
			const U8 laps = GetLevelState().mLaps;

			if ( lap > laps )
				lap = laps;

			if ( laps == 99 ) // [PK] hack, need to sort out a proper system for infinite laps
				m_VariableText.Format( idInf.String() );
			else
				m_VariableText.Format( id.String(), lap, laps );
		}
	}
	else if ( !strcmp( pVariableName, "LAP_TIME" ) )
	{
		Game::C_LevelState& levelState = GetLevelState();

		if ( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Elimination && levelState.GetMultiplayerSettings().mEliminationEvent == UI::E_EliminationEvent_Time )
		{
			F32 fTime = ((Game::C_RaceModeMultiplayer_Elimination*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr())->GetRaceTimeout();

			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime ) );

			return m_VariableText.Chars();
		}

		F32 fTimeLimit = (F32)GetLevelState().mTimeLimit;
		F32 fTime = pLogicActor->GetLapInfo().mCurrentLapTime;

		if ( fTimeLimit > 0 )
		{
			const float rt = pRLC->GetRaceTime();
			fTime = fTimeLimit - rt;

			// #18829 - Reinforce timer every 'n' seconds
			float mod = 30.0f;
			if( fTime <= 10.0f )
				mod = 1.0;
			else
			if( fTime < 30.0f )
				mod = 10.0;

			float tmp = (float)fmodf(rt, mod );
			float ft = pRLC->GetFrameTime();
			if( mod - tmp <= ft )
			{
				UI::C_UiTimelineMessage tlm( "reinforce_time", true, 0.0f, true );
				_UI_MESSAGE_FULL( tlm );

				if( fTime <= 10.0f )
					UiInterface()->PlaySound( E_UiSoundEffect_RaceCountdownActionIn );
			}

			// #18829 - Turn timer red when it goes under 10 seconds
			if( fTime < 10.0f && fTime + ft >= 10.0f )
				Lua::Call( UI::g_pUILua->GetState(), "HUD_ChangeTimerColour", "s", "Support_3" );
		}

		// distance from driver countdown timer
		F32 fDistanceFromDriverCountdown = 0.0f;
		
		for ( S32 i = 0; i < Game::C_RaceRulesManager::Instance().GetNumFailRules(); i++ )
		{
			Game::RaceModeRuleBase* pRuleFail = Game::C_RaceRulesManager::Instance().GetRaceRuleFail( i );

			if ( !pRuleFail )
				continue;
			
			if ( pRuleFail->GetType() == Game::E_RaceRule_DistanceFromDriver )
			{
				Game::RaceModeRule_DriverDistanceFromDriver* pRule = (Game::RaceModeRule_DriverDistanceFromDriver*)pRuleFail;

				if ( pRule->IsFailing( sVehicleIndex ) && ( ( fDistanceFromDriverCountdown == 0.0f ) 
					|| ( pRule->GetCountDownTimer( sVehicleIndex ) < fDistanceFromDriverCountdown ) ) )
				{
					fDistanceFromDriverCountdown = pRule->GetCountDownTimer( sVehicleIndex );
				}
			}
		}

		// special case for timer expired events
		for ( S32 i = 0; i < Game::C_RaceRulesManager::Instance().GetNumFailRules(); i++ )
		{
			Game::RaceModeRuleBase* pRuleFail = Game::C_RaceRulesManager::Instance().GetRaceRuleFail( i );

			if ( !pRuleFail )
				continue;
			
			if ( pRuleFail->GetType() == Game::E_RaceRule_TimerExpired )
			{
				Game::RaceModeRule_TimerExpired* pRule = (Game::RaceModeRule_TimerExpired*)pRuleFail;
				fTime = pRule->GetTimer( sVehicleIndex );

				break;
			}
		}

		if ( fDistanceFromDriverCountdown != 0.0f )
		{
			m_bDistanceFromDriverFailing = true;
			fTime = fDistanceFromDriverCountdown;
		}
		else
			m_bDistanceFromDriverFailing = false;

		if ( fTime < 0 )
			fTime = 0;

		m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime ) );
	}
	else if ( !strcmp( pVariableName, "TIME_EXPIRED_BONUS" ) )
	{
		for ( S32 i = 0; i < Game::C_RaceRulesManager::Instance().GetNumFailRules(); i++ )
		{
			Game::RaceModeRuleBase* pRuleFail = Game::C_RaceRulesManager::Instance().GetRaceRuleFail( i );

			if ( !pRuleFail )
				continue;
			
			if ( pRuleFail->GetType() == Game::E_RaceRule_TimerExpired )
				pRuleFail->GetRuleString( (U32)sVehicleIndex, m_VariableText, Game::RACE_MODE_RULE_STRING_TYPE_HUD_BONUS, true );
		}
	}
	else if ( !strcmp( pVariableName, "SPEED_MPH" ) )
	{
		static C_TextStringID id( "HUD_SPEED_MPH" );

		U32 speed = (U32)Maths::Abs<F32>( pLogicActor->GetPositionInfo().mSpeedInMph );
		m_VariableText.Format( id.String(), speed );
	}
	else if ( !strcmp( pVariableName, "MP_PLACE" ) )
	{
		m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition, false, NULL, NULL, true, true ) );
	}
	else if ( !strcmp( pVariableName, "MP_TIMEOUT" ) )
	{
		F32 fTime = 0.0f;

		Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

		if ( pRaceMode )
			fTime = pRaceMode->GetRaceTimeout();

		if ( fTime < 0.0f )
			fTime = 0.0f;

		m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime ) );
	}
	else
	if ( strcmp( pVariableName, "MP_ROLLING_START" ) == 0 )
	{
		//S32 distance = (S32)AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1) * ( 1 - pLogicActor->GetPositionInfo().mLogicPosition  );
		//m_VariableText.Format( L"%sRolling Start %im", distance );
	}
	else
	if ( !strcmp( pVariableName, "FANS_STRING" ) )
	{
		static C_TextStringID id( "HUD_FANS_STRING" );
		m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertNumberToCommas( pLogicActor->GetScoring().mFinalScoreForUI ) );
	}
	else
	if ( !strcmp( pVariableName, "FANS" ) )
	{
		const VehicleScoringInfo& scoring = pLogicActor->GetScoring();
		if(scoring.mAllFansGained)
			m_VariableText.Format( L"MAX" );
		else
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( scoring.mFinalScoreForUI ) );
	}
	else
	if ( !strcmp( pVariableName, "COMBO" ) )
	{
		const VehicleScoringInfo& scoring = pLogicActor->GetScoring();

		if ( scoring.mCombo <= 0 )
			return L"";

		m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( scoring.mCombo ) );
	}
	else
	if ( !strncmp( pVariableName, "MP_", 3 ) )
	{
		pVariableName += 3;

		if ( !strcmp( pVariableName, "TARGET" ) )
		{
			Game::C_LevelState& levelState = GetLevelState();

			if ( !levelState.mNetworkRace || ( levelState.mRaceMode != Game::E_RaceMode_Multiplayer_Destruction ) )
				return L"";

			static C_TextStringID id( "MP_HUD_DESTRUCTION_TARGET" );
			m_VariableText.Format( id.String(), levelState.GetMultiplayerSettings().mScoreLimit );
		}
		else
		if ( !strcmp( pVariableName, "CURRENT_POINTS" ) )
		{
			S32 sPoints = Game::C_MpResults::Instance().m_LiveVehicleData[ sVehicleIndex ].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( sPoints ) );
		}
		else
		if ( !strncmp( pVariableName, "RANK", 4 ) )
		{
			pVariableName += 4;

			S32 sRank = (S32)ParseIndexAdvance( pVariableName, "_" );

			m_VariableText.Format( C_TextStringID( "MP_RANK_AND_TITLE" ).String(), sRank );
		}
		else
		if ( !strncmp( pVariableName, "NEXT_RANK_IN", 12 ) )
		{
			pVariableName += 12;

			S32 sFans = (S32)ParseIndexAdvance( pVariableName, "_" );
			S32 sRank = (S32)ParseIndexAdvance( pVariableName, "_" );

			m_VariableText.Format( C_TextStringID( "MP_FANS_NEEDED" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( sFans ), sRank );
		}
		else
		if( !strncmp( pVariableName, "TEAM_POINTS", 11))
		{
			pVariableName += 11;

			U8 playerTeam = Game::C_MpResults::Instance().m_LiveVehicleData[ sVehicleIndex ].m_Team;
			S32 sPoints = -99;

			if( !strncmp( pVariableName, "_CURRENT", 8))
			{
			}
			else
			if( !strncmp( pVariableName, "_OPPOSITION", 13))
			{
				playerTeam ^= 1;				
			}

			sPoints = Game::C_MpResults::Instance().m_Teams[playerTeam].m_TeamScore;
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( sPoints ) );
		}
	}
	else
	{
		// not found
		m_VariableText.Format( L"<< NOT FOUND - %S >>", pVariableName );
	}

	return m_VariableText.Chars();
}

const WCHAR* C_HudDataFinder::GetVariableTextSP( const char* pVariableName )
{
	Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();

	if ( !pRaceBookLayer )
		return L"";

	if ( !strncmp( pVariableName, "RACE_STREET_", 12 ) )
	{
		pVariableName += 12;

		// street races
		if ( !pRaceBookLayer->GetRaceModePtr() || (( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_StreetRace ) && ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_StreetRace )) )
			return L"";

		Game::C_RaceModeStreetRace* pRace = (Game::C_RaceModeStreetRace*)pRaceBookLayer->GetRaceModePtr();

		if ( !strcmp( pVariableName, "DIFF_TO_MEDAL" ) )
		{
			const Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( 0 );

			if ( !pLogicActor )
				return L"";

			S32 sPosition = pLogicActor->GetPositionInfo().mRacePosition;
			S32 sPositionNext = -1;

			if ( sPosition > 3 )
				sPositionNext = 3;
			else
				sPositionNext = sPosition - 1;

			if ( sPositionNext > 0 )
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( sPositionNext ) );
			else
			{
				S32 sBestFans = SP_GetEventBestFans( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );
				S32 sCurrentFans = pLogicActor->GetScoring().mFinalScoreForUI;
				S32 sFans = sBestFans;

				if ( sCurrentFans > sBestFans )
					sFans = sCurrentFans;

				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( sFans ) );
			}
		}
	}
	else
	if ( !strncmp( pVariableName, "RACE_CHECKPOINT_", 16 ) )
	{
		pVariableName += 16;

		// checkpoint races
		if ( !pRaceBookLayer->GetRaceModePtr() || (( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_Checkpoint ) && ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_Checkpoint )) )
			return L"";

		Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)pRaceBookLayer->GetRaceModePtr();

		if ( !strcmp( pVariableName, "NUM_PASSED_OF_TOTAL" ) )
		{
			static C_TextStringID id( "HUD_CHECKPOINT_NUM_PASSED_OF_TOTAL" );
			m_VariableText.Format( id.String(), pRace->GetCheckpointsPassed(), pRace->GetNumCheckpoints() );
		}
		else
		if ( !strcmp( pVariableName, "NUM_PASSED" ) )
		{
			m_VariableText.Format( L"%d", pRace->GetCheckpointsPassed() );
		}
		else
		if ( !strcmp( pVariableName, "NUM_OF_TOTAL" ) )
		{
			static C_TextStringID id( "HUD_CHECKPOINT_OF_NUM" );
			m_VariableText.Format( id.String(), pRace->GetNumCheckpoints() );
		}
		else
		if ( !strcmp( pVariableName, "TIME_LEFT" ) )
		{
			F32 fTime = Maths::Max( pRace->GetTimeLeft(), 0.0f );
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime, true, false, false ) );
		}
		else
		if ( !strcmp( pVariableName, "TIME_DIFF_TO_MEDAL" ) )
		{
			if ( pRace->GetEventState() == kSP_Gold )
			{
				F32 fBestTime = SP_GetEventBestTime( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );
				F32 fTimeLeft = pRace->GetTimeLeft();
				F32 fTime = fBestTime;

				if ( fTimeLeft > fBestTime )
					fTime = fTimeLeft;

				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime, true, false, false ) );
			}
			else
			{
				static C_TextStringID id( "HUD_TIME_SEC" );
				m_VariableText.Format( id.String(), (S32)pRace->GetTimeDiffToMedal() );
			}
		}
		else
		if ( !strcmp( pVariableName, "TIME_ADDED_CHECKPOINT" ) )
		{
			static C_TextStringID id( "HUD_TIME_PLUS_SEC" );
			m_VariableText.Format( id.String(), (S32)pRace->GetTimeAddedCheckpoint() );
		}
		else
		if ( !strcmp( pVariableName, "TIME_ADDED_STOPWATCH" ) )
		{
			static C_TextStringID id( "HUD_TIME_PLUS_SEC" );
			m_VariableText.Format( id.String(), (S32)pRace->GetTimeAddedStopwatch() );
		}
		else
		if ( !strcmp( pVariableName, "LAST_TIME" ) )
		{
			static C_TextStringID id( "HUD_TIME_PLUS_SEC" );
			m_VariableText.Format( id.String(), (S32)pRace->GetLastTime() );
		}
	}
	else
	if ( !strncmp( pVariableName, "RACE_DESTRUCTION_", 17 ) )
	{
		pVariableName += 17;

		// destruction races
		if ( !pRaceBookLayer->GetRaceModePtr() || (( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_Destruction ) && ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_Destruction )) )
			return L"";

		Game::C_RaceModeDestruction* pRace = (Game::C_RaceModeDestruction*)pRaceBookLayer->GetRaceModePtr();

		if ( !strcmp( pVariableName, "NUM_POINTS" ) )
		{
			m_VariableText.Format( L"%d", pRace->GetNumPoints() );
		}
		else
		if ( !strcmp( pVariableName, "TIME_LEFT" ) )
		{
			F32 fTime = Maths::Max( pRace->GetTimeLeft(), 0.0f );
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime, true, false, false ) );
		}
		else
		if ( !strcmp( pVariableName, "DIFF_TO_MEDAL" ) )
		{
			if ( pRace->GetEventState() == kSP_Gold )
			{
				S32 sBestPoints = SP_GetEventBestPoints( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );
				S32 sCurrentPoints = pRace->GetNumPoints();
				S32 sPoints = sBestPoints;

				if ( sCurrentPoints > sBestPoints )
					sPoints = sCurrentPoints;

				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( sPoints ) );
			}
			else
			{
				S32 sDiffToMedal = pRace->GetDiffToMedal();
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( sDiffToMedal ) );
			}
		}
		else
		if ( !strcmp( pVariableName, "LAST_TIME" ) )
		{
			static C_TextStringID id( "HUD_TIME_PLUS_SEC" );

			S32 sLastTime = (S32)pRace->GetLastTime();

			if ( sLastTime > 0 )
				m_VariableText.Format( id.String(), sLastTime );
		}
		else
		if ( !strcmp( pVariableName, "LAST_POINTS_NUM" ) )
		{
			static C_TextStringID id( "HUD_POINTS_PLUS" );
			static C_TextStringID idOne( "HUD_POINT_PLUS" );

			S32 sLastPoints = pRace->GetLastPoints();

			if ( sLastPoints == 0 )
				return L"";

			m_VariableText.Format( ( sLastPoints == 1 ) ? idOne.String() : id.String(), sLastPoints );
		}
		else
		if ( !strcmp( pVariableName, "LAST_POINTS_STRING" ) )
		{
			m_VariableText.Format( L"%s", pRace->GetLastPointsString().Chars() );
		}
		else
		if ( !strcmp( pVariableName, "LAST_POINTS_NAME" ) )
		{
			m_VariableText.Format( L"%s", pRace->GetLastPointsName().Chars() );
		}
		else
		if ( !strcmp( pVariableName, "LAST_BONUS_POINTS_NUM" ) )
		{
			static C_TextStringID id( "HUD_POINTS_PLUS" );
			static C_TextStringID idOne( "HUD_POINT_PLUS" );

			S32 sLastPoints = pRace->GetLastBonusPoints();

			if ( sLastPoints == 0 )
				return L"";

			m_VariableText.Format( ( sLastPoints == 1 ) ? idOne.String() : id.String(), sLastPoints );
		}
		else
		if ( !strcmp( pVariableName, "LAST_BONUS_POINTS_STRING" ) )
		{
			m_VariableText.Format( L"%s", pRace->GetLastBonusPointsString().Chars() );
		}
		else
		if ( !strcmp( pVariableName, "LAST_BONUS_POINTS_NAME" ) )
		{
			m_VariableText.Format( L"%s", pRace->GetLastBonusPointsName().Chars() );
		}
	}
	else
	if ( !strncmp( pVariableName, "RACE_FAN_DEMAND_", 16 ) )
	{
		pVariableName += 16;

		// fan demand races
		if ( !pRaceBookLayer->GetRaceModePtr() || (( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FanDemand ) && ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_FanDemand )) )
			return L"";

		Game::C_RaceModeFanDemand* pRace = (Game::C_RaceModeFanDemand*)pRaceBookLayer->GetRaceModePtr();

		if ( !strcmp( pVariableName, "NUM_FAN_DEMANDS_COMPLETED" ) )
		{
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( pRace->GetNumDemandsComplete() ) );
		}
		else
		if ( !strcmp( pVariableName, "NUM_FAN_DEMANDS_TOTAL" ) )
		{
			static C_TextStringID id( "HUD_FAN_DEMAND_OF_NUM" );
			
			Game::C_RaceLogicLayer* pRaceLogicLayer = GetRaceLogicLayer();

			if ( pRaceLogicLayer )
				m_VariableText.Format( id.String(), pRaceLogicLayer->mNumFanPickups - 1 );
		}
		else
		if ( !strcmp( pVariableName, "TIME_LEFT" ) )
		{
			F32 fTime = Maths::Max( pRace->GetTimeLeft(), 0.0f );
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime, true, false, false ) );
		}
		else
		if ( !strcmp( pVariableName, "DIFF_TO_MEDAL" ) )
		{
			if ( pRace->GetEventState() == kSP_Gold )
			{
				S32 sBestFans = SP_GetEventBestFans( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );
				S32 sCurrentFans = pRace->GetNumFans();
				S32 sFans = sBestFans;

				if ( sCurrentFans > sBestFans )
					sFans = sCurrentFans;

				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( sFans ) );
			}
			else
			{
				S32 sDiffToMedal = pRace->GetDiffToMedal();
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( sDiffToMedal ) );
			}
		}
	}
	else
	if ( !strncmp( pVariableName, "RACE_BOSS_", 10 ) )
	{
		pVariableName += 10;

		// boss races
		if ( !pRaceBookLayer->GetRaceModePtr() || (( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_BossBattle ) && ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_BossBattle )) )
			return L"";

		Game::C_RaceModeBossBattle* pRace = (Game::C_RaceModeBossBattle*)pRaceBookLayer->GetRaceModePtr();
	}
	else
	if ( !strncmp( pVariableName, "FD_RACE_", 8 ) )
	{
		pVariableName += 8;

		// friend demand street races
		if ( !pRaceBookLayer->GetRaceModePtr() || 
				(( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_StreetRace ) &&
					( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_BossBattle ) &&
						( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_Destruction ) &&
							( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_Checkpoint ) &&
								( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_FanDemand ) ) )
			return L"";

		const Game::FriendChallenge* pFC = Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand();

		if ( !pFC )
			return L"";

		if ( !strncmp( pVariableName, "MEDAL_TARGET", 12 ) )
		{
			pVariableName += 12;
			S32 sMedal = ParseIndex( pVariableName, "_" );

			if ( sMedal < 1 || sMedal > 3 )
				return L"";

			if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_StreetRace )
			{
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( sMedal ) );
			}
			else
			if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_Destruction )
			{
				Game::C_RaceModeDestruction* pRace = (Game::C_RaceModeDestruction*)pRaceBookLayer->GetRaceModePtr();
				AmaxRaceModeDestructionVars* pVars = pRace->GetDifficultyVars();

				if ( !pVars )
					return L"";

					 if ( sMedal == 1 )		m_VariableText.Format( L"%d", pVars->goldPoints );
				else if ( sMedal == 2 )		m_VariableText.Format( L"%d", pVars->silverPoints );
				else if ( sMedal == 3 )		m_VariableText.Format( L"%d", pVars->bronzePoints );
			}
			else
			if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_Checkpoint )
			{
				Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)pRaceBookLayer->GetRaceModePtr();
				AmaxRaceModeCheckpointVars* pVars = pRace->GetDifficultyVars();

				if ( !pVars )
					return L"";

				static C_TextStringID id( "HUD_TIME_SEC" );

					 if ( sMedal == 1 )		m_VariableText.Format( id.String(), (S32)pVars->goldTimeRemaining );
				else if ( sMedal == 2 )		m_VariableText.Format( id.String(), (S32)pVars->silverTimeRemaining );
				else if ( sMedal == 3 )		m_VariableText.Format( id.String(), (S32)0.0f );
			}
			else
			if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_BossBattle )
			{
				// only target in boss race is to win
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( 1 ) );
			}
			else
			{
				// fd race mode not handled
				return L"";
			}
		}
		else
		if ( !strcmp( pVariableName, "CRITERIA" ) )
		{
			static C_TextStringID  id( "HUD_FD_CRITERIA" );

			if(pFC->mType == Game::FRIEND_CHALLENGE_TYPE_RACE || pFC->mType == Game::FRIEND_CHALLENGE_TYPE_FAN_DEMAND || Game::FRIEND_CHALLENGE_TYPE_DESTRUCTION)
				m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertNumberToCommas( pFC->mU32Criteria ) );

			if(pFC->mType == Game::FRIEND_CHALLENGE_TYPE_CHECKPOINT)
				m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( pFC->mFloatCriteria ) );

			if(pFC->mType == Game::FRIEND_CHALLENGE_TYPE_BOSS)
			{
				static C_TextStringID  id( "HUD_FD_PARAM" );

				//always assuming 0 as its a friend challenge
				const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( 0 );
				if(pLogicActor)
				{
					UI::C_WideString32 str;
					str.Format(UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(pLogicActor->GetStatistics().mTotalRaceTime));
					m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( pFC->mFloatCriteria ), str.Chars() );
				}
			}
		}
		else
		if ( !strcmp( pVariableName, "PARAM" ) )
		{
			static C_TextStringID  id( "HUD_FD_PARAM" );

			UI::C_WideString64 param;
			Game::C_FriendChallenges::Instance().AttemptingParamResult( &param );

			m_VariableText.Format( id.String(),
				Game::C_FriendChallenges::Instance().GetParamVariableString( 0, Game::C_FriendChallenges::Instance().GetAttemptingFriendDemandIndex(), NULL ),
				param.Chars()
			);
		}
	}
	else
	if ( !strncmp( pVariableName, "FD_", 3 ) )		//friend demand vars
	{
		pVariableName += 3;

		if ( !strncmp( pVariableName, "CRITERIA_RESULT_", 16 ) )
		{
			pVariableName += 16;

			if ( !strcmp( pVariableName, "TITLE" ) )
				Game::C_FriendChallenges::Instance().GetCriteriaResults( &m_VariableText );
			else
			if ( !strcmp( pVariableName, "YOU" ) )
				Game::C_FriendChallenges::Instance().GetCriteriaResults( NULL, &m_VariableText );
			else
			if ( !strcmp( pVariableName, "THEM" ) )
				Game::C_FriendChallenges::Instance().GetCriteriaResults( NULL, NULL, &m_VariableText );
		}
		else
		if ( !strncmp( pVariableName, "PARAM_RESULT_", 13 ) )
		{
			pVariableName += 13;

			if ( !strcmp( pVariableName, "TITLE" ) )
				Game::C_FriendChallenges::Instance().GetParamResults( &m_VariableText );
			else
			if ( !strcmp( pVariableName, "YOU" ) )
				Game::C_FriendChallenges::Instance().GetParamResults( NULL, &m_VariableText );
			else
			if ( !strcmp( pVariableName, "THEM" ) )
				Game::C_FriendChallenges::Instance().GetParamResults( NULL, NULL, &m_VariableText );
		}
	}
	else
	if ( !strcmp( pVariableName, "RACE_RESULT" ) )
	{
		if ( !pRaceBookLayer->GetRaceModePtr() )
			return L"";

		Game::C_LevelResult& levelResult = Game::C_Game::GetInstance()->GetLevelResult();

		if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_StreetRace || pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_StreetRace )
		{
			static C_TextStringID id( "SP_RESULT_RACE" );
			m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertNumberToPosition( levelResult.mCars[ 0 ].mPosition ) );
		}
		else
		if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_Checkpoint || pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_Checkpoint )
		{
			static C_TextStringID id( "SP_RESULT_CHECKPOINT" );

			Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)pRaceBookLayer->GetRaceModePtr();

			F32 fTime = Maths::Max( pRace->GetTimeLeft(), 0.0f );
			m_VariableText.Format( id.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( fTime, true, false, false ) );
		}
		else
		if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_Destruction || pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_Destruction )
		{
			static C_TextStringID id( "SP_RESULT_DESTRUCTION" );

			Game::C_RaceModeDestruction* pRace = (Game::C_RaceModeDestruction*)pRaceBookLayer->GetRaceModePtr();

			m_VariableText.Format( id.String(), pRace->GetNumPoints() );
		}
		else
		if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FanDemand || pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_FanDemand )
		{
			static C_TextStringID id( "SP_RESULT_FAN_DEMAND" );

			Game::C_RaceModeFanDemand* pRace = (Game::C_RaceModeFanDemand*)pRaceBookLayer->GetRaceModePtr();

			m_VariableText.Format( id.String(), pRace->GetNumDemandsComplete() );
		}
		else
		if ( pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_BossBattle || pRaceBookLayer->GetRaceMode() == Game::E_RaceMode_FriendDemand_BossBattle )
		{
			static C_TextStringID idWin( "SP_RESULT_BOSS_WIN" );
			static C_TextStringID idWinWreck( "SP_RESULT_BOSS_WIN_WRECK" );
			static C_TextStringID idLose( "SP_RESULT_BOSS_LOSE" );
			
			SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), GetLevelState().mSpEventId );

			if ( !pEvent || !pEvent->tier || !pEvent->tier->boss )
				return L"";

			Game::C_RaceModeBossBattle* pRace = (Game::C_RaceModeBossBattle*)pRaceBookLayer->GetRaceModePtr();

			m_VariableText.Format( pRace->HasWonByWreck() ? idWinWreck.String() : ( pRace->HasWon() ? idWin.String() : idLose.String() ), C_TextStringID( pEvent->tier->boss->name.id ).String() );
		}
	}
	else
	{
		// not found
		m_VariableText.Format( L"<< SP_ - NOT FOUND - %S >>", pVariableName );
	}

	return m_VariableText.Chars();
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
