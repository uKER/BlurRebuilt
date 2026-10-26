// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdNet.h>
#include <bdNet/bdGetHostByName/bdGetHostByName.h>

#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#include <bdSocket/bdNAT/bdIPDiscoveryClient.h>
#include <bdSocket/bdNAT/bdNATTypeDiscoveryClient.h>

#include <bdConnection/bdConnection.h>
#include <bdConnection/bdConnectionStore.h>

#define BD_LOG_LEVEL	"bdNet/net"

#define BD_NET_MAX_SHUTDOWN_DURATION (1.0f)
#define BD_MAX_UPNP_COLLISION_RETRIES (20u)

BD_REGISTER_SINGLETON(bdNetImpl)

bdBool bdNetImpl::start(const bdNetStartParams& params)
{
	bdBool success = true;

	if (m_status != BD_NET_STOPPED)
	{
		BD_WARN(BD_LOG_LEVEL, "bdNet can only be started when in the uninitialized state. Call stop() to reset");
		success = false;
	}
	else
	{
		m_params = params;

		bdAddr addr;
		const bdBool getBindAddrResult = getBindAddr(addr);

		if(false == getBindAddrResult)
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to configure bind address");
			m_status = BD_NET_PARAMS_CONFIG_ERROR;
		}
#if defined BD_PLATFORM_WII
		else if( m_params.m_isDummy || findFreePort(addr))
#else
		else if(findFreePort(addr))
#endif
		{
			bdBool socketCreateResult = true;

			if(!m_params.m_socket)
			{
				m_params.m_socket = new bdDefaultSocket;
				const bdBool blocking = false;
				socketCreateResult = m_params.m_socket->create(blocking);
			}

					
			const bdSocketStatusCode bindResult = m_params.m_socket->bind(addr);

			if (bindResult == BD_NET_SUCCESS)
			{
				m_params.m_gamePort = addr.getPort();
				BD_INFO(BD_LOG_LEVEL, "Requested port %u, using port %u", params.m_gamePort, m_params.m_gamePort);

				
				if(false == socketCreateResult )
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to create game socket");
					m_status = BD_NET_BIND_FAILED;
					success = false;
				}
				else
				{

#if defined BD_COMPILE_RELAY_SOCKET_ROUTER
					if(m_params.m_useRelaySocketRouter)
					{
						m_socketRouter = new bdRelaySocketRouter();
					}
					else
					{
						m_socketRouter = new bdSocketRouter();
					}
#else
					m_socketRouter = new bdSocketRouter();
#endif
					// if it's an online game then start looking for our local game address (public address).
					if(m_params.m_onlineGame)
					{
						m_status = BD_NET_PENDING;
						m_currentNatTravAddrIndex = 0;
						m_currentNatTravHostIndex = 0;

						m_getHostByName = new bdGetHostByName();
						const bdBool getHostByNameStartResult = m_getHostByName->start(m_params.m_natTravHosts[m_currentNatTravHostIndex], m_params.m_hostNameLookupConfig);
						
						if(false == getHostByNameStartResult)
						{
							BD_WARN(BD_LOG_LEVEL, "Failed to start DNS lookup task.");
							m_status = BD_NET_INIT_FAILED;
							success = false;
						}
						else
						{
							// start UPnP
							const bdBool UPnPInitResult = m_UPnP.init(	m_params.m_gamePort, 
																		&(m_params.m_localAddresses),
																		m_params.m_UPnPConfig);

							if(false == UPnPInitResult)
							{
								BD_ERR(BD_LOG_LEVEL, "UPnP module failed to start.");
							}
						}
					}
					else
					{
						m_status = BD_NET_DONE;
						
						// Get the local IP address
						BD_ASSERT(!m_params.m_localAddresses.isEmpty(), "No local address specified when starting bdNet. See bdNetStartParams.");

						bdArray<bdAddr> localAddresses;
						for(bdUInt i = 0; i < m_params.m_localAddresses.getSize(); i++)
						{
							const bdAddr local(m_params.m_localAddresses[i], m_params.m_gamePort);
							localAddresses.pushBack(local);
						}

						// Use an invalid public address because we are not online.
						bdAddr remAddr;

						// create the common address
						bdCommonAddrRef localCommonAddr = new bdCommonAddr(	localAddresses,
																			remAddr);

						//create the socket router
						m_socketRouter->init(m_params.m_socket, localCommonAddr);

						// create the connection store.
						m_connectionStore.init(m_socketRouter);
						
					}
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to bind");
				m_status = BD_NET_BIND_FAILED;
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to find a free port");
			m_status = BD_NET_BIND_FAILED;
			success = false;
		}
	}

	return success;
}

