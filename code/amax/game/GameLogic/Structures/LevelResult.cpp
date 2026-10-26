#include "stdafx.h"

#include <GameLogic/Structures/LevelResult.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

namespace Game
{

void C_LevelResult::Clear()
{
	mFake=false;
	mNumCars=0;

	//mFriendDemandComplete=false;
	//for(int a=0;a<MAX_LEVELRESULT_CARS;a++)
	//{
		//mCars[a].mPosition = INVALID_RACE_POSITION;
		//mCars[a].mScore = 0;
		//mCars[a].mPassedRule.Clear();
		//mCars[a].mFailedRule.Clear();
		//mCars[a].mStatistics.Clear();
	//}
	mRaceStatistics.Initialise();
}

void C_LevelResult::Finished()
{


}

} // namespace Game
