#include "stdafx.h"

#include "RLTrackPositionController.h"
#include "../Core/RaceLogicCore.h"
#include "../Data/RLDefinitions.h"

#include "RLDebugLineController.h"
#include "../Database/RLDatabase.h"


#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <RacingPhysics/RacingPhysics.h>
#include <Vehicle/VehicleModel.h>
#include <Game/GameWorld/GameWorld.h>

#include <GameApp.h>
#include "../../../Game/Game.h"

#include <AI/AI_Amax.h>
#include <GameLogic/Messaging/GameLogicMessages.h>

#ifdef RLC_DEBUG
#pragma optimize ("", off) //RLC_DEBUG
#endif

static Debug::Logger &g_RLTrackPositionLog = Debug::GetLogger("Amax.RLC.RLTrackPositionLog");
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern Debug::Bool g_Stamp;

Debug::Int g_NoWaypointsInSFBlackSpot("RLC.TrackPosition.SFBlackSpotSize", 1, Debug::Range(1, 100), Debug::Step(1));
Debug::Float g_MaxWrongWayDistance("RLC.TrackPosition.MaxWrongWayDistance", 50.f);

Debug::Bool g_UseOldFashionedSplitLineAnalysis("RLC.TrackPosition.UseOldFashionedSplitLineAnalysis", false);
Debug::Bool g_AttemptToReorderSplitLines("RLC.TrackPosition.AttemptToReorderSplitLines", false);
Debug::Bool g_ReAnalyseSplits("RLC.TrackPosition.ReAnalyseSplits", false);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace Game
{

namespace Actors
{
RLTrackPositionController::RLTrackPositionController()
{
	Initialise();
}

RLTrackPositionController::~RLTrackPositionController()
{
}

void RLTrackPositionController::Initialise()
{
	meType = E_RLCOMPONENTTYPE_TRACKPOSITION_CONTROLLER;
}

void RLTrackPositionController::AnalyseSpline() 
{
	RLTrackData *pRLTrackData = m_pRLC->GetTrackData();

	pRLTrackData->mNumWaypoints = -1;
	pRLTrackData->mWaypointSize = -1.f;
	pRLTrackData->mTrackLength = -1;

	pRLTrackData->mAIDataAnalysed = false;

	//Check the AI data's initialised 
	if(pRLTrackData->mAIDataReceived)
	{
		//Now we need to start getting the gubins 
		airacingtech::ITrackDataAI *pTrackData = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();

		//Holds all the routes
		airacingtech::AITrackData *pAITrackData = pTrackData->GetTrackData();

		airacingtech::AITrackDataRoutes* pLogicRoute = pAITrackData->m_TrackLogicData.m_pLogicRoute;
		//
		pRLTrackData->m_pLogicRoute = pLogicRoute;
		
		//Work out how long the route is by working out the number of waypoints times the distance between the waypoints
		pRLTrackData->mNumWaypoints = pLogicRoute->GetNumSegments();
		pRLTrackData->mWaypointSize = pLogicRoute->GetStepSize();

		pRLTrackData->mTrackLength = (int)(pRLTrackData->mNumWaypoints * pRLTrackData->mWaypointSize);

		//Tell the world we've got AI data
		pRLTrackData->mAIDataAnalysed = true;
	}
}

void RLTrackPositionController::AnalyseSplits()
{

	RLTrackData *pRLTrackData = m_pRLC->GetTrackData();
	pRLTrackData->mSplitsAnalysed = false;

	if(pRLTrackData->mSplitsReceived)
	{
		//Analyse splits
		AnalyseActorStudioSplits();
	}

	if(m_pRLC->IsSplitDataAnalysed())
	{
		//Place the splits in the right order
		if(g_AttemptToReorderSplitLines)
			CalculateSplitOrder();

		//Get the start finish waypoint and position from our split data
		m_pRLC->GetTrackData()->mStartFinishPosition = m_pRLC->GetTrackData()->mSplitData[0].mCentre;
		m_pRLC->GetTrackData()->mStartFinishAccurateWaypoint = m_pRLC->GetTrackData()->mSplitData[0].mAccurateWaypoint;
	}
}

void RLTrackPositionController::AnalyseActorStudioSplits()
{
	if(!m_pRLC->GetTrackData()->mSplitsReceived)
	{
		m_pRLC->GetTrackData()->mSplitsAnalysed = false;
		return;
	}

	bool aiAvailable = m_pRLC->IsAIDataAnalysed();

	int splineLength = (m_pRLC->GetTrackData()->mNumWaypoints-1);

	//Pick any old point on the spline
	int		startWaypoint = 1,
			waypointIndex = 0,
			noSplits = 0;
	float	distance = 0.f,
			offset = 0.5f,
			trackPercentage = 0.f,
			splitLineLength = 0.f;

	Maths::HVec3 centrePoint = Maths::HVec3();
	Maths::HVec3 waypointPos = Maths::HVec3();
	Maths::HVec3 direction = Maths::HVec3();


	noSplits = m_pRLC->GetTrackData()->mSplitData.Size();
	for(int splitIndex = 0; splitIndex < noSplits; splitIndex++)
	{
		SplitData* pSplit = &m_pRLC->GetTrackData()->mSplitData[splitIndex];

		waypointIndex = -1;
		distance = 0.f;

		//BUG FIX This is caused by the splits sometimes being set up so far below the circuit that an error occurs 
		//in the calculation of the nearest split. The fix will be to place the the split line on the AI spline by getting 
		//what we 'think' is a reasonable waypoint then re-find the waypoint based on the new position, this should minimise 
		//the problem...I hope
		
		//Work out the centre of the split line
		centrePoint = (pSplit->mStart + pSplit->mEnd) / 2.f;
		trackPercentage = (1.f / noSplits) * splitIndex;

		if(aiAvailable)
		{
			//Work out which waypoint is closest to the centre point of the split
			waypointIndex = m_pRLC->GetTrackData()->m_pLogicRoute->FindNearest(startWaypoint, splineLength, centrePoint, distance, m_pRLC->GetTrackData()->mWaypointSize);

			if(waypointIndex >= 0)
			{
				//Now get the position of the waypoint exactly (not sure what the offset's for)
				m_pRLC->GetTrackData()->m_pLogicRoute->GetPosition(offset, waypointIndex, waypointPos);

				m_pRLC->GetTrackData()->m_pLogicRoute->GetDirection(waypointIndex, direction);
				
				float x = direction.X();

				direction.X(-direction.Z());
				direction.Z(x);

				//Put the split at exactly the right height of the waypoint
				//pSplit->mStart.Y(waypointPos.Y());
				//pSplit->mEnd.Y(waypointPos.Y());

				splitLineLength = Maths::HVec3(pSplit->mStart - pSplit->mEnd).Length();

				//Now we can work out the proper centre of the split based upon the corrected position of the split
				centrePoint = (pSplit->mStart + pSplit->mEnd) / 2.f;
				pSplit->mTrueStart = centrePoint - (direction * splitLineLength/2.f);
				pSplit->mTrueEnd = centrePoint + (direction * splitLineLength/2.f);


				//...and get the correct waypoint index and position for this corrected position
				waypointIndex = m_pRLC->GetTrackData()->m_pLogicRoute->FindNearest(startWaypoint, splineLength, centrePoint, distance, m_pRLC->GetTrackData()->mWaypointSize);
			}
		}

		pSplit->mCentre = centrePoint;
		pSplit->mWaypoint = waypointIndex;
		pSplit->mAccurateWaypoint = waypointIndex + distance;

		//We'll set this as a default value, just in case we don't have an AI spline to order the splits correctly
		pSplit->mTrackPercentage = trackPercentage;
		pSplit->mSorted = true;
	}

	//We're all done here
	m_pRLC->GetTrackData()->mSplitsAnalysed = true;
}

//CA - There was always a problem with previous code where on a reverse circuit (3 splits) the order of the splits would be 
//0 - 2 - 1 this meant that every bit of code that dealt with splits or anything always had to do some validation which was monkey! 
//This function will get around the problem by defining split 1 as the first split after the start/finish line travelling the right way.
//Nb/ Split 0 is still always the start/finish line
void RLTrackPositionController::CalculateSplitOrder()
{
	if(!m_pRLC->IsSplitDataAnalysed())
		return;
	
	SplitArray tempSplits;
	for(int i = 0; i < m_pRLC->GetTrackData()->mSplitData.Size(); i++)
	{
		m_pRLC->GetTrackData()->mSplitData[i].mTrackPercentage = CalculateTrackPercentage((float)m_pRLC->GetTrackData()->mSplitData[i].mAccurateWaypoint);
		m_pRLC->GetTrackData()->mSplitData[i].mSorted = false;
	}

	//Now lets sort wooo!
	float leastPercentage;
	int smallestOrdinal;
	for(int i = 0; i < m_pRLC->GetTrackData()->mSplitData.Size(); i++)
	{
		leastPercentage = 200.f;
		smallestOrdinal = -1;

		for(int j = 0; j < m_pRLC->GetTrackData()->mSplitData.Size(); j++)
		{
			if(m_pRLC->GetTrackData()->mSplitData[j].mSorted)
				continue;

			if(m_pRLC->GetTrackData()->mSplitData[j].mTrackPercentage <= leastPercentage)
			{
				leastPercentage = m_pRLC->GetTrackData()->mSplitData[j].mTrackPercentage;
				smallestOrdinal = j;
			}

		}

		if(smallestOrdinal == -1 || smallestOrdinal >= m_pRLC->GetTrackData()->mSplitData.Size())
			assert(!"RaceLogicSplineAnalysisProcessor::CalculateSplitOrder() > Charlie something's gone wrong with sorting the splits...n00b!");
		else
		{
			m_pRLC->GetTrackData()->mSplitData[smallestOrdinal].mSorted = true;
			m_pRLC->GetTrackData()->mSplitData[smallestOrdinal].mSplitNumber = i;
			tempSplits.PushBack(m_pRLC->GetTrackData()->mSplitData[smallestOrdinal]);
		}
	}

	m_pRLC->GetTrackData()->mSplitData = tempSplits;
}


void RLTrackPositionController::PreUpdate()
{
	PERF_EVENT("RLTrackPositionController::PreUpdate");

	if(g_ReAnalyseSplits)
	{
		AnalyseSplits();
		g_ReAnalyseSplits = false;
	}

	for(int vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)
	{
		if(!m_pRLC->IsVehicleLoaded(vehicleIndex))
			continue;

		//Make sure we have a valid pointer to the vehicle actor
		if(m_pRLC->mVehicles[vehicleIndex].m_pVehicleActor)
		{
			PreUpdateVehicle(vehicleIndex);
		}
	}

	//Now calculate how the relative positions of each vehicle
	CalculateProximities();
}

void RLTrackPositionController::PreUpdateVehicle(const U8 vehicleIndex)
{
	PERF_EVENT("RLTrackPositionController::PreUpdateVehicle");

	//Get the vehicle
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	//...and the vehicle's data as it was last frame
	const RLVehicleData *pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];

	//Make sure we're ok to do this
	bool bothValid = pVehicle->mValid && pLastVehicle->mValid;

	//Get the model input structure
	const Vehicle::C_ModelInput *pModelInput = m_pRLC->GetVehicleModelInput(vehicleIndex);

	pVehicle->mWorldMatrix = pVehicle->m_pVehicleActor->GetMatrix();
	Gravity::C_Body* pBody = pVehicle->m_pVehicleActor->GetCollisionBody();
	pVehicle->mAngularVelocity = pBody->AngularVelocity();

	//Get the world position this frame
	pVehicle->mWorldPosition = pVehicle->mWorldMatrix.AW();
	pVehicle->mDirection = pVehicle->mWorldMatrix.AZ();
	pVehicle->mUp = pVehicle->mWorldMatrix.AY();
	pVehicle->mLeft = pVehicle->mWorldMatrix.AX();

	pVehicle->mVelocity = pModelInput->m_vVelocity;

	// Only need this for humans atm
	if( !m_pRLC->IsVehicleAIControlled(pVehicle->mVehicleIndex) )
		pVehicle->mDotUp = Maths::DotProduct(pVehicle->mUp, Maths::HVec3(0.f, 1.f, 0.f) );

	float speedInMetresPerSecond = abs(Magnitude(pVehicle->mVelocity));

	//[ChadW] Optim
	//if(speedInMetresPerSecond > 0.f)
	//{
	//	pVehicle->mVelocityDirection = pVehicle->mVelocity;
	//	pVehicle->mVelocityDirection.Normalise();
	//}
	//else
	//	pVehicle->mVelocityDirection = pVehicle->mDirection;

	pVehicle->mSpeedInMetresPerSecond = speedInMetresPerSecond;
	pVehicle->mSpeedInMilesPerHour = (speedInMetresPerSecond * 3600.f) / 1609.f;

	if(bothValid)
	{
		Maths::HVec3 distanceTravelledSinceLastUpdate = pLastVehicle->mWorldPosition - pVehicle->mWorldPosition;
		pVehicle->mDistanceTravelledSinceLastUpdate = distanceTravelledSinceLastUpdate.Length();
	}

	if(m_pRLC->IsAIDataAnalysed())
	{
		//Lets get the waypoint
		airacingtech::ISplineTracker *pVehicleSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(vehicleIndex);
		pVehicle->mWaypointIndex = pVehicleSplineTracker->GetLogicRoute()->m_index;
		pVehicle->mAccurateWaypoint = pVehicle->mWaypointIndex + pVehicleSplineTracker->GetLogicRoute()->m_ZDist;
	}

}

void RLTrackPositionController::Update()
{
	PERF_EVENT("RLTrackPositionController::Update");

	//Go through each vehicle
	for(int vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)
	{
		UpdateVehicle(vehicleIndex);
	}
}

void RLTrackPositionController::UpdateVehicle(const U8 vehicleIndex)
{
	PERF_EVENT("RLTrackPositionController::UpdateVehicle");

	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	bool aiAnalysed = m_pRLC->IsAIDataAnalysed();

	//See if we've sorted out the vehicles next and previous split yet, these will have an index of -1
	if(pVehicle->mNextSplitIndex < 0 || pVehicle->mPreviousSplitIndex < 0)
	{
		CalculateInitialNextAndPreviousSplitIndices(vehicleIndex);
	}

	CheckVehicleSplitPosition(vehicleIndex);

	if(aiAnalysed)
	{
		//Work out how far around the track we are
		CalculateVehicleTrackPercentage(vehicleIndex);

		//...and how far through the race we are
		CalculateVehicleRacePercentage(vehicleIndex);

		//Now work out where in the race we are, ie how far from the start line we are...
		CalculateVehicleDistanceFromStartLine(vehicleIndex);

		//...and now from the split lines ahead and behind
		CalculateVehicleDistanceToNextAndPreviousSplitLine(vehicleIndex);

		//...then work out the distance to cars ahead and behind
		CalculateVehicleDistancesToCarsAheadAndBehind(vehicleIndex);

		//Work out if we're going the right way
		CalculateVehicleWrongWayDistance(vehicleIndex);
	}
}

//Check if the vehicle's crossed any splits since we last looked
void RLTrackPositionController::CheckVehicleSplitPosition(const U8 vehicleIndex)
{
	if(!m_pRLC->VehicleValid(vehicleIndex)) 
	{
		assert(!"RLTrackPositionController::CheckVehicleSplitPosition() > Invalid vehicleId");
		return;
	}

	//The current position of the vehicle
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	//The position last update
	RLVehicleData *pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];

	//If the vehicle data's invalid then there's not point in carrying on, we'll just wait for next time round
	if(!pVehicle->mValid || !pLastVehicle->mValid)
		return;

	if(m_pRLC->IsAIDataAnalysed())
	{
		//Work out the difference between the two waypoints
		float difference = fabs(pVehicle->mAccurateWaypoint - pLastVehicle->mAccurateWaypoint);
		//If the difference between the two waypoints is more than 99% of the entire tracks we're probably looping over so don't check splits as that way lies only pain
		if(difference > (m_pRLC->GetTrackData()->mNumWaypoints * 0.99f))
			return;
	}

	//Cycle through each split to see if the split line has been crossed
	SplitArray splits = m_pRLC->GetTrackData()->mSplitData;
	for(U8 i = 0; i < splits.Size(); i++)
	{
		float accurateWaypoint = splits[i].mAccurateWaypoint;
		/*
		if(g_UseOldFashionedSplitLineAnalysis)
		{
			//We don't have AI data so we'll use the old fashion
			if(Line2DIntersect(pVehicle->mWorldPosition, pLastVehicle->mWorldPosition, splits[i].mStart, splits[i].mEnd))
				//Tell the controller we've passed a split
				SplitCrossed(vehicleIndex, splits[i].mSplitNumber);
		}
		//Check to see if the current vehicle crossed a split
		else if(!m_pRLC->IsAIDataAnalysed() && pVehicle->mValid && pLastVehicle->mValid )
		*/
		if( (!m_pRLC->IsAIDataAnalysed() && pVehicle->mValid && pLastVehicle->mValid ) || g_UseOldFashionedSplitLineAnalysis )
		{
			//We don't have AI data so we'll use the old fashion
			if(Line2DIntersect(pVehicle->mWorldPosition, pLastVehicle->mWorldPosition, splits[i].mStart, splits[i].mEnd))
				//Tell the controller we've passed a split
				SplitCrossed(vehicleIndex, splits[i].mSplitNumber);
		}
		else if( (pLastVehicle->mAccurateWaypoint < splits[i].mAccurateWaypoint && pVehicle->mAccurateWaypoint > splits[i].mAccurateWaypoint)
					||
					(pLastVehicle->mAccurateWaypoint > splits[i].mAccurateWaypoint && pVehicle->mAccurateWaypoint < splits[i].mAccurateWaypoint) )
		{
			//Tell the controller we've passed a split
			SplitCrossed(vehicleIndex, splits[i].mSplitNumber);
		}
	}
}

