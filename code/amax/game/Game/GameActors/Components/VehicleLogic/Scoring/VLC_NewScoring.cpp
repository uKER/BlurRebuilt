#include "stdafx.h"

#include <ui/Game/StringStore.h>

#include "VLC_NewScoring.h"

#include <Automation\GameDataLogging.h>
#include <Game/Messaging/GameMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <ui/UiMessages.h>
#include <ui/UiHelpers.h>

#include <Game/Messaging/PickupActorMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <Vehicle/VehicleModel.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/AchievementsManager.h>
#include <GameLogic\StickerManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/VehicleLogic/Multiplayer/VLC_MultiplayerChallenges.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_FanDemands.h>

#include <GameLogic\Structures\levelState.h>

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

#include "GameLogic/Structures/MultiplayerAbilities.h"
#include <GameLogic\Structures\NetVehicleData.h>
#include <GameLogic\Structures\MpResults.h>

#include <Game\Achievements\AvatarAwardSystem.h>
#include <GameLogic\Pickups\Upgraded\PickupBomb.h>






Debug::Logger &gVLC_NewScoringLogger( Debug::GetLogger("Vehicle.NewScoring") );

Debug::Int g_debug_NewScoring_OnlyLogPLayer("Vehicle.Components.NewScoring.OnlyLogPLayer", 0);

Debug::Float g_debug_NewScoring_UIScreenTimer("Vehicle.Components.NewScoring.UIScreenTimer", 3.0f);

Debug::Bool g_debug_NewScoring_GiveOvertake("Vehicle.Components.NewScoring.GiveOvertake", false);
Debug::Bool g_debug_NewScoring_GiveWreck("Vehicle.Components.NewScoring.GiveWreck", false);
Debug::Bool g_debug_NewScoring_GiveShunt("Vehicle.Components.NewScoring.GiveShunt", false);

Debug::Bool g_debug_Motormash_GiveRivalPickupHit("Multiplayer.MotorMash.GiveRivalPickupHit", false);
Debug::Bool g_debug_Motormash_GiveRivalPhysicalHit("Multiplayer.MotorMash.GiveRivalPhysicalHit", false);
Debug::Bool g_debug_Motormash_GivePickupHit("Multiplayer.MotorMash.GivePickupHit", false); 
Debug::Bool g_debug_Motormash_GivePhysicalHit("Multiplayer.MotorMash.GivePhysicalHit", false);

extern Debug::Float g_RecentlyHitByPerkTime;
extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

namespace Components
{

bool C_VehicleComponent_NewScoring::CreateImpl()
{
	const I_VehicleComponent_NewScoring_CreateInfo* pCreation_data = static_cast<const I_VehicleComponent_NewScoring_CreateInfo *>(GetCreator().GetData());
	BIZ_ASSERT_MSG(pCreation_data, "C_VehicleComponent_NewScoring::CreateImpl() > Couldn't get a pointer to the creation data, this is going to go badly");

	Game::Actors::C_VehicleLogicActor* vehActor			= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	//Cache the stuff in the creation data
	mVehicleIndex = pCreation_data->mVehicleIndex;
	mRaceLogicLayerPtr = pCreation_data->mpRaceLogicLayer;

	m_pScoring = pCreation_data->mpScoringInfo;

	m_pMpConfig = C_MultiplayerData::Instance().m_pConfig;

	AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_PerkEnded );

	AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
	AddSubscriber( E_VehicleLogicMsg_Repair );

	AddSubscriber( E_VehicleLogicMsg_PowerUpDeleted );

	AddSubscriber( Game::E_GameLogicMessage_GameplayImpact );

	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	
	AddSubscriber( Game::E_GameflowMessage_VehicleFinished );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );

	AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );

	AddSubscriber( E_VehicleLogicMsg_AggressiveScore );

	AddSubscriber( E_VehicleLogicMsg_PowerUpDestroyedByPowerUp );

	AddSubscriber( E_VehicleLogicMsg_EvadedPowerUp );

	AddSubscriber( Game::E_GameLogicMessage_LapIncremented );

	AddSubscriber( E_VehicleLogicMsg_Fan_Demand_Passed );

	AddSubscriber( Game::E_VehicleMessage_ResetAfterWrecked );

	AddSubscriber( E_VehicleLogicMsg_HOGReset );


	AddSubscriber(Game::E_VehicleMessage_LeftRace);

	AddSubscriber( Game::E_PickupMessage_CollectStopwatch );

	// Cache level state reference
	mpLevelState = &Game::C_Game::GetInstance()->GetLevelState();
	BIZ_ASSERT_MSG(mpLevelState, "C_VehicleComponent_Scoring::CreateImpl() > Couldn't get a pointer to the levelState, this is going to go badly");

	mRaceStarted=false;

	mTotalScore=0;
	mLastScore=0;
	mScoreUITimer=0.f;
	mCurrentScoreBase=E_BLUR_SCORE_MOVE_BASE_NOTHING;

	mHitsInARow=0;
	mPreviousFiredPickupID=-1;
	mHitWithPowerUp=false;

	mHitSameVehicleInARow=0;

	mWrecksInARow=0;

	mVendettaIndex=-1;
	mVendettaCount=0;
	mVendettaTime=0.f;

	mDrifting=false;
	mDriftDistance=0.f;



	mUsingNitroAltType=false;
	mGotFirstPlaceDominator1=false;
	mGotFirstPlaceDominator2=false;

	for(U32 a=0;a<MAX_VEHICLES;a++)
	{
		mGotPlace[a]=false;
		if(a >= mpLevelState->mNumCars)
		{
			mGotPlace[a]=true;
		}
	}

	mInFirstPlace=false;
	mTimeInFirstPlace=0.f;

	mGotFirstSpeed=false;
	mGotSecondSpeed=false;
	mGotThirdSpeed=false;
	mGotTopSpeed=false;

	mShieldedFromVehicle=0;
	mShieldedFromPerkUniqueID=0;
	mShieldedTime=0.f;

	mNoRepairOrWreck=true;
	mNitroDamageReceived=false;
	mLapDamageReceived=false;

	mJumpDistance=0.f;
	mHOGTimer=0.f;

	mNumBarges=0;
	mFirstBargeIndex=-1;

	mBargeDrift=false;
	mBargeNitrous=false;
	mBargeAir=false;

	mNitrousAir=false;
	mRepeatNitros=0;

	mFanDemandsInARow=0;

	mPowerUpFireStates.Clear();
	mPowerUpBoltStates.Clear();
	mShuntsDestroyed.Clear();

	mCombo=0;
	mComboTimer=0.f;
	
	m_pScoring->mCombo=mCombo;
	m_pScoring->mComboTimer=mComboTimer;

	mCheckpointsPassed	= 0;
	mCurrentRaceRuleArea= -1;

	mCheckpointsDamageRecieved=false;
	
	mAttemptedFanRun=false;

	mNitrousOvertakes = 0;

	for(int a=0;a<MAX_VEHICLES;a++)
	{
		mLastHitVehicleTimers[a]=0.f;
		mLastHitByVehicleTimers[a]=0.f;
	}

	// Cache pointer to owner vehicle's abilities
	mpMultiplayerAbilities = mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
	BIZ_ASSERT_MSG(mpMultiplayerAbilities,"Error.. VLC_Scoring CreateImpl() unable to obtain pointer to Multiplayer Abilities");

	mIsMotorMash = mpLevelState->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction;

	mGivePositionandOvertake=true;
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer() || mpLevelState->mRaceMode == Game::E_RaceMode_Destruction || mpLevelState->mRaceMode == Game::E_RaceMode_FanDemand 
						|| mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_Destruction || mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_FanDemand)
	{
		mGivePositionandOvertake=false;
	}

	mIsSPDestruction=false;
	if(mpLevelState->mRaceMode == Game::E_RaceMode_Destruction || mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_Destruction)
	{
		mIsSPDestruction=true;
	}

	// Initialise Rivals Logic
	// Seed rand based on time + joinID. 
	mRandomGenerator.Seed((U32)Time::GetCurrentTime() * mpLevelState->mCars[mVehicleIndex].mNetJoinId);
	mIsLocalGame =  !C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork();
	mRivalDamageScoreMultiplier = m_pMpConfig->m_MotormashScoringValues.m_RivalDamageMultiplier;

	// Drifter fx
	mTimeTillNextDrifterFX = 0.0f; 

	mMaxLapReached = 1;

	mDrifterAbility.Init(mVehicleIndex, &Game::C_Game::GetInstance()->GetMultiplayerAbilities(), m_pMpConfig, vehActor, this, mpLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities() );
	mFreeFallingOverCliffTimer = 0.0f;

	mHasDrowned=false;
	mHasPlummeted=false;

	mNumBoltsPerSet = 3;
	Game::C_MultiplayerVehicleAbility* pAdditionalBlindfireShotsAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots);
	if(pAdditionalBlindfireShotsAbility && pAdditionalBlindfireShotsAbility->GetActive())
		mNumBoltsPerSet += pAdditionalBlindfireShotsAbility->GetValue();

	return true;
}

