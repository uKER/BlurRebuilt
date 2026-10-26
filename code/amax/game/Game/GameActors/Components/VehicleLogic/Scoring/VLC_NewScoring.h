#ifndef VLC_NEW_SCORING_H
#define VLC_NEW_SCORING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <game\GameActors\Components\GameComponents.h>
#include <GameLogic\Structures\VehicleScoringInfo.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <ui/util/WideString.h>
#include <GameLogic/SinglePlayer/SP_BossDemands.h>
#include <GameLogic/Abilities/DrifterAbility.h>

#define POWER_UP_BOLT_STATE_DELETION_TIME 2.f

// Forward decls
namespace Game
{
	class C_LevelState;
}

namespace Util
{
	class C_Random;
}

namespace Components
{
	class BaseMessageHandler;

	struct PowerUpFireState
	{
		bool mUsingNitrous;
		bool mInAir;
		bool mDrifting;
	};

	struct PowerUpBoltState
	{
		PowerUpBoltState() : mDeletionTimer(POWER_UP_BOLT_STATE_DELETION_TIME) {};

		U8 mVehicleIndex;
		U8 mTimesHit;
		U8 mTimesFired;
		U32 mLastID;
		float mDeletionTimer;
	};

	struct MinorScore
	{
		MinorScore(){ mTimer=0.f; mString.Clear(); };

		float mTimer;
		int mNumber;
		UI::C_WideString32 mString;
	};

	// To save passing all parameters around to functions individially...
	struct BlurScoreMoveSummary
	{
		BlurScoreMoveSummary()
		{
			mBase				= E_BLUR_SCORE_MOVE_BASE_NOTHING;
			mBaseType			= E_BLUR_SCORE_MOVE_BASE_TYPE_MINOR;
			mSkill				= E_BLUR_SCORE_MOVE_SKILL_LEVEL_1;
			mBonus			    = E_BLUR_SCORE_MOVE_BONUS_NONE;
			mBonusLevel			= E_BLUR_SCORE_MOVE_BONUS_LEVEL_1;
		};

		E_BLUR_SCORE_MOVE_BASE				mBase;
		E_BLUR_SCORE_MOVE_BASE_TYPE			mBaseType;
		E_BLUR_SCORE_MOVE_SKILL				mSkill;
		E_BLUR_SCORE_MOVE_BONUS				mBonus;
		E_BLUR_SCORE_MOVE_BONUS_LEVEL		mBonusLevel;

	};

	class ScoreEventHistory
	{
	public:
		class ScoreEvent
		{
		public:
			ScoreEvent() {}
			ScoreEvent( int score, F32 timeStamp );
		
			F32 mTimeStamp;
			int mScore;
		};

	public:
														ScoreEventHistory();						
		
		void											Reset();
		void											AddScoringEvent( const ScoreEvent& sEvent );
		const ScoreEventHistory::ScoreEvent&			GetScoringEventAtIndex( int index ) const;
		int												GetScoringEventCount() const;

	private:

		static const int								MAX_SCORE_EVENTS = 200;
		ScoreEvent										mScoringEventsCircularBuffer[MAX_SCORE_EVENTS];
		int												mSize;
		int												mIndex;
	};

	class I_VehicleComponent_NewScoring_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_NewScoring_CreateInfo() :	mVehicleIndex(INVALID_VEHICLE_IDX), 
														mpRaceLogicLayer(NULL),
														mpScoringInfo(NULL) {}

