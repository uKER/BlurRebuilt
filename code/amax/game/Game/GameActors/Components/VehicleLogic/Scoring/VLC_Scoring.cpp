#include "stdafx.h"

#include "VLC_NewScoring.h"

#include <Automation\GameDataLogging.h>
#include <Game/Messaging/GameMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <ui/UiMessages.h>

#include <Game/Messaging/PickupActorMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <Vehicle/VehicleModel.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/AchievementsManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/VehicleLogic/Multiplayer/VLC_MultiplayerChallenges.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_FanDemands.h>

#include <GameLogic\Structures\LevelState.h>

#include <RaceBook\RaceBookManager.h>

#include <game/gamecontrols/GameControls.h>

#include <GameLogic\GameData\GameData.h>
#include <Game\Achievements\AchievementSystem.h>

#include <GameLogic\ActorStudioActors\DynamicObjectMessages.h>

#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <ui/UiInterface.h>
#include <Ui\SoundEffects.h>
#include<GameLogic\MultiplayerUtilities.h>

#include <GameLogic\Structures\VehicleScoringInfo.h>
#include <GameLogic/Profile/GameProfileManager.h>


Debug::Logger &gVLC_ScoringLogger( Debug::GetLogger("Vehicle.Scoring") );
Debug::Float g_debugRaceStartedTimer("Vehicle.Components.Scoring.RaceStartedTimer", 2.0f);
Debug::Bool g_InfiniteComboTimer("Vehicle.Components.Scoring.InfiniteComboTimer", false);
Debug::Bool g_DebugDrifterOnNormalDrift("Multiplayer.Upgrades.DrifterOnNormalDrift", false, Debug::Persist());

extern Debug::Float g_RecentlyHitByPerkTime;
extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

namespace Components
{

bool C_VehicleComponent_Scoring::CreateImpl()
{
	const I_VehicleComponent_Scoring_CreateInfo* pCreation_data = static_cast<const I_VehicleComponent_Scoring_CreateInfo *>(GetCreator().GetData());
	BIZ_ASSERT_MSG(pCreation_data, "C_VehicleComponent_Scoring::CreateImpl() > Couldn't get a pointer to the creation data, this is going to go badly");

	Game::Actors::C_VehicleLogicActor* vehActor			= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	BIZ_ASSERT_MSG((C_GameData::Instance().mScoringConfig.Number() == 1),"Unable to get Scoring Config from Sorcery");
	if (C_GameData::Instance().mScoringConfig.Number() == 0)
		return false;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	//Cache the stuff in the creation data
	mVehicleIndex = pCreation_data->mVehicleIndex;
	mRaceLogicLayerPtr = pCreation_data->mpRaceLogicLayer;
	m_pScoring = pCreation_data->mpScoringInfo;
	BIZ_ASSERT_MSG(m_pScoring, "C_VehicleComponent_Scoring::CreateImpl() > Couldn't get a pointer to the scoring info, oh dear...");

	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	//AddSubscriber( GameMessaging::E_GameMsg_PostUpdate );
	
	AddSubscriber( Game::E_VehicleMessage_Overtake );
	AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );

	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );

	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
	AddSubscriber( E_VehicleLogicMsg_AddOnTrackScore );
	AddSubscriber( E_VehicleLogicMsg_AggressiveScore );
	AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	AddSubscriber( E_VehicleLogicMsg_PerkEnded );
	AddSubscriber( E_VehicleLogicMsg_Recovered );
	AddSubscriber( E_VehicleLogicMsg_SurvivedElimination );
	AddSubscriber( E_VehicleLogicMsg_HOGReset );
	AddSubscriber( Game::E_GameflowMessage_VehicleFinished );
	AddSubscriber( Game::E_PickupMessage_PickupPickedUp );
	AddSubscriber( Game::E_VehicleMessage_ResetAfterWrecked );

	AddSubscriber( Game::E_PickupMessage_ShockedAPickup );

	AddSubscriber( E_VehicleLogicMsg_Fan_Demand_Passed );

	AddSubscriber( E_VehicleLogicMsg_EvadedPowerUp );
	AddSubscriber( E_VehicleLogicMsg_PowerUpDeleted );

	AddSubscriber(ActorMsg::E_DynamicObjMsg_VehicleSloMoEntry);
	AddSubscriber(Game::E_GameLogicMessage_GameplayImpact);

	AddSubscriber(E_VehicleLogicMsg_DestroyedPickupWithPickup);

	AddSubscriber(E_VehicleLogicMsg_SideSwipe);

	AddSubscriber(Game::E_GameLogicMessage_LapIncremented);

	AddSubscriber(E_VehicleLogicMsg_Repair);

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	mBaseMultiplier=0.f;

	mCurrentXP = 0;
	mHitByPerkRecentlyTimer = 0.f;
	mHitByVehicleRecentlyTimer = 0.f;
	mpLastShuntedActor = NULL;

	mUsingNitroDamage=0.f;
	mUsingNitroTimer=0.f;
	mCarsOvertakenWithNitro=0;
	mLastOvertakenVehicle=0;
	
	mLastShuntedIdx = -1;
	mShuntedTimer = 0.f;
	mShockedTimer = 0.f;

	mMoveScoreToUITimer = 0.f;
		
	mGotFirstPlaceDominator1=false;
	mGotFirstPlaceDominator2=false;
	mGotFirstPlaceDominator3=false;
	mGotFirstPlaceDominator4=false;
	mGotSecondPlace=false;
	mGotThirdPlace=false;
	
	mGotFirstPlace=false;
	mInFirstPlace=false;
	mTimeInFirstPlace=0.f;

	mRaceStarted=false;
	mRaceStartedTimer=g_debugRaceStartedTimer;
	mFansDisabled=false;

	mShieldedFromVehicle=0;
	mShieldedFromPerkUniqueID=0;

	mJumpDistance=0.f;

	mDrifting=false;
	mDriftDistance=0.f;

	mAchievedTarget=false;

	mCurrentComboXP=0;
	mComboNumber=0;
	mComboTimer=0.f;

	mLastBargeID=-1;

	mHOGTimer=0.f;
	mCat=false;

	mVendettaIndex=-1;
	mVendettaCount=0;
	mVendettaTime=0.f;

	mGotFirstSpeed=false;
	mGotSecondSpeed=false;
	mGotThirdSpeed=false;

	mDoing360=0;
	mPreviousAngle=0;

	mPreviousFiredPickupID=-1;
	mHitsInARow=0;
	mHitWithPowerUp=false;

	mNoRepairOrWreck=true;

	mAllScoresBuffer.Clear();

	mAmaxScoreBuffer.Clear();
	mAmaxScoreBufferTimer=pScoringConfig->message_delay_timer;

	m_pScoring->mAllFansGained=false;
	m_pScoring->mSkillShots = 0;
	m_pScoring->mLongestChain = 0;

	mLastDriftingPickup = E_PICKUP_TYPE_NONE;

	mWreckStreak=0;

	mPickupFiredInAir=false;
	mPickupFiredWhilstNitro=false;
	mPickupFiredWhilstDrifting=false;

	for(int a=0;a<MAX_VEHICLES;a++)
	{
		mHitVehicleWithPerkTimer[a]=0.f;
		mHitVehicleWithPerkType[a] = E_PICKUP_TYPE_NONE;
	}

	for(int a=0;a<AMAX_SCORING_EVENT_MAX;a++)
	{
		mIndividualScoringStore[a]=0;
	}
	
	mIsMultiplayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer();

	// Some modes aren't races (e.g. survivor on the bowl), so "ruling the race!" etc doesnt really make sense. 
	// Just disable racing specific rewards in these modes (see Craig Howard).
	if(mIsMultiplayer)
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

		// Splitscreen
		if(levelState.mNumViewports > 1)
		{
			// If in splitscreen then ask the splitscreen event for its details
			mRacingContextFanRewardsAllowed = !levelState.mDisableRacingContextFanRewards;
		}
		
		// TODO: may have to also add online MP exceptions here (e.g. bowl etc)
	}
	else
	{
		// Allow by default
		mRacingContextFanRewardsAllowed = true; 
	}

	// Cache pointer to owner vehicle's abilities
	mpMultiplayerAbilities = levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
	BIZ_ASSERT_MSG(mpMultiplayerAbilities,"Error.. VLC_Scoring CreateImpl() unable to obtain pointer to Multiplayer Abilities");

	mIsMotorMash = Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Destruction;;

	return true;
}

