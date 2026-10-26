#include "stdafx.h"

#include "VLC_Statistics.h"

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/RaceLogicCore/Data/RLVehicleData.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <GameLogic\StickerManager.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <ai/AI_Amax.h>

#include "Automation\GameDataLogging.h"

#include <ui/UiMessages.h>
#include <ui/UiInterface.h>
#include <Ui\SoundEffects.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MultiplayerChallenges.h>
#include "Game\Game.h"

Debug::Float g_VehicleOvertakeTimer("Vehicle.Components.Statistics.VehicleOvertakeTimer", 2.f, Debug::Range(0.f, 5.f), Debug::Step(0.1f));
Debug::Float g_EndOfLapPercent("Vehicle.Components.Statistics.EndOfLapPercent", 0.95f, Debug::Range(0.f, 1.f), Debug::Step(0.05f));
#ifndef CFG_GOLD
extern Debug::Float g_RecentlyHitByPerkTime;
#endif

Debug::Bool g_debug_Motormash_GiveRivalWreck("Multiplayer.MotorMash.GiveRivalWreck", false); 
Debug::Bool g_debug_Motormash_GiveWreck("Multiplayer.MotorMash.GiveWreck", false);

Debug::Logger &g_StatisticsLogger( Debug::GetLogger("Vehicle.Component.Statistics") );

const int firedPerkPositionsGainedTime = 5;

namespace Components
{

	bool C_VehicleComponent_Statistics::CreateImpl()
	{
		const I_VehicleComponent_Statistics_CreateInfo* pCreation_data = static_cast<const I_VehicleComponent_Statistics_CreateInfo *>(GetCreator().GetData());
		BIZ_ASSERT_MSG(pCreation_data, "C_VehicleComponent_Statistics::CreateImpl() > Couldn't get a pointer to the creation data, watch the fireworks!! :-S");

		//Cache stuff from the creation data
		mVehicleIndex = pCreation_data->mVehicleIndex;
		mRaceLogicLayerPtr = pCreation_data->mpRaceLogicLayer;
		m_pStatistics = pCreation_data->mpStatistics;
		BIZ_ASSERT_MSG(m_pStatistics, "C_VehicleComponent_Statistics::CreateImpl() > Couldn't get a pointer to the statistics info, KAPPOOWWWW!!!!");

		//AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( ::Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( ::Game::E_GameflowMessage_VehicleFinished );
		AddSubscriber( ::Game::E_GameflowMessage_ResultsCalculated );
		AddSubscriber( ::Game::E_VehicleMessage_Eliminated );
		AddSubscriber( ::Game::E_VehicleMessage_Overtake );
		AddSubscriber( ::Game::E_GameLogicMessage_GameplayImpact );
		AddSubscriber( ::Game::E_VehicleMessage_FatallyDamaged );
		AddSubscriber( ::Game::E_GameLogicMessage_LapIncremented );
		AddSubscriber( ::Game::E_GameLogicMessage_Failed_Rule );
		AddSubscriber( ::Game::E_GameLogicMessage_Passed_All_Rule );
		AddSubscriber( ::Game::E_PickupMessage_PickupPickedUp );
		AddSubscriber( ::Game::E_PickupMessage_CollectStopwatch );	//35844 - stopwatches not being added to stats
		AddSubscriber( E_VehicleLogicMsg_FiredPerk );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
		AddSubscriber( E_VehicleLogicMsg_AggressiveScore );
		AddSubscriber( ::Game::E_GameflowMessage_QuitAndWinRace ) ;
		AddSubscriber( ::Game::E_GameflowMessage_RaceFinished );
		AddSubscriber( ::Game::E_GameflowMessage_QuitRace );
		AddSubscriber( ::Game::E_GameflowMessage_QuitLevel );

		AddSubscriber( E_VehicleLogicMsg_VehicleMurdered );
		AddSubscriber( E_VehicleLogicMsg_VehicleSuicided );
		AddSubscriber( E_VehicleLogicMsg_DecoyPickupDropOnTrack );

		::Game::Actors::C_VehicleLogicActor* vehActor		= ((::Game::Actors::C_VehicleLogicActor*)GetActor());

		vehActor->mStatistics.Clear();

		mLastOvertakeTime = mNitroTimer = 0.f;

		mRacePositionWhenFired  = mNoOvertaken = mLastFiredID = 0;
#ifndef CFG_GOLD
		mFiredPerkRecentlyTimer = -1;
#endif
		mEnabled = mNitroDouble = false;

		// Cache scoring component
		Components::C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
		if(pScoring)
		{
			m_pScoring = pScoring;
		}
		else
		{
			m_pScoring = NULL; 
		}
		BIZ_ASSERT_MSG(m_pScoring,"C_VehicleComponent_Statistics::CreateImpl() < could not cache pointer to scoring component");

		return true;
	}

