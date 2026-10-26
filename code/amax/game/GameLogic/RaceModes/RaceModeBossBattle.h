#ifndef RACEMODE_BOSSBATTLE_H
#define RACEMODE_BOSSBATTLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

struct SP_BossEvent;

namespace Game
{

class C_RaceModeBossBattle : public C_RaceModeBase
{
public:
	void Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void SetupRace();
	void StartLevel();
	void StartRace();

	void Update();
	void UpdateHUD();
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

//	void StartSecondStage();
//	void RepositionOnStartPoint(S32 vehicle_index, S32 start_pos_index, F32 speed);
	void SetControlType(S32 vehicle_index, bool player_control);
//	F32  GetVehicleGap(S32 vehicle_index1, S32 vehicle_index2);

	SP_BossEvent* GetEvent();
//	bool IncRangeTimer(F32 maxtime);

	void EndEvent(bool won, bool hasWonByWreck = false);
	void SetupResults();

	bool HasWon()						{ return mWon; }
	bool HasWonByWreck()				{ return mHasWonByWreck; }

	SP_EventState GetEventState(){ return mEventState; }

private:
	SP_EventState mEventState;

/*
	bool								mFirstStage;			// True if fighting the minions
	S32									mNumberWrecked;			// How many minions have been wrecked

	S32									mIgnoreDistanceChecks;	// For when we're moving veicles around the level

	S32									mPlayerIndex;
	S32									mBossIndex;
	S32									mFirstMinderIndex;
	S32									mNumberMinders;

	F32									mTimeOutsideRange;

	F32									mDistanceFromMinders;			// From Event
	F32									mTimeInFrontOfMinders;
	bool								mWreckMindersToWin;

	F32									mDistanceFromBoss;
	F32									mTimeInFrontOfBoss;
	bool								mWreckBossToWin;
*/

	S32									mNumBosses;
	bool								mWon;
	bool								mHasWonByWreck;
};

} // namespace Game

#endif // RACEMODE_BOSSBATTLE_H