void C_VehicleComponent_Scoring::Update()
{
	PERF_EVENT("C_VehicleComponent.Scoring.Update");

	float frameTime	= C_GameApp::GetInstance()->GetTimeStep();
	UpdateTimers(frameTime);

	if(!mRaceStarted || mRaceStartedTimer > 0.0f)
	{
		if(mRaceStarted && mRaceStartedTimer > 0.0f)
			mRaceStartedTimer -= frameTime;

		return;
	}

	BIZ_ASSERT_MSG((C_GameData::Instance().mScoringConfig.Number() == 1),"Unable to get Scoring Config from Sorcery");
	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(!mGotFirstSpeed && vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_1_mph)
	{
		GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_SPEED1);
		mGotFirstSpeed=true; 
	}
	if(!mGotSecondSpeed && vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_2_mph)
	{
		GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_SPEED2);
		mGotSecondSpeed=true; 
	}
	if(!mGotThirdSpeed && vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_3_mph)
	{
		GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_SPEED3);
		mGotThirdSpeed=true; 
	}

	mInFirstPlace=false;
	if(vehActor->mPositionInfo.mSpRacePosition == 1)
	{
		mInFirstPlace=true;
	}

	if(mWreckStreak != vehActor->GetStatistics().mWreckStreak)
	{
		mWreckStreak=vehActor->GetStatistics().mWreckStreak;

		if(mWreckStreak == 3)
		{
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_TIMES_THREE_KILLING_SPREE);
		}
		if(mWreckStreak == 5)
		{
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_TIMES_FIVE_KILLING_SPREE);
		}
		if(mWreckStreak == 10)
		{
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_TIMES_TEN_KILLING_SPREE);
		}
		if(mWreckStreak == 20)
		{
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_TIMES_TWENTY_KILLING_SPREE);
		}
	}

	//	[GavinY] - Knock these out in MP because someone always starts in 1st, and gets a load of fans right away...
	if(!mIsMultiplayer)
	{
		if(!mGotFirstPlace && vehActor->mPositionInfo.mSpRacePosition < 2 && levelState.mNumCars > 1)
		{
			mGotFirstPlace=true;
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_FIRST_PLACE_KING);
		}

		if(!mGotSecondPlace && vehActor->mPositionInfo.mSpRacePosition < 3  && levelState.mNumCars > 2)
		{
			mGotSecondPlace=true;
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SECOND_PLACE);
		}

		if(!mGotThirdPlace && vehActor->mPositionInfo.mSpRacePosition < 4  && levelState.mNumCars > 3)
		{
			mGotThirdPlace=true;
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_THIRD_PLACE);
		}
	}

	//	[GavinY] - Knock these out in Fans (aka Points) MP races because the fan reward for staying out in front totally unbalances the game.
	if(mInFirstPlace && levelState.mNumCars > 1 && mRacingContextFanRewardsAllowed && levelState.mRaceMode != Game::E_RaceMode_Multiplayer_Points)
	{
		mTimeInFirstPlace += frameTime;
		if(!mGotFirstPlaceDominator1 && mTimeInFirstPlace > pScoringConfig->first_place_dom_1)
		{
			GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR);
			mGotFirstPlaceDominator1=true;
		}
		if(!mGotFirstPlaceDominator2 && mTimeInFirstPlace > pScoringConfig->first_place_dom_2)
		{
			GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR2);
			mGotFirstPlaceDominator2=true;
		}
		if(!mGotFirstPlaceDominator3 && mTimeInFirstPlace > pScoringConfig->first_place_dom_3)
		{
			GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR3);
			mGotFirstPlaceDominator3=true;
		}
		if(!mGotFirstPlaceDominator4 && mTimeInFirstPlace > pScoringConfig->first_place_dom_4)
		{
			GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR4);
			mGotFirstPlaceDominator4=true;
		}
	}
	else
	{
		mTimeInFirstPlace=0.f;
	}

	if(mHOGTimer)
	{
		mHOGTimer-=frameTime;
		if(mHOGTimer < 0.f)
		{
			mHOGTimer=0.f;
		}
	}

	if(mComboTimer)
	{
		if(!g_InfiniteComboTimer)
		{
			mComboTimer-=frameTime;
		}

		if(mComboTimer < 0.f)
		{
			int score=0;

			RLHermiteKeyFrame keyFrame;
			Curve::C_HermiteCurve effectCurve;
			for(int j = 0; j < pScoringConfig->combo_end_multiplier_graph.count; j++)
			{
				keyFrame = pScoringConfig->combo_end_multiplier_graph[j];
				effectCurve.AddKeyFrame(keyFrame.mTime, keyFrame.mValue, keyFrame.mInTangent, keyFrame.mOutTangent);
			}

			float step=(float)mComboNumber / (float)pScoringConfig->max_achieveble_chain;
			step=Maths::Clamp(step,0.f,1.0f);
			score=int(mCurrentComboXP * effectCurve.Evaluate(step));

			if(mComboNumber < 2)
			{
				score=0;
			}

			mCurrentComboXP+=score;
			mCurrentXP+=score;
			m_pScoring->mFinalScore=mCurrentXP;

			//log the score to the stats server
			AmaxDataLogging::LogScore(mVehicleIndex,vehActor->mPositionInfo.mWorldPosition,AMAX_SCORING_EVENT_CHAIN_COMPLETE,score,mCurrentComboXP,mComboNumber,mCurrentXP);

			if(mComboNumber > m_pScoring->mLongestChain)
			{
				m_pScoring->mLongestChain = mComboNumber;
			}

			AmaxScoreBuffer buffer;
			buffer.mFansAdded=score;
			buffer.mFanTotal=mCurrentComboXP;
			buffer.mComboNumber=-1;
			buffer.mScoreType=AMAX_SCORING_EVENT_CHAIN_COMPLETE;
			buffer.mString.Clear();
			if(mComboNumber > 1)
			{
				AmaxScoringEventType *scoring_event=NULL;

				int score=0;
				for(int a=0;a<C_GameData::Instance().mScoringEventTypes.Number();a++)
				{
					if(C_GameData::Instance().mScoringEventTypes[a]->type == AMAX_SCORING_EVENT_CHAIN_COMPLETE)
					{
						scoring_event=C_GameData::Instance().mScoringEventTypes[a];
						break;
					}
				}

				if(!scoring_event)
				{
					BIZ_ASSERT_FAIL("Trying to Grant XP for a scoring event that isn't in Sorcery");
					return;
				}

				buffer.mString=UI::C_TextStringID(scoring_event->loc_string.id).String();
			}

			
			C_VehicleLogicMsg_ChainCompleted chainCompleteMessage(mVehicleIndex, mComboNumber);
			if(vehActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
			{
				//Tell the multiplayer challenges component (So it can reset any "per chain" challenges)
				C_VehicleLogicMsg_Scored scoringMessage(mVehicleIndex, AMAX_SCORING_EVENT_CHAIN_COMPLETE);

				GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &scoringMessage);
				GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &chainCompleteMessage);
			}

			mAmaxScoreBuffer.PushBack(buffer);

			AmaxEvent *ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();
			if(ev)
			{
				if(m_pScoring->mFinalScore > 250000)
				{
					m_pScoring->mFinalScore=250000;
					m_pScoring->mAllFansGained=true;
				}
			}

			mCurrentComboXP=0;
			mComboNumber=0;
			mComboTimer=0.f;
		}
	}

	if(mUsingNitroTimer)
	{
		mUsingNitroTimer-=frameTime;

		if(mUsingNitroTimer < 0.f)
		{
			if(mCarsOvertakenWithNitro)
			{
				int score=0;

				if(mCarsOvertakenWithNitro == 1)
					score=1;
				else if(mCarsOvertakenWithNitro == 2)
					score=2;
				else if(mCarsOvertakenWithNitro == 3)
					score=3;
				else if(mCarsOvertakenWithNitro > 3)
					score=4;

				if(mShuntedTimer || mShockedTimer)
				{
					GrantVehicleXP(mLastOvertakenVehicle, AMAX_SCORING_EVENT_THE_OLD_ONE_TWO);
				}
				else
				{
					GrantVehicleXP(mLastOvertakenVehicle, AMAX_SCORING_EVENT_NITROUS_OVERTAKE);
				}

				Game::C_AchievementsManager::Instance().OvertakeWithNitro(mVehicleIndex, mCarsOvertakenWithNitro);
			}

			if(mUsingNitroDamage == 0.f)
			{
				GrantVehicleXP(mLastOvertakenVehicle, AMAX_SCORING_EVENT_CLEAN_RACER);
			}

			mCarsOvertakenWithNitro=0;
			mUsingNitroTimer=0.f;
		}
	}

	if(mHitByPerkRecentlyTimer > 0)
	{
		const RLHandOfGodParameters *pHoGParams	= RLDatabase::Instance().mRLSystems[0]->mHandOfGodParameters.Object();

		float mDotUp = Maths::DotProduct( vehActor->GetWorldMatrix().AY(), Maths::HVec3(0.f, 1.f, 0.f) );
		if( mDotUp <= pHoGParams->mMinDotUp )
		{
			Game::Actors::C_VehicleLogicActor* otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(m_pScoring->mMostRecentVehicleIdxHitBy);
			
			if (otherVehActor)
			{
				otherVehActor->mStatistics.mDualVehicleStats[mVehicleIndex].mFlippedOutByAggressivePerk = true;

				C_VehicleLogicMsg_HitByPerkSpecialEventFlippedMsg flipSpecialMsg(mVehicleIndex, m_pScoring->mMostRecentVehicleIdxHitBy);

				if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
				{
					_UI_MESSAGE_FULL( flipSpecialMsg );
				}

				//if(Game::C_InGameDialogueManager::Instance().IsEnabled())
				//	Game::C_InGameDialogueManager::Instance().MessageHandler(&flipSpecialMsg);

				Game::C_AchievementsManager::Instance().MessageHandler(&flipSpecialMsg);

				AmaxDataLogging::LogPerkCausedFlip(m_pScoring->mMostRecentVehicleIdxHitBy, mVehicleIndex, mRaceLogicLayerPtr->ConvertPickupTypeToString(mMostRecentPerkHitBy));
				if(vehActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED && m_pScoring->mFlippedOutTimer > 0.f && m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX)
				{
					VictimScore(AMAX_SCORING_EVENT_FLIPPED_A_CAR);
					m_pScoring->mFlippedOutTimer = 0.f;
				}
			}
		}

		mHitByPerkRecentlyTimer -= frameTime;
	}

	if(mHitByVehicleRecentlyTimer > 0)
	{
		mHitByVehicleRecentlyTimer -= frameTime;
	}

	if(vehActor->GetGraphicalActor(mVehicleIndex)->InWater())
	{
		//Add some damage for the nitro to stop clean nitrous being wrongly given
		if(mUsingNitroTimer)
		{
			mUsingNitroDamage += 1;
		}
		if(m_pScoring->mDrownedTimer > 0.f && m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX)
		{
			//Murder
			VictimScore(AMAX_SCORING_EVENT_DROWN);

			m_pScoring->mDrownedTimer = 0.f;

		}
		
		if(!mDrowned)
		{
			vehActor->mStatistics.mNumTimesDrowned++;
			mDrowned = true;
		}
	}
	else
		mDrowned = false;

	if(vehActor->GetGraphicalActor(mVehicleIndex)->HasGoneOffTrack())
	{
		//Add some damage for the nitro to stop clean nitrous being wrongly given
		if(mUsingNitroTimer)
		{
			mUsingNitroDamage += 1;
		}
		if(m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX && (mHitByPerkRecentlyTimer > 0 || mHitByVehicleRecentlyTimer > 0))
		{
			VictimScore(AMAX_SCORING_EVENT_CLIFF_DIVER);
		}
		vehActor->GetGraphicalActor(mVehicleIndex)->SetGoneOffTrack(false);
	}

	if(mVendettaTime)
	{
		mVendettaTime -= frameTime;
		if(mVendettaTime <=0.f)
		{
			mVendettaTime=0.f;
			mVendettaCount=0;
		}
	}

	if(mShockedTimer)
	{
		mShockedTimer -= frameTime;
		if(mShockedTimer <=0.f)
			mShockedTimer=0.f;
	}

	if(mMoveScoreToUITimer)
	{
		mMoveScoreToUITimer -= frameTime;
		if(mMoveScoreToUITimer <=0.f)
		{
			mMoveScoreToUITimer=0.f;
			m_pScoring->mFinalScoreForUI=m_pScoring->mFinalScore;
		}
	}
	
	if(mShuntedTimer)
	{
		mShuntedTimer -= frameTime;
		if(mShuntedTimer <= 0.f)
			mShuntedTimer = 0.f;

		if(mpLastShuntedActor)
		{
			Maths::HVec3 vec = Maths::Normalise(mpLastShuntedActor->GetWorldMatrix().AW() - vehActor->GetWorldMatrix().AW());
			Maths::HVec3 up(0.f, 1.f, 0.f);
			float dot = Maths::DotProduct(vec, up);

			if(dot > 0.9f)
			{
				GrantVehicleXP(mpLastShuntedActor->GetVehicleIndex(), AMAX_SCORING_EVENT_UNDER);
				Game::C_AchievementsManager::Instance().UnderShuntPerk(mVehicleIndex);
				mpLastShuntedActor = NULL; // So we don't get in here again for this shunt.
			}
		}
	}

	if(vehActor->mShieldTime == 0.f)
	{
		mShieldedFromVehicle=mVehicleIndex;		//make it you so we test against it later on
		mShieldedFromPerkUniqueID=0;
	}

	//have we achieved target
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();
	bool passed=true;
	for(int a=0; a < results.mCars[mVehicleIndex].mPassedRule.Size();a++)
	{
		if(results.mCars[mVehicleIndex].mPassedRule[a] == false)
		{
			passed=false;
			break;
		}
	}

	if(passed && !mAchievedTarget)
	{
		//GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_TARGET_ACHIEVED);
		mAchievedTarget=true;
	}

	//jumping & drifting
	const Vehicle::C_ModelInput *pModelInput = vehActor->GetGraphicalActor(mVehicleIndex)->GetVehicleInput();

	mNumWheelsOnGround=0;
	mSlipAmount=0.f;

	for(int iWheel = 0; iWheel < pModelInput->m_Wheels.Size(); iWheel++)
	{
		mSlipAmount += abs(pModelInput->m_Wheels[iWheel].m_fSlipAngle);

		if(pModelInput->m_Wheels[iWheel].m_bOnGround)
		{
			mNumWheelsOnGround++;
		}
	}

	if(mNumWheelsOnGround == 0)
	{
		mJumpDistance+=vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate;
		m_pScoring->mTimeInAir += frameTime;
		
		mCat=true;
	}
	else
	{
		if(m_pScoring->mTimeInAir > 0)
		{
			C_VehicleLogicMsg_LandedMsg msg(mVehicleIndex, m_pScoring->mTimeInAir);
			if(vehActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
			{
				GetActor()->SendMessageToComponent(E_IF_VehicleComponent_Damage, &msg);
			}

			if(m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
			{
				Components::C_VehicleComponent_MP_Challenges* pChallenges = (Components::C_VehicleComponent_MP_Challenges*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges);
				if(pChallenges)
				{
					pChallenges->OnMessage(&msg);
				}
			}

			m_pScoring->mTimeInAir = 0;
		}

		if(mJumpDistance)
		{
			if(mJumpDistance > pScoringConfig->minimum_jump_distance && mUsingNitroTimer)
			{
				GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_NITROUS_JUMP);
				Game::C_AchievementsManager::Instance().PerformedNitroJump(mVehicleIndex);
			}
		}

		if(mNumWheelsOnGround != 4)
		{
			mCat=false;
		}
		else if(mNumWheelsOnGround == 4)
		{
			if(mCat && mHOGTimer == 0.f && mHitByPerkRecentlyTimer <= 0.f)
			{
				if(mJumpDistance > pScoringConfig->minimum_cat_distance)
					GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_THE_CAT);
			}

			mCat=false;
		}

		mJumpDistance=0.f;
	}

	mDrifting=false;

	Maths::HVec3 vel_heading=pModelInput->m_vVelocity;
	if(vel_heading.Length())
	{
		vel_heading.Normalise();
	}
	else
	{
		vel_heading=vehActor->mPositionInfo.mDirection;
	}

	mDriftingAngle=Maths::DotProduct(vel_heading,vehActor->mPositionInfo.mDirection);
	float angle_in_degs=(float)Maths::ArcCos(mDriftingAngle) * (180/Maths::Pi);

	if(angle_in_degs >= pScoringConfig->drift_angle_min && angle_in_degs <= pScoringConfig->drift_angle_max)
	{
		if(mSlipAmount >= pScoringConfig->drift_slip_min && mSlipAmount <= pScoringConfig->drift_slip_max)
		{
			mDrifting=true;
			mDriftDistance+=vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate;
		}
	}

	vehActor->mStatistics.mIsDriftingWithScore = false;
	if(!mDrifting && mDriftDistance)
	{
		int score=0;
		if(mDriftDistance > pScoringConfig->mega_drift_distance_to_score)
		{
			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_MEGA_DRIFT);

			if(!g_DebugDrifterOnNormalDrift)
			{
				CheckDrifter();
			}
		}
		else if(mDriftDistance > pScoringConfig->super_drift_distance_to_score)
		{
			if(g_DebugDrifterOnNormalDrift)
			{
				CheckDrifter();
			}

			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SUPER_DRIFT);
		}
		else if(mDriftDistance > pScoringConfig->drift_distance_to_score)
		{
			if(g_DebugDrifterOnNormalDrift)
			{
				CheckDrifter();
			}

			GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_DRIFT);
		}

		if(score)
		{
			vehActor->mStatistics.mIsDriftingWithScore = true;
		}

 		mLastDriftingPickup=E_PICKUP_TYPE_NONE;
		if(mDriftDistance > vehActor->mStatistics.mLongestDrift)
		{
			vehActor->mStatistics.mLongestDrift = mDriftDistance;
		}
		mDriftDistance=0.f;
	}

	//////////////////////////// 360 //////////////////////////////

	float vangle = Maths::ArcTan( vehActor->GetWorldMatrix().AZ().X(), vehActor->GetWorldMatrix().AZ().Z() );
	vangle+=Maths::Pi;

	if(mDone360Timer)
	{
		mDone360Timer-=frameTime;

		if(mDone360Timer < 0.f)
		{
			mDone360Timer=0.f;
			mDoing360=0;
		}
	}
	else
	{
		float angle_diff=0;
		if(mPreviousAngle > Maths::Pi15 && vangle < Maths::PiBy2)
		{
			angle_diff=Maths::Pi2-mPreviousAngle;
			angle_diff+=vangle;
		}
		else if(mPreviousAngle < Maths::PiBy2 && vangle > Maths::Pi15)
		{
			angle_diff=Maths::Pi2-vangle;
			angle_diff+=mPreviousAngle;
			angle_diff=-angle_diff;
		}
		else
		{
			angle_diff=vangle-mPreviousAngle;
		}

		if(abs(angle_diff) < pScoringConfig->min_360_angular_speed || abs(angle_diff) > pScoringConfig->max_360_angular_speed)
		{
			mDoing360=0;
		}

		if(mDoing360==0)
		{
			//should we start?
			mTotalAngle=0.f;

			if(angle_diff > 0)
			{
				mDoing360=1;
			}
			else if(angle_diff < 0)
			{
				mDoing360=-1;
			}
		}
		else if(mDoing360 == 1)
		{
			if(angle_diff > 0)
			{
				mTotalAngle+=angle_diff;
			}
			else
			{
				mTotalAngle=0.f;
				mDoing360=0;
			}
		}
		else if(mDoing360 == -1)
		{
			if(angle_diff < 0)
			{
				mTotalAngle+=-angle_diff;
			}
			else
			{
				mTotalAngle=0.f;
				mDoing360=0;
			}
		}

		if(mTotalAngle >= Maths::Pi2)
		{
			mDone360Timer=pScoringConfig->cooldown360;
			mTotalAngle=0.f;
			mDoing360=0;
			GrantVehicleXP(mVehicleIndex,AMAX_SCORING_EVENT_360);
		}
	}

	mPreviousAngle=vangle;

	//UI message buffer
	if(mAmaxScoreBuffer.Size())
	{
		mAmaxScoreBufferTimer-=frameTime;

		if(mAmaxScoreBufferTimer < 0.f)
		{
			mAmaxScoreBufferTimer=pScoringConfig->message_delay_timer;

			AmaxScoreBuffer buffer=mAmaxScoreBuffer[0];

			UI::C_UiScoringEventFans uimsg(buffer.mFanTotal, buffer.mComboNumber==1 ? false : true, mVehicleIndex);
			_UI_MESSAGE_FULL( uimsg );

			if(buffer.mComboNumber > 1 || buffer.mComboNumber == -1)
			{
				//UI::C_UiScoringEventChain uimsg2(buffer.mComboNumber, buffer.mComboNumber==2 ? false : true, mVehicleIndex);
				//_UI_MESSAGE_FULL( uimsg2 );
			}

//			UI::C_UiScoringEventPerk uimsg1(E_PICKUP_TYPE_NONE,mVehicleIndex,mVehicleIndex,buffer.mString.Chars());
			UI::C_UiScoringEventPerk uimsg1( buffer.mString.Chars(), buffer.mComboNumber, buffer.mFansAdded, buffer.mFanTotal, mVehicleIndex );
			_UI_MESSAGE_FULL( uimsg1 );

			if(buffer.mScoreType == AMAX_SCORING_EVENT_CHAIN_COMPLETE)
			{
				if(buffer.mFansAdded)
					mMoveScoreToUITimer=1.33f;
				else
					mMoveScoreToUITimer=0.5f;
			}

			mAmaxScoreBuffer.EraseByIndex(0);
		}
	}
	else
	{
		mAmaxScoreBufferTimer=pScoringConfig->message_delay_timer;
	}

}

