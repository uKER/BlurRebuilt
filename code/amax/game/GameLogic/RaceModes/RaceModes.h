#ifndef RACEMODES_H
#define RACEMODES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeBase.h>

#include <GameLogic/RaceModes/RaceModeStreetRace.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerRacing.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerElimination.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerPoints.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerDestruction.h>

#include <GameLogic/RaceModes/RaceModeDestruction.h>
#include <GameLogic/RaceModes/RaceModeCheckpoint.h>
#include <GameLogic/RaceModes/RaceModeFanDemand.h>
#include <GameLogic/RaceModes/RaceModeBossBattle.h>

#include <GameLogic/RaceModes/RaceModeFriendDemandStreetRace.h>
#include <GameLogic/RaceModes/RaceModeFriendDemandBossBattle.h>
#include <GameLogic/RaceModes/RaceModeFriendDemandCheckpoint.h>
#include <GameLogic/RaceModes/RaceModeFriendDemandDestruction.h>
#include <GameLogic/RaceModes/RaceModeFriendDemandFanDemand.h>



namespace Game
{
	namespace Actors
	{
		class RaceLogicCore;
	}
	class C_RaceLogicLayer;


class C_RaceModeStore
{

public:
	C_RaceModeStore()
	{
		//Make an array of them (in E_RaceMode order)
		mRaceModes.PushBack(&mRaceModeNothing);
		mRaceModes.PushBack(&mRaceModeStreetRace);

		mRaceModes.PushBack(&mRaceModeMultiplayerRacing);
		mRaceModes.PushBack(&mRaceModeMultiplayerElimination);
		mRaceModes.PushBack(&mRaceModeMultiplayerPoints);
		mRaceModes.PushBack(&mRaceModeMultiplayerDestruction);

		mRaceModes.PushBack(&mRaceModeDestruction);
		mRaceModes.PushBack(&mRaceModeCheckpoint);
		mRaceModes.PushBack(&mRaceModeFanDemand);
		mRaceModes.PushBack(&mRaceModeBossBattle);

		mRaceModes.PushBack(&mRaceModeFriendDemandStreetRace);
		mRaceModes.PushBack(&mRaceModeFriendDemandDestruction);
		mRaceModes.PushBack(&mRaceModeFriendDemandCheckpoint);
		mRaceModes.PushBack(&mRaceModeFriendDemandFanDemand);
		mRaceModes.PushBack(&mRaceModeFriendDemandBossBattle);
	};

	inline C_RaceModeBase* operator() (E_RaceMode rm)
	{
		return mRaceModes[rm];
	}

	void CachePointerToTheRLC(Game::Actors::RaceLogicCore *pRLC = NULL)
	{
		for(int a=0;a<mRaceModes.Size();a++)
		{
			mRaceModes[a]->SetRLC(pRLC);
		}
	}

	void CachePointerToTheRaceLogicLayer(Game::C_RaceLogicLayer *pRaceLogicLayer = NULL)
	{
		for(int a=0;a<mRaceModes.Size();a++)
		{
			mRaceModes[a]->SetRaceLogicLayer(pRaceLogicLayer);
		}
	}

private:
	Ut::Vector<C_RaceModeBase*> mRaceModes;

	//Create all the race modes
	C_RaceModeBase						mRaceModeNothing;
	C_RaceModeStreetRace				mRaceModeStreetRace;
	C_RaceModeMultiplayer_Racing		mRaceModeMultiplayerRacing;
	C_RaceModeMultiplayer_Elimination	mRaceModeMultiplayerElimination;
	C_RaceModeMultiplayer_Points		mRaceModeMultiplayerPoints;
	C_RaceModeMultiplayer_Destruction	mRaceModeMultiplayerDestruction;

	C_RaceModeDestruction				mRaceModeDestruction;
	C_RaceModeCheckpoint				mRaceModeCheckpoint;
	C_RaceModeFanDemand					mRaceModeFanDemand;
	C_RaceModeBossBattle				mRaceModeBossBattle;

	C_RaceModeFriendDemandStreetRace	mRaceModeFriendDemandStreetRace;
	C_RaceModeFriendDemandDestruction	mRaceModeFriendDemandDestruction;
	C_RaceModeFriendDemandCheckpoint	mRaceModeFriendDemandCheckpoint;
	C_RaceModeFriendDemandFanDemand		mRaceModeFriendDemandFanDemand;
	C_RaceModeFriendDemandBossBattle	mRaceModeFriendDemandBossBattle;
};

extern C_RaceModeStore gRaceModeStore;

} // namespace Game

#endif // RACEMODES_H
