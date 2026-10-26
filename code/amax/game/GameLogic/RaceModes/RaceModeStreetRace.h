#ifndef RACEMODE_STREETRACE_H
#define RACEMODE_STREETRACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

struct SP_RaceEvent;

namespace Game
{

class C_RaceModeStreetRace : public C_RaceModeBase
{
public:
	void Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
	void Update();
	//void UpdateForPosition(NetVehicleIndexStack &localList);
	//void UpdateForLastManStanding(NetVehicleIndexStack &localList);
	void StartLevel();
	void StartRace();
	void UpdateHUD();
	void SetupFakeResults(); // For UI testing
	void SetupResults();
	void SetupQuitAndWinResults(lua_State *pState);

	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	bool ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel);
	bool ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel);

	SP_RaceEvent* GetEvent();

	//temp until we have a proper gadget
	const UI::C_WideString512 GetScoreString();
	const UI::C_WideString512 GetRulePassedString(U32 idx);
	const UI::C_WideString512 GetRuleFailedString(U32 idx);

	SP_EventState GetEventState(){ return mEventState; }

private:

	SP_EventState mEventState;
};


} // namespace Game

#endif // RACEMODE_STREETRACE_H