void C_VehicleComponent_Scoring::UpdateTimers(const float frameTime)
{
	//scoring timers
	if(m_pScoring->mFlippedOutTimer)
	{
		m_pScoring->mFlippedOutTimer-=frameTime;

		if(m_pScoring->mFlippedOutTimer < 0.f)
			m_pScoring->mFlippedOutTimer=0.f;
	}
	if(m_pScoring->mCrashedOutTimer)
	{
		m_pScoring->mCrashedOutTimer-=frameTime;

		if(m_pScoring->mCrashedOutTimer < 0.f)
			m_pScoring->mCrashedOutTimer=0.f;
	}
	if(m_pScoring->mCollisionTimer)
	{
		m_pScoring->mCollisionTimer-=frameTime;

		if(m_pScoring->mCollisionTimer < 0.f)
			m_pScoring->mCollisionTimer=0.f;
	}
	if(m_pScoring->mRevengeTimer)
	{
		m_pScoring->mRevengeTimer-=frameTime;

		if(m_pScoring->mRevengeTimer < 0.f)
			m_pScoring->mRevengeTimer=0.f;
	}
	if(m_pScoring->mWreckedTimer)
	{
		m_pScoring->mWreckedTimer-=frameTime;

		if(m_pScoring->mWreckedTimer < 0.f)
			m_pScoring->mWreckedTimer=0.f;
	}
	if(m_pScoring->mDrownedTimer)
	{
		m_pScoring->mDrownedTimer-=frameTime;

		if(m_pScoring->mDrownedTimer < 0.f)
			m_pScoring->mDrownedTimer=0.f;
	}
	if(m_pScoring->mOutOfBoundsTimer > 0.f)
	{
		m_pScoring->mOutOfBoundsTimer-=frameTime;

		if(m_pScoring->mOutOfBoundsTimer <= 0.f)
			m_pScoring->mOutOfBoundsTimer = 0.f;
	}
}