void C_VehicleComponent_NewScoring::Update()
{
	PERF_EVENT("C_VehicleComponent.NewScoring.Update");

	if(!mRaceStarted)
		return;

	float frameTime	= C_GameApp::GetInstance()->GetTimeStep();
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	bool is_multiplayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer();

	if (mVehicleIndex == 0)						// Only monitoring player overtakes
	{
		int overtakes=C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetNumberOfPlayerOvertakes();
		mNitrousOvertakes += overtakes;

		//just send this message to the fan demands so its instant
		if(overtakes && vehActor->GetScoring().mUsingNitroTimer)
		{
			Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> skills;
			Btl::Vector<E_BLUR_SCORE_MOVE_BONUS> bonus;

			skills.Clear();
			bonus.Clear();

			if(mNitrousOvertakes < 5)
				skills.PushBack(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
			else if(mNitrousOvertakes < 10)
				skills.PushBack(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
			else if(mNitrousOvertakes < 15)
				skills.PushBack(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
			else
				skills.PushBack(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4);

			const C_VehicleLogicMsg_Scored msg(mVehicleIndex, E_BLUR_SCORE_MOVE_BASE_OVERTAKE, skills, bonus);
			Components::C_VehicleComponent_Fan_Demand* pFanDemand = (Components::C_VehicleComponent_Fan_Demand*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand);
			if(pFanDemand)
				pFanDemand->OnMessage(&msg);
		}
	}

	if(mTimeTillNextDrifterFX > 0.0f)
	{
		mTimeTillNextDrifterFX -= frameTime;
		if(mTimeTillNextDrifterFX < 0.0f)
		{
			mTimeTillNextDrifterFX = 0.0f; 
		}
	}

#ifndef CFG_GOLD
	UpdateMotormashDebugValues();
#endif // #ifndef CFG_GOLD

	if(g_debug_NewScoring_GiveOvertake)
	{
		StartScore(E_BLUR_SCORE_MOVE_BASE_OVERTAKE, 1);
		AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
		EndScore();

		g_debug_NewScoring_GiveOvertake=false;
	}

	if(g_debug_NewScoring_GiveShunt)
	{
		StartScore(E_BLUR_SCORE_MOVE_BASE_SHUNT, 1);
		EndScore();

		g_debug_NewScoring_GiveShunt=false;
	}

	if(g_debug_NewScoring_GiveWreck)
	{
		GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_WRECKED, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, 1);

		g_debug_NewScoring_GiveWreck=false;
	}

	if(mScoreUITimer)
	{
		mScoreUITimer-=frameTime;
		if( mScoreUITimer <= 0.f )
		{
			UI::C_UiScoringEventPerk uimsg1( L"", -1, mCurrentScore, mCurrentScore, mVehicleIndex );
			_UI_MESSAGE_FULL( uimsg1 );

			mCurrentScore=0;
			mScoreUITimer=0.f;
		}
	}

	if(vehActor->mShieldTime == 0.f)
	{
		mShieldedFromVehicle=mVehicleIndex;		//make it you so we test against it later on
		mShieldedFromPerkUniqueID=0;
	}

	for(int a=0;a<MAX_VEHICLES;a++)
	{
		if(mLastHitVehicleTimers[a])
		{
			mLastHitVehicleTimers[a]-=frameTime;
			if( mLastHitVehicleTimers[a] < 0.f )
			{
				mLastHitVehicleTimers[a]=0.f;
			}
		}
		if(mLastHitByVehicleTimers[a])
		{
			mLastHitByVehicleTimers[a]-=frameTime;
			if( mLastHitByVehicleTimers[a] < 0.f )
			{
				mLastHitByVehicleTimers[a]=0.f;
			}
		}
	}

	if(mHOGTimer)
	{
		mHOGTimer-=frameTime;
		if(mHOGTimer < 0.f)
		{
			mHOGTimer=0.f;
		}
	}

	if(mVendettaTime)
	{
		mVendettaTime -= frameTime;
		if(mVendettaTime <=0.f)
		{
			mVendettaTime=0.f;
			mVendettaCount=0;

			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Vendetta against %d Finished", mVehicleIndex, mVendettaIndex );
		}
	}

	if(m_pScoring->mRevengeTimer)
	{
		m_pScoring->mRevengeTimer-=frameTime;

		if(m_pScoring->mRevengeTimer < 0.f)
		{
			m_pScoring->mRevengeTimer=0.f;
	
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Revenge against %d Finished", mVehicleIndex, m_pScoring->mRevengeIndex );
		}
	}

	// If we have been falling off a cliff.. 
	if(mHasPlummeted == false && mHasDrowned == false)
	{
		CheckFreeFallingCliffTimer(frameTime); 
	}

	if(vehActor->GetGraphicalActor(mVehicleIndex)->InWater() && mHasDrowned == false && mHasPlummeted == false )
	{
		if(m_pScoring->mDrownedTimer > 0.f && m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX && m_pScoring->mMostRecentVehicleIdxHitBy != Game::SYSTEM_FIRING_INDEX)
		{
			BlurScoreMoveSummary summary;
			summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_DROWN;

			const Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			const AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
			if (!pCity->forceFallIntoWater)					// Bodge because drown timer is not reliable! CANT HAVE BOTH ON IN THE SAME CITY
				summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_CLIFF;

			VictimBonusScore(summary);
			m_pScoring->mDrownedTimer = 0.f;

			mHasDrowned = true;

			// If we landed in water... then if we were previously unsure on wether to award a cliff or drown score.. there is now no doubt
			// give the drown score and forget about the cliff score!. 
			mFreeFallingOverCliffTimer = 0.0f;
			vehActor->GetGraphicalActor(mVehicleIndex)->SetGoneOffTrack(false);
		}
	}

	if(vehActor->GetGraphicalActor(mVehicleIndex)->HasGoneOffTrack() && mHasPlummeted == false && mHasDrowned == false )
	{
		// Then we have gone off a cliff...
		// However.. we dont know until we LAND wether or not we've landed in water.. in which case this isnt a cliff scoring move.. its a drown...
		if(m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX && m_pScoring->mMostRecentVehicleIdxHitBy != Game::SYSTEM_FIRING_INDEX && (mLastHitByVehicleTimers[m_pScoring->mMostRecentVehicleIdxHitBy] > 0) )
		{
			// Lets flag ourselves as having fallen off a cliff.. 
			// if we dont hear a *splash* into some water soon, then we will just chalk this one down to a cliff score
			if(m_pScoring->mDrownedTimer > 0.0f)
			{
				mFreeFallingOverCliffTimer = m_pScoring->mDrownedTimer; // only ever have to listen for this long! After this reaches 0 it cant be a drown anyway!
			}
			else
			{
				// If drowned timer is 0 or less.. then no chance of getting a drown. Cliff Ftw.
				mHasPlummeted = true;

				BlurScoreMoveSummary summary;
				summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_CLIFF;

				const Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
				const AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
				if (pCity->forceFallIntoWater)					// Bodge because drown timer is not reliable! CANT HAVE BOTH ON IN THE SAME CITY
					summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_DROWN;
				

				VictimBonusScore(summary);

			}
		}
		vehActor->GetGraphicalActor(mVehicleIndex)->SetGoneOffTrack(false);
	}


	//jumping & drifting
	const Vehicle::C_ModelInput *pModelInput = vehActor->GetGraphicalActor(mVehicleIndex)->GetVehicleInput();

	int prev_num_wheels_on_ground=mNumWheelsOnGround;

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
		if(prev_num_wheels_on_ground)
		{
			if(vehActor->GetScoring().mUsingNitroTimer)
				mJumpedWithNitrous=true;
			else
				mJumpedWithNitrous=false;
		}

		mJumpDistance+=vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate;
		m_pScoring->mTimeInAir += frameTime;
	}
	else
	{
		if(mJumpDistance > pScoringConfig->minimum_jump_distance && mHOGTimer == 0.f)
		{
			StartScore(E_BLUR_SCORE_MOVE_BASE_JUMPING, mVehicleIndex);
			if(mJumpedWithNitrous)
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
			else
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
			EndScore();
		}
		mJumpDistance=0.f;

		if(m_pScoring->mTimeInAir > 0)
		{
			if(m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
			{
				Components::C_VehicleComponent_MP_Challenges* pChallenges = (Components::C_VehicleComponent_MP_Challenges*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges);
				if(pChallenges)
				{
					C_VehicleLogicMsg_LandedMsg msg(mVehicleIndex, m_pScoring->mTimeInAir);
					pChallenges->OnMessage(&msg);
				}
	
				BossDemandAction(E_BOSSDEMAND_ACTION_AIR_TIME, m_pScoring->mTimeInAir);
			}


			m_pScoring->mTimeInAir = 0;
		}
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
			if(!mDrifting && mDriftDistance <= 0.001f)
			{
				CheckDrifter(false, E_BLUR_SCORE_MOVE_SKILL_DRIFT);
			}
			mDrifting=true;
			mDriftDistance+=vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate;

			//Check to see if we have gone over a drifter firing boundary
			if(mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Drifter)->GetActive())
			{
				XtArray<F32>& drifterLengths = m_pMpConfig->m_MiscModSettings.m_DrifterLengths;
				for(S8 i = drifterLengths.count-1; i >= 0; i--)
				{
					if(mDriftDistance > drifterLengths[i])
					{
						if(mDriftDistance - vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate <= drifterLengths[i])
						{
							CheckDrifter(true, E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
						}
						break;
					}
				}
			}
		}
	}


	if(!mDrifting && mDriftDistance)
	{
		if(mDriftDistance > pScoringConfig->drift_distance_to_score)
		{
			StartScore(E_BLUR_SCORE_MOVE_BASE_DRIFTING, mVehicleIndex);

			if(mDriftDistance > pScoringConfig->mega_drift_distance_to_score)
			{
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
			}
			else if(mDriftDistance > pScoringConfig->super_drift_distance_to_score)
			{
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
			}
			else 
			{
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
			}

			EndScore();
	
			BossDemandAction(E_BOSSDEMAND_ACTION_DRIFTED, mDriftDistance);
		}

		if(mDriftDistance > vehActor->mStatistics.mLongestDrift)
		{
			vehActor->mStatistics.mLongestDrift = mDriftDistance;
		}

		mDriftDistance=0.f;
	}


	if (mDrifting)
		vehActor->GetScoring().mValidDriftDistance = mDriftDistance;
	else
		vehActor->GetScoring().mValidDriftDistance = 0.0f;


	if(vehActor->GetScoring().mUsingNitroTimer)
	{
		vehActor->GetScoring().mUsingNitroTimer-=frameTime;
		if(vehActor->GetScoring().mUsingNitroTimer < 0.f)
		{
			if(!mNitroDamageReceived)
			{
				if(mNitrousAir)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_NITROUS, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_AIR);
				}
				else
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);

					BossDemandAction(E_BOSSDEMAND_ACTION_CLEAN_NITRO);
					if (mRepeatNitros >= 3)
						BossDemandAction(E_BOSSDEMAND_ACTION_CLEAN_TRIPLE_NITRO);
				}

				EndScore();
			}

			if(mNitrousOvertakes > 0)
			{
				StartScore(E_BLUR_SCORE_MOVE_BASE_OVERTAKE, mVehicleIndex);

				if(mNitrousOvertakes < 5)
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
				else if(mNitrousOvertakes < 10)
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
				else if(mNitrousOvertakes < 15)
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
				else
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4);

				EndScore();
			}

			vehActor->GetScoring().mUsingNitroTimer=0.f;
			mRepeatNitros = 0;
		}
	}

	mShieldedTime += frameTime;

	if(mGivePositionandOvertake)
	{
		int pos_index=vehActor->mPositionInfo.mSpRacePosition-1;

		if( pos_index >= 0)
		{
			if(!mGotPlace[pos_index] && vehActor->mPositionInfo.mSpRacePosition < 2 && mpLevelState->mNumCars > 1)
			{
				mGotPlace[pos_index]=true;
				StartScore(E_BLUR_SCORE_MOVE_BASE_POSITION, mVehicleIndex);
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
				EndScore();
			}

			if(!mGotPlace[pos_index] && vehActor->mPositionInfo.mSpRacePosition < 3  && mpLevelState->mNumCars > 2)
			{
				mGotPlace[pos_index]=true;
				StartScore(E_BLUR_SCORE_MOVE_BASE_POSITION, mVehicleIndex);
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
				EndScore();
			}

			if(!mGotPlace[pos_index] && vehActor->mPositionInfo.mSpRacePosition < 4  && mpLevelState->mNumCars > 3)
			{
				mGotPlace[pos_index]=true;
				StartScore(E_BLUR_SCORE_MOVE_BASE_POSITION, mVehicleIndex);
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
				EndScore();
			}
		}
	}

	mInFirstPlace=false;
	if(vehActor->mPositionInfo.mSpRacePosition == 1)
	{
		mInFirstPlace=true;
	}

	if(mInFirstPlace && mpLevelState->mNumCars > 1)
	{
		mTimeInFirstPlace += frameTime;

		if(mGivePositionandOvertake)
		{
			if(!mGotFirstPlaceDominator1 && mTimeInFirstPlace > pScoringConfig->first_place_dom_1)
			{
				StartScore(E_BLUR_SCORE_MOVE_BASE_POSITION, mVehicleIndex);
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4);
				EndScore();
				mGotFirstPlaceDominator1=true;
			}
			if(!mGotFirstPlaceDominator2 && mTimeInFirstPlace > pScoringConfig->first_place_dom_2)
			{
				StartScore(E_BLUR_SCORE_MOVE_BASE_POSITION, mVehicleIndex);
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_5);
				EndScore();
				mGotFirstPlaceDominator2=true;
			}
		}
	}
	else
	{
		if ((mRaceStarted) && (mTimeInFirstPlace > 0.0f))				// Don't include any time after the race has finished
			BossDemandAction(E_BOSSDEMAND_ACTION_SECONDS_IN_FIRST, mTimeInFirstPlace);
		mTimeInFirstPlace=0.f;
	}

	if(vehActor->GetScoring().mUsingNitroTimer == 0.f && mNumWheelsOnGround > 0)
	{
//		if (vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_1_mph)					// Only check boss demand if over speed 1
//			BossDemandAction(E_BOSSDEMAND_ACTION_SPEED, vehActor->mPositionInfo.mSpeedInMph);

		if(!mGotFirstSpeed && vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_1_mph)
		{
			StartScore(E_BLUR_SCORE_MOVE_BASE_SPEED, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
			EndScore();
			mGotFirstSpeed=true; 
		}
		if(!mGotSecondSpeed && vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_2_mph)
		{
			StartScore(E_BLUR_SCORE_MOVE_BASE_SPEED, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
			EndScore();
			mGotSecondSpeed=true; 
		}
		if(!mGotThirdSpeed && vehActor->mPositionInfo.mSpeedInMph >= pScoringConfig->speed_3_mph)
		{
			StartScore(E_BLUR_SCORE_MOVE_BASE_SPEED, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
			EndScore();
			mGotThirdSpeed=true; 
		}

		if(!mGotTopSpeed)
		{
			AmaxVehicle *av = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)mpLevelState->mCars[mVehicleIndex].mCarID];

			if(vehActor->mPositionInfo.mSpeedInMph >= av->mGameplayTopSpeed)
			{
				StartScore(E_BLUR_SCORE_MOVE_BASE_SPEED, mVehicleIndex);
				AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4);
				EndScore();
				mGotTopSpeed=true; 
			}
		}
	}

	if(mpLevelState->mRaceMode == Game::E_RaceMode_Checkpoint || mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_Checkpoint)
	{
		VehiclePositionInfo* posinfo = &vehActor->GetPositionInfo();

		if (posinfo->mLastRaceRuleArea != mCurrentRaceRuleArea)		// Not checking for only next so just look for a change
		{
			mCurrentRaceRuleArea = posinfo->mLastRaceRuleArea;

			if ((mCheckpointsPassed > 0)) // Ignore start 
			{
				Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

				if(pRace && pRace->GetEventState() == kSP_Gold)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_BANKING_TIME, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
					EndScore();
				}
				else if(!mCheckpointsDamageRecieved)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_BANKING_TIME, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
					EndScore();
				}
				else if(vehActor->GetScoring().mCheckpointTimeLeft < 0.5f)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_BANKING_TIME, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL);
					EndScore();
				}
				else
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_BANKING_TIME, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
					EndScore();
				}
			}

			mCheckpointsDamageRecieved=false;
			mCheckpointsPassed++;
		}
	}

	if(mIsSPDestruction)
	{
		if(m_pScoring->mDestruction_WreckedEasy)
		{
			m_pScoring->mDestruction_WreckedEasy=false;

			StartScore(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
			EndScore();
		}
		if(m_pScoring->mDestruction_WreckedMedium)
		{
			m_pScoring->mDestruction_WreckedMedium=false;

			StartScore(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
			EndScore();
		}
		if(m_pScoring->mDestruction_WreckedHard)
		{
			m_pScoring->mDestruction_WreckedHard=false;

			StartScore(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
			EndScore();
		}
		if(m_pScoring->mDestruction_WreckedVHard)
		{
			m_pScoring->mDestruction_WreckedVHard=false;

			StartScore(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
			EndScore();
		}

		//By Design - removed
		//if(m_pScoring->mDestruction_WreckedAll)
		//{
		//	m_pScoring->mDestruction_WreckedAll=false;

		//	StartScore(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, mVehicleIndex);
		//	AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4);
		//	EndScore();
		//}
		if(m_pScoring->mDestruction_CloseCallWrecked)
		{
			m_pScoring->mDestruction_CloseCallWrecked=false;

			StartScore(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, mVehicleIndex);
			AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL);
			EndScore();
		}
	}

	if(mComboTimer)
	{
		mComboTimer-=frameTime;
		if(mComboTimer < 0.f || mCombo >= pScoringConfig->mComboMax)
		{
			if(mCombo > 1)
			{
				mComboScore = mCombo * pScoringConfig->mComboFansMultiplier;
				GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_COMBO, mCombo >= pScoringConfig->mComboMax ? E_BLUR_SCORE_MOVE_BONUS_LEVEL_2 : E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, mVehicleIndex);
			}

			if(mCombo >= 5)
			{
				BossDemandAction(E_BOSSDEMAND_ACTION_X5_FAN_COMBO);
			}

			if(mCombo > 0)
			{
				// The challenges component would like to know about this...
				Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				Game::Actors::C_VehicleLogicActor* pLogicActor = pLogicLayer->GetLogicVehicleActor(mVehicleIndex);
				if(pLogicActor)
				{
					// Only send this in modes where challenges exist...
					if(pLogicActor->QueryInterface(E_IF_VehicleComponent_Challenges))
					{
						C_VehicleLogicMsg_ChainCompleted msg(mVehicleIndex,mCombo);
						pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &msg);
					}
				}
			}

			mComboTimer=0.f;
			mCombo=0;
			
			m_pScoring->mCombo=mCombo;
		}

		m_pScoring->mComboTimer=mComboTimer;
	}

	// Update Jump statistics
	UpdateJumpStats();


	for( Btl::Map<U32, PowerUpBoltState>::Iterator it = mPowerUpBoltStates.Begin(); it != mPowerUpBoltStates.End(); )
	{
		PowerUpBoltState& boltState = it->GetValue();
		if(boltState.mTimesFired >= mNumBoltsPerSet)
			boltState.mDeletionTimer -= frameTime;

		if(boltState.mDeletionTimer <= 0.f || boltState.mTimesHit >= mNumBoltsPerSet)
			mPowerUpBoltStates.Erase(it++);
		else
			++it;
	}
}

void C_VehicleComponent_NewScoring::UpdateJumpStats()
{
	Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(mVehicleIndex);
	const Vehicle::C_ModelInput *pModelInput = pVehicleLogicActor->GetGraphicalActor(mVehicleIndex)->GetVehicleInput();

	if(pVehicleLogicActor)
	{
		// Get the vehicle stats
		VehicleRaceStatistics& stats = pVehicleLogicActor->mStatistics;

		S32 mNumWheelsOnGround=0;

		for(int iWheel = 0; iWheel < pModelInput->m_Wheels.Size(); iWheel++)
		{
			if(pModelInput->m_Wheels[iWheel].m_bOnGround)
			{
				mNumWheelsOnGround++;
			}
		}

		if(mNumWheelsOnGround == 0)
		{
			stats.mCurrentDistanceTraveledInJump += pVehicleLogicActor->GetPositionInfo().mDistanceTravelledSinceLastUpdate;
			
			if(stats.mCurrentDistanceTraveledInJump > stats.mLongestDistanceTraveledInJump)
			{
				stats.mLongestDistanceTraveledInJump = stats.mCurrentDistanceTraveledInJump;
			}
		}
		else
		{
			stats.mCurrentDistanceTraveledInJump = 0.0f;
		}
	}
}

#ifndef CFG_GOLD