		static Component::C_ComponentTypeId				ClassType() {return E_VehicleComponent_NewScoring;}
		virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return E_VehicleComponent_NewScoring;}

		int												mVehicleIndex;
		Game::C_RaceLogicLayer*							mpRaceLogicLayer;
		VehicleScoringInfo*								mpScoringInfo;
	};

	class C_VehicleComponent_NewScoring : public I_GameActorComponent
	{
	public:

		C_VehicleComponent_NewScoring() :mpLevelState(NULL) {mRaceStarted=false;};

		static Component::C_ComponentInterfaceId		InterfaceType() {return E_IF_VehicleComponent_NewScoring;}
		virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return E_IF_VehicleComponent_NewScoring;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId		ClassType() {return E_VehicleComponent_NewScoring;}
		virtual Component::C_ComponentInterfaceId		GetComponentTypeID() const {return E_VehicleComponent_NewScoring;} // from Component::I_GameComponent

		virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

		void											Update();
		void											NearMiss(int vehicle_index);
		void											IncreaseScore(int score);

		F32												GetDriftDistance() const { return mDriftDistance; };
		bool											GetIsVehicleDrifting() const {return mDrifting;}

		bool											WreckedByAnotherPlayer() const;
		bool											DisplayWreckedByAnotherPlayer() const;
		void											SetDisplayWreckedByAnotherPlayer(bool wrecked); 
		void											SetWreckedByAnotherPlayer(bool wrecked);
		S32												GetIndexOfVehicleThatWreckedUs() const;
		void											ResetHitByPerkRecentlyTimer(int firingIdx);
		bool											WasRecentlyHitByVehicle(U8 vehicleIndex) const;
		bool											CurrentlyFreeFallingOverCliff() const {return (mFreeFallingOverCliffTimer > 0.0f);}

		int												GetTotalFans(){return mTotalScore;};

		void											HandleMotorMashMessagePickupHit(U8 vehicleHit, U8 firingVehicle, E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType);
		void											HandleMotorMashMessageAggressiveImpact(U8 vehicleHit, U8 aggressiveVehicle, E_IMPACT_LEVEL impactLevel);
		void											HandleMotormashMessageAggressiveScore(const U8 vehicleHit, const U8 aggressiveVehicle,E_BLUR_SCORE_MOVE_BONUS scoringMove);

		const ScoreEventHistory&						GetScoringHistory() const { return mScoringHistory; }


		// Public as VLC_Statistics needs to perform this operation for 'wrecks'. Save duplicating an identical function in each component.
		int												ApplyRivalScoreMultiplier(const int baseScore) const;

		// Rivals Functionality
		S8												GetRivalIndex() const;
		void											SelectRivalIndex_UseCurrentLeaderboard(); 
		void											SelectRivalIndex_Random(); 
		void											HandleRivalDestroyed(); 

	protected:
		virtual bool									CreateImpl();
		void											SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }
		void											BossDemandAction(E_BOSSDEMAND_ACTION type, F32 value = 1.0f);	// Report this action to the Boss demand system but only if it's the player


	//Functions
	private:

		BaseMessageHandler*								mMessageHandler;
		Actor::C_ActorHandle							mActorHandle;
		U8												mVehicleIndex;
		Game::C_RaceLogicLayer*							mRaceLogicLayerPtr;
		VehicleScoringInfo*								m_pScoring;

		bool											mRaceStarted;

		bool											IsCurrentScoreBaseMinor(bool checkMove=false);
		void											StartScore(E_BLUR_SCORE_MOVE_BASE base, int other_vehicle);
		void											AddSkillScore(E_BLUR_SCORE_MOVE_SKILL skill);
		void											AddBonusScore(E_BLUR_SCORE_MOVE_BONUS bonus, E_BLUR_SCORE_MOVE_BONUS_LEVEL level);
		void											EndScore();

		void											UpdateJumpStats();

		void											GiveInstantBonusScore(E_BLUR_SCORE_MOVE_BONUS bonus, E_BLUR_SCORE_MOVE_BONUS_LEVEL level, int about_vehicle);

		int												mTotalScore;
		int												mLastScore;

		int												mCurrentScore;
		E_BLUR_SCORE_MOVE_BASE							mCurrentScoreBase;
		E_BLUR_SCORE_MOVE_BASE_TYPE						mCurrentScoreBaseType;
		Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> 			mCurrentScoreSkills;
		Btl::Vector<E_BLUR_SCORE_MOVE_BONUS> 			mCurrentScoreBonus;

		int												mCurrentOtherVehicle;

		float											mScoreUITimer;

		bool											mHitWithPowerUp;
		int												mHitsInARow;
		int												mPreviousFiredPickupID;

		Btl::Vector<float, Btl::VectorParameters_Array<MAX_VEHICLES> >	mLastHitVehicleTimers;
		Btl::Vector<float, Btl::VectorParameters_Array<MAX_VEHICLES> >	mLastHitByVehicleTimers;
		
		ScoreEventHistory								mScoringHistory;
		
		int												mHitSameVehicleInARow;
		
		void											VictimBonusScore( BlurScoreMoveSummary& summary, U8 aggressorVehicleIndex= U8(-1) );

		int												mWrecksInARow;

		//vendetta
		int												mVendettaIndex;
		int												mVendettaCount;
		float											mVendettaTime;

		//drifting
		void											CheckDrifter(bool activate, E_BLUR_SCORE_MOVE_SKILL skill);
		float											mSlipAmount;
		float											mDriftingAngle;
		bool											mDrifting;
		float											mDriftDistance;
		Game::C_DrifterAbility							mDrifterAbility;

		//jumping
		U8												mNumWheelsOnGround;
		float											mJumpDistance;
		bool											mJumpedWithNitrous;

		bool											mUsingNitroAltType;

		Btl::Map< int, PowerUpFireState >				mPowerUpFireStates;

		//1st place
		bool											mGotFirstPlaceDominator1;
		bool											mGotFirstPlaceDominator2;

		Btl::Vector<bool, Btl::VectorParameters_Array<MAX_VEHICLES> > mGotPlace;

		bool											mInFirstPlace;
		float											mTimeInFirstPlace;

		//speed
		bool											mGotFirstSpeed;
		bool											mGotSecondSpeed;
		bool											mGotThirdSpeed;
		bool											mGotTopSpeed;

		//shielded
		U8												mShieldedFromVehicle;
		U32												mShieldedFromPerkUniqueID;
		float											mShieldedTime;

		//clean racing
		bool											mNitroDamageReceived;
		bool											mLapDamageReceived;
		bool											mNoRepairOrWreck;

		//fan demands
		int												mFanDemandFans;
		int												mFanDemandsInARow;

		int												mSurvivedBarrageFans;


		// Access to the multiplayer abilities of the vehicle that owns this component
		Game::C_LevelStateMultiplayerVehicleAbilities*  mpMultiplayerAbilities;

		bool											ShouldCalculateMotorMashScoring(U8 firingVehicle, U8 hitVehicle);
		bool											mIsMotorMash;

		float											mHOGTimer;

		float											mRivalDamageScoreMultiplier;
		void											HandleMotorMashScoreCalculation(Game::Actors::C_VehicleLogicActor* pLogicActor,bool wasRivalAttack, bool wasHighDamageAttack, int& baseScore, int& bonusScore); 
