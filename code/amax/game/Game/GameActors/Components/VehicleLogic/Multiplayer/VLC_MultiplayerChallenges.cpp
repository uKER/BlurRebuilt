#include "stdafx.h"

#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MultiplayerChallenges.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include <Game/Messaging/GameflowMessages.h>
#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"
#include "Game/Game.h"
#include <GameLogic\Structures\LevelResult.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include "Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h"

#include <system\time\Time.h>

#include <misc_shared/messaging/GameMessaging.h>

#include "racingphysics/vehicles/car/car.h"
#include <GameLogic/Structures/MpResults.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include "GameLogic/Structures/MultiplayerChallenge.h"

#include <vehicle\Input.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game\Game.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>

Debug::Bool g_FavouriteGroupsUpdate("Multiplayer.FavouriteGroups.Update", false );
Debug::Logger &g_MultiplayerChallengesLogger( Debug::GetLogger("Amax.VLC.MultiplayerChallenges") );
Debug::Float g_TimeMinesConsideredRecentlyDroppedInSeconds("Amax.VLC.MultiplayerChallenges.TimeMinesConsideredRecentlyDroppedInSeconds",2.5f,Debug::Persist());

namespace Components
{
	const int C_VehicleComponent_MP_Challenges::s_mQuickDrawCarsToWreck = 3;
	const float C_VehicleComponent_MP_Challenges::s_mMetresInAMile = 1609.f;

	C_VehicleComponent_MP_Challenges::C_VehicleComponent_MP_Challenges() :	mVehicleIndex(-1), 
																			m_pRaceLogicLayerPtr(0), 
																			m_pRaceBookLayerPtr(0), 
																			m_pRLC(0), 
																			m_pLogicActor(0), 
																			mCarID(0)
	{
	}

