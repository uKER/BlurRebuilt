#ifndef RACE_STATISTICS_H
#define RACE_STATISTICS_H

#ifdef _MSC_VER
#pragma once
#endif

//If you add or want to add a statistic to track then you'll need to make sure
//the particular statistic is tracked by the RaceLogicCore's RLStatisticsController...
//to do this you could either ask Charlie A to make sure it does or do it yourself.
//Don't break anything though!
struct RaceStatistics
{
//Functions
public:
	void Initialise()
	{
		mMaxCarsInCrash = 0;
		mTotalRaceTime = 0.f;
		mRaceQuit = mRaceCheated = false;

		mMedal = 0;
		mFansEarned = 0;
		mFanDemandsComplete = 0;
		mGoldenFanDemandComplete = false;

		mGoldenFanDemandScore = 0;

		mDestruction_NumPoints = 0;
		mDestruction_TotalCarsWrecked=0;
		mDestruction_EasyCarsWrecked=0;
		mDestruction_MediumCarsWrecked=0;
		mDestruction_HardCarsWrecked=0;
		mDestruction_VHardCarsWrecked=0;
	}

//Variables
public:
	int											mMaxCarsInCrash;
	float										mTotalRaceTime;
	bool										mRaceQuit,						//Quit out
												mRaceCheated;					//Used Quit and Win
	
	S32											mMedal;
	S32											mFansEarned;
	S32											mFanDemandsComplete;
	bool										mGoldenFanDemandComplete;
	S32											mGoldenFanDemandScore;

	S32											mDestruction_NumPoints;
	S32											mDestruction_TotalCarsWrecked;
	S32											mDestruction_EasyCarsWrecked;
	S32											mDestruction_MediumCarsWrecked;
	S32											mDestruction_HardCarsWrecked;
	S32											mDestruction_VHardCarsWrecked;
};


#endif //RACE_STATISTICS_H
