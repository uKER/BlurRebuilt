// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdUPnP/bdUPnPDevice.h>
#include <bdNet/bdUPnP/bdUPnPCommon.h>

#define BD_LOG_LEVEL "bdnet/upnpdevice"


bdUPnPDevice::bdUPnPDevice()
: m_state( BD_UPNP_DEVICE_UNINITIALISED ),
  m_shutdownRequested(false),
  m_portStatus( BD_UPNP_PORT_STATUS_UNKNOWN ),
  m_localAddrs(BD_NULL),
  m_isIp(true),
  m_gamePort(BD_INVALID_PORT),
  m_requestSize(0),
  m_bytesReceived(0)
{
}

bdUPnPDevice::~bdUPnPDevice()
{
}

bdBool bdUPnPDevice::init(	const bdUPnPConfig config, 
							bdArray<bdInetAddr> *localAddrs,
							const bdAddr deviceAddress,
							const bdByte8 *const fetchLocation,
							const bdUInt fetchLocationLength,
							const bdPort port)
{
	bdBool result = false;

	if( m_state == BD_UPNP_DEVICE_UNINITIALISED )
	{
		m_deviceAddr = deviceAddress;
		m_gamePort = port;
		m_config = config;
		m_localAddrs = localAddrs;
		bdSnprintf(m_fetchLocation, fetchLocationLength+1, fetchLocation);

		m_state = BD_UPNP_DEVICE_INITIALISED;
		m_portStatus = BD_UPNP_PORT_STATUS_UNKNOWN;
		m_shutdownRequested = false;
		if( m_streamSocket.isConnected() )
		{
			m_streamSocket.close();
		}
		result = true;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Cannot initialize class unless it is in the uninitialized state.");
	}

	return result;
}

