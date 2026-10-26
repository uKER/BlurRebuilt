#include "stdafx.h"
#include <GameLogic/SinglePlayer/SinglePlayerImpl.h>

#include <GameLogic/Structures/LevelState.h>
#include <Game/Game.h>
#include <GameLogic/GameData/GameVehicleData.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic/Structures/LevelStateMultiplayerAbilities.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic/SinglePlayer/SP_BossDemands.h>

#include <Ui/UiMessages.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>

//#pragma optimize("", off)





//=========================================================================
//					Main Routines
//=========================================================================


SP_BossDemands	SP_BossDemands::s_Instance;				// Our single instance


//=========================================================================

SP_BossDemands::SP_BossDemands()
{
}

//=========================================================================

SP_BossDemands::~SP_BossDemands()
{
}



//=========================================================================
//					Main Routines
//=========================================================================


void SP_BossDemands::RegisterBossDemandAction(E_BOSSDEMAND_ACTION type, F32 value)
{
	// no boss demands in friend challenges
	if(Game::C_FriendChallenges::Instance().IsAttemptingFriendDemand())
		return;

	Game::C_LevelState* levstate = &Game::C_Game::GetInstance()->GetLevelState();
	if (!levstate)
		return;

	XtId	eventId = levstate->mSpEventId;
	XtId	player_veh_ID = levstate->mCars[0].mCarID;



	switch(type)
	{
		case E_BOSSDEMAND_ACTION_NEW_EVENT:
		{
			ResetEventStats();

			SP_Boss* boss = GetCurrentBoss();
			if (boss)
			{
				for(int i = 0; i < boss->demands.count; i++)
				{
					if (SP_BossDemand* demand = boss->demands[i])
					{
						if (demand->kind == kSP_BossDemand_Tier_Reach_Speed)
						{
							min_speed = ((SP_BossDemand_Tier_Reach_Speed*)demand)->minspeed;
						}

						if (demand->kind == kSP_BossDemand_Event_Wreck_And_Win)
						{
							target_AI_wrecked = ((SP_BossDemand_Event_Wreck_And_Win*)demand)->numcars;
						}
					}
				}
			}
			break;
		}

		case E_BOSSDEMAND_ACTION_EVENT_FINISHED:
		{
			Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

			if (SP_SaveGame* sg = SP_GetCurrentSaveGame())						// Check for Nats car
			{
				if (SP_Event* sp_event = SP_GetEventById(sg, eventId))
				{
					if (SP_Tier* tier = sp_event->tier)
					{
						if (SP_Boss* boss = tier->boss)
						{
							for(int i = 0; i < boss->demands.count; i++)
							{
								if (SP_BossDemand* demand = boss->demands[i])
								{
									if (demand->kind == kSP_BossDemand_Event_Finish_In_Nats_BMW)
									{
										if (results.mRaceStatistics.mMedal == kSP_Gold)					// Must have won
										{
											XtId	nats_veh_ID = ((SP_BossDemand_Event_Finish_In_Nats_BMW*)demand)->vehicle->ID;
											if (player_veh_ID == nats_veh_ID)
												ModifyBossDemandValue(kSP_BossDemand_Event_Finish_In_Nats_BMW, 1.0f, true);		// *** Tier 7 ***
										}
									}
								}
							}
						}

						if (tier->index == 0)												// Check for driving new car in tier 0
						{
							Game::C_GameProfile* gameProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
							if(gameProf)
							{
								bool found = false;
								for( Btl::Set<XtId>::Iterator it = gameProf->mVehiclesUsedInTierOne.Begin(); it != gameProf->mVehiclesUsedInTierOne.End(); it++ )
								{
									if (player_veh_ID == *it)
									{
										found = true;
										break;
									}
								}
								if (!found)
								{
									gameProf->mVehiclesUsedInTierOne.Insert(player_veh_ID);
								}
								ModifyBossDemandValue(kSP_BossDemand_Tier_Finish_In_Different_Car, (F32)gameProf->mVehiclesUsedInTierOne.Size(), true);	// *** Tier 1 ***
							}
						}

					}
					
					if (!has_been_wrecked && !used_repair)								// Changed to Not used repair and not wrecked
						if (sp_event->kind == kSP_RaceEvent)							// Also now changed from event specific to any Race
							ModifyBossDemandValue(kSP_BossDemand_Event_Finish_Without_SorR, value, false);			// *** Tier 8 ***
				}
			}


			if (got_x5_fan_combo)												// If got a x5 fab combo
				if (results.mRaceStatistics.mMedal == kSP_Gold)										// Must have won
					ModifyBossDemandValue(kSP_BossDemand_Event_Get_5X_Combo, value, false);				// *** Tier 3 ***


			if (finished_with_1_health)
				if (results.mRaceStatistics.mMedal >= kSP_Bronze)				// Must have come in top three
					ModifyBossDemandValue(kSP_BossDemand_Tier_Win_With_1_Health, value, false);			// *** Tier 8 ***


			if (results.mRaceStatistics.mMedal >= kSP_Gold)						// Wreck X number and win
				if (num_AI_wrecked >= target_AI_wrecked)
					ModifyBossDemandValue(kSP_BossDemand_Event_Wreck_And_Win, value, false);


			ModifyBossDemandValue(kSP_BossDemand_Event_Time_Remaining, results.mRaceStatistics.mTotalRaceTime, true);			// *** Tier 2 ***

			break;
		}

		case E_BOSSDEMAND_ACTION_EVENT_RESULTS_UPDATED:
			break;
			
		case E_BOSSDEMAND_ACTION_GOT_FANS:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Earn_Fans, value, false);					// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_PLAYER_WRECKED:
			has_been_wrecked = true;
			break;

		case E_BOSSDEMAND_ACTION_WRECKED_ANOTHER:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Wreck, 1.0f, false);						// *** Tier 4 ***
			num_AI_wrecked++;
			break;

		case E_BOSSDEMAND_ACTION_RAM_INTO_SEA:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Ram_Into_Sea, value, false);				// *** Tier 4 ***
			break;

		case E_BOSSDEMAND_ACTION_BARGE_OFF_CLIFF:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Barge_Off_Cliff, value, false);			// *** Tier 5 ***
			break;
			
		case E_BOSSDEMAND_ACTION_DRIFTED:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Drift, value, false);						// *** Tier 6 ***
			break;
			
		case E_BOSSDEMAND_ACTION_AIR_TIME:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Air_Time, value, false);					// *** Tier 8 ***
			break;
			
		case E_BOSSDEMAND_ACTION_SPEED:
			if (min_speed != 0.0f)						// If we're checking for minimum speeds
			{
				F32 currentpos = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0)->GetPositionInfo().mLogicPosition;
				if (currentpos < 0.0025f)		// Smallest lap length ~2000m and A class can travel 5m in 1 frame, therefore = 5/2000 = 0.0025
					currentpos = 0.0025f;		// Stops the case where the player exceeds min speed exactly over the start line

				if (value >= min_speed)				// If still above min speed
				{
					if ((min_speed_last_pos < min_speed_start_pos) && (currentpos >= min_speed_start_pos))
						ModifyBossDemandValue(kSP_BossDemand_Tier_Reach_Speed, 1.0f, true);				// *** Tier 9 ***
				}
				else								// If below min speed
				{
					min_speed_start_pos = currentpos;
				}
				min_speed_last_pos = currentpos;
			}

			break;
			
		case E_BOSSDEMAND_ACTION_USED_POWERUP:
			// Updated from other actions
