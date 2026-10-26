#ifndef GAMEMODE_BASE_H
#define GAMEMODE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UILua.h>
#include <GameLogic/GameModes/GameModeDefines.h>
#include <SafeString.inl>
#include <GameLogic/Structures/NetStartRacePacket.h>

// forward declerations 
namespace RacerNet{ struct NetRaceSettings; struct NetRaceSettingsQos; class C_NetMessage; }

namespace Game
{

class C_GameModeBase
{
public:
	//Called on Bootup to set up any variables etc
	virtual void Initialise();
	
	//Update function called every frame
	virtual void Update();

	//Called when the user first enters that game modes screen
	virtual void Start();

	//Called with a pointer to data once the race has been decided
	virtual void SetupRace(void *data);

	//Called at the start of the race, in case there is any last second stuf to sort out
	virtual void StartRace();

	//Called once the race has finished and the results are valid
	virtual void InterpretRaceResults();

	//helper function to send the gamemode data to lua
	virtual int SendInfoToLua(lua_State *pState);

	// tests to see if the game mode is network enabled. network game modes need to overrride this.
	virtual bool IsNetwork(void) const {return false;}

	// tests to see if the game mode is online enabled. network game modes need to overrride this.
	virtual bool IsOnline(void) const {return false;}

	// tests to see if the game mode is an online ranked event. we override this in GameModeNetworkRanked.
	virtual bool IsRanked(void) const { return false; }

	// Flags the game mode as ranked / not ranked
	virtual void SetRanked( const bool bRanked ){};

	// tests to see if the game mode is multiplayer (network or splitscreen). multiplayer game modes need to overrde this.
	virtual bool IsMultiplayer(void) const {return false;}

	// network game modes should override this to store their settings correctly
	virtual bool GetNetRaceSettings(RacerNet::NetRaceSettings& out);

	// network game modes should override this to store their settings correctly
	virtual bool GetNetRaceSettingsQos(RacerNet::NetRaceSettingsQos& out);

	// network game modes should override this to get access to the currect settings.
	virtual void SetNetRaceSettings(RacerNet::NetRaceSettings& in);

	// called when player join / leave the network game.
	virtual void PlayerNotification(const WCHAR* pPlayerName, const bool join, const bool party) {};

	// network game modes will need to override this to receive game mode specific reliable network messages.
	// senderId is the remote players JoinId who sent the message to us.
	// if partyChannel is true then this message came from a party player otherwise its from a race player
	// return true if the net message was processed.
	virtual bool ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel) {return false;}

	// network game modes will need to override this to receive game mode specific unreliable network messages.
	// senderId is the remote players JoinId who sent the message to us.
	// if partyChannel is true then this message came from a party player otherwise its from a race player
	// return true if the net message was processed.
	virtual bool ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel) {return false;}

	// Network game modes should override this function and fill out the NetStartRacePacket structure
	virtual bool BuildStartRacePacket(RacerNet::NetStartRacePacket* packet) {return false;};

	void SetInternalNameAndID(char *name, E_GameMode id)
	{
		Amax::C_SafeString::StringCopy(mName,32,name);
		mID=id;
	}
	const char *GetName() const { return mName; };
    E_GameMode GetID() const { return mID; };

	// TODO_MP_REFACTOR
	virtual void SetPlayMode( const E_PlayMode eMode ){};

	virtual E_PlayMode GetPlayMode() const { return E_PlayMode_Nothing; }

protected:
	static void EnableChangeViewOnAllViewports();

private:
	
	char mName[32];
	E_GameMode mID;
};

} // namespace Game

#endif // GAMEMODE_BASE_H