const bool RLTrackPositionController::Line2DIntersect(Maths::HVec3 currentPosition, Maths::HVec3 lastPosition, Maths::HVec3 lineStart, Maths::HVec3 lineEnd)
{
	Maths::HVec3 normal = Maths::HVec3(lineStart.Z() - lineEnd.Z(), 0.f, -lineStart.X() + lineEnd.X());

	float a1 = normal | (lineStart - currentPosition);
	float a2 = normal | (lineStart - lastPosition);

	if(a1 * a2 >= 0.f || a1 == 0.f)
		return false;

	Maths::HVec3 direction = (lineEnd - currentPosition);

	//Reject by line normal
	normal = Maths::HVec3((currentPosition.Z() - lastPosition.Z()), 0.f, -currentPosition.X() + lastPosition.X()); 

	float a21 = normal | (lineStart - currentPosition);
	float a22 = normal | direction;

	if(a21 * a22 >= 0.f || a21 == 0.f)
		return false;

	return true;
}

void RLTrackPositionController::CalculateInitialNextAndPreviousSplitIndices(const U8 vehicleIndex)
{
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];

	//Probably a terrible assumption but lets assume that the player's going the right way and will cross the start finish line first
	if(pVehicle->mNextSplitIndex < 0 || pVehicle->mPreviousSplitIndex < 0)
	{
		pVehicle->mNextSplitIndex = 0;
		pVehicle->mPreviousSplitIndex = m_pRLC->GetTrackData()->mSplitData.Size() - 1;

		//If we've just had to work out the previous and next splits it seems to me that last update's
		//data is probably junk so we'll invalidate the data
		pLastVehicle->mValid = false;
	}
}

