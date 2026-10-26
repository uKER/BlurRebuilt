// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: UPnP Nat traversal module

#include <bdNet/bdUPnP/bdUPnP.h>
#include <bdNet/bdUPnP/bdUPnPCommon.h>

#define BD_LOG_LEVEL "bdnet/upnp"

bdUPnP::bdUPnP()
: m_state(BD_UPNP_UNINITIALISED),
  m_gamePort(BD_INVALID_PORT),
  m_localAddrs(BD_NULL),
  m_bytesReceived(0),
  m_numDiscoveryAttempts(0),
  m_shutdownRequested(false)
{
}

bdUPnP::~bdUPnP()
{
}

bdBool bdUPnP::init(bdPort port,
					bdArray<bdInetAddr> *localAddrs,
					bdUPnPConfig config)
{
	bdBool result = false;

	if( config.m_disabled )
	{
		BD_INFO(BD_LOG_LEVEL , "UPnP disabled.");
		m_state = BD_UPNP_FINISHED;
		result = true;
	}
	else if( m_state == BD_UPNP_UNINITIALISED )
	{

		if( !localAddrs->isEmpty() )
		{
			m_localAddrs = localAddrs;
			m_gamePort = port;
			m_config = config;
			m_config.sanityCheckConfig();

			if( !m_config.m_gatewayAddr.isValid() )
			{
				BD_WARN(BD_LOG_LEVEL, "Starting UPnP without setting a gateway address in the config. Will use first UPnP device found");
			}

			m_state = BD_UPNP_INITIALISED;
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL , "Cannot start class unless at least one local common addr is supplied..");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Cannot initialize class unless it is in the uninitialized state.");
	}

	return result;
}

