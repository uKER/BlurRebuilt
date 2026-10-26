#include "stdafx.h"
#include "TestSession.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include "netState/statemanager.h"
#include "netState/gamestate.h"
#include "netState/factory/ifactory.h"
#include "netSession/netEnumeratedSession.h"
#include "sessionEnums.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace Test
{

	Player::Player()
	{ 
		memset(m_name, 0, sizeof(m_name)); 
		m_timestamp = 0; 
	}		
	bool Player::serialise(class netStream* stream)
	{
		stream->writeBytes(m_name, sizeof(m_name));
		stream->writeType(m_timestamp);
		return stream->ok(); 
	}
	bool Player::deserialise(class netStream* stream)
	{
		stream->readBytes(m_name, sizeof(m_name));
		stream->readType(m_timestamp);
		return stream->ok(); 
	}
	HostState::HostState()
	{}
	HostState::~HostState()
	{}
	bool HostState::serialise(class netStream* stream)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			m_players[i].serialise(stream);
		}
		return stream->ok();
	}
	bool HostState::deserialise(class netStream* stream)
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			m_players[i].deserialise(stream);
		}
		return stream->ok();
	}
	PeerState::PeerState()
	{}
	PeerState::~PeerState()
	{}
	bool PeerState::serialise(class netStream* stream)
	{
		netMediation::PeerState::serialise(stream);
		m_player.serialise(stream);
		return stream->ok();
	}
	bool PeerState::deserialise(class netStream* stream)
	{
		netMediation::PeerState::deserialise(stream);
		m_player.deserialise(stream);
		return stream->ok();
	}

	Session* s_instance=NULL;

	Session* Session::instance()
	{
		return s_instance;
	}

	Session::Session()
	: netMediation::Session("testSession", 16, 1000, new netTransport::Socket(new netTransport::MessageParser()), 0)
	{
		s_instance = this;
		m_socket->registerListener(this);
	}

	Session::~Session()
	{
		m_socket->unregisterListener(this);
		s_instance = NULL;
	}

	void Session::poll()
	{
		m_socket->listen();

		setPresence(true);

		syncPeerState();

		syncHostState();

		netMediation::Session::poll();
	}

	void Session::syncPeerState()
	{
		if(mesh().isNull()) return;

		PeerStateRef p = (PeerStateRef) peer(mesh()->selfGuid());
		if(p.isNull()) return;

		netPlatform::Core::instance()->getConsoleName(p->m_player.m_name, sizeof(p->m_player.m_name));

		if(netPlatform::Core::instance()->randomInt(1000) < 5 || p->m_player.m_timestamp == 0)
			p->m_player.m_timestamp = netClock::instance()->now();
	}

	void Session::syncHostState()
	{
		if(mesh().isNull()) return;
		if(!mesh()->hosting()) return;
		if(m_hostState.isNull()) return;

		for(int i = 0; i < HostState::MAX_PLAYERS; i ++)
		{
			// check current peer state
			PeerStateRef p = (PeerStateRef) peer(netTransport::ConnGuid(i));
			
			if(p.isNull())
			{
				m_hostState->m_players[i] = Player();
			}
			else
			{
				m_hostState->m_players[i] = p->m_player;
			}
		}
	}

	void Session::render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
	{
		if(netDebug::dbg_renderMode == E_DbgRnd_GameState)
		{
			C_StateManager::GetInstance()->Render(DbUtil::DefaultDebugRenderSys()->Immediate(), x, y, w, h);

			renderGraph(DbUtil::DefaultDebugRenderSys()->Immediate(), x + w * 0.75f, y, w * 0.25f, h);
		}
		else if(netDebug::dbg_renderMode == E_DbgRnd_None)
		{
			for(int i = 0; i < HostState::MAX_PLAYERS; i ++)
			{
				// check current peer state
				PeerStateRef p = (PeerStateRef) peer(netTransport::ConnGuid(i));
				if(p.isNull()) continue;

				const char* name = p->m_player.m_name;
				float time = (float)(((F64)p->m_player.m_timestamp) / (1000.0 * 1000.0));

				char str[256];
				sprintf_s(str, sizeof(str), "%s (%.3f)", name, time);
				debugRenderPtr->Draw2dText(Maths::HVec2(x, y), str, Maths::RGBA(255, 255, 255, 255), 32);
				y += 32;
			}
			renderGraph(DbUtil::DefaultDebugRenderSys()->Immediate(), x + w * 0.75f, y, w * 0.25f, h);
		}
		else
		{
			netMediation::Session::render(debugRenderPtr, x, y, w, h);
		}
	}

	const char* Session::renderGraphNodeName(const netTransport::ConnGuid& guid, char* buff, int len)
	{
		buff[0] = 0;
		PeerStateRef p = (PeerStateRef) peer(guid);
		if(p.isNull()) return buff;

		sprintf_s(buff, len, p->m_player.m_name);
		return buff;
	}
	
	bool Session::recv(netTransport::Message* msg, int socketPort, netTransport::ConnGuid from)
	{
		return receive(from, msg);
	}

	bool Session::processEvent(netTransport::EventRef ev)
	{
		if(ev.isNull())
			return false;

		netMediation::Session::processEvent(ev);

		switch(ev->m_eventId)
		{
		default:
			{
				return true;
			}
		// we've been assigned a guid.
		case netTransport::EVENT_ID_GUID_ASSIGNED:
			{
				netTransport::Event_GuidAssignedRef e = ((netTransport::Event_GuidAssignedRef)ev);

				// create the game states managed by that session
				m_replicator->addState(0, "lobby", (1000 * 1000 / 5));
				m_replicator->setHost(e->m_hostGuid);

				// create self entities
				if((e->m_selfGuid == e->m_hostGuid))
				{
					// create host state
					factoryCreate(E_NetEnt_HostSessionState, true);
				
					// create self peer state
					createClientEntities(e->m_selfGuid);
					return true;
				}
				return true;
			}				
			// connection waiting for accept
		case netTransport::EVENT_ID_CONNECT_PENDING:
			{
				netTransport::Event_ConnPendingRef e = ((netTransport::Event_ConnPendingRef)ev);
				if(0)
				{
					m_connectedMesh->close(e->m_guid, netTransport::CLOSE_REASON_REFUSED);
					return true;
				}
				else
				{
					m_connectedMesh->accept(e->m_guid);
				}
				return true;
			}
		}
		return true;
	}

	bool Session::createClientEntities(const netTransport::ConnGuid& guid)
	{
		// only host side. Peer stuff gets instanciated by host updates
		if(m_connectedMesh->hosting())
		{
			// create peer entities attached to that connection
			factoryCreate((E_NetEnt_PeerSessionState_0 + guid.m_guid), true);
		}
		return true;
	}

	bool Session::destroyClientEntities(const netTransport::ConnGuid& guid)
	{
		// only host side. Peer stuff gets instanciated by host updates
		if(m_connectedMesh->hosting())
		{
			// create peer entities attached to that connection
			factoryDestroy(E_NetEnt_PeerSessionState_0 + guid.m_guid);
		}
		return true;
	}

	bool Session::entityCreate(int type, const netTransport::ConnGuid& guid)
	{
		switch(type)
		{
		default:
			{
				BIZ_ASSERT_MSG(false, "unknown entity!");
				return false;
			}
		case E_NetEnt_HostSessionState:
			{
				if(m_hostState.notNull())
				{
					LOGWARN(netDebug::g_Log, "<netGameMediator::Session::createEntity> host already created.");
					break;
				}			
				m_hostState = new Test::HostState();
				break;
			}
		CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
			{
				if(m_peers.containsKey(guid.m_guid))
				{
					LOGWARN(netDebug::g_Log, "<netGameMediator::Session::createEntity> peer(%d) already created.", guid.m_guid);
					break;
				}				
				netMediation::PeerStateRef peerState = new Test::PeerState();
				m_peers.put(guid.m_guid, peerState);
				break;
			}
		}
		return true;
	}
	
	netTransport::ConnGuid Session::entityClientGuid(int type)
	{
		switch(type)
		{
		default:
			{
				BIZ_ASSERT_MSG(false, "unknown entity!");
				return netTransport::ConnGuid();
			}
		case E_NetEnt_HostSessionState:
			{
				if(mesh().isNull())
					return netTransport::ConnGuid();
				return mesh()->hostGuid();
			}
		CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
			{
				netTransport::ConnGuid guid(type - E_NetEnt_PeerSessionState_0);
				return guid;
			}
		}
		return netTransport::ConnGuid();
	}


	bool Session::entityDestroy(int type, const netTransport::ConnGuid& guid)
	{
		switch(type)
		{
		default:
			{
				BIZ_ASSERT_MSG(false, "unknown entity!");
				return false;
			}
		case E_NetEnt_HostSessionState:
			{
				m_hostState = NULL;
				break;
			}
		CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
			{
				m_peers.remove(guid.m_guid);
				break;
			}
		}
		return true;
	}	

	bool Session::entitySerialise(int type, const netTransport::ConnGuid& guid, netStream* stream)
	{
		switch(type)
		{
		default:
			{
				BIZ_ASSERT_MSG(false, "unknown entity!");
				return false;
			}
		case E_NetEnt_HostSessionState:
			{
				if(m_hostState.isNull()) 
					return false;

				return m_hostState->serialise(stream);
			}
		CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
			{
				netMediation::PeerStateRef peer;
				if(!m_peers.get(guid.m_guid, peer)) return false;
				return peer->serialise(stream);
			}
		}
		return false;
	}

	bool Session::entityDeserialise(int type, const netTransport::ConnGuid& guid, netStream* stream)
	{
		switch(type)
		{
		default:
			{
				BIZ_ASSERT_MSG(false, "unknown entity!");
				return false;
			}
		case E_NetEnt_HostSessionState:
			{
				if(m_hostState.isNull()) 
					return false;

				return m_hostState->deserialise(stream);
			}
		CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
			{
				netMediation::PeerStateRef peer;
				if(!m_peers.get(guid.m_guid, peer)) return false;
				return peer->deserialise(stream);
			}
		}
		return false;
	}
}
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

