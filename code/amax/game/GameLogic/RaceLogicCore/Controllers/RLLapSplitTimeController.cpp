#include "stdafx.h"

#include "RLLapSplitTimeController.h"
#include "../Data/RLDefinitions.h"

#include "../Core/RaceLogicCore.h"

#ifdef RLC_DEBUG
#pragma optimize ("", off) //RLC_DEBUG
#endif

static Debug::Logger &g_RLLapSplitTimeLog = Debug::GetLogger("Amax.RLC.RLLapSplitTimeLog");
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern Debug::Bool g_Stamp;

Debug::Bool g_OutputLapInformation("RLC.LapSplitTimeController.OutputLapTimeInformation", false);
Debug::Bool g_OutputSplitInformation("RLC.LapSplitTimeController.OutputSplitTimeInformation", false);
Debug::Bool g_IncludeLapStoring("RLC.LapSplitTimeController.IncludeLapStoring", false);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



namespace Game
{
namespace Actors
{
RLLapSplitTimeController::RLLapSplitTimeController()
{
	Initialise();
}

RLLapSplitTimeController::~RLLapSplitTimeController()
{
}

void RLLapSplitTimeController::Initialise()
{
	meType = E_RLCOMPONENTTYPE_LAPSPLITTIME_CONTROLLER;
}

void RLLapSplitTimeController::Update()
{
	PERF_EVENT("RLLapSplitTimeController::Update");

	//Go through all the vehicles
	for(int vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)	
	{
		UpdateVehicle(vehicleIndex);
	}
}
void RLLapSplitTimeController::UpdateVehicle(const U8 vehicleIndex)
{
	PERF_EVENT("RLLapSplitTimeController::UpdateVehicle");

	//Get the vehicle's data for this frame and last
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];

	//Check if we've crossed the lap end
	if(pVehicle->mMaximumLap > pLastVehicle->mMaximumLap)
		VehicleCompletedLap(vehicleIndex);
	else if(pVehicle->mMaximumSplit > pLastVehicle->mMaximumSplit)
		VehicleCompletedSplit(vehicleIndex);

	//Update the lap and split times
	UpdateVehicleLapTime(vehicleIndex);

	//Update the lap end and split display timers
	UpdateVehicleTimers(vehicleIndex);

#ifndef CFG_GOLD
	if(g_Stamp)
	{
		LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::UpdateVehicle() > Veh: %d pos: %d DFS: %d current split (%d) %3.02fs, lap (%d) %3.02fs and race %3.02fs", 
					vehicleIndex, 
					pVehicle->mRacePosition,
					(int)pVehicle->mDistanceFromStartLine,
					pVehicle->mCurrentLapData.mCurrentSplit.mSplitIndex, 
					pVehicle->mCurrentLapData.mCurrentSplit.mSplitTime, 
					pVehicle->mCurrentLapData.mLapNo, 
					pVehicle->mCurrentLapData.mLapTime, 
					pVehicle->mCurrentLapData.mRaceTime);
	}
#endif
}

void RLLapSplitTimeController::UpdateVehicleLapTime(const U8 vehicleIndex)
{
	//We know that this vehicle has completed a lap
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	float frameTime = m_pRLC->GetFrameTime();

	//Now we can update the lap time
	pVehicle->mCurrentLapData.mLapTime += frameTime;
	pVehicle->mCurrentLapData.mRaceTime = m_pRLC->GetTimeData()->mRaceTime;

	pVehicle->mCurrentLapData.mCurrentSplit.mLapTime = pVehicle->mCurrentLapData.mLapTime;
	pVehicle->mCurrentLapData.mCurrentSplit.mSplitTime += frameTime;
	pVehicle->mCurrentLapData.mCurrentSplit.mRaceTime = m_pRLC->GetTimeData()->mRaceTime;
}

void RLLapSplitTimeController::UpdateVehicleTimers(const U8 vehicleIndex)
{
	//We know that this vehicle has completed a lap
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];

	float frameTime = m_pRLC->GetFrameTime();

	//If we're displaying the lap end information update the time
	if( (pVehicle->mDisplayLapEndTimer - frameTime) < 0.f)
	{
		pVehicle->mDisplayLapEndTimer = 0.f;
	}
	else
		pVehicle->mDisplayLapEndTimer -= frameTime;

	//If we're displaying the split end information update the time
	if( (pVehicle->mDisplaySplitTimer - frameTime) < 0.f)
	{
		pVehicle->mDisplaySplitTimer = 0.f;
	}
	else
		pVehicle->mDisplaySplitTimer -= frameTime;

	if(pVehicle->mDisplaySplitTimer == 0.f && pVehicle->mDisplayLapEndTimer == 0.f)
		pVehicle->mSplitDifference = 0.f;

}

