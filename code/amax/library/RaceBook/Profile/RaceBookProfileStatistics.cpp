#include "stdafx.h"

#include "RaceBookProfileStatistics.h"

#include <Game/Game.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Structures/LevelState.h>
#include <GameLogic/Structures/LevelResult.h>

#include "RaceBook/Profile/RaceBookProfile.h"

Debug::Logger &g_RaceBookStatisticsLogger( Debug::GetLogger("RaceBook.Statistics") );

namespace RaceBook
{

typedef Btl::Map<U32,RaceBookProfileDualStatistics>::Iterator DualStatsIterator;

void C_RaceBookProfileStatistics::Initialise(C_RaceBookProfile* prof, RaceBookCharacter* character)
{
	parent		= prof;

	mRacesAttempted = mRacesPassed = mChallengesUnlocked = mTimesWrecked = mShopItemsBought =
	mTimesWreckedOtherVehicle = mNumFans = mXpPoints = mTotalPickupsFired = mFavouriteTrack = mFavouriteCar = 0;

	mFavouriteCarClass = AmaxCarClass_ANY;
	mFavouritePickup = E_PICKUP_TYPE_NONE;

	mDistanceDriven = mBankBalance = mDistanceTravelled = 0.f;

	mTimesRepaired=0;
	mTimesShielded=0;

	mDualStatistics.Clear();

	mVehiclesUsed.Clear();
	mRoutesDriven.Clear();

	mPickupsFired.Clear();
	for( int i = 0; i < E_PICKUP_TYPE_MAX; i++ )
		mPickupsFired.PushBack(0);


	mPickupsHit.Clear();
	for( int i = 0; i < E_PICKUP_TYPE_MAX; i++ )
		mPickupsHit.PushBack(0);
}

void C_RaceBookProfileStatistics::PreRaceUpdate()
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(!parent)
	{
		LOGWARN(g_RaceBookStatisticsLogger, "Parent undefined in RaceBookProfileStatistics - no statistics will be recorded for this profile!\n");
		return;
	}

	mRacesAttempted++;

	int currentCarIdx = parent->mCurrentCarIdx;	
	Btl::InsertionResult< Btl::Map<XtId, int>::Iterator > insResult = mVehiclesUsed.Insert(levelState.mCars[currentCarIdx].mCarID, 1);
	if( !insResult.inserted )
	{
		Btl::Map<XtId, int>::Iterator it = insResult.iterator;
		it->GetValue()++;
	}

	insResult = mRoutesDriven.Insert(levelState.mRoute, 1);
	if( !insResult.inserted )
	{
		Btl::Map<XtId, int>::Iterator it = insResult.iterator;
		it->GetValue()++;
	}
}

void C_RaceBookProfileStatistics::PostRaceUpdate()
{
	Game::C_LevelResult& results	= Game::C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state		= Game::C_Game::GetInstance()->GetLevelState();

	if(!parent)
	{
		LOGWARN(g_RaceBookStatisticsLogger, "Parent undefined in RaceBookProfileStatistics - no statistics will be recorded for this profile!\n");
		return;
	}

	int currentCarIdx = parent->mCurrentCarIdx;
	
	Game::LevelResultCar* car = &results.mCars[currentCarIdx];
	VehicleRaceStatistics* stats = &car->mStatistics;

	mBankBalance = parent->mBankBalance;
	mNumFans = parent->mNumFans;
	mXpPoints += car->mScore;
	mDistanceDriven += car->mStatistics.mDistanceTravelled;

	mTimesShielded = results.mCars[currentCarIdx].mStatistics.mNumTimesShielded;
	mTimesRepaired = results.mCars[currentCarIdx].mStatistics.mNumTimesRepaired;

	if( car->mStatistics.mWrecked )
		mTimesWrecked++;

	// If it's not zero, we haven't set our stats properly.
	if( !results.mCars[currentCarIdx].mStatistics.mNumPerksFired.Size() )
		LOGWARN(g_RaceBookStatisticsLogger, "RaceBookProfileStatistics have not been set properly!\n");

	S32 last = 0;
	mTotalPickupsFired = 0;
	for( int i = 0; i < results.mCars[currentCarIdx].mStatistics.mNumPerksFired.Size(); i++ )
	{
		mPickupsFired[i] += results.mCars[currentCarIdx].mStatistics.mNumPerksFired[i];
		mTotalPickupsFired += mPickupsFired[i];

		// calculate favourite pickup
		if ( mPickupsFired[i] > last )
		{
			last = mPickupsFired[i];
			mFavouritePickup = (E_PICKUP_TYPE)i;
		}
	}

	// Calculate favourite car and car class
	S32 max_car_usage = 0;
	for ( S32 i = 0; i < C_RaceBookData::Instance().mVehicles.Number(); i++ )
	{
		if ( !C_RaceBookData::Instance().mVehicles[i]->vehicle )
		{
			LOGWARN(g_RaceBookStatisticsLogger, "RaceBookVehicle has invalid AmaxVehicle ptr!\n");
			continue;
		}

		Btl::Map<XtId, int>::Iterator it = mVehiclesUsed.Find(C_RaceBookData::Instance().mVehicles[i]->vehicle->ID);

		if(it != mVehiclesUsed.End())
		{
			S32 amount = it->GetValue();

			if ( amount > max_car_usage )
			{
				max_car_usage = amount;
				mFavouriteCar = it->GetKey();
				mFavouriteCarClass = C_RaceBookData::Instance().mVehicles[i]->vehicle->carClass;
			}
		}
	}

	// Calculate favourite track
	S32 max_track_driven = 0;
	for ( S32 i = 0; i < C_GameData::Instance().mRoutes.Number(); i++ )
	{
		Btl::Map<XtId, int>::Iterator it = mRoutesDriven.Find(C_RaceBookData::Instance().mVehicles[i]->vehicle->ID);

		if(it != mRoutesDriven.End())
		{
			S32 amount = it->GetValue();

			if ( amount > max_car_usage )
			{
				max_track_driven = amount;
				mFavouriteTrack = it->GetKey();
			}
		}
	}

	if( !results.mCars[currentCarIdx].mStatistics.mNumPerksHit.Size() )
		LOGWARN(g_RaceBookStatisticsLogger, "RaceBookProfileStatistics have not been set properly!\n");
	
	for( int i = 0; i < E_PICKUP_TYPE_MAX; i++ )
		mPickupsHit[i] += results.mCars[currentCarIdx].mStatistics.mNumPerksHit[i];		

	for( unsigned int b = 0; b < state.mNumCars; b++ )
	{
		if(results.mCars[currentCarIdx].mStatistics.mDualVehicleStats[b].mTookOutDriver)
			mTimesWreckedOtherVehicle++;

		// Try adding me to the map.
		Btl::InsertionResult<DualStatsIterator> insResult	= mDualStatistics.Insert(state.mCars[b].mCharacterID);
		DualStatsIterator dualStatsIt						= insResult.iterator;

		dualStatsIt->GetValue().mDriverRacedNum++;
		dualStatsIt->GetValue().mDriverPerksNum += results.mCars[currentCarIdx].mStatistics.mDualVehicleStats[b].mNumberOfTimesHitWithAggressivePerk;

		if( results.mCars[currentCarIdx].mPosition < results.mCars[b].mPosition )
			dualStatsIt->GetValue().mDriverBeatenNum++;
	}
}

