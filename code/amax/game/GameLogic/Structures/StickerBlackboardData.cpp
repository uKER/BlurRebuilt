#include "stdafx.h"

#include "StickerBlackboardData.h"

namespace Game
{
void C_StickerBlackboardData::Initialise()
{
	InitForNextRace();
	mCurrentCityId = 0;
	mVisitedCitiesBitset = 0;
	mInRace = false;
	mNumAwardsLocalPlayerWonConsecutively = 0;
	mHasUnlockedNewStickers = false;
}

void C_StickerBlackboardData::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	U8 hasUnlockedStickers = (U8)mHasUnlockedNewStickers;
	out << mVisitedCitiesBitset << hasUnlockedStickers;
}

void C_StickerBlackboardData::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	U8 hasUnlockedStickers = 0;
	in >> mVisitedCitiesBitset >> hasUnlockedStickers;
	mHasUnlockedNewStickers = hasUnlockedStickers != 0;
}

void C_StickerBlackboardData::InitForNextRace()
{
	// here you zero some data you want to be cleared race by race
	mNumVehiclesHitByOneBarge = 0;
	mNumVehiclesHitByOneAirbrake = 0;
	mShuntWasEvadedByDestroying = 0;
	mVehiclesHitByShockBitset = 0;
	mBargedSomeGuyOffCliff = 0;
	mVehicleIndexYouHitWithBolt = 0;
	mNumTimesHitSameCarWithBolt = 0;
	mGroupIdOfLastBoltHit = 0;
	mNumBoltsFiredThisRace = 0;
	mNumBoltsHitThisRace = 0;
	mNumDifferentCarsHitWithSingleBolt = 0;
	mNumTimesAirbrakedInFirstPosition = 0;
	mNumBoltsHitDuringThisAirbrake = 0;
	mIsAirbraking = false;
	mIsUsingNitro = false;
	mIsInNitroThroughTriggerBox = false;
	mCanGetNitroTriggerBoxAchievement = false;
	mTimeSinceWeDroppedOurLastMine = 0;
	mOurRecentlyFiredMineId = 0;
	mHasBeenHitThisLap = false;
	mHasBeenHitThisRace = false;
	mPowerupTypesHitThisRace = 0;
	mAttacksShieldedThisShield = 0;
	mHasUsedAStopwatchThisRace = false;
	mHasUsedNitroThisRace = false;
	mDamageTakenThisRace = 0;
	mDamageDoneThisRace = 0;
	mHasBeenWreckedThisRace = false;
	mAmountRepairedThisRace = 0;
	mCanGetSmokingOilfieldAchievement = false;
	mIsInAmboyOilfield = false;
	mFanDemandsCompleteThisEvent = 0;

	mCrossedCheckpointWithoutNitrous = false;
	mVehiclesHitThisRace.ClearBits();
}

}; //namespace Game
