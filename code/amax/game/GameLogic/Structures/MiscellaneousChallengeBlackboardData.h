#ifndef MISCELLANEOUS_CHALLENGE_BLACKBOARD_DATA_H
#define MISCELLANEOUS_CHALLENGE_BLACKBOARD_DATA_H

#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <Network/Misc/NetDefines.h>

namespace Game
{
	//NB/ Don't forget to change the profile version when you change the size of this structure
	class MiscellaneousChallengeBlackboardData
	{
	public:
		typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U8, Btl::VectorParameters_Array<1> > > > BitArray8;

		MiscellaneousChallengeBlackboardData() {Initialise();}

		void										Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );


		void										Initialise();
		void										InitForNextRace(); // Clear any temporary blackboard data here. 
		void										ClearVehiclesWreckedInCurrentRace();

		// Record progress of ultimate comeback status through an enum
		enum E_UltimateComebackStatus
		{	
			E_UltimateComeback_NotYetInFirstPlace = 0,
			E_UltimateComeback_HasBeenInFirstPlace,
			E_UltimateComeback_ThenFellToLastPlace,
		};

		enum CountriesToVisit{
			JAPAN	= 1,
			USA		= (1 << 1),
			UK		= (1 << 2),
			SPAIN	= (1 << 3),
		};

	public:
		U32											mDailyChallengeDayStamp;
		S32											mDailyChallengeTimeZoneAdjustStamp;

		//1 if true, 0 if false, for vehicle 1<<(vehicle index in C_GameVehicleData::Instance().mVehicles)
		U64											mVehiclesUsedToday; 

		
		float										mSecondsRemainderInFirstPosition;
		U8											mCountriesVisitedTodayBitField;
		
		// Temporary data (i.e. cleared when race starts)
		int											mNumCarsTakenOutThisRace; 
		int											mNumCarsInWreckStreak;	//Number of vehicles wrecked without me dying
		int											mNumRevengeAttacksThisRace;
		int											mNumCarsCrashedOutInChain;
		int											mCurrentComboNumber;
		U32											mHasRammedVehicle; //1 if true, 0 if false, for vehicle 1<<index

		int											mLastVehicleIndexHitByPerk;
		int											mNumSuccessivePerkHitsOnLastVehicleHit; 

		BitArray8									mPerksUsed;

		int											mTimesPlayerWrecked[RacerNet::NET_MAX_VEHICLES];

		int											mOvertakes;
		int											mNitrousOvertakes;
		int											mNumStartingPlayers;

		int											mNumVehiclesHitByOneBarge;

		float										mCurrentTimeTravelingAtXMPH;
		float										mLongestTimeTravelingAtXMPH;

		float										mQuickDrawStartRaceTime;
		int											mQuickDrawNumberOfVehiclesWrecked;

		float										mTimeSpentInAir;
		float										mCurrentTimeInFirstPosition;
		bool										mPreviouslyInFirstPosition;
		E_UltimateComebackStatus					mUltimateComebackStatus; 

		int											mSkillshotsPerformed;
		int											mSuccessfulPowerupHits;
		bool										mForwardPickupFireUsed; 
		bool										mReversePickupFireUsed; 

		typedef Btl::Pair< U32, float >				IDTimePair;
		Btl::Vector<IDTimePair>						mRecentlyFiredMineIDs; 

		bool										mLastGaspIsActive;
		int											mNumCarsHitWithLastGasp;
		int											mNumNitroJumps;
		int											mNumVehiclesHitWithLastGaspThisLife;
	};
};


#endif //MISCELLANEOUS_CHALLENGE_BLACKBOARD_DATA_H