void C_VehicleComponent_NewScoring::UpdateMotormashDebugValues()
{
	// Motormash Scoring Debug
	if(g_debug_Motormash_GiveRivalPickupHit)
	{
		// Set our rival index to ourselves, 
		U8 prevRivalIndex = GetRivalIndex();
		SetRivalIndex(0);

		// simulate hitting ourselves
		HandleMotorMashMessagePickupHit(0,0,E_PICKUP_TYPE_SHUNT,E_PICKUP_SUB_TYPE_NORMAL);

		//set rival index back
		SetRivalIndex( prevRivalIndex ); 
		g_debug_Motormash_GiveRivalPickupHit = false;
	}

	if(g_debug_Motormash_GiveRivalPhysicalHit)
	{
		// Set our rival index to ourselves, 
		U8 prevRivalIndex = GetRivalIndex();
		SetRivalIndex(0);

		// simulate hitting ourselves
		HandleMotorMashMessageAggressiveImpact(0,0,E_IMPACT_LEVEL_HEAVY);

		//set rival index back
		SetRivalIndex( prevRivalIndex ); 
		g_debug_Motormash_GiveRivalPhysicalHit = false;
	}

	if(g_debug_Motormash_GivePickupHit)
	{
		// Set our rival index to ourselves, 
		U8 prevRivalIndex = GetRivalIndex();
		SetRivalIndex(-1);

		// simulate hitting ourselves
		HandleMotorMashMessagePickupHit(0,0,E_PICKUP_TYPE_SHUNT, E_PICKUP_SUB_TYPE_NORMAL);

		//set rival index back
		SetRivalIndex( prevRivalIndex ); 
		g_debug_Motormash_GivePickupHit = false;
	}

	if(g_debug_Motormash_GivePhysicalHit)
	{
		// Set our rival index to ourselves, 
		U8 prevRivalIndex = GetRivalIndex();
		SetRivalIndex(-1);

		// simulate hitting ourselves
		HandleMotorMashMessageAggressiveImpact(0,0,E_IMPACT_LEVEL_HEAVY);

		//set rival index back
		SetRivalIndex( prevRivalIndex ); 
		g_debug_Motormash_GivePhysicalHit = false;
	}
}

#endif


void C_VehicleComponent_NewScoring::CheckDrifter(bool activate, E_BLUR_SCORE_MOVE_SKILL skill)
{
	//Give a nitro if the player has Drifter upgrade
	Game::C_MultiplayerVehicleAbility* ability = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Drifter);
	if(Game::VehicleIsLocal(mVehicleIndex) && ability->GetActive())
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
		Game::Actors::C_VehicleActor* pActor = pLogicActor->GetGraphicalActor(mVehicleIndex);
	
		//And when activated
		if(activate)
		{

			if(pLogicActor && pLogicActor->mPickupInfo.mPickupStore.Size() < mpLevelState->mCars[mVehicleIndex].mUpgrades.NumPickupSlots())
			{
				Game::C_GiftDelayedPickupWithAnimMsg grantWinnerPickupMessage(mVehicleIndex,1,E_PICKUP_TYPE_NITROUS);
				mpMultiplayerAbilities->SetPickupSlotGiftedInto(pLogicActor->mPickupInfo.mPickupStore.Size() + 1);
				pLogicActor->SendMessageToComponent(E_IF_VehicleComponent_Pickups, &grantWinnerPickupMessage);
			}

			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Drifter, true, false, mVehicleIndex);

			//Start the special fx	
			RLPickupType* pNitro = (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS);
			Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(((RLNitrousSpecialFX*)pNitro->mNormal->mSpecialFX)->mDrifterGiftedNitroSpecialFX, 
				pLogicActor->GetWorldMatrix(), mVehicleIndex, -1, -1, Game::E_InGameSpecialFX_Trigger_Repairing);
		}
	}	
}

bool C_VehicleComponent_NewScoring::ShouldCalculateMotorMashScoring( U8 firingVehicle, U8 hitVehicle )
{
	// Check for early out
	if(!mIsMotorMash)
	{
		return false;
	}

	// We only want to send scoring messages to vehicles local to us
	if( !Game::VehicleIsLocal(firingVehicle) || Game::VehiclesAreTeammates(firingVehicle, hitVehicle))
	{
		return false;
	}

	return true;
}


void C_VehicleComponent_NewScoring::HandleMotorMashMessageAggressiveImpact(U8 vehicleHit, U8 aggressiveVehicle, E_IMPACT_LEVEL impactLevel)
{
	if(!ShouldCalculateMotorMashScoring(aggressiveVehicle, vehicleHit))
	{
		return;
	}

	// Make sure we tell the logic actor about this score, not just the UI
	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	MotorMashScoringValues* pScoringValues =	&m_pMpConfig->m_MotormashScoringValues;
	int ourScore = 0;

	bool wasRivalAttack = false;
	if( vehicleHit == GetRivalIndex())
	{
		wasRivalAttack = true;
	}

	UI::C_WideString128 strBase;
	strBase.Clear();

	bool wasHighDamageAttack = false;
	switch(impactLevel)
	{
		case E_IMPACT_LEVEL_MEDIUM:
		{
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashLightImpactActionIn );
			strBase += UI::C_TextStringID("MP_LOW_DAMAGE_ATTACK").String();
			wasHighDamageAttack = false;
			break;
		}

		case E_IMPACT_LEVEL_HEAVY:
		{
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashHeavyImpactActionIn );
			strBase += UI::C_TextStringID("MP_HIGH_DAMAGE_ATTACK").String();
			wasHighDamageAttack = true;
			break;
		}

		// We don't want to handle anything else
		default:
			return;
	}

	// Ensure UI score notification reflects any rival multiplier
	int baseScore = 0;
	int bonusScore = 0;
	HandleMotorMashScoreCalculation(pLogicActor, wasRivalAttack, wasHighDamageAttack, baseScore, bonusScore);

	// Add gamertag
	UI::C_WideString128 strName;
	strName.Clear();
	strName += mpLevelState->mCars[vehicleHit].mNameAboveCar.Chars();

	UI::C_UiScoringEventPerk uimsg1( strBase.Chars(), strName.Chars(), 0, baseScore, baseScore, mVehicleIndex );
	_UI_MESSAGE_FULL( uimsg1 );

	if(wasRivalAttack)
	{
		// If we wrecked a rival , and they are not dead, show 'rival hit'. If they are dead.. dont bother!, we will show a 'wrecked' bonus instead
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleHit);
		if( pLogicActor && pLogicActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED )
		{
			
			UI::C_WideString128 strRivalHit;
			strRivalHit.Clear();
			strRivalHit += UI::C_TextStringID("MP_RIVAL_HIT").String();

			UI::C_UiScoringEventChain uimsg2(strRivalHit.Chars(), bonusScore, mVehicleIndex);
			_UI_MESSAGE_FULL( uimsg2 );
		}
	}
}

void C_VehicleComponent_NewScoring::HandleMotorMashScoreCalculation(Game::Actors::C_VehicleLogicActor* pLogicActor, const bool wasRivalAttack, const bool wasHighDamageAttack, int& baseScore, int& bonusScore)
{
	MotorMashScoringValues* pScoringValues =	&m_pMpConfig->m_MotormashScoringValues;
	baseScore  = 0;
	baseScore  = 0;

	if(wasRivalAttack)
	{
		if(wasHighDamageAttack)
		{
			pLogicActor->mStatistics.mNumRivalHighDamageAttacks++;
			baseScore += pScoringValues->m_HighDamageScore;
		}
		else
		{
			pLogicActor->mStatistics.mNumRivalLowDamageAttacks++;
			baseScore += pScoringValues->m_LowDamageScore;
		}

		// Apply rival score multiplier to 'base score' ONLY
		int totalScore = ApplyRivalScoreMultiplier(baseScore);
		bonusScore = totalScore - baseScore;
	}
	else
	{
		if(wasHighDamageAttack)
		{
			pLogicActor->mStatistics.mNumHighDamageAttacks++;
			baseScore += pScoringValues->m_HighDamageScore;
		}
		else
		{
			pLogicActor->mStatistics.mNumLowDamageAttacks++;
			baseScore += pScoringValues->m_LowDamageScore;
		}
	}
}

void C_VehicleComponent_NewScoring::HandleMotormashMessageAggressiveScore(const U8 vehicleHit, const U8 aggressiveVehicle,E_BLUR_SCORE_MOVE_BONUS scoringMove)
{
	// If this isnt motormash, or we arent local.. dont worry about any of this. Also dont credit killing team mates 
	if( !mIsMotorMash ||
		!Game::VehicleIsLocal(aggressiveVehicle) || 
		 Game::VehiclesAreTeammates(aggressiveVehicle, vehicleHit))
	{
		return;
	}

	if( scoringMove != E_BLUR_SCORE_MOVE_BONUS_DROWN && 
		scoringMove != E_BLUR_SCORE_MOVE_BONUS_CLIFF )
	{
		return;
	}

	// Make sure we tell the logic actor about this score, not just the UI
	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	if(!pLogicActor)
	{
		return;
	}

	MotorMashScoringValues* pScoringValues =	&m_pMpConfig->m_MotormashScoringValues;
	int ourScore = 0;

	bool wasRivalAttack = false;
	if( vehicleHit == GetRivalIndex())
	{
		wasRivalAttack = true;
	}

	UI::C_WideString128 strBase;
	strBase.Clear();

	switch(scoringMove)
	{
		case E_BLUR_SCORE_MOVE_BONUS_DROWN:
		{
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashHeavyImpactActionIn );
			strBase += UI::C_TextStringID("SCR_NEW_AMAX_SCORING_DROWNING_SHOT_NO_NAME").String();
			break;
		}

		case E_BLUR_SCORE_MOVE_BONUS_CLIFF:
		{
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_MotorMashHeavyImpactActionIn );
			strBase += UI::C_TextStringID("SCR_NEW_AMAX_SCORING_PLUMMETED_NO_NAME").String();
			break;
		}

		// We don't want to handle anything else
		default:
			return;
	}

	// Ensure UI score notification reflects any rival multiplier
	int baseScore = 0;
	int bonusScore = 0;
	HandleMotorMashScoreCalculation(pLogicActor, wasRivalAttack, true, baseScore, bonusScore);

	// Add gamertag
	UI::C_WideString128 strName;
	strName.Clear();
	strName += mpLevelState->mCars[vehicleHit].mNameAboveCar.Chars();

	UI::C_UiScoringEventPerk uimsg1( strBase.Chars(), strName.Chars(), 0, baseScore, baseScore, mVehicleIndex );
	_UI_MESSAGE_FULL( uimsg1 );

	if(wasRivalAttack)
	{
		// If we wrecked a rival , and they are not dead, show 'rival hit'. If they are dead.. dont bother!, we will show a 'wrecked' bonus instead
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleHit);
		if( pLogicActor && pLogicActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED )
		{
			
			UI::C_WideString128 strRivalHit;
			strRivalHit.Clear();
			strRivalHit += UI::C_TextStringID("MP_RIVAL_HIT").String();

			UI::C_UiScoringEventChain uimsg2(strRivalHit.Chars(), bonusScore, mVehicleIndex);
			_UI_MESSAGE_FULL( uimsg2 );
		}
	}
}

void C_VehicleComponent_NewScoring::HandleMotorMashMessagePickupHit(U8 vehicleHit, U8 firingVehicle, E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType)
{
	if(!ShouldCalculateMotorMashScoring(firingVehicle, vehicleHit))
	{
		return;
	}
	
	// If we fired and hit ourselves... we dont get any points.. just a well-deserved *facepalm*
	if( vehicleHit == mVehicleIndex )
	{
		return;
	}

	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	UI::C_WideString128 strBase;
	strBase.Clear();


	bool wasRivalAttack = false;
	if( vehicleHit == GetRivalIndex())
	{
		wasRivalAttack = true;
	}

	bool wasHighDamageAttack = false;

	// Special check for decoy drops
	if(subType == E_PICKUP_SUB_TYPE_DECOY)
	{
		strBase += UI::C_TextStringID("SCR_NEW_AMAX_SCORING_DECOY_HIT").String();
		wasHighDamageAttack = true;
	}
	else
	{
		switch(pickupType)
		{
			case E_PICKUP_TYPE_BARGE:
			{
				if(subType == E_PICKUP_SUB_TYPE_MOD)
				{
					strBase += UI::C_TextStringID("SCR_NEW_AMAX_SCORING_LAST_GASP").String();
				}
				else
				{
					strBase += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_BARGE").String();
				}
				wasHighDamageAttack = true;
				break;
			}
			case E_PICKUP_TYPE_BLINDFIRE:
			{
				strBase += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_BLINDFIRE").String();
				if( mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist)->GetActive() )
				{
					wasHighDamageAttack = Game::C_Game::GetInstance()->GetMultiplayerAbilities().IronFistActivatedOnPickupType(pickupType) > 0;
				}
				else
				{
					wasHighDamageAttack = false;
				}
				break;
			}
			case E_PICKUP_TYPE_BOMB:
			{
				strBase += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_BOMB").String();
				wasHighDamageAttack = true;
				break;
			}
			case E_PICKUP_TYPE_SHUNT:
			{
				strBase += UI::C_TextStringID("SCR_AMAX_SCORING_EVENT_SHUNT").String();
				wasHighDamageAttack = true;
				break;
			}
			// We don't know what message to display since we don't know the pickup, so bail out
			default:
				return;
		}
	}

	// Ensure UI score notification reflects any rival multiplier
	int baseScore = 0;
	int bonusScore  = 0;
	HandleMotorMashScoreCalculation(pLogicActor, wasRivalAttack, wasHighDamageAttack, baseScore, bonusScore);

	// Add gamertag
	UI::C_WideString128 strName;
	strName.Clear();
	strName += mpLevelState->mCars[vehicleHit].mNameAboveCar.Chars();

	UI::C_UiScoringEventPerk uimsg1( strBase.Chars(), strName.Chars(), 0, baseScore, baseScore, mVehicleIndex );
	_UI_MESSAGE_FULL( uimsg1 );

	if(wasRivalAttack)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(vehicleHit);
		if( pLogicActor && pLogicActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED )
		{
			UI::C_WideString128 strRivalHit;
			strRivalHit.Clear();
			strRivalHit += UI::C_TextStringID("MP_RIVAL_HIT").String();

			UI::C_UiScoringEventChain uimsg2(strRivalHit.Chars() , bonusScore, mVehicleIndex);
			_UI_MESSAGE_FULL( uimsg2 );
		}
	}
}

int C_VehicleComponent_NewScoring::ApplyRivalScoreMultiplier(const int baseScore) const
{
	// Atm just lose any fractional part.. could give option to round up. 
	int ourScore = (int)((float)(baseScore) * mRivalDamageScoreMultiplier); // ich.. horrible conversions TODO: expose var stating round up or down on fractional score
	return ourScore;
}

bool C_VehicleComponent_NewScoring::IsCurrentScoreBaseMinor(bool checkMove)
{
	if (   ( checkMove && ( mCurrentScoreBase > E_BLUR_SCORE_MOVE_BASE_RAMMING ) )
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_Checkpoint ) 
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_Destruction ) 
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_FanDemand ) 
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_Checkpoint ) 
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_Destruction ) 
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_FriendDemand_FanDemand ) 
		|| ( mpLevelState->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction ) 
		|| ( mRaceLogicLayerPtr->mFanDemandActive ) )
	{
		return true;
	}

	return false;
}

