#include "stdafx.h"

#include <GameLogic/Structures/NetGameVars.h>
#include <GameLogic/GameData/GameData.h>

namespace Game
{
	C_NetGameVars:: C_NetGameVars()
	{
		m_EventManager = new C_NetEventManager();
		Clear();
	}

	C_NetGameVars::~C_NetGameVars()
	{ 
		SAFE_DELETE_PTR(m_EventManager)
	}

	void C_NetGameVars::Clear(bool clearPersistent)
	{
		m_InRace = false;
		m_PacketTransferEnabled = false;
		m_ReceivedRemoteRacePackets = false;
		m_RaceHost = false;

		m_StartRaceDataValid = false;

		m_NetUpdateTimestep = RacerNet::NET_VEHICLE_UPDATE_TIME;
		m_InstantUpdate = false;
		m_LobbyState = UI::E_LobbyState_Waiting;
		m_LobbyTimer = -1.0f;
		m_InactivePlayer = false; 

		m_VoteStarted = false;
		m_VoteCast = false;

		BIZ_ASSERT(m_EventManager);

		if (m_EventManager)
			m_EventManager->Clear();

		m_LobbyWaitingInfo.Reset();

		if (clearPersistent)
		{
			// any variables that a persist between multiple network races.
		}
	}

	void C_NetGameVars::Update()
	{
	}

	void C_NetGameVars::Finished()
	{
	}

	void C_NetGameVars::ClearNetEventList()
	{
		m_EventManager->Clear();
	}
} // namespace Game