//The vehicle could be going around the circuit the wrong way
const bool RLTrackPositionController::ValidSplit(const U8 vehicleIndex, const U8 splitIndex) const
{
	//Simply put if the split index is the index of the split we're waiting for all is cool!
	return (splitIndex == m_pRLC->mVehicles[vehicleIndex].mNextSplitIndex);
}

void RLTrackPositionController::SplitCrossed(const U8 vehicleIndex, const U8 splitIndex)
{
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	int noSplits = m_pRLC->GetTrackData()->mSplitData.Size();

	bool validSplit = ValidSplit(vehicleIndex, splitIndex);
	if(validSplit)
	{
		//Normal
		pVehicle->mPreviousSplitIndex = pVehicle->mLastSplitIndex = pVehicle->mNextSplitIndex;

		//Check to see if this is the last split
		if(splitIndex == noSplits -1)
			pVehicle->mNextSplitIndex = 0;
		else
			pVehicle->mNextSplitIndex++;

		//We've passed the split we were expecting so we can increment this
		pVehicle->mNoSplits++;
		//If this is the start finish line we can increment the maximum laps too 
		if(splitIndex == 0)
		{
			pVehicle->mNoLaps++;

			if(pVehicle->mCurrentLapData.mLapNo < pVehicle->mNoLaps )
			{
				C_LapIncrementedMsg lapMsg(vehicleIndex, pVehicle->mNoLaps, pVehicle->mCurrentLapData.mLapTime);
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&lapMsg);
				m_pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&lapMsg);
			}
		}
		else
		{
			C_SplitCrossedMsg splitMsg(vehicleIndex, pVehicle->mNextSplitIndex, pVehicle->mNoLaps);
			m_pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&splitMsg);
		}

		//These two variables represent a 'high tide' of splits/laps
		if(pVehicle->mNoSplits > pVehicle->mMaximumSplit)
			pVehicle->mMaximumSplit = pVehicle->mNoSplits;

		if(pVehicle->mNoLaps > pVehicle->mMaximumLap)
			pVehicle->mMaximumLap = pVehicle->mNoLaps;
	}
	else if(pVehicle->mPreviousSplitIndex == splitIndex)
	{
		//Ok we're going backwards, so we'll assume the player's going to turn around
		pVehicle->mNextSplitIndex = splitIndex;

		//Check to see if this is the last split
		if(splitIndex == 0)
			pVehicle->mPreviousSplitIndex = noSplits -1;
		else
			pVehicle->mPreviousSplitIndex--;

		//We've gone backwards passed the split we were sort of expecting them to so decrement this
		pVehicle->mNoSplits--;
		//If this is the start finish line we need to decrement this too so we don't mess with our distance from startline calcs
		if(splitIndex == 0)
			pVehicle->mNoLaps--;
	}
	else
	{
		//TODO
		//We've reset the car in some weird place 
		int todoSortOutResetingCorrectlyWithinTheTrackPositionController = 0;
		todoSortOutResetingCorrectlyWithinTheTrackPositionController++;
		
		
	}
}

