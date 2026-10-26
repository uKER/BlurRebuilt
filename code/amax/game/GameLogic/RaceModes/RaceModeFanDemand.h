#ifndef RACEMODE_FANDEMAND_H
#define RACEMODE_FANDEMAND_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>
#include <GameLogic/GameData/GameEventData.h>


struct SP_FanEvent;

namespace Game
{

class C_RaceModeFanDemand : public C_RaceModeBase
{
public:
	void Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void SetupRace();
	void Update();
	void CheckRaceEnd();
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
	
	SP_FanEvent* GetEvent();

	SP_EventState		GetEventState()			{ return mEventState; }
	float				GetTimeLeft()			{ return mTimeLeft; }
	S32					GetNumFans()			{ return mNumFans; }
	S32					GetNumDemandsComplete()	{ return mNumDemandsCompleted; }
	S32					GetDiffToMedal()		{ return mDiffToMedal; }

	AmaxRaceModeFanDemandVars*	GetDifficultyVars();
	
public:

	bool										mPassed;

	SP_EventState								mEventState;
	float										mTimeLeft;
	S32											mNumFans;
	S32											mDiffToMedal;
	S32											mNumDemandsCompleted;
};


} // namespace Game

#endif // RACEMODE_FANDEMAND_H