//			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_SHOCK_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Shock_Opponents, value, false);			// *** Tier 2 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_BARGE_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Barge_Hits, value, false);				// *** Tier 4 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_SHUNT_HIT:
//			ModifyBossDemandValue(kSP_BossDemand_Tier_Shunt_Hits, value, false);				// *** Tier 5 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_REVERSE_SHUNT_HIT:
			// Note: E_BOSSDEMAND_ACTION_SHUNT_HIT will still be sent prior to this one
			ModifyBossDemandValue(kSP_BossDemand_Tier_Shunt_Hits, value, false);				// *** Tier 5 ***	// Must now be a reverse hit
			break;

		case E_BOSSDEMAND_ACTION_MINE_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Mine_Hits, value, false);					// *** Tier 7 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_BOLT_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_FORWARD_MINE_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Forward_Mine_Hits, value, false);			// *** Tier 5 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_MULTIPLE_BARGE_HIT:									// Value = number of cars hit
			ModifyBossDemandValue(kSP_BossDemand_Tier_Multiple_Barge_Hits, 1.0f, false);		// *** Tier 7 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Barge_Hits, value - 1.0f, false);			// *** Tier 4 ***		// E_BOSSDEMAND_ACTION_BARGE_HIT is also called which incs use_powerups and one of the barge_hits
			break;

		case E_BOSSDEMAND_ACTION_3_BOLT_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_3_Bolt_Hits, value, false);				// *** Tier 8 ***