void RLLapSplitTimeController::VehicleCompletedLap(const U8 vehicleIndex)
{
	//We know that this vehicle has completed a lap
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//We only want to start displaying lap end information at the end of lap 1
	if(pVehicle->mMaximumLap > 1)
	{
		pVehicle->mDisplayLapEndTimer = LAP_END_DISPLAY_TIME;
		pVehicle->mSplitDifference = pVehicle->mCurrentLapData.mLapTime - pVehicle->mFastestLapData.mLapTime;

		//See if this is the fastest lap
		CheckFastestLap(vehicleIndex);

		//See if this is the slowest lap
		CheckSlowestLap(vehicleIndex);

		//Collect and store the final information for the split
		FinaliseSplit(vehicleIndex);

		pVehicle->mLastLapData = pVehicle->mCurrentLapData;

		StoreLap(vehicleIndex, pVehicle->mCurrentLapData);
	}
	else
	{
		pVehicle->mVehicleRaceStarted = true;
	}

	pVehicle->mCurrentLapData.Initialise();
	pVehicle->mCurrentLapData.mLapNo = pVehicle->mMaximumLap;

	/* TODO
	//Tell the world we've completed a lap
	GameUITimeMessage  tmsg1(vehicleIndex,pVehicle->mLastLapData.mRaceTime, pVehicle->mLastLapData.mLapTime, pVehicle->mCurrentLapData.mLapNo);
	m_pRLC->SendMessage(GAMEUI_MESSAGE_TIME_LAP,&tmsg1);
	*/

	if(g_OutputLapInformation)
		LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::VehicleCompletedLap() > Veh: %d completed lap %d in %3.02fs", (int)vehicleIndex, pVehicle->mLastLapData.mLapNo, pVehicle->mLastLapData.mLapTime, pVehicle->mLastLapData.mRaceTime);

	//Initialise the next split
	InitialiseCurrentSplit(vehicleIndex);
}

void RLLapSplitTimeController::VehicleCompletedSplit(const U8 vehicleIndex)
{
	//We know that this vehicle has completed a lap
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	//Get the current split information
	SplitTimeData currentSplit = pVehicle->mCurrentLapData.mCurrentSplit;
	
	//Make sure we're not still on our first lap
	if(pVehicle->mFastestLapData.mLapNo > -1)
	{
		//We actually want to find the index of the split the vehicle last crossed correctly not the current previous split
		int lastSplit = pVehicle->mLastSplitIndex;	
		if(pVehicle->mFastestLapData.mSplitTimeData.Size() > (lastSplit -1))
			pVehicle->mSplitDifference = pVehicle->mCurrentLapData.mCurrentSplit.mLapTime - pVehicle->mFastestLapData.mSplitTimeData[lastSplit-1].mLapTime;
		else 
			pVehicle->mSplitDifference = 0.f;
	}
		
	pVehicle->mDisplaySplitTimer = SPLIT_DISPLAY_TIME;

	//Collect and store the final information for the split
	FinaliseSplit(vehicleIndex);

	//Initialise the next split
	InitialiseCurrentSplit(vehicleIndex);
}