#ifndef CFG_GOLD
		void											UpdateMotormashDebugValues();
#endif // #ifndef CFG_GOLD

		int												mNumBarges;
		int												mFirstBargeIndex;
		bool											mBargeDrift;
		bool											mBargeNitrous;
		bool											mBargeAir;

		bool											mNitrousAir;
		int												mRepeatNitros;

		Btl::Map< U32, PowerUpBoltState >				mPowerUpBoltStates;

		int												mCombo;
		int												mComboScore;
		float											mComboTimer;

		// Rivals mode 
		Util::C_Random									mRandomGenerator;
		bool											mIsLocalGame; 
		Game::C_LevelState*								mpLevelState;

		void											SetRivalIndex(S8 rivalIndex);

		bool											IsVehicleValidRival( const U8 vehIndex ) const;
		S8												GetVehicleIndexAtRacePosition(const U8 vehicleRacePosition ) const;
		bool											OtherVehiclesStillInRace() const;
		void											BuildValidRivalVehicleIndices_Online(Btl::Vector<U8>* container);
		void											BuildValidRivalVehicleIndices_Local(Btl::Vector<U8>* container); 

		bool											mGivePositionandOvertake;

		S32												mCheckpointsPassed;
		S32												mCurrentRaceRuleArea;
		bool											mCheckpointsDamageRecieved;

		bool											mAttemptedFanRun;

		int												mNitrousOvertakes;
		F32												mTimeTillNextDrifterFX;

		Btl::Set<U32>									mShuntsDestroyed;

		int												mMaxLapReached;
		MultiplayerConfig*								m_pMpConfig;

		bool											mIsSPDestruction;

		void											CheckFreeFallingCliffTimer(const F32 frameTime);
		F32												mFreeFallingOverCliffTimer;

		bool											mHasDrowned;
		bool											mHasPlummeted;

		float											mNumBoltsPerSet;
	};
}

#endif //#ifndef VLC_NEW_SCORING_H