void bdNetImpl::pump()
{
	if(m_params.m_onlineGame && m_status == BD_NET_PENDING)
	{
		bdBool finishedIPLookup = false;
		bdBool finishedNATLookup = false;
		const bdUInt numNATHosts = m_params.m_natTravHosts.getSize();
		bdBool finishedNATHostLookup = m_currentNatTravHostIndex == numNATHosts;

		if(	m_UPnP.isRunning() )
		{
			m_UPnP.pump();
		}
		else if( m_UPnP.getState() == bdUPnP::BD_UPNP_FINISHED &&
				 m_UPnP.getPortStatus() == bdUPnPDevice::BD_UPNP_PORT_COLLISION_FOUND )
		{
			// In case the UPnP gets into a bad way we will only retry
			// with a new port a certain number of times, after which 
			// we will simply try without UPNP.
			m_upnpCollisionRetryCount++;
			if(m_upnpCollisionRetryCount >  BD_MAX_UPNP_COLLISION_RETRIES)
			{
				// stop everything UPnP.
				m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
				BD_WARN(BD_LOG_LEVEL, "UPnP stopping after %u port collisions. ", m_upnpCollisionRetryCount);

				bdNetStartParams params = m_params;

				params.m_socket = BD_NULL;

				// Reset the port to the default port and the UPNP mode to get external IP only.
				// This will only try to get our external IP and then do a NAT type discovery without
				// trying to open a port since we have been unable to open any ports.
				params.m_gamePort = BD_DEFAULT_PORT;
				params.m_UPnPConfig.m_runMode = bdUPnPConfig::BD_UPNP_EXTERNAL_IP_ONLY;

				bdNet::getInstance()->stop();
				// start again with new port and new UPNP mode.
				start(params);
			}
			else
			{
				bdNetStartParams params = m_params;

				// This is kinda evil. We need to clear the net start params
				// socket so that start will create a new one.
				// TODO Review socket ownership!
				params.m_socket = BD_NULL;

				// the desired port mapping is already in use, pick the next one
				params.m_gamePort++;

				BD_INFO(BD_LOG_LEVEL, "UPnP Mapping collision found. New port Selected: %d", params.m_gamePort);

				bdNet::getInstance()->stop();
				// start again with new port.
				start(params);
			}
		}
		else 
		{
			// Nothing more to do.
		}

		if (m_getHostByName)
		{
			bdGetHostByName::bdStatus status = m_getHostByName->getStatus();
			if (status == bdGetHostByName::BD_LOOKUP_PENDING)
			{
				m_getHostByName->pump();
			}
			else if (status == bdGetHostByName::BD_LOOKUP_SUCCEEDED)
			{
				bdUInt numAddrs = m_getHostByName->getNumAddresses();

				for(bdUInt j = 0; j < numAddrs; j++)
				{
					const bdInetAddr inAddr( m_getHostByName->getAddressAt(j).inUn.m_iaddr);
					const bdAddr addr(inAddr, m_params.m_natTravPort);

					// Check if the addr is already contained in m_natTravAddrs
					// before adding it again? This prevents superfluous duplicate 
					// STUN traffic to the same STUN server.
					bdUInt index = 0;
					bdNChar8 addrBuffer [BD_ADDR_STRING_SIZE];
					addr.toString(addrBuffer, BD_ADDR_STRING_SIZE);
					if(false == m_natTravAddrs.findFirst(addr, index))
					{
						BD_INFO(BD_LOG_LEVEL, "Adding IP %s for Host: %s ", addrBuffer, m_params.m_natTravHosts[m_currentNatTravHostIndex].getBuffer());
						m_natTravAddrs.pushBack(addr);
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL, "Duplicate IP %s for Host: %s ignored", addrBuffer, m_params.m_natTravHosts[m_currentNatTravHostIndex].getBuffer());
					}
				}
				m_currentNatTravHostIndex++;
			}
			else
			{
				// No addresses found, try next STUN addr
				BD_WARN(BD_LOG_LEVEL, "No IP found for Host: %s ", m_params.m_natTravHosts[m_currentNatTravHostIndex].getBuffer());
				m_currentNatTravHostIndex++;
			}

			// Start a new lookup if there are still hostnames left
			if (m_currentNatTravHostIndex < numNATHosts && status != bdGetHostByName::BD_LOOKUP_PENDING)
			{
				// reset the host name lookup class so it can be reused.
				m_getHostByName->quit();
				const bdBool getHostByNameStartResult = m_getHostByName->start(m_params.m_natTravHosts[m_currentNatTravHostIndex], m_params.m_hostNameLookupConfig);
				if(false == getHostByNameStartResult)
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to start DNS lookup task.");
					m_status = BD_NET_INIT_FAILED;
				}
			}
			if (m_currentNatTravHostIndex == numNATHosts)
			{
				// Host name lookup code should be quited to avoid memory leak.
				m_getHostByName->quit();
				delete m_getHostByName;
				m_getHostByName = BD_NULL;
				// No more addresses to lookup, now we can start the IP discovery and NAT-type
				// discovery tasks
			}
		}

		if ((!m_getHostByName) && 
			(!m_UPnP.isRunning()) &&
			(m_UPnP.getPortStatus() != bdUPnPDevice::BD_UPNP_PORT_COLLISION_FOUND) &&
			(!m_ipDiscClient )&& 
			(!m_natTypeDiscClient))
		{
			// start ip and nat type discovery
			if(!m_natTravAddrs.isEmpty())
			{
				bdAddr addr = m_natTravAddrs[m_currentNatTravAddrIndex];

				m_ipDiscClient = new bdIPDiscoveryClient();
				const bdBool startIPDiscoveryResult = m_ipDiscClient->init(	m_params.m_socket, 
																			addr);

				m_natTypeDiscClient = new bdNATTypeDiscoveryClient();
				const bdBool startNATDiscoveryResult = m_natTypeDiscClient->init(m_params.m_socket, 
																			addr);

				if(false == startIPDiscoveryResult )
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to start IP discovery");
					m_status = BD_NET_ONLINE_FAILED;
				}
				if (false == startNATDiscoveryResult)
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to start NAT type discovery");
					m_status = BD_NET_ONLINE_FAILED;
				}
			}
			else
			{
				for(bdUInt i = 0; i < m_params.m_natTravHosts.getSize(); i++)
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to lookup address %s", m_params.m_natTravHosts[i].getBuffer());
				}
				m_status = BD_NET_ONLINE_FAILED;
			}
		}

		const bdUInt dataSize = BD_MAX_DATAGRAM_SIZE;
		bdUByte8 data[dataSize];
		bdAddr fromAddr;
		bdInt received = m_params.m_socket->receiveFrom(fromAddr, data, dataSize);

		if(m_ipDiscClient)
		{
			m_ipDiscClient->pump(fromAddr, data, received);
			if (bdIPDiscoveryClient::BD_IP_DISC_RUNNING != m_ipDiscClient->getStatus())
			{
				finishedIPLookup = true;
			}
		}

		if( m_natTypeDiscClient)
		{
			m_natTypeDiscClient->pump(fromAddr, data, received);
			if (false == m_natTypeDiscClient->isRunning())
			{
				finishedNATLookup = true;
			}
		}

		if(finishedNATHostLookup && finishedIPLookup && finishedNATLookup)
		{
			if(	(bdIPDiscoveryClient::BD_IP_DISC_SUCCESS == m_ipDiscClient->getStatus()) && 
				(bdNATTypeDiscoveryClient::BD_NTDCS_FINI == m_natTypeDiscClient->getStatus()))
			{
				// Get the local IP address
				BD_ASSERT(!m_params.m_localAddresses.isEmpty(), "No local address specified when starting bdNet. See bdNetStartParams.");

				bdArray<bdAddr> localAddresses;
				for(bdUInt i = 0; i < m_params.m_localAddresses.getSize(); i++)
				{
					const bdAddr local(m_params.m_localAddresses[i], m_params.m_gamePort);
					localAddresses.pushBack(local);
				}

				// the public address is the address the IP discovery client found.
				bdAddr publicAddr = m_ipDiscClient->getPublicAddress();

				bdNATType clientNAT = m_natTypeDiscClient->getNATType();
				bdAddr UPnPExternal;
				if( m_UPnP.getExternalAddr( UPnPExternal ) )
				{
					if ( UPnPExternal == publicAddr )
					{
						BD_INFO(BD_LOG_LEVEL, "UPnP mapping confirmed.");
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL, "External IP detected through STUN server does not match the one detected through UPnP. Possible Nested NATs.");
					}
				}

				// create the common address
				bdCommonAddrRef localCommonAddr = new bdCommonAddr(	localAddresses,
																	publicAddr,
																	clientNAT);

				// clean up memory used for NAT and IP discovery.
				if (m_ipDiscClient)
				{
					m_ipDiscClient->quit();
					delete m_ipDiscClient;
					m_ipDiscClient = BD_NULL;
				}

				if (m_natTypeDiscClient)
				{
					m_natTypeDiscClient->quit();
					delete m_natTypeDiscClient;
					m_natTypeDiscClient = BD_NULL;
				}
				
				bdArray<bdAddr> introducers = m_natTravAddrs;
				// we want to put the working introducer as the first in the array. we can
				// leave the rest in the array (even if they aren't working) because
				// they might start working later and it would be a good idea to send
				// keep alives to them in that case.
				if(m_currentNatTravAddrIndex != 0)
				{
					const bdAddr tmpAddr = m_natTravAddrs[0];
					introducers[0] = m_natTravAddrs[m_currentNatTravAddrIndex];
					introducers[m_currentNatTravAddrIndex] = tmpAddr;
				}

				// create the socket router.
				m_socketRouter->init(m_params.m_socket, localCommonAddr);

				m_socketRouter->setupIntroducers(introducers);

				// create the connection store.
				m_connectionStore.init(m_socketRouter);

				m_status = BD_NET_DONE;
			}
			else
			{
				if(m_currentNatTravAddrIndex + 1 < m_natTravAddrs.getSize())
				{
					BD_INFO(BD_LOG_LEVEL, "Trying next IP address for IP and NAT discovery.");

					m_currentNatTravAddrIndex++;
					bdAddr addr = m_natTravAddrs[m_currentNatTravAddrIndex];

					if (m_ipDiscClient)
					{
						m_ipDiscClient->quit();
						delete m_ipDiscClient;
						m_ipDiscClient = BD_NULL;
					}
					m_ipDiscClient = new bdIPDiscoveryClient();
					const bdBool startIPDiscoveryResult = m_ipDiscClient->init(m_params.m_socket, addr);

					if (m_natTypeDiscClient)
					{
						m_natTypeDiscClient->quit();
						delete m_natTypeDiscClient;
						m_natTypeDiscClient = BD_NULL;	
					}

					m_natTypeDiscClient = new bdNATTypeDiscoveryClient();
					const bdBool startNATDiscoveryResult = m_natTypeDiscClient->init(m_params.m_socket, addr);

					if(false == startIPDiscoveryResult )
					{
						BD_WARN(BD_LOG_LEVEL, "Failed to start IP discovery");
						m_status = BD_NET_ONLINE_FAILED;
					}
					if(false == startNATDiscoveryResult )
					{
						BD_WARN(BD_LOG_LEVEL, "Failed to start IP discovery");
						m_status = BD_NET_ONLINE_FAILED;
					}
				}
				else
				{
					for(bdUInt i = 0; i < m_params.m_natTravHosts.getSize(); i++)
					{
						BD_WARN(BD_LOG_LEVEL, "Failed to do IP and NAT discovery using %s", m_params.m_natTravHosts[i].getBuffer());
					}
					m_status = BD_NET_ONLINE_FAILED;
				}
			}
		}
	}
}

