#ifndef VEHICLE_RACE_STATISTICS_H
#define VEHICLE_RACE_STATISTICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

struct DualVehicleRaceStatistics
{
	void Clear()
	{
		mFinishedVehicleDistances = 0.f;

		mFlippedOutByAggressivePerk = mStoppedAttackWithShield = mTookOutDriver = false;

		mNumberOfShocksAgainstDriver = mNumberOfShuntsAgainstDriver = mNumberOfTimesHitWithAggressivePerk = mDamageCausedToDriver = 
		mNumberOfBargesAgainstDriver = mNumTimesOvertookWithNitro = mNumberOfFlashesAgainstDriver = mNumberOfBombsAgainstDriver = 0;

		mNumberOfDoubleShocksAgainstDriver = mNumberOfDoubleShuntsAgainstDriver = mNumberOfTimesHitWithDoubleAggressivePerk = 
		mNumberOfDoubleBargesAgainstDriver = mNumTimesOvertookWithDoubleNitro = mNumberOfDoubleFlashesAgainstDriver = mNumberOfDoubleBombsAgainstDriver = 0;

		mDamageCausedToDriver = mNumTimesHit = mNumTimesHitBy = 0;
	}

	int		mNumberOfShuntsAgainstDriver, mNumberOfShocksAgainstDriver, mNumberOfTimesHitWithAggressivePerk, 
			mNumberOfBargesAgainstDriver, mNumberOfFlashesAgainstDriver, mNumberOfBombsAgainstDriver, mNumTimesOvertookWithNitro,

			mNumberOfDoubleShuntsAgainstDriver, mNumberOfDoubleShocksAgainstDriver, mNumberOfTimesHitWithDoubleAggressivePerk, 
			mNumberOfDoubleBargesAgainstDriver, mNumberOfDoubleFlashesAgainstDriver, mNumberOfDoubleBombsAgainstDriver, mNumTimesOvertookWithDoubleNitro,
			
			mDamageCausedToDriver, mNumTimesHit, mNumTimesHitBy;

	bool	mTookOutDriver, mStoppedAttackWithShield, mFlippedOutByAggressivePerk;

	float	mFinishedVehicleDistances;
};