void C_VehicleComponent_Scoring::PostUpdate()
{
	if(!mRaceStarted || mRaceStartedTimer > 0.f)
		return;




}

void C_VehicleComponent_Scoring::NearMiss(int vehicle_index)
{
	if(vehicle_index != -1)
	{
		GrantVehicleXP(vehicle_index, AMAX_SCORING_EVENT_NEAR_MISSED_CAR);
	}
	else
	{
		GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_NEAR_MISSED_ENVIRONMENT);
	}
}

void C_VehicleComponent_Scoring::ResetHitByPerkRecentlyTimer()
{
	mHitByPerkRecentlyTimer = g_RecentlyHitByPerkTime;
}

void C_VehicleComponent_Scoring::CheckDrifter()
{
	//Give a nitro if the player has Drifter upgrade
	Game::C_MultiplayerVehicleAbility* ability = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Drifter);
	if(Game::VehicleIsLocal(mVehicleIndex) && ability->GetActive())
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

		Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
		if(pLogicActor && pLogicActor->mPickupInfo.mPickupStore.Size() < levelState.mCars[mVehicleIndex].mUpgrades.NumPickupSlots())
		{
			Game::Actors::C_GrantPickupMsg GrantWinnerPickupMessage(mVehicleIndex, E_PICKUP_TYPE_NITROUS);
			pLogicActor->SendMessageToComponent(E_IF_VehicleComponent_Pickups, &GrantWinnerPickupMessage);
		}

		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Drifter, true);
	}	
}

void C_VehicleComponent_Scoring::GrantVehicleXP(int about_vehIndex, AMAX_SCORING_EVENT se)
{
	Game::Actors::C_VehicleLogicActor* vehActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

	if(!mRaceStarted || mRaceStartedTimer > 0.f/* || vehActor->GetRLC()->IsVehicleAIControlled(mVehicleIndex)*/)
		return;

	BIZ_ASSERT_MSG((C_GameData::Instance().mScoringConfig.Number() == 1),"Unable to get Scoring Config from Sorcery");
	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(m_pScoring->mAllFansGained)
		return;

	AmaxScoringEventType *scoring_event=NULL;

	int score=0;
	for(int a=0;a<C_GameData::Instance().mScoringEventTypes.Number();a++)
	{
		if(C_GameData::Instance().mScoringEventTypes[a]->type == se)
		{
			scoring_event=C_GameData::Instance().mScoringEventTypes[a];
			break;
		}
	}

	if(!scoring_event)
	{
		BIZ_ASSERT_FAIL("Trying to Grant XP for a scoring event that isn't in Sorcery");
		return;
	}

	int diff=scoring_event->max_score-scoring_event->min_score;
	score += scoring_event->min_score + (rand()&diff);
	score += (int)mBaseMultiplier;

	if(se == AMAX_SCORING_EVENT_FAN_DEMAND_COMPLETE)
	{
		score=mFanDemandFans;
	}
	else if( se == AMAX_SCORING_EVENT_CHALLENGE_COMPLETE)
	{
		score = mChallengeFans;
	}

	// Trick shots for the race rule
	// Referring to statistics in this component is naughty, but quick and easy. I won't tell if you don't.
	bool skill_shot=false;
	if( se == AMAX_SCORING_EVENT_DRIFTING )
	{
		skill_shot=true;
		m_pScoring->mSkillShots++;
		vehActor->mStatistics.mNumberOfSkillShotsDrifting++;
	}
	else if( se == AMAX_SCORING_EVENT_AIR )
	{
		skill_shot=true;
		m_pScoring->mSkillShots++;
		vehActor->mStatistics.mNumberOfSkillShotsAir++;
	}
	else if( se == AMAX_SCORING_EVENT_SKILL_SHOT_NITROUS )
	{
		skill_shot=true;
		m_pScoring->mSkillShots++;
	}
	else if( se == AMAX_SCORING_EVENT_CAMERA_LOOK )
	{
		skill_shot=true;
		m_pScoring->mSkillShots++;
	}

	// Any successful Power-up impact scoring event
	if( se == AMAX_SCORING_EVENT_SHUNT || se == AMAX_SCORING_EVENT_LEVEL2SHUNT
		|| se == AMAX_SCORING_EVENT_BARGE || se == AMAX_SCORING_EVENT_LEVEL2BARGE
		|| se == AMAX_SCORING_EVENT_SHOCK || se == AMAX_SCORING_EVENT_LEVEL2SHOCK
		|| se == AMAX_SCORING_EVENT_BOMB || se == AMAX_SCORING_EVENT_LEVEL2BOMB
		|| se == AMAX_SCORING_EVENT_BLINDFIRE )
	{
		// Showy Flourish ability -- gives extra fans on successful power-up impact
		Game::C_MultiplayerVehicleAbility* ability = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Showy_Flourish);
		if(ability->GetActive())
		{
			U8 extraFans = ( se==AMAX_SCORING_EVENT_BLINDFIRE ? C_MultiplayerData::Instance().m_pConfig->m_ShowyFlourishBlindfireExtraFans : (U8)(ability->GetValue()) ); 

			score += extraFans;
			m_pScoring->mShowyFlourishFans += extraFans;

			#ifndef CFG_GOLD
				if( (extraFans != 0) && g_DebugShowMpAbilityActivateSpam)
				{
					LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Showy Flourish upgrade was activated! %d extra fans were gained because you were so gnarly.",
													mVehicleIndex, extraFans);
				}
			#endif//CFG_GOLD

			//ability activated
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Showy_Flourish, true);
		}
	}

	mComboTimer=pScoringConfig->combo_time;

	//MP - Fan Multiplier for things like double xp weekends
	score *= levelState.mFanMultiplier;

	mCurrentComboXP+=score;
	mComboNumber++;

	if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER && !mFansDisabled)
	{
		if(mComboNumber == 10)
		{
			if(Game::C_AchievementSystem::Instance().HasAchievement(levelState.mCars[mVehicleIndex].mController, Game::BLUR_ACHIEVEMENT_CHAINER))
			{
				Game::C_AchievementSystem::Instance().GiveAchievement(levelState.mCars[mVehicleIndex].mController, Game::BLUR_ACHIEVEMENT_CHAINER);
			}
		}
	}

	mCurrentXP+=score;
	m_pScoring->mFinalScore=mCurrentXP;
	m_pScoring->mFinalScoreForUI=m_pScoring->mFinalScore-mCurrentComboXP;

	//log the score to the stats server
	AmaxDataLogging::LogScore(mVehicleIndex,vehActor->mPositionInfo.mWorldPosition,se,score,mCurrentComboXP,mComboNumber,mCurrentXP);

	mIndividualScoringStore[se]+=score;
	m_pScoring->mIndividualScoring[se]=mIndividualScoringStore[se];

	if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER && !mFansDisabled)
	{
		if(se == AMAX_SCORING_EVENT_DRIFTING || se == AMAX_SCORING_EVENT_AIR || se == AMAX_SCORING_EVENT_SKILL_SHOT_NITROUS || se == AMAX_SCORING_EVENT_CAMERA_LOOK)
		{
			if(Game::C_AchievementSystem::Instance().HasAchievement(levelState.mCars[mVehicleIndex].mController, Game::BLUR_ACHIEVEMENT_SKILL_SHOT))
			{
				Game::C_AchievementSystem::Instance().GiveAchievement(levelState.mCars[mVehicleIndex].mController, Game::BLUR_ACHIEVEMENT_SKILL_SHOT);
			}
		}

		UI::C_WideString128 str;
		str.Clear();

		switch(se)
		{
			case AMAX_SCORING_EVENT_PICKUP_SHOCKED_OUT:
				str = levelState.mCars[about_vehIndex].mNameAboveCar;
				str += UI::C_TextStringID(scoring_event->loc_string.id).String();

				if(mShockedPickupType == E_PICKUP_TYPE_NITROUS)
					str += UI::C_TextStringID("PIK_NITRO").String();
				else if(mShockedPickupType == E_PICKUP_TYPE_BARGE)
					str += UI::C_TextStringID("PIK_BARGE").String();
				else if(mShockedPickupType == E_PICKUP_TYPE_SHOCK)
					str += UI::C_TextStringID("PIK_SHOCK").String();
				else if(mShockedPickupType == E_PICKUP_TYPE_SHUNT)
					str += UI::C_TextStringID("PIK_SHUNT").String();
				else if(mShockedPickupType == E_PICKUP_TYPE_BOMB)
					str += UI::C_TextStringID("PIK_BOMB").String();
				break;
			
			case AMAX_SCORING_EVENT_ENEMY_SHIELDED:
				str = levelState.mCars[about_vehIndex].mNameAboveCar;
				str += UI::C_TextStringID(scoring_event->loc_string.id).String();
				break;

			case AMAX_SCORING_EVENT_CHALLENGE_COMPLETE:
				str=mChallengeString;
				break;

			default:
				str=UI::C_TextStringID(scoring_event->loc_string.id).String();
				if(scoring_event->append_name)
				{
					str += 	levelState.mCars[about_vehIndex].mNameAboveCar;
				}
				break;
		}

		AmaxScoreBuffer buffer;
		buffer.mFansAdded=score;
		buffer.mFanTotal=mCurrentComboXP;
		buffer.mComboNumber=mComboNumber;
		buffer.mString=str;
		buffer.mScoreType=se;
		mAmaxScoreBuffer.PushBack(buffer);

		const C_VehicleLogicMsg_Scored msg(mVehicleIndex, se);
		Components::C_VehicleComponent_Fan_Demand* pFanDemand = (Components::C_VehicleComponent_Fan_Demand*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand);
		if(pFanDemand)
			pFanDemand->OnMessage(&msg);

		// Don't inform the challenge system about a
		// challenge scoring event 
		if(se!=AMAX_SCORING_EVENT_CHALLENGE_COMPLETE)
		{
			Components::C_VehicleComponent_MP_Challenges* pChallenges = (Components::C_VehicleComponent_MP_Challenges*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges);
			if(pChallenges)
				pChallenges->OnMessage(&msg);
		}

		// Points scoring would like these too!
		Components::C_VehicleComponent_PointsSystem* pPointsSystem = (Components::C_VehicleComponent_PointsSystem*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Points);
		if(pPointsSystem)
		{
			pPointsSystem->OnMessage(&msg);
		}

		mAllScoresBuffer.PushBack(buffer);
	}

	AmaxEvent *ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();
	if(ev)
	{
		if(m_pScoring->mFinalScore > 250000)
		{
			m_pScoring->mFinalScore=250000;
			m_pScoring->mAllFansGained=true;

			mCurrentComboXP=0;
			mComboNumber=0;
			mComboTimer=0.f;
		}
	}
}