/*
void bdNetImpl::stop()
{
	// Only try to shutdown the upnp code, if it has been initialised.
	if(bdUPnP::BD_UPNP_UNINITIALISED != m_UPnP.getState())
	{
		m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT);
	}


	m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_ELEGANT);

	// Failsafe timer...!
	bdStopwatch timer;
	timer.start();

	bdBool waiting = true;
	while(waiting)
	{
		// Stop by default
		waiting = false;
		if(bdConnectionStore::BD_CONNECTION_STORE_UNINITIALIZED != m_connectionStore.getStatus())
		{
			waiting = true;
			receiveAll();
			dispatchAll();
			sendAll();
		}
		if(bdUPnP::BD_UPNP_UNINITIALISED != m_UPnP.getState())
		{
			waiting = true;
			// pump UPnP to close mappings while we wait...
			m_UPnP.pump();
		}

		if(timer.getElapsedTimeInSeconds() > BD_NET_MAX_SHUTDOWN_DURATION)
		{
			// waiting is over!
			waiting = false;
			m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);
			m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
		}

		bdPlatformTiming::sleep(20);
	}

	if( m_socketRouter )
	{
		m_socketRouter->quit();
		delete m_socketRouter;
		m_socketRouter = BD_NULL;
	}	

	if(BD_NULL != m_params.m_socket)
	{
		const bdBool socketCloseResult = m_params.m_socket->close();
		if(false == socketCloseResult)
		{
			BD_WARN(BD_LOG_LEVEL, "Game socket failed to close with an error");
		}
		delete m_params.m_socket;
		m_params.m_socket = BD_NULL;
	}

	if (m_ipDiscClient)
	{
		m_ipDiscClient->quit();
		delete m_ipDiscClient;
		m_ipDiscClient = BD_NULL;
	}
	if (m_natTypeDiscClient)
	{
		m_natTypeDiscClient->quit();
		delete m_natTypeDiscClient;
		m_natTypeDiscClient = BD_NULL;
	}

	if(m_getHostByName)
	{
		// Host name lookup code should be quited to avoid memory leak.
		// It is safe to call quit more than once.
		m_getHostByName->quit();
		delete m_getHostByName;
		m_getHostByName = BD_NULL;
	}

	m_status = BD_NET_STOPPED;
	m_currentNatTravAddrIndex = 0;
	m_currentNatTravHostIndex = 0;
	m_params.m_localAddresses.clear();
	m_params.m_natTravHosts.clear();
	m_natTravAddrs.clear();
}
*/

