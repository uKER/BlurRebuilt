#ifndef RACEMODE_CHECKPOINT_H
#define RACEMODE_CHECKPOINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

struct SP_CheckpointEvent;

namespace Game
{

class C_RaceModeCheckpoint : public C_RaceModeBase
{
public:
	void Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void InitialiseVars();
	void SetupRace();
	void Update();
	void StartLevel();
	void StartRace();
	void UpdateHUD();
	void SetupFakeResults(); // For UI testing
	void SetupResults();
	void SetupQuitAndWinResults(lua_State *pState);

	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	bool ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel);
	bool ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel);

	//temp until we have a proper gadget
	const UI::C_WideString512 GetScoreString();
	const UI::C_WideString512 GetRulePassedString(U32 idx);
	const UI::C_WideString512 GetRuleFailedString(U32 idx);
	
	SP_CheckpointEvent* GetEvent();

	SP_EventState		GetEventState()				{ return mEventState; }
	S32					GetNumCheckpoints()			{ return mNumCheckpoints; }
	S32					GetCheckpointsPassed()		{ if (mCheckpointsPassed > 0) return(mCheckpointsPassed-1); else return(0); }
	S32					GetActualCheckpointsPassed(){ return mCheckpointsPassed; }

	float				GetTimeLeft()				{ return mTimeLeft; }
	float				GetTimeDiffToMedal()		{ return mTimeDiffToMedal; }

	F32					GetTimeAddedCheckpoint()	{ return mUiTimeAddedCheckpoint; }
	F32					GetTimeAddedStopwatch()		{ return mUiTimeAddedStopwatch; }
	F32					GetLastTime()				{ return mUiTimeLast; }

	AmaxRaceModeCheckpointVars*	GetDifficultyVars();

public:
	SP_EventState								mEventState;
	
	float										mTimeLeft;
	float										mTimeDiffToMedal;
	S32											mNumCheckpoints;
	S32											mCheckpointsPassed;
	S32											mCurrentRaceRuleArea;

	F32											mUiTimeAddedCheckpoint;
	F32											mUiTimeAddedStopwatch;
	F32											mUiTimeLast;

	bool										mLastValidPlayerPosInitialised;
	F32											mLastValidPlayerPosLap;
	Maths::HVec3								mLastValidPlayerPosWorld;
	Maths::HVec3								mLastValidPlayerPosDirection;

	U8											mNitrosCollected;
	U8											mNitrosUsed;
	U8											mClocksCollected;
	F32											mTimeAtLastCheckpoint;

	void										GetValidPlayerDirection();
};


} // namespace Game

#endif // RACEMODE_CHECKPOINT_H