void C_VehicleComponent_NewScoring::StartScore(E_BLUR_SCORE_MOVE_BASE base, int other_vehicle)
{
	if(!mRaceStarted)
		return;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	switch(base)
	{
		case E_BLUR_SCORE_MOVE_BASE_SHOCK:
			BossDemandAction(E_BOSSDEMAND_ACTION_SHOCK_HIT);
			break;
		case E_BLUR_SCORE_MOVE_BASE_BARGE:
			BossDemandAction(E_BOSSDEMAND_ACTION_BARGE_HIT);
			break;
		case E_BLUR_SCORE_MOVE_BASE_SHUNT:	
			BossDemandAction(E_BOSSDEMAND_ACTION_SHUNT_HIT);			
			break;
		case E_BLUR_SCORE_MOVE_BASE_MINE:
			BossDemandAction(E_BOSSDEMAND_ACTION_MINE_HIT);
			break;
	}
	mCurrentScoreBase=base;

	if(IsCurrentScoreBaseMinor(true))
		mCurrentScoreBaseType=E_BLUR_SCORE_MOVE_BASE_TYPE_MINOR;
	else
		mCurrentScoreBaseType=E_BLUR_SCORE_MOVE_BASE_TYPE_MAJOR;

	mCurrentScoreSkills.Clear();
	mCurrentScoreBonus.Clear();

	mCurrentOtherVehicle=other_vehicle;

	switch(mCurrentScoreBase)
	{
		case E_BLUR_SCORE_MOVE_BASE_BARGE:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Barge", mVehicleIndex );
			mCurrentScore=pScoringConfig->mBargeBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_MINE:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Mine", mVehicleIndex );
			mCurrentScore=pScoringConfig->mMineBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_SHOCK:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Shock", mVehicleIndex );
			mCurrentScore=pScoringConfig->mShockBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_SHUNT:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Shunt", mVehicleIndex );
			mCurrentScore=pScoringConfig->mShuntBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_BOLT:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Bolt", mVehicleIndex );
			mCurrentScore=pScoringConfig->mBoltBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_HIT_DECOY:
			mCurrentScore = pScoringConfig->mDecoyHitScore;
			break;

		case E_BLUR_SCORE_MOVE_BASE_RAMMING:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Ram", mVehicleIndex );
			mCurrentScore=pScoringConfig->mRammingBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_NITROUS:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started a Nitrous", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_DRIFTING:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Drift", mVehicleIndex );
			mCurrentScore=pScoringConfig->mDriftLevel1;
			break;

		case E_BLUR_SCORE_MOVE_BASE_JUMPING:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Jump", mVehicleIndex );
			mCurrentScore=pScoringConfig->mJumpLevel1;
			break;

		case E_BLUR_SCORE_MOVE_BASE_OVERTAKE:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Overtake", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_POSITION:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started POsition", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_SPEED:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Speed", mVehicleIndex );
			mCurrentScore=pScoringConfig->mSpeedLevel1;
			break;

		case E_BLUR_SCORE_MOVE_BASE_EVASION:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Evasion", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_NEAR_MISS:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Near Miss", mVehicleIndex );
			mCurrentScore=pScoringConfig->mNearMiss;
			break;
		
		case E_BLUR_SCORE_MOVE_BASE_HIT_ANOTHER_PICKUP:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Hit Another PowerUp", mVehicleIndex );
			mCurrentScore=pScoringConfig->mHitAnotherPickup;
			break;

		case E_BLUR_SCORE_MOVE_BASE_SHIELD:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Shield", mVehicleIndex );
			mCurrentScore=pScoringConfig->mShieldBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_REPAIR:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Repair", mVehicleIndex );
			mCurrentScore=pScoringConfig->mRepairBase;
			break;

		case E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Clean Racing", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_STOPWATCH:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Stopwatch", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_BANKING_TIME:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Checkpoint", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_DESTRUCTION:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Destruction", mVehicleIndex );
			mCurrentScore=0;
			break;

		case E_BLUR_SCORE_MOVE_BASE_LAST_GASP:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Started Last Gasp Hit", mVehicleIndex );
			mCurrentScore= pScoringConfig->mLastGaspHitScore;
			break;

		default:
			mCurrentScore=0;
			break;
	}
}

void C_VehicleComponent_NewScoring::AddSkillScore(E_BLUR_SCORE_MOVE_SKILL skill)
{
	if(!mRaceStarted)
		return;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	mCurrentScoreSkills.PushBack(skill);

	switch(skill)
	{
		case E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Alt Fire Skill", mVehicleIndex );
			
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_MINE)
					mCurrentScore+=pScoringConfig->mMineAltFire;
				
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_SHUNT)
				{
					mCurrentScore+=pScoringConfig->mShuntAltFire;
					BossDemandAction(E_BOSSDEMAND_ACTION_REVERSE_SHUNT_HIT);			
				}

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BOLT)
					mCurrentScore+=pScoringConfig->mBoltAltFire;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_NITROUS)
					mCurrentScore+=pScoringConfig->mNitrousAltFire;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_STOPWATCH)
					mCurrentScore+=pScoringConfig->mCheckpointAirBrakeCollect;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_LONG_SHOT:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Long Shot Skill", mVehicleIndex );

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BOLT)
					mCurrentScore+=pScoringConfig->mBoltLongShot;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_AIR:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Air Skill", mVehicleIndex );
		
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeAir;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_MINE)
					mCurrentScore+=pScoringConfig->mMineAir;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BOLT)
					mCurrentScore+=pScoringConfig->mBoltAir;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_RAMMING)
					mCurrentScore+=pScoringConfig->mRammingAir;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_NITROUS)
					mCurrentScore+=pScoringConfig->mNitrousAir;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_DRIFT:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Drift Skill", mVehicleIndex );

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeDrift;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BOLT)
					mCurrentScore+=pScoringConfig->mBoltDrift;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_NITROUS)
					mCurrentScore+=pScoringConfig->mNitrousDrift;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_NITROUS:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Nitrous Skill", mVehicleIndex );
		
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeNitrous;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_RAMMING)
					mCurrentScore+=pScoringConfig->mRammingNitrous;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Close Call Skill", mVehicleIndex );

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_SHIELD)
					mCurrentScore+=pScoringConfig->mShieldCloseCall;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_REPAIR)
					mCurrentScore+=pScoringConfig->mRepairCloseCall;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DESTRUCTION)
					mCurrentScore+=pScoringConfig->mDestructionCloseCallWreck;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BANKING_TIME)
					mCurrentScore+=pScoringConfig->mCheckpointCloseCall;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_STOPWATCH)
					mCurrentScore+=pScoringConfig->mCheckpointStopwatchCloseCall;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_LEVEL_1:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Level 1 Skill", mVehicleIndex );

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_MINE)
					mCurrentScore+=pScoringConfig->mMineLongShot;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DRIFTING)
					mCurrentScore+=pScoringConfig->mDriftLevel1;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_JUMPING)
					mCurrentScore+=pScoringConfig->mJumpLevel1;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_POSITION)
					mCurrentScore+=pScoringConfig->mPositionThird;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_EVASION)
					mCurrentScore+=pScoringConfig->mEvasionShock;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_SPEED)
					mCurrentScore+=pScoringConfig->mSpeedLevel1;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING)
					mCurrentScore+=pScoringConfig->mCleanRacingNitrous;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeLevel1;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BANKING_TIME)
					mCurrentScore+=pScoringConfig->mCheckpointCleanSection;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DESTRUCTION)
					mCurrentScore+=pScoringConfig->mDestructionEasyWreck;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_OVERTAKE)
					mCurrentScore+=pScoringConfig->mOvertakeNitrous;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_LEVEL_2:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Level 2 Skill", mVehicleIndex );
			
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DRIFTING)
					mCurrentScore+=pScoringConfig->mDriftLevel2;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_JUMPING)
					mCurrentScore+=pScoringConfig->mJumpNitrous;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_POSITION)
					mCurrentScore+=pScoringConfig->mPositionSecond;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_EVASION)
					mCurrentScore+=pScoringConfig->mEvasionShunt;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_SPEED)
					mCurrentScore+=pScoringConfig->mSpeedLevel2;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING)
					mCurrentScore+=pScoringConfig->mCleanRacingNoWreckRepairLap;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeLevel2;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BANKING_TIME)
					mCurrentScore+=pScoringConfig->mCheckpointGoldSection;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DESTRUCTION)
					mCurrentScore+=pScoringConfig->mDestructionMediumWreck;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_OVERTAKE)
					mCurrentScore+=pScoringConfig->mOvertakeNitrousGood;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_LEVEL_3:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Level 3 Skill", mVehicleIndex );
				
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DRIFTING)
					mCurrentScore+=pScoringConfig->mDriftLevel3;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_JUMPING)
					mCurrentScore+=pScoringConfig->mJumpLevel2;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_POSITION)
					mCurrentScore+=pScoringConfig->mPositionFirst;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_SPEED)
					mCurrentScore+=pScoringConfig->mSpeedLevel3;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING)
					mCurrentScore+=pScoringConfig->mCleanRacingLap;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeLevel3;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BANKING_TIME)
					mCurrentScore+=pScoringConfig->mCheckpointSectionPassed;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DESTRUCTION)
					mCurrentScore+=pScoringConfig->mDestructionHardWreck;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_OVERTAKE)
					mCurrentScore+=pScoringConfig->mOvertakeNitrousGreat;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_LEVEL_4:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Level 4 Skill", mVehicleIndex );
		
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_POSITION)
					mCurrentScore+=pScoringConfig->mPositionFirstLevel1;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeLevel4;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_SPEED)
					mCurrentScore+=pScoringConfig->mSpeedLevel4;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_DESTRUCTION)
					mCurrentScore+=pScoringConfig->mDestructionAllWreck;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_OVERTAKE)
					mCurrentScore+=pScoringConfig->mOvertakeNitrousMega;
			}
			break;

		case E_BLUR_SCORE_MOVE_SKILL_LEVEL_5:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Level 5 Skill", mVehicleIndex );
		
				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_POSITION)
					mCurrentScore+=pScoringConfig->mPositionFirstLevel2;

				if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_BARGE)
					mCurrentScore+=pScoringConfig->mBargeLevel5;
			}
			break;

		default:
			break;
	}
}

void C_VehicleComponent_NewScoring::AddBonusScore(E_BLUR_SCORE_MOVE_BONUS bonus, E_BLUR_SCORE_MOVE_BONUS_LEVEL level)
{
	if(!mRaceStarted)
		return;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	mCurrentScoreBonus.PushBack(bonus);

	switch(bonus)
	{
		case E_BLUR_SCORE_MOVE_BONUS_ACCURACY:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Accuracy Bonus", mVehicleIndex );
			mCurrentScore+=pScoringConfig->mBonusAccuracy;
			break;

		case E_BLUR_SCORE_MOVE_BONUS_WRECKED:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Wrecked Bonus", mVehicleIndex );
			mCurrentScore+=pScoringConfig->mBonusWrecked;
			break;

		case E_BLUR_SCORE_MOVE_BONUS_KILLING_SPREE:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Killing Spree Bonus", mVehicleIndex );
			mCurrentScore+=pScoringConfig->mBonusKillingSpree;
			break;

		case E_BLUR_SCORE_MOVE_BONUS_BULLYING:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Bullying Bonus", mVehicleIndex );
			mCurrentScore+=pScoringConfig->mBonusBullying;
			break;

		case E_BLUR_SCORE_MOVE_BONUS_REVENGE:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Revenge Bonus", mVehicleIndex );
			mCurrentScore+=pScoringConfig->mBonusRevenge;
			break;

		case E_BLUR_SCORE_MOVE_BONUS_FULLY_BOLTED:
			if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
				LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Fully Bolted Bonus", mVehicleIndex );
			mCurrentScore+=pScoringConfig->mBonusFullyBolted;
			break;


		default:
			mCurrentScore+=0;
			break;
	}
}

void C_VehicleComponent_NewScoring::NearMiss(int vehicle_index)
{
	StartScore(E_BLUR_SCORE_MOVE_BASE_NEAR_MISS, vehicle_index);
	EndScore();
}