void bdUPnP::pump()
{
	if( m_shutdownRequested )
	{
		m_shutdownRequested = false; // So that we do not keep setting state to shutting down.
		m_state = BD_UPNP_SHUTTING_DOWN;
	}

	switch (m_state)
	{
		case BD_UPNP_UNINITIALISED:
		{
			BD_WARN(BD_LOG_LEVEL , "Pumping an uninitialized device has no effect.");
		}
		break;
		case BD_UPNP_INITIALISED:
		{
			if( setupDiscoverySocket() )
			{
				if( startDiscovery() )
				{
					m_bytesReceived = 0;
					bdMemset( m_readBuffer, BD_NULL, BD_MAX_UPNP_DISCOVERY_REPLY+1 );
					m_discoveryTimer.reset();
					m_discoveryTimer.start();
					m_numDiscoveryAttempts++;
					m_state = BD_UPNP_DISCOVERING_DEVICES;
				}
				else
				{
					m_state = BD_UPNP_FAILED; 
				}
			}
			else
			{
				m_state = BD_UPNP_FAILED; 
			}
		}
		break;
		case BD_UPNP_DISCOVERING_DEVICES:
		{
			if ( m_discoveryTimer.getElapsedTimeInSeconds() >= m_config.m_discoveryTimeout )
			{
				if( m_cachedDevice.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_INITIALISED )
				{
					BD_INFO(BD_LOG_LEVEL, "Device discovery timed out after %fs. Using cached device", m_discoveryTimer.getElapsedTimeInSeconds());
					m_device = m_cachedDevice;

					BD_ASSERT( m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_INITIALISED, "bdUPnPDevice copied incorrectly" );
					m_state = BD_UPNP_DISCOVERED_DEVICE;
				}
				else
				{
					if( m_discoverySocket.close() )
					{
						if(m_numDiscoveryAttempts <= m_config.m_discoveryRetries)
						{
							// Go back and retry the discovery	
							BD_INFO(BD_LOG_LEVEL, "Discovery timed out. Retrying.");
							m_state = BD_UPNP_INITIALISED;
						}
						else
						{
							BD_INFO(BD_LOG_LEVEL, "Discovery timed out. Max retries reached. Stopping");
							m_state = BD_UPNP_FINISHED;
						}
					}
					else
					{
						m_state = BD_UPNP_FAILED;
					}
				}
			}
			else
			{
				bdUPnPDevice discoveredDevice;
				BD_ASSERT( discoveredDevice.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_UNINITIALISED, "bdUPnPDevice constructed incorrectly" );

				bdBool ok = checkForDiscoveredDevices( discoveredDevice );
				if( discoveredDevice.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_INITIALISED && ok )
				{
					if( m_config.m_gatewayAddr.isValid() && 
					  ( discoveredDevice.getDeviceAddr().getAddress() == m_config.m_gatewayAddr ))
					{
						BD_INFO(BD_LOG_LEVEL , "UPnP capable device discovered which matches gateway. Finished Discovery.");
						m_device = discoveredDevice;
						m_state = BD_UPNP_CONFIGURING_DEVICE;
					}
					else
					{
						if( m_cachedDevice.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_UNINITIALISED )
						{
							BD_INFO(BD_LOG_LEVEL , "UPnP capable device discovered which does not match gateway. Setting as cached device." );
							m_cachedDevice = discoveredDevice;
						}
						else
						{
							discoveredDevice.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
							BD_INFO(BD_LOG_LEVEL , "UPnP capable device discovered which does not match gateway. Ignoring." );
						}
					}
				}
				else if( !ok )
				{
					m_state = BD_UPNP_FAILED;
				}
			}
		}
		break;
		case BD_UPNP_DISCOVERED_DEVICE:
		{
			if( m_discoverySocket.close() )
			{
				m_state = BD_UPNP_CONFIGURING_DEVICE;
			}
			else
			{
				m_state = BD_UPNP_FAILED;
			}
		}
		break;
		case BD_UPNP_CONFIGURING_DEVICE:
		{
			m_device.pump();
			if( m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FINISHED )
			{
				m_state = BD_UPNP_FINISHED;
			}
			else if(m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FAILED ||
				    m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_UNINITIALISED )
			{
				m_state = BD_UPNP_FAILED;
			}
		}
		break;
		case BD_UPNP_SHUTTING_DOWN:
		{
			m_device.pump();
			if( m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_UNINITIALISED )
			{
				m_state = BD_UPNP_UNINITIALISED;
				m_gamePort = BD_INVALID_PORT;
				m_localAddrs = BD_NULL;
				m_config.reset();
				m_discoverySocket.close();
				m_bytesReceived = 0;
				m_device = bdUPnPDevice();
				m_cachedDevice = bdUPnPDevice();
				m_discoveryTimer.reset();
			}
			else if( m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FAILED )
			{
				m_state = BD_UPNP_FAILED;
			}
		}
		break;
		case BD_UPNP_FINISHED:
		{
			// Nothing left to do.
		}
		break;
		case BD_UPNP_FAILED:
		{
			// Cleanup.
			m_discoverySocket.close();
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL, "Pumping UPnP in Unknown State");
			m_state = BD_UPNP_FAILED;
		}
	}
}


bdBool bdUPnP::isRunning()
{
	bdBool isRunning =	((bdUPnP::BD_UPNP_FINISHED != m_state) &&
						 (bdUPnP::BD_UPNP_FAILED != m_state) &&
						 (bdUPnP::BD_UPNP_UNINITIALISED != m_state)
						);
	return isRunning;
}

void bdUPnP::startShutdown(const bdUPnPDevice::bdUPnPDeviceShutdownType shutdownType )
{
	if(	shutdownType == bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT ||
		shutdownType == bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE )
	{
		if( m_state != BD_UPNP_SHUTTING_DOWN &&
			m_state != BD_UPNP_UNINITIALISED )
		{
			m_device.startShutdown( shutdownType );
			m_shutdownRequested = true;
		}
	}
}

bdUPnP::bdUPnPState bdUPnP::getState() const
{
	return m_state;
}

bdBool bdUPnP::getExternalAddr(bdAddr& externalAddr ) const
{
	bdBool result = false;
	externalAddr = m_device.getExternalAddr();
	if( externalAddr.getAddress().isValid() )
	{
		result = true;
	}
	return result;
}


bdUPnPDevice::bdUPnPPortStatus bdUPnP::getPortStatus() const
{
	if( m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FINISHED )
	{
		return m_device.getPortStatus();
	}
	else
	{
		return bdUPnPDevice::BD_UPNP_PORT_STATUS_UNKNOWN;
	}
}