//			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_REVERSE_BOLT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Reverse_Bolt_Hits, value, false);			// *** Tier 9 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Powerups, value, false);				// *** Tier 1 ***
			break;

		case E_BOSSDEMAND_ACTION_COMPLETE_FAN_DEMAND:			// Done
			ModifyBossDemandValue(kSP_BossDemand_Tier_Complete_FanDemands, value, false);		// *** Tier 3 ***
			break;

		case E_BOSSDEMAND_ACTION_COMPLETED_SINGLE_FAN_RUN:		// Done
			ModifyBossDemandValue(kSP_BossDemand_Event_Complete_FanRun, value, false);			// *** Tier 5 ***
			break;

		case E_BOSSDEMAND_ACTION_REPAIR:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Repair_Car, value, false);				// *** Tier 2 ***
			used_repair = true;
			break;

		case E_BOSSDEMAND_ACTION_SHIELD_POWERUPS:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Shield_Powerups, value, false);			// *** Tier 2 ***
			used_shield = true;
			break;
	
		case E_BOSSDEMAND_ACTION_EVADE_SHOCK:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Evade_Full_Shocks, value, false);			// *** Tier 9 ***
			break;

		case E_BOSSDEMAND_ACTION_USE_WEAPON_DEFENCE:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Use_Weapon_Defence, value, false);		// *** Tier 6 ***
			break;

		case E_BOSSDEMAND_ACTION_SHIELD_MINES:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Shield_Mines, value, false);				// *** Tier 8 ***
			ModifyBossDemandValue(kSP_BossDemand_Tier_Shield_Powerups, value, false);			// *** Tier 2 ***	
			used_shield = true;
			break;

		case E_BOSSDEMAND_ACTION_CLEAN_NITRO:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Clean_Nitro, value, false);				// *** Tier 6 ***
			break;
		
		case E_BOSSDEMAND_ACTION_CLEAN_TRIPLE_NITRO:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Clean_Triple_Nitros, value, false);		// *** Tier 6 ***
			break;	
		
		case E_BOSSDEMAND_ACTION_NITRO_RAM:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Nitro_Ram, value, false);					// *** Tier 7 ***
			break;

		case E_BOSSDEMAND_ACTION_X5_FAN_COMBO:
			got_x5_fan_combo = true;
			break;

		case E_BOSSDEMAND_ACTION_SECONDS_IN_FIRST:
			if ((levstate->mRaceMode == Game::E_RaceMode_StreetRace) || (levstate->mRaceMode == Game::E_RaceMode_FriendDemand_StreetRace))		// Only include if in a race event
				ModifyBossDemandValue(kSP_BossDemand_Tier_Seconds_In_First, value, false);		// *** Tier 2 ***
			break;	
		
		case E_BOSSDEMAND_ACTION_WIN_WITH_1_HEALTH:
			finished_with_1_health = true;