void bdNetImpl::stop()
{
	if (bdConnectionStore::BD_CONNECTION_STORE_UNINITIALIZED != m_connectionStore.getStatus())
		m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);

	if (bdUPnP::BD_UPNP_UNINITIALISED != m_UPnP.getState())
		m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT);

	if (NULL != m_socketRouter)
	{
		m_socketRouter->quit();
		delete m_socketRouter;
		m_socketRouter = BD_NULL;
	}

	if(BD_NULL != m_params.m_socket)
	{
		const bdBool socketCloseResult = m_params.m_socket->close();
		if(false == socketCloseResult)
		{
			BD_WARN(BD_LOG_LEVEL, "Game socket failed to close with an error");
		}
		delete m_params.m_socket;
		m_params.m_socket = BD_NULL;
	}

	if (m_ipDiscClient)
	{
		m_ipDiscClient->quit();
		delete m_ipDiscClient;
		m_ipDiscClient = BD_NULL;
	}
	if (m_natTypeDiscClient)
	{
		m_natTypeDiscClient->quit();
		delete m_natTypeDiscClient;
		m_natTypeDiscClient = BD_NULL;
	}

	if(m_getHostByName)
	{
		// Host name lookup code should be quited to avoid memory leak.
		// It is safe to call quit more than once.
		m_getHostByName->quit();
		delete m_getHostByName;
		m_getHostByName = BD_NULL;
	}

	m_status = BD_NET_STOPPED;
	m_currentNatTravAddrIndex = 0;
	m_currentNatTravHostIndex = 0;
	m_params.m_localAddresses.clear();
	m_params.m_natTravHosts.clear();
	m_natTravAddrs.clear();
}