void C_RaceBookProfileStatistics::CalculateBufferedStats()
{
	mTotalPickupsFired = 0;
	for( int i = 0; i < mPickupsFired.Size(); i++ )
		mTotalPickupsFired += mPickupsFired[i];
}

void C_RaceBookProfileStatistics::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	stream <<	mRacesAttempted << mRacesPassed << mChallengesUnlocked << mTimesWrecked <<
				mTimesWreckedOtherVehicle << mBankBalance << mNumFans << mXpPoints << mDistanceDriven << mTimesShielded << mTimesRepaired;
																										 
	stream << mDualStatistics.Size();
	for( DualStatsIterator it = mDualStatistics.Begin(); it != mDualStatistics.End() ; it++ )
	{
		stream << it->GetKey() << it->GetValue();
	}

	stream << mPickupsFired.Size();
	for( int i = 0; i < mPickupsFired.Size(); i++ )
	{
		stream << mPickupsFired[i];
	}

	stream << mPickupsHit.Size();
	for( int i = 0; i < mPickupsHit.Size(); i++ )
	{
		stream << mPickupsHit[i];
	}

	stream << mVehiclesUsed.Size();
	for( Btl::Map<XtId, int>::Iterator it = mVehiclesUsed.Begin(); it != mVehiclesUsed.End(); it++ )
	{
		stream << it->GetKey();
		stream << it->GetValue();
	}

	stream << mRoutesDriven.Size();
	for( Btl::Map<XtId, int>::Iterator it = mRoutesDriven.Begin(); it != mRoutesDriven.End(); it++ )
	{
		stream << it->GetKey();
		stream << it->GetValue();
	}
}

void C_RaceBookProfileStatistics::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	mDualStatistics.Clear();

	stream >>	mRacesAttempted >> mRacesPassed >> mChallengesUnlocked >> mTimesWrecked >>
				mTimesWreckedOtherVehicle >> mBankBalance >> mNumFans >> mXpPoints >> mDistanceDriven >> mTimesShielded >> mTimesRepaired;

	int numDualStats;
	stream >> numDualStats;
	mDualStatistics.Clear();
	for( int a = 0; a < numDualStats; a++ )
	{
		U32 key;
		RaceBookProfileDualStatistics stats;

		stream >> key >> stats;
		mDualStatistics.Insert(key, stats);
	}

	int numPickupsFired;
	stream >> numPickupsFired;
	mPickupsFired.Clear();
	for( int i = 0; i < numPickupsFired; i++ )
	{
		int firedNum;
		stream >> firedNum;

		if (i < E_PICKUP_TYPE_MAX)
			mPickupsFired.PushBack(firedNum);
	}

	int numPickupsHit;
	stream >> numPickupsHit;
	mPickupsHit.Clear();
	for( int i = 0; i < numPickupsHit; i++ )
	{
		int hitNum;
		stream >> hitNum;

		if (i < E_PICKUP_TYPE_MAX)
			mPickupsHit.PushBack(hitNum);
	}

	XtId key;
	int val;

	int numVehicles;
	stream >> numVehicles;
	for( int i = 0; i < numVehicles; i++ )
	{
		stream >> key;
		stream >> val;
		mVehiclesUsed.Insert(key, val);
	}

	int numRoutes;
	stream >> numRoutes;
	for( int i = 0; i < numRoutes; i++ )
	{
		stream >> key;
		stream >> val;
		mRoutesDriven.Insert(key, val);
	}
}

// ------------------------------------------------------------------------------------

void RaceBookProfileDualStatistics::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	stream << mDriverBeatenNum << mDriverPerksNum << mDriverRacedNum;
}

void RaceBookProfileDualStatistics::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	stream >> mDriverBeatenNum >> mDriverPerksNum >> mDriverRacedNum;
}

}
