#ifndef RACEBOOK_PROFILE_STATISTICS_H
#define RACEBOOK_PROFILE_STATISTICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>

#include <GameLogic/GameData/GameVehicleData.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

namespace Sys
{
	class BinarySerialiserAutoEndian;
	class BinaryDeserialiserAutoEndian;
}

struct RaceBookCharacter;

namespace RaceBook
{

class C_RaceBookProfile;

struct RaceBookProfileDualStatistics
{
	RaceBookProfileDualStatistics()
	{
		mDriverRacedNum = mDriverBeatenNum = mDriverPerksNum = 0;
	}

	void Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

public:
	int mDriverBeatenNum;
	int mDriverPerksNum;
	int mDriverRacedNum;
};

class C_RaceBookProfileStatistics
{
public:
	void Initialise( C_RaceBookProfile* prof, RaceBookCharacter* character );

	void PreRaceUpdate();
	void PostRaceUpdate();
	void CalculateBufferedStats();

	void Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	float CalculateAccuracy(E_PICKUP_TYPE pickup)
	{
		if( mPickupsFired[pickup] != 0 )
			return ( ( mPickupsHit[pickup] / (float)mPickupsFired[pickup] ) * 100 );

		return 0;
	}

	int		mRacesAttempted;
	int		mRacesPassed;
	int		mChallengesUnlocked;
	float	mBankBalance;
	int		mNumFans;
	int		mXpPoints;
	float	mDistanceDriven;

	int		mTimesWrecked;
	int		mTimesWreckedOtherVehicle;
	float	mDistanceTravelled;

	int		mTimesRepaired;
	int		mTimesShielded;

	Btl::Map<U32, RaceBookProfileDualStatistics>						mDualStatistics;

	Btl::Map<XtId, int>													mVehiclesUsed;
	Btl::Map<XtId, int>													mRoutesDriven;

	Btl::Vector<int, Btl::VectorParameters_Static<E_PICKUP_TYPE_MAX> >	mPickupsFired;
	Btl::Vector<int, Btl::VectorParameters_Static<E_PICKUP_TYPE_MAX> >	mPickupsHit;

	C_RaceBookProfile* parent;

	// Stuff that we buffer but do not save.
	U32				mTotalPickupsFired;
	U32				mFavouriteTrack;
	U32				mFavouriteCar;
	U32				mShopItemsBought;
	AmaxCarClass	mFavouriteCarClass;
	E_PICKUP_TYPE	mFavouritePickup;
};

}

#endif //RACEBOOK_PROFILE_STATISTICS_H
