
#include "stdafx.h"
#include "./netReplicator_State.h"
#include "netDebug/netDebug.h"
#include "netstate/gameState.h"
#include "netState/statemanager.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netReplicator
{
	State::State(int stateId, const char* debugName, int maxClients, int maxEntities, const netTransport::ConnGuid& self, U64 updateTickPeriod)
	: m_stateId(stateId)
	, m_debugName(debugName)
	, m_self(self)
	, m_updateTickPeriod(updateTickPeriod)
	{
		LOGINFO(netDebug::g_Log, "<netReplicator::State('%s')> created.", m_debugName);

		C_StateManager::GetInstance()->CreateGameState(m_stateId, m_self.m_guid, maxEntities, maxClients);

		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
	}

	State::~State()
	{
		C_StateManager::GetInstance()->FreeGameState(m_stateId);

		LOGINFO(netDebug::g_Log, "<netReplicator::State('%s')> destroyed.", m_debugName);
	}

	bool State::migrating()
	{
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
		return selfState->Migrating();
	}

	int State::listClientsForPromotion(int* clients, int maxClients)
	{
		int clientCount = 0;
		int latestAck = -1;

		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);

		if(!selfState->IsServer())
		{
			LOGERROR(netDebug::g_Log, "<netReplicator::State('%s')> listClientsForPromotion() only for server.", m_debugName);
			return 0;
		}

		// find all clients who received the latest state from the host.
		for(int i = 0; i < selfState->NumClients(); i ++)
		{
			if(!selfState->IsClientActive(i))
				continue;

			// what's the highest stet teh client received.
			int ack = selfState->OutboundAck(i);

			// this is the higest ack recorded.
			// reset the list of clients sharing that state.
			if(ack > latestAck || latestAck < 0)
			{
				clientCount = 0;
				latestAck = ack;
			}

			// client received the latest state. add to the list.
			if(ack == latestAck)
			{
				if(clientCount >= maxClients)
					return clientCount;
			
				clients[clientCount++] = i;
			}
		}
		return clientCount;
	}
	
	int State::migrationWeight(int hostGuid)
	{
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);

		if(hostGuid == selfState->MyHostId())
		{
			return selfState->SequenceNumber();
		}

		if(!selfState->IsClientActive(hostGuid))
			return -1;

		int sqn = selfState->InboundAck(hostGuid);

		return sqn;
	}

	bool State::poll()
	{
		bool ticked = m_updateTick.ticked(m_updateTickPeriod);
		
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
	
		selfState->Update(ticked);

		return true;
	}

	netTransport::ConnGuid State::getHost()
	{
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
		return netTransport::ConnGuid(selfState->ServerId());
	}

	void State::setHost(const netTransport::ConnGuid& hostGuid)
	{
		C_GameState* selfState = C_StateManager::GetInstance()->GetGameState(m_stateId);
		
		if(selfState->ServerId(hostGuid.m_guid))
		{
			LOGINFO(netDebug::g_Log, "<netReplicator::State('%s')> changed host(%d)", m_debugName, hostGuid.m_guid);
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