void bdUPnPDevice::pump()
{
	if( m_shutdownRequested )
	{
		m_shutdownRequested = false; // So that we do not keep setting state to shutting down.
		m_state = BD_UPNP_DEVICE_SHUTTING_DOWN;
	}

	switch (m_state)
	{
		case BD_UPNP_DEVICE_UNINITIALISED:
		{
			BD_WARN(BD_LOG_LEVEL , "Pumping an uninitialized device has no effect.");
		}
		break;
		case BD_UPNP_DEVICE_INITIALISED:
		{
			prepareGetDescriptionRequest();
			if( connectToDevice() )
			{
				m_state = BD_UPNP_DEVICE_SENDING_DESCRIPTION_REQ;
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL , "Failed to connect to device.");
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_SENDING_DESCRIPTION_REQ:
		{
			bdBool isComplete = false;
			bdBool ok = pumpSendRequest( isComplete );

			if( isComplete && ok )
			{
				if( setLocalConnectedAddr() )
				{
					m_state = BD_UPNP_DEVICE_GETTING_DESCRIPTION;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to set the local addr.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
			else if( !ok )
			{
				BD_WARN(BD_LOG_LEVEL , "Failed to send device description request.");
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_GETTING_DESCRIPTION:
		{
			bdBool receiving = pumpReceive();
			if( !receiving )
			{
				if( parseDescriptionResponse() )
				{
					m_state = BD_UPNP_DEVICE_GOT_DESCRIPTION;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to parse the device description.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
		}
		break;
		case BD_UPNP_DEVICE_GOT_DESCRIPTION:
		{
			BD_INFO(BD_LOG_LEVEL, "Successfully received the device description" );
			
			prepareGetExternalIPRequest();
			if( connectToDevice() )
			{
				m_state = BD_UPNP_DEVICE_SENDING_IP_REQ;
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL , "Failed to connect to device.");
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_SENDING_IP_REQ:
		{
			bdBool isComplete = false;
			bdBool ok = pumpSendRequest( isComplete );

			if( isComplete && ok )
			{
				m_state = BD_UPNP_DEVICE_DISCOVERING_IP;
			}
			else if( !ok )
			{
				BD_WARN(BD_LOG_LEVEL , "Failed to send the exernal IP request.");
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_DISCOVERING_IP:
		{
			bdBool receiving = pumpReceive();
			if( !receiving )
			{
				if( parseGetExternalIPResponse() )
				{
					m_state = BD_UPNP_DEVICE_DISCOVERED_IP;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to parse the external IP response.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
		}
		break;
		case BD_UPNP_DEVICE_DISCOVERED_IP:
		{
			if( m_config.m_runMode == bdUPnPConfig::BD_UPNP_DO_PORT_MAPPING )
			{
				prepareGetMappingsRequest();
				if( connectToDevice() )
				{
					m_state = BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to connect to device.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
			else
			{
				// In the BD_UPNP_EXTERNAL_IP_ONLY run mode we are finished after 
				// discovering the external ip of the device.
				BD_INFO(BD_LOG_LEVEL, "UPnP device configuration complete");
				m_state = BD_UPNP_DEVICE_FINISHED;
			}
		}
		break;
		case BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ:
		{
			bdBool isComplete = false;
			bdBool ok = pumpSendRequest( isComplete );

			if( isComplete && ok )
			{
				m_state = BD_UPNP_DEVICE_DISCOVERING_PORT_MAPPING;
			}
			else if( !ok )
			{
				BD_WARN(BD_LOG_LEVEL , "Failed to send port query request.");
				m_state = BD_UPNP_DEVICE_FAILED;
			}			
		}
		break;
		case BD_UPNP_DEVICE_DISCOVERING_PORT_MAPPING:
		{
			bdBool receiving = pumpReceive();
			bdBool mappingExists = false;
			bdBool mappingIsMine = false;

			if( !receiving )
			{
				if( parseGetMappingsResponse(mappingExists, mappingIsMine) )
				{
					if( mappingExists && mappingIsMine )
					{
						m_portStatus = BD_UPNP_PORT_MAPPED;
					}
					else if( mappingExists && !mappingIsMine )
					{
						m_portStatus = BD_UPNP_PORT_COLLISION_FOUND;							
					}
					else
					{
						m_portStatus = BD_UPNP_PORT_NOT_MAPPED;
					}

					m_state = BD_UPNP_DEVICE_DISCOVERED_PORT_MAPPING;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to parse port query response.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
		}
		break;
		case BD_UPNP_DEVICE_DISCOVERED_PORT_MAPPING:
		{
			if( m_portStatus == BD_UPNP_PORT_MAPPED )
			{
				BD_INFO(BD_LOG_LEVEL, "Usable Port Mapping found. UPnP device configuration complete.");
				m_state = BD_UPNP_DEVICE_FINISHED;
			}
			else if( m_portStatus == BD_UPNP_PORT_COLLISION_FOUND )
			{
				BD_INFO(BD_LOG_LEVEL, "Port Collision Detected. UPnP device configuration complete");
				m_state = BD_UPNP_DEVICE_FINISHED;
			}
			else if( m_portStatus == BD_UPNP_PORT_NOT_MAPPED )
			{
				prepareCreatePortMappingRequest();
				if( connectToDevice() )
				{
					BD_INFO(BD_LOG_LEVEL, "No port mappings found. Creating one on port %u.", m_gamePort );
					m_state = BD_UPNP_DEVICE_SENDING_MAPPING_REQ;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to connect to device.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Unexpected port status" );
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_SENDING_MAPPING_REQ:
		{
			bdBool isComplete = false;
			bdBool ok = pumpSendRequest( isComplete );

			if( isComplete && ok )
			{
				m_state = BD_UPNP_DEVICE_MAPPING_PORT;
			}
			else if( !ok )
			{
				BD_WARN(BD_LOG_LEVEL , "Failed to send port mapping request.");
				m_state = BD_UPNP_DEVICE_FAILED;
			}				
		}
		break;
		case BD_UPNP_DEVICE_MAPPING_PORT:
		{
			bdBool receiving = pumpReceive();
			if( !receiving )
			{
				if( confirmHttpSuccess() )
				{
					m_portStatus = BD_UPNP_PORT_MAPPED;
					m_state = BD_UPNP_DEVICE_MAPPED_PORT;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to map port.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
		}
		break;
		case BD_UPNP_DEVICE_MAPPED_PORT:
		{
			if( m_portStatus == BD_UPNP_PORT_MAPPED )
			{
				BD_INFO(BD_LOG_LEVEL, "Port successfully mapped. UPnP device configuration complete");
				m_state = BD_UPNP_DEVICE_FINISHED;
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Unexpected port status" );
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ:
		{
			bdBool isComplete = false;
			bdBool ok = pumpSendRequest( isComplete );

			if( isComplete && ok )
			{
				m_state = BD_UPNP_DEVICE_UNMAPPING_PORT;
			}
			else if( !ok )
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to send port unmapping request." );
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_UNMAPPING_PORT:
		{
			bdBool receiving = pumpReceive();
			if( !receiving )
			{
				if( confirmHttpSuccess() )
				{
					BD_INFO(BD_LOG_LEVEL, "Port unmapping successful on port %u", m_gamePort);
					m_portStatus = BD_UPNP_PORT_NOT_MAPPED;
					m_state = BD_UPNP_DEVICE_UNMAPPED_PORT;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL , "Failed to unmap port.");
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
		}
		break;
		case BD_UPNP_DEVICE_UNMAPPED_PORT:
		{
			if( m_portStatus == BD_UPNP_PORT_NOT_MAPPED )
			{
				m_shutdownType = BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE;
				m_state = BD_UPNP_DEVICE_SHUTTING_DOWN;
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Unexpected port status" );
				m_state = BD_UPNP_DEVICE_FAILED;
			}
		}
		break;
		case BD_UPNP_DEVICE_SHUTTING_DOWN:
		{
			m_streamSocket.close();

			if( m_shutdownType == BD_UPNP_DEVICE_SHUTDOWN_ELEGANT 
				&& m_portStatus == BD_UPNP_PORT_MAPPED )
			{
				preparePortUnMappingRequest();
				if( connectToDevice() )
				{
					m_state = BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to connect to device." );
					m_state = BD_UPNP_DEVICE_FAILED;
				}
			}
			else
			{
				m_state = BD_UPNP_DEVICE_UNINITIALISED;
				m_shutdownRequested = false;
				m_portStatus = BD_UPNP_PORT_STATUS_UNKNOWN;
				m_deviceAddr = bdAddr();
				m_localAddrs = BD_NULL;
				m_localConnectedAddr = bdInetAddr();
				m_streamSocket.close();
				m_config = bdUPnPConfig();
				m_gamePort = BD_INVALID_PORT;
				m_externalDeviceAddr = bdInetAddr();
				m_requestSize = 0;
				m_bytesReceived = 0;
			}
		}
		break;
		case BD_UPNP_DEVICE_FINISHED:
		{
			// Finished. Nothing to do.
		}
		break;
		case BD_UPNP_DEVICE_FAILED:
		{
			// Failed. Cleanup.
			if( m_streamSocket.isConnected() )
			{
				m_streamSocket.close();
			}
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL, "Pumping Device in Unknown State");
			m_state = BD_UPNP_DEVICE_FAILED;
		}
	}
}

void bdUPnPDevice::startShutdown(const bdUPnPDeviceShutdownType shutdownType)
{
	if(	shutdownType == BD_UPNP_DEVICE_SHUTDOWN_ELEGANT 
		||  shutdownType == BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE )
	{
		if( m_state != BD_UPNP_DEVICE_UNINITIALISED &&
			m_state != BD_UPNP_DEVICE_SHUTTING_DOWN &&
			m_state != BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ &&
			m_state != BD_UPNP_DEVICE_UNMAPPING_PORT &&
			m_state != BD_UPNP_DEVICE_UNMAPPED_PORT )
		{
			m_shutdownRequested = true;
			m_shutdownType = shutdownType;
		}
	}
}

bdAddr bdUPnPDevice::getDeviceAddr() const
{
	return m_deviceAddr;
}

bdAddr bdUPnPDevice::getExternalAddr() const
{
	return bdAddr(m_externalDeviceAddr, m_gamePort );
}

bdUPnPDevice::bdUPnPPortStatus bdUPnPDevice::getPortStatus() const
{
	return m_portStatus;
}

bdUPnPDevice::bdUPnPDeviceState bdUPnPDevice::getDeviceState() const
{
	return m_state;
}

bdBool bdUPnPDevice::connectToDevice()
{
	bdBool result = false;
	const bdBool blocking = false;
	if ( m_streamSocket.create(blocking) )
	{
		// Default to BD_DEFAULT_UPNP_CONNECTION_PORT if no port was 
		// specified in the connection address.
		if (m_deviceAddr.getPort() == BD_INVALID_PORT )
		{
			m_deviceAddr.setPort(BD_DEFAULT_UPNP_CONNECTION_PORT);
		}

		// Establish connected TCP socket to given address
		bdSocketStatusCode sockStatus = m_streamSocket.connect(m_deviceAddr);
		if (sockStatus == BD_NET_SUCCESS || sockStatus == BD_NET_WOULD_BLOCK)
		{
			result = true;
			m_connectTimer.reset();
			m_connectTimer.start();
		}
	}
	return result;
}

bdBool bdUPnPDevice::setLocalConnectedAddr()
{
	bdBool result = false;

	// This is enabled on platforms that can have multiple NICs
	// as the UPnP code must then instruct the NAT to map to the
	// correct NIC/bdInAddr
#if defined BD_PLATFORM_WIN32

	if( m_streamSocket.isConnected())
	{
		// get the IP associated with this connection for the local
		// address parameter in port mapping.
		bdInAddr addr;
		if (m_streamSocket.getSocketAddr(addr))
		{
			m_localConnectedAddr.set(addr);
			result = true;
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL , "Local addr setup should only be attempted when connected.");
	}
#else
	if( !m_localAddrs->isEmpty() )
	{
		m_localConnectedAddr = ((*m_localAddrs)[0]);
		result = true;
	}
#endif

	return result;
}

void bdUPnPDevice::setupReceive()
{
	m_bytesReceived = 0;
	bdMemset( m_readBuffer, BD_NULL, BD_UPNP_MAX_DEVICE_REPLY_SIZE );

	m_responseTimer.reset();
	m_responseTimer.start();
}

bdBool bdUPnPDevice::pumpSendRequest( bdBool& isComplete )
{
	isComplete = false;
	bdBool result = true;

	if( m_streamSocket.isConnected() )
	{
		bdUInt requestLen = bdUInt( bdStrlen( m_requestBuffer ) );
		BD_ASSERT( requestLen == m_requestSize , "Corrupted Buffer" );

		bdInt sendResult = m_streamSocket.send(m_requestBuffer, m_requestSize);
		if ( sendResult > 0 )
		{
			setupReceive();
			isComplete = true;
		}
		else if( sendResult != BD_NET_WOULD_BLOCK )
		{
			BD_WARN(BD_LOG_LEVEL, "Error sending to UPnP Device");
			m_streamSocket.close();
			result = false;				
		}
	}
	else if( m_connectTimer.getElapsedTimeInSeconds() > m_config.m_connectTimeout )
	{ 
		BD_WARN(BD_LOG_LEVEL, "Timed out while connecting to UPnP device");
		m_streamSocket.close();
		result = false;
	}

	return result;
}

bdBool bdUPnPDevice::pumpReceive()
{
	bdBool result = false;

	if( m_responseTimer.getElapsedTimeInSeconds() <= m_config.m_responseTimeout )
	{
		// receive on the connect description socket
		const bdInt bytesReceived = m_streamSocket.recv((m_readBuffer + m_bytesReceived),
												(BD_UPNP_MAX_DEVICE_REPLY_SIZE - m_bytesReceived));

		if (bytesReceived > 0)
		{
			m_readBuffer[m_bytesReceived + bytesReceived] = BD_NULL;

			// advance position in buffer to account for new data received
			m_bytesReceived += bytesReceived;
			result = true;
		}
		else if (bytesReceived == BD_NET_WOULD_BLOCK)
		{
			result = true;
		}
		else if (bytesReceived != BD_NET_CONNECTION_CLOSED)
		{
			BD_INFO(BD_LOG_LEVEL, "Error recieving from socket %d", bytesReceived);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Timed out while getting response from device");
	}

	// We are no longer receiving so close the socket.
	if( result == false )
	{
		m_streamSocket.close();
	}

	return result;
}

bdBool bdUPnPDevice::confirmHttpSuccess()
{
	return ( bdStrstr(m_readBuffer, HTTP_OK) != BD_NULL );
}

bdBool bdUPnPDevice::checkForFinishedSoapResponse()
{
	return ( bdStrstr(m_readBuffer, SOAP_ENVELOPE_END_TAG) != BD_NULL );
}

void bdUPnPDevice::genUPnPCommand(	const bdByte8 *const command, const bdByte8 *const arguments )
{
	bdByte8 soapBuffer[s_maxSoapSize];

	/* soapMessage
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n"
		"<u:%(command)s xmlns:u=\"urn:schemas-upnp-org:service:%(serviceType)s\">\r\n"
		"%(arguments)s"
		"</u:%(command)s>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n";
	*/
	bdUInt actualSoapSize = bdSnprintf(soapBuffer, 
								s_maxSoapSize, 
								s_soapMessage,
								command,
								(m_isIp ? IP_SERVICE : PPP_SERVICE),
								arguments,
								command );

	bdByte8 addressBuff[BD_ADDR_STRING_SIZE];
	m_deviceAddr.toString(addressBuff, BD_ADDR_STRING_SIZE);

	bdByte8 numBuff[10];
	bdInt numLen = bdSnprintf(numBuff, 9, "%u", actualSoapSize);

	bdUInt totalRequestSize = bdUInt(	(bdStrlen(s_httpPost) 
										+ bdStrlen(m_controlURL) 
										+ bdStrlen(addressBuff) 
										+ bdStrlen( m_isIp ? IP_SERVICE : PPP_SERVICE ) 
										+ bdStrlen( command ) 
										+ numLen // length of the actualSoapSize number when converted to a string
										+ actualSoapSize) );

	BD_ASSERT( totalRequestSize < BD_UPNP_MAX_DEVICE_REQUEST_SIZE, "Request is too large for buffer" );

	/* httpPost
		"POST %(m_controlURL)s HTTP/1.1\r\n"
		"HOST: %(addressBuff)s\r\n"
		"SOAPACTION: \"urn:schemas-upnp-org:service:%(service)s#%(getExternalAddressCommand)s\"\r\n"
		"CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
		"CONTENT-LENGTH: %(actualSoapSize)d\r\n\r\n"
		"%(soapCommand)s";
	*/
	m_requestSize = bdUInt( bdSnprintf(	m_requestBuffer, 
										BD_UPNP_MAX_DEVICE_REQUEST_SIZE, 
										s_httpPost, 
										m_controlURL, 
										addressBuff,
										(m_isIp ? IP_SERVICE : PPP_SERVICE), 
										command, 
										actualSoapSize,
										soapBuffer) );
}

bdBool bdUPnPDevice::extractTag( bdByte8* tag, bdByte8* buffer, bdByte8** value, bdUInt& valueLen)
{
	bdBool result = false;
	*value = BD_NULL;
	valueLen = 0;

	bdByte8* elementStart = BD_NULL;
	bdByte8* elementEnd = BD_NULL;

	elementStart = bdStrstr( buffer, tag );
	if( elementStart != BD_NULL )
	{
		// Find the end of the start tag
		elementStart = bdStrstr(elementStart, CLOSE_TAG);
		if( elementStart != BD_NULL )
		{
			// Advance to the start of the element
			elementStart++;

			//Now find the end of the element
			elementEnd = bdStrstr(elementStart, OPEN_END_TAG);
			if( elementEnd != BD_NULL )
			{
				// Check for an element
				bdUInt elementLen = bdUInt(elementEnd - elementStart);
				if( elementLen > 0 )
				{
					*value = elementStart;
					valueLen = elementLen;
					result = true;
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Tag %s is improperly formatted", tag);
			}
		}
	}
	return result;
}

void bdUPnPDevice::prepareGetDescriptionRequest()
{
	bdNChar8 deviceAddrBuffer [BD_ADDR_STRING_SIZE];
	const bdUWord addrStringSize = m_deviceAddr.toString(deviceAddrBuffer, BD_ADDR_STRING_SIZE);

	// Compute the total expected size of the request.
	bdUInt totalRequestSize = bdUInt(bdStrlen(s_getRequest) + bdStrlen(m_fetchLocation) + addrStringSize );
	BD_ASSERT( totalRequestSize < BD_UPNP_MAX_DEVICE_REQUEST_SIZE, "Request is too large for buffer" );

	/*
		"GET %(m_fetchLocation)s HTTP/1.1\r\n"
		"Connection: close\r\n"
		"Host: %(m_deviceAddr)s\r\n\r\n"
	*/
	m_requestSize = bdUInt( bdSnprintf( m_requestBuffer, 
										BD_UPNP_MAX_DEVICE_REQUEST_SIZE, 
										s_getRequest, 
										m_fetchLocation, 
										deviceAddrBuffer) );
}

bdBool bdUPnPDevice::parseDescriptionResponse()
{
	bdBool ok = true;

	bdByte8* URLBase = BD_NULL;
	bdUInt URLBaseLen = 0;
	bdBool isIP = false;
	bdByte8* controlURL = BD_NULL;
	bdUInt controlURLLen = 0;

	ok = confirmHttpSuccess();

	if( ok )
	{
		ok = ok && extractURLBase(&URLBase, URLBaseLen);
	}

	if( ok )
	{
		ok = ok && extractServiceType(isIP);
	}

	if( ok )
	{
		ok = ok && extractControlURL(&controlURL, controlURLLen);
	}

	if( ok )
	{
		m_isIp = isIP;

		// form the full m_controlURL from the prefix and given controlURL
		if (URLBaseLen > 0 && ( (URLBaseLen + controlURLLen) < sizeof( m_controlURL )) )
		{
			// Sometimes the URLBase ends with a "/" and the controlURL begins with one.
			// In this case we need to remove one of the slashes.
			if( URLBase[URLBaseLen-1] == '/' && controlURL[0] == '/' )
			{
				// Advance past the '/' and account for this in the URLLen
				controlURL++;
				controlURLLen--;
			}

			// Copy the base and then the control url
			bdStrlcpy(m_controlURL, URLBase, URLBaseLen + 1);
			bdStrlcpy(m_controlURL + URLBaseLen, controlURL, controlURLLen + 1);
		}
		else if( controlURLLen > 0 && controlURLLen < sizeof( m_controlURL ))
		{
			bdStrlcpy(m_controlURL, controlURL, controlURLLen + 1);
		}
		else
		{
			ok = false;
		}
	}

	return ok;
}

bdBool bdUPnPDevice::extractURLBase( bdByte8** baseLoc, bdUInt& baseLen )
{
	bdBool result = false;
	*baseLoc = BD_NULL;
	baseLen = 0;

	bdByte8* elementStart = BD_NULL;
	bdUInt elementLen = 0;

	bdByte8 urlBase[] = URL_BASE;
	if( extractTag( urlBase, m_readBuffer, &elementStart, elementLen ) )
	{
		bdByte8* addressStart = BD_NULL;
		bdByte8* addressEnd = BD_NULL;
		addressStart = bdStrstr(elementStart, HTTP);

		if (addressStart == elementStart)
		{
			bdByte8* elementEnd = elementStart + elementLen;

			// advance past the http://
			addressStart += bdStrlen(HTTP);
			bdByte8* slashLoc = bdStrstr(addressStart, "/");

			// if there is a string matching "/" before the endtag it must
			// be prepended to the controlURL
			if (slashLoc < elementEnd )
			{
				*baseLoc = slashLoc;
				baseLen = bdUInt(elementEnd - slashLoc);
				addressEnd = slashLoc;
			}
			else
			{
				addressEnd = elementEnd;
			}

			// pull out address:port from URLBase
			const bdUInt addressLength = bdUInt(addressEnd - addressStart);
			bdByte8 addrString[BD_ADDR_STRING_SIZE];

			if(addressLength < sizeof(addrString) )
			{
				bdStrlcpy(addrString, addressStart, addressLength + 1);
				m_deviceAddr.set(addrString);
				result = true;
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Description [URLBase tag] is improperly formatted. Missing http://");
		}
	}

	return result;
}

bdBool bdUPnPDevice::extractServiceType( bdBool& serviceType )
{
	bdBool result = true;

	bdByte8* serviceDesc = bdStrstr(m_readBuffer, IP_SERVICE);
	if (serviceDesc != BD_NULL)
	{
		serviceType = true; // the service type is IP
	}
	else
	{
		serviceDesc = bdStrstr(m_readBuffer, PPP_SERVICE);
		if( serviceDesc != BD_NULL )
		{
			serviceType = false; // the service type is PPP
		}
		else
		{
			// Neither service was found
			BD_WARN(BD_LOG_LEVEL, "No service type found.");
			result = false;
		}
	}

	return result;
}

bdBool bdUPnPDevice::extractControlURL( bdByte8** controlLoc, bdUInt& controlLen )
{
	bdBool result = false;
	*controlLoc = BD_NULL;
	controlLen = 0;

	bdByte8* buffStart = BD_NULL;
	bdByte8* elementStart = BD_NULL;
	bdUInt elementLen = 0;
	// Advance pointer to the right location. The correct control URL is located right
	// after the service type.
	if( m_isIp )
	{
		buffStart = bdStrstr(m_readBuffer, IP_SERVICE);
	}
	else
	{
		buffStart = bdStrstr(m_readBuffer, PPP_SERVICE);
	}

	if( buffStart != BD_NULL )
	{
		bdByte8 control_url[] = CONTROL_URL;
		if( extractTag( control_url, buffStart, &elementStart, elementLen) )
		{
			bdByte8* addressStart = bdStrstr(elementStart, HTTP);
			bdByte8* addressEnd = BD_NULL;
			bdByte8* elementEnd = elementStart + elementLen;

			// Check if address exists. if so then use it as the device addr.
			if (addressStart == elementStart)
			{
				// advance past the http://
				addressStart += bdStrlen(HTTP);
				addressEnd = bdStrstr(addressStart, "/");	
				
				// If there is no slash at the end, the end of the address is the end of the element
				if( addressEnd == BD_NULL || addressEnd > elementEnd )
				{
					addressEnd = elementEnd;
				}

				// pull out address:port from URLBase
				const bdUInt addressLength = bdUInt(addressEnd - addressStart);
				bdByte8 addrString[BD_ADDR_STRING_SIZE];
				if( addressLength < sizeof(addrString) )
				{
					bdStrlcpy(addrString, addressStart, addressLength + 1);
					m_deviceAddr.set(addrString);
				}
				*controlLoc = addressEnd;
				controlLen = bdUInt(elementEnd - addressEnd);
			}
			else
			{
				*controlLoc = elementStart;
				controlLen = elementLen;						
			}

			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "ControlURL specifier tag is missing");
		}
	}

	return result;
}



void bdUPnPDevice::prepareGetExternalIPRequest()
{
	genUPnPCommand( s_getExternalAddressCommand, s_getExternalAddressArguments );
}

bdBool bdUPnPDevice::parseGetExternalIPResponse()
{
	bdBool ok = true;
	bdInetAddr externalAddress;

	ok = confirmHttpSuccess();

	if( ok )
	{
		ok = ok && extractExternalAddress( externalAddress );
	}
	
	if( ok )
	{
		m_externalDeviceAddr = externalAddress;
	}

	return ok;
}

bdBool bdUPnPDevice::extractExternalAddress( bdInetAddr& externalAddress )
{
	bdBool result = false;
	externalAddress = bdInetAddr();
	
	bdByte8 *addrLoc = BD_NULL;
	bdUInt addrLen = 0;

	bdByte8 external_ip_address_string[] = EXTERNAL_IP_ADDRESS_STRING;
	if( extractTag( external_ip_address_string, m_readBuffer, &addrLoc, addrLen ))
	{
		bdByte8 addrString[BD_ADDR_STRING_SIZE];

		if( addrLen > 0 && addrLen < sizeof(addrString) )
		{
			bdStrlcpy(addrString, addrLoc, addrLen + 1);
			externalAddress.set(addrString);
			result = true;
			BD_INFO(BD_LOG_LEVEL, "External address on device determined to be: %s", addrString);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "NewExternalIPAddress specifier tag is missing");
	}

	return result;
}


void bdUPnPDevice::prepareGetMappingsRequest()
{
	const bdUInt maxArgLength = bdUInt(sizeof(s_getPortMappingArguments) + 5 /* Max length of port string*/);
	bdByte8 queryMapArguments[maxArgLength];

	/*
		"<NewRemoteHost></NewRemoteHost>\r\n"
		"<NewExternalPort>%(m_port)d</NewExternalPort>\r\n"
		"<NewProtocol>UDP</NewProtocol>\r\n";
	*/
	bdSnprintf(queryMapArguments, maxArgLength, s_getPortMappingArguments, m_gamePort);

	genUPnPCommand( s_getPortMappingCommand, queryMapArguments);

	BD_INFO(BD_LOG_LEVEL , "Querying UPnP device for existing mappings on port %u.", m_gamePort);
}

bdBool bdUPnPDevice::parseGetMappingsResponse(bdBool& mappingExists, bdBool& mappingIsMine )
{
	bdBool ok = true;
	mappingExists = false;
	mappingIsMine = false;

	// In this case a HttpSuccess header indicates that the message exists
	mappingExists  = confirmHttpSuccess();

	if( mappingExists )
	{
		bdInetAddr mappingOwner;

		ok = extractMappingOwner( mappingOwner );

		for (bdUInt i = 0; i < m_localAddrs->getSize() && !mappingIsMine; i++) 
		{
			bdInetAddr currAddress;
			m_localAddrs->get(i, currAddress);
			if (currAddress == mappingOwner)
			{
				mappingIsMine = true;
			}
		}

		if( !mappingIsMine )
		{
			bdNChar8 addressBuffer[BD_ADDR_STRING_SIZE];
			mappingOwner.toString(addressBuffer , BD_ADDR_STRING_SIZE);
			BD_INFO(BD_LOG_LEVEL, "Port Mapping collision found, owned by %s.", addressBuffer );
		}
	}

	return ok;
}
bdBool bdUPnPDevice::extractMappingOwner( bdInetAddr& owner )
{
	bdBool result = false;
	owner = bdInetAddr();

	bdByte8 *addrLoc = BD_NULL;
	bdUInt addrLen = 0;
	
	bdByte8 internal_client_string[] = INTERNAL_CLIENT_STRING;	
	if( extractTag( internal_client_string, m_readBuffer, &addrLoc, addrLen ) )
	{
		bdByte8 addrString[BD_ADDR_STRING_SIZE];

		// convert string to address to compare with known local addresses
		if( addrLen > 0 && addrLen < sizeof(addrString) )
		{
			bdStrlcpy(addrString, addrLoc, addrLen + 1); 
			bdInetAddr mappingOwner(addrString);
			if( mappingOwner.isValid() )
			{
				owner.set( mappingOwner );
				result = true;
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "NewInternalClient specifier tag is missing");
	}

	return result;
}
void bdUPnPDevice::prepareCreatePortMappingRequest()
{
	const bdUInt maxArgLength = bdUInt(	sizeof(s_addPortMappingArguments) 
										+ BD_ADDR_STRING_SIZE 
										+ 10 /* For the two port string*/);

	bdByte8 portMapArguments[maxArgLength];

	bdByte8 addrString[BD_ADDR_STRING_SIZE];
	m_localConnectedAddr.toString(addrString, BD_ADDR_STRING_SIZE);

	/*
		"<NewRemoteHost></NewRemoteHost>\r\n"
		"<NewExternalPort>%(m_gamePort)d</NewExternalPort>\r\n"
		"<NewProtocol>UDP</NewProtocol>\r\n"
		"<NewInternalPort>%(m_gamePort)d</NewInternalPort>\r\n"
		"<NewInternalClient>%(localConectedAddr)s</NewInternalClient>\r\n"
		"<NewEnabled>1</NewEnabled>\r\n"
		"<NewPortMappingDescription>"
		"DemonwarePortMapping"
		"</NewPortMappingDescription>\r\n"
		"<NewLeaseDuration>0</NewLeaseDuration>\r\n";
	*/
	bdSnprintf(portMapArguments, maxArgLength, s_addPortMappingArguments, 
		m_gamePort, m_gamePort, addrString);

	genUPnPCommand( s_addPortMappingCommand, portMapArguments);

}

void bdUPnPDevice::preparePortUnMappingRequest()
{
	const bdUInt maxArgLength = bdUInt(	sizeof(s_removePortMappingArguments) 
										+ 5 /* For the port string*/ );

	bdByte8 unMapArguments[maxArgLength];

	/*
		"<NewRemoteHost></NewRemoteHost>\r\n"
		"<NewExternalPort>%(m_gamePort)d</NewExternalPort>\r\n"
		"<NewProtocol>UDP</NewProtocol>\r\n";
	*/
	bdSnprintf(unMapArguments, maxArgLength, s_removePortMappingArguments, m_gamePort);

	genUPnPCommand(s_removePortMappingCommand, unMapArguments);
}

