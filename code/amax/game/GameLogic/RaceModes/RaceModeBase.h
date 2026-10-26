#ifndef RACEMODE_BASE_H
#define RACEMODE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>

#include <ui/Game/UILua.h>
#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <gamelogic\racelogiccore\data\RLEnums.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <SafeString.inl>

#include <GameLogic\Structures\NetGameVars.h>
#include <Network/Misc/NetRaceModeListener.h>
#include <Network/Misc/NetDefines.h>
#include "ui/util/WideString.h"

#include "GameLogic\Structures\NetGameVars.h"
#include <GameLogic\VehicleSFX\LightPillarEffect.h>

namespace RacerNet{ class C_NetMessage; }

namespace Game
{
//Forward declaration! ftw!
class C_RaceLogicLayer;
namespace Actors { class RaceLogicCore; }
//Forward declaration! ftw!

class C_RaceModeBase : public Messaging::C_MessageHandler, public RacerNet::C_NetRaceModeListener
{
public:
	C_RaceModeBase();

	//Called on Bootup to set up any variables etc
	virtual void Initialise(Game::C_LevelState *pLevelState);

	//Called to setup race rules
	virtual void SetupRace();

	//Called to allow the racemode the option of setting up the opponents for this race. Return false if you want the default behaviour
	virtual bool SetupOpponents();

	//Update function called every frame
	virtual void Update();

	//Called at the start of the level (once the game has loaded)
	virtual void StartLevel();	

	//Called at the start of the race
	virtual void StartRace();	

	// For UI testing
	virtual void SetupFakeResults();
	
	//Called during the race to update all hud elements
	virtual void UpdateHUD();

	//Called at the end of the race to create the race results structure
	virtual void SetupResults();

	//Called from the debug quit and win screen
	virtual void SetupQuitAndWinResults(lua_State *pState);
	
	//Called if the player quits from the pause menu
	virtual void QuitLevel();	

	// Called when performing a quick restart
	virtual void Reset();

	//helper function to send the racemode data to lua
	virtual int SendInfoToLua(lua_State *pState);

	// network game modes will need to override this to receive game mode specific reliable network messages.
	// senderId is the remote players JoinId who sent the message to us.
	// if partyChannel is true then this message came from a party player otherwise its from a race player
	// return true if the net message was processed.
	virtual bool ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel);

	// network game modes will need to override this to receive game mode specific unreliable network messages.
	// senderId is the remote players JoinId who sent the message to us.
	// if partyChannel is true then this message came from a party player otherwise its from a race player
	// return true if the net message was processed.
	virtual bool ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel);

	void SetInternalNameAndID(char *name, E_RaceMode id)
	{
		Amax::C_SafeString::StringCopy(mName,32,name);
		mID=id;
	}

	char *GetName(){return mName;};
    E_RaceMode GetID() const {return mID;};

	// For game messages
	virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage);
	virtual const UI::C_WideString512 GetScoreString();
	virtual const UI::C_WideString512 GetRulePassedString(U32 idx);
	virtual const UI::C_WideString512 GetRuleFailedString(U32 idx);

	bool IsRaceInProgress(){return mRaceInProgress;};
	virtual void VehicleLeftRace(const U8 vehicleIndex);
	
	//	[GavinY]: Used in multiplayer games - if anyone can think of a better place to put this without horrible race mode ID switch blocks then let me know :)
	virtual bool	BuildLobbyWaitingRaceInfoData(LobbyWaitingRaceInfo& raceInfo) { return true; }

	virtual F32 	GetRaceTimeout() const { return 0.0f; }

	bool 			RaceFinished() const			{ return mRaceFinished; }
	bool 			PrimaryVehicleFinished() const	{ return mPrimaryVehicleFinished; }
	bool			RaceInProgress()		const	{ return mRaceInProgress;}

	void							SetRLC(Game::Actors::RaceLogicCore *pRLC);
	void							SetRaceLogicLayer(Game::C_RaceLogicLayer *pRaceLogicLayer);
	void							SetLevelState(Game::C_LevelState *pLevelState);

protected:
	bool							ShouldHandleMessages();

	void							SetupSpotlight(const U8 spotlightIndex, const SpecialFX::C_SpecialFX* pEffect);
	void							InitialiseSpotlights();

	//Use these functions if you want to know that the pointer's been checked
	Game::Actors::RaceLogicCore		*GetRLC_AlwaysChecked() const;
	Game::C_RaceLogicLayer			*GetRaceLogicLayer_AlwaysChecked() const;
	Game::C_LevelState				*GetLevelState_AlwaysChecked() const;

	//Use these functions if you're going to do the checking yourself
	Game::Actors::RaceLogicCore		*GetRLC() const;
	Game::C_RaceLogicLayer			*GetRaceLogicLayer() const;
	Game::C_LevelState				*GetLevelState() const;

protected:

	float mScoreTimer;
	int mScore;
	UI::C_WideString64 mScoreString;
	float mRulePassedTimer;
	int mRuleIndex;

	float mLevelledTimer;
	int mLevel;

	E_PICKUP_TYPE mCurrentPerk;

	E_PICKUP_TYPE mHoldingPerk;
	float mHoldingTimer;
	
	float mPerfectTimer;
	//temp until we have a proper gadget

	bool mLevelStarted;
	bool mRaceInProgress;

	bool mRaceFinished;
	bool mPrimaryVehicleFinished;

	static C_LightPillarEffect mPillarEffects[ MAX_VEHICLES ];

private:
	
	char mName[32];
	E_RaceMode mID;

	Game::Actors::RaceLogicCore *m_pRLC;
	Game::C_RaceLogicLayer *m_pRaceLogicLayer;
	Game::C_LevelState *m_pLevelState;
};

} // namespace Game

#endif // RACEMODE_BASE_H
