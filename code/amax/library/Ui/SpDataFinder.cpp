#include "Stdafx.h"
#include "SpDataFinder.h"
#include "UiHelpers.h"

#include <ui/Game/UILocaliser.h>

#include "GameLogic/SinglePlayer/SinglePlayer.h"

#include <Gamelogic\SocialNetworking\EventOwnerManager.h>
#include <GameLogic\SocialNetworking\FriendsManager.h>

#include <ui\Game\Profiles\ProfileInterface.h>

namespace UI
{

C_SpDataFinder C_SpDataFinder::m_sInstance;

void C_SpDataFinder::Init()
{
	//
}

const WCHAR* C_SpDataFinder::GetVariableText( const char* pVariableName ) 
{
	m_VariableText.Clear();

	SP_SaveGame* sg = SP_GetCurrentSaveGame();
	if( !sg )
		return L"<<Savegame error>>";

	if ( !strncmp( pVariableName, "SG_", 3 ) )
	{
		// Savegame
		pVariableName += 3;

		if( !strcmp( pVariableName, "RANK") )
		{
			m_VariableText.Format( L"%d", SP_GetRank( sg ) );
		}
		else
		if( !strcmp( pVariableName, "FANS") )
		{
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( SP_GetFans( sg ) ) );
		}
		else
		if( !strcmp( pVariableName, "STARS") )
		{
			m_VariableText.Format( L"%d", SP_CountTotalStars( sg ) );
		}
		else
		if( !strcmp( pVariableName, "MAX_STARS") )
		{
			m_VariableText.Format( L"%d", SP_CountMaxStars( sg ) );
		}
		if( !strcmp( pVariableName, "STARS_AND_MAX") )
		{
			m_VariableText.Format( L"%d/%d", SP_CountTotalStars( sg ), SP_CountMaxStars( sg ) );
		}
	}
	else
	if ( strncmp( pVariableName, "UNLOCK_STATUS", 13 ) == 0 )
	{
		pVariableName += 13;

		m_VariableText.Format( C_TextStringID( "SP_UNLOCK_STATUS" ).String(), (S32)ParseIndex( pVariableName, "" ) );
	}
	else
	if ( strncmp( pVariableName, "UNLOCK_FAN_STATUS", 17 ) == 0 )
	{
		pVariableName += 17;

		m_VariableText.Format( C_TextStringID( "SP_UNLOCK_FAN_STATUS" ).String(), (S32)ParseIndex( pVariableName, "" ) );
	}
	else
	if ( strncmp( pVariableName, "FANS_NEEDED", 11 ) == 0 )
	{
		pVariableName += 11;
		const S32 fans = (S32)ParseIndexAdvance( pVariableName, "" );
		const S32 rank = (S32)ParseIndex( pVariableName, "_" );

		m_VariableText.Format( C_TextStringID( "SP_FANS_NEEDED" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( fans ), rank );
	}
	else
	if ( !strncmp( pVariableName, "TIER_", 5 ) )
	{
		// Event
		pVariableName += 5;

		XtId tierId = (S32)ParseIndexAdvance( pVariableName, "" );

		SP_Tier* pTier = SP_GetTier( SP_GetCurrentSaveGame(), tierId );
		if( pTier )
		{
			if ( !strcmp( pVariableName, "STARSANDMAX" ) )
			{
				m_VariableText.Format( L"%d/%d", SP_CountTotalTierStars( sg, pTier ), SP_CountMaxTierStars( sg, pTier ) );
			}
			else
			if ( !strncmp( pVariableName, "BD", 2 ) )
			{
				// Boss demand
				pVariableName += 2;

				SP_Boss* pBoss = pTier->boss;

				S32 index = (S32)ParseIndexAdvance( pVariableName, "" );
				
				if( pBoss )
				{
					SP_BossDemand* pDemand = SP_GetBossDemand( sg, pBoss, index );
					if( pDemand )
					{
						if ( !strcmp( pVariableName, "_TITLE" ) )
						{
							const char* locIdName = NULL;

							// Zero
							switch( pDemand->kind )
							{
								case kSP_BossDemand_Tier_Win_All_Events:			locIdName = "SP_BDKIND_WIN_ALL_EVENTS"; 			break;
								case kSP_BossDemand_Event_Finish_Without_SorR:		locIdName = "SP_BDKIND_FINISH_WITHOUT_SORR"; 		break;
							}
							if( locIdName )
							{
								m_VariableText.Format( C_TextStringID( locIdName ).String() );
								return m_VariableText;
							}

							// One
							switch( pDemand->kind )
							{
								case kSP_BossDemand_Tier_Use_Powerups:				locIdName = "SP_BDKIND_USE_POWERUPS";				break;
//								case kSP_BossDemand_Tier_Earn_Fans:					locIdName = "SP_BDKIND_EARN_FANS";					break;
								case kSP_BossDemand_Tier_Finish_In_Different_Car:	locIdName = "SP_BDKIND_FINISH_IN_DIFFERENT_CAR";	break;
								case kSP_BossDemand_Tier_Finish_Events:				locIdName = "SP_BDKIND_COMPLETE_EVENTS";			break;
								case kSP_BossDemand_Tier_Shield_Powerups:			locIdName = "SP_BDKIND_SHIELD_POWERUPS";			break;
								case kSP_BossDemand_Tier_Shock_Opponents:			locIdName = "SP_BDKIND_SHOCK_OPPONENTS"; 			break;
								case kSP_BossDemand_Tier_Repair_Car:				locIdName = "SP_BDKIND_REPAIR_CAR"; 				break;
								case kSP_BossDemand_Tier_Seconds_In_First:			locIdName = "SP_BDKIND_SECONDS_IN_FIRST";			break;
//								case kSP_BossDemand_Event_Time_Remaining:			locIdName = "SP_BDKIND_TIME_REMAINING";				break;
								case kSP_BossDemand_Tier_Collect_Stopwatches:		locIdName = "SP_BDKIND_COLLECT_STOPWATCHES";		break;
								case kSP_BossDemand_Tier_Complete_FanDemands:		locIdName = "SP_BDKIND_COMPLETE_FANDEMANDS";		break;
								case kSP_BossDemand_Tier_Complete_FanRuns:			locIdName = "SP_BDKIND_COMPLETE_FANRUNS"; 			break;
								case kSP_BossDemand_Tier_Complete_FanPars:			locIdName = "SP_BDKIND_COMPLETE_FANPARS"; 			break;
//								case kSP_BossDemand_Event_Get_5X_Combo:				locIdName = "SP_BDKIND_GET_5X_COMBO"; 				break;
								case kSP_BossDemand_Tier_Ram_Into_Sea:				locIdName = "SP_BDKIND_RAM_INTO_SEA";				break;
								case kSP_BossDemand_Tier_Wreck:						locIdName = "SP_BDKIND_WRECK"; 						break;
								case kSP_BossDemand_Tier_Barge_Hits:				locIdName = "SP_BDKIND_BARGE_HITS";					break;
//								case kSP_BossDemand_Event_Get_All_Lights:			locIdName = "SP_BDKIND_GET_ALL_LIGHTS"; 			break;
								case kSP_BossDemand_Tier_Barge_Off_Cliff:			locIdName = "SP_BDKIND_BARGE_OFF_CLIFF"; 			break;
								case kSP_BossDemand_Tier_Forward_Mine_Hits:			locIdName = "SP_BDKIND_FORWARD_MINE_HITS"; 			break;
								case kSP_BossDemand_Tier_Shunt_Hits:				locIdName = "SP_BDKIND_SHUNT_HITS"; 				break;
//								case kSP_BossDemand_Event_Complete_FanRun:			locIdName = "SP_BDKIND_COMPLETE_FANRUN"; 			break;
								case kSP_BossDemand_Tier_Destruction_Bonus_Hits:	locIdName = "SP_BDKIND_DESTRUCTION_BONUS_HITS"; 	break;
//								case kSP_BossDemand_Tier_Drift:						locIdName = "SP_BDKIND_DRIFT";						break;
								case kSP_BossDemand_Tier_Use_Weapon_Defence:		locIdName = "SP_BDKIND_USE_WEAPON_DEFENCE"; 		break;
								case kSP_BossDemand_Tier_Clean_Nitro:				locIdName = "SP_BDKIND_CLEAN_NITRO"; 				break;
								case kSP_BossDemand_Tier_Clean_Triple_Nitros:		locIdName = "SP_BDKIND_TRIPLE_NITRO";				break;
								case kSP_BossDemand_Tier_Get_Lights:				locIdName = "SP_BDKIND_GET_LIGHTS"; 				break;
								case kSP_BossDemand_Tier_Mine_Hits:					locIdName = "SP_BDKIND_MINE_HITS"; 					break;
								case kSP_BossDemand_Tier_Nitro_Ram:					locIdName = "SP_BDKIND_NITRO_RAM"; 					break;
								case kSP_BossDemand_Tier_Multiple_Barge_Hits:		locIdName = "SP_BDKIND_MULTIPLE_BARGE_HITS";		break;
//								case kSP_BossDemand_Event_Finish_In_Nats_BMW:		locIdName = "SP_BDKIND_FINISH_IN_NATS_BMW"; 		break;
								case kSP_BossDemand_Tier_Shield_Mines:				locIdName = "SP_BDKIND_SHIELD_MINES"; 				break;
								case kSP_BossDemand_Tier_3_Bolt_Hits:				locIdName = "SP_BDKIND_3_BOLT_HITS"; 				break;
								case kSP_BossDemand_Tier_Air_Time:					locIdName = "SP_BDKIND_AIR_TIME"; 					break;
								case kSP_BossDemand_Tier_Win_With_1_Health:			locIdName = "SP_BDKIND_WIN_WITH_1_HEALTH";			break;
//								case kSP_BossDemand_Event_Finish_Without_SorR:		locIdName = "SP_BDKIND_FINISH_WITHOUT_SORR"; 		break;
//								case kSP_BossDemand_Tier_Reach_Speed:				locIdName = "SP_BDKIND_REACH_SPEED";				break;
//								case kSP_BossDemand_Event_Wreck_And_Win:			locIdName = "SP_BDKIND_WRECK_AND_WIN";				break;
								case kSP_BossDemand_Tier_Reverse_Bolt_Hits:			locIdName = "SP_BDKIND_REVERSE_BOLT_HITS"; 			break;
								case kSP_BossDemand_Tier_Evade_Full_Shocks:			locIdName = "SP_BDKIND_EVADE_FULL_SHOCKS"; 			break;
//								case kSP_BossDemand_Tier_Win_All_Events:			locIdName = "SP_BDKIND_WIN_ALL_EVENTS"; 			break;
							}
							if( locIdName )
							{
								// Param only
								if( SP_DoesBossDemandUseFloatValue( sg, pDemand ) )
									m_VariableText.Format(  C_TextStringID( locIdName ).String(), SP_GetBossDemandTargetFloat( sg, pDemand ) );
								else
									m_VariableText.Format(  C_TextStringID( locIdName ).String(), SP_GetBossDemandTargetInt( sg, pDemand ) );

								return m_VariableText;
							}


							//special case for %s comma strings
							switch( pDemand->kind )
							{
								case kSP_BossDemand_Tier_Earn_Fans:					locIdName = "SP_BDKIND_EARN_FANS";					break;
								case kSP_BossDemand_Tier_Drift:						locIdName = "SP_BDKIND_DRIFT";						break;
							}
							if( locIdName )
							{
								int num=SP_GetBossDemandTargetInt( sg, pDemand );

								if( SP_DoesBossDemandUseFloatValue( sg, pDemand ) )
									num = (int)SP_GetBossDemandTargetFloat( sg, pDemand );
							
								m_VariableText.Format(  C_TextStringID( locIdName ).String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(num) );

								return m_VariableText;
							}


							switch( pDemand->kind )
							{
								case kSP_BossDemand_Event_Complete_FanRun:			locIdName = "SP_BDKIND_COMPLETE_FANRUN_EVENT"; 		break;
								case kSP_BossDemand_Event_Finish_In_Nats_BMW:		locIdName = "SP_BDKIND_FINISH_IN_NATS_BMW";			break;
							}
							if( locIdName )
							{
								// Event only
								if( !pDemand->event )
									return L"<<Error : Boss demand requires event>>";

								m_VariableText.Format( C_TextStringID( locIdName ).String(), pDemand->event->index+1 );
								return m_VariableText;
							}

							// Two
							switch( pDemand->kind )
							{
								case kSP_BossDemand_Event_Time_Remaining:			locIdName = "SP_BDKIND_TIME_REMAINING";				break;
								case kSP_BossDemand_Event_Get_5X_Combo:				locIdName = "SP_BDKIND_GET_5X_COMBO"; 				break;
//								case kSP_BossDemand_Event_Finish_Without_SorR:		locIdName = "SP_BDKIND_FINISH_WITHOUT_SORR"; 		break;
							}
							if( locIdName )
							{
								if( !pDemand->event )
									return L"<<Error : Boss demand requires event>>";

								// Event & param
								if( SP_DoesBossDemandUseFloatValue( sg, pDemand ) )
									m_VariableText.Format( C_TextStringID( locIdName ).String(), pDemand->event->index+1, SP_GetBossDemandTargetFloat( sg, pDemand ) );
								else
									m_VariableText.Format( C_TextStringID( locIdName ).String(), pDemand->event->index+1, SP_GetBossDemandTargetInt( sg, pDemand ) );
								return m_VariableText;
							}

							switch( pDemand->kind )
							{
								case kSP_BossDemand_Event_Get_All_Lights:			locIdName = "SP_BDKIND_GET_ALL_LIGHTS"; 			break;
							}
							if( locIdName )
							{
								if( !pDemand->event )
									return L"<<Error : Boss demand requieres event>>";

								// Param & event
								m_VariableText.Format( C_TextStringID( locIdName ).String(), SP_GetBossDemandTargetInt( sg, pDemand ), pDemand->event->index+1 );
								return m_VariableText;
							}

							if (pDemand->kind == kSP_BossDemand_Tier_Reach_Speed)
							{
								m_VariableText.Format( C_TextStringID( "SP_BDKIND_REACH_SPEED" ).String(), (S32)((SP_BossDemand_Tier_Reach_Speed*)pDemand)->minspeed );
								return m_VariableText;
							}

							if (pDemand->kind == kSP_BossDemand_Event_Wreck_And_Win)
							{
								const S32 num_cars = (S32)((SP_BossDemand_Event_Wreck_And_Win*)pDemand)->numcars;

								m_VariableText.Format( ( num_cars == 1 ) ? C_TextStringID( "SP_BDKIND_WRECK_AND_WIN" ).String() : C_TextStringID( "SP_BDKIND_WRECK_AND_WIN_PLURAL" ).String(), num_cars, pDemand->event->index+1 );
								return m_VariableText;
							}

							return L"<<Error : Unknown Boss Demand>>";
						}
						else
						if ( !strcmp( pVariableName, "_PROGRESS" ) )
						{
							//apologies for the special casing, but I don't want to start changing whole systems this close to beta
							if( pDemand->kind == kSP_BossDemand_Tier_Earn_Fans)
							{
								UI::C_WideString16 str1( UI::C_UILocaliser::Instance().ConvertNumberToCommas( SP_GetBossDemandValueInt( sg, pDemand ) ) );
								UI::C_WideString16 str2( UI::C_UILocaliser::Instance().ConvertNumberToCommas( SP_GetBossDemandTargetInt( sg, pDemand ) ) );
								m_VariableText.Format( L"%s/%s", str1.Chars(), str2.Chars() );
							}
							else if( pDemand->kind == kSP_BossDemand_Tier_Drift)
							{
								UI::C_WideString16 str1( UI::C_UILocaliser::Instance().ConvertNumberToCommas( (int)SP_GetBossDemandValueFloat( sg, pDemand ) ) );
								UI::C_WideString16 str2( UI::C_UILocaliser::Instance().ConvertNumberToCommas( (int)SP_GetBossDemandTargetFloat( sg, pDemand ) ) );
								m_VariableText.Format( L"%s/%s", str1.Chars(), str2.Chars() );
							}
							else
							{
								if( SP_DoesBossDemandUseFloatValue( sg, pDemand ) )
								{
									m_VariableText.Format( L"%.0f/%.0f", SP_GetBossDemandValueFloat( sg, pDemand ), SP_GetBossDemandTargetFloat( sg, pDemand ) );
								}
								else
								{
									m_VariableText.Format( L"%d/%d", SP_GetBossDemandValueInt( sg, pDemand ), SP_GetBossDemandTargetInt( sg, pDemand ) );
								}
							}
						}
						else
						if ( !strcmp( pVariableName, "_PERCENT" ) )
						{
							m_VariableText.Format( L"%d%%", SP_GetBossDemandPercentageComplete( sg, pDemand ) );
						}
					}
				}
			}
			else
			if ( !strcmp( pVariableName, "DEMANDS" ) )
			{
				if( pTier->boss )
					m_VariableText.Format( C_TextStringID( "SP_DEMANDS_S" ).String(), C_TextStringID(pTier->boss->name.id).String() );
			}
			else
			if ( !strcmp( pVariableName, "DEMANDS_COMPLETE" ) )
			{
				if( pTier->boss )
					m_VariableText.Format( C_TextStringID( "SP_DEMANDS_COMPLETE_S" ).String(), C_TextStringID(pTier->boss->name.id).String() );
			}
			else
			if ( !strcmp( pVariableName, "DEFEATED" ) )
			{
				if( pTier->boss )
					m_VariableText.Format( C_TextStringID( "SP_DEFEATED_S" ).String(), C_TextStringID(pTier->boss->name.id).String() );
			}

		}
	}
	else
	if ( !strncmp( pVariableName, "POB_", 4 ) )
	{	

		// TODO - this needs sorting for loc!!		

		pVariableName += 4;
		if ( !strncmp( pVariableName, "STRING_", 7 ) )
		{
			pVariableName += 7;

			C_WideString512	m_newText(pVariableName);
			m_VariableText.Format( m_newText );
		}
		else
		if ( !strncmp( pVariableName, "NUMBER_", 7 ) )
		{
			pVariableName += 7;

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( ParseIndex(pVariableName, "" ) ) );
		} 
		else
		if ( !strncmp( pVariableName, "LIGHTS_NEEDED", 13 ) )
		{
			pVariableName += 13;

			m_VariableText.Format( C_TextStringID( "POB_UPCOMING_UNLOCKS_TIER_TEXT_1" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( ParseIndex(pVariableName, "" ) ) );
		}
		else
		if ( !strncmp( pVariableName, "TOTAL_LIGHTS", 12 ) )
		{
			pVariableName += 12;

			S32 CurrentLights = (S32)ParseIndexAdvance( pVariableName, "" );
			S32 MaxLights = (S32)ParseIndex( pVariableName, "_" );

			m_VariableText.Format( C_TextStringID( "POB_MILESTONES_TIER_TEXT_2" ).String(), CurrentLights, MaxLights );
		}
		else
		if ( !strncmp( pVariableName, "RANKUP_TEXT", 11 ) )
		{
			pVariableName += 11;

			S32 Rank = (S32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( C_TextStringID( "POB_UNLOCKS_FAN_STATUS_TEXT_1" ).String(), Rank );
		}
		else
		if ( !strncmp( pVariableName, "NEXT_RANKUP_TEXT", 16 ) )
		{
			pVariableName += 16;

			S32 Rank = (S32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( C_TextStringID( "POB_UPCOMING_UNLOCKS_FAN_STATUS_TEXT_1" ).String(), Rank );
		}

		


	}
	else
	if ( !strncmp( pVariableName, "LEADERBOARD_", 12 ) )
	{	

		pVariableName += 12;
		if ( !strncmp( pVariableName, "EVENT_SELECT_NAME", 17 ) )
		{
			pVariableName += 17;


			WCHAR* RaceType = NULL;
			
			if ( !strncmp( pVariableName, "boss", 4 ) )
			{
				pVariableName += 4;
				RaceType = (WCHAR*)C_TextStringID( "SP_KIND_BOSS" ).String();

			}
			else if ( !strncmp( pVariableName, "destruction", 11 ) )
			{
				pVariableName += 11;
				RaceType = (WCHAR*)C_TextStringID( "SP_KIND_DESTRUCTION" ).String();


			}
			else if ( !strncmp( pVariableName, "race", 4 ) )
			{
				pVariableName += 4;
				RaceType = (WCHAR*)C_TextStringID( "SP_KIND_RACE" ).String();


			}
			else if ( !strncmp( pVariableName, "checkpoint", 10 ) )
			{
				pVariableName += 10;
				RaceType = (WCHAR*)C_TextStringID( "SP_KIND_CHECKPOINT" ).String();

			}
			

			S32 EventIndex = (S32)ParseIndex( pVariableName, "" );
			if(RaceType != NULL)
			{
				m_VariableText.Format( C_TextStringID( "LEADERBOARD_EVENT_SELECTION" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas(EventIndex), RaceType );	// [Blur2PC] was a default-int cast (const)RaceType; [Blur2PC][x64] pass the pointer the x86 cast carried
			}
			else
			{
				m_VariableText.Format( L"ERROR" );
			}
		}
	}
	else
	if ( !strncmp( pVariableName, "PHOTOMODE_", 10 ) )
	{
		pVariableName += 10;

		if ( !strncmp( pVariableName, "TOTAL_PHOTOS_", 13 ) )
		{
			pVariableName += 13;

			S32 CurrenPhoto = (S32)ParseIndexAdvance( pVariableName, "" );
			S32 TotalPhotos = (S32)ParseIndex( pVariableName, "_" );

			m_VariableText.Format( C_TextStringID( "SP_PHOTOMODE_TOTAL_PHOTOS" ).String(), CurrenPhoto, TotalPhotos );

		}
	}
	else 
	if ( !strncmp( pVariableName, "EVENT_STAR_COUNT_", 17 ) )
	{
		pVariableName += 17;
	
		S32 numStars = (S32)ParseIndex( pVariableName, "" );

		m_VariableText.Format( C_TextStringID( "SP_LOADING_LIGHTS" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas(numStars) );

	}
	else 
	if ( !strncmp( pVariableName, "EVENT_", 6 ) )
	{
		// Event
		pVariableName += 6;

		XtId eventId = (U32)ParseIndexAdvance( pVariableName, "" );

		SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), eventId );
		if( pEvent )
		{
			if ( !strncmp( pVariableName, "_PRIMARY_GOAL", 13 ) )
			{
				pVariableName += 13;

				if ( !pEvent->event || !pEvent->event->raceMode )
					return L"<< NO EVENT || NO EVENT RACEMODE >>";

				S32 sMedalOverride = (S32)ParseIndexAdvance( pVariableName, "_" );
				S32 sDifficultyOverride = (S32)ParseIndexAdvance( pVariableName, "_" );

				static C_TextStringID s_RaceGoal( "SP_PGOAL_RACE" );
				static C_TextStringID s_RaceGoalOther( "SP_PGOAL_RACE_OTHER" );
				static C_TextStringID s_RaceGoalOtherAbove( "SP_PGOAL_RACE_OTHER_ABOVE" );
				static C_TextStringID s_FanGoal( "SP_PGOAL_FAN" );
				static C_TextStringID s_CheckpointGoal( "SP_PGOAL_CHECKPOINT" );
				static C_TextStringID s_DestructionGoal( "SP_PGOAL_DESTRUCTION" );
				static C_TextStringID s_BossGoal( "SP_PGOAL_BOSS" );

				Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

				if ( !pProfile )
					return L"";

				Game::GAME_DIFFICULTY eDiff = (Game::GAME_DIFFICULTY)pProfile->mOptions.mGameDifficulty;

				if ( sDifficultyOverride != -1 )
					eDiff = (Game::GAME_DIFFICULTY)sDifficultyOverride;

				switch( pEvent->kind )
				{
					case kSP_RaceEvent:
					{
						if ( sMedalOverride == -1 || sMedalOverride == 1 )
							m_VariableText.Format( s_RaceGoalOther.String(), C_UILocaliser::Instance().ConvertNumberToPosition( 1 ) ); //m_VariableText = s_RaceGoal;
						else
						if ( sMedalOverride == 2 )
							m_VariableText.Format( s_RaceGoalOther.String(), C_UILocaliser::Instance().ConvertNumberToPosition( 2 ) );
						else
						if ( sMedalOverride == 3 )
							m_VariableText.Format( s_RaceGoalOther.String(), C_UILocaliser::Instance().ConvertNumberToPosition( 3 ) );
						else
							m_VariableText.Format( s_RaceGoalOtherAbove.String(), C_UILocaliser::Instance().ConvertNumberToPosition( 3 ) );
						break;
					}
					case kSP_FanEvent:
					{
						if ( pEvent->event->raceMode->raceMode != Game::E_RaceMode_FanDemand )
							return L"<< WRONG RACEMODE >>";

						AmaxRaceModeFanDemand* pRaceMode = (AmaxRaceModeFanDemand*)pEvent->event->raceMode;

						AmaxRaceModeFanDemandVars* pVars = &pRaceMode->easy;

						if ( eDiff == Game::GAME_DIFFICULTY_NORMAL )
							pVars = &pRaceMode->medium;
						else if ( eDiff == Game::GAME_DIFFICULTY_HARD || eDiff == Game::GAME_DIFFICULTY_HARDCORE )
							pVars = &pRaceMode->hard;

						if ( sMedalOverride == -1 || sMedalOverride == 1 )
							m_VariableText.Format( s_FanGoal.String(), pVars->GoldNumToComplete );
						else
						if ( sMedalOverride == 2 )
							m_VariableText.Format( s_FanGoal.String(), pVars->SilverNumToComplete );
						else
							m_VariableText.Format( s_FanGoal.String(), pVars->BronzeNumToComplete );
						break;
					}
					case kSP_CheckpointEvent:
					{
						if ( pEvent->event->raceMode->raceMode != Game::E_RaceMode_Checkpoint )
							return L"<< WRONG RACEMODE >>";

						AmaxRaceModeCheckpoint* pRaceMode = (AmaxRaceModeCheckpoint*)pEvent->event->raceMode;

						AmaxRaceModeCheckpointVars* pVars = &pRaceMode->easy;

						if ( eDiff == Game::GAME_DIFFICULTY_NORMAL )
							pVars = &pRaceMode->medium;
						else if ( eDiff == Game::GAME_DIFFICULTY_HARD || eDiff == Game::GAME_DIFFICULTY_HARDCORE )
							pVars = &pRaceMode->hard;

						if ( sMedalOverride == -1 || sMedalOverride == 1 )
							m_VariableText.Format( s_CheckpointGoal.String(), pVars->goldTimeRemaining );
						else
						if ( sMedalOverride == 2 )
							m_VariableText.Format( s_CheckpointGoal.String(), pVars->silverTimeRemaining );
						else
							m_VariableText.Format( s_CheckpointGoal.String(), 0.0f );
						break;
					}
					case kSP_DestructionEvent:	
					{
						if ( pEvent->event->raceMode->raceMode != Game::E_RaceMode_Destruction )
							return L"<< WRONG RACEMODE >>";

						AmaxRaceModeDestruction* pRaceMode = (AmaxRaceModeDestruction*)pEvent->event->raceMode;

						AmaxRaceModeDestructionVars* pVars = &pRaceMode->easy;

						if ( eDiff == Game::GAME_DIFFICULTY_NORMAL )
							pVars = &pRaceMode->medium;
						else if ( eDiff == Game::GAME_DIFFICULTY_HARD || eDiff == Game::GAME_DIFFICULTY_HARDCORE )
							pVars = &pRaceMode->hard;

						if ( sMedalOverride == -1 || sMedalOverride == 1 )
							m_VariableText.Format( s_DestructionGoal.String(), pVars->goldPoints );
						else
						if ( sMedalOverride == 2 )
							m_VariableText.Format( s_DestructionGoal.String(), pVars->silverPoints );
						else
							m_VariableText.Format( s_DestructionGoal.String(), pVars->bronzePoints );
						break;
					}
					case kSP_BossEvent:
					{
						m_VariableText.Format( s_BossGoal.String(), C_TextStringID(pEvent->tier->boss->name.id).String() );
						break;
					}
				}
			}
			else
			if ( !strcmp( pVariableName, "_SECONDARY_GOAL" ) )
			{
				static C_TextStringID s_FanParGoal( "SP_SGOAL_PAR" );

				m_VariableText.Format( s_FanParGoal.String(), C_UILocaliser::Instance().ConvertNumberToCommas( pEvent->fanPar ) );
			}
			else
			if ( !strcmp( pVariableName, "_GFD" ) )
			{
				// TODO
			}
			else
			if ( !strcmp( pVariableName, "_LAPS" ) )
			{
				static C_TextStringID s_Lap( "CMN_D_LAP" );
				static C_TextStringID s_Laps( "CMN_D_LAPS" );

				XtHandle<AmaxEvent> amaxEvent = pEvent->event;
				if( amaxEvent && pEvent->event )
				{
					if(	amaxEvent->laps > 0 && amaxEvent->raceMode->raceMode != Game::E_RaceMode_Checkpoint && amaxEvent->raceMode->raceMode != Game::E_RaceMode_FriendDemand_Checkpoint )
					{
						m_VariableText.Format( ( amaxEvent->laps == 1 ) ? s_Lap.String() : s_Laps.String(), amaxEvent->laps );
					}
				}
			}
			else
			if ( !strcmp( pVariableName, "_OWNER_NAME" ) )
			{
				if(Game::C_EventOwnerManager::Instance().BeatingFriend(eventId) == Game::EVENT_OWNER_RESULT_WINNING)
				{
					U32 padNo = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
					SysProfile::FullProfile& profile = UI::C_ProfileInterface::Instance().GetPadProfile( padNo );
					m_VariableText.Format( profile.Name() );
				}
				if(Game::C_EventOwnerManager::Instance().BeatingFriend(eventId) == Game::EVENT_OWNER_RESULT_LOSING)
				{
					bdOnlineUserID id=Game::C_EventOwnerManager::Instance().GetFriend();

					SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromOnlineID(id);

					if(prof)
					{
						m_VariableText.Format(prof->Name());
					}
				}
				if(Game::C_EventOwnerManager::Instance().BeatingFriend(eventId) == Game::EVENT_OWNER_RESULT_SCORE_DRAW)
				{
					m_VariableText.Format( L"Equal Standing" );
				}
			}
			else
			if ( !strcmp( pVariableName, "_RIVAL_NAME" ) )
			{
				bdOnlineUserID id=Game::C_EventOwnerManager::Instance().GetFriend();
				SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromOnlineID(id);
				if(prof)
				{
					m_VariableText.Format(prof->Name());
				}
			}
			else
			if ( !strcmp( pVariableName, "_RIVAL_SCORE" ) )
			{
				m_VariableText.Format(Game::C_EventOwnerManager::Instance().GetScoreString(eventId));
			}
			else
			if ( !strcmp( pVariableName, "_OWNED" ) )
			{
				if(Game::C_EventOwnerManager::Instance().BeatingFriend(eventId) == Game::EVENT_OWNER_RESULT_WINNING)
				{
					m_VariableText.Format( L"Owning" );
				}
				if(Game::C_EventOwnerManager::Instance().BeatingFriend(eventId) == Game::EVENT_OWNER_RESULT_LOSING)
				{
					m_VariableText.Format( L"Losing" );
				}
			}
			else
			if ( !strcmp( pVariableName, "_NAME_NUM_TYPE" ) )
			{
				C_TextStringID idKind;

				if ( pEvent->kind == kSP_RaceEvent )
					idKind = UI::C_TextStringID( "SP_KIND_RACE" );
				else
				if ( pEvent->kind == kSP_CheckpointEvent )
					idKind = UI::C_TextStringID( "SP_KIND_CHECKPOINT" );
				else
				if ( pEvent->kind == kSP_DestructionEvent )
					idKind = UI::C_TextStringID( "SP_KIND_DESTRUCTION" );
				else
				if ( pEvent->kind == kSP_FanEvent )
					idKind = UI::C_TextStringID( "SP_KIND_FAN" );
				else
				if ( pEvent->kind == kSP_BossEvent )
					idKind = UI::C_TextStringID( "SP_KIND_BOSS" );

				m_VariableText.Format( C_TextStringID( "SP_EVENT_NAME_NUM_TYPE" ).String(), ( pEvent->index + 1 ), idKind.String() );
			}
			else
			if ( !strcmp( pVariableName, "_PRIMARY_BEST" ) )
			{
				if ( pEvent->kind == kSP_RaceEvent )
				{
					SP_EventState medal=SP_GetEventState( SP_GetCurrentSaveGame(), pEvent );

					if(medal == kSP_Gold)
					{
						m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToPosition(1));
					}
					else if(medal == kSP_Silver)
					{
						m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToPosition(2));
					}
					else if(medal == kSP_Bronze)
					{
						m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToPosition(3));
					}
					else
					{
						m_VariableText.Format( L"--" );
					}

				}
				else if ( pEvent->kind == kSP_CheckpointEvent )
				{
					F32 fBestTime = SP_GetEventBestTime( SP_GetCurrentSaveGame(), pEvent );
					if(fBestTime)
						m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( fBestTime, true, false, false ) );
					else
						m_VariableText.Format( L"--" );
				}
				else if ( pEvent->kind == kSP_DestructionEvent )
				{
					int points = SP_GetEventBestPoints( SP_GetCurrentSaveGame(), pEvent );
					if(points)
						m_VariableText.Format(UI::C_TextStringID( "SP_PGOAL_DESTRUCTION" ).String(), points);
					else
						m_VariableText.Format( L"--" );
				}
			}
			else
			if ( !strcmp( pVariableName, "_SECONDARY_BEST" ) )
			{
				int fans=SP_GetEventBestFans(SP_GetCurrentSaveGame(), pEvent);
				if(fans)
					m_VariableText.Format( UI::C_TextStringID( "SP_SGOAL_PAR" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( fans ) );
				else
					m_VariableText.Format( L"--" );
			}
		}
	}

	return m_VariableText;
}

}
