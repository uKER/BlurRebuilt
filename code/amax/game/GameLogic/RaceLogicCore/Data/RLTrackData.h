#ifndef RLTRACKDATA_H
#define RLTRACKDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include "RLDefinitions.h"

namespace airacingtech
{
	class AITrackDataRoutes;
};

class RLTrackData
{
//Functions
public:
	RLTrackData(){Initialise();}
	~RLTrackData(){};
	
	void										Initialise()
	{
		mRaceStarted = mRaceFinished = false;
		mSplitData.Clear();
		mStartPositions.Clear();
		mPickupPositions.Clear();

		mNumWaypoints = mTrackLength = -1;
		mStartFinishPosition = Maths::HVec3();
		mStartFinishAccurateWaypoint = mWaypointSize = -1.f;

		mAIDataAnalysed = mAIDataReceived = 
		mSplitsAnalysed = mSplitsReceived = false;

		// Seems obsolete, warn about things that don't matter
		mWarnAboutMissingStartPositions = false;

		//TODO Move this stuff to a better place, 
		mWarnAboutMissingVehicles = mWarnAboutMissingScoringData = mWarnAboutMissingPickupData = 
		mWarnAboutMissingDamageData = true;

		m_pLogicRoute = 0;

		mPileUpData.Clear();
		mPickupsEnabled = false;
	};

	Btl::Vector<RLStartPosition, Btl::VectorParameters_Static<MAX_START_POSITIONS> > mStartPositions;
	PickupPositionArray							mPickupPositions;

	SplitArray									mSplitData;

	Maths::HVec3								mStartFinishPosition;
	float										mStartFinishAccurateWaypoint;

	S32											mNumWaypoints,
												mTrackLength;												

	float										mWaypointSize;

	bool										mAIDataReceived,
												mAIDataAnalysed,

												mSplitsReceived,
												mSplitsAnalysed,

												mRaceStarted,
												mRaceFinished,

												mPickupsEnabled,

												mWarnAboutMissingVehicles,
												mWarnAboutMissingStartPositions,
												mWarnAboutMissingScoringData, 
												mWarnAboutMissingPickupData,
												mWarnAboutMissingDamageData;

	airacingtech::AITrackDataRoutes				*m_pLogicRoute;

	PileUpArray									mPileUpData;
};

#endif //RLTRACKDATA_H