void C_VehicleComponent_NewScoring::EndScore()
{
	if(!mRaceStarted)
		return;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];


	if(mCurrentScoreBase <= E_BLUR_SCORE_MOVE_BASE_RAMMING)
	{
		// Showy Flourish ability -- gives extra fans on successful power-up impact
		Game::C_MultiplayerVehicleAbility* ability = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Showy_Flourish);
		if(ability->GetActive())
		{
			U8 extraFans = (U8)(ability->GetValue()); 
			mCurrentScore += extraFans;

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


	//Data log strings
	UI::C_WideString64 dataSkills[3];

	UI::C_WideString128 str;
	UI::C_WideString32 strname;
	bool currentMajor = ( mCurrentScoreBaseType == E_BLUR_SCORE_MOVE_BASE_TYPE_MAJOR );
	bool showNames = UI::ShowAiNames();

	for(int a=0; a < mCurrentScoreSkills.Size(); a++)
	{
		UI::C_WideString64 skill;

		switch(mCurrentScoreSkills[a])
		{
			case E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE:
				{
					if(mCurrentScoreBase == E_BLUR_SCORE_MOVE_BASE_MINE)
					{
						static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_FORWARD" );
						skill.Add(text.String());
					}
					else
					{
						static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_REVERSE" );
						skill.Add(text.String());
					}
				}
				break;

			case E_BLUR_SCORE_MOVE_SKILL_LONG_SHOT:
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_LONG_SHOT" );
					skill.Add(text.String());
				}
				break;

			case E_BLUR_SCORE_MOVE_SKILL_AIR:
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_AIR" );
					skill.Add(text.String());
				}
				break;

			case E_BLUR_SCORE_MOVE_SKILL_DRIFT:
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_DRIFT" );
					skill.Add(text.String());
				}
				break;

			case E_BLUR_SCORE_MOVE_SKILL_NITROUS:
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS" );
					skill.Add(text.String());
				}
				break;

			case E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL:
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CLOSE_CALL" );
					skill.Add(text.String());
				}
				break;
			
			default:
				break;
		}

		str.Add(skill);

		if(a < 3)
		{
			dataSkills[a].Add(skill);
		}
	}

	UI::C_WideString128 baseStr;

	switch(mCurrentScoreBase)
	{
		case E_BLUR_SCORE_MOVE_BASE_BARGE:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_BARGED" );
				baseStr.Add(text.String());

				int multi_barge=0;

				for(int a=0;a<mCurrentScoreSkills.Size();a++)
				{
					if(mCurrentScoreSkills[a] >= E_BLUR_SCORE_MOVE_SKILL_LEVEL_1 && mCurrentScoreSkills[a] <= E_BLUR_SCORE_MOVE_SKILL_LEVEL_5)
					{
						multi_barge=mCurrentScoreSkills[a];
					}
				}

				if(!multi_barge)
				{
					if(showNames)
					{
						if(currentMajor)
							strname = mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
						else
							baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
					}
				}
				else
				{
					if(multi_barge == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
					{
						baseStr.Add(UI::C_TextStringID("SCR_NEW_AMAX_X2").String());
					}
					if(multi_barge == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
					{
						baseStr.Add(UI::C_TextStringID("SCR_NEW_AMAX_X3").String());
					}
					if(multi_barge == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
					{
						baseStr.Add(UI::C_TextStringID("SCR_NEW_AMAX_X4").String());
					}
					if(multi_barge == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4)
					{
						baseStr.Add(UI::C_TextStringID("SCR_NEW_AMAX_X5").String());
					}
					if(multi_barge == E_BLUR_SCORE_MOVE_SKILL_LEVEL_5)
					{
						baseStr.Add(UI::C_TextStringID("SCR_NEW_AMAX_MAX").String());
					}
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_MINE:
			{
				bool mining_path=false;
				for(int a=0;a<mCurrentScoreSkills.Size();a++)
				{
					if(mCurrentScoreSkills[a] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
					{
						mining_path=true;
					}
				}

				if(!mining_path)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_MINED" );
					baseStr.Add(text.String());

					if(showNames)
					{
						if(currentMajor)
							strname = mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
						else
							baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
					}
				}
				else
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_PATH_MINED" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_SHOCK:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SHOCKED" );
				baseStr.Add(text.String());
				
				if(showNames)
				{
					if(currentMajor)
						strname = mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_SHUNT:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SHUNTED" );
				baseStr.Add(text.String());
				
				if(showNames)
				{
					if(currentMajor)
						strname = mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_BOLT:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_BOLTED" );
				baseStr.Add(text.String());
				
				if(showNames)
				{
					if(currentMajor)
						strname =mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_HIT_DECOY:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_DECOY_HIT" );
				baseStr.Add(text.String());

				if(showNames)
				{
					if(currentMajor)
						strname =mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_LAST_GASP:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_LAST_GASP" );
				baseStr.Add(text.String());

				if(showNames)
				{
					if(currentMajor)
						strname =mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_RAMMING:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SLAMMED" );
				str.Add(text.String());
				
				if(showNames)
				{
					if(currentMajor)
						strname = mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_NITROUS:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS" );
				baseStr.Add(text.String());
				
				if(showNames)
				{
					if(currentMajor)
						strname = mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar;
					else
						baseStr.Add(mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar);
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_DRIFTING:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_GOOD_DRIFT" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SUPER_DRIFT" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_MEGA_DRIFT" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_JUMPING:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_GOOD_JUMP" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS_JUMP" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_OVERTAKE:
			{
				bool wasNitrous = false;
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS_OVERTAKE" );
					str.Format(text.String());
					wasNitrous = true;
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS_OVERTAKE_GOOD" );
					str.Format(text.String());
					wasNitrous = true;
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS_OVERTAKE_GREAT" );
					str.Format(text.String());
					wasNitrous = true;
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NITROUS_OVERTAKE_MEGA" );
					str.Format(text.String());
					wasNitrous = true;
				}

				// Lets store this stat
				if(wasNitrous)
				{	
					Game::Actors::C_VehicleLogicActor* pLogicActor = ((Game::Actors::C_VehicleLogicActor* )GetActor());
					if(pLogicActor)
					{
						pLogicActor->mStatistics.mNitroOvertakes++;
					}
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_POSITION:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_THIRD_PLACE" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SECOND_PLACE" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_FIRST_PLACE" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_RACE_DOMINATION" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_5)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_TOTAL_RACE_DOMINATION" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_SPEED:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_HIGH_SPEED" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SPEED_DEMON" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_KING_OF_SPEED" );
					str.Format(text.String());
				}
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_TOP_SPEED" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_EVASION:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_EVADED_A_SHOCK" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_EVADED_A_SHUNT" );
					str.Format(text.String());
				}
			}
			break;
		
		case E_BLUR_SCORE_MOVE_BASE_NEAR_MISS:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NEAR_MISS" );
				baseStr.Add(text.String());
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_HIT_ANOTHER_PICKUP:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_POWER_UP_HIT" );
				baseStr.Add(text.String());
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_SHIELD:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SHIELD" );
				baseStr.Add(text.String());
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_REPAIR:
			{
				static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_REPAIR" );
				baseStr.Add(text.String());
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CLEAN_NITRO" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_NO_REPAIR_OR_WRECK_LAP" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CLEAN_LAP" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_STOPWATCH:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_STOPWATCH_CLOSECALL" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_STOPWATCH_AIR_BRAKE_COLLECT" );
					str.Format(text.String());
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_BANKING_TIME:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CHECKPOINT_CLOSECALL" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CHECKPOINT_CLEAN_SECTION" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CHECKPOINT_GOLD_SECTION" );
					str.Format(text.String());
				}
				
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CHECKPOINT_PASSED" );
					str.Format(text.String());
				}			
			}
			break;

		case E_BLUR_SCORE_MOVE_BASE_DESTRUCTION:
			{
				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_EASY_CAR_WRECKED" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_MEDIUM_CAR_WRECKED" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_HARD_CAR_WRECKED" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_ALL_CAR_WRECKED" );
					str.Format(text.String());
				}

				if(mCurrentScoreSkills[0] == E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL)
				{
					static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_CLOSE_CALL_CAR_WRECKED" );
					str.Format(text.String());
				}
			}
			break;

		default:
			break;
	}

	if(baseStr.Length() > 0)
	{
		str.Add(baseStr);
	}
	//Used .Format rather than .Add above so make sure we catch this for the logging
	else
	{
		baseStr.Add(str);
	}
	

	if(mCurrentScoreBaseType == E_BLUR_SCORE_MOVE_BASE_TYPE_MINOR)
	{
		UI::C_WideString128 str1;
		static UI::C_TextStringID text( "SCR_NEW_AMAX_SCORING_SCORE_STRING" );
		str1.Format(text.String(), mCurrentScore);

		str.Add(str1.Chars());
	}

	UI::C_WideString128 bonus_str;
	bonus_str.Clear();
	int bonus_fans = 0;

	for(int a=0; a < mCurrentScoreBonus.Size(); a++)
	{
		switch(mCurrentScoreBonus[a])
		{
			case E_BLUR_SCORE_MOVE_BONUS_ACCURACY:
				{
					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_THREE_IN_A_ROW" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusAccuracy;
				}
				break;

			case E_BLUR_SCORE_MOVE_BONUS_BULLYING:
				{
					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_BULLYING" );
					static UI::C_TextStringID bonus_text_no_name( "SCR_NEW_AMAX_SCORING_BULLYING_NO_NAME" );

					if(showNames)
						bonus_str.Format(bonus_text.String(), mpLevelState->mCars[mCurrentOtherVehicle].mNameAboveCar.Chars());
					else
						bonus_str.Format(bonus_text_no_name.String());

					bonus_fans = pScoringConfig->mBonusBullying;
				}
				break;

			case E_BLUR_SCORE_MOVE_BONUS_REVENGE:
				{
					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_REVENGE" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusRevenge;
				}
				break;

			case E_BLUR_SCORE_MOVE_BONUS_FULLY_BOLTED:
				{
					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_FULLY_BOLTED" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusFullyBolted;
				}
				break;

			default:
				break;
		}
	}

	IncreaseScore(mCurrentScore);

	BossDemandAction(E_BOSSDEMAND_ACTION_GOT_FANS, (F32)mCurrentScore);
	
	mCombo++;
	mComboTimer=pScoringConfig->combo_time;
	
	m_pScoring->mCombo=mCombo;
	m_pScoring->mComboTimer=mComboTimer;

	mScoringHistory.AddScoringEvent( ScoreEventHistory::ScoreEvent(mCurrentScore, C_GameApp::GetInstance()->GetElapsedRaceTime()) );

	if(mCurrentScoreBaseType == E_BLUR_SCORE_MOVE_BASE_TYPE_MAJOR)
	{
		if(mScoreUITimer)
		{
			UI::C_UiScoringEventPerk uimsg1( L"", -1, mCurrentScore, mCurrentScore, mVehicleIndex );
			_UI_MESSAGE_FULL( uimsg1 );
			mScoreUITimer=0.f;
		}

		UI::C_UiScoringEventChain uimsg2(bonus_str.Chars(), bonus_fans, mVehicleIndex);
		_UI_MESSAGE_FULL( uimsg2 );

		UI::C_UiScoringEventFans uimsg(mTotalScore, false, mVehicleIndex);
		_UI_MESSAGE_FULL( uimsg );

		UI::C_UiScoringEventPerk uimsg1( str.Chars(), strname.Chars(), 0, mCurrentScore, mTotalScore, mVehicleIndex );
		_UI_MESSAGE_FULL( uimsg1 );
	}
	else
	{
		UI::C_UiScoringEventMinor uimsg3(str.Chars(), mVehicleIndex, (mCurrentScoreBase <= E_BLUR_SCORE_MOVE_BASE_RAMMING));
		_UI_MESSAGE_FULL( uimsg3 );
	}

	const C_VehicleLogicMsg_Scored msg(mVehicleIndex, mCurrentScoreBase, mCurrentScoreSkills, mCurrentScoreBonus);
	Components::C_VehicleComponent_Fan_Demand* pFanDemand = (Components::C_VehicleComponent_Fan_Demand*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand);
	if(pFanDemand)
		pFanDemand->OnMessage(&msg);
	Components::C_VehicleComponent_MP_Challenges* pChallenges = (Components::C_VehicleComponent_MP_Challenges*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Challenges);
	if(pChallenges)
		pChallenges->OnMessage(&msg);

	// Some stickers need to know when this happens
	Game::C_StickerManager::Instance().MessageHandler(&msg);
#ifndef CFG_GOLD
	//Log (Human players only)
	if(mpLevelState->mCars[mVehicleIndex].mHuman)
	{
		AmaxDataLogging::LogScore(mVehicleIndex, ((Game::Actors::C_VehicleLogicActor* )GetActor())->GetPositionInfo().mWorldPosition, baseStr.AsString(), mCurrentScoreBaseType, 
			dataSkills[0].AsString(), dataSkills[1].AsString(), dataSkills[2].AsString(), bonus_str.AsString(), mCurrentScore, mTotalScore-mLastScore-mCurrentScore+1, mCombo, mTotalScore);
		mLastScore = mTotalScore;
	}
#endif
	mScoreUITimer=g_debug_NewScoring_UIScreenTimer;
	mCurrentScoreBase=E_BLUR_SCORE_MOVE_BASE_NOTHING;

	m_pScoring->mFinalScore=mTotalScore;
	m_pScoring->mFinalScoreForUI=m_pScoring->mFinalScore;

	if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
		LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Ended with Score %d", mVehicleIndex, mTotalScore );
}

void C_VehicleComponent_NewScoring::IncreaseScore(int score)
{
	float scoreToAdd = score * mpLevelState->mFanMultiplier;
	mTotalScore += (int)Maths::Floor(scoreToAdd);
}

void C_VehicleComponent_NewScoring::GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS bonus, E_BLUR_SCORE_MOVE_BONUS_LEVEL level, int about_vehicle)
{
	if(!mRaceStarted)
		return;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	bool showNames = UI::ShowAiNames();

	UI::C_WideString128 bonus_str;
	bonus_str.Clear();
	int bonus_fans = 0;

	F32 TotalScore = (F32)mTotalScore;
	switch(bonus)
	{
		case E_BLUR_SCORE_MOVE_BONUS_ACCURACY:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Instant Three Hits in a Row", mVehicleIndex );
				IncreaseScore(pScoringConfig->mBonusAccuracy);

				static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_THREE_IN_A_ROW" );
				bonus_str.Format(bonus_text.String());
				bonus_fans = pScoringConfig->mBonusAccuracy;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_REVENGE:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Added Revenge Bonus", mVehicleIndex );

				IncreaseScore(pScoringConfig->mBonusRevenge);

				static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_REVENGE" );
				bonus_str.Format(bonus_text.String());
				bonus_fans = pScoringConfig->mBonusRevenge;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_WRECKED:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Instant Wrecking Shot", mVehicleIndex );
				IncreaseScore(pScoringConfig->mBonusWrecked);

				switch(level)
				{
					case E_BLUR_SCORE_MOVE_BONUS_LEVEL_1:
						{
							static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_WRECKING_SHOT" );
							bonus_str.Format(bonus_text.String());
						}
						break;

					case E_BLUR_SCORE_MOVE_BONUS_LEVEL_2:
						{
							static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_VAMPIRIC_WRECK" );
							bonus_str.Format(bonus_text.String());
						}
						break;
				}

				bonus_fans = pScoringConfig->mBonusWrecked;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_KILLING_SPREE:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Instant Killing Spree", mVehicleIndex );
				IncreaseScore(pScoringConfig->mBonusKillingSpree);

				static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_KILLING_SPREE" );
				bonus_str.Format(bonus_text.String());
				bonus_fans = pScoringConfig->mBonusKillingSpree;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_DROWN:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Instant Drowning Shot", mVehicleIndex );
				IncreaseScore(pScoringConfig->mBonusDrown);

				static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_DROWNING_SHOT" );
				static UI::C_TextStringID bonus_text_no_name( "SCR_NEW_AMAX_SCORING_DROWNING_SHOT_NO_NAME" );

				if(showNames)
					bonus_str.Format(bonus_text.String(), mpLevelState->mCars[about_vehicle].mNameAboveCar.Chars());
				else
					bonus_str.Format(bonus_text_no_name.String());

				bonus_fans = pScoringConfig->mBonusDrown;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_CLIFF:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Instant Plummeted", mVehicleIndex );
				IncreaseScore(pScoringConfig->mBonusPlummet);

				static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_PLUMMETED" );
				static UI::C_TextStringID bonus_text_no_name( "SCR_NEW_AMAX_SCORING_PLUMMETED_NO_NAME" );

				if(showNames)
					bonus_str.Format(bonus_text.String(), mpLevelState->mCars[about_vehicle].mNameAboveCar.Chars());
				else
					bonus_str.Format(bonus_text_no_name.String());

				bonus_fans = pScoringConfig->mBonusPlummet;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Fan Demand Complete", mVehicleIndex );

				IncreaseScore(mFanDemandFans);
				bonus_str.Clear();

				if(level == E_BLUR_SCORE_MOVE_BONUS_LEVEL_2)
				{
					IncreaseScore(pScoringConfig->mBonusFanDemandsHoarder);

					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_FAN_DEMAND_PICKUPS_LEFT" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusFanDemandsHoarder;
				}

				if(level == E_BLUR_SCORE_MOVE_BONUS_LEVEL_3)
				{
					IncreaseScore(pScoringConfig->mBonusFanDemandsPerfect);

					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_FAN_DEMAND_PERFECT" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusFanDemandsPerfect;
				}

				if(level == E_BLUR_SCORE_MOVE_BONUS_LEVEL_4)
				{
					IncreaseScore(pScoringConfig->mBonusFanDemandsInARow);

					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_FAN_DEMAND_IN_A_ROW" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusFanDemandsInARow;
				}

				if(level == E_BLUR_SCORE_MOVE_BONUS_LEVEL_5)
				{
					IncreaseScore(pScoringConfig->mBonusFanRunFirstTime);

					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_FAN_RUN_FIRST_TIME" );
					bonus_str.Format(bonus_text.String());
					bonus_fans = pScoringConfig->mBonusFanRunFirstTime;
				}
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_SURVIVE_BARRAGE:
			{
				IncreaseScore(mSurvivedBarrageFans);

				static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_SURVIVED_BARRAGES" );
				bonus_str.Format(bonus_text.String(), about_vehicle);
				bonus_fans = mSurvivedBarrageFans;
			}
			break;

		case E_BLUR_SCORE_MOVE_BONUS_COMBO:
			{
				if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
					LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Given Instant Fully Bolted", mVehicleIndex );
				IncreaseScore(mComboScore);

				if(level == E_BLUR_SCORE_MOVE_BONUS_LEVEL_1)
				{
					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_COMBO" );
					bonus_str.Format(bonus_text.String());
				}
				else
				{
					static UI::C_TextStringID bonus_text( "SCR_NEW_AMAX_SCORING_MAX_COMBO" );
					bonus_str.Format(bonus_text.String());
				}

				bonus_fans = mComboScore;
			}
			break;

		default:
			break;
	}

	if(bonus != E_BLUR_SCORE_MOVE_BONUS_COMBO)
	{
		mCombo++;
		mComboTimer=pScoringConfig->combo_time;
		m_pScoring->mCombo=mCombo;
		m_pScoring->mComboTimer=mComboTimer;
	}

	F32 additionalScore = mTotalScore - TotalScore;
	if(additionalScore>0)
	{
		mScoringHistory.AddScoringEvent( ScoreEventHistory::ScoreEvent( (int)additionalScore, C_GameApp::GetInstance()->GetElapsedRaceTime()) );
	}

	BossDemandAction(E_BOSSDEMAND_ACTION_GOT_FANS, additionalScore);

	if(bonus_str.Length())
	{
		if(IsCurrentScoreBaseMinor() || bonus == E_BLUR_SCORE_MOVE_BONUS_COMBO)
		{
			UI::C_WideString128 minor_str;
			UI::C_WideString64 score_str;

			static UI::C_TextStringID score_text( "SCR_NEW_AMAX_SCORING_SCORE_STRING" );
			score_str.Format( score_text.String(), bonus_fans );

			minor_str.Format( L"%s %s", bonus_str.Chars(), score_str.Chars() );

			UI::C_UiScoringEventMinor uimsg3(minor_str.Chars(), mVehicleIndex, true);
			_UI_MESSAGE_FULL( uimsg3 );

			if ( bonus == E_BLUR_SCORE_MOVE_BONUS_COMBO && mVehicleIndex < mpLevelState->mNumViewports )
				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_HudFanComboEndActionIn );	// only play for local players
		}
		else
		{
			UI::C_UiScoringEventChain uimsg2(bonus_str.Chars(), bonus_fans, mVehicleIndex);
			_UI_MESSAGE_FULL( uimsg2 );
		}
	}

	m_pScoring->mFinalScore=mTotalScore;
	m_pScoring->mFinalScoreForUI=m_pScoring->mFinalScore;
}