//			ModifyBossDemandValue(kSP_BossDemand_Tier_Win_With_1_Health, value, false);			// *** Tier 8 ***
			break;	

		case E_BOSSDEMAND_ACTION_DESTRUCTION_BONUS_HIT:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Destruction_Bonus_Hits, value, false);	// *** Tier 5 ***
			break;

		case E_BOSSDEMAND_ACTION_COLLECT_STOPWATCH:
			ModifyBossDemandValue(kSP_BossDemand_Tier_Collect_Stopwatches, value, false);		// *** Tier 2 ***
			break;
	}
}




//=========================================================================


void SP_BossDemands::ModifyBossDemandValue(SP_BossDemandKind kind, F32 value, bool use_max_value)
{
	S32		oldval,newval,target;


	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	XtId eventId = levelState.mSpEventId;

	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		if (SP_Event* sp_event = SP_GetEventById(sg, eventId))
		{
			if (SP_Tier* tier = sp_event->tier)
			{
				if (SP_Boss* boss = tier->boss)
				{
					for(int i = 0; i < boss->demands.count; i++)
					{
						if (SP_BossDemand* demand = boss->demands[i])
						{
							if (demand->kind == kind)
							{
								if ((!demand->event) || (demand->event == sp_event))		// If no event specified or it's the current one
								{

									if (SP_IsBossDemandComplete(sg, demand))				// Exit if already complete
										return;

									if (SP_DoesBossDemandUseFloatValue(sg, demand))			// Get the old value
									{
										oldval = (S32)SP_GetBossDemandValueFloat(sg, demand);
										target = (S32)SP_GetBossDemandTargetFloat(sg, demand);
									}
									else
									{
										oldval = SP_GetBossDemandValueInt(sg, demand);
										target = SP_GetBossDemandTargetInt(sg, demand);
									}

									if (use_max_value)										// Update the current value
									{
										if (SP_DoesBossDemandUseFloatValue(sg, demand))
										{
											if (SP_GetBossDemandValueFloat(sg, demand) < value)
												SP_SetBossDemandValueFloat(sg, demand, value);
										}
										else
										{
											if (SP_GetBossDemandValueInt(sg, demand) < value)
												SP_SetBossDemandValueInt(sg, demand, (int)value);
										}
									}
									else
									{
										if (SP_DoesBossDemandUseFloatValue(sg, demand))
											SP_SetBossDemandValueFloat(sg, demand, SP_GetBossDemandValueFloat(sg, demand) + value);
										else
											SP_SetBossDemandValueInt(sg, demand, SP_GetBossDemandValueInt(sg, demand) + (int)value);
									}

									if (SP_DoesBossDemandUseFloatValue(sg, demand))			// Get the new value
										newval = (S32)SP_GetBossDemandValueFloat(sg, demand);
									else
										newval = SP_GetBossDemandValueInt(sg, demand);

									if (ShouldSendHUDMessage(oldval, newval, target))		// Do we need to diplay this change
										_UI_MESSAGE1( UI::E_UiGameMsg_BossDemandIncrement, demand->index );
								}
							}
						}
					}
				}
			}
		}
	}
}


//=========================================================================


bool SP_BossDemands::ShouldSendHUDMessage(S32 oldval, S32 newval, S32 target)
{
	if (target == 0.0f)									// Failsafe
		return(false);

	if (newval == oldval)								// Don't show if nothings changed
		return(false);

	if (oldval == 0)									// Always show first
		return(true);

	if (newval == target)								// Always show last
		return(true);


	S32		always_min = 3;
	S32		always_max = target - 3;
	S32		step = 1;
	bool	use_every_hundred = false;


	if (target > 500)
	{
		use_every_hundred = true;
		always_min = 300;
		always_max = target - 300;
		step = 1000;
	}
	else if (target > 100)
	{
		step = 25;
	}
	else if (target > 50)
	{
		step = 20;
	}
	else if (target > 10)
	{
		step = 10;
	}
	else
	{
		return(true);					// Always display if <= 10
	}

	
	if ((newval <= always_min) || (newval > always_max))
	{
		if (use_every_hundred)
		{
			if ((newval / 100) != (oldval / 100))
				return(true);
			else
				return(false);
		}
		else
		{
			return(true);
		}
	}

	if ((newval / step) != (oldval / step))
		return(true);

	return(false);
}