bdNetImpl::bdNetStatus bdNetImpl::getStatus() const
{
	return m_status;
}

bdBool bdNetImpl::sendAll()
{
	const bdBool result = m_connectionStore.flushAll();
	if(false == result)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to flush all connections in the connection store");
	}
	return result;
}

bdBool bdNetImpl::receiveAll()
{
	const bdBool result = m_connectionStore.receiveAll();
	if(false == result)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to receive data for all connections in the connection store");
	}
	return result;
}

void bdNetImpl::dispatchAll()
{
	m_connectionStore.dispatchAll();
}

bdConnectionStore *bdNetImpl::getConnectionStore()
{
	return &m_connectionStore;
}

bdCommonAddrRef bdNetImpl::getLocalCommonAddr() const
{
	bdCommonAddrRef local = BD_NULL;
	if( m_socketRouter != BD_NULL )
	{
		local = m_socketRouter->getLocalCommonAddr();
	}
	return local;
}

const bdNetStartParams& bdNetImpl::getParams()
{
	return m_params;
}

bdNetImpl::bdNetImpl()
: m_socketRouter(BD_NULL),
  m_status(BD_NET_STOPPED),
  m_ipDiscClient(BD_NULL),
  m_natTypeDiscClient(BD_NULL),
  m_getHostByName(BD_NULL),
  m_currentNatTravAddrIndex(0),
  m_currentNatTravHostIndex(0),
  m_upnpCollisionRetryCount(0)
{
}

