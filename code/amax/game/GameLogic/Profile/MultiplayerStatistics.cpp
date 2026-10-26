#include "stdafx.h"

#include <GameLogic/Profile/MultiplayerStatistics.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic\Structures\LevelState.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <Game\Game.h>
#include <Game\dlc\DLC_Controller.h>
#include <GameLogic\GameModes\GameModeBase.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameApp.h>

extern Debug::Logger& g_GameProfileLogger;

namespace Game
{
	void MultiplayerStatistics::Init()
	{
		mNumEventsCompleted = mNumEventsWon = mNumEventsPlayed = mNumTimesWrecked = mNumTimesWreckedOtherDrivers = mNumLifetimeFans = 0;

		mTotalPlaytime = mTimeToAchieveLegend = 0;

		mNumTimesShield=mNumTimesShielded=mNumRacePoints=mNumTimesRepaired=0;

		mNumTimesInTopThree = 0;

		mDecoysDropped = mPlayersHitwithDecoys = 0;

		mCumulativePosition = mAveragePosition = 0.0f;

		mVehiclesUsed.Clear();
		mRoutesDriven.Clear();

		for( int i = 0; i < E_PICKUP_TYPE_MAX; i++ )
		{
			mPickupsFired[i] = 0;
			mPickupsHit[i] = 0;
		}

		for( int i = 0; i < mGameModesPlayed.Size(); i++ )
			mGameModesPlayed[i] = 0;

		for( int i = 0; i < AmaxCarClass_ANY; i++ )
			mCarClassesUsed[i] = 0;

		//Bug 24261
		CalculateBufferedStats();
	}

	void MultiplayerStatistics::PreRaceUpdate()
	{
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		// DLC fan multiplier cheats
		C_GameModeBase* pGameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
		if(pGameMode && pGameMode->IsOnline() && pGameMode->IsRanked())
		{
			levelState.mFanMultiplier *= UI::DLCController::Instance().GetFansMultiplier(this, levelState.mRaceMode);
			mGameModesPlayed[levelState.mRaceMode]++;
		}

		mCurrentPlaytime = Time::GetCurrentTime();

		mNumEventsPlayed++;

		Btl::InsertionResult< Btl::Map<XtId, int>::Iterator > insResult = mVehiclesUsed.Insert(levelState.mCars[0].mCarID, 1);
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

		AmaxCarClass carClass = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelState.mCars[0].mCarID]->carClass;

