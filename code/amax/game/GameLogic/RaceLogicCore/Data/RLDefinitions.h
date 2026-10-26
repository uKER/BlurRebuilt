#ifndef RLDEFINITIONS_H
#define RLDEFINITIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <actor_system/ActorDeclare.h>

#include "../../Structures/LevelStateConstants.h"
#include "RLEnums.h"
#include "RLPickupDefinitions.h"

namespace SpecialFX
{
	class I_SpecialFXInstance;
}

static const int								MAX_VEHICLES					= 20;
static const int								MAX_NUM_ACTORS					= 10000;
static const int								MAX_NUM_TRIGGERS				= 50;			// Actually how many that can be deleted in one frame
const static int								MAX_SPLITS						= 16;
const static int								MAX_STORED_LAPS					= 10;
static const int								MAX_START_POSITIONS				= 100;
static const int								MAX_PICKUP_POSITIONS			= 500;
const float										LAP_END_DISPLAY_TIME			= 4.f;
const float										SPLIT_DISPLAY_TIME				= 4.f;
const static U8									INVALID_VALUE					= 99;
const static U8									INVALID_RACE_POSITION			= 99;

static const int								MAX_VIEWPORTS					= 4;
static const int								MAX_PICKUP_SLOTS				= 5;
static const int								MAX_DAMAGE_EVENTS				= 5;
static const int								MAX_DEBUG_LINES					= 1000;
static const int								MAX_CONCURRENT_PILEUPS			= 3;
static const int								MAX_VEHICLE_SFX					= 10;


//FORWARD DECLARATION
struct RLPickupType;

//Store info on the split information
struct SplitData
{
	void										Initialise()
	{
		mStart = mEnd = mCentre = mTrueStart = mTrueEnd =Maths::HVec3();
		mSplitNumber = mWaypoint = 0;
		mTrackPercentage = mAccurateWaypoint = 0.f;
		mSorted = false;
	}

	Maths::HVec3								mStart, 
												mEnd,
												mCentre,

												mTrueStart, 
												mTrueEnd;
	
	int											mSplitNumber,
												mWaypoint;
	float										mTrackPercentage,
												mAccurateWaypoint;

	bool										mSorted : 1;
};

typedef Btl::Vector<SplitData, Btl::VectorParameters_Static<MAX_SPLITS> > SplitArray;

struct SplitTimeData
{
	void Initialise()
	{
		mRaceTime = mSplitTime = mLapTime = 0.f;
		mLapNo = mRaceSplitNumber = mSplitIndex = mRacePosition = -1;
	}

	float										mRaceTime, 
												mSplitTime,
												mLapTime;

	int											mLapNo,
												mRacePosition,
												mSplitIndex,				//Defined as the split index that starts the split, so for 0 -> 1 it'd be 0
												mRaceSplitNumber;
};

typedef Btl::Vector<SplitTimeData, Btl::VectorParameters_Static<MAX_SPLITS> > SplitTimeArray;

struct LapTimeData
{
	void Initialise()
	{
		mSplitTimeData.Clear();
		mCurrentSplit.Initialise();

		mLapNo = -1;
		mLapTime = 0.f;
	}

	SplitTimeArray								mSplitTimeData;			//Stores the split times for this lap
	SplitTimeData								mCurrentSplit;			//The current split we're on

	float										mLapTime,				//Time in seconds... ie 1 sec = 1.0f
												mRaceTime;					
	int											mLapNo;
};

typedef Btl::Vector<LapTimeData, Btl::VectorParameters_Static<MAX_STORED_LAPS> > LapTimeArray;

// ------------------------------------------------------------

struct RacePositionBase
{
	static bool SortBy(RacePositionBase* a, RacePositionBase* b) { return *a < *b; };

	virtual bool operator < (const RacePositionBase& other) { return mVehicleId < other.mVehicleId; };

	U8 mVehicleId;
};

struct RacePosition_ByDistance : RacePositionBase
{
	RacePosition_ByDistance(U8 v, float d) { mVehicleId = v; mDistanceFromStartLine = d; };

	bool operator < (const RacePositionBase& other) { return mDistanceFromStartLine > ((RacePosition_ByDistance&)other).mDistanceFromStartLine; };

	float mDistanceFromStartLine;
};

struct RacePosition_ByDamage : RacePositionBase
{
	RacePosition_ByDamage(U8 v, float d) { mVehicleId = v; mDamage = d; };

	bool operator < (const RacePosition_ByDamage& other) { return( mDamage < other.mDamage ); };

	float mDamage;
};

// ------------------------------------------------------------
	
//
struct VehicleProximity
{
	float										mDistanceSquared; 
	float										mRelativeHeading;
};

typedef Btl::Vector<VehicleProximity, Btl::VectorParameters_Array<MAX_VEHICLES> > VehicleProximities;

class RLViewportDetails
{
public:
	RLViewportDetails()
	{
		mVehicleIndex = INVALID_VALUE;			//No vehicle
		mControllerIndex = Game::INVALID_CONTROLLER;	//No controller
	}

	S8											mVehicleIndex;
	S8											mControllerIndex;
};

typedef Btl::Vector<int, Btl::VectorParameters_Static<MAX_VEHICLES> > LappedCounter;



struct RLDebugLine
{
	void Initialise()
	{
		meType = E_DEBUG_LINE_TYPE_NONE;
		mStart = mEnd = Maths::HVec3();
		mColour = Maths::RGBA(0xFFFFFFFF);
	}

