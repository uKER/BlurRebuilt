#ifndef RACEMODE_DESTRUCTION_H
#define RACEMODE_DESTRUCTION_H

#ifdef _MSC_VER
#pragma once
#include <GameLogic/GameData/GameEventData.h>	// [Blur2PC]
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

struct SP_DestructionEvent;
struct DifficultySettings;
struct AmaxRaceModeDestruction;
struct RLGameMode_Destruction;

class C_VehicleLogicMsg_AggressiveScore; 

namespace Game
{

class C_RaceModeDestruction : public C_RaceModeBase
{
public:
	void Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void SetupRace();
	bool SetupOpponents();
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

	bool SetupOpponent( E_DestructionMode_AI_Level level, int idx, DifficultySettings* difficultySettings, AmaxRaceModeDestruction* pRaceMode );
	//temp until we have a proper gadget
	const UI::C_WideString512 GetScoreString();
	const UI::C_WideString512 GetRulePassedString(U32 idx);
	const UI::C_WideString512 GetRuleFailedString(U32 idx);

	const AmaxRaceModeDestruction* GetSetupData() { return mRaceModeDestructionData; }
	SP_DestructionEvent* GetEvent() const;

	SP_EventState		GetEventState()			{ return mEventState; }
	S32					GetNumPoints()			{ return mNumPoints; }
	S32					GetDiffToMedal()		{ return mDiffToMedal; }
	float				GetTimeLeft()			{ return mTimeLeft; }

	F32						GetLastTime()			{ return mLastTime; }

	S32						GetLastPoints()			{ return mLastPoints; }
	UI::C_WideString128&	GetLastPointsString()	{ return mLastPointsString; }
	UI::C_WideString64&		GetLastPointsName()		{ return mLastPointsName; }

	S32						GetLastBonusPoints()		{ return mLastBonusPoints; }
	UI::C_WideString128&	GetLastBonusPointsString()	{ return mLastBonusPointsString; }
	UI::C_WideString64&		GetLastBonusPointsName()	{ return mLastBonusPointsName; }

	AmaxRaceModeDestructionVars* GetDifficultyVars();

protected:
	float										mTimeLeft;
	float										mWreckedTimer;
	bool										mPassed;
	bool										mStarted;

	SP_EventState								mEventState;
	S32											mNumPoints;
	S32											mDiffToMedal;
	AmaxRaceModeDestruction*					mRaceModeDestructionData;

	F32											mLastTime;

	S32											mLastPoints;
	UI::C_WideString128							mLastPointsString;
	UI::C_WideString64							mLastPointsName;

	F32											mLastBonusTimer;
	S32											mLastBonusPoints;
	UI::C_WideString128							mLastBonusPointsString;
	UI::C_WideString64							mLastBonusPointsName;

	S32 mTotalCarsWrecked;
	S32 mEasyCarsWrecked;
	S32 mMediumCarsWrecked;
	S32 mHardCarsWrecked;
	S32 mVHardCarsWrecked;

	S32	mNormalShots;
	S32	mLongShots;
	S32	mSkillShots;
	S32 mBonusPoints;


	static int									sortByDistanceFunc(const void *a, const void *b);

	// Helper functions for drown / cliff kill logic
	void										HandleAggressiveScoreMessage(const C_VehicleLogicMsg_AggressiveScore* pMsg);
	void										ProcessEnvironmentalKill(const U8 killerVehicleIdx, const U8 victimVehicleIdx, const U32 victimCarXtID, const bool wasDrown);
	void 										AwardEnvironmentalKillPoints(const U8 killerVehicleIdx, const U8 victimVehicleIdx, const U32 victimCarXtID, const bool wasDrown);
	void 										CleanUpEnvironmentalKillVictim(const U8 victimVehicleIdx);

	// Assign the correct number of points based on the difficulty of the damaged vehicle
	U8											ProcessVehicleDamagedScore(const U32 victimCarXtID, const bool isVictimWrecked, const RLGameMode_Destruction* destructionParams);

	// Assign the correct time bonus based on the difficulty of the wrecked vehicle
	F32											CalculateAdditionalTimeReward(const U32 victimCarXtID) const;

	// Choose the correct string for the job!
	void										SetupPointsString(const S32 string_index);

	// Cache these commonly used params 
	RLGameMode_Destruction*						mpDestructionParams;
};


} // namespace Game

#endif // RACEMODE_DESTRUCTION_H
