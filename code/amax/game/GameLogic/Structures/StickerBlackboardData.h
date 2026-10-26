#ifndef STICKER_BLACKBOARD_DATA_H
#define STICKER_BLACKBOARD_DATA_H

#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace Game
{
	//NB/ Don't forget to change the profile version when you change the size of this structure
	class C_StickerBlackboardData
	{
	public:
		C_StickerBlackboardData() {}

		void										Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );


		void										Initialise();
		void										InitForNextRace(); // Clear any temporary blackboard data here. 

	public:
		bool										mHasUnlockedNewStickers;

		bool										mInRace;
		U32											mCurrentCityId;
		U32											mVisitedCitiesBitset; // index into this as into the array containing all cities in sorcery

		U8											mNumVehiclesHitByOneBarge; //hit x cars at once
		U8											mNumVehiclesHitByOneAirbrake;

		bool										mShuntWasEvadedByDestroying; //evade shunt by driving

		U32											mVehiclesHitByShockBitset; //shock a guy twice

		float										mBargedSomeGuyOffCliff; // timer, self explanatory!

		U8											mVehicleIndexYouHitWithBolt; // for bolts
		U8											mNumTimesHitSameCarWithBolt;
		U8											mNumDifferentCarsHitWithSingleBolt;
		U32											mGroupIdOfLastBoltHit;
		U32											mNumBoltsFiredThisRace;
		U32											mNumBoltsHitThisRace;

		U8											mNumTimesAirbrakedInFirstPosition;
		bool										mIsAirbraking; // after the initial slow, but before the final kick
		bool										mIsUsingNitro;
		bool										mIsInNitroThroughTriggerBox;
		bool										mCanGetNitroTriggerBoxAchievement;
		bool										mHasUsedNitroThisRace;
		U8											mNumBoltsHitDuringThisAirbrake;

		U32											mOurRecentlyFiredMineId;
		float										mTimeSinceWeDroppedOurLastMine;

		bool										mHasBeenHitThisLap;
		bool										mHasBeenHitThisRace;
		U8											mPowerupTypesHitThisRace;

		U8											mAttacksShieldedThisShield;
		float										mShieldElapsedTime;
		float										mTimeSinceLastRepair;

		bool										mHasUsedAStopwatchThisRace;
		int											mDamageTakenThisRace;
		int											mDamageDoneThisRace;
		bool										mHasBeenWreckedThisRace;

		int											mAmountRepairedThisRace;
		bool										mCanGetSmokingOilfieldAchievement;
		bool										mIsInAmboyOilfield;

		U8											mFanDemandsCompleteThisEvent;

		U32											mNumAwardsLocalPlayerWonConsecutively;

		// SB - No need to serialise these.
		bool										mCrossedCheckpointWithoutNitrous;
		Btl::BitArray32								mVehiclesHitThisRace;
	};
};


#endif //STICKER_BLACKBOARD_DATA_H