struct VehicleRaceStatistics
{
	void operator =(VehicleRaceStatistics stats)
	{
		mTotalRaceTime						= stats.mTotalRaceTime;
		mFastestSpeedInMPH					= stats.mFastestSpeedInMPH;
		mLongestDrift						= stats.mLongestDrift;
		mNumDrifts							= stats.mNumDrifts;
		mLongestJump						= stats.mLongestJump;
		mDamageTakenPercent					= stats.mDamageTakenPercent;
		mMaxDamageTakenPercent				= stats.mMaxDamageTakenPercent;
		mSlowestLapTime						= stats.mSlowestLapTime;
		mFastestLapTime						= stats.mFastestLapTime;
		mEndOfRaceHealth					= stats.mEndOfRaceHealth;
		mDistanceTravelled					= stats.mDistanceTravelled;
		mLastWreckTime						= stats.mLastWreckTime;
		mLongestTimeWithoutWreck			= stats.mLongestTimeWithoutWreck;

		mMaxPlacesTakenInASingleManoeuvre	= stats.mMaxPlacesTakenInASingleManoeuvre;
		mMajorCrashLap						= stats.mMajorCrashLap;
		mMaxCarsInWorstCrash				= stats.mMaxCarsInWorstCrash;
		mTimesCrashed						= stats.mTimesCrashed;
		mMaxDriversInSingleShock			= stats.mMaxDriversInSingleShock;
		mDamageRepaired						= stats.mDamageRepaired;
		mDamageDealt						= stats.mDamageDealt;
		mDamageTaken						= stats.mDamageTaken;
		mNoRepairsUsed						= stats.mNoRepairsUsed;

		mTookFirstPlaceAtEndOfRace			= stats.mTookFirstPlaceAtEndOfRace;
		mLostFirstPlaceAtEndOfRace			= stats.mLostFirstPlaceAtEndOfRace;
		mOvertookAtEndOfRace				= stats.mOvertookAtEndOfRace;
		mMajorCrash							= stats.mMajorCrash;
		mWrecked							= stats.mWrecked;
		mFinished							= stats.mFinished;
		mFanPickupsCollected				= stats.mFanPickupsCollected;

		mNumTimesShield						= stats.mNumTimesShield;
		mNumTimesShielded					= stats.mNumTimesShielded;

		mNumTimesRepaired					= stats.mNumTimesRepaired;

		mNumVehiclesWrecked					= stats.mNumVehiclesWrecked;
		mNumRivalVehiclesWrecked			= stats.mNumRivalVehiclesWrecked;
		mWreckStreak						= stats.mWreckStreak;
		mBestWreckStreak					= stats.mBestWreckStreak;
		mLosingStreak						= stats.mLosingStreak;
		mWorstLosingStreak					= stats.mWorstLosingStreak;

		mNumTimesSuicided					= stats.mNumTimesSuicided;
		mNumTimesWrecked					= stats.mNumTimesWrecked;

		mNumberOfSkillShotsDrifting			= stats.mNumberOfSkillShotsDrifting;
		mNumberOfSkillShotsAir				= stats.mNumberOfSkillShotsAir;

		mLongestChain						= stats.mLongestChain;

		mNumLowDamageAttacks				= stats.mNumLowDamageAttacks;
		mNumHighDamageAttacks				= stats.mNumHighDamageAttacks;
		mNumRivalLowDamageAttacks			= stats.mNumRivalLowDamageAttacks;
		mNumRivalHighDamageAttacks			= stats.mNumRivalHighDamageAttacks;
		mCarsRammed							= stats.mCarsRammed;

		mDistToFirstOnFinish				= stats.mDistToFirstOnFinish;
		mNumTimesHit						= stats.mNumTimesHit;
		mNumTimesDrowned					= stats.mNumTimesDrowned;
		mNumTimesOffEdge					= stats.mNumTimesOffEdge;
		mPickupsEvaded						= stats.mPickupsEvaded;

		mTotalPickupsCollected				= stats.mTotalPickupsCollected;
		
		mShowyFlourishFans					= stats.mShowyFlourishFans;
		mStopwatchPickupsCollected			= stats.mStopwatchPickupsCollected;
		mNitrousPickupsCollected			= stats.mNitrousPickupsCollected;

		mMinesFragmentedWithMastermine		= stats.mMinesFragmentedWithMastermine;
		mPlayersScrambled					= stats.mPlayersScrambled;					  
		mCarsBatteringRammed				= stats.mCarsBatteringRammed;			
		mTimesProtectedByStableFrame		= stats.mTimesProtectedByStableFrame;	
		mPlayersHitWithIronFist				= stats.mPlayersHitWithIronFist;
		mDecoysDropped						= stats.mDecoysDropped;	
		mPlayersHitwithDecoys				= stats.mPlayersHitwithDecoys;	
		mPowerupsBlocked					= stats.mPowerupsBlocked;
		mPowerupsAbsorbedBySafetyNet		= stats.mPowerupsAbsorbedBySafetyNet;
		mNitroOvertakes						= stats.mNitroOvertakes;
		mNumPowerupsDropped				    = stats.mNumPowerupsDropped;

		mTimeStealthed						= stats.mTimeStealthed;
		mTimeInFirstPlace					= stats.mTimeInFirstPlace;
		mLongestTimeNotDamaged				= stats.mLongestTimeNotDamaged;

		mCurrentDistanceTraveledInJump		= stats.mCurrentDistanceTraveledInJump;
		mLongestDistanceTraveledInJump		= stats.mLongestDistanceTraveledInJump;

		mCriticalObjectsHit.Clear();
		mNumPerksFired.Clear();
		mNumPerksHit.Clear();

		for( Btl::Set<S32>::Iterator it = stats.mCriticalObjectsHit.Begin(); it != stats.mCriticalObjectsHit.End(); it++ )
			mCriticalObjectsHit.Insert(*it);

		for( int i = 0; i < stats.mNumPerksFired.Size(); i++ )
			mNumPerksFired.PushBack(stats.mNumPerksFired[i]);

		for( int i = 0; i < stats.mNumPerksHit.Size(); i++ )
			mNumPerksHit.PushBack(stats.mNumPerksHit[i]);

		for( int i = 0; i < stats.mNumHoGTimes.Size(); i++ )
			mNumHoGTimes.PushBack(stats.mNumHoGTimes[i]);

		for( int i = 0; i < stats.mDualVehicleStats.Size(); i++ )
			mDualVehicleStats[i] = stats.mDualVehicleStats[i];
	}