//	Instruct another vehicle to score fans based on aggressively affecting this vehicle.
void C_VehicleComponent_NewScoring::VictimBonusScore( BlurScoreMoveSummary& summary, U8 aggressorVehicleIndex )
{
	U8 lastHitBy = m_pScoring->mMostRecentVehicleIdxHitBy;
	//	Don't give scores for doing anything to yourself...obviously.
	if(lastHitBy == mVehicleIndex || lastHitBy == Game::SYSTEM_FIRING_INDEX)
		return;

	Game::Actors::C_VehicleLogicActor* pLogicActor = ((Game::Actors::C_VehicleLogicActor* )GetActor());
	//	We know which vehicle last hit this one, so that's the aggressor. Unless we specified one as a parameter...
	aggressorVehicleIndex = aggressorVehicleIndex == (U8)-1 ? lastHitBy : aggressorVehicleIndex;

	//	Don't give scores for attacking teammates
	if(Game::VehiclesAreTeammates(mVehicleIndex, aggressorVehicleIndex))
		return;

	BIZ_ASSERT_MSG(aggressorVehicleIndex != INVALID_VEHICLE_IDX, "C_VehicleComponent_NewScoring::VictimBonusScore() > Invalid aggressor vehicle index...");
	
	C_VehicleLogicMsg_AggressiveScore scoreEvent(aggressorVehicleIndex, mVehicleIndex,summary);

	if(Game::VehicleIsLocal(aggressorVehicleIndex))
	{
		Game::Actors::C_VehicleLogicActor* pLocalAggressorLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(aggressorVehicleIndex);
		pLogicActor->SendMessageToActor(pLocalAggressorLogicActor->GetHandle(), &scoreEvent);


		//SP Destruction is interested in this too
		if(mIsSPDestruction)
		{
			Game::C_RaceModeDestruction* pRaceMode = (Game::C_RaceModeDestruction*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
			pRaceMode->MessageHandler(&scoreEvent); // Should we really be sending this directly... ?
		}
	}
	// Some stickers need to know when this happens
	Game::C_StickerManager::Instance().MessageHandler(&scoreEvent);
	
	//	If we're playing a network game we need to send this to the aggressor (but probably let everyone have it so we can use it for cool UI stuff?)
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
	{
		const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_EventManager->Add_AggressiveAction(mpLevelState->mCars[aggressorVehicleIndex].mNetJoinId, mpLevelState->mCars[mVehicleIndex].mNetJoinId, summary);
	}
}

void C_VehicleComponent_NewScoring::SetRivalIndex(S8 rivalIndex)
{
	
	// Store prev index (if was valid)
	if(m_pScoring->mRivalIndex != -1)
	{
		m_pScoring->mPrevRivalIndex = m_pScoring->mRivalIndex;
	}
	
	// Store the new index
	m_pScoring->mRivalIndex = rivalIndex;
}

S8 C_VehicleComponent_NewScoring::GetRivalIndex() const
{
	return m_pScoring->mRivalIndex;
}

void C_VehicleComponent_NewScoring::SelectRivalIndex_UseCurrentLeaderboard()
{
	// Are there more than 1 racers (should be in a real race... but for debug..)
	if(!OtherVehiclesStillInRace())
	{
		return;
	}

	// So... look at current leaderboard	
	S8 rivalVehIndex = -1;

	Game::Actors::C_VehicleLogicActor* vehActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	if( vehActor->GetPositionInfo().mRacePosition != 1 )
	{
		// Set rival index to person in position[OurPos + 1]	
		U8 rivalPosition	 = vehActor->GetPositionInfo().mRacePosition-1;
		S8 VehicleIndexAtPos = GetVehicleIndexAtRacePosition(rivalPosition);

		// Attempt to choose person 1 pos ahead unless they were previous rival
		if( VehicleIndexAtPos == GetRivalIndex() )
		{
			// Reset to check next player
			VehicleIndexAtPos = -1; 

			// Whilst still more racers ahead of us to try....
			if(vehActor->GetPositionInfo().mRacePosition != 2 ) // if in 2nd then we've already tried guy in 1st... no1 left!
			{
				rivalPosition++;
				VehicleIndexAtPos = GetVehicleIndexAtRacePosition(rivalPosition);
			}
		}

		if(IsVehicleValidRival( VehicleIndexAtPos ))
		{
			rivalVehIndex = VehicleIndexAtPos; 
		}
	}
	else
	{
		// SPECIAL CASE - if in first, rival is player in 2nd
		U8 nextPosToTry = 2; // try second player
		S8 VehicleIndexAtPos = GetVehicleIndexAtRacePosition(nextPosToTry);
		if(VehicleIndexAtPos == GetRivalIndex())
		{
			// Reset to check next player
			VehicleIndexAtPos = -1; 

			// If more than 2 cars in race...
			if(mpLevelState->mNumCars >= 3)
			{
				// If 2nd not valid try 3rd place...
				nextPosToTry++;
				VehicleIndexAtPos = GetVehicleIndexAtRacePosition(nextPosToTry);
			}
		}

		if(IsVehicleValidRival( VehicleIndexAtPos ))
		{
			rivalVehIndex = VehicleIndexAtPos; 
		}
	}

	if(rivalVehIndex != -1)
	{
		SetRivalIndex( rivalVehIndex );
	}
	else
	{
		// Plan B!
		SelectRivalIndex_Random();
	}

	// New rival selected.. fire off a UI message incase we need to refresh the display
	S32 playerIndex = UI::GetLocalPlayerIndex(mVehicleIndex); // Make sure this is a player, not an AI...
	if(playerIndex >= 0)	
	{
		_UI_MESSAGE1(UI::E_UiGameMsg_NewRivalSelected, playerIndex);
	}
}

S8 C_VehicleComponent_NewScoring::GetVehicleIndexAtRacePosition(const U8 queryRacePosition ) const
{
	// Search through data structures to find the vehicle we are after 
	if( !mIsLocalGame  )
	{
		Game::C_MpResults MPResults = Game::C_MpResults::Instance();
		U8 numVehicles = mRaceLogicLayerPtr->GetNumVehicles();  
		for(U8 i = 0; i < numVehicles; i++)
		{
			if( MPResults.GetPlayerPosition(i) == queryRacePosition )
			{
				return i;
			}
		}
	}
	else
	{
		// Build using local level state data
		for(U8 i = 0; i < mpLevelState->mNumCars; i++)
		{
			if(i != mVehicleIndex )
			{
				Game::Actors::C_VehicleLogicActor* pLogicActor = mRaceLogicLayerPtr->GetLogicVehicleActor(i);
				if(pLogicActor && pLogicActor->GetPositionInfo().mRacePosition == queryRacePosition )
				{
					return i;
				}
			}
		}
	}

	// If reach here then could not find vehicle at this position
	return -1;
}

bool C_VehicleComponent_NewScoring::OtherVehiclesStillInRace() const
{
	if( !mIsLocalGame )
	{
		Game::MpVehiclesData* pMPVehicleData = &Game::C_MpResults::Instance().m_LiveVehicleData;
		U8 numVehicles = mRaceLogicLayerPtr->GetNumVehicles(); 
		for(U8 i = 0; i < numVehicles; i++)
		{
			U8 localID = Game::GetLocalIndexFromJoinId((*pMPVehicleData)[i].m_NetJoinId);
			// Only valid rival if not self, and vehicle still in play
			if(localID != mVehicleIndex &&
				(*pMPVehicleData)[i].m_State == Game::E_VEHICLE_STATE_RACING )
			{
				return true;
			}
		}
	}
	else
	{
		// Build using local level state data
		for(U8 i = 0; i < mpLevelState->mNumCars; i++)
		{
			if(i != mVehicleIndex &&
				mpLevelState->mCars[i].mState == Game::E_VEHICLE_STATE_RACING )
			{
				return true;
			}
		}
	}

	// No other vehicles still in race
	return false;
}

bool C_VehicleComponent_NewScoring::IsVehicleValidRival( const U8 vehIndex ) const
{
	// Cant have ourselves as a rival
	if(vehIndex == mVehicleIndex) // || vehIndex < 0) - Removed as U8 can never be less than zero
	{
		return false;
	}

	// If somebody else, check they are still racing
	if( !mIsLocalGame  )
	{
		// ONLINE / SYSTEM LINK //
		if( vehIndex < mRaceLogicLayerPtr->GetNumVehicles() && 
			Game::C_MpResults::Instance().m_LiveVehicleData[vehIndex].m_State == Game::E_VEHICLE_STATE_RACING )
		{
			return true;
		}
	}
	else
	{	
		// LOCAL //
		if( vehIndex < mpLevelState->mNumCars && 
			mpLevelState->mCars[vehIndex].mState == Game::E_VEHICLE_STATE_RACING  )
		{
			return true;
		}
		
	}

	// Not valid
	return false;
}

void C_VehicleComponent_NewScoring::SelectRivalIndex_Random()
{
	// Build a local list of all vehicles still in race
	Btl::Vector<U8> validRivalVehicleIndices;
	validRivalVehicleIndices.Reserve(MAX_VEHICLES); 
	validRivalVehicleIndices.Clear();

	// Use live vehicle data if online else local
	if( !mIsLocalGame  )
	{
		BuildValidRivalVehicleIndices_Online(&validRivalVehicleIndices);
	}
	else
	{	
		BuildValidRivalVehicleIndices_Local(&validRivalVehicleIndices);
	}

	// Randomly select any racer!
	if(!validRivalVehicleIndices.Empty())
	{
		U8 randomRivalIndex = mRandomGenerator.Rand( validRivalVehicleIndices.Size() - 1 );
		SetRivalIndex( validRivalVehicleIndices[randomRivalIndex] );
	}

	// New rival selected.. fire off a UI message incase we need to refresh the display
	S32 playerIndex = UI::GetLocalPlayerIndex(mVehicleIndex); // Make sure this is a player, not an AI...
	if(playerIndex >= 0)	
	{
		_UI_MESSAGE1(UI::E_UiGameMsg_NewRivalSelected, playerIndex);
	}
	
}

void C_VehicleComponent_NewScoring::BuildValidRivalVehicleIndices_Online(Btl::Vector<U8>* constainer)
{
	// Build using live multiplayer data
	Game::MpVehiclesData* pMPVehicleData = &Game::C_MpResults::Instance().m_LiveVehicleData;
	U8 numVehicles = mRaceLogicLayerPtr->GetNumVehicles(); 
	for(U8 i = 0; i < numVehicles; i++)
	{
		U8 localID = Game::GetLocalIndexFromJoinId((*pMPVehicleData)[i].m_NetJoinId);
		// Only valid rival if not self, and vehicle still in play, and not prev rival
		if(localID != mVehicleIndex &&
		   (*pMPVehicleData)[i].m_State == Game::E_VEHICLE_STATE_RACING &&
		   localID != GetRivalIndex() )
		{
			constainer->PushBack(i); 
		}
	}
}

void C_VehicleComponent_NewScoring::BuildValidRivalVehicleIndices_Local(Btl::Vector<U8>* container)
{
	// Build using local level state data
	for(U8 i = 0; i < mpLevelState->mNumCars; i++)
	{
		if(i != mVehicleIndex &&
			mpLevelState->mCars[i].mState == Game::E_VEHICLE_STATE_RACING &&
		    i != GetRivalIndex())
		{
			container->PushBack(i); 
		}
	}
}

void C_VehicleComponent_NewScoring::HandleRivalDestroyed()
{
	// ATTEMPT to select new rival (case everybody has left race..)
	SelectRivalIndex_UseCurrentLeaderboard(); 
}

bool C_VehicleComponent_NewScoring::WreckedByAnotherPlayer() const 
{
	return  m_pScoring ? m_pScoring->mWreckedByAnotherPlayer : false; 
}

bool C_VehicleComponent_NewScoring::DisplayWreckedByAnotherPlayer() const 
{
	return  m_pScoring ? m_pScoring->mDisplayWreckedByAnotherPlayer : false; 
}

bool C_VehicleComponent_NewScoring::WasRecentlyHitByVehicle(U8 vehicleIndex) const
{
	return mLastHitByVehicleTimers[vehicleIndex] > 0.0f;
}

void C_VehicleComponent_NewScoring::SetWreckedByAnotherPlayer(bool wrecked)
{ 
	if(m_pScoring){ m_pScoring->mWreckedByAnotherPlayer = wrecked; }
}

void C_VehicleComponent_NewScoring::SetDisplayWreckedByAnotherPlayer(bool wrecked)
{ 
	if(m_pScoring){ m_pScoring->mDisplayWreckedByAnotherPlayer = wrecked; }
}

S32 C_VehicleComponent_NewScoring::GetIndexOfVehicleThatWreckedUs() const 
{ 
	return m_pScoring ? m_pScoring->mMostRecentVehicleIdxWreckedBy : -1;
}

void C_VehicleComponent_NewScoring::ResetHitByPerkRecentlyTimer(int firingIdx)
{
	if(firingIdx != Game::SYSTEM_FIRING_INDEX)
		mLastHitByVehicleTimers[firingIdx]=3.f;
}

bool C_VehicleComponent_NewScoring::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("C_VehicleComponent.NewScoring.MessageHandler");
	
	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	bool fully_bolted=false;

	switch(pMessage->Type())
	{
		case Game::E_VehicleMessage_LeftRace:
			{
				const Game::Actors::C_VehicleLeftRaceMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleLeftRaceMsg>(pMessage);

				// If this was our rival.. choose a new rival
				if(pMsg->m_VehicleIndex == GetRivalIndex())
				{
					SelectRivalIndex_UseCurrentLeaderboard();
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
						mPreviousFiredPickupID=pMsg->mUniqueId;
						mHitWithPowerUp=false;

						if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
							LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Fired Pickup %d with ID %d", mVehicleIndex, pMsg->mPickupType, mPreviousFiredPickupID );

						PowerUpFireState pu_states;
						pu_states.mDrifting = mDrifting;
						pu_states.mInAir = (m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count);
						pu_states.mUsingNitrous = vehActor->GetScoring().mUsingNitroTimer ? true : false;
						mPowerUpFireStates.Insert(pMsg->mUniqueId, pu_states);
					}
					else if(pMsg->mPickupType == E_PICKUP_TYPE_NITROUS)
					{
						if (vehActor->GetScoring().mUsingNitroTimer <= 0.0f)		// Don't reset if already nitroing
						{
							mNitrousOvertakes = 0;
							mRepeatNitros = 1;
						}
						else
						{
							mRepeatNitros++;
						}

						vehActor->GetScoring().mUsingNitroTimer=pScoringConfig->using_nitrous_timer;
						
						if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
							mUsingNitroAltType=true;
						else
							mUsingNitroAltType=false;

						mNitroDamageReceived=false;
						mNitrousAir=false;

						if(mDrifting)
						{
							StartScore(E_BLUR_SCORE_MOVE_BASE_NITROUS, mVehicleIndex);
							AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_DRIFT);
							EndScore();
						}
						else if((m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count))
						{
							mNitrousAir=true;
						}
					}
					else if(pMsg->mPickupType == E_PICKUP_TYPE_SHIELD)
					{
						mShieldedTime=0.f;
					}
				}
				
				return true;
			}

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				//	No score for hitting a teammate
				if(Game::VehiclesAreTeammates(pMsg->mFiringVehicleIndex, pMsg->mVehicleIndex))
					return true;

				bool end_score=false;

				if( pMsg->mVehicleIndex == mVehicleIndex && pMsg->mFiringVehicleIndex != Game::SYSTEM_FIRING_INDEX)
				{
					mLastHitByVehicleTimers[pMsg->mFiringVehicleIndex]=3.f;

					m_pScoring->mRevengeIndex=pMsg->mFiringVehicleIndex;
					m_pScoring->mRevengeTimer=pScoringConfig->revenge_timer;

					if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
						LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Hit By Vehicle %d with %d", mVehicleIndex, pMsg->mFiringVehicleIndex, pMsg->mPickupType );
				}
				else if( pMsg->mFiringVehicleIndex == mVehicleIndex )
				{
					if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
						LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Hit Vehicle %d with %d", mVehicleIndex, pMsg->mVehicleIndex, pMsg->mPickupType );

					mLastHitVehicleTimers[pMsg->mVehicleIndex]=0.5f;

					if(pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_DECOY)	// We'll need a separate scoring message for decoy
					{
						switch(pMsg->mPickupType)
						{
							case E_PICKUP_TYPE_SHUNT:
								{
									StartScore(E_BLUR_SCORE_MOVE_BASE_SHUNT, pMsg->mVehicleIndex);
									end_score=true;
								}
								break;

							case E_PICKUP_TYPE_BLINDFIRE:
								{
									BossDemandAction(E_BOSSDEMAND_ACTION_BOLT_HIT);

									if(mIsSPDestruction)
									{
										const int damageInflicted = RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE)->mNormal->mBalancingData->GetDamageInflicted();

										Game::Actors::C_VehicleLogicActor* otherVehActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(pMsg->mVehicleIndex);
										
										if(otherVehActor->GetCurrentHealth() <= damageInflicted)
										{
											return true;
										}
									}

									StartScore(E_BLUR_SCORE_MOVE_BASE_BOLT, pMsg->mVehicleIndex);
									end_score=true;

									float dist=vehActor->mVehicleProximities[pMsg->mVehicleIndex].mDistanceSquared;
									if(dist > (pScoringConfig->long_distance*pScoringConfig->long_distance))
									{
										AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LONG_SHOT);
									}

									Btl::Map<U32, PowerUpBoltState>::Iterator it = mPowerUpBoltStates.Find(pMsg->mGroupID);
									if(it == mPowerUpBoltStates.End())	//must be the first bolt of this group
									{
										PowerUpBoltState bolt_state;
										bolt_state.mVehicleIndex = pMsg->mVehicleIndex;
										bolt_state.mTimesHit = 1;
										bolt_state.mTimesFired = 1;
										bolt_state.mLastID = pMsg->mUniqueID;
										mPowerUpBoltStates.Insert(pMsg->mGroupID, bolt_state);
									}
									else
									{
										if(it->GetValue().mLastID == pMsg->mUniqueID)
										{
											it->GetValue().mVehicleIndex = pMsg->mVehicleIndex;
										}

										if(it->GetValue().mVehicleIndex == pMsg->mVehicleIndex)
										{
											it->GetValue().mTimesHit++;
											it->GetValue().mLastID = pMsg->mUniqueID;

											if(it->GetValue().mTimesHit == mNumBoltsPerSet)
											{
												fully_bolted=true;
												BossDemandAction(E_BOSSDEMAND_ACTION_3_BOLT_HIT);
												AddBonusScore(E_BLUR_SCORE_MOVE_BONUS_FULLY_BOLTED, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1);
											}
										}
									}
								}
								break;

							case E_PICKUP_TYPE_BARGE:

								// If last gasp barge.. listen for the seperate last gasp scoring move
								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_MOD)
								{
									StartScore(E_BLUR_SCORE_MOVE_BASE_LAST_GASP, pMsg->mVehicleIndex);
									EndScore();
								}
								else
								{
									mNumBarges++;
									if(mNumBarges == 1)
									{
										mBargeDrift=false;
										mBargeNitrous=false;
										mBargeAir=false;

										mFirstBargeIndex=pMsg->mVehicleIndex;
									}
								}
								break;

							case E_PICKUP_TYPE_BOMB:
								{
									StartScore(E_BLUR_SCORE_MOVE_BASE_MINE, pMsg->mVehicleIndex);

									Game::C_PickupBomb* pBomb = (Game::C_PickupBomb*)(Game::C_PickupsManager::Instance().GetPickup(pMsg->mUniqueID));

									if(pBomb)
									{
										if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_NORMAL && pBomb->GetHasBeenRecentlyLobbed())
										{
											AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
										}
									}

									end_score=true;
								}
								break;

							case E_PICKUP_TYPE_SHOCK:
								{
									StartScore(E_BLUR_SCORE_MOVE_BASE_SHOCK, pMsg->mVehicleIndex);
									end_score=true;
								}
								break;

							case E_PICKUP_TYPE_NITROUS:
								if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
								{
									StartScore(E_BLUR_SCORE_MOVE_BASE_NITROUS, pMsg->mVehicleIndex);
									end_score=true;
									AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE);
								}
								break;
						}
					}

					if(pMsg->mPickupType == E_PICKUP_TYPE_SHOCK || pMsg->mPickupType == E_PICKUP_TYPE_SHUNT || pMsg->mPickupType == E_PICKUP_TYPE_BARGE || pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE || pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
					{
						if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED || pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_MOD_REVERSED || 
							pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED)
						{
							if(pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
							{
								BossDemandAction(E_BOSSDEMAND_ACTION_FORWARD_MINE_HIT);
							}
							
							if(pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE)
							{
								BossDemandAction(E_BOSSDEMAND_ACTION_REVERSE_BOLT);
							}
							
							AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE);
						}

						Btl::Map<int, PowerUpFireState>::Iterator it = mPowerUpFireStates.Find(pMsg->mUniqueID);
						if(it != mPowerUpFireStates.End())
						{
							if(it->GetValue().mDrifting)
							{
								if(pMsg->mPickupType == E_PICKUP_TYPE_BARGE)
									mBargeDrift=true;
	
								if(pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE)
									AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_DRIFT);
							}
							if(it->GetValue().mInAir)
							{
								if(pMsg->mPickupType == E_PICKUP_TYPE_BARGE)
									mBargeAir=true;
								
								if(pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE || pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
									AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_AIR);
							}
							if(it->GetValue().mUsingNitrous)
							{
								if(pMsg->mPickupType == E_PICKUP_TYPE_BARGE)
									mBargeNitrous=true;
							}
							mPowerUpFireStates.Erase(pMsg->mUniqueID);
						}

						if( mPreviousFiredPickupID == pMsg->mUniqueID && pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE)
						{
							mHitsInARow++;
							mHitWithPowerUp=true;
							if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
								LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Got %d Hits in a Row", mVehicleIndex, mHitsInARow );
						}
						
						if( mHitsInARow == 3 )
						{
							if(!fully_bolted)
							{
								AddBonusScore(E_BLUR_SCORE_MOVE_BONUS_ACCURACY, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1);
							}

							if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
								LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Reseting Hits in a Row", mVehicleIndex );

							mHitsInARow = 0;	
						}
					}

					if(mVendettaIndex == pMsg->mVehicleIndex)
					{
						mVendettaCount++;
						mVendettaTime=pScoringConfig->vendetta_cooldown;

						if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
							LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Hit Vehicle %d - %d Times in a Row", mVehicleIndex, mVendettaIndex, mVendettaCount );
						
						if(mVendettaCount == 4)	//scale depending on size of race?
						{
							AddBonusScore(E_BLUR_SCORE_MOVE_BONUS_BULLYING, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1);
						}
					}
					else
					{
						if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
							LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Vehicle %d Vendetta Over", mVehicleIndex, mVendettaIndex );

						mVendettaTime = 0.f;
						mVendettaCount = 0;
						mVendettaIndex = pMsg->mVehicleIndex;
					}

					if(m_pScoring->mRevengeTimer && m_pScoring->mRevengeIndex == pMsg->mVehicleIndex)
					{
						if(pMsg->mPickupType == E_PICKUP_TYPE_BARGE)
							GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_REVENGE, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, m_pScoring->mRevengeIndex);
						else
							AddBonusScore(E_BLUR_SCORE_MOVE_BONUS_REVENGE, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1);
					}

					if(end_score)
					{
						EndScore();
					}
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

						StartScore(E_BLUR_SCORE_MOVE_BASE_SHIELD, mVehicleIndex);
						if(mShieldedTime < 0.25f)
						{
							AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL);
						}
						EndScore();
						
						if(pMsg->mPickupType == E_PICKUP_TYPE_BOMB)
						{
							BossDemandAction(E_BOSSDEMAND_ACTION_SHIELD_MINES);
						}
						else
						{
							BossDemandAction(E_BOSSDEMAND_ACTION_SHIELD_POWERUPS);
						}
					}
				}
				else if( pMsg->mFiringVehicleIndex == mVehicleIndex )
				{
					//GrantVehicleXP(pMsg->mDefendingVehicleIndex, AMAX_SCORING_EVENT_ENEMY_SHIELDED);
				}

				return true;
			}

		case E_VehicleLogicMsg_Repair:
			{
				const C_VehicleLogicRepairMsg* pMsg		= Messaging::MessageCast<C_VehicleLogicRepairMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_REPAIR, mVehicleIndex);
					if(vehActor->GetCurrentHealth() < pScoringConfig->late_repair_level)
					{
						AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL);
					}
					EndScore();
					
					//TODO - PK, only give if there was some health
					BossDemandAction(E_BOSSDEMAND_ACTION_REPAIR);

					mNoRepairOrWreck=false;
				}
		
				return true;
			}

		case E_VehicleLogicMsg_PowerUpDeleted:
			{
				const C_VehicleLogicMsg_PowerUpDeleted* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_PowerUpDeleted>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					if(pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE)
					{
						Btl::Map<U32, PowerUpBoltState>::Iterator it = mPowerUpBoltStates.Find(pMsg->mGroupId);
						if(it == mPowerUpBoltStates.End())	//must be the first bolt of this group
						{
							PowerUpBoltState bolt_state;
							bolt_state.mVehicleIndex = -1;
							bolt_state.mTimesHit = 0;
							bolt_state.mTimesFired = 1;
							bolt_state.mLastID = pMsg->mUniqueId;
							mPowerUpBoltStates.Insert(pMsg->mGroupId, bolt_state);
						}
						else
						{
							if(it->GetValue().mLastID != pMsg->mUniqueId)	//this didn't hit so we should deal with it
							{
								it->GetValue().mTimesFired++;
							}
						}

						if( mPreviousFiredPickupID == pMsg->mUniqueId && !mHitWithPowerUp)
						{
							if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
								LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - PowerUp %d Deleted and Reseting Hits in a Row", mVehicleIndex, mPreviousFiredPickupID );

							mHitsInARow=0;
						}
					}

					if(pMsg->mPickupType == E_PICKUP_TYPE_BARGE)
					{
						if( mNumBarges)
						{
							StartScore(E_BLUR_SCORE_MOVE_BASE_BARGE, mFirstBargeIndex);

							if(mBargeDrift)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_DRIFT);
										
							if(mBargeAir)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_AIR);

							if(mBargeNitrous)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_NITROUS);

							if(mNumBarges == 2)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
							if(mNumBarges == 3)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
							if(mNumBarges == 4)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
							if(mNumBarges == 5)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4);
							if(mNumBarges > 5)
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_5);

							if(mNumBarges > 1)
							{
								BossDemandAction(E_BOSSDEMAND_ACTION_MULTIPLE_BARGE_HIT, (F32)mNumBarges);
							}
							
							EndScore();

							mNumBarges=0;
						}
					}
				}

				return true;
			}

		case Game::E_GameLogicMessage_GameplayImpact:
			{
				const Game::C_GameplayImpactMsg* pMsg		= Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if( pMsg->mVehicleIdx != mVehicleIndex )
					return true;

				//	No score for hitting a teammate
				if(Game::VehiclesAreTeammates(pMsg->mAggressiveVehicleIdx, pMsg->mOtherVehicleIdx))
					return true;

				mCheckpointsDamageRecieved=true;
				mNitroDamageReceived=true;
				mLapDamageReceived=true;

				if(pMsg->mImpactLevel >= E_IMPACT_LEVEL_MEDIUM)
				{
					// If i was not the aggressor....
					if(pMsg->mAggressiveVehicleIdx != INVALID_VEHICLE_IDX && pMsg->mAggressiveVehicleIdx != mVehicleIndex) 
					{
						vehActor->ResetHitBy(pMsg->mAggressiveVehicleIdx);

						mLastHitByVehicleTimers[pMsg->mAggressiveVehicleIdx]=3.f;
					}
					else if(pMsg->mAggressiveVehicleIdx == mVehicleIndex)
					{
						Game::Actors::C_VehicleLogicActor* otherVehActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(pMsg->mOtherVehicleIdx);

						if( otherVehActor->mShieldTime == 0.f )
						{
							HandleMotorMashMessageAggressiveImpact(pMsg->mOtherVehicleIdx, pMsg->mAggressiveVehicleIdx, pMsg->mImpactLevel);
						}

						if(!mpLevelState->mCars[pMsg->mOtherVehicleIdx].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Stable_Frame)->GetActive())
						{
							StartScore(E_BLUR_SCORE_MOVE_BASE_RAMMING, pMsg->mOtherVehicleIdx);
							if(m_pScoring->mTimeInAir > pScoringConfig->time_in_air_for_it_to_count)
							{
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_AIR);
							}
							if(vehActor->GetScoring().mUsingNitroTimer)
							{
								AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_NITROUS);
								BossDemandAction(E_BOSSDEMAND_ACTION_NITRO_RAM);
							}

							if(mVendettaIndex == pMsg->mOtherVehicleIdx)
							{
								mVendettaCount++;
								mVendettaTime=pScoringConfig->vendetta_cooldown;

								if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
									LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Hit Vehicle %d - %d Times in a Row", mVehicleIndex, mVendettaIndex, mVendettaCount );
								
								if(mVendettaCount == 4)
								{
									AddBonusScore(E_BLUR_SCORE_MOVE_BONUS_BULLYING, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1);
								}
							}
							else
							{
								if(g_debug_NewScoring_OnlyLogPLayer == -1 || g_debug_NewScoring_OnlyLogPLayer == mVehicleIndex)
									LOGDEBUG(gVLC_NewScoringLogger, "Vehicle %d - Vehicle %d Vendetta Over", mVehicleIndex, mVendettaIndex );

								mVendettaTime = 0.f;
								mVendettaCount = 0;
								mVendettaIndex = pMsg->mOtherVehicleIdx;
							}

							if(m_pScoring->mRevengeTimer && m_pScoring->mRevengeIndex == pMsg->mOtherVehicleIdx)
							{
								AddBonusScore(E_BLUR_SCORE_MOVE_BONUS_REVENGE, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1);
							}

							vehActor->mStatistics.mCarsRammed++;

							EndScore();
						}
					}
				}

				return true;
			}

		case Game::E_GameflowMessage_RaceStarted:
			{
				AmaxEvent *ev=C_GameEventData::Instance().mEvents[(GAMEDATA_ID)mpLevelState->mEvent];

				mRaceStarted=true;

				return true;
			}

		case Game::E_GameflowMessage_VehicleFinished:
		case Game::E_GameflowMessage_RaceFinished:
			{
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
				
				BossDemandAction(E_BOSSDEMAND_ACTION_SECONDS_IN_FIRST, mTimeInFirstPlace);		// Report any time in first place when the race ends

				mRaceStarted=false;
		
				return true;
			}

		case Game::E_VehicleMessage_FatallyDamaged:
			{
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(mVehicleIndex == pMsg->m_VehicleIndex)
				{
					// default this var
					m_pScoring->mDisplayWreckedByAnotherPlayer = false;

					if (mVehicleIndex == 0)
						BossDemandAction(E_BOSSDEMAND_ACTION_PLAYER_WRECKED);

					mWrecksInARow=0;
					mNoRepairOrWreck=false;

					if(m_pScoring->mWreckedTimer > 0.f && m_pScoring->mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX && m_pScoring->mMostRecentVehicleIdxHitBy != Game::SYSTEM_FIRING_INDEX && m_pScoring->mMostRecentVehicleIdxHitBy != mVehicleIndex && (mLastHitByVehicleTimers[m_pScoring->mMostRecentVehicleIdxHitBy] > 0))
					{
						BlurScoreMoveSummary summary;
						summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_WRECKED;

						if(mpLevelState->mCars[m_pScoring->mMostRecentVehicleIdxHitBy].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck)->GetActive())
						{
							summary.mBonusLevel = E_BLUR_SCORE_MOVE_BONUS_LEVEL_2;
						}

						VictimBonusScore(summary);
						m_pScoring->mWreckedTimer = 0.f;
						m_pScoring->mWreckedByAnotherPlayer = true;
						m_pScoring->mDisplayWreckedByAnotherPlayer = true;
						m_pScoring->mMostRecentVehicleIdxWreckedBy = m_pScoring->mMostRecentVehicleIdxHitBy;

						if( mpLevelState->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction && !mpLevelState->mTeams )
						{
							// Record our new rival
							SetRivalIndex(m_pScoring->mMostRecentVehicleIdxHitBy);
						}
					}
				}

				return true;
			}

		case E_VehicleLogicMsg_AggressiveScore:
			{
				const C_VehicleLogicMsg_AggressiveScore* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AggressiveScore>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					// If in motormash we want to award points for drowning / pushing another vehicle off a cliff
					HandleMotormashMessageAggressiveScore(pMsg->mAboutVehicleIndex, pMsg->mVehicleIndex, pMsg->mBonusScoringEvent);

					if(pMsg->mBonusScoringEvent == E_BLUR_SCORE_MOVE_BONUS_DROWN)
					{
						BossDemandAction(E_BOSSDEMAND_ACTION_RAM_INTO_SEA);
					}
					if(pMsg->mBonusScoringEvent == E_BLUR_SCORE_MOVE_BONUS_CLIFF)
					{
						BossDemandAction(E_BOSSDEMAND_ACTION_BARGE_OFF_CLIFF);
					}
					if(pMsg->mBonusScoringEvent == E_BLUR_SCORE_MOVE_BONUS_WRECKED)
					{
						//give the wrecked T-Shirt
						if(mVehicleIndex == 0)
						{
							Game::C_AvatarAwardSystem::Instance().GiveAvatarAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), Game::BLUR_AVATAR_AWARD_WRECK_TEE);
						}

						BossDemandAction(E_BOSSDEMAND_ACTION_WRECKED_ANOTHER);
					}
				
					if(pMsg->mBonusScoringEvent == E_BLUR_SCORE_MOVE_BONUS_WRECKED)
					{
						mWrecksInARow++;

						if(mWrecksInARow == 3)
						{
							mWrecksInARow=0;

							if(!mIsSPDestruction)
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_KILLING_SPREE, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, pMsg->mAboutVehicleIndex);
						}
						else
						{
							if(!mIsSPDestruction)
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_WRECKED, pMsg->mBonusLevel, pMsg->mAboutVehicleIndex);
						}

						return true;
					}
					if(pMsg->mBase == E_BLUR_SCORE_MOVE_BASE_HIT_DECOY)
					{
						mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Decoy_Drop, true);

						// Lets track this useful stat
						Game::Actors::C_VehicleLogicActor* pLogicActor			= ((Game::Actors::C_VehicleLogicActor*)GetActor());
						if(pLogicActor)
						{
							pLogicActor->mStatistics.mPlayersHitwithDecoys++; 
						}
						StartScore(E_BLUR_SCORE_MOVE_BASE_HIT_DECOY, pMsg->mAboutVehicleIndex);
						EndScore();
						
						return true;
					}
					GiveInstantBonusScore(pMsg->mBonusScoringEvent, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, pMsg->mAboutVehicleIndex);
				}

				return true;
			}

		case E_VehicleLogicMsg_PowerUpDestroyedByPowerUp:
			{
				const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_PowerUpDestroyedByPowerUp>(pMessage);

				bool isMiniMine = (pMsg->mPickupType == E_PICKUP_TYPE_BOMB && (pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_MINI || pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_MOD));
				if(pMsg->mVehicleIndex == mVehicleIndex && !isMiniMine)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_HIT_ANOTHER_PICKUP, mVehicleIndex);
					EndScore();
					
					if(pMsg->mPickupType == E_PICKUP_TYPE_SHUNT)
					{
						mShuntsDestroyed.Insert(pMsg->mUniqueId);
					}

					// Record this stat
					Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
					if(pLogicActor)
					{
						pLogicActor->mStatistics.mPowerupsBlocked++;
					}

					BossDemandAction(E_BOSSDEMAND_ACTION_USE_WEAPON_DEFENCE);
				}

				// Some stickers need to know when this happens
				Game::C_StickerManager::Instance().MessageHandler(pMsg);

				return true;
			}

		case E_VehicleLogicMsg_EvadedPowerUp:
			{
				const C_VehicleLogicMsg_EvadedPowerUp* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_EvadedPowerUp>(pMessage);

				if( mVehicleIndex == pMsg->mVehicleIndex && pMsg->mVehicleIndex != pMsg->mFirerIndex)
				{
					Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

					if( pMsg->mPickupType == E_PICKUP_TYPE_SHOCK)
					{
						StartScore(E_BLUR_SCORE_MOVE_BASE_EVASION, pMsg->mVehicleIndex);
						AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1);
						EndScore();

						BossDemandAction(E_BOSSDEMAND_ACTION_EVADE_SHOCK);
					}
					else if( pMsg->mPickupType == E_PICKUP_TYPE_SHUNT)
					{
						Btl::Set<U32>::Iterator it = mShuntsDestroyed.Find(pMsg->mUniqueId);

						if (it == mShuntsDestroyed.End())	//we didn't destroy it, so its a dodge
						{
							StartScore(E_BLUR_SCORE_MOVE_BASE_EVASION, pMsg->mVehicleIndex);
							AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
							EndScore();
						}
						else
						{
							mShuntsDestroyed.Erase(it);
						}
					}
				}

				// Some stickers need to know when this happens
				Game::C_StickerManager::Instance().MessageHandler(pMsg);
				
				return true;
			}

		case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(pMsg->mLap > mMaxLapReached)
					{
						if(!mLapDamageReceived)
						{
							StartScore(E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING, pMsg->mVehicleIndex);
							AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3);
							EndScore();
						}
						else if(mNoRepairOrWreck)
						{
							StartScore(E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING, pMsg->mVehicleIndex);
							AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2);
							EndScore();
						}

						mNoRepairOrWreck=true;
						mLapDamageReceived=false;
						mMaxLapReached = pMsg->mLap;
					}
				}

				return true;
			}

		case E_VehicleLogicMsg_Fan_Demand_Passed:
			{
				const C_VehicleLogicMsg_Fan_Demand_Passed* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_Fan_Demand_Passed>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					if(pMsg->mActuallyFailed == false)
					{
						mFanDemandsInARow++;
						mFanDemandFans=pMsg->mFans;

						if(pMsg->mFanRun)
						{
							if(!mAttemptedFanRun)
							{
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, E_BLUR_SCORE_MOVE_BONUS_LEVEL_5, mVehicleIndex);
							}
							else
							{
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, mVehicleIndex);
							}

							mAttemptedFanRun=true;
						}
						else
						{
							if(mFanDemandsInARow == 3)
							{
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, E_BLUR_SCORE_MOVE_BONUS_LEVEL_4, mVehicleIndex);
							}
							else if(pMsg->mPerfect)
							{
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, E_BLUR_SCORE_MOVE_BONUS_LEVEL_3, mVehicleIndex);
							}
							else if(pMsg->mPickupsLeft)
							{
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, E_BLUR_SCORE_MOVE_BONUS_LEVEL_2, mVehicleIndex);
							}
							else
							{
								GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, mVehicleIndex);
							}
						}
					}
					else
					{
						mFanDemandsInARow=0;
						if(pMsg->mFanRun)
						{
							mAttemptedFanRun=true;
						}
					}
				}

				return true;
			}

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

		case E_VehicleLogicMsg_HOGReset:
			{
				const C_VehicleLogicMsg_HoGResetMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HoGResetMsg>(pMessage);

				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
					vehActor->GetGraphicalActor(mVehicleIndex)->SetGoneOffTrack(false);

					mHasDrowned=false;
					mHasPlummeted=false;
					mHOGTimer=0.25f;
				}

				return true;
			}

		case Game::E_PickupMessage_CollectStopwatch:
			{
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(vehActor->GetScoring().mCheckpointTimeLeft < 0.5f)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_STOPWATCH, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL);
					EndScore();
				}

				if(vehActor->GetScoring().mUsingNitroTimer > 0.f && mUsingNitroAltType)
				{
					StartScore(E_BLUR_SCORE_MOVE_BASE_STOPWATCH, mVehicleIndex);
					AddSkillScore(E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE);
					EndScore();
				}
			}
			break;

		case Game::E_PickupMessage_HitDecoy:
			{
				const Game::Actors::HitDecoyMsg* pMsg	= Messaging::MessageCast<Game::Actors::HitDecoyMsg>(pMessage);
				if(pMsg->mFiringVehicleIndex != mVehicleIndex)
				{
					BlurScoreMoveSummary summary;
					summary.mBase = E_BLUR_SCORE_MOVE_BASE_HIT_DECOY;
					VictimBonusScore(summary,pMsg->mFiringVehicleIndex);
				}
				else
				{
					//oops, we hit our own decoy! silly us, we should have remembered it was there since we dropped it...
				}

				return true;
			}

		case E_VehicleLogicMsg_SurvivedElimination:
			{
				const C_VehicleLogicMsg_Survived_EliminationMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_Survived_EliminationMsg>(pMessage);
				mSurvivedBarrageFans = pMsg->m_SurvivalFans;
				GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS_SURVIVE_BARRAGE, E_BLUR_SCORE_MOVE_BONUS_LEVEL_1, pMsg->m_BarrageNumberSurvived);
				return true;
			}

		return I_GameActorComponent::OnMessage(pMessage);
	}

	return false;
}

