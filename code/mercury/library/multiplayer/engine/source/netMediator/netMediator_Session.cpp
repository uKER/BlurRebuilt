#include "stdafx.h"
#include "./netMediator_Session.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include "netSession/netEnumeratedSession.h"
#include "netSession/netSessionJoinData.h"
#include "netSession/netSessionCreateData.h"
#include "netSession/netSessionQoS.h"
#include "netSession/netSessionState.h"
#include "netState/factory/ifactory.h"
#include "netState/stateEnums.h"
#include "netTransport/netTransport_Enums.h"
#include <network\securekey.h>
#include <Network\SessionDeletion.h>
#include <bdNet/bdNet.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>
#include <ui/graphics/simple2d.h>
#include <system/debug/I_DebugRender.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netMediation
{
	Session::Session(const char* debugName, int maxClients, int maxEntities, netTransport::SocketRef socket, unsigned char socketPort)
	: m_debugName			(debugName)
	, m_maxClients			(maxClients)
	, m_maxEntities			(maxEntities)
	, m_socketPort			(socketPort)
	, m_socket				(socket)
	, m_status				(STATUS_OK)
	, m_state				(STATE_NOT_SET)
	, m_step				(0)
	, m_online				(false)
	, m_presence			(false)
	, m_sendCapKBpS			(16.0f)
	, m_receiveCapKBpS		(64.0f)
	, m_receiveLoadKBpS		(0.0f)
	, m_sendLoadKBpS		(0.0f)
	, m_NATtype				(netPlatform::NAT_TYPE_NOT_SET)
	{}


	Session::~Session()
	{
		if(m_session.notNull())
		{
			LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> session not in inactive state (state = %d)", debugName(), m_session->SessionState());
			m_session->Delete();
		}
		cleanup();
	}

	PeerStateRef Session::peer(const netTransport::ConnGuid& guid)
	{
		PeerStateRef session;
		m_peers.get(guid, session);
		return session;
	}

	netSessionRef Session::session()
	{
		return m_session;
	}
		
	netEnumeratedSessionRef Session::sessionInfo()
	{
		return m_sessionInfo;
	}

	bool Session::setPresence(bool onOff)
	{
		m_presence = onOff;
		return true;
	}
	
	bool Session::isPresence() const
	{
		return m_presence;
	}

	int Session::setStatus(int status)
	{
		m_status = status;
		return m_status;
	}

	int Session::setState(int state)
	{
		m_state = state;
		m_step = 0;
		poll();
		return m_state;
	}

	void Session::render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
	{	
		Maths::RGBA c(255, 255, 255, 255);
		char str[256];
				
		sprintf_s(str, sizeof(str), "session(%s) state(%s)", m_debugName, stateToString(m_state));
		debugRenderPtr->Draw2dText(Maths::HVec2(x, y), str, c, 24);
		
		debugRenderPtr->SetDepthEnable(false);

		x += 48;
		w -= 48;
		y += 48;
		h -= 48 * 2;

		if(netDebug::dbg_renderMode == E_DbgRnd_Migration)
		{
			renderMigration(debugRenderPtr, x, y, w * 0.65f, h);
		}
		else
		{
			renderTraffic(debugRenderPtr, x, y, w * 0.65f, h);
		}
		x += w * 0.75f;
		w -= w * 0.75f;
				
		renderGraph(debugRenderPtr, x, y, w, h);

		debugRenderPtr->SetDepthEnable(true);
	}

	void Session::leave(bool fast)
	{
		if(fast)
			setState(STATE_LEFT);
		else
			setState(STATE_LEAVING);
	}

	bool Session::join(netSessionJoinDataRef sessionJoinData, netStreamRef gameParams)
	{
		if(m_state != STATE_NOT_SET)
		{
			LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> not disconnected.", debugName());
			return false;
		}

		m_sessionInfo		= sessionJoinData->m_enumeratedSession;
		m_sessionState		= sessionJoinData->toSessionState();
		m_sessionQoS		= sessionJoinData->m_enumeratedSession->m_qos;
		m_gameParams		= gameParams;
		setState(STATE_JOINING);
		return true;
	}

	netEnumeratedSessionRef Session::createGame(netSessionCreateDataRef sessionCreateData)
	{
		bdNetImpl *const net = bdNet::getInstance();
		const bdCommonAddrRef localAddr = net->getLocalCommonAddr();
		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();

		// set up a found game packet send to discovery requests
		netEnumeratedSessionRef sessionInfo = new netEnumeratedSession();
		
		// create and register a LAN security key
		if(!sessionCreateData->m_online)
		{
			bdSecurityID secID;
			bdSecurityKey secKey;
			keyMap->initKey(secID, secKey);
			sessionInfo->setSecurityID(secID);
			sessionInfo->setSecurityKey(secKey);
		}

		sessionInfo->setHostAddr(localAddr);
		sessionInfo->setTitleID(sessionCreateData->m_titleId);
	
		return sessionInfo;
	}

	bool Session::create(netSessionCreateDataRef sessionCreateData, netStreamRef gameParams)
	{
		if(m_state != STATE_NOT_SET)
		{
			LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> not disconnected.", debugName());
			return false;
		}
		
		m_sessionInfo	= createGame(sessionCreateData); 
		m_sessionState	= sessionCreateData->toSessionState();
		m_gameParams	= gameParams;
		m_sessionQoS	= new netSessionQoS;
		*m_sessionQoS	= sessionCreateData->m_qos;
		

		setState(STATE_CREATING);
		return true;
	}

	void Session::poll()
	{
		if(m_session.notNull())		m_session->Poll();
		if(m_replicator.notNull())	m_replicator->poll();

		updateTraffic();

		updateForwarders();

		syncClients();
		
		pollConnections();

		updateSelf();

		updateState();

		updateNATtype();

		syncPresence();
	}

	void Session::pollConnections()
	{
		if(m_connectedMesh.notNull())
		{
			// poll events and process them
			netTransport::EventRef e;
			while(processEvent(m_connectedMesh->poll()))
			{}
		}
	}

	bool Session::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		return false;
	}
	
	void Session::updateNATtype()
	{
		m_NATtype = netPlatform::NAT_TYPE_NOT_SET;

		// scan all peers. find the one with the best connection to that guy.
		bdHashMap<int, PeerStateRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			// check current peer state
			PeerStateRef peer = m_peers.getValue(it);
			netTransport::ConnGuid guid = netTransport::ConnGuid(m_peers.getKey(it));
			m_peers.next(it);

			if((m_NATtype == netPlatform::NAT_TYPE_NOT_SET) || (peer->m_session.m_NATtype > m_NATtype))
				m_NATtype = peer->m_session.m_NATtype;
		}
		m_peers.releaseIterator(it);
	}

	void Session::updateState()
	{
		switch(m_state)
		{
		default:
		case STATE_NOT_SET: 
			{
				break;
			}
		case STATE_JOINING:
			{
				if(m_step == 0)
				{
					// wait till all sessions are removed
					if(NetPlay::C_SessionDeletion::Instance().ActiveCount() > 0)
						break;
					

					m_connectedMesh = NULL;
					m_online = m_sessionState->m_online;
					
					// lan game
					if(!m_online)
					{
						// nothing to do
						m_step++;
						break;
					}
					// online game
					else
					{
						m_session = new netSession(m_debugName);

						if(!m_session->create(m_sessionState))
						{
							LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> netSession::createPeer() failed.", debugName());
							setState(STATE_DISCONNECTED);
							setStatus(STATUS_ERROR);
							break;
						}
						m_step++;
						break;
					}
				}

				// first join phase, need to negotiate with the session first
				if(m_step == 1)
				{
					if(!m_online)
					{
						// nothing else to do for systemconnection.
						// now try to connect to host.
						m_step++;
						break;
				
					}
					else
					{
						// check session state
						const NetPlay::C_Session::eState state = m_session->SessionState();

						switch (state)
						{
						case NetPlay::C_Session::eStateCreating:
							{
								// continue wait
								return; 
							}
						case NetPlay::C_Session::eStateRunning:
							{
								LOGINFO(netDebug::g_Log, "<netMediator::Session(%s)> session running.", debugName());
								// now try to connect to host.
								m_step++;
								break;
							}
						default:
						case NetPlay::C_Session::eStateError:
							{
								LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> session in error state.", debugName());
								setState(STATE_DISCONNECTED);
								setStatus(STATUS_ERROR);
								break;
							}
						}
					}
				}
				if(m_step == 2)
				{
					// connect to the host
					m_connectedMesh = new netTransport::ConnectedMesh(m_online, m_maxClients, m_sessionInfo->getTitleID(), m_socket, m_socketPort);

					if(!m_connectedMesh->join(	m_sessionInfo->getSecurityID(), 
												m_sessionInfo->getSecurityKey(), 
												m_sessionInfo->getHostAddr(),
												m_gameParams))
					{
						LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> netTransport::ConnectedMesh::join() failed.", debugName());
						setState(STATE_DISCONNECTED);
						setStatus(STATUS_ERROR);
						break;				
					}
					m_step++;
				}
				
				if(m_step == 3)
				{
					// we've been kicked out of the graph
					if(m_connectedMesh->closed())
					{
						LOGWARN(netDebug::g_Log, "<netMediator::Session(%s)> ConnectedMesh closed.", debugName());
						setState(STATE_DISCONNECTED);
						setStatus(STATUS_ERROR);
						break;
					}

					// we've been accepted!
					if(m_connectedMesh->peering() || m_connectedMesh->hosting())
					{
						setState(STATE_CONNECTED);
						break;
					}
				}
			break;
			}
		case STATE_CREATING:
			{
				if(m_step == 0)
				{
					// wait till all sessions are removed
					if(NetPlay::C_SessionDeletion::Instance().ActiveCount() > 0)
						break;
					
					m_connectedMesh = NULL;
					m_online = m_sessionState->m_online;
						
					// lan game
					if(!m_online)
					{
						// nothing to do
						m_step++;
						break;
					}
					// online game
					else
					{
						// session
						m_session = new netSession(m_debugName);

						if (!m_session->create(m_sessionState))
						{
							LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> netSession::createHost() failed.", debugName());
							setState(STATE_DISCONNECTED);
							setStatus(STATUS_ERROR);
							return;
						}
						m_step++;
						break;
					}
				}
				// first join phase, need to negotiate with the session first
				if(m_step == 1)
				{
					if(!m_online)
					{
						// nothing else to do for systemconnection.
						// now try to connect to host.
						m_step++;
						break;
				
					}
					else
					{
						// check session state
						const NetPlay::C_Session::eState state = m_session->SessionState();

						switch (state)
						{
						case NetPlay::C_Session::eStateCreating:
							{
								// continue wait
								return; 
							}
						case NetPlay::C_Session::eStateRunning:
							{
								// session done
								NetPlay::SessionSecurity sec;
								if(!m_session->GetSecurityInfo(sec)) 
								{
									LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> C_Session::GetSecurityInfo() failed.", debugName());
									setState(STATE_DISCONNECTED);
									setStatus(STATUS_ERROR);
									break;
								}

								// done with the connect info
								m_sessionInfo->setTitleID(m_sessionState->m_titleId);
								m_sessionInfo->setSecurityID(sec.m_Id);
								m_sessionInfo->setSecurityKey(sec.m_Key);
								m_sessionInfo->setHostAddr(netPlatform::Core::instance()->myCommonAddr());
								LOGINFO(netDebug::g_Log, "<netMediator::Session(%s)> created session.", debugName());
								
								// now try to connect to host.
								m_step++;
								break;
							}
						default:
						case NetPlay::C_Session::eStateError:
							{
								LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> session in error state.", debugName());
								setState(STATE_DISCONNECTED);
								setStatus(STATUS_ERROR);
								break;
							}
						}
					}
				}
				if(m_step == 2)
				{
					// start host
					m_connectedMesh = new netTransport::ConnectedMesh(m_online, m_maxClients, m_sessionInfo->getTitleID(), m_socket, m_socketPort);

					if(!m_connectedMesh->host(	m_sessionInfo->getSecurityID(), 
												m_sessionInfo->getSecurityKey()))
					{
						LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> ConnectedMesh::host() failed.", debugName());
						setState(STATE_DISCONNECTED);
						setStatus(STATUS_ERROR);
						break;				
					}
					m_step++;
				}
				
				// last join phase
				if(m_step == 3)
				{
					// we've been kicked out of the graph
					if(m_connectedMesh->closed())
					{
						LOGWARN(netDebug::g_Log, "<netMediator::Session(%s)> ConnectedMesh closed.", debugName());
						setState(STATE_DISCONNECTED);
						setStatus(STATUS_ERROR);
						break;
					}

					// we've been accepted!
					if(m_connectedMesh->peering() || m_connectedMesh->hosting())
					{
						setState(STATE_CONNECTED);
						break;
					}
				}
			break;
			}
		case STATE_CONNECTED:
			{
				if(m_step == 0)
				{
					m_step++;
				}
		
				if(m_step == 1)
				{
					// sync stuff
					synchronise();

					// we've been kicked out of the session
					if(m_connectedMesh->closed())
					{
						// re-create a new one
						setState(STATE_DISCONNECTED);
						break;
					}

					if(m_session.notNull())
					{
						// check session state
						const NetPlay::C_Session::eState state = m_session->SessionState();
						
						if(	state != NetPlay::C_Session::eStateRunning &&
							state != NetPlay::C_Session::eStatePlaying)
						{
							LOGERROR(netDebug::g_Log, "<netMediator::Session(%s)> netSession in error state.", debugName());
							m_connectedMesh->close();
							break;
						}
					}
				}
				break;
			}
		case STATE_DISCONNECTED:
			{
				break;
			}
		case STATE_LEAVING:
			{
				if(m_step == 0)
				{
					// shutdown
					if(m_connectedMesh.notNull())
					{
						m_connectedMesh->close();
					}
					m_step++;
				}

				if(m_step == 1)
				{
					// connections not closed
					if (m_connectedMesh.notNull() && !m_connectedMesh->closed())
						break;

					m_step++;
				}

				if(m_step == 2)
				{
					// connections shutdown, close the sessions
					if(m_session.notNull()) 
					{
						// wait till all sessions are removed
						if(NetPlay::C_SessionDeletion::Instance().ActiveCount() > 0)
							break;
					
						m_session->Delete();
					}
					m_step++;
				}
			
				if(m_step == 3)
				{
					// wait till session is deleted
					if(NetPlay::C_SessionDeletion::Instance().ActiveCount() > 0)
						break;

					// all closed, bugger off
					setState(STATE_LEFT);
					break;
				}
				break;
			}
			break;
		case STATE_LEFT:
			{
				if(m_step == 0)
				{
					if(m_session.notNull())
					{
						m_session->Delete();
					}
					m_step++;
				}
				break;
			}
		}
	}

	void Session::cleanup()
	{
		m_gameParams = NULL;
		m_replicator = NULL;
		m_connectedMesh = NULL;
		m_socket = NULL;

		if(m_session.notNull())
		{
			m_session->Delete();
			m_session = NULL;
		}
	}

	bool Session::receive(const netTransport::ConnGuid& from, const netTransport::Message* msg)
	{
		if(m_replicator.isNull())
			return false;

		return m_replicator->receive(from, msg);
	}

	//------------------------------
	// create a new replicator client
	// when a connection appears
	//------------------------------
	bool Session::createClient(const netTransport::ConnGuid& guid)
	{
		if(m_replicator.notNull())
			m_replicator->addClient(guid);

		createClientEntities(guid);
		return true;
	}

	//------------------------------
	// destroy an old replicator client
	// when a connection disappears
	//------------------------------
	bool Session::destroyClient(const netTransport::ConnGuid& guid)
	{
		destroyClientEntities(guid);
		
		if(m_replicator.notNull())
			m_replicator->removeClient(guid);
		return true;
	}

	bool Session::hosting() const
	{
		return m_connectedMesh.notNull() && m_connectedMesh->hosting();
	}

	bool Session::peering() const
	{
		return m_connectedMesh.notNull() && m_connectedMesh->peering();
	}

	//------------------------------
	// sync presence state
	//------------------------------
	void Session::syncPresence()
	{
		if(m_sessionInfo.notNull())
			m_sessionInfo->m_qos = m_sessionQoS;

		if(m_connectedMesh.notNull())
			m_connectedMesh->advertise((m_presence && m_sessionQoS.notNull())? (bdGameInfoRef)m_sessionInfo : NULL);

		// qos only for sessions using matchmaking.
		if(m_session.notNull() && m_session->matchmaking())
			m_session->pendingQoS(m_presence && hosting()? m_sessionQoS : NULL);
	}

	//------------------------------
	// sync clients with connections
	//------------------------------
	void Session::syncClients()
	{
		if(m_connectedMesh.isNull()) return;

		if(m_replicator.isNull()) return;

		//----------------------------------------
		// the lists to sync
		//----------------------------------------
		bdHashMap<int, netReplicator::ClientRef>& clients = m_replicator->clients();
		bdHashMap<int, netTransport::ConnectionRef>& connections = m_connectedMesh->connections();
		
		//----------------------------------------
		// check if clients disappeared
		//----------------------------------------
		{
			bdHashMap<int, netReplicator::ClientRef>::Iterator jt = clients.getIterator();
			while(jt)
			{
				netReplicator::ClientRef client = clients.getValue(jt);
				netTransport::ConnGuid guid = clients.getKey(jt);
				
				// find connections attached to client
				netTransport::ConnectionRef conn;
				connections.get(guid.m_guid, conn);

				// nope, connection is gone.
				if(conn.isNull()/* || conn->destroyed() || conn->closed()*/)
				{
					// remove the client and his entities, he is no longer required.
					destroyClientEntities(guid);
					clients.remove(jt);
				}
				else
				{
					clients.next(jt);
				}
			}
			clients.releaseIterator(jt);
		}
		

		//----------------------------------------
		// check if connections appeared
		//----------------------------------------
		{
			bdHashMap<int, netTransport::ConnectionRef>::Iterator it = connections.getIterator();
			while(it)
			{
				netTransport::ConnectionRef conn = connections.getValue(it);
				netTransport::ConnGuid guid = connections.getKey(it);
				connections.next(it);

				// nope, connection is gone.
				if(conn->established())
				{
					// find client attached to connection
					netReplicator::ClientRef client;
					clients.get(guid.m_guid, client);

					// no client found.
					if(client.isNull())
					{
						// add a new replicator client for that connection
						createClient(guid);
					}
				}
			}
			connections.releaseIterator(it);
		}
	}
	
	bool Session::processEvent(netTransport::EventRef ev)
	{
		if(ev.isNull())
			return false;

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
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> self(%d) host(%d).", e->debugName(), e->m_selfGuid.m_guid, e->m_hostGuid.m_guid);

				// now we can setup the replicator.
				assert(m_replicator.isNull());
				m_replicator = new netReplicator::Replicator(	m_debugName,
																m_maxClients, m_maxEntities, 
																e->m_selfGuid,
																m_socket, m_socketPort);

				m_replicator->setHost(e->m_hostGuid);
				return true;
			}				
			// connection waiting for accept
		case netTransport::EVENT_ID_CONNECT_PENDING:
			{
				netTransport::Event_ConnPendingRef e = ((netTransport::Event_ConnPendingRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> self(%d).", e->debugName(), e->m_guid.m_guid);
				return true;
			}
			// connection attempt accepted by host.
		case netTransport::EVENT_ID_CONNECT_ACCEPTED:
			{
				netTransport::Event_ConnAcceptedRef e = ((netTransport::Event_ConnAcceptedRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> self(%d), host(%d).", e->debugName(), e->m_assignedGuid.m_guid, e->m_hostGuid.m_guid);
				return true;
			}
			// established a connection with a peer (if peering) or a client (if we're the host).
		case netTransport::EVENT_ID_CONNECT_ESTABLISHED:
			{
				netTransport::Event_ConnEstablishedRef e = ((netTransport::Event_ConnEstablishedRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> %s(%d).", e->debugName(), m_connectedMesh->hosting()? "client" : "peer", e->m_guid.m_guid);
			
				/*
				// remove previous client instances
				if(m_replicator.notNull() && m_replicator->client(e->m_guid).notNull())
				{
					netDebug::logError(netDebug::g_MediatorLog, "<netMediator::Session::processEvent(%s)> %s(%d) already present!", e->debugName(), m_connectedMesh->hosting()? "client" : "peer", e->m_guid.m_guid);
					destroyClient(e->m_guid);
				}
				createClient(e->m_guid);
				*/
				return true;
			}
			// connection closed
		case netTransport::EVENT_ID_CONNECT_CLOSED:
			{
				netTransport::Event_ConnClosedRef e = ((netTransport::Event_ConnClosedRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> guid(%d), reason(%d).", e->debugName(), e->m_guid.m_guid, e->m_reason);
				//destroyClient(e->m_guid);
				return true;
			}
			// we're hosting.
		case netTransport::EVENT_ID_MESH_HOSTING:
			{
				netTransport::Event_MeshHostingRef e = ((netTransport::Event_MeshHostingRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> self(%d).", e->debugName(), e->m_selfGuid.m_guid);
				
				if(m_replicator.notNull())
				{
					m_replicator->setHost(e->m_selfGuid);
				}
				//-------------------------------------------------
				// re-sync clients after a migration
				//-------------------------------------------------
				//syncClients();
				return true;
			}
			// we're peering.
		case netTransport::EVENT_ID_MESH_PEERING:
			{
				netTransport::Event_MeshPeeringRef e = ((netTransport::Event_MeshPeeringRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> self(%d), host(%d).", e->debugName(), e->m_selfGuid.m_guid, e->m_hostGuid.m_guid);
				if(m_replicator.notNull())
				{
					m_replicator->setHost(e->m_hostGuid);
				}
				return true;
			}
			// we're closed!
		case netTransport::EVENT_ID_MESH_CLOSED:
			{
				netTransport::Event_MeshClosedRef e = ((netTransport::Event_MeshClosedRef)ev);
				LOGINFO(netDebug::g_Log, "<netMediator::Session::processEvent(%s)> reason(%d).", e->debugName(), e->m_reason);
				setStatus(e->m_reason);
				
				if(m_state < STATE_DISCONNECTED)
					setState(STATE_DISCONNECTED);

				return true;
			}
		}
		return true;
	}
		
	const char* Session::stateToString(int state)
	{
		switch(state)
		{
			default:					return "UNKNOWN";
			case STATE_NOT_SET:			return "NOT SET";
			case STATE_CREATING:		return "CREATING";
			case STATE_JOINING:			return "JOINING";
			case STATE_CONNECTED:		return "CONNECTED";
			case STATE_DISCONNECTED:	return "DISCONNECTED";
			case STATE_LEAVING:			return "LEAVING";
			case STATE_LEFT:			return "LEFT";
		}
	}


	void Session::synchronise()
	{
		// update our session state
		syncConnections();
		syncMigration();
	}

	void Session::syncConnections()
	{
		if(m_connectedMesh.isNull())
			return;

		netTransport::ConnGuid selfGuid = m_connectedMesh->selfGuid();
		netTransport::ConnGuid hostGuid = m_connectedMesh->hostGuid();

		// get our session state
		PeerStateRef selfState = peer(selfGuid);
		if(selfState.isNull()) return;

		//-------------------------------------
		// graphState sync
		//-------------------------------------
		if (m_syncHost.notNull())
		{
			// sync our connection list with our host connection list
			if(hostGuid != selfGuid)
			{
				m_connectedMesh->syncPeers(m_syncHost->m_connections.m_peers, m_syncHost->m_connections.m_peerCount); 
			}
		}
	}

	netTransport::ConnGuid Session::findForwarder(const netTransport::ConnGuid& target, const netTransport::Peer* myPeers, int myPeerCount)
	{
		// the forwarder we will use to communicate with target.
		netTransport::ConnGuid forwarder;

		if(!netTransport::Debugging::dbg_forwardEnabled)
			return forwarder;
		
		if(m_connectedMesh.isNull())
			return forwarder;

		// ourselves
		netTransport::ConnGuid selfGuid = m_connectedMesh->selfGuid();
		PeerStateRef selfState = peer(selfGuid);
		if(selfState.isNull()) return forwarder;
		
		int qosRatingChosen = netTransport::CONN_QOS_UNAVAILABLE;
		
		// scan all peers. find the one with the best connection to that guy.
		bdHashMap<int, PeerStateRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			// check current peer state
			PeerStateRef peer = m_peers.getValue(it);
			netTransport::ConnGuid guid = netTransport::ConnGuid(m_peers.getKey(it));
			m_peers.next(it);

			// that peer is our target. we cant use him as a forwarder to himself.
			if(guid == target)
				continue;

			// that peer is us.
			if(guid == selfGuid)
				continue;

			if(peer->m_session.m_selfGuid != guid)
				continue;

			// connection with peer not established. cant use him as a forwarder.
			netTransport::ConnectionRef peerConn = m_connectedMesh->findConnection(guid);
			if(peerConn.isNull() || !peerConn->established())
				continue;

			// NAT address not connected. cant use him as a forwarder.
			int addrStatus = netPlatform::Core::instance()->getHandleStatus(peerConn->getAddrHandle());
			if(addrStatus != netPlatform::ADDRESS_STATUS_CONNECTED)
				continue;
			
			// find the connection status of the peer with our target.
			const netTransport::Peer* conn = peer->m_connections.peer(target);
			if(conn == NULL || !conn->established()) continue;
			
			// that peer has a better connection to target.
			if(conn->m_qos > qosRatingChosen)
			{
				qosRatingChosen = conn->m_qos;
				forwarder = guid;
			}
		}
		m_peers.releaseIterator(it);
		return forwarder;
	}

	void Session::updateForwarders()
	{
		if(m_connectedMesh.isNull())
			return;

		// get the list of peers we are connected to.
		PeerState::ConnectionList peers;
		m_connectedMesh->getPeers(peers.m_peers, peers.m_peerCount, netMediation::MAX_PEERS);

		for(int i = 0; i < peers.m_peerCount; i ++)
		{
			// connection layer
			const netTransport::Peer* peerConn = &(peers.m_peers[i]);

			// find connection with the best qos to the peer.
			netTransport::ConnGuid forwarder = findForwarder(peerConn->m_guid, peers.m_peers, peers.m_peerCount);

			// use him as a forwarder in case we loose connection with that peer.
			m_connectedMesh->setForwarder(peerConn->m_guid, forwarder);
		}
	}
		
	void Session::updateTraffic()
	{
		if(m_connectedMesh.isNull())
			return;

		if(m_outTrafficLogger.isNull())
			m_outTrafficLogger = new netTransport::TrafficLog("outstream");

		if(m_inTrafficLogger.isNull())
			m_inTrafficLogger = new netTransport::TrafficLog("instream");

		// bandiwth throttle in bytes / s.
		m_connectedMesh->setRates(m_sendCapKBpS * 1024.0f, m_receiveCapKBpS * 1024.0f);

		//---------------------------------------------------------------
		// get traffic report
		//---------------------------------------------------------------
		netTransport::TrafficSample outReport;
		netTransport::TrafficSample inReport;
		m_connectedMesh->getOutboundTraffic(outReport);
		m_connectedMesh->getInboundTraffic(inReport);

		m_receiveLoadKBpS = (inReport.m_bytes[netTransport::TRAFFIC_TYPE_SOCKET] / 1024.0f); // in kilobytes / second
		m_sendLoadKBpS    = (outReport.m_bytes[netTransport::TRAFFIC_TYPE_SOCKET] / 1024.0f); // in kilobytes / second

		if(m_outTrafficLogger.notNull())
		{
			m_outTrafficLogger->record(outReport);
			m_outTrafficLogger->update();
			//m_sendLoadKBpS = m_outTrafficLogger->getBandwidth();
		}

		if(m_inTrafficLogger.notNull())
		{
			m_inTrafficLogger->record(inReport);
			m_inTrafficLogger->update();
			//m_receiveLoadKBpS = m_inTrafficLogger->getBandwidth();
		}
	}

	bool Session::promotePeer()
	{
		if(m_connectedMesh.isNull())
			return false;

		if(!m_connectedMesh->hosting())
			return false;

		if(m_replicator.isNull())
			return false;

		PeerStateRef selfState = peer(m_connectedMesh->selfGuid());
		if(selfState.isNull()) return false;
		
		// get the list of peers we are connected to.
		PeerState::ConnectionList peers;
		m_connectedMesh->getPeers(peers.m_peers, peers.m_peerCount, netMediation::MAX_PEERS);

		// list of clients with our latest state.
		int clients[netMediation::MAX_PEERS];
		int numClients = m_replicator->listClientsForPromotion(clients, netMediation::MAX_PEERS);

		// filter out the junk.
		for(int i = 0; i < numClients; i ++)
		{
			// connection layer
			const netTransport::Peer* peerConn = peers.peer(clients[i]);

			// peer data
			PeerStateRef peerState = peer(netTransport::ConnGuid(clients[i]));

			bool invalidate = (!peerConn->established()); // connection not healthy
			invalidate |= (peerState.isNull()); // no peer data
			invalidate |= (peerState->m_session.m_hostGuid != m_connectedMesh->selfGuid()); // peer host isn't us.
			invalidate |= (peerState->m_session.m_electionCandidate != peerState->m_session.m_hostGuid); // peer migrarting.
			invalidate |= (peerConn->m_guid == m_connectedMesh->selfGuid()); // peer is self. cant promote self.

			// remove potential candidate from list, he is no good.
			if(invalidate)
			{
				numClients--;
				clients[i] = clients[numClients];
			}
		}

		// nobody can be promoted. 
		if(numClients <= 0)
			return false;

		// pick random one.
		int pick = netPlatform::Core::instance()->randomInt(numClients);

		// set our promoted client.
		selfState->m_session.m_electionCandidate = netTransport::ConnGuid(clients[pick]);

		// debug output.
		PeerStateRef peerState = peer(selfState->m_session.m_electionCandidate);

		LOGINFO(netDebug::g_MigrationLog, "<netMediator::Session(%s)> promoting peer(%d, '%s') to host status...", debugName(), selfState->m_session.m_electionCandidate.m_guid, peerState->m_session.m_name);
		return true;
	}

	PeerStateRef Session::electMigrationCandidate(PeerStateRef a, PeerStateRef b)
	{
		if(a->m_session.m_electionWeight < b->m_session.m_electionWeight)
			return b;
		
		if(b->m_session.m_electionWeight < a->m_session.m_electionWeight)
			return a;

		if(b->m_session.m_selfGuid < a->m_session.m_selfGuid)
			return b;
		
		if(a->m_session.m_selfGuid < b->m_session.m_selfGuid)
			return a;

		return b;
	}		

	void Session::updateSelf()
	{
		if(netTransport::Debugging::dbg_callOutputMigrate)
		{
			debugOutputMigration();
			netTransport::Debugging::dbg_callOutputMigrate = false;
		}
		
		if(m_connectedMesh.isNull())
			return;

		// not hosting or peering. dont bother with migration for now.
		if(!m_connectedMesh->hosting() && !m_connectedMesh->peering())
			return;

		// session not ready.
		if(m_session.notNull() && m_session->sessionState().isNull())
			return;

		// our current host/self guids.
		netTransport::ConnGuid selfGuid = m_connectedMesh->selfGuid();
		netTransport::ConnGuid hostGuid = m_connectedMesh->hostGuid();

		// get our session state
		PeerStateRef selfState = peer(selfGuid);
		PeerStateRef hostState = peer(hostGuid);
		if(selfState.isNull()) return;
		m_syncHost = hostState;

		// the migration weight
		int electionWeight = m_replicator->migrationWeight(hostGuid);

		// setup our connectivity state.
		selfState->m_session.m_hostGuid			= hostGuid;
		selfState->m_session.m_selfGuid			= selfGuid;
		selfState->m_session.m_electionWeight	= (electionWeight >= 0)? electionWeight : selfState->m_session.m_electionWeight;
		selfState->m_session.m_NATtype			= netPlatform::Core::instance()->NATtype();

		netPlatform::Core::instance()->getConsoleName(selfState->m_session.m_name, sizeof(selfState->m_session.m_name));

		// debug migration
		if(netTransport::Debugging::dbg_callMigrate)
		{
			promotePeer();
			netTransport::Debugging::dbg_callMigrate = false;
		}

		// get the list of peers we are connected to.
		m_connectedMesh->getPeers(selfState->m_connections.m_peers, selfState->m_connections.m_peerCount, netMediation::MAX_PEERS);

		// copy migration info
		if(m_session.notNull() && m_session->sessionState().notNull())
		{	
			m_session->sessionState()->m_migrationInfo;
		}

		// sort out migration host. If promoted peer not valid, set us as host.
		if(m_connectedMesh->hosting())
		{
			netTransport::ConnectionRef conn = m_connectedMesh->findConnection(selfState->m_session.m_electionCandidate);
			if(conn.isNull() || !conn->established())
			{
				selfState->m_session.m_electionCandidate = selfGuid;
			}
		}
		
		// now work out the new migration host for peers.
		if(m_connectedMesh->peering())
		{
			// find our host in the list
			const netTransport::Peer* host = selfState->m_connections.peer(hostGuid);
			
			// host healthy. Use his promoted peer as migration target.
			if(host && host->established() && hostState.notNull())
			{
				m_syncHost = hostState;
				selfState->m_session.m_electionCandidate = hostState->m_session.m_electionCandidate;
			}
			// elect a peer as our migration candidate.
			else
			{
				// vote for ourselves for the moment.
				m_syncHost = selfState;
				selfState->m_session.m_electionCandidate = selfGuid;
				
				// find best, healthy peer to sync with.
				for(int i = 0; i < selfState->m_connections.m_peerCount; i ++)
				{
					// peer not healthy. skip.
					const netTransport::Peer* peerConn = &(selfState->m_connections.m_peers[i]);
					if(!peerConn->established()) continue;

					// get the peer's session state.
					PeerStateRef peerState = peer(peerConn->m_guid);
					if(peerState.isNull()) continue;
					if(peerState->m_session.m_selfGuid != peerConn->m_guid) continue;

					// elect best of the two candidates. 
					m_syncHost = electMigrationCandidate(peerState, m_syncHost);

					// use his candidate as the migration host
					selfState->m_session.m_electionCandidate = m_syncHost->m_session.m_electionCandidate;
				}
			}
		}

		// check if our close peers agrees with our candidate.
		selfState->m_session.m_electionResults = true;
		for(int i = 0; i < selfState->m_connections.m_peerCount; i ++)
		{
			// peer not healthy. skip.
			const netTransport::Peer* peerConn = &(selfState->m_connections.m_peers[i]);
			if(!peerConn->established()) continue;

			// check current peer state
			PeerStateRef peerState = peer(peerConn->m_guid);
			if(peerState.isNull()) continue;
			if(peerState->m_session.m_selfGuid != peerConn->m_guid) continue; // peer data incomplete

			// peer disagrees (or not migrating yet).
			if(	peerState->m_session.m_electionCandidate != selfState->m_session.m_electionCandidate ||
				peerState->m_session.m_electionWeight < 0)
			{
				selfState->m_session.m_electionResults = false;
				break;
			}
		}		


		// check if everyone agrees with our candidate.
		selfState->m_session.m_electionUnanimous = selfState->m_session.m_electionResults;
		
		for(int i = 0; i < selfState->m_connections.m_peerCount; i ++)
		{
			// peer not healthy. skip.
			const netTransport::Peer* peerConn = &(selfState->m_connections.m_peers[i]);
			if(!peerConn->established()) continue;

			// check current peer state
			PeerStateRef peerState = peer(peerConn->m_guid);
			if(peerState.isNull()) continue;
			if(peerState->m_session.m_selfGuid != peerConn->m_guid) continue; // peer data incomplete

			// peer disagrees
			if((peerState->m_session.m_electionCandidate != selfState->m_session.m_electionCandidate) || !(peerState->m_session.m_electionResults))
			{
				selfState->m_session.m_electionUnanimous = false;
				break;
			}
		}

		// set weight to -1 if we are not migrating.
		if(selfState->m_session.m_hostGuid == selfState->m_session.m_electionCandidate)
		{
			selfState->m_session.m_electionWeight = -1;
		}		
	}

	void Session::syncMigration()
	{
		// no migration allowed
		if(!netTransport::Debugging::dbg_migrationEnabled)
			return;

		// no mesh?
		if(m_connectedMesh.isNull())
			return;

		// not hosting or peering. dont bother with migration for now.
		if(!m_connectedMesh->hosting() && !m_connectedMesh->peering())
			return;

		// session not ready yet.
		if(m_session.notNull() && m_session->sessionState().isNull())
			return;
		
		// migrating. wait.
		if(m_session.notNull() && m_session->migrating())
			return;

		if(m_replicator.isNull() || m_replicator->migrating())
			return;
		
		// local peer state
		PeerStateRef selfState = peer(m_connectedMesh->selfGuid());
		if(selfState.isNull()) return;

		// new peer promoted to host status.
		if(	(selfState->m_session.m_selfGuid == selfState->m_session.m_electionCandidate) && 
			(selfState->m_session.m_selfGuid != selfState->m_session.m_hostGuid) &&
			(selfState->m_session.m_electionCandidate != m_connectedMesh->hostGuid()) && 
			(selfState->m_session.m_electionUnanimous))
		{
			// promote us.
			LOGINFO(netDebug::g_MigrationLog, "<netMediator::Session(%s)> we(%d, '%s) have been promoted to host status.", debugName(), selfState->m_session.m_electionCandidate.m_guid, selfState->m_session.m_name);
			m_connectedMesh->setHost(selfState->m_session.m_electionCandidate);
			m_replicator->setHost(selfState->m_session.m_electionCandidate);

			// kick dodgy peers.
			kickOutOfSyncClients();
		}
		// find promoted peer.
		else
		{
			bdHashMap<int, PeerStateRef>::Iterator it = m_peers.getIterator();
			while(it)
			{
				// check current peer state
				PeerStateRef thepeer = m_peers.getValue(it);
				m_peers.next(it);
		
				// promoted peer
				if(	thepeer->m_session.m_selfGuid == thepeer->m_session.m_hostGuid && 
					thepeer->m_session.m_selfGuid == thepeer->m_session.m_electionCandidate && 
					thepeer->m_session.m_selfGuid != m_connectedMesh->hostGuid())
				{
					netTransport::ConnectionRef conn = m_connectedMesh->findConnection(thepeer->m_session.m_selfGuid);
					if(conn.notNull() && conn->established())
					{
						LOGINFO(netDebug::g_MigrationLog, "<netMediator::Session(%s)> peer(%d, '%s) promoted to host status.", debugName(), thepeer->m_session.m_selfGuid.m_guid, thepeer->m_session.m_name);
						m_connectedMesh->setHost(thepeer->m_session.m_selfGuid);
						m_replicator->setHost(thepeer->m_session.m_selfGuid);
						break;
					}
				}
			}
			m_peers.releaseIterator(it);
		}

		// update session migration info.
		bool hosting = (m_connectedMesh->hosting());

		// update session migration info and security info (migration only for sessions using matchmaking).
		if(m_session.notNull() && m_session->matchmaking())
		{
			// grab new security info
			m_connectedMesh->setSecurity(m_session->sessionState()->m_securityInfo.m_Id, m_session->sessionState()->m_securityInfo.m_Key);
		
			// migrate from client to host or something.
			m_session->pendingState()->m_hosting = hosting;
				
			// update migration info (host broadcast to peers).
			if(hosting)
			{
				selfState->m_session.m_migrationInfo = m_session->sessionState()->m_migrationInfo;
			}
			// update migration info (peers get host info).
			else
			{
				m_session->pendingState()->m_migrationInfo = m_syncHost->m_session.m_migrationInfo;
			}
		}
	}

	// kick dodgy peers.
	bool Session::kickOutOfSyncClients()
	{
		if(m_connectedMesh.isNull() || !m_connectedMesh->hosting())
			return false;
			
		// get the list of peers we are connected to.
		PeerState::ConnectionList peers;
		m_connectedMesh->getPeers(peers.m_peers, peers.m_peerCount, netMediation::MAX_PEERS);

		for(int i = 0; i < peers.m_peerCount; i ++)
		{
			const netTransport::Peer* conn = &(peers.m_peers[i]);
			PeerStateRef state = peer(conn->m_guid);
						
			bool kick = (!conn->established());
			kick |= (state.isNull());
			kick |= (state->m_session.m_selfGuid.isNull());
			kick |= (state->m_session.m_electionCandidate != m_connectedMesh->selfGuid()); // hmmmm.... Really?

			if(kick)
			{
				m_connectedMesh->close(conn->m_guid, netTransport::CLOSE_REASON_KICKED);
				LOGWARN(netDebug::g_MigrationLog, "<netMediator::Session(%s)> kicking out-of-sync peer(%d, '%s')...", debugName(), state->m_session.m_electionCandidate.m_guid, state->m_session.m_name);
			}			
		}
		return true;
	}

	bool Session::factoryCreate(int type, bool registerFactory)
	{
		netTransport::ConnGuid guid = entityClientGuid(type);
		if(guid.isNull())
		{
			return false;
		}
		
		if(!entityCreate(type, guid))
		{
			return false;
		}

		if(registerFactory && !I_Factory::GetInstance()->CreatedGameObject(type, guid.m_guid, g_sNetIdAutoAssign, g_sCalculateSize))
		{
			BIZ_ASSERT_MSG(false, "CreatedGameObject() failed!");
			return false;
		}

		const char* entName = I_Factory::GetInstance()->NetEntityString(type);
		LOGINFO(netDebug::g_Log, "<netMediator::Session(%s)> factory created entity('%s') guid(%d).", debugName(), entName, guid.m_guid);
		return true;
	}

	bool Session::factoryDestroy(int type)
	{
		netTransport::ConnGuid guid = entityClientGuid(type);
		if(guid.isNull())
		{
			return false;
		}
		if(!entityDestroy(type, guid))
		{
			return false;
		}

		const char* entName = I_Factory::GetInstance()->NetEntityString(type);
		LOGINFO(netDebug::g_Log, "<netMediator::Session(%s)> factory destroyed entity('%s') guid(%d).", debugName(), entName, guid.m_guid);
		return true;
	}

	bool Session::factorySerialise(int type, netStream* stream)
	{
		netTransport::ConnGuid guid = entityClientGuid(type);
		if(guid.isNull())
		{
			return false;
		}
	
		return entitySerialise(type, guid, stream);
	}
	
	bool Session::factoryDeserialise(int type, netStream* stream)
	{
		netTransport::ConnGuid guid = entityClientGuid(type);
		if(guid.isNull())
		{
			return false;
		}
		return entityDeserialise(type, guid, stream);
	}

	void Session::updateRenderGraph(float x, float y, float w, float h)
	{
		m_renderGraph.clear();

		if(m_connectedMesh.isNull())
			return;

		w -= 32;
		h -= 32;
		if(w < 100) w = 100;
		if(h < 100) h = 100;
		float r = (w < h)? w / 2 : h / 2;
		Maths::HVec2 c(x + w / 2, y + h / 2);
		
		//---------------------------------------------------
		// list all known peers
		//---------------------------------------------------
		bdHashMap<int, PeerStateRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			// check current peer state
			PeerStateRef state = m_peers.getValue(it);
			netTransport::ConnGuid guid = m_peers.getKey(it);
			m_peers.next(it);

			if(!m_renderGraph.containsKey(guid))
			{
				netTransport::ConnectionRef conn = m_connectedMesh.isNull()? NULL : m_connectedMesh->findConnection(guid);
				bool established = (conn.notNull() && conn->established());
				established |= (m_connectedMesh.notNull() && m_connectedMesh->selfGuid() == guid);

				char buff[128];
				const char* name = renderGraphNodeName(guid, buff, sizeof(buff));
				m_renderGraph.put(guid, new PeerStateRender(name, guid, state, established));
			}

			// list peers of that peer
			for(int j = 0; j < state->m_connections.m_peerCount; j++)
			{
				const netTransport::Peer* peerConn = &(state->m_connections.m_peers[j]);

				if(!m_renderGraph.containsKey(peerConn->m_guid))
				{
					PeerStateRef peerstate = peer(peerConn->m_guid);

					netTransport::ConnectionRef conn = m_connectedMesh.isNull()? NULL : m_connectedMesh->findConnection(peerConn->m_guid);
					bool established = (conn.notNull() && conn->established());
					established |= (m_connectedMesh.notNull() && m_connectedMesh->selfGuid() == peerConn->m_guid);

					char buff[128];
					const char* name = renderGraphNodeName(peerConn->m_guid, buff, sizeof(buff));
					m_renderGraph.put(peerConn->m_guid, new PeerStateRender(name, peerConn->m_guid, peerstate, established));
				}
			}
		}
		m_peers.releaseIterator(it);
		
		PeerStateRef selfState = peer(m_connectedMesh->selfGuid());

		// total 
		int count = m_renderGraph.getSize();
		if(count <= 0) return;

		float  a = -(Maths::Pi / 2.0f);
		float da =  (Maths::Pi * 2.0f) / count;
			
		PeerStateRenderRef selfStateRender;
		PeerStateRenderRef hostStateRender;

		if(m_connectedMesh.notNull())
		{
			m_renderGraph.get(m_connectedMesh->hostGuid(), hostStateRender);
			m_renderGraph.get(m_connectedMesh->selfGuid(), selfStateRender);
		}

		//---------------------------------------------------
		// setup the render bits
		//---------------------------------------------------
		it = m_renderGraph.getIterator();
		while(it)
		{
			PeerStateRenderRef stateRender = m_renderGraph.getValue(it);
			m_renderGraph.next(it);

			Maths::HVec2 d(Maths::Cos(a), Maths::Sin(a));
			a += da;
			
			stateRender->init(c, d, r);
		}
		m_renderGraph.releaseIterator(it);
	}

	void Session::setBandwidthCap(float in, float out)
	{
		m_sendCapKBpS = out;
		m_receiveCapKBpS = in;
	}
	
	void Session::getBandwidthCap(float& in, float& out)
	{
		in = m_receiveCapKBpS;
		out = m_sendCapKBpS;
	}
	
	void Session::getBandwidthSpare(float& in, float& out)
	{
		in = m_receiveCapKBpS - m_receiveLoadKBpS;
		out = m_sendCapKBpS - m_sendLoadKBpS;
	}

	void Session::getBandwidthLoad(float& in, float& out)
	{
		out = m_sendLoadKBpS;
		in = m_receiveLoadKBpS;
	}

	bool Session::renderMigration(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
	{
		const float font = 24.0f;
		y += font;
		h -= font;

		//---------------------------------------------------
		// list all known peers
		//---------------------------------------------------
		bdHashMap<int, PeerStateRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			char buffer[1024];

			// check current peer state
			PeerStateRef thepeer = m_peers.getValue(it);
			netTransport::ConnGuid guid(m_peers.getKey(it));
			m_peers.next(it);
			
			debugOutputMigration(thepeer, buffer, sizeof(buffer));

			netTransport::ConnectionRef conn = m_connectedMesh.isNull()? NULL : m_connectedMesh->findConnection(guid);
			
			Maths::RGBA colour = (conn.isNull())? Maths::RGBA(100, 100, 100, 255) : (!conn->established())? Maths::RGBA(160, 160, 160, 255) : Maths::RGBA(255, 255, 255, 255);

			if(m_connectedMesh.notNull() && m_connectedMesh->selfGuid() == guid)
				colour = Maths::RGBA(255, 255, 150, 255);
			
			debugRenderPtr->Draw2dText(Maths::HVec2(x, y), buffer, colour, font);
			y += font;
			h -= font;
		}
		m_peers.releaseIterator(it);
		return true;
	}

	bool Session::debugOutputMigration(PeerStateRef data, char* buffer, int len)
	{
		// check current peer state
		PeerStateRef host = peer(data->m_session.m_hostGuid);
		PeerStateRef migr = peer(data->m_session.m_electionCandidate);

		sprintf_s(buffer, len, "peer(%d, %s) host(%d, %s) mig(%d, %s) weight(%d) results(%s) unanimous(%s) peers(%d).", 
								data.isNull()? -1 :			data->m_session.m_selfGuid.m_guid, 
								data.isNull()? "unknown" :	data->m_session.m_name,
								host.isNull()? -1 :			data->m_session.m_hostGuid.m_guid, 
								host.isNull()? "unknown" :	host->m_session.m_name,
								migr.isNull()? -1 :			data->m_session.m_electionCandidate.m_guid, 
								migr.isNull()? "unknown" :	migr->m_session.m_name,
								data.isNull()? -1 :			data->m_session.m_electionWeight,
								data.isNull()? "no" :		data->m_session.m_electionResults? "yes" : "no",
								data.isNull()? "no" :		data->m_session.m_electionUnanimous? "yes" : "no",
								data.isNull()? 0 :			data->m_connections.m_peerCount);
		return true;
	}

	bool Session::debugOutputMigration()
	{
		LOGINFO(netDebug::g_MigrationLog, "-------- migration state :");
			
		//---------------------------------------------------
		// list all known peers
		//---------------------------------------------------
		bdHashMap<int, PeerStateRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			char buffer[1024];

			// check current peer state
			PeerStateRef thepeer = m_peers.getValue(it);
			m_peers.next(it);
			
			debugOutputMigration(thepeer, buffer, sizeof(buffer));

			LOGINFO(netDebug::g_MigrationLog, buffer);
		}
		m_peers.releaseIterator(it);
		return true;
	}

	void Session::renderTraffic(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
	{
		if(m_outTrafficLogger.notNull())
		{
			m_outTrafficLogger->render(debugRenderPtr, x, y, w, h / 2);
			y += h / 2;
		}

		if(m_inTrafficLogger.notNull())
		{
			m_inTrafficLogger->render(debugRenderPtr, x, y, w, h / 2);
			y += h / 2;
		}
	}

	void Session::renderGraph(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
	{
		//---------------------------------------------------
		// update graph
		//---------------------------------------------------
		updateRenderGraph(x, y, w, h);

		//---------------------------------------------------
		// render
		//---------------------------------------------------
		bdHashMap<int, PeerStateRenderRef>::Iterator it = m_renderGraph.getIterator();
		while(it)
		{
			PeerStateRenderRef state = m_renderGraph.getValue(it);
			m_renderGraph.next(it);

			state->render(debugRenderPtr, m_connectedMesh->selfGuid());
			
			bdHashMap<int, PeerStateRenderRef>::Iterator jt = m_renderGraph.getIterator();
			while(jt)
			{
				PeerStateRenderRef other = m_renderGraph.getValue(jt);
				m_renderGraph.next(jt);

				state->renderLink(debugRenderPtr, other, m_connectedMesh->selfGuid());
			}
			m_renderGraph.releaseIterator(jt);
		}
		m_renderGraph.releaseIterator(it);
	}

	const char* Session::renderGraphNodeName(const netTransport::ConnGuid& guid, char* buff, int len)
	{
		buff[0] = 0;

		PeerStateRef peerState = peer(guid);
		if(peerState.isNull()) return buff;

		sprintf_s(buff, len, "%s", peerState->m_session.m_name);
		return buff;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif