#ifndef LEVEL_RESULT_H
#define LEVEL_RESULT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include "RaceStatistics.h"
#include "VehicleRaceStatistics.h"
#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

const int MAX_LEVELRESULT_CARS=20;


typedef struct LevelResultCar
{
	char  mPosition;
	int   mScore;
	U8	  mNetJoinId;

	Btl::Vector<int,Btl::VectorParameters_Array<AMAX_SCORING_EVENT_MAX> > mIndividualScoringStore;
	VehicleRaceStatistics mStatistics;

	Btl::Vector<bool> mPassedRule;
	Btl::Vector<bool> mFailedRule;
}LevelResultCar;

class C_LevelResult
{
public:
	C_LevelResult()
	{
		Clear();
	}

	void Clear();
	void Finished();

	bool mFake;
	Btl::Vector<LevelResultCar,Btl::VectorParameters_Array<MAX_LEVELRESULT_CARS> > mCars;
	int	mNumCars;
	RaceStatistics mRaceStatistics;

	U8 mFriendDemandComplete;


private:

};

} // namespace Game

#endif // LEVEL_RESULT_H