	E_DEBUG_LINE_TYPE							meType;
	Maths::HVec3								mStart, 
												mEnd;
	Maths::RGBA									mColour;
};

struct RLStartPosition
{
	void										Initialise()
	{
		mStartNumber = -1;
		mWorldMatrix = Maths::HMat43();
		mWorldPosition = Maths::HVec3();
		meType = E_STARTPOSITION_TYPE_STANDARD;
	}

	S8											mStartNumber;
	Maths::HMat43								mWorldMatrix;
	Maths::HVec3								mWorldPosition;
	enum E_STARTPOSITION_TYPE					meType;
};

struct RLTarget
{
	void Initialise() 
	{
		mVehicleIndex = -1;
		mCurrentlyTargeted = mAlreadyTargeted = mFirstTargetedFrame = mScored = false;
		mTimeTargeted = 0.f;
		mDistanceSquared = mDotRelativeHeading = 10000.f;

	}

	int											mVehicleIndex;
	bool										mCurrentlyTargeted,				//Currently targeted
												mAlreadyTargeted,				//Targeted before now
												mFirstTargetedFrame,			//First frame
												mScored;						//Scored
	float										mTimeTargeted, 
												mDistanceSquared,
												mDotRelativeHeading;
};

typedef Btl::Vector<RLTarget, Btl::VectorParameters_Array<Game::MAX_LEVELSTATE_CARS> > RLPickupPodTargets;

struct RLPickupPod
{
	void Initialise()
	{
		mFiredByVehicleIndex = -1;
		mWorldMatrix = Maths::HMat43();

		meType = E_PICKUP_TYPE_NONE;
		//meUsageType = E_PICKUP_USAGE_TYPE_NONE;
		meAIType = E_PICKUP_AI_TYPE_NONE;
		meTargetZoneType = E_PICKUP_TARGET_ZONE_TYPE_NONE;

		//mPickupLevel.Initialise();
		m_pPickupPodSpecialFX = 0;

		mPercentProgress = mTimeLeft = mTotalTime = 0.f;
		
		mUsing = mSingleFrame = false;
		mSendNetworkUsing = false;
		mTargetFiringVehicle = false;
		mBackwardShot = false;

		mTargetZoneRadius = mTargetZoneAngleInDegrees = mMaxTargetZoneRadius = 0.f;
		mMaxTargetsAllowed = mNoTargetsHit = mNoAttacksBlocked = 0;

		for(int vehicleIndex = 0; vehicleIndex < Game::MAX_LEVELSTATE_CARS; vehicleIndex++)
			mTargetedVehicles[vehicleIndex].Initialise();
	}

	int											mFiredByVehicleIndex,

												mNoTargetsHit,
												mNoAttacksBlocked,
												mMaxTargetsAllowed;

	Maths::HMat43								mWorldMatrix;

	enum E_PICKUP_TYPE							meType;
	//enum E_PICKUP_USAGE_TYPE					meUsageType;
	enum E_PICKUP_AI_TYPE						meAIType;
	enum E_PICKUP_TARGET_ZONE_TYPE				meTargetZoneType;

	//RLPickupLevel								mPickupLevel;
	SpecialFX::I_SpecialFXInstance				*m_pPickupPodSpecialFX;
	
	float										mPercentProgress,				//Used to time all the effects

												mTargetZoneRadius,
												mTargetZoneAngleInDegrees,
												mMaxTargetZoneRadius,
												
												mTimeLeft,
												mTotalTime;

	bool										mUsing,
												mSingleFrame,
												mTargetFiringVehicle,
												mSendNetworkUsing,

												mBackwardShot;

	RLPickupPodTargets							mTargetedVehicles;
	
};

typedef Btl::Vector<RLPickupPod, Btl::VectorParameters_Array<MAX_PICKUP_SLOTS> > PodArray;

struct RLDamageEvent
{
	void Initialise()
	{
		mEventTime = 0.f;
		meImpactLevel = E_IMPACT_LEVEL_NONE;
		mOtherVehicleIndex = -1;
	}

	float										mEventTime;
	enum E_IMPACT_LEVEL							meImpactLevel;
	int											mOtherVehicleIndex;
};

typedef Btl::Vector<RLDamageEvent, Btl::VectorParameters_Static<MAX_DAMAGE_EVENTS> > DamageEventArray;

struct RLPileUpData
{
	void Initialise()
	{
		mPileUp = false;
		mPileUpCentrePosition = Maths::HVec3();
		mPileUpTimer = 0.f;
		mPileUpVehicleIndices.Clear();
	}

	bool										mPileUp;
	float										mPileUpTimer;
	Maths::HVec3								mPileUpCentrePosition;

	Btl::Vector<int, Btl::VectorParameters_Static<MAX_VEHICLES> > mPileUpVehicleIndices;
};

typedef Btl::Vector<RLPileUpData, Btl::VectorParameters_Static<MAX_CONCURRENT_PILEUPS> > PileUpArray;

struct RLPickupPosition
{
	void Initialise()
	{
		mId = -1;
		meType = E_PICKUP_TYPE_NONE;
		mWorldPosition = Maths::HVec3();
	}

	int											mId;
	enum E_PICKUP_TYPE							meType;
	Maths::HVec3								mWorldPosition;
	Actor::C_ActorHandle						mActorHandle;					//Handle to the actor
};

typedef Btl::Vector<RLPickupPosition, Btl::VectorParameters_Static<MAX_PICKUP_POSITIONS> > PickupPositionArray;


#endif //RLDEFINITIONS_H