bdBool bdUPnP::setupDiscoverySocket()
{
	bdBool result = false;
	const bdBool blocking = false;

	if (m_discoverySocket.create(blocking))
	{
		result = true;
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to create discovery socket.");
	}

	return result;
}

bdBool bdUPnP::startDiscovery()
{
	bdBool result = true;

	// The discovery message buffer.
	bdByte8 messageBuffer[sizeof(s_discoveryString) + MAX_SERVICE_LEN];

	// discover both IP connection services and PPP connection services
	for (bdUInt i = 0; i < 2; i++)
	{
		const bdInt discLength = bdSnprintf(messageBuffer, 
											sizeof(messageBuffer), 
											s_discoveryString,
											(i == 0 ? IP_SERVICE : PPP_SERVICE) );

		const bdAddr discoveryAddr (s_discoveryAddress);
		const bdInt bytesSent = m_discoverySocket.sendTo(discoveryAddr, messageBuffer, static_cast<bdUInt>(discLength));
		if (bytesSent != discLength)
		{
			result = false;
			BD_ERR(BD_LOG_LEVEL, "Failed to send discovery packet");
		}
	}

	return result;
}

bdBool bdUPnP::checkForDiscoveredDevices( bdUPnPDevice& discoveredDevice )
{
	bdBool result = false;

	bdAddr incomingAddress;
	const bdInt received = m_discoverySocket.receiveFrom(incomingAddress, m_readBuffer, BD_MAX_UPNP_DISCOVERY_REPLY);

	if( received > 0 && received <= BD_MAX_UPNP_DISCOVERY_REPLY )
	{
		m_bytesReceived = received;
		m_readBuffer[m_bytesReceived] = 0;
		result = handleDiscoveryResponse( discoveredDevice );
	}
	else if ( received == BD_NET_WOULD_BLOCK )
	{
		result = true;
	}

	return result;
}

bdBool bdUPnP::handleDiscoveryResponse( bdUPnPDevice& discoveredDevice )
{
	bdBool ok = false;

	bdAddr discoveredDeviceAddr;
	bdByte8* fetchLoc = BD_NULL;
	bdUInt fetchLen = 0;

	ok = extractDeviceInfo( discoveredDeviceAddr, &fetchLoc, fetchLen );

	if( ok )
	{
		bdNChar8 addressBuffer[BD_ADDR_STRING_SIZE];
		discoveredDeviceAddr.toString(addressBuffer, BD_ADDR_STRING_SIZE);
		BD_INFO(BD_LOG_LEVEL, "Discovered Device at address %s", addressBuffer);

		ok = discoveredDevice.init(	m_config, 
									m_localAddrs, 
									discoveredDeviceAddr, 
									fetchLoc, 
									fetchLen, 
									m_gamePort);
	}

	return ok;
}


bdBool bdUPnP::extractDeviceInfo( bdAddr& deviceAddr, bdByte8** fetchLoc, bdUInt& fetchLen )
{
	bdBool result = false;

	// search the response for the device URL starting with "http://"
	bdByte8 *httpString = bdStrstr(m_readBuffer, HTTP);
	if (httpString != BD_NULL)
	{
		// advance to the actual address which follows "http://"
		httpString += strlen(HTTP);

		// find the / character following the address
		// to deliminate address from fetch location
		bdByte8 *slashLoc = bdStrstr(httpString, "/" );
		if (slashLoc != BD_NULL)
		{
			// grab address string which occurs between "http://" and the first /
			bdUInt addrStringLength = bdUInt(slashLoc - httpString);
			bdByte8 addrString[BD_ADDR_STRING_SIZE];

			if( addrStringLength > 0 && addrStringLength < BD_ADDR_STRING_SIZE )
			{
				bdStrlcpy(addrString, httpString, addrStringLength + 1);
				deviceAddr = bdAddr(addrString);
				
				// grab fetch location string which occurs after
				// the address and before the line ends
				bdByte8 *lineEnd = bdStrchr(httpString, '\r');
				if (lineEnd != BD_NULL)
				{
					*fetchLoc = slashLoc;
					fetchLen = bdUInt( lineEnd - slashLoc );
					result = true;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "No carriage return after fetch location");
				}
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "No fetch path found after URL");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "no http:// token found in discovery response");
	}

	return result;
}