	bool C_VehicleComponent_MP_Challenges::CreateImpl()
	{
		const I_VehicleComponent_MP_Challenges_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Challenges_CreateInfo *>(GetCreator().GetData());

		if(creation_data)
		{
			mVehicleIndex=creation_data->mVehicleIndex;
			m_pRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
			m_pRaceBookLayerPtr=creation_data->mRaceBookLayerPtr;
			
			m_pRLC=creation_data->m_pRLC;
			BIZ_ASSERT_MSG(m_pRLC, "C_VehicleComponent_MP_Challenges::CreateImpl() > No pointer to the RLC...kabooom!!!");

			m_pLogicActor=creation_data->m_pLogicActor;
			BIZ_ASSERT_MSG(m_pLogicActor, "C_VehicleComponent_MP_Challenges::CreateImpl() > Logic actor is NULL!");

			//Cache our stuff here
			m_pMultiplayerProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile();
			m_pMultiplayerProfile->GetMultiplayerStatistics().mNumChallengesCompletedThisRace = m_pMultiplayerProfile->GetMultiplayerStatistics().mNumFansEarnedFromChallengesThisRace = 0;
			m_pLevelState = &(C_GameApp::GetInstance()->GetGame()->GetLevelState());
			m_pLevelResult = &(C_GameApp::GetInstance()->GetGame()->GetLevelResult());
			mCarID = m_pLevelState->mCars[mVehicleIndex].mCarID;

			// Clear any temporary 'per-race' blackboard data
			m_pMultiplayerProfile->GetChallengeBlackboard().InitForNextRace();
		}

		mStartingLevel=m_pMultiplayerProfile->GetCurrentLevel();

		// Reset timer
		mLastGaspChallengeDelayTimer = 0.0f;

		// Control messages we react to
		AddSubscriber(Game::E_GameflowMessage_RaceStarted);
		AddSubscriber(Game::E_GameflowMessage_RaceFinished);
		AddSubscriber(Game::E_GameflowMessage_QuitRace);
		AddSubscriber(Game::E_GameflowMessage_QuitLevel);
		AddSubscriber(Game::E_VehicleMessage_FatallyDamaged);
		AddSubscriber(Game::E_VehicleMessage_Eliminated);
		AddSubscriber(Game::E_GameflowMessage_VehicleFinished);
		AddSubscriber(Game::E_GameLogicMessage_LapIncremented);
		AddSubscriber(E_VehicleLogicMsg_AggressiveScore);
		AddSubscriber(E_VehicleLogicMsg_DestroyedPickupWithPickup);
		AddSubscriber(E_VehicleLogicMsg_PowerUpDeleted);
		AddSubscriber(GameMessaging::E_GameMsg_Update);

		// Ones we pass on to our challenges
		AddSubscriber(Game::E_VehicleMessage_Overtake);
		AddSubscriber(E_VehicleLogicMsg_Skillshot);
		AddSubscriber(E_VehicleLogicMsg_Wrecked);
		AddSubscriber(E_VehicleLogicMsg_Scored);
		AddSubscriber(E_VehicleLogicMsg_FiredPerk);
		AddSubscriber(E_VehicleLogicMsg_HitByPerk);
		AddSubscriber(E_VehicleLogicMsg_PerkEnded);
		AddSubscriber(E_VehicleLogicMsg_ShieldedPerk);
		AddSubscriber(Game::E_PickupMessage_PickupPickedUp);
		AddSubscriber(Game::E_GameLogicMessage_GameplayImpact);
		AddSubscriber(E_VehicleLogicMsg_ModActivated);
		AddSubscriber(E_VehicleLogicMsg_ChainCompleted);
		AddSubscriber(Game::E_GameLogicMessage_MastermineSpawnedMinis);
		AddSubscriber(Game::E_VehicleMessage_ResetAfterWrecked);

		mEnabled = false;
		LOGDEBUG(g_MultiplayerChallengesLogger, "---------------------------------------------------------------------");

		return true;
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------

#ifndef CFG_GOLD
	void C_VehicleComponent_MP_Challenges::UpdateDebugValues()
	{
	}
#endif

	// ---------------------------------------------------------------------------------------------------------------------------------------

	const bool C_VehicleComponent_MP_Challenges::ChallengeCurrentlyCompletable(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType) const
	{
		// This one function checks both whether the challenge is currently unlocked, and if so, is it currently uncompleted. 
		if(CurrentlyUnlocked(eChallengeType) && !AlreadyComplete(eChallengeType))
		{
			// Then challenge can be completed. 
			return true;
		}

		// if reach here then challenge cannot be completed
		return false; 
	}

	const bool C_VehicleComponent_MP_Challenges::CurrentlyUnlocked(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType) const
	{
		// Simply get the cached value from the profile! (all the longwinded challenge unlocked checks were moved to just after race load!)
		BIZ_ASSERT_MSG( m_pMultiplayerProfile, "No multiplayer profile" );
		return m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(eChallengeType).IsUnlocked(mStartingLevel);
	}


	//Check to see if the challenge is complete, we might need to add versions for carID/challenge type and pickupType/challengeType, we don't need them yet though
	const bool C_VehicleComponent_MP_Challenges::AlreadyComplete(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType, const bool overallProgress) const
	{
		const Game::C_MultiplayerChallenge& challenge=m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(eChallengeType);
	
		return challenge.IsCompleted();
	}

	const bool C_VehicleComponent_MP_Challenges::CheckChallenges(const Messaging::C_MessageBase *pMessage )
	{
		if(!mEnabled)
			return true;

		Game::MiscellaneousChallengeBlackboardData& blackboard = m_pMultiplayerProfile->GetChallengeBlackboard();
		Game::C_MultiplayerChallengeSystem& challengeSystem = m_pMultiplayerProfile->GetChallengeSystem();
		
		switch(pMessage->Type())
		{
		case E_VehicleLogicMsg_Skillshot:
			{
				blackboard.mSkillshotsPerformed++;
				break;
			}
		case E_VehicleLogicMsg_FiredPerk:
			{
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_FiredPerkMsg>(pMessage);
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					UpdatePerkFiredData(pMsg);
				}
				break;
			}

		case E_VehicleLogicMsg_Landed:
			{
				const C_VehicleLogicMsg_LandedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_LandedMsg>(pMessage);
				if(pMsg->mAirborneVehicleIndex == mVehicleIndex)
				{
					blackboard.mTimeSpentInAir += pMsg->mTimeInAir;
				}
				break;
			}
		case E_VehicleLogicMsg_Wrecked:
			{
				const C_VehicleLogicMsg_WreckedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_WreckedMsg>(pMessage);

				if(pMsg->mWreckeeIdx != mVehicleIndex)
				{
					// If not us that has been wrecked
					// Did we wreck the car that is now wrecked?
					if(pMsg->mWreckerIdx == mVehicleIndex)
					{
						CheckVendetta(pMsg->mWreckeeIdx);

						blackboard.mNumCarsTakenOutThisRace++; 
						blackboard.mNumCarsInWreckStreak++;
						CheckPunisher();
						CheckRampage();

						// Was the vehicle airborne when wrecked?
						CheckSkeetShooter(pMsg->mWreckeeIdx); 

						// Tally up some wreckage for us
						CheckWreckingBall();

						// We have wrecked somebody today! 
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DEMOLITIONS_EXPERT);

						// Did we wreck this guy by ramming him?
						CheckBruteForce(pMsg->mWreckeeIdx);

						// Have we wrecked X vehicles in s_mQuickDrawTimeoutInSeconds seconds
						CheckQuickDraw();
					}
				}
				break;
			}

		case Game::E_VehicleMessage_Overtake:
			{
				//Bug 24261 - If we're in destruction mode we don't want to be getting overtaking stats
				if(m_pRaceBookLayerPtr->GetRaceMode() == Game::E_RaceMode_Multiplayer_Destruction)
				{
					break;
				}

				const ::Game::Actors::C_VehicleOvertakeMsg* pMsg = Messaging::MessageCast< ::Game::Actors::C_VehicleOvertakeMsg >(pMessage);
				//Check that its us who's overtaken 
				if(pMsg->mOvertakerVehicleIndex == mVehicleIndex)
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_OVERTAKE);
					UpdateUltimateComebackStatus(); 

					//Check invisible with silent runner
					if(m_pLogicActor && m_pLogicActor->IsCloaked())
					{
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_STEALTH);
					}
				}
				else if(pMsg->mOvertakeeVehicleIndex == mVehicleIndex)
				{
					UpdateUltimateComebackStatus(); 
				}
				break;
			}
		case E_VehicleLogicMsg_ChainCompleted:
			{
				const C_VehicleLogicMsg_ChainCompleted* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_ChainCompleted >(pMessage);
				CheckChainReaction(pMsg->mChainLength);
				break;
			}

		case Game::E_GameLogicMessage_LapIncremented:
			{
				//See if the vehicle is crossing the line for the last time
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast< Game::C_LapIncrementedMsg >(pMessage);
				bool completedFinalLap = false; 
				// [mtaylor] #37260 - Added vehicle index check because other vehicles going across the line may trigger this
				if( pMsg->mVehicleIndex == mVehicleIndex && pMsg->mLap == (int)m_pLevelState->mLaps+1)
				{
					completedFinalLap = true;
				}

				if(completedFinalLap)
				{
					// Update Any other 'completedFinalLap'  challenges here
					CheckUntouchable(); 
				}

				break;
			}
	
		case Game::E_GameLogicMessage_MastermineSpawnedMinis:
			{
				// Give some progress for spawning some evil mastermine babies!
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASTERMINE);
				break;
			}
		
		case E_VehicleLogicMsg_Scored:
			{
				const C_VehicleLogicMsg_Scored* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_Scored >(pMessage);
				//Check that this scoring event relates to us
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					CheckBaseMoves(pMsg->mBaseMove, pMsg->mSkillMoves);
					CheckSkillMoves(pMsg->mBaseMove, pMsg->mSkillMoves);
					CheckBonusMoves(pMsg->mBonusMoves);

					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_STYLE);
				}
				break;
			}
		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);

				// Now that this vehicle has been damaged by a perk, we know a subsequent wreck was not caused by damage from us ramming them
				blackboard.mHasRammedVehicle = blackboard.mHasRammedVehicle & ( ~(1<<msg->mVehicleIndex) );

				// If we fired this perk at someone else
				if( msg->mVehicleIndex != mVehicleIndex && msg->mFiringVehicleIndex == mVehicleIndex )
				{
					//	No challenge progress for hitting a teammate
					if(Game::VehiclesAreTeammates(msg->mFiringVehicleIndex, msg->mVehicleIndex))
						break;

					blackboard.mSuccessfulPowerupHits++; 
					
					//We fired this perk
					if(blackboard.mLastVehicleIndexHitByPerk == msg->mVehicleIndex)
					{
						// Increment successiveNumtimes hit last vehicle; 
						blackboard.mNumSuccessivePerkHitsOnLastVehicleHit++; 

						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_AGGRESSIVE_PERSONALITY);
					}
					else
					{
						// Hit new vehicle (i.e. hit streak broken)
						blackboard.mLastVehicleIndexHitByPerk = msg->mVehicleIndex; 

						// Reset Vendetta progress for new streak (starts at One since this hit began new streak)
						blackboard.mNumSuccessivePerkHitsOnLastVehicleHit = 1; 
						
					}

					// If we fired a double barge
					if(msg->mPickupType == E_PICKUP_TYPE_BARGE)
					{
						// Was this a last gasp?
						if( msg->mPickupSubType == E_PICKUP_SUB_TYPE_MOD )
						{
							blackboard.mNumVehiclesHitWithLastGaspThisLife++; 
						}
						else
						{
							// regular barge

							// Keep a count of how many vehicles it hits
							blackboard.mNumVehiclesHitByOneBarge++;

							//C heck that we have hit X vehicles with a double barge
							CheckCauseAndEffect();	
						}
					}
					
					CheckPickupUseChallenges(msg->mPickupType, msg->mPickupSubType);

					// Was the pickup a reverse fire pickup
					if(msg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED ||
					   msg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP)
					{
						CheckReverseFireChallenges(msg->mPickupType);

						if(msg->mPickupType == E_PICKUP_TYPE_BLINDFIRE)
						{
							if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE))
							{
								//Update fired bolts information
								UpdateChosenOneBlindfires();

								Btl::List<BlindfireInformation>::Iterator blindfireIter = m_RecentBlindfiresFired.Begin();
								//Now see if the latest pickup is still valid and change its id if so to mark it as a success
								while(blindfireIter != m_RecentBlindfiresFired.End())
								{
									if(blindfireIter->id == msg->mUniqueID)
									{
										//Magical value that implies the blindfire has hit. Need to check if this is safe!!!!!
										blindfireIter->id = (U32)(INTPTR)Maths::Constants<U32>::Max;	// [Blur2PC][x64] retail compares against the address of Max(); keep its low 32 bits
										break;
									}
									++blindfireIter;
								}

								//Check progress
								CheckChosenOne();
							}
						}
					}
				}
				else if(msg->mVehicleIndex == mVehicleIndex) // If we have been hit
				{
					if(msg->mPickupType == E_PICKUP_TYPE_REPAIR)
					{
						blackboard.mPerksUsed.SetBit(E_PICKUP_USED_REPAIR);
					}
				}
				break;
			}
		case Game::E_PickupMessage_PickupPickedUp:
			{
				const Game::Actors::PickupPickedUpMsg* pMsg = Messaging::MessageCast< Game::Actors::PickupPickedUpMsg >(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HOARDER);
					CheckSidewinder(); 
				}
					
				break;
			}
		case Game::E_GameLogicMessage_GameplayImpact:
			{
				const Game::C_GameplayImpactMsg* pMsg = Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);

				// Were we the aggressive one?
				if(pMsg->mVehicleIdx == mVehicleIndex && pMsg->mAggressiveVehicleIdx == mVehicleIndex && pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX)
				{
					// Also, let's record that we've just rammed that guy so if he gets wrecked, we know we wrecked him/her by ramming
					if(pMsg->mImpactLevel > E_IMPACT_LEVEL_NONE)
					{
						blackboard.mHasRammedVehicle = blackboard.mHasRammedVehicle | (1<<pMsg->mOtherVehicleIdx);
					}

					if( pMsg->mImpactLevel == E_IMPACT_LEVEL_MEDIUM) // When we send out scoring moves medium impact == 'light hit'
					{
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_LIGHT_WEIGHT);
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FACE_PLANT);  // face plant progress for medium OR heavy hits
					}
					else if(pMsg->mImpactLevel == E_IMPACT_LEVEL_HEAVY) // When we send out scoring moves heavy impact == 'heavy hit'
					{
						//If this was a significant impact then increment our challenge
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUISER);
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FACE_PLANT);   // face plant progress for medium OR heavy hits
					}
					
				}
				else
				{
					if(pMsg->mImpactLevel > E_IMPACT_LEVEL_NONE)
					{
						// If this vehicle has taken damage from something that wasn't us ramming it, then we cannot have wrecked it by ramming!
						blackboard.mHasRammedVehicle = blackboard.mHasRammedVehicle & ( ~(1<<pMsg->mOtherVehicleIdx) );
					}
				}
				break;
			}
		case E_VehicleLogicMsg_AggressiveScore:
			{
				const C_VehicleLogicMsg_AggressiveScore* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AggressiveScore>(pMessage);
				HandleAggressiveScoreMessage(pMsg); 
				break;
			}
		case E_VehicleLogicMsg_ShieldedPerk:
			{
				const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
				//Check this is for us (it always should be but you never *quite* know)
				if( pMsg->mDefendingVehicleIndex == mVehicleIndex && 
					pMsg->mFiringVehicleIndex != mVehicleIndex ) // Dont give progress for shielding own pickups (e.g mines etc)
			{
					if(pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
					{
						// Increment challenge progress
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINESWEEPER);
					}

					if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHIELD))
					{
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHIELD);
					}
				}

				break;
			}
		case E_VehicleLogicMsg_DestroyedPickupWithPickup:
			{
				const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_DestroyedPickupWithPickup>(pMessage);
				HandleDestroyedPickupWithPickup(pMsg); 
				break;
			}
		case Game::E_VehicleMessage_FatallyDamaged:
			{
				//Reset the wreck streak
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);
				if(pMsg->m_VehicleIndex == mVehicleIndex)
				{
					//Reset the wreck streak
					blackboard.mNumCarsInWreckStreak = 0;
				}
				break;
			}
		case E_VehicleLogicMsg_ModActivated:
			{
				const C_VehicleLogicMsg_ModActivated* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ModActivated>(pMessage);
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					CheckModActivatedChallenges(pMsg->mModActivated);
				}
			}
			break;

		case E_VehicleLogicMsg_PowerUpDeleted:
			{
				const C_VehicleLogicMsg_PowerUpDeleted* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_PowerUpDeleted>(pMessage);
				//Check blindfires against cached unique IDs
				if(pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE)
				{
					if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE))
					{
						Btl::List<BlindfireInformation>::Iterator blindfireIter = m_RecentBlindfiresFired.Begin();
						while(blindfireIter != m_RecentBlindfiresFired.End())
						{
							if(pMsg->mUniqueId == blindfireIter->id)
							{
								blindfireIter->timeDestroyed = m_pRLC->GetRaceTime();
							}
							++blindfireIter;
						}

						UpdateChosenOneBlindfires();
					}
				}
				break;
			}
		case Game::E_VehicleMessage_ResetAfterWrecked:
			{
				const Game::Actors::C_VehicleResetAfterWreckedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAfterWreckedMsg>(pMessage);

				//Check that its us who's been wrecked
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					// At this point we are safe to increment challenge progress (may add a delay) for last gasp, as the user can safely see it again. 
					if(blackboard.mNumVehiclesHitWithLastGaspThisLife > 0)
					{
						// Set delayed increment progress timer
						mLastGaspChallengeDelayTimer = 1.0f;
					}
				}
				return true;
			}
		}

		return true;
	}

	void C_VehicleComponent_MP_Challenges::CheckModActivatedChallenges(const UI::E_MultiplayerVehicleAbilities modActivated)
	{
		switch(modActivated)
		{
		case UI::E_MultiplayerVehicleAbilities_Scrambler:
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SCRAMBLER);
			}
			break;

		case UI::E_MultiplayerVehicleAbilities_Battering_Ram: 
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BATTERING_RAM);
			}
			break;

		case UI::E_MultiplayerVehicleAbilities_Safety_Net: 
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SAFETY_NET);
			}
			break;

		case UI::E_MultiplayerVehicleAbilities_Road_Sweeper: 
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_ROAD_SWEEPER);
			}
			break;

		case UI::E_MultiplayerVehicleAbilities_Drifter: 
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DRIFTER);
			}
			break;
		default:
			break;
		}
	}

	void C_VehicleComponent_MP_Challenges::HandleDestroyedPickupWithPickup(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg)
	{
		// Did we destroy the pickup , and did it belong to somebody else?
		if( pMsg->mPickupKillerOwnerID == mVehicleIndex && 
		    pMsg->mPickupVictimOwnerID != mVehicleIndex )
		{
			CheckForValidMinesweeper(pMsg);

			// Take this check outside this 'if' block if you want a player to be able to do this challenge by hitting their own mines
			CheckMasterExploder(pMsg);
		}

		// Rear guard can occur when we both do / do not own the 'killer' projectile/pickup
		CheckForValidRearGuard(pMsg);
	}

	void C_VehicleComponent_MP_Challenges::UpdatePerkFiredData(const C_VehicleLogicMsg_FiredPerkMsg* pMsg)
	{
		Game::MiscellaneousChallengeBlackboardData& blackboard = m_pMultiplayerProfile->GetChallengeBlackboard();

		// Was the pickup a forward firing pickup?
		if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_NORMAL ||
		   pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
		{
			blackboard.mForwardPickupFireUsed = true; 
		}
		else
		{
			blackboard.mReversePickupFireUsed = true; 
		}

		switch(pMsg->mPickupType)
		{
			//have we just used a double nitro?
			case E_PICKUP_TYPE_NITROUS:
			{
				// Is this an airbrake?
				if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED ||
				   pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP)
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_AIRBRAKE);
				}
				blackboard.mPerksUsed.SetBit(E_PICKUP_USED_NITRO);
				break;
			}

			case E_PICKUP_TYPE_BARGE:
			{
				// dont count last gasp barges
				if( pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_MOD )
				{
					blackboard.mPerksUsed.SetBit(E_PICKUP_USED_BARGE);

					//If we fired a barge we want to re-set the number of vehicles hit by a barge
					blackboard.mNumVehiclesHitByOneBarge = 0;
				}
			
				break;
			}

			case E_PICKUP_TYPE_SHOCK:
			{
				blackboard.mPerksUsed.SetBit(E_PICKUP_USED_SHOCK);
				break;
			}

			case E_PICKUP_TYPE_SHUNT:
			{
				blackboard.mPerksUsed.SetBit(E_PICKUP_USED_SHUNT);
				break;
			}
			case E_PICKUP_TYPE_BOMB:
			{
				blackboard.mPerksUsed.SetBit(E_PICKUP_USED_BOMB);

				// If we fired a mine.. we store that mine ID on the blackboard. If we destroy an enemy perk with this mine
				// within a short time period, that counts as 'blocking' an enemy with that mine. 
				blackboard.mRecentlyFiredMineIDs.PushBack(Btl::Pair<U32,float>(pMsg->mUniqueId,0.0f));
				break;
			}
			case E_PICKUP_TYPE_SHIELD:
			{
				blackboard.mPerksUsed.SetBit(E_PICKUP_USED_SHIELD);
				break;
			}

			case E_PICKUP_TYPE_BLINDFIRE:
			{
				blackboard.mPerksUsed.SetBit(E_PICKUP_USED_BLINDFIRE);

				//Cache reverse blindfire IDs
				if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED && ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE))
				{
					// If we were travelling at the qualifying speed.. then push this one back..
					Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
					if(pLogicActor)
					{
						float currentSpeed = pLogicActor->GetPositionInfo().mSpeedInMph;
						S32 qualifyingSpeed = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE).GetData();
						if(currentSpeed > qualifyingSpeed)
						{
							BlindfireInformation blindfire;
							blindfire.id = pMsg->mUniqueId;
							m_RecentBlindfiresFired.PushBack( blindfire );
						}
						else
						{
							// If not.. then this one doesnt count.. and the chain is broken!
							m_RecentBlindfiresFired.Clear();
						}
					}
				}
				break;
			}
		}

	}

	void C_VehicleComponent_MP_Challenges::CheckRivalry( const U8 wreckedVehicleIndex )
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
		BIZ_ASSERT_MSG(pLogicActor, "C_VehicleComponent_MP_Challenges::UpdateUltimateComebackStatus() > LogicActor is null");
		if( pLogicActor )
		{
			if( wreckedVehicleIndex == pLogicActor->GetScoring().mRivalIndex )
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RIVALRY);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckForValidMinesweeper(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg)
	{
		bool validMineDestruction = true; 
		// Was the pickup we killed a mine
		if(pMsg->mPickupVictimType != E_PICKUP_TYPE_BOMB)
		{
			validMineDestruction = false; 
		}

		// Did we kill it with a shunt, blindfire or barge?
		if(pMsg->mPickupKillerType != E_PICKUP_TYPE_BLINDFIRE &&
			pMsg->mPickupKillerType != E_PICKUP_TYPE_SHUNT &&
			pMsg->mPickupKillerType != E_PICKUP_TYPE_BARGE )
		{
			validMineDestruction = false; 	
		}

		if(validMineDestruction)
		{
			// we validly destroyed a mine.. give some minesweeper progress. 
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINESWEEPER);
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckMasterExploder(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg)
	{
		// Has a mine destroyed a mine?
		if(pMsg->mPickupVictimType == E_PICKUP_TYPE_BOMB && pMsg->mPickupKillerType == E_PICKUP_TYPE_BOMB)
		{
			//note that a check when this function is called means the player will not be able to
			//increment this challenge by destroying their own mines with their own mines
			IncreaseChallengeProgress( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASTER_EXPLODER );
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckForValidRearGuard(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg)
	{
		// Rear Guard occurs when..

		// 1) A mine blocks a shunt/ blindfire
		// 2) A REVERSE shunt/BlindFire blocks a shunt/Blindfire
		// 3) Barge Neutralises a shunt / blindfire
		if(CheckRearGuardMineBlocksProjectile(pMsg) ||
		   CheckRearGuardReverseProjectileBlocksProjectile(pMsg) ||
		   CheckRearGuardBargeNeutralisesProjectile(pMsg))
		{
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_REAR_GUARD);
		}
	}

	bool C_VehicleComponent_MP_Challenges::CheckRearGuardMineBlocksProjectile(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg) const
	{
		// Mines are always victims! (since they go boom). 
		// Check that our mine was hit, and by somebody ELSES pickup

		if(pMsg->mPickupVictimOwnerID == mVehicleIndex &&
		   pMsg->mPickupKillerOwnerID != mVehicleIndex && 
		   pMsg->mPickupVictimType == E_PICKUP_TYPE_BOMB) 
		{
			// Should have a record of when we fired this mine. If find it, then return true
			Btl::Vector< Btl::Pair<U32,float> >* recentlyFiredMineIDs =  &m_pMultiplayerProfile->GetChallengeBlackboard().mRecentlyFiredMineIDs;
			if(!recentlyFiredMineIDs->Empty())
			{
				for( Btl::Vector< Btl::Pair<U32,float> >::ConstIterator iter = recentlyFiredMineIDs->Begin(); iter != recentlyFiredMineIDs->End(); iter++ )
				{
					if(iter->first == pMsg->mPickupVictimActorID )
					{
						// Valid
						return true;
					}
				}
			}
		}

		// not valid
		return false;
	}

	bool C_VehicleComponent_MP_Challenges::CheckRearGuardReverseProjectileBlocksProjectile(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg) const
	{
		// Check that WE killed somebody ELSES pickup
		if( pMsg->mPickupKillerOwnerID == mVehicleIndex && 
		    pMsg->mPickupVictimOwnerID != mVehicleIndex )
		{
			// If the pickups are of the correct types
			if((pMsg->mPickupKillerType == E_PICKUP_TYPE_SHUNT ||
			   pMsg->mPickupKillerType == E_PICKUP_TYPE_BLINDFIRE) &&
			   (pMsg->mPickupVictimType == E_PICKUP_TYPE_SHUNT ||
			   pMsg->mPickupVictimType == E_PICKUP_TYPE_BLINDFIRE))
			{
				// Check that we used a reverse 
				if( pMsg->mPickupKillerSubType == E_PICKUP_SUB_TYPE_REVERSED ||
					pMsg->mPickupKillerSubType == E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP)
				{
					// Then valid!
					return true; 
				}
			}
		}

		// not valid
		return false;
	}

	bool C_VehicleComponent_MP_Challenges::CheckRearGuardBargeNeutralisesProjectile(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg) const
	{
		// Check that WE killed somebody ELSES pickup
		if( pMsg->mPickupKillerOwnerID == mVehicleIndex && 
		    pMsg->mPickupVictimOwnerID != mVehicleIndex )
		{
			// If we killed an Enemy shunt / BlindFire with a barge
			if(pMsg->mPickupKillerType == E_PICKUP_TYPE_BARGE && 
				pMsg->mPickupKillerSubType != E_PICKUP_SUB_TYPE_MOD  &&  // dont count last gasp barges
			   (pMsg->mPickupVictimType == E_PICKUP_TYPE_SHUNT ||
			   pMsg->mPickupVictimType == E_PICKUP_TYPE_BLINDFIRE))
			{
				// Valid read guard!
				return true; 
			}
		}

		// not valid
		return false;
	}

	void C_VehicleComponent_MP_Challenges::UpdateRecentlyFiredMineIDs(float timeStep)
	{
		Game::MiscellaneousChallengeBlackboardData& blackboard = m_pMultiplayerProfile->GetChallengeBlackboard();

		if(!blackboard.mRecentlyFiredMineIDs.Empty())
		{
			// For each mine ID we have recently fired
			Btl::Vector< Btl::Pair<U32,float> >::Iterator iter = blackboard.mRecentlyFiredMineIDs.Begin();
			while( iter != blackboard.mRecentlyFiredMineIDs.End() )
			{
				// Update its time stamp, 
				iter->second += timeStep; 

				// and remove if no longer considered 'recent'
				if(iter->second >= g_TimeMinesConsideredRecentlyDroppedInSeconds)
				{
					// erase this entry
					iter = blackboard.mRecentlyFiredMineIDs.Erase(iter);
				}
				else
				{
					// next
					iter++; 
				}
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckReverseFireChallenges(E_PICKUP_TYPE type)
	{
		switch(type)
		{
			case E_PICKUP_TYPE_SHUNT:
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_SHUNT); 
					break;
				}
			case E_PICKUP_TYPE_BLINDFIRE:
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_BLIND_FIRE); 
					break;
				}
			case E_PICKUP_TYPE_BOMB:
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_LOBBER); 
					break;
				}
			case E_PICKUP_TYPE_NITROUS:
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO_RIFT); 
					break;
				}
			default:
			break;
		}
	}

	void C_VehicleComponent_MP_Challenges::UpdateChosenOneBlindfires()
	{
		//Check times first - mark where to delete upto
		S8 deletionPoint = -1;
		int i = 0;
		Btl::List<BlindfireInformation>::Iterator blindfireIter = m_RecentBlindfiresFired.Begin();
		while(blindfireIter != m_RecentBlindfiresFired.End())
		{
			if(blindfireIter->id != (U32)(INTPTR)Maths::Constants<U32>::Max && blindfireIter->timeDestroyed >= 0.001f && m_pRLC->GetRaceTime() - blindfireIter->timeDestroyed > 1.0f)
			{
				deletionPoint = i;
			}
			i++;
			++blindfireIter;
		}

		//Then delete them all (As they have been invalidated by the latest miss)
		for(int i = 0; i <= deletionPoint; i++)
		{
			m_RecentBlindfiresFired.PopFront();
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckChosenOne()
	{
		//Check progress - have we hit with enough successive blindfires in a row?
		if(m_RecentBlindfiresFired.Size() >= m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE).GetTargetAmount())
		{
			U8 chosenOneProgress = 0;
			bool iAmTheChosenOne = false;
			Btl::List<BlindfireInformation>::Iterator blindfireIter = m_RecentBlindfiresFired.Begin();
			while(blindfireIter != m_RecentBlindfiresFired.End())
			{
				if(blindfireIter->id == (U32)(INTPTR)Maths::Constants<U32>::Max)	// [Blur2PC][x64] retail compares against the address of Max(); keep its low 32 bits
				{
					chosenOneProgress++;
					if(chosenOneProgress >= m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE).GetTargetAmount())
					{
						iAmTheChosenOne = true;
						break;
					}
				}
				else
				{
					chosenOneProgress = 0;
				}
				++blindfireIter;
			}

			if(iAmTheChosenOne)
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE, 
					m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE).GetTargetAmount());
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::HandleAggressiveScoreMessage(const C_VehicleLogicMsg_AggressiveScore* pMsg)
	{
		Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();

		if(pMsg->mVehicleIndex == mVehicleIndex)  
		{
			switch(pMsg->mBonusScoringEvent)
			{
			case E_BLUR_SCORE_MOVE_BONUS_CLIFF: //Drop down
			case E_BLUR_SCORE_MOVE_BONUS_DROWN:
				{
					if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTAL_DELUXE))
					{
						//If we pushed them off
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTAL_DELUXE);
					}
				}
				break;

			default:
				break;
			}

			if(pMsg->mBase == E_BLUR_SCORE_MOVE_BASE_HIT_DECOY)
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DECEIVER);
			}

		}// end if msg veh index == mVehicleIndex
	}

	void C_VehicleComponent_MP_Challenges::CheckPostRaceChallenges()
	{
		// Did the owner of this component win?
		bool won = ( m_pLevelResult->mCars[mVehicleIndex].mPosition == 1 );

		// What is the player's vehicle ID
		IncrementCarChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES);
	
		IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PARTICIPANT);
		
		if(won)
		{
			IncrementCarChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER);
		
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HARD_DAYS_WORK);
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK);

			//Did we win without using any pickups?
			CheckPacifist();

			// Did we make an epic comeback
			CheckUltimateComeback(); 

			//Did we win by 30+ seconds?
			CheckTearaway();
		}
		else
		{
			//We lost a race so reset the hat trick
			ResetChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK);
		}

		// Check this regardless of win/lose
		CheckShowOff();
		CheckCarnage();
		CheckCarCrazy();
		CheckPiedPiper();

		CheckMineSpecialist(); 
		CheckBargeSpecialist();
		CheckAfterburner();
		CheckBlindfireSpecialist();

		CheckMasochist();	 // Dont use a shield during the race

		CheckThickSkin();	 // Dont use a repair during the race

		CheckHeadInTheClouds();	//Time in air. Store it all until the end so we minimize loss of time due to rounding.
		
		CheckDistance();	 // Work out how many miles we've travelled hiring

		CheckBackwardsThinking(); // only use backwards pickups during a race

		
	}

	// Check for successful use of a pickup on somebody else
	void C_VehicleComponent_MP_Challenges::CheckPickupUseChallenges( const enum E_PICKUP_TYPE ePickupType, const E_PICKUP_SUB_TYPE eSubType )
	{
		enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType = UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE;

		if(eSubType == E_PICKUP_SUB_TYPE_DECOY)
		{
			// we are done here!
			return;
		}

		switch(ePickupType)
		{
		case E_PICKUP_TYPE_BOMB:
			{
			
				eChallengeType = UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE;
				break;
			}
		case E_PICKUP_TYPE_SHUNT:
			{
				eChallengeType = UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT;
				break;
			}
		case E_PICKUP_TYPE_SHOCK:
			{
				eChallengeType = UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK;
				break;
			}
		case E_PICKUP_TYPE_BARGE:
			{
				if( eSubType != E_PICKUP_SUB_TYPE_MOD )
				{
					eChallengeType = UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE;
				}
				break;
			}
		case E_PICKUP_TYPE_BLINDFIRE:
			{
				eChallengeType = UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE;
				break;
			}
		}
		

		//See if we care about this pickup type
		if(eChallengeType != UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE)
		{
			//Now we've mapped our challenge type
			if(ChallengeCurrentlyCompletable(eChallengeType))
			{
				IncreaseChallengeProgress(eChallengeType);
			}
		}

	}

	void C_VehicleComponent_MP_Challenges::CheckSidewinder()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SIDEWINDER))
		{
			if(m_pScoring->GetIsVehicleDrifting())
			{
				// Increment progress
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SIDEWINDER);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckUltimateComeback()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_ULTIMATE_COMEBACK))
		{
			// Have we finished in first, after being first then last?
			if(m_pMultiplayerProfile->GetChallengeBlackboard().mUltimateComebackStatus == Game::MiscellaneousChallengeBlackboardData::E_UltimateComeback_ThenFellToLastPlace)
			{
				// Increment progress
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_ULTIMATE_COMEBACK);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::UpdateUltimateComebackStatus()
	{
		// Get the vehicle's current race position
		Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
		BIZ_ASSERT_MSG(pLogicActor, "C_VehicleComponent_MP_Challenges::UpdateUltimateComebackStatus() > LogicActor is null");

		// Get race position
		S8 myRacePosition = pLogicActor->GetPositionInfo().mRacePosition; 

		Game::MiscellaneousChallengeBlackboardData& blackboard = m_pMultiplayerProfile->GetChallengeBlackboard();

		// Interested in checking different things in different states...
		switch (blackboard.mUltimateComebackStatus)
		{
		case Game::MiscellaneousChallengeBlackboardData::E_UltimateComeback_NotYetInFirstPlace:
			{
				// If the driver IS now in first place... :)
				if(myRacePosition == 1)
				{
					// Transition to next state
					blackboard.mUltimateComebackStatus = Game::MiscellaneousChallengeBlackboardData::E_UltimateComeback_HasBeenInFirstPlace;
				}
			}
			break;
		case Game::MiscellaneousChallengeBlackboardData::E_UltimateComeback_HasBeenInFirstPlace:
			{
				// If the driver has now fallen to last place ... :(
				int numCarsRemaining = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetNumLiveVehicles();
				if(myRacePosition == numCarsRemaining )
				{
					// Transition to next state
					blackboard.mUltimateComebackStatus = Game::MiscellaneousChallengeBlackboardData::E_UltimateComeback_ThenFellToLastPlace; 
				}
			}
			break;

		default:
		break;
		}

	}

	bool C_VehicleComponent_MP_Challenges::GetFinishedLast(const int vehicleIndex)
	{
		Game::C_MpResults& Results = Game::C_MpResults::Instance();

		if(Results.GetPlayerPosition(vehicleIndex) == 1)
		{
			return false; 
		}

		// How many cars didnt finish?
		unsigned int numCarsNotFinished =  0; // i.e. wrecked / eliminated etc
		for(unsigned int i = 0; i < m_pLevelState->mNumCars; i++)
		{
			if( Results.m_SortedVehicleResults[i].m_State == Game::E_VEHICLE_STATE_FATALLY_DAMAGED )
			{
				numCarsNotFinished++; 
			}
		}

		int numCarsFinished = m_pLevelState->mNumCars - numCarsNotFinished; 

		// Of the cars that did finish (i.e. not wrecked or eliminated), was the vehicle index last?
		return (Results.GetPlayerPosition(vehicleIndex) == numCarsFinished );
	}

	void C_VehicleComponent_MP_Challenges::CheckHighSlider(float driftDistance)
	{
		if( ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HIGH_SLIDER) )
		{
			// If our drift is a longer distance than the target (gnarlier?) then challenge complete!
			S32 targetAmount = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HIGH_SLIDER).GetTargetAmount();
			if( driftDistance >= targetAmount )
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HIGH_SLIDER, targetAmount );
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckShowOff()	
	{
		// If place 3rd or better in a race then 'Show' challenge has been achieved 
		const static int MIN_POSITION_REQUIRED_FOR_SHOW_OFF = 3;

		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOW_OFF))
		{
			Game::C_MpResults& Results = Game::C_MpResults::Instance();

			if(Results.GetPlayerPosition(mVehicleIndex) <= MIN_POSITION_REQUIRED_FOR_SHOW_OFF)
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOW_OFF);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckVendetta(U16 wreckeeIdx)
	{
		// Hardcoded hitcount for vendetta progress check
		const static int NUM_TIMES_TO_WRECK_CAR_FOR_VENDETTA = 2; 

		//Increment the new wreckee by one
		m_pMultiplayerProfile->GetChallengeBlackboard().mTimesPlayerWrecked[wreckeeIdx]++;

		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_VENDETTA))
		{
			// Have we achieved the required number of successive hits?
			if(m_pMultiplayerProfile->GetChallengeBlackboard().mTimesPlayerWrecked[wreckeeIdx] >=  NUM_TIMES_TO_WRECK_CAR_FOR_VENDETTA)
			{
				// Increment progress
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_VENDETTA);
			}
		}
	}

	void  C_VehicleComponent_MP_Challenges::CheckWreckingBall()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_WRECKING_BALL))
		{
			// If this has been called, then we can inc the num cars we have wrecked in this race
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_WRECKING_BALL);
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckPunisher()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PUNISHER))
		{
			//If enough cars have been wrecked
			Game::C_MultiplayerChallenge& challenge = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PUNISHER);
			int target = challenge.GetTargetAmount(); 
			int amount = challenge.GetAmount();

			if(m_pMultiplayerProfile->GetChallengeBlackboard().mNumCarsTakenOutThisRace >= target)
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PUNISHER, (target-amount));
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckSkeetShooter(const int victimVehicleIndex)
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SKEET_SHOOTER))
		{
			// Get the victim actor
			Game::Actors::C_VehicleLogicActor *pLActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(victimVehicleIndex);
			BIZ_ASSERT_MSG(pLActor,"C_VehicleComponent_MP_Challenges::CheckSkeetShooter(const int victimVehicleIndex) < logic Actor Pointer obtained from given vehicle index was invalid");
			if(pLActor)
			{
				Game::Actors::C_VehicleActor *pActor =  pLActor->GetGraphicalActor(victimVehicleIndex);
				BIZ_ASSERT_MSG(pActor,"C_VehicleComponent_MP_Challenges::CheckSkeetShooter(const int victimVehicleIndex) < Vehicle Actor Pointer obtained from given vehicle index was invalid");
				if(pActor)
				{
					RacingPhysics::Car* physics = pActor->GetVehiclePhysics();
					BIZ_ASSERT_MSG(physics,"C_VehicleComponent_MP_Challenges::CheckSkeetShooter(const int victimVehicleIndex) < RacingPhysics::Car* physics invalid");
					if(physics)
					{
						const int NUM_WHEELS = 4; 
						BIZ_ASSERT(physics->GetNumWheels() <= NUM_WHEELS);

						bool victimOnGround = false;
						for(int i = 0; i < NUM_WHEELS; i++)
						{
							if(physics->WheelOnGround(i))
							{
								// Set flag and end check
								victimOnGround = true;
								i = NUM_WHEELS; 
							}
						}

						if(!victimOnGround)
						{
							// If this has been called, then we have wreckd a car whilst it was in mid air
							IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SKEET_SHOOTER);
						}
					}
				}
			}
		}

	}

	void C_VehicleComponent_MP_Challenges::CheckTearaway()
	{
		//Check if we've completed this challenge already, if we're 100% complete we don't need to go further
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_TEARAWAY))
		{
			if(m_pLevelState->mRaceMode != Game::E_RaceMode_Multiplayer_Racing || m_pLevelState->mNumCars < 2)
			{
				return;
			}

			Game::C_MultiplayerChallenge& challenge = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_TEARAWAY);
			float target = (float)challenge.GetTargetAmount();
			F32 theirTime = Game::C_MpResults::Instance().m_SortedVehicleResults[1].m_Statistics[UI::E_MultiplayerStat_RaceTime];
			F32 myTime = Game::C_MpResults::Instance().m_SortedVehicleResults[0].m_Statistics[UI::E_MultiplayerStat_RaceTime];
			
			if(Game::C_MpResults::Instance().m_SortedVehicleResults[1].m_State != Game::E_VEHICLE_STATE_LEFT_RACE && theirTime - myTime >= target) //If second place didn't leave and we beat them by X+
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_TEARAWAY, (S32)target);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckUntouchable()
	{
		//Check if we've completed this challenge already, if we're 100% complete we don't need to go further
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_UNTOUCHABLE))
		{
			if(m_pLogicActor->GetStatistics().mDamageTaken <= 0.01f)
			{
				// Then havent taken damage!
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_UNTOUCHABLE);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckBruteForce(const int wreckeeIndex)
	{
		//Check if we've completed this challenge already, if we're 100% complete we don't need to go further
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTE_FORCE))
		{
			// Since we check in other areas for other possible sources of damage to this car, if we still have "just rammed" this guy
			// and he's now wrecked, we know it was our ram that did the wrecking
			if( m_pMultiplayerProfile->GetChallengeBlackboard().mHasRammedVehicle & (1<<wreckeeIndex) )
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTE_FORCE);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckCarnage()
	{
		//Check if we've completed this challenge already, if we're 100% complete we don't need to go further
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CARNAGE))
		{
			int ourDamageDealt = m_pLogicActor->GetStatistics().mDamageDealt;
			Game::C_MpResults& Results = Game::C_MpResults::Instance();
			Game::NetVehicleData data;
			for(unsigned int i = 0; i < m_pLevelState->mNumCars; i++)
			{
				if(mVehicleIndex != i) // Don't check if we've done more damage than ourselves
				{
					Results.GetPlayerResults(i, data);
					if(data.m_Statistics[UI::E_MultiplayerStat_DamageDealt] >= ourDamageDealt)
					{
						return; //has another vehicle dealt more damage than us? early out if so
					}
				}
			}

			//it would be too easy if you could get this after everyone had quit out!
			if( m_pLevelState->mNumCars > 1 )
			{
				// We have not encountered an early out! Then we have done the most damage!
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CARNAGE);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckRetribution()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RETRIBUTION))
		{
			const Game::C_MultiplayerChallenge& challenge = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RETRIBUTION);
			int target = challenge.GetTargetAmount(); 
			int currentAmount = challenge.GetAmount();

			m_pMultiplayerProfile->GetChallengeBlackboard().mNumRevengeAttacksThisRace++;

			if(m_pMultiplayerProfile->GetChallengeBlackboard().mNumRevengeAttacksThisRace >= target)
			{
				//One revenge attack performed
				// [JBunner]BUG: 37523: 
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RETRIBUTION,target - currentAmount);
			}
		}
	}

	//Skill moves that don't rely on a base move
	void C_VehicleComponent_MP_Challenges::CheckSkillMoves(E_BLUR_SCORE_MOVE_BASE base, const Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>& skills)
	{
		for(Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>::ConstIterator it = skills.Begin(); it != skills.End(); it++)
		{
			switch(*it)
			{
				case E_BLUR_SCORE_MOVE_SKILL_LONG_SHOT:
				{
					if(base == E_BLUR_SCORE_MOVE_BASE_BOLT )
					{
						IncreaseChallengeProgress( UI::E_MULTIPLAYER_CHALLENGE_TYPE_ELF_EYES );
					}
					break;
				}
			}
		}
	}

	//Call this 
	void C_VehicleComponent_MP_Challenges::CheckBaseMoves(E_BLUR_SCORE_MOVE_BASE base, const Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>& skills)
	{
		Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
		Game::C_MultiplayerChallengeSystem& challengeSystem = m_pMultiplayerProfile->GetChallengeSystem();

		switch(base)
		{
			case E_BLUR_SCORE_MOVE_BASE_NOTHING:
				break;

			case E_BLUR_SCORE_MOVE_BASE_JUMPING:
				{
					CheckExtremelyExtreme(); // We may have just landed a jump that was extremely extreme!

					// Was this a nitro jump?
					if( SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_2) )
					{
						blackBoard.mNumNitroJumps++;

						// If in a race
						if(m_pRaceBookLayerPtr && m_pRaceBookLayerPtr->GetRaceMode() != Game::E_RaceMode_Multiplayer_Destruction)
						{
							CheckJumper(); 
						}
					}
				}
				break;

			case E_BLUR_SCORE_MOVE_BASE_OVERTAKE:
			{
				
				if( SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_1) ||
					SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_2) ||
					SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_3) ||
					SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_4) ||
					SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_5))
				{
					blackBoard.mNitrousOvertakes++;
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO);
				}
			}
			break;

			case E_BLUR_SCORE_MOVE_BASE_DRIFTING:
			{
				CheckHighSlider(m_pScoring->GetDriftDistance());
			}
			break;

			case E_BLUR_SCORE_MOVE_BASE_EVASION:
			{
				if( SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_1) )
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK_EVADER);
				}
				else if( SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_LEVEL_2 ) )
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT_EVADER);
				}
			}
			break;

			case E_BLUR_SCORE_MOVE_BASE_RAMMING:
			{
				if( SkillContainerContainsSkill(skills, E_BLUR_SCORE_MOVE_SKILL_NITROUS) )
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO_RAM);
				}
			}
			break;
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckJumper()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_JUMPER))
		{
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();

			// Check the amount we need
			const Game::C_MultiplayerChallenge& challenge = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_JUMPER);
			S32 ChallengeAmount = challenge.GetTargetAmount();
			S32 currentAmount = challenge.GetAmount();
			
			// If we have enough to pass
			if(blackBoard.mNumNitroJumps >= ChallengeAmount)
			{
				// Since this is a one off, we increase by the full amount we need to pass.
				// [JBunner]BUG: 37523: 
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_JUMPER, ChallengeAmount - currentAmount);
			}
		}
	}

	bool  C_VehicleComponent_MP_Challenges::SkillContainerContainsSkill(const Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>& skills, const E_BLUR_SCORE_MOVE_SKILL desiredSkill) const
	{
		if(skills.Empty())
		{
			return false;
		}

		for(int i = 0; i < skills.Size(); i++)
		{
			if(skills[i] == desiredSkill)
			{
				return true;
			}
		}
		return false;
	}

	void C_VehicleComponent_MP_Challenges::CheckBonusMoves(const Btl::Vector<E_BLUR_SCORE_MOVE_BONUS>& bonus)
	{
		for(int i = 0; i < bonus.Size(); i++)
		{
			switch(bonus[i])
			{
			case E_BLUR_SCORE_MOVE_BONUS_NONE:
				break;

			case E_BLUR_SCORE_MOVE_BONUS_REVENGE:
				{
					CheckRetribution();
				}
				break;

			case E_BLUR_SCORE_MOVE_BONUS_FULLY_BOLTED:
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FULLY_BOLT);
				}
				break;
			case E_BLUR_SCORE_MOVE_BONUS_BULLYING:
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BULLY);
				}
				break;
			}
		}
	}
	
	void C_VehicleComponent_MP_Challenges::IncrementCarChallengeProgress(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType)
	{
		m_pMultiplayerProfile->GetChallengeSystem().IncreaseCarChallengeAmount(mCarID, eChallengeType);
	}
	

	void C_VehicleComponent_MP_Challenges::IncreaseChallengeProgress(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType, const S32 sAmount )
	{
		//Make sure we've not already completed the challenge
		if(ChallengeCurrentlyCompletable(eChallengeType)) // Just check if unlocked here (dont care if 'complete' at this stage)
		{
			//Increment the challenge
			m_pMultiplayerProfile->GetChallengeSystem().IncreaseChallengeAmount(eChallengeType, sAmount);
		}
	}

	void C_VehicleComponent_MP_Challenges::ResetChallengeProgress(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType)
	{
		//Make sure we've not already completed the challenge
		if(ChallengeCurrentlyCompletable(eChallengeType)) // Just check if unlocked here (dont care if 'complete' at this stage)
		{
			//Reset challenge
			m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(eChallengeType).Reset();
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckPacifist()
	{
		//Make sure we've not already completed the challenge
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST))
		{
			//Check if the race is a valid type, and we haven't used a pickup 
			if( m_pLevelState->mUiRaceType != UI::E_RaceType_Hardcore && !m_pMultiplayerProfile->GetChallengeBlackboard().mPerksUsed.CountBitsSet() == 0)
			{
				//No...still a pacifist
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckMineSpecialist() // only use mines
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_SPECIALIST))
		{
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			if(blackBoard.mPerksUsed.CountBitsSet() == 1 && blackBoard.mPerksUsed.GetBitValue(E_PICKUP_USED_BOMB))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_SPECIALIST);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckBargeSpecialist() // only use barges
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE_SPECIALIST))
		{
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			if(blackBoard.mPerksUsed.CountBitsSet() == 1 && blackBoard.mPerksUsed.GetBitValue(E_PICKUP_USED_BARGE))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE_SPECIALIST);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckAfterburner() // only use nitros
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_AFTERBURNER))
		{
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			if(blackBoard.mPerksUsed.CountBitsSet() == 1 && blackBoard.mPerksUsed.GetBitValue(E_PICKUP_USED_NITRO))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_AFTERBURNER);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckBlindfireSpecialist()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE_SPECIALIST))
		{
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			if(blackBoard.mPerksUsed.CountBitsSet() == 1 && blackBoard.mPerksUsed.GetBitValue(E_PICKUP_USED_BLINDFIRE))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE_SPECIALIST);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckCauseAndEffect() // // Hit X cars with one double barge
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSE_AND_EFFECT))
		{
			Game::MiscellaneousChallengeBlackboardData& BlackBoard =m_pMultiplayerProfile->GetChallengeBlackboard();

			// Check the amount we need
			const Game::C_MultiplayerChallenge& challenge = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSE_AND_EFFECT);
			S32 ChallengeAmount = challenge.GetTargetAmount();
			S32 currentAmount = challenge.GetAmount();
			
			
			// If we have enough to pass
			if(BlackBoard.mNumVehiclesHitByOneBarge >= ChallengeAmount)
			{
				// Since this is a one off, we increase by the full amount we need to pass.
				// [JBunner]BUG: 37523: 
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSE_AND_EFFECT, ChallengeAmount - currentAmount);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckFTL()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FTL))
		{
			Game::MiscellaneousChallengeBlackboardData& BlackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			
			// Check the amount of time we need
			S32 ChallengeAmount = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FTL).GetTargetAmount();
			
			// If we have enough to pass
			if(BlackBoard.mLongestTimeTravelingAtXMPH >= (float)s_mTimeToAchieveFTL)
			{
				// Since this is a one off, we increase by the full amount we need to pass.
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FTL, ChallengeAmount);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckExtremelyExtreme()
	{

		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_EXTREMELY_EXTREME))
		{
			if(m_pLogicActor)
			{
				// Get the vehicle stats
				VehicleRaceStatistics& stats = m_pLogicActor->mStatistics;

				// Check the amount of time we need
				S32 ChallengeAmount = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_EXTREMELY_EXTREME).GetTargetAmount();
				
				// If we have enough to pass
				if(stats.mLongestDistanceTraveledInJump >= ChallengeAmount)
				{
					// Since this is a one off, we increase by the full amount we need to pass.
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_EXTREMELY_EXTREME, ChallengeAmount);
				}
			}
		}

	}

	void C_VehicleComponent_MP_Challenges::CheckMasochist() // don't use any shields
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASOCHIST))
		{
			if(!m_pMultiplayerProfile->GetChallengeBlackboard().mPerksUsed.GetBitValue(E_PICKUP_USED_SHIELD))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASOCHIST);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckThickSkin() // Dont use repairs
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_THICK_SKIN))
		{
			if(!m_pMultiplayerProfile->GetChallengeBlackboard().mPerksUsed.GetBitValue(E_PICKUP_USED_REPAIR))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_THICK_SKIN);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckRampage() //Wreck X cars in a race without being wrecked yourself
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE))
		{
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			Game::C_MultiplayerChallengeSystem& challengeSystem = m_pMultiplayerProfile->GetChallengeSystem();

			int target = challengeSystem.GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE).GetTargetAmount();
			int currentAmount = challengeSystem.GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE).GetAmount();

			if(blackBoard.mNumCarsInWreckStreak >= target)
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE, target - currentAmount);
			}
		}
	}


	void C_VehicleComponent_MP_Challenges::CheckHeadInTheClouds()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS))
		{
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS, (S32)m_pMultiplayerProfile->GetChallengeBlackboard().mTimeSpentInAir);
		}
	}

	void C_VehicleComponent_MP_Challenges::UpdateDominator()
	{
		Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(mVehicleIndex);

		Game::MiscellaneousChallengeBlackboardData& BlackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();

		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR))
		{
			// If the player is in first position
			if(pVehicleLogicActor->GetPositionInfo().mRacePosition == 1)
			{
				// Store this for the next frame
				BlackBoard.mPreviouslyInFirstPosition = true;

				// Add the frame time onto the running total
				BlackBoard.mCurrentTimeInFirstPosition +=  C_GameApp::GetInstance()->GetTimeStep();	
			}
			else
			{
				// If we are no longer in first but we were in the last frame
				if(BlackBoard.mPreviouslyInFirstPosition)
				{
					// Re-set the fact we were in first position last frame
					BlackBoard.mPreviouslyInFirstPosition = false;

					//Update the running total
					UpdateDominatorRunningTotal();					
				}				
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckChainReaction(const int comboLength)
	{
		const Game::C_MultiplayerChallenge& challenge  = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHAIN_REACTION );

		if( challenge.GetTargetAmount() <= comboLength )
		{
			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHAIN_REACTION,challenge.GetTargetAmount());
		}
		
	}

	void C_VehicleComponent_MP_Challenges::UpdateCasualNexus()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSAL_NEXUS))
		{
			const Game::C_MultiplayerChallenge& challenge  = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSAL_NEXUS );
		
			if( CheckFansInTimePeriod( challenge.GetTargetAmount(), (F32)challenge.GetData() ))
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSAL_NEXUS, challenge.GetTargetAmount() );
			}
		}

	}

	bool C_VehicleComponent_MP_Challenges::CheckFansInTimePeriod( S32 fansRequired, F32 timePeriod )
	{
		BIZ_ASSERT(m_pScoring);
		const Components::ScoreEventHistory& scoringHistory = m_pScoring->GetScoringHistory();
		S32 fansGainedInTimePeriod;
		S32 fansRequiredInTimePeriod;
		F32 currentTimeStamp;
		F32 timePeriodInSeconds = timePeriod;
		
		
		fansGainedInTimePeriod = 0;
		fansRequiredInTimePeriod = fansRequired;
		currentTimeStamp = C_GameApp::GetInstance()->GetElapsedRaceTime();

		for(int i=0; i<scoringHistory.GetScoringEventCount(); i++)
		{
			const Components::ScoreEventHistory::ScoreEvent& sEvent = scoringHistory.GetScoringEventAtIndex(i);
			F32 timeDiff = currentTimeStamp - sEvent.mTimeStamp;
			if( timeDiff >= 0.0f && timeDiff < timePeriodInSeconds )
			{
				fansGainedInTimePeriod += sEvent.mScore;
			}
			else
			{
				// Exceeded our desired time period so there is no point in continuing.
				break;
			}
		}

		return fansGainedInTimePeriod >= fansRequiredInTimePeriod;

	}

	void C_VehicleComponent_MP_Challenges::UpdateDominatorRunningTotal()
	{
		Game::MiscellaneousChallengeBlackboardData& BlackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();

		//Add the current time to the seconds left over from last time we were in first
		S32 timeToAdd = S32(BlackBoard.mCurrentTimeInFirstPosition + BlackBoard.mSecondsRemainderInFirstPosition);

		// Set the current remainder of seconds
		BlackBoard.mSecondsRemainderInFirstPosition = (float)(timeToAdd % 60);

		// Convert time to whole mins only
		timeToAdd = (timeToAdd/60);

		// Add this onto our running total
		IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR, timeToAdd);

		//Re-Set the Current Amount of time in first position
		BlackBoard.mCurrentTimeInFirstPosition = 0.0f;
	}

	void C_VehicleComponent_MP_Challenges::CheckCarCrazy() // Use X different cars in a day
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY))
		{
			int index = 0;
			C_GameVehicleData::Instance().mVehicles.AlphaSort(); // We cannot guarantee the order of the data being read in, we must use mSortedData
			Ut::Vector<int> sortedIndexes = C_GameVehicleData::Instance().mVehicles.mSortedData;

			for(int i=0; i<C_GameVehicleData::Instance().mVehicles.Number(); i++)
			{
				if(m_pLevelState->mCars[mVehicleIndex].mCarID == (U32)(C_GameVehicleData::Instance().mVehicles[sortedIndexes[i]]->ID) )
				{
					index = i;
					break;
				}
			}

			// First let's check if we've used a car we haven't used already this day
			if( !( m_pMultiplayerProfile->GetChallengeBlackboard().mVehiclesUsedToday & ((U64)1<<index) ) )
			{
				// We haven't used this car already, increment progress
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY);
			}

			// Now, let's set our current car to "used"
			m_pMultiplayerProfile->GetChallengeBlackboard().mVehiclesUsedToday |= (U64)1 << index;
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckPiedPiper()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER))
		{
			// Must be a 20 player race
			if(m_pMultiplayerProfile->GetChallengeBlackboard().mNumStartingPlayers == 20)
			{
				bool hasMoreFansThanAllOthers = true;
				Game::C_MpResults& Results = Game::C_MpResults::Instance();
				Game::NetVehicleData data;

				// Populate our fans
				Results.GetPlayerResults( mVehicleIndex, data );
				U32 ourFans = (U32)data.m_Statistics[UI::E_MultiplayerStat_Fans];

				// Check against all other fan scores
				for(int i=0; i<20; i++)
				{
					Results.GetPlayerResults( (U8)i, data );
					if(mVehicleIndex != i)
					{
						if(data.m_Statistics[UI::E_MultiplayerStat_Fans] > ourFans)
						{
							// Someone has more fans than us! Challenge not complete
							hasMoreFansThanAllOthers = false;
						}
					}
				}

				if(hasMoreFansThanAllOthers)
				{
					IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER);
				}
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::UpdateWellTraveled()
	{

		//If we are tracking this Challenge
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_WELL_TRAVELLED))
		{
			// Get the current route
			AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_Game::GetInstance()->GetLevelState().mRoute];
		
			Game::MiscellaneousChallengeBlackboardData& BlackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();

			const char* countryCodes[] = { "JPN", "USA", "UK", "SPA", };
			Game::MiscellaneousChallengeBlackboardData::CountriesToVisit countries[] = { Game::MiscellaneousChallengeBlackboardData::JAPAN, Game::MiscellaneousChallengeBlackboardData::USA, 
																						 Game::MiscellaneousChallengeBlackboardData::UK, Game::MiscellaneousChallengeBlackboardData::SPAIN,
																						};

			const int NumCountries = 4;

			for(int i = 0; i < NumCountries; i++)
			{
				//Check The string for the country code
				int numLettersInCountryCode = i == 2 ? 2 : 3;
				if(strncmp(pRoute->tag.string, countryCodes[i], numLettersInCountryCode) == 0)
				{
					// If we have already visited This Country today
					if(BlackBoard.mCountriesVisitedTodayBitField & countries[i])
					{
						// Dont need to do anything
					}
					else
					{
						BlackBoard.mCountriesVisitedTodayBitField = (BlackBoard.mCountriesVisitedTodayBitField | countries[i]);
						// Add this onto our running total
						IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_WELL_TRAVELLED, 1);
					}
				}

			}
		}

	}

	// ---------------------------------------------------------------------------------------------------------------------------------------

	void C_VehicleComponent_MP_Challenges::CheckQuickDraw() // Wreck x cars in s_mQuickDrawTimeoutInSeconds seconds
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_QUICK_DRAW))
		{
			//Check the time since the last wreck
			Game::MiscellaneousChallengeBlackboardData& blackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();
			const float currentRaceTime = m_pRLC->GetRaceTime();

			const int targetTime = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_QUICK_DRAW).GetTargetAmount();
			//Check the timeout
			if( (currentRaceTime - blackBoard.mQuickDrawStartRaceTime) > targetTime )
			{
				//Reset progress
				blackBoard.mQuickDrawNumberOfVehiclesWrecked = 0;
				blackBoard.mQuickDrawStartRaceTime = currentRaceTime;
			}

			//Increment the number of vehicles wrecked in this quick draw
			blackBoard.mQuickDrawNumberOfVehiclesWrecked++;
			
			//Check to see if we've slaughtered enough vehicles to satisfy the gods of challenge-ness
			if( blackBoard.mQuickDrawNumberOfVehiclesWrecked >= s_mQuickDrawCarsToWreck )
			{
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_QUICK_DRAW, targetTime);
			}
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckDistance()
	{
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE))
		{
			//We're not bothered about being especially accurate with this but we'll round to the nearest mile because I'm a 
			//little anal but not really all that anal...
			

			float roundedDistanceTravelledInMetres = m_pLogicActor->GetStatistics().mDistanceTravelled + (0.5f * s_mMetresInAMile);
			int noMilesTravelledThisRace = (int)(roundedDistanceTravelledInMetres / s_mMetresInAMile);

			IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE, noMilesTravelledThisRace);
		}
	}

	void C_VehicleComponent_MP_Challenges::CheckBackwardsThinking()
	{
		//Make sure we've not already completed the challenge
		if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BACKWARDS_THINKING))
		{
			//Check if we've used a forward pickup 
			if(!m_pMultiplayerProfile->GetChallengeBlackboard().mForwardPickupFireUsed && 
				m_pMultiplayerProfile->GetChallengeBlackboard().mReversePickupFireUsed)
			{
				//No...still a pacifist
				IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_BACKWARDS_THINKING);
			}
		}

	}

	void C_VehicleComponent_MP_Challenges::IncrementLastGaspChallenge(const int numVehiclesHitWithLastGaspThisLife)
	{	
		IncreaseChallengeProgress(UI::E_MULTIPLAYER_CHALLENGE_TYPE_LAST_GASP, numVehiclesHitWithLastGaspThisLife);
	}

	void C_VehicleComponent_MP_Challenges::CheckForDelayedLastGaspChallengeProgress()
	{
		// We need to delay the completion of a last gasp challenge.. so that it doesnt stomp all over the wrecekd screen
		if(mLastGaspChallengeDelayTimer > 0.0f)
		{
			mLastGaspChallengeDelayTimer -= C_GameApp::GetInstance()->GetTimeStep();
			if(mLastGaspChallengeDelayTimer <= 0.0f)
			{
				// Increment challenge progress now
				IncrementLastGaspChallenge(m_pMultiplayerProfile->GetChallengeBlackboard().mNumVehiclesHitWithLastGaspThisLife); 
			
				mLastGaspChallengeDelayTimer = 0.0f;

				// Reset counter for this new life
				m_pMultiplayerProfile->GetChallengeBlackboard().mNumVehiclesHitWithLastGaspThisLife = 0; 
			}

		}
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_MP_Challenges::OnMessage( const Messaging::C_MessageBase *pMessage )
	{
		PERF_EVENT("C_VehicleComponent.MultiplayerChallenges.MessageHandler");

		S32 msgType = pMessage->Type();

		// Handle the "control" cases first
		switch(msgType)
		{
		case Game::E_GameflowMessage_RaceStarted:
			{
				m_pMultiplayerProfile->GetChallengeSystem().GetTracker().Clear();
				m_pMultiplayerProfile->GetChallengeBlackboard().mNumStartingPlayers = m_pLevelState->mNumCars;

				m_pScoring = static_cast<const C_VehicleComponent_NewScoring*>(((Game::Actors::C_VehicleLogicActor*)GetActor())->QueryInterface(E_IF_VehicleComponent_NewScoring));
				UpdateWellTraveled();

				mEnabled = true;

				// May Need to check if we are in first place at this point
				if(m_pRaceBookLayerPtr->GetRaceMode() != Game::E_RaceMode_Multiplayer_Destruction)
				{
					UpdateUltimateComebackStatus(); 
				}
				
				break;
			}

		case Game::E_GameflowMessage_RaceFinished:
			{
				mEnabled = false;
				if(ChallengeCurrentlyCompletable(UI::E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR))
				{
					UpdateDominatorRunningTotal();
				}
				break;
			}
		case Game::E_VehicleMessage_FatallyDamaged:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_QuitLevel:
		case Game::E_GameflowMessage_VehicleFinished:
		case E_VehicleLogicMsg_Skillshot:
		case E_VehicleLogicMsg_Wrecked:
		case E_VehicleLogicMsg_Scored:
		case E_VehicleLogicMsg_FiredPerk:
		case Game::E_VehicleMessage_Overtake:
		case Game::E_GameLogicMessage_LapIncremented:
		case E_VehicleLogicMsg_HitByPerk:
		case E_VehicleLogicMsg_PerkEnded:
		case E_VehicleLogicMsg_ShieldedPerk:
		case Game::E_PickupMessage_PickupPickedUp:
		case Game::E_GameLogicMessage_GameplayImpact:
		case E_VehicleLogicMsg_AggressiveScore:
		case E_VehicleLogicMsg_Landed:
		case E_VehicleLogicMsg_DestroyedPickupWithPickup:
		case E_VehicleLogicMsg_ModActivated:
		case E_VehicleLogicMsg_PowerUpDeleted:
		case E_VehicleLogicMsg_ChainCompleted:
		case Game::E_GameLogicMessage_MastermineSpawnedMinis:
		case Game::E_VehicleMessage_ResetAfterWrecked:
			{
				if(mEnabled)
				{
					return CheckChallenges( pMessage );
				}
				break;
			}

		case GameMessaging::E_GameMsg_Update:
			{	
				if(mEnabled)
				{
					Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
					Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(mVehicleIndex);

					float speed = pVehicleLogicActor->GetPositionInfo().mSpeedInMph;

					S32 TargetSpeed = m_pMultiplayerProfile->GetChallengeSystem().GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_FTL).GetTargetAmount();

					Game::MiscellaneousChallengeBlackboardData& BlackBoard = m_pMultiplayerProfile->GetChallengeBlackboard();

					if(speed >= TargetSpeed)
					{
						BlackBoard.mCurrentTimeTravelingAtXMPH +=  C_GameApp::GetInstance()->GetTimeStep();
						if( BlackBoard.mCurrentTimeTravelingAtXMPH > BlackBoard.mLongestTimeTravelingAtXMPH)
						{
							BlackBoard.mLongestTimeTravelingAtXMPH = BlackBoard.mCurrentTimeTravelingAtXMPH;
							CheckFTL();
						}
					}
					else
					{
						BlackBoard.mCurrentTimeTravelingAtXMPH = 0.0f;
					}

					UpdateDominator();
					UpdateCasualNexus();
					UpdateRecentlyFiredMineIDs(C_GameApp::GetInstance()->GetTimeStep()); 

					CheckForDelayedLastGaspChallengeProgress(); 
				}
				
				//DEBUG DEBUG DEBUG
#ifndef CFG_GOLD
				UpdateDebugValues();
#endif //CFG_GOLD
				//DEBUG
				break;
			}

			
		}

		if(!mEnabled)
			return true;


		return true;
	}

} // namespace Components