//Calculate how close to each other the vehicles are
void RLTrackPositionController::CalculateProximities()
{
	//Go through each vehicle and work out how the distance between it and every other vehicle
	for(U8 vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)
	{
		CalculateVehicleProximities(vehicleIndex);
	}
}

void RLTrackPositionController::CalculateVehicleProximities(const U8 vehicleIndex)
{
	PERF_EVENT("RLTrackPositionController::CalculateVehicleProximities");

	float reportingProximityDistanceSquared = -1.f;
	if(ValidateDatabase())
	{
		reportingProximityDistanceSquared = m_pRLC->GetRLSystem()->mReportingParameters->mProximityReportingDistanceSquared;
	}

	//Get the vehicle
	RLVehicleData *pCurrentVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pOtherVehicle = 0;

	//[ChadW] - Optim
	////Clear out the last frame's calculations
	//for(int index = 0; index < m_pRLC->mVehicles.Size(); index++)
	//	pCurrentVehicle->mVehicleProximities[index].Initialise(index);

	Maths::HVec3	relativePos, 
					relativeDir, 
					relativeVel, 
					relativeVelDir,
					
					heading;

	//Now go through each other vehicle
	VehicleProximity* pProximity = NULL;
	for(int otherVehicleIndex = 0; otherVehicleIndex < m_pRLC->mVehicles.Size(); otherVehicleIndex++)
	{
		//[ChadW] - Optim
		pCurrentVehicle->mVehicleProximities[otherVehicleIndex].Initialise(otherVehicleIndex);

		//We don't want to look at our own vehicle!
		if(otherVehicleIndex == vehicleIndex)
			continue;

		pProximity = &pCurrentVehicle->mVehicleProximities[otherVehicleIndex];

		pOtherVehicle = &m_pRLC->mVehicles[otherVehicleIndex];
		
		relativePos = pCurrentVehicle->mWorldPosition - pOtherVehicle->mWorldPosition;
		pProximity->mDistanceSquared = relativePos.LengthSquared();
		pProximity->mVehicleIndex = otherVehicleIndex;
		if (pProximity->mDistanceSquared > 40000.0f) //200metres
			continue;
		relativeDir = relativePos;
		relativeDir.Normalise();

		//[ChadW] - Optim
		//relativeVel = pCurrentVehicle->mVelocity - pOtherVehicle->mVelocity;
		//relativeVelDir = relativeVel;
		
		//[ChadW] - Optim
		//relativeVel *= relativeDir;
		
		//[ChadW] - Optim
		//proximity.mClosingSpeedSquared = relativeVel.LengthSquared();

		//[ChadW] - Optim
		//if(Maths::DotProduct(relativeVelDir, relativeDir) > 0.f)
		//{
		//	//We're going away from this vehicle
		//	proximity.mClosingSpeedSquared *= -1.f;
		//}

		//[ChadW] - Optim
		////See if the other vehicle's in the 180 deg ahead of the vehicle
		//if(Maths::DotProduct(pCurrentVehicle->mDirection, relativeDir) > 0.f)
		//	proximity.mAhead = false;
		//else
		//	proximity.mAhead = true;

		//[ChadW] - Optim
		//if(Maths::DotProduct(pCurrentVehicle->mLeft, relativeDir) > 0.f)
		//	proximity.mToTheLeft = false;
		//else
		//	proximity.mToTheLeft = true;

		pProximity->mRelativeHeading = Maths::DotProduct(pCurrentVehicle->mDirection, -relativeDir);

		//[ChadW] - Optim
		//pCurrentVehicle->mVehicleProximities[otherVehicleIndex] = proximity;

		//[ChadW] - Optim
		//if(proximity.mDistanceSquared < reportingProximityDistanceSquared)
		//{
		//	C_VehicleCloseProximityMsg proxWarningMessage(pCurrentVehicle->mActorHandle, 
		//												  pCurrentVehicle->mVehicleIndex, 
		//												  pOtherVehicle->mActorHandle, 
		//												  pOtherVehicle->mVehicleIndex, 
		//												  proximity.mDistanceSquared);
		//}
	}
}

