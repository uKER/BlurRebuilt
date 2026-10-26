#ifndef NET_GAME_VARS_H
#define NET_GAME_VARS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <Network/Core/NetTimer.h>
#include <Network/Misc/NetDefines.h>
#include <Network/Message/NetMessage.h>
#include <GameLogic/Structures/NetStartRacePacket.h>
#include <GameLogic\Structures\NetEventManager.h>
#include <GameLogic\Structures\LobbyWaitingRaceInfo.h>
#include <btl/Containers/containers.h>

namespace Game
{

	class C_NetGameVars
	{
	public:
		C_NetGameVars();
		~C_NetGameVars();

	//Helper functions
		void										Clear(bool clearPersistent = true);
		void										ClearNetEventList();
		void										Update();
		void										Finished();

		LobbyWaitingRaceInfo&						GetLobbyWaitingRaceInfo() { return m_LobbyWaitingInfo; }

	//Variables
	public:	
		bool										m_StartRaceDataValid;
		RacerNet::NetStartRacePacket				m_StartRaceData;

		bool										m_InRace;						// is the player actually in the race.
		bool										m_ReceivedRemoteRacePackets;	// should go to true when we receive any game logic packets from remote peers. helps make all peers start the race quicker.
		bool										m_PacketTransferEnabled;		// goes true when we enter the 321go lua script, game logic should then start sending out packets
		bool										m_RaceHost;						// are we hosting the current race, only valid after loading the race.

		C_NetEventManager*							m_EventManager;					//	Manager that handles the recording of multiplayer events - and writes them into net messages to send.

		U32											m_NetUpdateTimestep;
		bool										m_InstantUpdate;
		
		UI::E_LobbyState							m_LobbyState;
		F32											m_LobbyTimer;
		bool										m_InactivePlayer; 

		bool										m_VoteStarted;					// has the host instructed us to begin the voting process?
		bool										m_VoteCast;						// True once we have casted our vote for that race

	private:
		LobbyWaitingRaceInfo						m_LobbyWaitingInfo;				//	Information we show to players who join a race in progress and are waiting in the lobby. Sent by each in-race player.
	};

} // namespace Game

#endif // NET_GAME_VARS_H
