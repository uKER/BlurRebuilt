#ifndef VEHICLE_SCORING_INFO_H
#define VEHICLE_SCORING_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\RaceLogicCore\Data\RLVehicleData.h>

struct VehicleScoringItem
{
	AMAX_SCORING_EVENT mBase;
	int mModifier;
};

struct VehicleScoringInfo
{
	bool					mAllFansGained;
	bool					mWreckedByAnotherPlayer;		   // Only valid for duration of wreck
	bool					mDisplayWreckedByAnotherPlayer;    // Doesn't get reset until next time we are wrecked. 
	
	int						mFinalScore;
	int						mFinalScoreForUI;
	
	float					mFlippedOutTimer;
	float					mCrashedOutTimer;
	float					mCollisionTimer;
	float					mDrownedTimer;
	float					mOutOfBoundsTimer;
	
	int						mRevengeIndex;
	float					mRevengeTimer;
	
	float					mWreckedTimer;

	int						mRivalIndex;	
	int						mPrevRivalIndex;
	float					mTimeInAir;

	int						mSkillShots;
	int						mLongestChain;

	int						mShowyFlourishFans;

	bool					mGoldenFanDemandComplete;
	int						mFanDemandsComplete;
	int						mGoldenFanDemandScore;

	int						mDestructionPoints;
	int						mDestructionWrecks;
	int						mDestructionBlueWrecks;
	int						mDestructionRedWrecks;
	int						mDestructionYellowWrecks;
	int						mDestructionGreenWrecks;

	int						mCombo;
	float					mComboTimer;

	float					mUsingNitroTimer;

	float					mValidDriftDistance;

	float					mCheckpointTimeLeft;

	//Moved here from VLC_Scoring so it can be used elsewhere
	U8						mMostRecentVehicleIdxHitBy;			
	U8						mMostRecentVehicleIdxWreckedBy; // This will ALWAYS be valid until we are next wrecked

	bool					mDestruction_WreckedEasy;
	bool					mDestruction_WreckedMedium;
	bool					mDestruction_WreckedHard;
	bool					mDestruction_WreckedVHard;
	bool					mDestruction_WreckedAll;
	bool					mDestruction_CloseCallWrecked;

	Btl::Vector<int,Btl::VectorParameters_Array<AMAX_SCORING_EVENT_MAX> > mIndividualScoring;
	Btl::Vector<VehicleScoringItem>	mScoringStore;

	VehicleScoringInfo()
	{
		Reset();	
	}

	void Reset()
	{
		mAllFansGained = false;

		mWreckedByAnotherPlayer = false;
		mDisplayWreckedByAnotherPlayer = false;

		mCombo = mFinalScore = mFinalScoreForUI = mRevengeIndex = mSkillShots = mLongestChain = mShowyFlourishFans = 0;

		mComboTimer=0.f;

		mGoldenFanDemandComplete = false;
		mFanDemandsComplete = 0;
		mGoldenFanDemandScore = 0;

		mDestructionPoints=0;
		mDestructionWrecks=0;
		mDestructionBlueWrecks=0;
		mDestructionRedWrecks=0;
		mDestructionYellowWrecks=0;
		mDestructionGreenWrecks=0;

		mCheckpointTimeLeft=0.f;
		mUsingNitroTimer=0.f; 

		mValidDriftDistance = 0.0f;

		mRivalIndex = -1; 

		mPrevRivalIndex = -1; 

		mFlippedOutTimer = mCrashedOutTimer = mCollisionTimer = mDrownedTimer = mRevengeTimer = mWreckedTimer = mTimeInAir = mOutOfBoundsTimer = 0.f;

		mMostRecentVehicleIdxHitBy = INVALID_VEHICLE_IDX;
		mMostRecentVehicleIdxWreckedBy = INVALID_VEHICLE_IDX;

		mDestruction_WreckedEasy=false;
		mDestruction_WreckedMedium=false;
		mDestruction_WreckedHard=false;
		mDestruction_WreckedVHard=false;
		mDestruction_WreckedAll=false;
		mDestruction_CloseCallWrecked=false;
	}
};

#endif //VEHICLE_SCORING_INFO_H
