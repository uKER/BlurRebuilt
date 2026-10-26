#include "stdafx.h"

#include "MiscellaneousChallengeBlackboardData.h"
#include <system\time\Time.h>

namespace Game
{
void MiscellaneousChallengeBlackboardData::Initialise()
{
	//This is only used at the end of the race for a few frames whilst we await confirmation of race results 
	//so we don't need to serialise this

	// One off initialisations
	mDailyChallengeDayStamp = 0;
	mDailyChallengeTimeZoneAdjustStamp = 0;
	mCurrentTimeTravelingAtXMPH		= 0.0f;
	mLongestTimeTravelingAtXMPH		= 0.0f;
	mCurrentTimeInFirstPosition		= 0.0f;
	mSecondsRemainderInFirstPosition= 0.0f;
	mPreviouslyInFirstPosition		= false;

	// The usual suspects (re initialised every race)
	InitForNextRace(); 

	mCountriesVisitedTodayBitField = 0;
}

void MiscellaneousChallengeBlackboardData::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );

	out << mDailyChallengeDayStamp << mSecondsRemainderInFirstPosition << mVehiclesUsedToday << mCountriesVisitedTodayBitField << mDailyChallengeTimeZoneAdjustStamp;
}

void MiscellaneousChallengeBlackboardData::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );

	in >> mDailyChallengeDayStamp >> mSecondsRemainderInFirstPosition >> mVehiclesUsedToday >> mCountriesVisitedTodayBitField >> mDailyChallengeTimeZoneAdjustStamp;
}

void MiscellaneousChallengeBlackboardData::InitForNextRace()
{
	mNumCarsTakenOutThisRace = 0;
	mNumCarsInWreckStreak = 0;
	mNumRevengeAttacksThisRace = 0;
	mNumCarsCrashedOutInChain = 0;
	mLastVehicleIndexHitByPerk = -1;
	mNumSuccessivePerkHitsOnLastVehicleHit = 0; 
	mCurrentComboNumber = 0;
	mHasRammedVehicle = 0;

	mPerksUsed.ClearBits();
	
	mOvertakes = 0;
	mNitrousOvertakes = 0;

	ClearVehiclesWreckedInCurrentRace();

	mQuickDrawStartRaceTime = 0.f;
	mQuickDrawNumberOfVehiclesWrecked = 0;

	mTimeSpentInAir = 0.0f;
	mUltimateComebackStatus = E_UltimateComeback_NotYetInFirstPlace;
	mSkillshotsPerformed = 0;
	mSuccessfulPowerupHits = 0;
	mNumVehiclesHitByOneBarge = 0;
	mForwardPickupFireUsed = false; 
	mReversePickupFireUsed = false; 

	mLastGaspIsActive = false;
	mNumCarsHitWithLastGasp = 0;

	if(!mRecentlyFiredMineIDs.Empty())
	{
		mRecentlyFiredMineIDs.Clear();
	}

	mNumNitroJumps = 0;
	mNumVehiclesHitWithLastGaspThisLife = 0; 
}

void MiscellaneousChallengeBlackboardData::ClearVehiclesWreckedInCurrentRace()
{
	for(U16 i = 0; i < RacerNet::NET_MAX_VEHICLES; i++)
	{
		mTimesPlayerWrecked[i] = 0;
	}
}

}; //namespace Game