//Works out the percentage distance around the track the current vehicle is
void RLTrackPositionController::CalculateVehicleTrackPercentage(const U8 vehicleIndex)
{
	//Get the vehicle
	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	pVehicle->mTrackPercentage = CalculateTrackPercentage(pVehicle->mAccurateWaypoint);
	
}

const float RLTrackPositionController::CalculateTrackPercentage(const float accurateWaypointPosition) const
{
	if(m_pRLC->GetTrackData()->mTrackLength < 0)
	{
		LOGERROR(g_RLTrackPositionLog, "RLTrackPositionController::CalculateTrackPercentage() > Spline not initialised yet");
		return -1.f;
	}

	float percentage = 0.f;
	float startFinishWaypoint = m_pRLC->GetTrackData()->mStartFinishAccurateWaypoint;
	float numWaypoints = (float)m_pRLC->GetTrackData()->mNumWaypoints;

	if(accurateWaypointPosition < startFinishWaypoint)
		percentage = ((accurateWaypointPosition - startFinishWaypoint) + numWaypoints) / numWaypoints;
	else
		percentage = (accurateWaypointPosition - startFinishWaypoint)/ numWaypoints;

	return percentage;
}

//Calculates a number between 0 - 1 for the race percentage. 0 means the race is just beginning and 1.f means just ended
void RLTrackPositionController::CalculateVehicleRacePercentage(const U8 vehicleIndex)
{
	//bool databaseValid = ValidateDatabase();
	float endOfRacePercentage = 1.f;
	if( ValidateDatabase() )
		endOfRacePercentage = m_pRLC->GetRLSystem()->mReportingParameters->mEndOfRacePercentage;


	//Get the vehicle
	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData* pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];
	pVehicle->mRacePercentage = 0.f;

	int noLaps = C_GameApp::GetInstance()->GetGame()->GetLevelState().mLaps;
	float newRacePercentage = 0.f;
	
	//Make sure we've got a valid number of laps and that the vehicle's on the first lap
	if(noLaps > 0 && pVehicle->mNoLaps > 0)
	{
		pVehicle->mRacePercentage = (pVehicle->mTrackPercentage + (pVehicle->mMaximumLap-1)) / noLaps;

		if(pVehicle->mRacePercentage >= endOfRacePercentage
			&&
			pLastVehicle->mRacePercentage < endOfRacePercentage)
		{
			C_VehicleNearlyFinishedRaceMsg nearlyFinishedMsg(pVehicle->mActorHandle, vehicleIndex, pVehicle->mRacePercentage);
			m_pRLC->BroadcastMessage(&nearlyFinishedMsg);
		}

		if(pVehicle->mRacePercentage >= endOfRacePercentage)
			pVehicle->mNearEndOfRace = true;
		else
			pVehicle->mNearEndOfRace = false;
	}
}