//=========================================================================

SP_Boss* SP_BossDemands::GetCurrentBoss()
{
	XtId eventId = Game::C_Game::GetInstance()->GetLevelState().mSpEventId;

	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
		if (SP_Event* sp_event = SP_GetEventById(sg, eventId))
			if (SP_Tier* tier = sp_event->tier)
				return(tier->boss);

	return(NULL);
}

//=========================================================================

void SP_BossDemands::ResetEventStats()
{
	used_shield = false;
	used_repair = false;
	has_been_wrecked = false;
	got_x5_fan_combo = false;
	finished_with_1_health = false;

	min_speed = 0.0f;
	min_speed_start_pos = 0.0025f;				// 0.0025 Stops logic errors - See below
	min_speed_last_pos = 0.0025f;

	num_AI_wrecked = 0;
	target_AI_wrecked = 0;
}

//=========================================================================

void SP_BossDemands::UpdateBossDemandsForTier(SP_SaveGame* sg, SP_Tier* tier)
{
	SP_Boss* boss = tier->boss;

	if (!boss)
		return;

	S32 finish_events = 0;
	S32 complete_fanruns = 0;
	S32 complete_fanpars = 0;
	S32 get_lights = 0;
	S32 win_all_events = 0;

	for(int a=0;a<tier->events.count;a++)
	{
		if (SP_Event* tierEvent = tier->events[a])
		{
			
			if (SP_SaveEvent* se = SP_GetSaveEvent(sg, tierEvent->id))
			{
				if ((tierEvent->kind != kSP_BossEvent) && (se->IsComplete())) // se->GotGold()
					finish_events++;

				if (SP_IsEventFanRunComplete(sg, tierEvent))
					complete_fanruns++;
					
				if (SP_IsEventFanParComplete(sg, tierEvent))
					complete_fanpars++;
					
				get_lights += SP_CountTotalEventStars(sg, tierEvent);
				
				if (se->Unlocked() && (se->State() == kSP_Gold))
					win_all_events++;
			}
		}
	}

	for(int demandNo = 0; demandNo < boss->demands.count; demandNo++)
	{
		if (SP_BossDemand* demand = boss->demands[demandNo])
		{
			switch(demand->kind)
			{
				case kSP_BossDemand_Tier_Finish_Events:		SP_SetBossDemandValueInt(sg, demand, finish_events);	break;
				case kSP_BossDemand_Tier_Complete_FanRuns:	SP_SetBossDemandValueInt(sg, demand, complete_fanruns);	break;
				case kSP_BossDemand_Tier_Complete_FanPars:	SP_SetBossDemandValueInt(sg, demand, complete_fanpars);	break;
				case kSP_BossDemand_Tier_Get_Lights:		SP_SetBossDemandValueInt(sg, demand, get_lights);		break;
				case kSP_BossDemand_Tier_Win_All_Events:	SP_SetBossDemandValueInt(sg, demand, win_all_events);	break;
				
				case kSP_BossDemand_Event_Get_All_Lights:
					if (SP_Event* tierEvent = ((SP_BossDemand_Event_Get_All_Lights*)demand)->event)
					{
						if (SP_SaveEvent* se = SP_GetSaveEvent(sg, tierEvent->id))
						{
							SP_SetBossDemandValueInt(sg, demand, SP_CountTotalEventStars(sg, tierEvent));
						}
					}
				break;
			}
		}
	}
	
}




