#ifndef RLTIMEDATA_H
#define RLTIMEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
namespace Actors
{
//Everything internal to the RLC should be public
class RLTimeData
{
public:	
	void										Initialise()
	{
		mRaceTime = mEventTime = mRacePositionCalculationTimer = 0.f;
		mPaused = false;
	}

public:
	
	float										mRaceTime,				//Time since the race started 
												mEventTime,				//Time since the event finished loading...

	//We don't necessarily want to calculate the vehicles' race position every frame as it would tend to flutter around
	//but we also want to be able to force calculation the position when significant things happen ie splits, laps and 
	//the end of the race...
												mRacePositionCalculationTimer;

	bool										mPaused;
};

};//namespace Actors
};//namespace Game

#endif //RLTIMEDATA_H