//	Instruct another vehicle to score fans based on aggressively affecting this vehicle.
void C_VehicleComponent_Scoring::VictimScore(AMAX_SCORING_EVENT se )
{
	//	Don't give scores for doing anything to yourself...obviously.
	if(m_pScoring->mMostRecentVehicleIdxHitBy == mVehicleIndex || m_pScoring->mMostRecentVehicleIdxHitBy == Game::SYSTEM_FIRING_INDEX)
		return;

	Game::Actors::C_VehicleLogicActor* pLogicActor = ((Game::Actors::C_VehicleLogicActor* )GetActor());
	//	We know which vehicle last hit this one, so that's the aggressor.
	U8 aggressorVehicleIndex = m_pScoring->mMostRecentVehicleIdxHitBy;
	BIZ_ASSERT_MSG(aggressorVehicleIndex != INVALID_VEHICLE_IDX, "C_VehicleComponent_Scoring::VictimScore() > Invalid aggressor vehicle index...");
	
	C_VehicleLogicMsg_AggressiveScore scoreEvent(aggressorVehicleIndex, mVehicleIndex, se);

	if(Game::VehicleIsLocal(aggressorVehicleIndex))
	{
		Game::Actors::C_VehicleLogicActor* pLocalAggressorLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(aggressorVehicleIndex);
		if(pLogicActor)
			pLogicActor->SendMessageToActor(pLocalAggressorLogicActor->GetHandle(), &scoreEvent);
	}

	//	If we're playing a network game we need to send this to the aggressor (but probably let everyone have it so we can use it for cool UI stuff?)
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_EventManager->Add_AggressiveAction(Game::GetJoinIdFromLocalIndex(aggressorVehicleIndex), Game::GetJoinIdFromLocalIndex(mVehicleIndex), se);
	}
}

bool C_VehicleComponent_Scoring::ShouldCalculateMotorMashScoring(U8 firingVehicle)
{
	// Check for early out
	if(!mIsMotorMash)
	{
		return false;
	}

	// We only want to send scoring messages to vehicles local to us
	if( !Game::VehicleIsLocal(firingVehicle) )
	{
		return false;
	}

	return true;
}


void C_VehicleComponent_Scoring::HandleMotorMashMessageAggressiveImpact(U8 vehicleHit, U8 aggressiveVehicle, E_IMPACT_LEVEL impactLevel)
{
	if(!ShouldCalculateMotorMashScoring(aggressiveVehicle))
	{
		return;
	}

	// Make sure we tell the logic actor about this score, not just the UI
	BIZ_ASSERT(GetActor()!=0);
	Game::Actors::C_VehicleLogicActor& pLogicActor = *(Game::Actors::C_VehicleLogicActor*)GetActor();

	UI::C_WideString128 strSmall;
	strSmall.Clear();

	const MotorMashScoringValues& pScoringValues = C_MultiplayerData::Instance().m_pConfig->m_MotormashScoringValues;
	int ourScore = 0;

	if( vehicleHit == GetRivalIndex())
	{
		ourScore += pScoringValues.m_RivalDamageScore;
		strSmall += UI::C_TextStringID("MP_RIVAL_IMAGE").String();
		pLogicActor.mStatistics.mNumRivalDamageAttacks++;
	}

	switch(impactLevel)
	{
		case E_IMPACT_LEVEL_MEDIUM:
		{
			strSmall += UI::C_TextStringID("MP_LOW_DAMAGE_ATTACK").String();
			AwardLowDamageScore(ourScore, pScoringValues, pLogicActor);
			break;
		}

		case E_IMPACT_LEVEL_HEAVY:
		{
			strSmall += UI::C_TextStringID("MP_HIGH_DAMAGE_ATTACK").String();
			AwardHighDamageScore(ourScore, pScoringValues, pLogicActor);
			break;
		}

		// We don't want to handle anything else
		default:
			return;
	}

	UI::C_WideString128 strSmallLine2;
	strSmallLine2.Clear();
	strSmallLine2 = Game::C_Game::GetInstance()->GetLevelState().mCars[vehicleHit].mNameAboveCar.Chars();

	UI::C_WideString128 str;
	str.Clear();
	if( ourScore != 1 )
	{
		str.Format( UI::C_TextStringID("MP_MOTORMASH_POINTS").String(), ourScore );
	}
	else // Points mean prizes, but only if pluralised
	{
		str.Format( UI::C_TextStringID("MP_MOTORMASH_POINT").String(), ourScore );
	}

	// PR - this is no longer supported
	//UI::C_UiGenericMessage uiMsg(str, strSmall, strSmallLine2, aggressiveVehicle);
	//_UI_MESSAGE_FULL( uiMsg );
}

void C_VehicleComponent_Scoring::HandleMotorMashMessagePickupHit(U8 vehicleHit, U8 firingVehicle, E_PICKUP_TYPE pickupType)
{
	if(!ShouldCalculateMotorMashScoring(firingVehicle))
	{
		return;
	}

	BIZ_ASSERT(GetActor()!=0);
	Game::Actors::C_VehicleLogicActor& pLogicActor = *(Game::Actors::C_VehicleLogicActor*)GetActor();

	UI::C_WideString128 strSmall;
	strSmall.Clear();

	const MotorMashScoringValues& pScoringValues =	C_MultiplayerData::Instance().m_pConfig->m_MotormashScoringValues;
	int ourScore = 0;

	if( vehicleHit == GetRivalIndex())
	{
		ourScore += pScoringValues.m_RivalDamageScore;
		strSmall += UI::C_TextStringID("MP_RIVAL_IMAGE").String();
		pLogicActor.mStatistics.mNumRivalDamageAttacks++;
	}

	switch(pickupType)
	{
		case E_PICKUP_TYPE_BARGE:
		{
			strSmall += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_BARGE").String();
			AwardHighDamageScore(ourScore, pScoringValues, pLogicActor);
			break;
		}
		case E_PICKUP_TYPE_BLINDFIRE:
		{
			strSmall += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_BLINDFIRE").String();
			if( mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist)->GetActive() )
			{
				AwardHighDamageScore(ourScore, pScoringValues, pLogicActor);
			}
			else
			{
				AwardLowDamageScore(ourScore, pScoringValues, pLogicActor);
			}
			break;
		}
		case E_PICKUP_TYPE_BOMB:
		{
			strSmall += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_BOMB").String();
			AwardHighDamageScore(ourScore, pScoringValues, pLogicActor);
			break;
		}
		case E_PICKUP_TYPE_SHUNT:
		{
			strSmall += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_SHUNT").String();
			AwardHighDamageScore(ourScore, pScoringValues, pLogicActor);
			break;
		}
		// We don't know what message to display since we don't know the pickup, so bail out
		default:
			return;
	}

	UI::C_WideString128 strSmallLine2;
	strSmallLine2.Clear();
	strSmallLine2 = Game::C_Game::GetInstance()->GetLevelState().mCars[vehicleHit].mNameAboveCar.Chars();

	UI::C_WideString128 str;
	str.Clear();
	if( ourScore != 1 )
	{
		str.Format( UI::C_TextStringID("MP_MOTORMASH_POINTS").String(), ourScore );
	}
	else // Points mean prizes, but only if pluralised
	{
		str.Format( UI::C_TextStringID("MP_MOTORMASH_POINT").String(), ourScore );
	}

	// PR - this is no longer supported
	//UI::C_UiGenericMessage uiMsg(str, strSmall, strSmallLine2, firingVehicle);
	//_UI_MESSAGE_FULL( uiMsg );
}