	void C_VehicleComponent_Statistics::Update()
	{
		if(!mEnabled)
			return;

#ifndef CFG_GOLD
		UpdateDebugValues(); 
#endif // #ifndef CFG_GOLD

		PERF_EVENT("C_VehicleComponent.Statistics.Update");

		::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());
		float frameTime								= C_GameApp::GetInstance()->GetTimeStep();

		if(vehActor->mPositionInfo.mSpeedInMph > vehActor->mStatistics.mFastestSpeedInMPH)
			vehActor->mStatistics.mFastestSpeedInMPH = vehActor->mPositionInfo.mSpeedInMph;

		if(vehActor->mStatistics.mIsDriftingWithScore)
		{
			mWasJustDrifting = true;
		}
		else if(!vehActor->mStatistics.mIsDriftingWithScore && mWasJustDrifting)
		{
			mWasJustDrifting = false;
			vehActor->mStatistics.mNumDrifts++;
		}

		//if(vehicleData.mDriftDistance > vehActor->mStatistics.mLongestDrift)
		//	vehActor->mStatistics.mLongestDrift = vehicleData.mDriftDistance;

		//if(vehicleData.mAirDistance > vehActor->mStatistics.mLongestJump)
		//	vehActor->mStatistics.mLongestJump = vehicleData.mAirDistance;

		if(mNitroTimer > 0)
			mNitroTimer -= frameTime;
#ifndef CFG_GOLD
   		if(mFiredPerkRecentlyTimer != -1)
		{
			if( mFiredPerkRecentlyTimer > g_RecentlyHitByPerkTime )
			{
				AmaxDataLogging::LogPlacesGainedFromPerk(mVehicleIndex, mRacePositionWhenFired, vehActor->mPositionInfo.mRacePosition, 
														 mRaceLogicLayerPtr->ConvertPickupTypeToString(mLastFiredPerk));
				mFiredPerkRecentlyTimer = -1;
			}
			else
				mFiredPerkRecentlyTimer += frameTime;
		}
#endif
		vehActor->mStatistics.mTotalRaceTime = vehActor->GetRLC()->GetRaceTime();
	}

#ifndef CFG_GOLD
	void C_VehicleComponent_Statistics::UpdateDebugValues()
	{
		if(g_debug_Motormash_GiveRivalWreck)
		{
			Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
			if(pLogicActor)
			{
				// NOTE: Test this debug functionality... may not work with new set rival index interface
				// Set our rival index to ourselves, 
				m_pScoring->HandleRivalDestroyed(); 

				// simulate wrecking ourselves
				int baseScore = 0;
				int bonusPoints = 0;
				HandleMotorMashWreckScoreCalculation(pLogicActor, true,baseScore,bonusPoints);
			}
			g_debug_Motormash_GiveRivalWreck = false;	
		}

		if(g_debug_Motormash_GiveWreck)
		{
			Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
			if(pLogicActor)
			{
				int baseScore = 0;
				int bonusPoints = 0;
				HandleMotorMashWreckScoreCalculation(pLogicActor, false,baseScore,bonusPoints);
			}
			g_debug_Motormash_GiveWreck = false;
		}
	}