//Collect and store the final information for the split
void RLLapSplitTimeController::FinaliseSplit(const U8 vehicleIndex)
{
	//We know that this vehicle has completed a lap
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	//Get the current split information
	SplitTimeData currentSplit = pVehicle->mCurrentLapData.mCurrentSplit;

	//Finalise the data collection for the current split
	pVehicle->mCurrentLapData.mCurrentSplit.mRaceTime = m_pRLC->GetTimeData()->mRaceTime;
	pVehicle->mCurrentLapData.mCurrentSplit.mRacePosition = pVehicle->mRacePosition;

	//Store the split we've just finished
	pVehicle->mCurrentLapData.mSplitTimeData.PushBack(currentSplit);

	/* TODO Get this stuff in
	if(g_OutputSplitInformation)
	{
		LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::FinaliseSplit() > Veh: %d in pos %d split %d (%d) in %3.02fs, lap (%d) %3.02fs and race %3.02fs", 
					vehicleIndex, 
					pVehicle->mCurrentLapData.mCurrentSplit.mRacePosition, 
					pVehicle->mCurrentLapData.mCurrentSplit.mSplitIndex, 
					pVehicle->mCurrentLapData.mCurrentSplit.mRaceSplitNumber, 
					pVehicle->mCurrentLapData.mCurrentSplit.mSplitTime, 
					pVehicle->mCurrentLapData.mCurrentSplit.mLapNo, 
					pVehicle->mCurrentLapData.mCurrentSplit.mLapTime, 
					pVehicle->mCurrentLapData.mCurrentSplit.mRaceTime);

		if(m_pRLC->VehicleValid(pVehicle->mVehicleIndexAhead))
			LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::FinaliseSplit() > Veh: %d dist to veh: %d (ahead) %3.02fwp, time %3.02fs, real %3.02fm", 
						vehicleIndex, 
						pVehicle->mVehicleIndexAhead, 
						pVehicle->mDistanceToVehicleAhead, 
						pVehicle->mTimeToVehicleAhead, 
						pVehicle->mRealDistanceToVehicleAhead); 


		if(m_pRLC->VehicleValid(pVehicle->mVehicleIndexBehind))
			LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::FinaliseSplit() > Veh: %d dist to veh: %d (behind) %3.02fwp, time %3.02fs, real %3.02fm", 
						vehicleIndex, 
						pVehicle->mVehicleIndexBehind, 
						pVehicle->mDistanceToVehicleBehind, 
						pVehicle->mTimeToVehicleBehind, 
						pVehicle->mRealDistanceToVehicleBehind);

	}
	*/
}

void RLLapSplitTimeController::InitialiseCurrentSplit(const U8 vehicleIndex)
{
	//We know that this vehicle has completed a lap
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//Blank the current split information and start again
	pVehicle->mCurrentLapData.mCurrentSplit.Initialise();

	pVehicle->mCurrentLapData.mCurrentSplit.mLapNo = pVehicle->mNoLaps;
	pVehicle->mCurrentLapData.mCurrentSplit.mSplitIndex = pVehicle->mLastSplitIndex;
	pVehicle->mCurrentLapData.mCurrentSplit.mRaceSplitNumber = pVehicle->mMaximumSplit;
}

void RLLapSplitTimeController::CheckFastestLap(const U8 vehicleIndex)
{
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//See if we've stored a fastest lap so far
	if(pVehicle->mFastestLapData.mLapNo == -1										//None stored so far
		||
	   pVehicle->mFastestLapData.mLapTime > pVehicle->mCurrentLapData.mLapTime)		//Current lap faster than previous record
	{
		pVehicle->mFastestLapData = pVehicle->mCurrentLapData;
	}
}

void RLLapSplitTimeController::CheckSlowestLap(const U8 vehicleIndex)
{
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//See if we've stored a fastest lap so far
	if(pVehicle->mSlowestLapData.mLapNo == -1									//None stored so far
		||
	   pVehicle->mSlowestLapData.mLapTime < pVehicle->mCurrentLapData.mLapTime)		//Current lap faster than previous record
	{
		pVehicle->mSlowestLapData = pVehicle->mCurrentLapData;
	}
}

//We will store the most recent x laps worth of data, this will allow us to do things like 
//the old Career Target Table better and (hopefully) soooo much easier!
void RLLapSplitTimeController::StoreLap(const U8 vehicleIndex, LapTimeData newLap)
{
	//We want to make sure that the most recent lap is in ordinal 0 and the most distant lap is in last place
	LapTimeArray newLapTimeArray;
	newLapTimeArray.PushBack(newLap);

	if(g_OutputLapInformation && g_IncludeLapStoring)
		LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::StoreLap() > Veh: %d storing at index 0 lap %d which took %3.02fs", vehicleIndex, newLap.mLapNo, newLap.mLapTime);

	//Get the vehicle
	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//Go through the last set of laps and copy them into the new arrays
	for(int lapIndex = 0; lapIndex < pVehicle->mLapTimeArray.Size(); lapIndex++)
	{
		if(newLapTimeArray.Size() >= newLapTimeArray.Capacity())
			break;

		newLapTimeArray.PushBack( pVehicle->mLapTimeArray[lapIndex] );

		if(g_OutputLapInformation && g_IncludeLapStoring)
			LOGDEBUG(g_RLLapSplitTimeLog, "RLLapSplitTimeController::StoreLap() > Veh: %d storing at index %d lap %d which took %3.02fs", vehicleIndex, lapIndex+1, pVehicle->mLapTimeArray[lapIndex].mLapNo, pVehicle->mLapTimeArray[lapIndex].mLapTime);
	}

	pVehicle->mLapTimeArray = newLapTimeArray;
}



};//Actors
};//Game

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