void C_VehicleComponent_Scoring::CalculateSkillShots(E_PICKUP_TYPE type)
{
	if(mPickupFiredWhilstDrifting)
	{
		GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_DRIFTING);

		const C_VehicleLogicMsg_SkillshotMsg skillshotMsg(UI::E_SKILLSHOT_DRIFTING, type);
		if(GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
		{
			GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &skillshotMsg);
		}
	}
	
	if(mPickupFiredInAir)
	{
		GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_AIR);

		const C_VehicleLogicMsg_SkillshotMsg skillshotMsg(UI::E_SKILLSHOT_AIR, type);
		if(GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
		{
			GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &skillshotMsg);
		}
	}

	if(mPickupFiredWhilstNitro)
	{
		GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_NITROUS);
	}
}

bool C_VehicleComponent_Scoring::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("C_VehicleComponent.Scoring.MessageHandler");
	
	BIZ_ASSERT_MSG((C_GameData::Instance().mScoringConfig.Number() == 1),"Unable to get Scoring Config from Sorcery");
	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	switch(pMessage->Type())
	{
		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		Update();
		//		return true;
		//	}

		//case GameMessaging::E_GameMsg_PostUpdate:
		//	{
		//		PostUpdate();
		//		return true;
		//	}
	case Game::E_VehicleMessage_ResetAfterWrecked:
		{
			const Game::Actors::C_VehicleResetAfterWreckedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAfterWreckedMsg>(pMessage);
			Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

			if(pMsg->mVehicleIndex == mVehicleIndex)
			{
				//re set - we just been respawned
				m_pScoring->mWreckedByAnotherPlayer = false;
			}
		}
		break;
		case E_VehicleLogicMsg_FiredPerk:
			{
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_FiredPerkMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(pMsg->mPickupType == E_PICKUP_TYPE_SHOCK || pMsg->mPickupType == E_PICKUP_TYPE_SHUNT || pMsg->mPickupType == E_PICKUP_TYPE_BARGE || pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE || pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
					{
						if(m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
							mPickupFiredInAir=true;
						else
							mPickupFiredInAir=false;

						if(mUsingNitroTimer)
							mPickupFiredWhilstNitro=true;
						else
							mPickupFiredWhilstNitro=false;

						if(mDrifting)
							mPickupFiredWhilstDrifting=true;
						else
							mPickupFiredWhilstDrifting=false;

						mPreviousFiredPickupID=pMsg->mUniqueId;
						mHitWithPowerUp=false;
					}

					if(pMsg->mPickupType == E_PICKUP_TYPE_NITROUS)
					{
						mUsingNitroDamage=0.f;
						mUsingNitroTimer=pScoringConfig->using_nitrous_timer;

						if(mDrifting)
						{
							GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_NITROUS_DRIFT);

							const C_VehicleLogicMsg_SkillshotMsg skillshotMsg(UI::E_SKILLSHOT_DRIFTING, pMsg->mPickupType);
							if(GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
							{
								GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &skillshotMsg);
							}
						}

						if(vehActor->mDraftingAmount)
						{
							GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_NITROUS_DRAFT_BOOST);
						}

						if(m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
						{
							GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_NITROUS_AIR);
						}
					}
				}
				
				return true;
			}

		case Game::E_PickupMessage_PickupPickedUp:
			{
				const Game::Actors::PickupPickedUpMsg* pMsg = Messaging::MessageCast< Game::Actors::PickupPickedUpMsg >(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(mDrifting)
					{
						if(mLastDriftingPickup == pMsg->mePickupType)
						{
							GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_DRIFTDOUBLEPICKUP);
							mLastDriftingPickup=E_PICKUP_TYPE_NONE;
						}
						else
						{
							mLastDriftingPickup=pMsg->mePickupType;
						}
					}
				}

				return true;
			}

		case Game::E_PickupMessage_ShockedAPickup:
			{
				const Game::Actors::PickupMessage_ShockedAPickup* pMsg = Messaging::MessageCast< Game::Actors::PickupMessage_ShockedAPickup >(pMessage);

				if(pMsg->mePickupType == E_PICKUP_TYPE_NONE)
				{
					GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_NO_PICKUP_SHOCKED);
				}
				else
				{
					mShockedPickupType=pMsg->mePickupType;
					GrantVehicleXP(pMsg->mShockedFromVehicleIndex, AMAX_SCORING_EVENT_PICKUP_SHOCKED_OUT);
				}
				
				return true;
			}

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

				if( pMsg->mVehicleIndex == mVehicleIndex )
				{
					mMostRecentPerkHitBy = pMsg->mPickupType;
					mHitByPerkRecentlyTimer = g_RecentlyHitByPerkTime;

					if(pMsg->mFiringVehicleIndex == mVehicleIndex)
					{
						//	WHOOPS! We hit ourself with one of our own power-ups (mine or shock right now...)

					}
					else
					{
						m_pScoring->mRevengeIndex=pMsg->mFiringVehicleIndex;
						m_pScoring->mRevengeTimer=pScoringConfig->revenge_timer;
					}
				}
				else if( pMsg->mFiringVehicleIndex == mVehicleIndex )
				{
					Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

					//Was It Good For You ability - DOESNT EXIST ANYMORE?
					/*int numSlotsToRepair = levelState.mCars[mVehicleIndex].mUpgrades.HealthSlotsRepairedOnPerkOpponent(); 
					if(numSlotsToRepair > 0)
					{
						// Not the best way of doing this.. but doubt sorcery val will ever specify more then 1 slot to repair (seems overpowered as it is!).  
						const int ONE_HEALTH_PIP = -1; 
						for(int i = 0; i < numSlotsToRepair; i++)
						{
							C_VehicleLogicRepairMsg repairMsg(mVehicleIndex, ONE_HEALTH_PIP );
							vehActor->SendMessageToActor(vehActor->GetHandle(), &repairMsg);
						}
						if(g_DebugShowMpAbilityActivateSpam)
						{
							LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Was It Good for You ability has been activated!", mVehicleIndex);
						}
					}*/

					if(pMsg->mPickupType == E_PICKUP_TYPE_SHOCK || pMsg->mPickupType == E_PICKUP_TYPE_SHUNT || pMsg->mPickupType == E_PICKUP_TYPE_BARGE || pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE || pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
					{
						if( mPreviousFiredPickupID == pMsg->mUniqueID )
						{
							mHitsInARow++;
							mHitWithPowerUp=true;
						}
						
						if( mHitsInARow == 3 )
						{
							GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_THREE_IN_A_ROW);
						}
						else if( mHitsInARow == 5 )
						{
							GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_FIVE_IN_A_ROW);
						}
						else if( mHitsInARow == 10 )
						{
							GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_TEN_IN_A_ROW);
						}
					}
					
					if(m_pScoring->mRevengeTimer && m_pScoring->mRevengeIndex == pMsg->mVehicleIndex)
					{
						GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_REVENGE_ATTACK);

						C_VehicleLogicMsg_HitByPerkSpecialRevengeMsg revengeMsg(pMsg->mFiringVehicleIndex, pMsg->mTargetedVehicleIndex);
						Game::C_AchievementsManager::Instance().MessageHandler(&revengeMsg);
					}

					if(mVendettaIndex == pMsg->mVehicleIndex)
					{
						mVendettaCount++;
						mVendettaTime=pScoringConfig->vendetta_cooldown;

						if(mVendettaCount == 2)
						{
							GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_VENDETTA);
						}
						else if(mVendettaCount == 3)
						{
							GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_PERSECUTER);
						}
						else if(mVendettaCount == 4)
						{
							GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_TERRORIZER);
						}
					}
					else
					{
						mVendettaTime = 0.f;
						mVendettaCount = 0;
						mVendettaIndex = pMsg->mVehicleIndex;
					}

					mHitVehicleWithPerkTimer[pMsg->mVehicleIndex]=pScoringConfig->hit_vehicle_with_perk_timer;
					mHitVehicleWithPerkType[pMsg->mVehicleIndex] = pMsg->mPickupType;

					switch(pMsg->mPickupType)
					{
						case E_PICKUP_TYPE_SHUNT:
							{
								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
									GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_LEVEL2SHUNT);
								else
									GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_SHUNT);

								CalculateSkillShots(pMsg->mPickupType);

								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ALT_FIRE);
								}

								Game::Actors::C_VehicleLogicActor* otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(pMsg->mVehicleIndex);
								if(otherVehActor->GetScoring().mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ENEMY_IN_AIR);
								}

								mLastShuntedIdx = pMsg->mVehicleIndex;
								mShuntedTimer = 3.0f;

								mpLastShuntedActor = mRaceLogicLayerPtr->GetLogicVehicleActor(mLastShuntedIdx);
							}
							break;

						case E_PICKUP_TYPE_SHOCK:
							if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
								GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_LEVEL2SHOCK);
							else
								GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_SHOCK);

							mShockedTimer = 3.0f;

							break;

						case E_PICKUP_TYPE_BARGE:
							{
								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
								{
									if(mLastBargeID == pMsg->mBargeID)
										GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_DOUBLE_BARGE);
									else
										GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_LEVEL2BARGE);
								}
								else
								{
									//if(mLastBargeID == pMsg->mBargeID)
									//	GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_DOUBLE_BARGE);
									//else
									GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_BARGE);
								}

								CalculateSkillShots(pMsg->mPickupType);

								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ALT_FIRE);
								}

								Game::Actors::C_VehicleLogicActor* otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(pMsg->mVehicleIndex);
								if(otherVehActor->GetScoring().mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ENEMY_IN_AIR);
								}

								mLastBargeID=pMsg->mBargeID;
							}
							break;

						case E_PICKUP_TYPE_BOMB:
							{
								if(pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_DECOY)
								{
									if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
										GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_LEVEL2BOMB);
									else
										GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_BOMB);

									if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
									{
										GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ALT_FIRE);
									}	
	
									Game::Actors::C_VehicleLogicActor* otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(pMsg->mVehicleIndex);
									if(otherVehActor->GetScoring().mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
									{
										GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ENEMY_IN_AIR);
									}
								}
							}
							break;


						case E_PICKUP_TYPE_BLINDFIRE:
							{
								GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_BLINDFIRE);

								CalculateSkillShots(pMsg->mPickupType);

								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ALT_FIRE);
								}

								Game::Actors::C_VehicleLogicActor* otherVehActor = mRaceLogicLayerPtr->GetLogicVehicleActor(pMsg->mVehicleIndex);
								if(otherVehActor->GetScoring().mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_ENEMY_IN_AIR);
								}

								float dist=vehActor->mVehicleProximities[pMsg->mVehicleIndex].mDistanceSquared;

								if(dist > (pScoringConfig->long_distance*pScoringConfig->long_distance))
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_DISTANCE_LONG);
								}
								else if(dist > (pScoringConfig->medium_distance*pScoringConfig->medium_distance))
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_DISTANCE_MEDIUM);
								}
								else if(dist > (pScoringConfig->short_distance*pScoringConfig->short_distance))
								{
									GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_DISTANCE_SHORT);
								}
							}

							break;

						default:
							break;
					}

					if(pMsg->mDroppedByIndex == pMsg->mVehicleIndex)
					{
						// [GavinY] - Knocked this out until Kerbz has a chance to revisit and fix it
						//GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SHOCKED_PICKUP_HIT);
					}
				}

				return true;
			}
		case Game::E_PickupMessage_HitDecoy:
			{
				const Game::Actors::HitDecoyMsg* pMsg	= Messaging::MessageCast<Game::Actors::HitDecoyMsg>(pMessage);
				if(pMsg->mFiringVehicleIndex != mVehicleIndex)
				{
					VictimScore(AMAX_SCORING_EVENT_DECOY);
				}
				else
				{
					//oops, we hit our own decoy! silly us, we should have remembered it was there since we dropped it...
				}

				return true;
			}

		case E_VehicleLogicMsg_PerkEnded:
			{
				const C_VehicleLogicMsg_PerkEnded* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_PerkEnded>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
				}
			
				return true;
			}

		case E_VehicleLogicMsg_ShieldedPerk:
			{
				const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor		= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if( pMsg->mDefendingVehicleIndex == mVehicleIndex )
				{
					if(mShieldedFromVehicle != pMsg->mFiringVehicleIndex && mShieldedFromPerkUniqueID != pMsg->mUniqueID)
					{
						mShieldedFromVehicle=pMsg->mFiringVehicleIndex;
						mShieldedFromPerkUniqueID=pMsg->mUniqueID;

						const float time=pMsg->mTime;

						if(pMsg->mLevel > 0)
						{
							GrantVehicleXP(pMsg->mFiringVehicleIndex, AMAX_SCORING_EVENT_SHIELD_PENETRATED);
						}
						else if(time <= 0.75f && time >= pScoringConfig->shield_time_superquick)
						{
							GrantVehicleXP(pMsg->mFiringVehicleIndex, AMAX_SCORING_EVENT_SUPERQUICKSHIELD);
						}
						else if(time < pScoringConfig->shield_time_superquick && time >= pScoringConfig->shield_time_quick)
						{
							GrantVehicleXP(pMsg->mFiringVehicleIndex, AMAX_SCORING_EVENT_QUICKSHIELD);
						}
						else
						{
							GrantVehicleXP(pMsg->mFiringVehicleIndex, AMAX_SCORING_EVENT_SHIELD);
						}
					}

					if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
					{
						C_VehicleLogicMsg_HitByPerkSpecialEventShieldedMsg shieldedMsg(mVehicleIndex, pMsg->mFiringVehicleIndex);
						_UI_MESSAGE_FULL( shieldedMsg );
					}
				}
				else if( pMsg->mFiringVehicleIndex == mVehicleIndex )
				{
					if(pMsg->mLevel > 0)
						GrantVehicleXP(pMsg->mDefendingVehicleIndex, AMAX_SCORING_EVENT_ENEMY_SHIELD_PENETRATED);
					else
						GrantVehicleXP(pMsg->mDefendingVehicleIndex, AMAX_SCORING_EVENT_ENEMY_SHIELDED);
				}

				return true;
			}

		case Game::E_VehicleMessage_Overtake:
			{
				PERF_EVENT("C_VehicleComponent.Scoring.Overtake");

				const Game::Actors::C_VehicleOvertakeMsg* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleOvertakeMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor		= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(pMsg->mOvertakerVehicleIndex == pMsg->mOvertakeeVehicleIndex)
					return true;

				if(mUsingNitroTimer)
				{
					if(pMsg->mOvertakerVehicleIndex == mVehicleIndex)
					{
						mCarsOvertakenWithNitro++;
						mLastOvertakenVehicle = pMsg->mOvertakeeVehicleIndex;

						if(Game::C_InGameDialogueManager::Instance().IsEnabled())
						{
							Game::Actors::C_VehicleOvertakeWithNitroMsg nitroMsg(pMsg->mOvertakerVehicleIndex, pMsg->mOvertakeeVehicleIndex);
							Game::C_InGameDialogueManager::Instance().MessageHandler(&nitroMsg);
						}

						if(mShuntedTimer)
						{
							GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SHUNT_OVERTAKE);
						}
						if(mShockedTimer)
						{
							GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SHUNT_OVERTAKE);
						}
					}
					else if(pMsg->mOvertakeeVehicleIndex == mVehicleIndex)
					{
						mCarsOvertakenWithNitro--;
						if(mCarsOvertakenWithNitro < 0) 
							mCarsOvertakenWithNitro=0;
					}
				}

				return true;
			}

		case Game::E_VehicleMessage_FatallyDamaged:
			{
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(vehActor->mVehicleIndex == pMsg->m_VehicleIndex)
				{
					if(m_pScoring->mWreckedTimer > 0.f && m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX && m_pScoring->mMostRecentVehicleIdxHitBy != mVehicleIndex && (mHitByVehicleRecentlyTimer > 0 || mHitByPerkRecentlyTimer > 0))
					{
						mNoRepairOrWreck=false;

						VictimScore(AMAX_SCORING_EVENT_WRECKED_A_CAR);
						m_pScoring->mWreckedTimer = 0.f;
						m_pScoring->mWreckedByAnotherPlayer = true;

						Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
						if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
						{
							UI::C_WideString128 str;
							str.Clear();
							str.Format(UI::C_TextStringID("MP_RIVAL_ATTACK_PROMPT").String(), levelState.mCars[m_pScoring->mMostRecentVehicleIdxHitBy].mNameAboveCar.Chars());

							// PR - this is no longer supported
							//UI::C_UiGenericMessage uiMsg(str, vehActor->mVehicleIndex, true);
							//_UI_MESSAGE_FULL( uiMsg );

							// Record our new rival
							SetRivalIndex(m_pScoring->mMostRecentVehicleIdxHitBy);
							
						}
					}
				}

				return true;
			}

		case E_VehicleLogicMsg_Repair:
			{
				const C_VehicleLogicRepairMsg* pMsg		= Messaging::MessageCast<C_VehicleLogicRepairMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					if(vehActor->GetCurrentHealth() < pScoringConfig->late_repair_level)
					{
						GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_CLOSEREPAIR);
					}
					else
					{
						GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_REPAIR);
					}

					mNoRepairOrWreck=false;
				}
		
				return true;
			}

		case E_VehicleLogicMsg_SurvivedElimination:
			{
				//	This enum gets sent directly to the correct vehicle's scoring component so no checking is required
				GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_SURVIVED_ELIMINATION);
				return true;
			}

		case Game::E_GameLogicMessage_GameplayImpact:
			{
				const Game::C_GameplayImpactMsg* pMsg		= Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if( pMsg->mVehicleIdx != mVehicleIndex )
					return true;

				if(m_pScoring->mTimeInAir > 0 && vehActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
				{
					C_VehicleLogicMsg_LandedMsg msg(mVehicleIndex, m_pScoring->mTimeInAir);
					GetActor()->SendMessageToComponent(E_IF_VehicleComponent_Damage, &msg);
				}

				if(pMsg->mImpactLevel > E_IMPACT_LEVEL_LIGHT && pMsg->mVehicleIdx == mVehicleIndex)
				{
					//hit the world
					mUsingNitroDamage+=pMsg->mDamage;

					if(vehActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED && m_pScoring->mCrashedOutTimer > 0.f && 
						m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX && m_pScoring->mMostRecentVehicleIdxHitBy != mVehicleIndex)
					{
						VictimScore(AMAX_SCORING_EVENT_CRASHED_OUT_A_CAR);
						m_pScoring->mCrashedOutTimer = 0.f;
					}
				}

				if(pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX && pMsg->mAggressiveVehicleIdx == pMsg->mVehicleIdx)
				{
					Game::Actors::C_VehicleLogicActor* otherVehActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(pMsg->mOtherVehicleIdx);
					// We've aggressively hit another vehicle
					if(pMsg->mImpactLevel == E_IMPACT_LEVEL_HEAVY)
					{
						if(  otherVehActor->mShieldTime == 0.f )
						{
							HandleMotorMashMessageAggressiveImpact(pMsg->mOtherVehicleIdx, pMsg->mAggressiveVehicleIdx, pMsg->mImpactLevel);
						}

						if(mUsingNitroTimer)
						{
							GrantVehicleXP(pMsg->mOtherVehicleIdx, AMAX_SCORING_EVENT_BATTERING_RAM);
						}
						else
						{
							GrantVehicleXP(pMsg->mOtherVehicleIdx, AMAX_SCORING_EVENT_HIT_A_CAR);
						}
						vehActor->mStatistics.mCarsRammed++;
					}
					else if(pMsg->mImpactLevel == E_IMPACT_LEVEL_MEDIUM && otherVehActor->mShieldTime == 0.f )
					{
						HandleMotorMashMessageAggressiveImpact(pMsg->mOtherVehicleIdx, pMsg->mAggressiveVehicleIdx, pMsg->mImpactLevel);
					}
				}

				if( pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX && pMsg->mAggressiveVehicleIdx == pMsg->mOtherVehicleIdx )
				{
					//Hit by another car
					vehActor->ResetHitBy(pMsg->mAggressiveVehicleIdx);
					mHitByVehicleRecentlyTimer = g_RecentlyHitByPerkTime;
				}

				if( mHitByPerkRecentlyTimer > 0 && pMsg->mAggressiveVehicleIdx != m_pScoring->mMostRecentVehicleIdxHitBy && pMsg->mOtherVehicleIdx != m_pScoring->mMostRecentVehicleIdxHitBy )
				{
					C_VehicleLogicMsg_HitByPerkSpecialEventCollidedMsg specialCollideMsg(mVehicleIndex, m_pScoring->mMostRecentVehicleIdxHitBy);

					if( pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX )	
						specialCollideMsg.mCollidedVehicleIndex = pMsg->mOtherVehicleIdx;

					if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
					{
						_UI_MESSAGE_FULL( specialCollideMsg );
					}

					AmaxDataLogging::LogPerkCausedCrash(m_pScoring->mMostRecentVehicleIdxHitBy, mVehicleIndex, mRaceLogicLayerPtr->ConvertPickupTypeToString(mMostRecentPerkHitBy));

					//if(Game::C_InGameDialogueManager::Instance().IsEnabled())
					//	Game::C_InGameDialogueManager::Instance().MessageHandler(&specialCollideMsg);

					Game::C_AchievementsManager::Instance().MessageHandler(&specialCollideMsg);
				}

				return true;
			}

		case E_VehicleLogicMsg_HOGReset:
			{
				const C_VehicleLogicMsg_HoGResetMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HoGResetMsg>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					mHOGTimer=0.25f;

					if(mComboTimer)
						mComboTimer=1/120.f;
				}

				return true;
			}

		case E_VehicleLogicMsg_Fan_Demand_Passed:
			{
				const C_VehicleLogicMsg_Fan_Demand_Passed* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_Fan_Demand_Passed>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					mFanDemandFans=pMsg->mFans;
					GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_FAN_DEMAND_COMPLETE);
				}

				return true;
			}

		case E_VehicleLogicMsg_Recovered:
			{
				const C_VehicleLogicMsg_HoGRecoveryMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HoGRecoveryMsg>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_RECOVERY);
				}

				return true;
			}

		case ActorMsg::E_DynamicObjMsg_VehicleSloMoEntry:
			{
				GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_MEGA_JUMP);
				return true;
			}

		case E_VehicleLogicMsg_AggressiveScore:
			{
				const C_VehicleLogicMsg_AggressiveScore* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AggressiveScore>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					GrantVehicleXP(pMsg->mAboutVehicleIndex, pMsg->mScoringEvent);

					if(pMsg->mScoringEvent == AMAX_SCORING_EVENT_DROWN)
					{
						Game::C_AchievementsManager::Instance().DrownVehicle(mVehicleIndex, pMsg->mAboutVehicleIndex);
					}
				}

				return true;
			}

		case Game::E_GameflowMessage_RaceStarted:
			{
				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
				AmaxEvent *ev=C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];

				mRaceStarted=true;
				mRaceStartedTimer=g_debugRaceStartedTimer;

				if(ev && ev->disableFans)
				{
					mFansDisabled=true;
				}

				return true;
			}

		case E_VehicleLogicMsg_AddOnTrackScore:
			{
				const C_VehicleLogicMsg_AddOnTrackScore* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AddOnTrackScore>(pMessage);

				if( mVehicleIndex == pMsg->mVehicleIndex )
					GrantVehicleXP(mVehicleIndex, pMsg->mScoringEvent);

				return true;
			}

		case Game::E_GameflowMessage_VehicleFinished:
		case Game::E_GameflowMessage_RaceFinished:
			{
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
				m_pScoring->mFinalScore=mCurrentXP;
				m_pScoring->mFinalScoreForUI=m_pScoring->mFinalScore;

				mRaceStarted=false;
				
				return true;
			}

		case E_VehicleLogicMsg_VehicleMurdered:
			{
				//There's been a murrrduurrr!
				VictimScore(AMAX_SCORING_EVENT_WRECKED_A_CAR);

				return true;
			}

		case E_VehicleLogicMsg_EvadedPowerUp:
			{
				const C_VehicleLogicMsg_EvadedPowerUp* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_EvadedPowerUp>(pMessage);

				if( mVehicleIndex == pMsg->mVehicleIndex )
				{
					Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
					if( pMsg->mPickupType == E_PICKUP_TYPE_SHOCK)
					{
						GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_EVADED_SHOCK );
					}
					else if( pMsg->mPickupType == E_PICKUP_TYPE_SHUNT)
					{
						GrantVehicleXP(mVehicleIndex, AMAX_SCORING_EVENT_EVADED_SHUNT );
					}
					vehActor->mStatistics.mPickupsEvaded++;
				}
				
				return true;
			}

		case E_VehicleLogicMsg_PowerUpDeleted:
			{
				const C_VehicleLogicMsg_PowerUpDeleted* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_PowerUpDeleted>(pMessage);

				if(pMsg->mPickupType == E_PICKUP_TYPE_SHOCK || pMsg->mPickupType == E_PICKUP_TYPE_SHUNT || pMsg->mPickupType == E_PICKUP_TYPE_BARGE || pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE || pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
				{
					if( mPreviousFiredPickupID == pMsg->mUniqueId && !mHitWithPowerUp)
					{
						mHitsInARow=0;
					}
				}

				return true;
			}

		case E_VehicleLogicMsg_SideSwipe:
			{
				const C_VehicleLogicMsg_SideSwipe* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_SideSwipe>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(pMsg->mImpact > pScoringConfig->sideswipe_impact)
					{
						GrantVehicleXP(pMsg->mSideSwipedVehicleIndex, AMAX_SCORING_EVENT_BATTLING );
					}
				}

				return true;
			}
		case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(mNoRepairOrWreck && pMsg->mLap > 1)
					{
						GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_NO_REPAIR_LAP );
					}

					mNoRepairOrWreck=true;
				}

				return true;
			}

		case E_VehicleLogicMsg_PowerUpDestroyedByPowerUp:
			{
				const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_PowerUpDestroyedByPowerUp>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					GrantVehicleXP(pMsg->mVehicleIndex, AMAX_SCORING_EVENT_SKILL_SHOT_HIT_ANOTHER_POWERUP );
				}

				return true;
			}

		return I_GameActorComponent::OnMessage(pMessage);
	}

	return false;
}