void C_VehicleComponent_NewScoring::BossDemandAction(E_BOSSDEMAND_ACTION type, F32 value)
{
	if (mVehicleIndex == 0)
		SP_BossDemands::Instance().RegisterBossDemandAction(type, value);
}


void C_VehicleComponent_NewScoring::CheckFreeFallingCliffTimer(const F32 frameTime)
{
	if(mFreeFallingOverCliffTimer > 0.0f)
	{
		mFreeFallingOverCliffTimer -= frameTime;
		if(mFreeFallingOverCliffTimer < 0.0f)
		{
			// Then we have NOT heard a *SPLASH*.. credit the player with some cliff pushing!
			mHasPlummeted = true;

			BlurScoreMoveSummary summary;
			summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_CLIFF;

			const Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			const AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
			if (pCity->forceFallIntoWater)					// Bodge because drown timer is not reliable! CANT HAVE BOTH ON IN THE SAME CITY
				summary.mBonus = E_BLUR_SCORE_MOVE_BONUS_DROWN;

			VictimBonusScore(summary);
		}
	}
}

ScoreEventHistory::ScoreEvent::ScoreEvent( int score, F32 timeStamp )
: mScore(score), mTimeStamp(timeStamp)
{
}

ScoreEventHistory::ScoreEventHistory()						
{
	Reset();
}

void
ScoreEventHistory::Reset()	
{
	mSize = 0;
	mIndex = 0;
}


void		
ScoreEventHistory::AddScoringEvent( const ScoreEventHistory::ScoreEvent& sEvent )
{
	mScoringEventsCircularBuffer[mIndex++] = sEvent;

	mIndex = mIndex % MAX_SCORE_EVENTS;
	mSize = mSize < MAX_SCORE_EVENTS ? mSize+1:mSize;
}

const ScoreEventHistory::ScoreEvent& 		
ScoreEventHistory::GetScoringEventAtIndex( int index ) const
{
	BIZ_ASSERT(index<GetScoringEventCount());

	int actualIndex = (mIndex - (index+1));
	actualIndex = actualIndex < 0 ? actualIndex + GetScoringEventCount():actualIndex; 
	return mScoringEventsCircularBuffer[ actualIndex ];
}

int							
ScoreEventHistory::GetScoringEventCount() const
{
	return mSize;
}


} // namespace Components