void RLTrackPositionController::CalculateVehicleDistanceFromStartLine(const U8 vehicleIndex)
{
	if(!m_pRLC->IsAIDataAnalysed())
		return;
	else if(!m_pRLC->IsSplitDataAnalysed())
		return;

	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//Make sure we're on the right lap, this is not necessarily the number we'd show the player as if they drive backwards this will go negative
	float trackLength = m_pRLC->GetTrackData()->mNumWaypoints * m_pRLC->GetTrackData()->mWaypointSize;
	float waypointSize = m_pRLC->GetTrackData()->mWaypointSize;
	float startFinishWaypointPercentage = m_pRLC->GetTrackData()->mSplitData[0].mTrackPercentage; //Ought to be 0%
	float absDistToSF = (float)abs(AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(startFinishWaypointPercentage, pVehicle->mTrackPercentage) ) * waypointSize;

	//Work out where we are on the circuit
	float newDistanceFromStartLine = (((float)(pVehicle->mNoLaps - 1) + pVehicle->mTrackPercentage) * trackLength);

	//Make sure we don't have great changes unexpectedly (caused by a monkey of a bug)
	//if(absDistToSF > g_NoWaypointsInSFBlackSpot)
		pVehicle->mDistanceFromStartLine = newDistanceFromStartLine;
	
}

void RLTrackPositionController::CalculateVehicleDistanceToNextAndPreviousSplitLine(const U8 vehicleIndex)
{
	if(!m_pRLC->IsAIDataAnalysed())
		return;
	else if(!m_pRLC->IsSplitDataAnalysed())
		return;

	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];

	//Make sure we've not invalidated this for any reason...
	if(!pVehicle->mValid)
		return;

	//Check that the vehicle's got a valid waypoint, next and previous split
	if(pVehicle->mNextSplitIndex < 0 || pVehicle->mPreviousSplitIndex < 0 || pVehicle->mWaypointIndex < 0)
		return;

	
	float waypointSize = m_pRLC->GetTrackData()->mWaypointSize;

	float nextSplitWaypointPercentage = m_pRLC->GetTrackData()->mSplitData[pVehicle->mNextSplitIndex].mTrackPercentage;
	float previousSplitWaypointPercentage = m_pRLC->GetTrackData()->mSplitData[pVehicle->mPreviousSplitIndex].mTrackPercentage;

	pVehicle->mDistanceToNextSplit = (S32)abs(AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(nextSplitWaypointPercentage, pVehicle->mTrackPercentage));
	pVehicle->mDistanceToPreviousSplit = (S32)abs(AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(previousSplitWaypointPercentage, pVehicle->mTrackPercentage));
}

void RLTrackPositionController::CalculateVehicleWrongWayDistance(const U8 vehicleIndex)
{
	if(!m_pRLC->IsAIDataAnalysed())
		return;

	RLVehicleData* pCurrentVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData* pLastVehicle = &m_pRLC->mLastVehicles[vehicleIndex];

	//If the vehicle data's invalid then there's not point in carrying on, we'll just wait for next time round
	if(!pCurrentVehicle->mValid || !pLastVehicle->mValid)
		return;

	float distanceFromStartLineSinceLastFrame = pCurrentVehicle->mDistanceFromStartLine - pLastVehicle->mDistanceFromStartLine;
	if(distanceFromStartLineSinceLastFrame < 0.f)
	{
		//Looks like we're going backwards
		pCurrentVehicle->mWrongWayDistance += abs(distanceFromStartLineSinceLastFrame);

		if(pCurrentVehicle->mWrongWayDistance >= g_MaxWrongWayDistance)
			pCurrentVehicle->mGoingTheWrongWay = true;
	}
	else
	{
		pCurrentVehicle->mWrongWayDistance = 0;
		pCurrentVehicle->mGoingTheWrongWay = false;
	}
}