bool C_VehicleComponent_Scoring::WreckedByAnotherPlayer() const 
{
	return  m_pScoring ? m_pScoring->mWreckedByAnotherPlayer : false; 
}

void C_VehicleComponent_Scoring::SetWreckedByAnotherPlayer(bool wrecked)
{ 
	if(m_pScoring){ m_pScoring->mWreckedByAnotherPlayer = wrecked; }
}
S32 C_VehicleComponent_Scoring::GetIndexOfVehicleThatWreckedUs() const 
{ 
	return m_pScoring ? m_pScoring->mMostRecentVehicleIdxHitBy : -1;
}

void C_VehicleComponent_Scoring::SetRivalIndex(S8 rivalIndex)
{
	// Store the new index
	m_pScoring->mRivalIndex = rivalIndex;

	// Send a UI message to inform the UI that this has changed?
}

S8 C_VehicleComponent_Scoring::GetRivalIndex()
{
	return m_pScoring->mRivalIndex;
}

void C_VehicleComponent_Scoring::AwardLowDamageScore(int& ourScore, const MotorMashScoringValues& pScoringValues, Game::Actors::C_VehicleLogicActor& pLogicActor)
{
	UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashLightImpactActionIn );
	ourScore += pScoringValues.m_LowDamageScore;
	pLogicActor.mStatistics.mNumLowDamageAttacks++;
}

void C_VehicleComponent_Scoring::AwardHighDamageScore(int& ourScore, const MotorMashScoringValues& pScoringValues, Game::Actors::C_VehicleLogicActor& pLogicActor)
{
	UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashHeavyImpactActionIn );
	ourScore += pScoringValues.m_HighDamageScore;
	pLogicActor.mStatistics.mNumHighDamageAttacks++;
}


} // namespace Components
