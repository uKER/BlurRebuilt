#ifndef GAME_MULTIPLAYER_STATISTICS_H
#define GAME_MULTIPLAYER_STATISTICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <Xt/XtGame.h>

#include <GameLogic\GameData\GameVehicleData.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\RaceModes\RaceModeDefines.h>

namespace Game
{
	struct MultiplayerStatistics
	{
		static const int MOST_LIFETIME_FANS_EVAAAARRRR = 2000000000;

		void	Init();


		void	PreRaceUpdate();
		void	PostRaceUpdate(int fansToAdd);
		void	SessionStatsUpdate();
		void	CalculateBufferedStats();

		void	Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void	Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		int		mNumLifetimeFans;
		int		mNumEventsPlayed;

		Btl::Vector<int, Btl::VectorParameters_Array<E_RaceMode_MAX> > mGameModesPlayed; 
		
		int		mNumEventsWon;
		float	mDistanceDriven;
		U64		mTotalPlaytime;
		U64		mTimeToAchieveLegend;

		int		mNumTimesShield;
		int		mNumTimesShielded;

		int		mNumTimesRepaired;

		int		mNumTimesInTopThree;

		//	Cumulative race points value (awarded at end of each race based on finish position)
		unsigned int mNumRacePoints;

		Btl::Map<XtId, int>													mVehiclesUsed;
		Btl::Map<XtId, int>													mRoutesDriven;

		Btl::Vector<int, Btl::VectorParameters_Array<AmaxCarClass_ANY> >	mCarClassesUsed; 
		Btl::Vector<int, Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> >	mPickupsFired;
		Btl::Vector<int, Btl::VectorParameters_Array<E_PICKUP_TYPE_MAX> >	mPickupsHit;

		int		mDecoysDropped;								//Decoy drop
		int		mPlayersHitwithDecoys;						//Decoy drop, number of decoys dropped by the player and hit by some n00b

		// Stuff that we buffer but do not save.
		int		mTotalPickupsFired;
		int		mTotalPickupsHit;
		int		mNumChallengesCompletedThisRace;
		int		mNumFansEarnedFromChallengesThisRace;
		U32		mNumRacePointsThisSession;		//	Reset when we join/create a multiplayer lobby.
		U8		mNumRacesThisSession;			//	Reset when we join/create a multiplayer lobby.
		U8		mNumWinsThisSession;			//	Reset when we join/create a multiplayer lobby.
		U8		mWinningStreak;

		// Useful data we use to calculate stats but never directly display
		float	mCumulativePosition;
		U64		mCurrentPlaytime;

		//	Stuff below here is calculated/save but NOT actually ever used in the game/UI/anything so could be considered
		//	for deletion if we need to cut down the size of the save file
		int		mNumEventsCompleted;
		float	mAveragePosition;
		int		mNumTimesWrecked;
		int		mNumTimesWreckedOtherDrivers;



	};
}

#endif //GAME_MULTIPLAYER_STATISTICS_H