void RLTrackPositionController::CalculateVehicleDistancesToCarsAheadAndBehind(const U8 vehicleIndex)
{
	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pOtherVehicle = 0;
	Maths::HVec3 between;
	int otherVehicleIndex;
	int trackLength = m_pRLC->GetTrackData()->mTrackLength;

	pVehicle->mDistanceToVehicleAhead = pVehicle->mDistanceToVehicleBehind = 0;

	//We haven't worked out the race position yet or this is a single player event...either way we don't want to go any further
	if(pVehicle->mRacePosition == INVALID_RACE_POSITION)
		return;
	

	//Default the values
	pVehicle->mDistanceToVehicleAhead = pVehicle->mRealDistanceToVehicleAhead = 0.f;
	pVehicle->mTimeToVehicleAhead = 0.f;
	pVehicle->mVehicleIndexAhead = -1;

	pVehicle->mDistanceToVehicleBehind = pVehicle->mRealDistanceToVehicleBehind = 0.f;
	pVehicle->mTimeToVehicleBehind = 0.f;
	pVehicle->mVehicleIndexBehind = -1;

	//Car ahead make sure we're not in first
	if(pVehicle->mRacePosition > 1)
	{
		//Not in first
		otherVehicleIndex = m_pRLC->GetVehicleIndexAtRacePosition(pVehicle->mRacePosition-1);
		//Check nothing's going bonkers
		if(m_pRLC->VehicleValid(otherVehicleIndex))
		{
			pOtherVehicle = &m_pRLC->mVehicles[otherVehicleIndex];

			between = pOtherVehicle->mWorldPosition - pVehicle->mWorldPosition;

			pVehicle->mVehicleIndexAhead = otherVehicleIndex;
			pVehicle->mDistanceToVehicleAhead = CalculateDistanceToVehicle(vehicleIndex, otherVehicleIndex);
			pVehicle->mRealDistanceToVehicleAhead = between.Length();
			pVehicle->mTimeToVehicleAhead = CalculateTimeToVehicle(vehicleIndex, otherVehicleIndex);

#ifndef CFG_GOLD
			if(g_Stamp)
				LOGDEBUG(g_RLTrackPositionLog, "RLTrackPositionController() > Veh: %d Dist to car ahead %3.02fwp time %3.02fs real %3.02fm", 
							vehicleIndex,
							pVehicle->mDistanceToVehicleAhead, 
							pVehicle->mTimeToVehicleAhead, 
							pVehicle->mRealDistanceToVehicleAhead);
#endif
		}
	}

	//Car behind
	if(pVehicle->mRacePosition < m_pRLC->mVehicles.Size())
	{
		//Not last
		otherVehicleIndex = m_pRLC->GetVehicleIndexAtRacePosition(pVehicle->mRacePosition+1);
		//Check nothing's going bonkers
		if(m_pRLC->VehicleValid(otherVehicleIndex))
		{
			pOtherVehicle = &m_pRLC->mVehicles[otherVehicleIndex];

			between = pOtherVehicle->mWorldPosition - pVehicle->mWorldPosition;

			pVehicle->mVehicleIndexBehind = otherVehicleIndex;
			pVehicle->mDistanceToVehicleBehind = CalculateDistanceToVehicle(vehicleIndex, otherVehicleIndex);
			pVehicle->mRealDistanceToVehicleBehind = between.Length();	
			pVehicle->mTimeToVehicleBehind = CalculateTimeToVehicle(vehicleIndex, otherVehicleIndex);

	#ifndef CFG_GOLD
			if(g_Stamp)
				LOGDEBUG(g_RLTrackPositionLog, "RLTrackPositionController() > Veh: %d Dist to car behind %3.02fwp time %3.02fs real %3.02fm", 
							vehicleIndex, 
							pVehicle->mDistanceToVehicleBehind, 
							pVehicle->mTimeToVehicleBehind, 
							pVehicle->mRealDistanceToVehicleBehind);
	#endif
		}
	}
}

//Calculates the distance between vehicles based upon their distanceFromStartline values
const float RLTrackPositionController::CalculateDistanceToVehicle(const U8 vehicleIndex, const U8 otherVehicleIndex)
{
	//Make sure we're being passed valid vehicle indices
	if(!m_pRLC->VehicleValid(vehicleIndex))
	{
		BIZ_ASSERT_FAIL("RLTrackPositionController::CalculateDistanceToVehicle > Being sent an invalid vehicleIndex");
		return 0;
	}
	if(!m_pRLC->VehicleValid(otherVehicleIndex))
	{
		BIZ_ASSERT_FAIL("RLTrackPositionController::CalculateDistanceToVehicle > Being sent an invalid otherVehicleIndex");
		return 0;
	}

	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pOtherVehicle = &m_pRLC->mVehicles[otherVehicleIndex];

	return pOtherVehicle->mDistanceFromStartLine - pVehicle->mDistanceFromStartLine;
}

const float RLTrackPositionController::CalculateTimeToVehicle(const U8 vehicleIndex, const U8 otherVehicleIndex)
{
	//Make sure we're being passed valid vehicle indices
	if(!m_pRLC->VehicleValid(vehicleIndex))
	{
		BIZ_ASSERT_FAIL("RLTrackPositionController::CalculateTimeToVehicle > Being sent an invalid vehicleIndex");
		return 0;
	}
	if(!m_pRLC->VehicleValid(otherVehicleIndex))
	{
		BIZ_ASSERT_FAIL("RLTrackPositionController::CalculateTimeToVehicle > Being sent an invalid otherVehicleIndex");
		return 0;
	}

	RLVehicleData *pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLVehicleData *pOtherVehicle = &m_pRLC->mVehicles[otherVehicleIndex];
	float timeBetween = 0.f, 
		  minimumSpeedThreshold = 0.5f;	//We don't want to go around dividing by 0


	//Work out the distance between the vehicles based upon the distance from startline
	float intraVehiclularDistance = pOtherVehicle->mDistanceFromStartLine - pVehicle->mDistanceFromStartLine;

	if(intraVehiclularDistance < 0.f && pOtherVehicle->mSpeedInMetresPerSecond > minimumSpeedThreshold)
		//Other vehicle's behind
		timeBetween = intraVehiclularDistance / pOtherVehicle->mSpeedInMetresPerSecond;
	else if(pVehicle->mSpeedInMetresPerSecond > minimumSpeedThreshold)
		//Other vehicle's ahead
		timeBetween = intraVehiclularDistance / pVehicle->mSpeedInMetresPerSecond;

	return timeBetween;
}