		if(carClass >= AmaxCarClass_A && carClass < AmaxCarClass_ANY)
			mCarClassesUsed[(int)carClass]++;
		else
			LOGWARN(g_GameProfileLogger, "Car class used not recorded as vehicle %s has bad car class %i", C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelState.mCars[0].mCarID]->tag.Text(),(int)carClass);
	}

	void MultiplayerStatistics::PostRaceUpdate( int fansToAdd )
	{
		MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

		int playerIdx = 0; // How do I get this for reals?

		const Game::C_LevelResult& results = C_Game::GetInstance()->GetLevelResult();
		const Game::LevelResultCar& levelCar = results.mCars[playerIdx];
		
		U64 playtime = Time::GetCurrentTime() - mCurrentPlaytime;
		mTotalPlaytime += playtime;

		BIZ_ASSERT_MSG_SPEED(fansToAdd >= 0, "MultiplayerStatistics - fansToAdd < 0, tell SteveB or WillB");

		if(!multiplayerConfig)
			BIZ_ASSERT_FAIL("Unable to find multiplayer config");

		for( int i = 0; i < levelCar.mStatistics.mNumPerksFired.Size(); i++ )
			mPickupsFired[i] += levelCar.mStatistics.mNumPerksFired[i];

		for( int i = 0; i < levelCar.mStatistics.mNumPerksHit.Size(); i++ )
			mPickupsHit[i] += levelCar.mStatistics.mNumPerksHit[i];

		mDistanceDriven += levelCar.mStatistics.mDistanceTravelled;

		mDecoysDropped += levelCar.mStatistics.mDecoysDropped;
		mPlayersHitwithDecoys += levelCar.mStatistics.mPlayersHitwithDecoys;
		
		//PK needs to be done
		mNumTimesShield=levelCar.mStatistics.mNumTimesShield;
		mNumTimesShielded=levelCar.mStatistics.mNumTimesShielded;
		mNumTimesRepaired=levelCar.mStatistics.mNumTimesRepaired;

		if(levelCar.mStatistics.mWrecked)
			mNumTimesWrecked++;

		mNumTimesWreckedOtherDrivers += levelCar.mStatistics.mNumVehiclesWrecked + levelCar.mStatistics.mNumRivalVehiclesWrecked;

		mNumLifetimeFans += fansToAdd;
		mNumLifetimeFans = Maths::Clamp(mNumLifetimeFans, 0, MOST_LIFETIME_FANS_EVAAAARRRR);

		if(levelCar.mPosition < 4)
		{
			mNumTimesInTopThree++;
		}

		mNumRacePoints += GetDriverRacePoints(levelCar.mPosition, results.mNumCars);

		mNumEventsCompleted++;

		if(levelCar.mPosition == 1)
		{
			mNumEventsWon++;
		}

		mCumulativePosition += (float)levelCar.mPosition / (float)results.mNumCars;
		if (mNumEventsCompleted==0)
		{
			mAveragePosition = 0.f;
		}
		else
		{
			mAveragePosition = mCumulativePosition / (float)mNumEventsCompleted;
		}

		CalculateBufferedStats();
	}

	//	Post race update we call in all network games (i.e. LAN, Private Game online and Ranked online)
	void MultiplayerStatistics::SessionStatsUpdate()
	{
		int playerIdx = 0; // How do I get this for reals?

		const Game::C_LevelResult& results = C_Game::GetInstance()->GetLevelResult();
		const Game::LevelResultCar& levelCar = results.mCars[playerIdx];
		
		mNumRacesThisSession++;

		if(levelCar.mPosition == 1)
		{
			mNumWinsThisSession++;
			mWinningStreak++;
		}
		else
			mWinningStreak = 0;

		mNumRacePointsThisSession += GetDriverRacePoints(levelCar.mPosition, results.mNumCars);
	}

	void MultiplayerStatistics::CalculateBufferedStats()
	{
		mTotalPickupsFired = 0;
		for( int i = 0; i < mPickupsFired.Size(); i++ )
		{
			if(i == E_PICKUP_TYPE_SHOCK || i == E_PICKUP_TYPE_SHUNT || i == E_PICKUP_TYPE_BARGE || i == E_PICKUP_TYPE_BLINDFIRE || i == E_PICKUP_TYPE_BOMB)
				mTotalPickupsFired += mPickupsFired[i];
		}
		//Bug#35097 - Add on the number decoys dropped
		mTotalPickupsFired += mDecoysDropped;


		mTotalPickupsHit = 0;
		for( int i = 0; i < mPickupsHit.Size(); i++ )
		{
			if(i == E_PICKUP_TYPE_SHOCK || i == E_PICKUP_TYPE_SHUNT || i == E_PICKUP_TYPE_BARGE || i == E_PICKUP_TYPE_BLINDFIRE || i == E_PICKUP_TYPE_BOMB)
				mTotalPickupsHit += mPickupsHit[i];
		}
		//Bug#35097 - Add on the number decoys hit
		mTotalPickupsHit += mPlayersHitwithDecoys;
	}

	void MultiplayerStatistics::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mNumLifetimeFans << mNumEventsPlayed << mNumEventsCompleted << mNumEventsWon << mNumRacePoints << mCumulativePosition << 
			mAveragePosition << mDistanceDriven << mNumTimesWrecked << mNumTimesWreckedOtherDrivers << 
			mNumTimesShield << mNumTimesShielded << mTotalPlaytime << mNumTimesRepaired << mNumTimesInTopThree << mTimeToAchieveLegend
			<< mDecoysDropped << mPlayersHitwithDecoys;

		stream << mGameModesPlayed.Size();
		for( int i = 0; i < mGameModesPlayed.Size(); i++ )
			stream << mGameModesPlayed[i];

		stream << mPickupsFired.Size();
		for( int i = 0; i < mPickupsFired.Size(); i++ )
			stream << mPickupsFired[i];

		stream << mPickupsHit.Size();
		for( int i = 0; i < mPickupsHit.Size(); i++ )
			stream << mPickupsHit[i];

		stream << mCarClassesUsed.Size();
		for( int i = 0; i < mCarClassesUsed.Size(); i++ )
			stream << (S32&)mCarClassesUsed[i];

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

	void MultiplayerStatistics::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mNumLifetimeFans >> mNumEventsPlayed >> mNumEventsCompleted >> mNumEventsWon >> mNumRacePoints >> mCumulativePosition >> 
			mAveragePosition >> mDistanceDriven >> mNumTimesWrecked >> mNumTimesWreckedOtherDrivers >>
			mNumTimesShield >> mNumTimesShielded >> mTotalPlaytime >> mNumTimesRepaired >> mNumTimesInTopThree >> mTimeToAchieveLegend
			>> mDecoysDropped >> mPlayersHitwithDecoys;

		int numGameModesPlayed;
		stream >> numGameModesPlayed;
		for( int i = 0; i < numGameModesPlayed; i++ )
		{
			int played;
			stream >> played;
			mGameModesPlayed[i] = played;
		}

		int numPickupsFired;
		stream >> numPickupsFired;
		for( int i = 0; i < numPickupsFired; i++ )
		{
			int firedNum;
			stream >> firedNum;
			mPickupsFired[i] = firedNum;
		}

		int numPickupsHit;
		stream >> numPickupsHit;
		for( int i = 0; i < numPickupsHit; i++ )
		{
			int firedNum;
			stream >> firedNum;
			mPickupsHit[i] = firedNum;
		}

		int numClassesUsed;
		stream >> numClassesUsed;
		for( int i = 0; i < numClassesUsed; i++ )
		{
			int carClass;
			stream >> carClass;
			mCarClassesUsed[i] = (AmaxCarClass)carClass;
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

		mWinningStreak = 0;

		CalculateBufferedStats();
	}

}