#endif // #ifndef CFG_GOLD

	void C_VehicleComponent_Statistics::SetVehicleOvertakeeStatistics(int racePos)
	{
		::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());
		::Game::C_LevelState& levelState				= ::Game::C_Game::GetInstance()->GetLevelState();

		if( vehActor->mLapsInfo.mCurrentLap == levelState.mLaps && vehActor->mPositionInfo.mLogicPosition > g_EndOfLapPercent )
		{
			vehActor->mStatistics.mTookFirstPlaceAtEndOfRace = false;

			if( racePos == 2 )
				vehActor->mStatistics.mLostFirstPlaceAtEndOfRace = false;
		}
	}

	void C_VehicleComponent_Statistics::HandleMyDeath(const bool suicided)
	{
		::Game::Actors::C_VehicleLogicActor* pLogicActor = ((::Game::Actors::C_VehicleLogicActor*)GetActor());

		pLogicActor->mStatistics.mNumTimesWrecked++;
		CheckWreckRaceTime();
		
		//Streaks
		pLogicActor->mStatistics.mWreckStreak = 0;
		pLogicActor->mStatistics.mLosingStreak++;
		if(pLogicActor->mStatistics.mLosingStreak > pLogicActor->mStatistics.mWorstLosingStreak)
		{
			pLogicActor->mStatistics.mWorstLosingStreak = pLogicActor->mStatistics.mLosingStreak;
		}

		if(suicided)
		{
			pLogicActor->mStatistics.mNumTimesSuicided++;

			::Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
			{
				UI::C_UiScoringEventPerk uimsg1( UI::C_TextStringID("MP_SUICIDE").String(), 0, -1, -1, mVehicleIndex );
				_UI_MESSAGE_FULL( uimsg1 );
			}
		}
	}

	void C_VehicleComponent_Statistics::HandleSomeoneElsesDeath(U8 aboutVehicleIndex)
	{
		::Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		::Game::Actors::C_VehicleLogicActor* pLogicActor = ((::Game::Actors::C_VehicleLogicActor*)GetActor());

		if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		{
			UI::C_WideString128 strBonus;
			strBonus.Clear();

			bool wasRival = aboutVehicleIndex == m_pScoring->GetRivalIndex();

			// Lets work out what our base score was, and wether we got any bonus points
			int baseScore = 0;
			int bonusScore= 0;
			HandleMotorMashWreckScoreCalculation(pLogicActor,wasRival,baseScore,bonusScore);

			if(levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck)->GetActive())
			{
				if(wasRival)
				{
					strBonus += UI::C_TextStringID("MP_RIVALVAMPIRICWRECK").String(); 
				}
				else
				{
					strBonus += UI::C_TextStringID("SCR_NEW_AMAX_SCORING_VAMPIRIC_WRECKED").String();
				}
			}
			else
			{
				if(wasRival)
				{
					strBonus += UI::C_TextStringID("MP_RIVAL_WRECK").String(); 
				}
				else
				{
					strBonus += UI::C_TextStringID("SCR_NEW_AMAX_SCORING_WRECKING_SHOT").String();
				}
			}

			// Wrecks are ALWAYS 'bonus' text on the display now, so show ALL points as a bonus
			UI::C_UiScoringEventChain uimsg2(strBonus.Chars(), baseScore + bonusScore, mVehicleIndex);
			_UI_MESSAGE_FULL( uimsg2 );

			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashWreckedActionIn );

			if( wasRival )
			{
				// When we wreck our rival, that person is no longer our rival 
				Game::C_StickerManager::Instance().CheckWreckYourRival(mVehicleIndex);
				Components::C_VehicleComponent_MP_Challenges* pChallenges = (Components::C_VehicleComponent_MP_Challenges*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges);
				if(pChallenges)
				{
					pChallenges->CheckRivalry(aboutVehicleIndex);
				}

				m_pScoring->HandleRivalDestroyed(); 
			}
		}
		else
		{
			pLogicActor->mStatistics.mNumVehiclesWrecked++;
		}

		//Streaks
		pLogicActor->mStatistics.mWreckStreak++;
		if(pLogicActor->mStatistics.mWreckStreak > pLogicActor->mStatistics.mBestWreckStreak)
		{
			pLogicActor->mStatistics.mBestWreckStreak = pLogicActor->mStatistics.mWreckStreak;
		}
		pLogicActor->mStatistics.mLosingStreak = 0;
	}

	void C_VehicleComponent_Statistics::HandleMotorMashWreckScoreCalculation(Game::Actors::C_VehicleLogicActor* pLogicActor,const bool wasRivalAttack, int& basePoints, int& bonusPoints)
	{
		MotorMashScoringValues* pScoringValues =	&C_MultiplayerData::Instance().m_pConfig->m_MotormashScoringValues;
		basePoints = 0;
		bonusPoints = 0;

		// Add on our wreck score
		basePoints += pScoringValues->m_WreckScore;

		if(wasRivalAttack)
		{
			// Apply multiplier to 'base score' ONLY - let scoring component do this work for us. 
			int totalPoints = m_pScoring->ApplyRivalScoreMultiplier(basePoints);
			bonusPoints = totalPoints - basePoints; // work out how many points were added
			pLogicActor->mStatistics.mNumRivalVehiclesWrecked++;	
		}
		else
		{
			pLogicActor->mStatistics.mNumVehiclesWrecked++;
		}

	}

	void C_VehicleComponent_Statistics::SetVehicleOvertakerStatistics(int racePos, int otherVehIdx)
	{
		::Game::Actors::RaceLogicCore* pRLC			= ((::Game::Actors::C_VehicleLogicActor*)GetActor())->GetRLC();
		float currentTime							= pRLC->GetRaceTime();
		::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());
		::Game::C_LevelState& levelState				= ::Game::C_Game::GetInstance()->GetLevelState();

		if( vehActor->mLapsInfo.mCurrentLap == levelState.mLaps && vehActor->mPositionInfo.mLogicPosition > g_EndOfLapPercent && 
			vehActor->mPositionInfo.mRacePosition == 1 )
		{
			vehActor->mStatistics.mOvertookAtEndOfRace = true;

			if( racePos == 1 )
			{
				vehActor->mStatistics.mTookFirstPlaceAtEndOfRace = true;
				vehActor->mStatistics.mLostFirstPlaceAtEndOfRace = false;
			}
		}

		float timeSinceLastOvertake = currentTime - mLastOvertakeTime;

		if(timeSinceLastOvertake < g_VehicleOvertakeTimer)
		{
			if(++mNoOvertaken > vehActor->mStatistics.mMaxPlacesTakenInASingleManoeuvre)
				vehActor->mStatistics.mMaxPlacesTakenInASingleManoeuvre = mNoOvertaken;
		}
		else
		{
			mNoOvertaken = 1;

			if(!vehActor->mStatistics.mMaxPlacesTakenInASingleManoeuvre)
				vehActor->mStatistics.mMaxPlacesTakenInASingleManoeuvre = mNoOvertaken;
		}	

		mLastOvertakeTime = currentTime;

		if( mNitroTimer > 0 )
		{
			if(otherVehIdx < vehActor->mStatistics.mDualVehicleStats.Size())
			{
				if(mNitroDouble)
					vehActor->mStatistics.mDualVehicleStats[otherVehIdx].mNumTimesOvertookWithDoubleNitro++;
				else
					vehActor->mStatistics.mDualVehicleStats[otherVehIdx].mNumTimesOvertookWithNitro++;
			}
			else
				LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
		}
	}

	void C_VehicleComponent_Statistics::SetRaceCompletedStatistics()
	{
		::Game::Actors::C_VehicleLogicActor* vehActor			= ((::Game::Actors::C_VehicleLogicActor*)GetActor());
		::Game::Actors::C_VehicleLogicActor* otherVehActor	= NULL;

		CheckWreckRaceTime();
		vehActor->mStatistics.mEndOfRaceHealth = (float)vehActor->GetCurrentHealth();
		vehActor->mStatistics.mDistanceTravelled = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(mVehicleIndex);

		for(int a = 0; a < mRaceLogicLayerPtr->GetNumVehicles(); a++)
		{
			otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(a);

			if(!otherVehActor)
			{
				LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
				continue;

			}

			float distance = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(vehActor->mPositionInfo.mLogicPosition, otherVehActor->mPositionInfo.mLogicPosition);

			if(a < vehActor->mStatistics.mDualVehicleStats.Size())
				vehActor->mStatistics.mDualVehicleStats[a].mFinishedVehicleDistances++;
			else
				LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
		}

		::Game::C_LevelResult& results = ::Game::C_Game::GetInstance()->GetLevelResult();
		results.mCars[mVehicleIndex].mStatistics	= vehActor->mStatistics;
		results.mCars[mVehicleIndex].mPosition		= vehActor->mPositionInfo.mRacePosition;
		results.mCars[mVehicleIndex].mScore			= vehActor->mScoringInfo.mFinalScore;
		
		results.mCars[mVehicleIndex].mStatistics.mLongestChain	= vehActor->mScoringInfo.mLongestChain;
		results.mCars[mVehicleIndex].mStatistics.mShowyFlourishFans = vehActor->mScoringInfo.mShowyFlourishFans;

		for(int a=0;a<AMAX_SCORING_EVENT_MAX;a++)
		{
			results.mCars[mVehicleIndex].mIndividualScoringStore[a]=vehActor->mScoringInfo.mIndividualScoring[a];
		}

		// We need to update the stats for every vehicle if the player is finished.
		if( vehActor->GetVehicleIndex() == 0 && C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer())
		{
			for( int i = 1; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
			{	
				otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(i);

				if(!otherVehActor)
				{
					LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
					continue;
				}

				C_VehicleComponent_Statistics* statsComponent = (C_VehicleComponent_Statistics*)otherVehActor->QueryInterface(E_IF_VehicleComponent_Statistics);

				if( statsComponent )
					statsComponent->SetRaceCompletedStatistics();
			}
		}
	}

	//Calculate time since last wrecked and longest time without being wrecked
	void C_VehicleComponent_Statistics::CheckWreckRaceTime()
	{
		::Game::Actors::C_VehicleLogicActor* pLogicActor = ((::Game::Actors::C_VehicleLogicActor*)GetActor());

		float raceTime = pLogicActor->GetRLC()->GetRaceTime();
		float timeSinceLastWreck = raceTime - pLogicActor->mStatistics.mLastWreckTime;
		if(timeSinceLastWreck > pLogicActor->mStatistics.mLongestTimeWithoutWreck)
		{
			pLogicActor->mStatistics.mLongestTimeWithoutWreck = timeSinceLastWreck;
		}
		pLogicActor->mStatistics.mLastWreckTime = raceTime;
	}

	const bool C_VehicleComponent_Statistics::OffensivePerk(const enum E_PICKUP_TYPE ePerkType, const enum E_PICKUP_SUB_TYPE eSubType, const bool fired)
	{
		if(eSubType == E_PICKUP_SUB_TYPE_DECOY)
		{
			//Bug35097 - Decoys counted as offensive but calculated elsewhere
			return true;
		}		
		else if((ePerkType == E_PICKUP_TYPE_BOMB && eSubType == E_PICKUP_SUB_TYPE_MINI)	//Bug35097 - Remove mastermine hits
				||
				(ePerkType == E_PICKUP_TYPE_BOMB && eSubType == E_PICKUP_SUB_TYPE_MOD))		//Created when a mastermine is converted, we don't want to count this
		{
			//Bug35097 - Mastermine doesn't count as a valid pickup hit or fired owing to fairness 
			//issues, ie you convert a mine to 6 mini-mines its unlikely all 6 are hit so your hit
			//ratio would suffer
			return false;
		}
		else if(ePerkType == E_PICKUP_TYPE_BOMB && eSubType == E_PICKUP_SUB_TYPE_REVERSED) //When a mine is forward fired
		{
			//If someone forward fires a bomb, we don't want to count it if its in flight but we do when we hit someone
			return !fired;
		}
		else if(ePerkType == E_PICKUP_TYPE_FAN || ePerkType == E_PICKUP_TYPE_NITROUS || ePerkType == E_PICKUP_TYPE_SHIELD || ePerkType == E_PICKUP_TYPE_REPAIR)
		{
			//Non offensive power ups (what about nitro rift??!?!?)
			return false;
		}
		else
		{
			//Everything else
			return true;
		}
	}

	bool C_VehicleComponent_Statistics::OnMessage(const Messaging::C_MessageBase *pMessage)
	{
		PERF_EVENT("C_VehicleComponent.Statistics.MessageHandler");
		switch(pMessage->Type())
		{
			//case GameMessaging::E_GameMsg_Update:
			//	{
			//		if(!mEnabled)
			//			return true;

			//		Update();
			//		return true;
			//	}

			case ::Game::E_GameflowMessage_RaceStarted:
				{
					mEnabled = true;
					return true;
				}

			case ::Game::E_VehicleMessage_Overtake:
				{
					PERF_EVENT("C_VehicleComponent.Statistics.Overtake");

					if(!mEnabled)
						return true;

					const ::Game::Actors::C_VehicleOvertakeMsg* pMsg = Messaging::MessageCast< ::Game::Actors::C_VehicleOvertakeMsg >(pMessage);

					if( pMsg->mOvertakerVehicleIndex == mVehicleIndex )
						SetVehicleOvertakerStatistics(pMsg->mOvertakerVehicleIndex, pMsg->mOvertakeeVehicleIndex);
					else if( pMsg->mOvertakeeVehicleIndex == mVehicleIndex )
						SetVehicleOvertakeeStatistics(pMsg->mOvertakeeVehicleIndex);

					return true;
				}

			//35844 - stopwatches not being added to stats
			case ::Game::E_PickupMessage_CollectStopwatch:
				{
					::Game::Actors::C_VehicleLogicActor* pActor	=(::Game::Actors::C_VehicleLogicActor *)GetActor();
					pActor->mStatistics.mStopwatchPickupsCollected++;
				
					return true;
				}

			case ::Game::E_PickupMessage_PickupPickedUp:
				{
					const ::Game::Actors::PickupPickedUpMsg* pMsg	= Messaging::MessageCast< ::Game::Actors::PickupPickedUpMsg >(pMessage);
					::Game::Actors::C_VehicleLogicActor* pActor	=(::Game::Actors::C_VehicleLogicActor *)GetActor();

					if(mVehicleIndex == pMsg->mVehicleIndex && pMsg->mePickupType == E_PICKUP_TYPE_REPAIR)
					{
						pActor->mStatistics.mNumTimesRepaired++;
					}

					if(mVehicleIndex == pMsg->mVehicleIndex && pMsg->mePickupType == E_PICKUP_TYPE_FAN)
					{
						pActor->mStatistics.mFanPickupsCollected++;
					}
					
					//35844 - stopwatches not being added to stats 
					//if(mVehicleIndex == pMsg->mVehicleIndex && pMsg->mePickupType == E_PICKUP_TYPE_STOPWATCH)
					//{
					//	pActor->mStatistics.mStopwatchPickupsCollected++;
					//}

					if(mVehicleIndex == pMsg->mVehicleIndex && pMsg->mePickupType == E_PICKUP_TYPE_NITROUS)
					{
						pActor->mStatistics.mNitrousPickupsCollected++;
					}

					// Keep track of total num pickups collected
					pActor->mStatistics.mTotalPickupsCollected++;

					return true;
				}

			case E_VehicleLogicMsg_FiredPerk:
				{
					if(!mEnabled)
						return true;

					const C_VehicleLogicMsg_FiredPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_FiredPerkMsg>(pMessage);

					if( pMsg->mVehicleIndex == mVehicleIndex)
					{
						::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());

						if( pMsg->mPickupType == E_PICKUP_TYPE_SHIELD )
						{
							vehActor->mStatistics.mNumTimesShield++;
						}
							
						//Stop it recording the blindfire shotgun (Only record the blindfires it generates)
						if(!pMsg->mSamePerkComponent && !(pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE && pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_MOD))
						{
							if( OffensivePerk(pMsg->mPickupType, pMsg->mPickupSubType, true) || pMsg->mPickupType == E_PICKUP_TYPE_NITROUS)
							{
								//We don't want to count a decoy as its associated pickup type, decoys don't use this message but just incase
								if(pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_DECOY)
								{
									vehActor->mStatistics.mNumPerksFired[pMsg->mPickupType] += (pMsg->mLevel + 1);
								}
							}
						}

						mLastFiredID = pMsg->mUniqueId;
#ifndef CFG_GOLD
						mFiredPerkRecentlyTimer = 0;
#endif
						mRacePositionWhenFired = vehActor->mPositionInfo.mRacePosition;
						mLastFiredPerk = pMsg->mPickupType;
					}

					return true;
				}

			case E_VehicleLogicMsg_HitByPerk:
				{
					if(!mEnabled)
						return true;

					const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
					::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());

					const enum E_PICKUP_TYPE ePickupType = pMsg->mPickupType;
					const enum E_PICKUP_SUB_TYPE ePickupSubType = pMsg->mPickupSubType;

					if( pMsg->mVehicleIndex == mVehicleIndex )
					{
						//	Seriously? We don't record any statistics when a vehicle is hit by a power-up???
						//	[GavinY] - TODO: Increment the number of times hit here
					}
					else if( pMsg->mFiringVehicleIndex == mVehicleIndex && OffensivePerk(ePickupType, ePickupSubType, false))
					{
						Btl::InsertionResult<Btl::Set<U32>::Iterator> insRes = mPerksHit.Insert(pMsg->mUniqueID);

						if(ePickupType == E_PICKUP_TYPE_BOMB && ePickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
						{
							//Someone's been hit by a forward fired mine, we don't count it as a fired 
							//pickup until it stops however if it hits someone on the way we'll need to add a fired stat too...
							vehActor->mStatistics.mNumPerksFired[E_PICKUP_TYPE_BOMB]++;
						}

						//If the perk type is a decoy this stat'll be totted up in VLC_NewScoring
						if(ePickupSubType != E_PICKUP_SUB_TYPE_DECOY)
						{
							//Standard
							if(insRes.inserted)
								vehActor->mStatistics.mNumPerksHit[ePickupType]++;
						}

						if(pMsg->mVehicleIndex >= vehActor->mStatistics.mDualVehicleStats.Size())
						{
							LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
							break;
						}

						switch(ePickupType)
						{
						case E_PICKUP_TYPE_SHOCK:
							if(pMsg->mLevel)
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfDoubleShocksAgainstDriver++;
							else
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfShocksAgainstDriver++;

							break;

						case E_PICKUP_TYPE_SHUNT:
							if(pMsg->mLevel)
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfDoubleShuntsAgainstDriver++;
							else
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfShuntsAgainstDriver++;

							break;

						case E_PICKUP_TYPE_BARGE:
							if(pMsg->mLevel)
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfDoubleBargesAgainstDriver++;
							else
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfBargesAgainstDriver++;

						case E_PICKUP_TYPE_BOMB:
							if(pMsg->mLevel)
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfDoubleBombsAgainstDriver++;
							else
								vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfBombsAgainstDriver++;

							break;
						}

						if(pMsg->mLevel)
							vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfTimesHitWithDoubleAggressivePerk++;
						else
							vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mNumberOfTimesHitWithAggressivePerk++;

						RLPickupType* pPickup	= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(ePickupType);
						RLPickupLevel* level = pPickup->mNormal.Object();
						int damage = pPickup->mNormal->mBalancingData->GetDamageInflicted();

						vehActor->mStatistics.mDualVehicleStats[pMsg->mVehicleIndex].mDamageCausedToDriver += damage;
					}

					return true;
				}

			case E_VehicleLogicMsg_ShieldedPerk:
				{
					if(!mEnabled)
						return true;

					const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
					::Game::Actors::C_VehicleLogicActor* vehActor		= ((::Game::Actors::C_VehicleLogicActor*)GetActor());

					if( pMsg->mDefendingVehicleIndex == mVehicleIndex )
					{
						vehActor->mStatistics.mNumTimesShielded++;

						if(pMsg->mFiringVehicleIndex < vehActor->mStatistics.mDualVehicleStats.Size())
							vehActor->mStatistics.mDualVehicleStats[pMsg->mFiringVehicleIndex].mStoppedAttackWithShield = true;
						else
							LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
					}

					return true;
				}

			case ::Game::E_GameLogicMessage_GameplayImpact:
				{
					if(!mEnabled)
						return true;

					const ::Game::C_GameplayImpactMsg* pMsg		= Messaging::MessageCast< ::Game::C_GameplayImpactMsg >(pMessage);
					::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());

					if( pMsg->mVehicleIdx != mVehicleIndex )
						return true;

					float damagePercent = (float)pMsg->mDamage / (float)vehActor->GetMaxHealth();
					vehActor->mStatistics.mDamageTakenPercent += damagePercent;

					if( damagePercent > vehActor->mStatistics.mMaxDamageTakenPercent )
						vehActor->mStatistics.mMaxDamageTakenPercent = damagePercent;

					// Need these to be this way because we don't know the calling order - at this point we 
					// only know that our damage has been applied - not the other vehicle.
					if( pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX )
					{
						if( pMsg->mAggressiveVehicleIdx == pMsg->mVehicleIdx )
						{
							if(pMsg->mOtherVehicleIdx < vehActor->mStatistics.mDualVehicleStats.Size())
								vehActor->mStatistics.mDualVehicleStats[pMsg->mOtherVehicleIdx].mDamageCausedToDriver += pMsg->mDamage;
							else
								LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
						}
						else
						{
							::Game::Actors::C_VehicleLogicActor* otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(pMsg->mOtherVehicleIdx);

							if(!otherVehActor)
							{
								LOGWARN(g_StatisticsLogger, "Attempting to record dual statistics for a vehicle that does not exist; ignoring.");
								return true;
							}

							if( vehActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED )
								otherVehActor->mStatistics.mDualVehicleStats[mVehicleIndex].mTookOutDriver = true;
						}
					}

					//E_IMPACT_LEVEL eMajorCrashImpactLevel = pRLC->GetRLSystem()->mReportingParameters->meMajorCrashImpactLevel;
					//if(pMsg->mImpactLevel >= eMajorCrashImpactLevel)
					//{
					//	vehActor->mStatistics.mMajorCrash = true;
					//	vehActor->mStatistics.mMajorCrashLap = pRLC->GetVehicleCurrentLap(mVehicleIndex);
					//}

					return true;
				}

			case ::Game::E_GameLogicMessage_LapIncremented:
				{
					if(!mEnabled)
						return true;

					const ::Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast< ::Game::C_LapIncrementedMsg >(pMessage);

					if( pMsg->mVehicleIndex != mVehicleIndex )
						return true;

					::Game::Actors::C_VehicleLogicActor* vehActor		= ((::Game::Actors::C_VehicleLogicActor*)GetActor());

					if(	vehActor->mLapsInfo.mCurrentLap <= 1)
						break;

					if( pMsg->mTime < vehActor->mStatistics.mFastestLapTime || vehActor->mLapsInfo.mCurrentLap == 2)
						vehActor->mStatistics.mFastestLapTime = pMsg->mTime;
					else if( pMsg->mTime > vehActor->mStatistics.mSlowestLapTime )
						vehActor->mStatistics.mSlowestLapTime = pMsg->mTime;

					return true;
				}

			case ::Game::E_GameLogicMessage_Failed_Rule:
				{
					if(!mEnabled)
						return true;

					const ::Game::C_FailedRuleMsg* pMsg = Messaging::MessageCast< ::Game::C_FailedRuleMsg >(pMessage);

					if( pMsg->mVehicleIndex != mVehicleIndex || pMsg->mVehicleIndex == 0 )
						return true;

					SetRaceCompletedStatistics();
					return true;
				}

			case ::Game::E_GameflowMessage_QuitAndWinRace:
			case ::Game::E_GameflowMessage_RaceFinished:
			case ::Game::E_GameflowMessage_QuitRace:
			case ::Game::E_GameflowMessage_QuitLevel:
			case ::Game::E_GameflowMessage_VehicleFinished:
			case ::Game::E_GameflowMessage_ResultsCalculated:
				{
					SetRaceCompletedStatistics();

					return true;
				}

			case Game::E_VehicleMessage_Eliminated:
				{
					const Game::Actors::C_VehicleEliminationMessage* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleEliminationMessage>(pMessage);

					if(pMsg->mVehicleIndex == mVehicleIndex)
						SetRaceCompletedStatistics();

					return true;
				}
				break;

			case ::Game::E_GameLogicMessage_Passed_All_Rule:
				{
					if(!mEnabled)
						return true;

					const ::Game::C_PassedAllRuleMsg* pMsg = Messaging::MessageCast< ::Game::C_PassedAllRuleMsg >(pMessage);

					if( pMsg->mVehicleIndex != mVehicleIndex && pMsg->mVehicleIndex != 0 )
						return true;

					::Game::Actors::C_VehicleLogicActor* vehActor	= ((::Game::Actors::C_VehicleLogicActor*)GetActor());
					vehActor->mStatistics.mFinished = true;

					SetRaceCompletedStatistics();

					return true;
				}

			case ::Game::E_VehicleMessage_FatallyDamaged:
				{
					const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);

					if(!mEnabled || pMsg->m_VehicleIndex != mVehicleIndex)
						return true;

					::Game::Actors::C_VehicleLogicActor* vehActor = ((::Game::Actors::C_VehicleLogicActor*)GetActor());

					if(vehActor->mScoringInfo.mWreckedTimer > 0.f && vehActor->mScoringInfo.mMostRecentVehicleIdxHitBy == mVehicleIndex)
					{
						HandleMyDeath(true);
					}
					else
						HandleMyDeath(false);

					return true;
				}

			case E_VehicleLogicMsg_AggressiveScore:
				{
					const C_VehicleLogicMsg_AggressiveScore* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AggressiveScore>(pMessage);

					if(mVehicleIndex == pMsg->mVehicleIndex)
					{				

						//new scoring system
						if(pMsg->mBonusScoringEvent == E_BLUR_SCORE_MOVE_BONUS_WRECKED)
						{
							HandleSomeoneElsesDeath(pMsg->mAboutVehicleIndex);

							const C_VehicleLogicMsg_WreckedMsg wreckedMsg(mVehicleIndex, pMsg->mAboutVehicleIndex, E_PICKUP_TYPE_NONE);
							if(GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
							{
								GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &wreckedMsg);
								
							}
							if(GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Damage))
							{
								GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Damage, &wreckedMsg);
							}

							Game::C_StickerManager::Instance().MessageHandler(&wreckedMsg);
						}
					}

					return true;
				}

			case E_VehicleLogicMsg_VehicleMurdered:
				{
					const C_VehicleLogicMsg_VehicleMurdered* pMsg = Messaging::MessageCast_Safe<C_VehicleLogicMsg_VehicleMurdered>(pMessage);

					if(mVehicleIndex == pMsg->mVictimVehicleIndex)
					{
						HandleMyDeath(false);
					}
					return true;
				}

			case E_VehicleLogicMsg_VehicleSuicided:
				{
					const C_VehicleLogicMsg_VehicleSuicided* pMsg = Messaging::MessageCast_Safe<C_VehicleLogicMsg_VehicleSuicided>(pMessage);

					if(mVehicleIndex == pMsg->mVictimVehicleIndex)
					{
						HandleMyDeath(true);
					}

					return true;
				}

			case E_VehicleLogicMsg_DecoyPickupDropOnTrack:
				{
					const C_VehicleLogicDecoyPickupDropOnTrackMsg* pMsg = Messaging::MessageCast_Safe<C_VehicleLogicDecoyPickupDropOnTrackMsg>(pMessage);

					if(mVehicleIndex == pMsg->mDecoyDropperVehicleIndex)
					{
						::Game::Actors::C_VehicleLogicActor* pActor	=(::Game::Actors::C_VehicleLogicActor *)GetActor();
						pActor->mStatistics.mDecoysDropped++;
					}

					return true;
				}


			return I_GameActorComponent::OnMessage(pMessage);
		}

		return false;
	}
}