const SplitTimeData RLTrackPositionController::GetSplitTimeDataAtRaceSplitNumber(const U8 vehicleIndex, const int raceSplitNumber) const
{
	SplitTimeData data;
	data.Initialise();

	//Go through all the records we have for this vehicle and see if we can find this raceSplitNumber, we'll just do it by brute force...sorry...
	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];
		
	//Check the current lap
	for(int i = 0; i < pVehicle->mCurrentLapData.mSplitTimeData.Size(); i++)
	{
		if(pVehicle->mCurrentLapData.mSplitTimeData[i].mRaceSplitNumber == raceSplitNumber)
		{
			return pVehicle->mCurrentLapData.mSplitTimeData[i];
		}
	}

	//If we're here, we didn't find the split so we'll check the back records
	//Go through each back lap
	for(int lap = 0; lap < pVehicle->mLapTimeArray.Size(); lap++)
	{
		//Go through each back split within the back lap
		for(int split = 0; split < pVehicle->mLapTimeArray[lap].mSplitTimeData.Size(); split++)
		{
			if(pVehicle->mLapTimeArray[lap].mSplitTimeData[split].mRaceSplitNumber == raceSplitNumber)
				return pVehicle->mLapTimeArray[lap].mSplitTimeData[split];
		}
	}
	
	//If we're here we can't have found the record...this could be because it happened so long ago or some other reason...
	return data;
}

void RLTrackPositionController::AddDebugLines()
{
	AddDebugLinesForSplitLines();
	AddDebugLinesForStartPositions();
	//AddDebugLinesForPickupPositions();
}

void RLTrackPositionController::AddDebugLinesForSplitLines()
{
	//Go through each split line and add a single line for each split
	Maths::RGBA colour, fadedColour;
	Maths::HVec3 y, x, z;
	float halfSize = 2.f;
	for(int splitIndex = 0; splitIndex < m_pRLC->GetTrackData()->mSplitData.Size(); splitIndex++)
	{
		SplitData split = m_pRLC->GetTrackData()->mSplitData[splitIndex];

		if(split.mSplitNumber == 0)
		{
			colour = Maths::RGBA(0xff, 0x00, 0x00, 0xff);	//Red for the start/finish
			fadedColour = Maths::RGBA(0x80, 0x00, 0x00, 0xff);
		}
		else
		{
			colour = Maths::RGBA(0x00, 0x00, 0xff, 0xff);	//Blue for a split
			fadedColour = Maths::RGBA(0x00, 0x00, 0x80, 0xff);
		}
		
		x = split.mStart - split.mEnd;
		x = Maths::Normalise(x);

		y = Maths::HVec3(0.f, 1.f, 0.f);
		z = x ^ y;
		z = Maths::Normalise(z);

		x *= halfSize;
		y *= halfSize;
		z *= halfSize;

		//Make a cross at the centre of the split line
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_SPLIT_LINE, split.mCentre - z, split.mCentre + z, colour );
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_SPLIT_LINE, split.mCentre - y, split.mCentre + y, colour );
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_SPLIT_LINE, split.mCentre - x, split.mCentre + x, colour );

		//The line itself
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_SPLIT_LINE, split.mStart, split.mEnd, colour );

		//The line of the waypoint
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_SPLIT_LINE, split.mTrueStart, split.mTrueEnd, fadedColour );

	
	}


}

void RLTrackPositionController::AddDebugLinesForStartPositions()
{
	//Go through each start position and add lines to show where it is and which way its pointing
	Maths::RGBA colour = Maths::RGBA(0xff, 0x00, 0xff, 0xff);	//Magenta
	Maths::HVec3 pos, x, y, z;
	float scale = 0.5f;
	
	for(int i = 0; i < m_pRLC->GetTrackData()->mStartPositions.Size(); i++)
	{
		const RLStartPosition& startPos = m_pRLC->GetTrackData()->mStartPositions[i];

		pos = startPos.mWorldMatrix.AW();
		x = startPos.mWorldMatrix.AX();
		y = startPos.mWorldMatrix.AY();
		z = startPos.mWorldMatrix.AZ();

		//Forward/back line
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_START_POSITION, pos - (z*scale), pos + (z*scale*5.f), colour );
		//Arrow 
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_START_POSITION, pos + (z*scale*5.f), pos+(z*scale*4.f)-(x*scale*0.5f), colour );
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_START_POSITION, pos + (z*scale*5.f), pos+(z*scale*4.f)+(x*scale*0.5f), colour );
	
		//Left/right line
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_START_POSITION, pos - (x*scale), pos + (x*scale), colour );
		//Up/down
		m_pRLC->GetDebugLineController()->AddDebugLine(E_DEBUG_LINE_TYPE_START_POSITION, pos - (y*scale), pos + (y*scale), colour );
	}
}

void RLTrackPositionController::ResetVehicleOnPoint(int vehicleIndex, Maths::HMat33 orientation, Maths::HVec3 velocity, Maths::HVec3 point)
{

}

const bool RLTrackPositionController::ValidateDatabase() const
{
	//[ChadW] Optim
	//return true;

	bool valid = true;
	
	if(m_pRLC->GetDatabase()->mRLSystems.Size() < 1)
		valid = false;
	else if(!m_pRLC->GetDatabase()->mRLSystems[0]->mReportingParameters)
		valid = false;
	else if(!m_pRLC->GetDatabase()->mRLSystems[0]->mHandOfGodParameters)
		valid = false;

	return valid;
}

};//namespace Actors
};//namespace Game

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
