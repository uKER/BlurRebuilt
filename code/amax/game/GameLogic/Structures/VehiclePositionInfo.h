#ifndef VEHICLE_POSITION_INFO_H
#define VEHICLE_POSITION_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

struct VehiclePositionInfo
{	
	S8				mRacePosition;
	S8				mPreviousRacePosition;
	S8				mPreviousSpRacePosition;
	S8				mSpRacePosition;	//	We place the race position that's calculated by the AI stuff in here, and then if we're in single player the VLA puts it into mRacePosition
	
	float			mLogicPosition;
	float			mPrevLogicPosition;
	
	float			mLogicPositionCombined;
	float			mDistanceFromStart;
	bool			mGoingWrongWay;
	bool			mInRangeOfHuman;

	float			mForwardSpeed;
	float			mSpeedInMph;
	Maths::HVec3	mWorldPosition;
	Maths::HVec3	mDirection;

	S32				mCurrentRaceRuleArea;
	S32				mLastRaceRuleArea;

	float			mDistanceTravelledSinceLastUpdate;
	float			mHeightTravelledSinceLastUpdate;
};

#endif //VEHICLE_POSITION_INFO_H