	void Clear()
	{
		mTotalRaceTime = mFastestSpeedInMPH = mLongestDrift = mLongestJump = mDamageTakenPercent = mMaxDamageTakenPercent = 
		mSlowestLapTime = mFastestLapTime = mEndOfRaceHealth = mDistanceTravelled = mDistToFirstOnFinish = mLastWreckTime = mLongestTimeWithoutWreck = 0.f;

		mMaxPlacesTakenInASingleManoeuvre = mMajorCrashLap = mDamageRepaired = mDamageDealt = mDamageTaken = mFanPickupsCollected = mNumDrifts = mNumberOfSkillShotsDrifting = 
		mNumberOfSkillShotsAir = mNumTimesShield = mNumTimesShielded = mNumTimesRepaired = mNumVehiclesWrecked = mNumRivalVehiclesWrecked = mWreckStreak = mBestWreckStreak = mLosingStreak = mWorstLosingStreak = 
		mNumTimesSuicided = mNumTimesWrecked = mNumTimesHit = mNumTimesDrowned = mNumTimesOffEdge = mPickupsEvaded = mTotalPickupsCollected = mCarsRammed = mTimesCrashed =
		mLongestChain = mNumLowDamageAttacks = mNumHighDamageAttacks = mShowyFlourishFans = mStopwatchPickupsCollected = mNumRivalLowDamageAttacks = mNumRivalHighDamageAttacks = mNitrousPickupsCollected = 0;

		mTookFirstPlaceAtEndOfRace = mLostFirstPlaceAtEndOfRace = mOvertookAtEndOfRace = mMajorCrash = mWrecked = mFinished = false;

		mCriticalObjectsHit.Clear();
		mNumPerksFired.Clear();
		mNumPerksHit.Clear();

		mMinesFragmentedWithMastermine = 0; 
		mPlayersScrambled			   = 0;
		mCarsBatteringRammed		   = 0;
		mTimesProtectedByStableFrame   = 0;
		mPlayersHitWithIronFist		   = 0;
		mDecoysDropped				   = 0;
		mPlayersHitwithDecoys		   = 0;
		mPowerupsBlocked			   = 0;
		mPowerupsAbsorbedBySafetyNet   = 0;
		mNitroOvertakes				   = 0;
		mNumPowerupsDropped			   = 0;
		mTimeStealthed				   = 0.0f;
     	mTimeInFirstPlace			   = 0.0f;
		mLongestTimeNotDamaged		   = 0.0f;
		mCurrentDistanceTraveledInJump = 0.0f;
		mLongestDistanceTraveledInJump = 0.0f;

		for( int i = 0; i < E_PICKUP_TYPE_MAX; i++ )
		{
			mNumPerksFired.PushBack(0);
			mNumPerksHit.PushBack(0);
		}

		for( int i = 0; i < E_HandOfGodResetType_MAX; i++ )
		{
			mNumHoGTimes.PushBack(0);
		}

		for( int i = 0; i < mDualVehicleStats.Size(); i++ )
		{
			mDualVehicleStats[i].Clear();
		}
	}

	float	mTotalRaceTime, mFastestSpeedInMPH, mLongestDrift, mLongestJump, mDamageTakenPercent, 
			mMaxDamageTakenPercent, mSlowestLapTime, mFastestLapTime, mEndOfRaceHealth, mDistanceTravelled, mDistToFirstOnFinish, mLastWreckTime, mLongestTimeWithoutWreck;
	int		mMaxPlacesTakenInASingleManoeuvre, mMajorCrashLap, mMaxCarsInWorstCrash, mMaxDriversInSingleShock, mNumberOfSkillShotsDrifting, mNumberOfSkillShotsAir,
			mDamageRepaired, mDamageDealt, mDamageTaken, mNoRepairsUsed, mFanPickupsCollected, mNumDrifts, mNumTimesShield, mNumTimesShielded, mLongestChain, mNumTimesRepaired, 
			mNumVehiclesWrecked, mNumRivalVehiclesWrecked,mWreckStreak, mBestWreckStreak, mLosingStreak, mWorstLosingStreak, mNumTimesSuicided, mNumTimesWrecked, mNumTimesHit, mNumTimesDrowned,
			mNumTimesOffEdge, mPickupsEvaded, mTotalPickupsCollected, mCarsRammed, mTimesCrashed, mNumLowDamageAttacks, mNumHighDamageAttacks,
			mNumRivalLowDamageAttacks, mNumRivalHighDamageAttacks, mShowyFlourishFans, mStopwatchPickupsCollected, mNitrousPickupsCollected;

	int		mMinesFragmentedWithMastermine;
	int		mPlayersScrambled;
	int		mCarsBatteringRammed;
	int		mTimesProtectedByStableFrame;
	int		mPlayersHitWithIronFist;
	int		mDecoysDropped;
	int		mPlayersHitwithDecoys;
	int	    mPowerupsBlocked;
	int     mPowerupsAbsorbedBySafetyNet;
	int		mNitroOvertakes;
	int		mNumPowerupsDropped;
	float   mTimeStealthed;
	float   mTimeInFirstPlace;
	float	mLongestTimeNotDamaged;
	float	mCurrentDistanceTraveledInJump;
	float	mLongestDistanceTraveledInJump;

	bool	mTookFirstPlaceAtEndOfRace, mLostFirstPlaceAtEndOfRace, mOvertookAtEndOfRace, mMajorCrash, mWrecked, mFinished;

	Btl::Set<S32>		mCriticalObjectsHit;
	Btl::Vector<int>	mNumPerksFired;
	Btl::Vector<int>	mNumPerksHit;
	Btl::Vector<int>	mNumHoGTimes;

	Btl::Vector<DualVehicleRaceStatistics,Btl::VectorParameters_Array<MAX_VEHICLES> >   mDualVehicleStats;

	// We won't record these, they are just useful temporarily.
	bool	mIsDriftingWithScore;
};

#endif //VEHICLE_RACE_STATISTICS_H