bdNetImpl::~bdNetImpl()
{
	if (m_getHostByName)
	{
		// Host name lookup code should be quited to avoid memory leak.
		// It is safe to call quit more than once.
		m_getHostByName->quit();
		delete m_getHostByName;
		m_getHostByName = BD_NULL;
	}
	if (m_natTypeDiscClient)
	{
		m_natTypeDiscClient->quit();
		delete m_natTypeDiscClient;
		m_natTypeDiscClient = BD_NULL;
	}
	if (m_ipDiscClient)
	{
		m_ipDiscClient->quit();
		delete m_ipDiscClient;
		m_ipDiscClient = BD_NULL;
	}
	if( m_socketRouter )
	{
		m_socketRouter->quit();
		delete m_socketRouter;
		m_socketRouter = BD_NULL;
	}
}


bdBool bdNetImpl::findFreePort(bdAddr& addr)
{
	bdPort port = addr.getPort();
	bdBool success = false;

	for(bdUInt i = 0; i < 100; i++)
	{
		bdSocket socket;
		const bdBool blocking = false;
		const bdBool socketCreateResult = socket.create(blocking);
		if(false == socketCreateResult)
		{
			BD_WARN(BD_LOG_LEVEL, "Create socket failed ");
			break;
		}
		else 
		{
			const bdAddr tryAddr(addr.getAddress(), port);

			const bdSocketStatusCode bindResult = socket.bind(tryAddr);
			if(BD_NET_SUCCESS == bindResult)
			{
				const bdBool socketCloseResult = socket.close();
				if(false == socketCloseResult)
				{
					BD_WARN(BD_LOG_LEVEL, "Socket failed to close with an error");
				}
				success = true;
				addr = tryAddr;
				break;
			}
			else if (BD_NET_ADDRESS_IN_USE == bindResult)
			{
				// We expect the socket closing to fail if the bind failed because 
				// of the addr already being in use.
				// So we will flag success as unexpected!
				const bdBool socketCloseResult = socket.close();
				if(true == socketCloseResult)
				{
					BD_WARN(BD_LOG_LEVEL, "Socket bind failed, but subsequent close succeeded!");
				}
			}
			else
			{
				const bdBool socketCloseResult = socket.close();
				if(false == socketCloseResult)
				{
					BD_WARN(BD_LOG_LEVEL, "Socket failed to close with an error");
				}
			}
			port++;
		}
	}

	return success;
}

bdBool bdNetImpl::getBindAddr(bdAddr &addr)
{
	bdBool result = false;

	if (false == m_params.m_useAnyIP )
	{
		if(m_params.m_localAddresses.getSize() > 1)
		{
			BD_WARN(BD_LOG_LEVEL, "When binding to a specific IP, entries in m_localAddresses other than the zeroth are ignored");
		}

		if(m_params.m_localAddresses.getSize() > 0)
		{
			addr = bdAddr(m_params.m_localAddresses[0], m_params.m_gamePort);
			result = true;
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "When binding to a specific IP, m_localAddresses should not be empty");
		}
	}
	else
	{
		addr = bdAddr(bdInetAddr::Any(), m_params.m_gamePort);
		result = true;
	}

	return result;
}

bdSocketRouter* bdNetImpl::getSocketRouter()
{
	return m_socketRouter;
}
