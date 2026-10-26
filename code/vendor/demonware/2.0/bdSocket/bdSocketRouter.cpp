// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdSocketRouter.h>

#include <bdCore/bdSocket/bdCommonAddrInfo.h>
#include <bdCore/bdSocket/bdSecurityInfo.h>
#include <bdSocket/bdAddrHandle.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSInit.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSInitAck.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSCookieEcho.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSCookieAck.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSError.h>

#define BD_LOG_LEVEL "bdSocket/socket"

bdSocketRouter::bdSocketRouter()
: m_socket(BD_NULL),
  m_status(BD_SOCKET_ROUTER_UNINITIALIZED)
{
}

bdSocketRouter::~bdSocketRouter()
{
	m_keyStore.unregisterListener();

	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		delete m_dtls.getValue(it);
		m_dtls.next(it);
	}

}

bdBool bdSocketRouter::init(bdDefaultSocket *socket,
							const bdCommonAddrRef localCommonAddr,
							const bdSocketRouterConfig &config)
{
	bdBool result = true;

	if(BD_SOCKET_ROUTER_UNINITIALIZED != m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "init() called multiple times.");
	}
	else if(BD_NULL == socket)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid socket pointer is required by this class");
	}
	else if (localCommonAddr.isNull())
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid local common addr ref is required by this class");
	}
	else
	{
		m_socket = socket;
		m_localCommonAddr = localCommonAddr;

		// The ecc key only need to be initialised once.
		if(m_ECCKey.getStatus() == bdECCKey::BD_ECC_KEY_UNINITIALIZED)
		{
			result = m_ECCKey.init();
		}

		result = result && m_natTrav.init(socket, &m_qosBandwidth, m_localCommonAddr);
		result = result && m_qosProber.init(socket, &m_natTrav, &m_qosBandwidth);


		if(true == result)
		{
			m_status = BD_SOCKET_ROUTER_INITIALIZED;
			registerInterceptor(&m_natTrav);
			registerInterceptor(&m_qosProber);
			m_keyStore.registerListener(this);
			m_config = config;
			// Run a sanity check on the config info.
			m_config.sanityCheckConfig();

		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Socket Router initialization failed.");
			m_status = BD_SOCKET_ROUTER_ERROR;

		}
	}

	return result;
}


void bdSocketRouter::pump()
{
	m_natTrav.pump();
	m_qosProber.pump();

	bdQueue<bdEndpoint> toRemove;
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		const bdEndpoint &endpoint = m_dtls.getKey(it);
		bdDTLSAssociation *conn = m_dtls.getValue(it);
		conn->pump();

		switch(conn->getStatus())
		{
			case BD_SOCKET_IDLE:
			BD_WARN(BD_LOG_LEVEL, "Invalid state!");
			break;
			case BD_SOCKET_PENDING:
			case BD_SOCKET_CONNECTED:
			break;
			case BD_SOCKET_LOST:
			{
				bdNChar8 addrString[BD_ADDR_STRING_SIZE];
				bdAddrHandleRef addrHandle = conn->getAddrHandle();

				addrHandle->getRealAddr().toString(addrString, BD_ADDR_STRING_SIZE);

				bdNChar8 secIDString[BD_SECURITY_ID_STRING_LENGTH];
				bdSecurityInfo::toString(endpoint.getSecID(), secIDString, BD_SECURITY_ID_STRING_LENGTH);
				BD_INFO(BD_LOG_LEVEL, "DTLS closed. Removing address %s on security ID %s.", addrString, secIDString);

				addrHandle->setStatus(bdAddrHandle::BD_ADDR_UNRESOLVED);
				toRemove.enqueue(endpoint);
				m_addrMap.unregisterAddr(addrHandle);
			}
			break;
		}

		m_dtls.next(it);
	}
	m_dtls.releaseIterator(it);

	while(!toRemove.isEmpty())
	{
		const bdEndpoint &endpoint = toRemove.peek();
		it = m_dtls.getIterator(endpoint);

		BD_ASSERT(it != BD_NULL, "Couldn't find dlts connection in map.");

		bdDTLSAssociation *dtls = m_dtls.getValue(it);

		delete dtls;
		m_dtls.remove(it);

		toRemove.dequeue();
		m_dtls.releaseIterator(it);
	}
}

bdBool bdSocketRouter::quit()
{
	bdBool result = false;

	if(BD_SOCKET_ROUTER_UNINITIALIZED == m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "quit() called multiple times.");
	}
	else
	{
		// stop listening for key map events
		m_keyStore.unregisterListener();

		// Delete all active DTLS associations
		bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
		while(it)
		{
			delete m_dtls.getValue(it);
			m_dtls.next(it);
		}

		// empty the dtls list
		m_dtls.clear();

		// clear pointers
		m_socket = BD_NULL;

		// call quit on helper classes
		result = m_natTrav.quit();
		result = m_qosProber.quit() && result;

		// unregister interceptors
		unregisterInterceptor(&m_natTrav);
		unregisterInterceptor(&m_qosProber);

		// clear any remaining interceptors
		m_interceptors.clear();

		// clear the common addr -> addr handle lookup table
		m_endpointToAddrMap.clear();

		m_keyStore.clear();

		m_config.reset();

		m_status = BD_SOCKET_ROUTER_UNINITIALIZED;
	}

	return result;
}

bdBool bdSocketRouter::connect(bdAddrHandleRef &addrHandle)
{
	bdBool success = false;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		success = false;
		BD_WARN(BD_LOG_LEVEL, "Cannot connect before class has been initialized");
	}
	else if(addrHandle.isNull())
	{
		success = false;
		BD_WARN(BD_LOG_LEVEL, "Cannot connect using null addr handle ref");
	}
	else if (addrHandle->m_endpoint.getCommonAddr()->isLoopback())
	{
		// It is not possible to create a socket router connection to oneself...
		success = false;
		BD_WARN(BD_LOG_LEVEL, "Cannot make loopback associations with the socket router");
	}
	else
	{
		bdCommonAddrRef remote = addrHandle->m_endpoint.getCommonAddr();

		BD_INFO(BD_LOG_LEVEL, "	Connecting to address.");

		bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
		bdCommonAddrInfo::getInfo(remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
		BD_INFO(BD_LOG_LEVEL, commonAddrInfo);
		
		bdNChar8 secIDString[BD_SECURITY_ID_STRING_LENGTH];
		bdSecurityInfo::toString(addrHandle->m_endpoint.getSecID(), secIDString, BD_SECURITY_ID_STRING_LENGTH);
		BD_INFO(BD_LOG_LEVEL, "Using security ID: %s" , secIDString);

		// Check the security ID is registered.
		if(false == m_keyStore.contains(addrHandle->m_endpoint.getSecID()) )
		{
			BD_WARN(BD_LOG_LEVEL, "Security ID is not registered. ");
		}

		const bdAddrHandle::bdAddrHandleStatus status = addrHandle->getStatus();
		switch(status)
		{
			case bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED:
			case bdAddrHandle::BD_ADDR_NOT_RESOLVED:
			case bdAddrHandle::BD_ADDR_ERROR:
			case bdAddrHandle::BD_ADDR_UNRESOLVED:
			{
				// different debug output, based on addr handle status
				if(bdAddrHandle::BD_ADDR_ERROR == status)
				{
					BD_INFO(BD_LOG_LEVEL, "Address resolution failed. Trying again.");
				}
				else if (bdAddrHandle::BD_ADDR_UNRESOLVED  == status)
				{
					BD_INFO(BD_LOG_LEVEL, "Address has been unregistered. Starting again.");
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Address not resolved. Sending to NAT Trav.");
				}

				// First we will try to add an entry to the lookup map
				bdEndpoint endpoint(remote, addrHandle->m_endpoint.getSecID());
				bdBool added = m_endpointToAddrMap.put(endpoint, addrHandle);

				if(false == added)
				{
					// If we fail to add an entry then something is odd.
					BD_WARN(BD_LOG_LEVEL, "Remote addr already exists in map");

					BD_INFO(BD_LOG_LEVEL, "Cancelling existing NAT Trav if any....");
					m_natTrav.cancelConnect(remote);

					// In case this is a new adrHandle to remote mapping we will remove and re add.
					BD_INFO(BD_LOG_LEVEL, "Removing addr from map .... and re-adding");
					m_endpointToAddrMap.remove(endpoint);
					added = m_endpointToAddrMap.put(endpoint, addrHandle);
				}
				if(added)
				{
					success = m_natTrav.connect(remote, this);
				}
				else
				{
					// If we fail to add an entry again then something is VERY odd.
					BD_ERR(BD_LOG_LEVEL, "Repeated failure to add addr to map");
					success = false;
				}
			}
			break;
			case bdAddrHandle::BD_ADDR_RESOLVED:
			{
				BD_INFO(BD_LOG_LEVEL, "Address already resolved. Nothing to do.");
			}
			break;
		}
	}

	return success;
}

bdBool bdSocketRouter::disconnect(bdAddrHandleRef &addrHandle, bdBool allowDisconnectBroadcast)
{
	bdBool success = false;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot disconnect unless class is initialized");
	}
	else if(addrHandle.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot disconnect using null addr handle ref");
	}
	else
	{
		bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
		bdUInt numFound = 0;
		while (it)
		{
			bdDTLSAssociation *conn = m_dtls.getValue(it);
			if (*(conn->getAddrHandle()) == *addrHandle)
			{
				delete conn;
				m_dtls.remove(it);
				numFound++;
			}
			else
			{
				m_dtls.next(it);
			}
		}
		m_dtls.releaseIterator(it);
		if (numFound > 1)
		{
			BD_ERR(BD_LOG_LEVEL, "More than one DTLS connection found for an addrHandle");
		}
		success = m_addrMap.unregisterAddr(addrHandle);
	}

	return success;
}


bdDTLSAssociationStatus bdSocketRouter::getStatus(const bdAddrHandleRef addrHandle)
{
	bdDTLSAssociationStatus status = BD_SOCKET_IDLE;

	if(addrHandle.isNull())
	{
		status = BD_SOCKET_LOST;
		BD_WARN(BD_LOG_LEVEL, "Cannot get status of null addr handle ref");
	}
	else
	{
		bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
		bdAddressMap::addrToString(addrHandle, addrStr, sizeof(addrStr));

		switch(addrHandle->getStatus())
		{
			case bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED:
			{
				status = BD_SOCKET_IDLE;
			}
			break;
			case bdAddrHandle::BD_ADDR_NOT_RESOLVED:
			{
				if( m_natTrav.isConnectInProgress( addrHandle->m_endpoint.getCommonAddr() ) )
				{
					status = BD_SOCKET_PENDING;
				}
				else
				{
					status = BD_SOCKET_LOST;
				}
			}
			break;
			case bdAddrHandle::BD_ADDR_ERROR:
			{
				BD_INFO(BD_LOG_LEVEL, "Address handle error. Socket lost. %s", addrStr);
				status = BD_SOCKET_LOST;
			}
			break;
			case bdAddrHandle::BD_ADDR_RESOLVED:
			{
				bdEndpoint endpoint(addrHandle->m_endpoint.getCommonAddr(), addrHandle->m_endpoint.getSecID());

				bdDTLSAssociation *dtlsSocket = BD_NULL;
				const bdBool found = m_dtls.get(endpoint, dtlsSocket);

				if(found && dtlsSocket)
				{
					status = dtlsSocket->getStatus();
				}
				else if(false == found)
				{
					BD_ERR(BD_LOG_LEVEL, "DTLS not found for %s", addrStr);
					status = BD_SOCKET_LOST;
				}
				else
				{
					BD_ERR(BD_LOG_LEVEL, "Invalid state! %s", addrStr);
					status = BD_SOCKET_LOST;
				}
			}
			break;
			case bdAddrHandle::BD_ADDR_UNRESOLVED:
			{
				BD_INFO(BD_LOG_LEVEL, "Address handle unresolved. Socket lost. %s", addrStr);
				status = BD_SOCKET_LOST;
			}
			break;
		}
	}

	return status;
}

bdSocketRouter::bdSocketRouterStatus bdSocketRouter::getStatus()
{
	return m_status;
}

bdInt bdSocketRouter::sendTo(const bdAddrHandleRef addrHandle,
							 void *data,
							 const bdUInt length)
{
	bdInt val = BD_NET_ERROR;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot send data before class has been initialized");
	}
	else if(addrHandle.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot send data to null addr handle ref");
	}
	else
	{
		const bdAddr &realAddr = addrHandle->getRealAddr();

		bdDTLSAssociation *dtlsSocket = BD_NULL;
		bdEndpoint endpoint(addrHandle->m_endpoint.getCommonAddr(), addrHandle->m_endpoint.getSecID());

		m_dtls.get(endpoint, dtlsSocket);

		if(dtlsSocket)
		{
			val = dtlsSocket->sendTo(realAddr, data, length);
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Not established! Call connect() first.");
			bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
			bdCommonAddrInfo::getBriefInfo(addrHandle->m_endpoint.getCommonAddr(), commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
			BD_INFO(BD_LOG_LEVEL, commonAddrInfo);
		}
	}

	return val;
}

bdInt bdSocketRouter::receiveFrom(bdAddrHandleRef &addrHandle,
								  void *data,
								  const bdUInt size,
								  bool& didErrorOccur)
{
	didErrorOccur = false;
	bdInt val = BD_NET_WOULD_BLOCK;

	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot receive data before class has been initialized");
	}
	else
	{
		bdBool receiving = true;
		bdAddr realAddr;

		bdUInt resetCount = 0;
		while(receiving)
		{
			val = m_socket->receiveFrom(realAddr, data, size);

			if(val > 0)
			{
				val = processPacket( addrHandle, realAddr, data, size, val, receiving );
			}
			else if (val < 0)
			{
				processError( realAddr, val, resetCount, receiving );
			}
			else
			{
				// This is not something that is expected to occur.
				BD_ERR(BD_LOG_LEVEL, "socket receiveFrom returned 0 ");
				receiving = false;
			}
		}
	}
	return val;
}

bdInt bdSocketRouter::processPacket(bdAddrHandleRef &addrHandle,
									bdAddr &realAddr,
									void *data,
									const bdUInt size,
									const bdUInt bytesTransferred,
									bdBool &receiving )
{
	bdInt val = bytesTransferred;
	const bdUByte8 type = *(reinterpret_cast<const bdUByte8 *>(data));
	bdBool createNewDTLS = false;
	switch(type)
	{
		case BD_DTLS_INIT:
		{
			bdDTLSInit init;
			bdUInt offset = 0;
			bdBool ok = init.deserialize(data, val, offset, offset);
			if(ok)
			{
				bdSecurityID initSecID;
				init.getSecID(initSecID);
				//If an existing dtls association exists, don't create a new one
				if(!(tryToFindConnection(data, size, initSecID, realAddr, addrHandle, val)))
				{
					bdDTLSAssociation serverDTLS(m_socket,
										&m_keyStore,
										&m_ECCKey,
										realAddr,
										addrHandle,
										m_localCommonAddr,
										&m_addrMap,
										m_config.m_DTLSAssociationReceiveTimeout);

					val = serverDTLS.receiveFrom(realAddr, data, static_cast<bdUInt>(val), addrHandle, data, size);
					createNewDTLS = (serverDTLS.getStatus() == BD_SOCKET_CONNECTED);
				}
			}
		}
		break;
		case BD_DTLS_COOKIE_ECHO:
		{
			bdDTLSCookieEcho cookieEcho;
			bdUInt offset = 0;
			bdBool ok = cookieEcho.deserialize(data, val, offset, offset);
			if(ok)
			{
				bdSecurityID cookieEchoSecID;
				bdMemcpy(cookieEchoSecID.ab, cookieEcho.getSecID(), BD_SECURITY_ID_LENGTH);
				//If an existing dtls association exists, don't create a new one
				if(!(tryToFindConnection(data, size, cookieEchoSecID, realAddr, addrHandle, val)))
				{
					bdDTLSAssociation serverDTLS(m_socket,
										&m_keyStore,
										&m_ECCKey,
										realAddr,
										addrHandle,
										m_localCommonAddr,
										&m_addrMap,
										m_config.m_DTLSAssociationReceiveTimeout);

					val = serverDTLS.receiveFrom(realAddr, data, static_cast<bdUInt>(val), addrHandle, data, size);
					createNewDTLS = (serverDTLS.getStatus() == BD_SOCKET_CONNECTED);
				}
			}
		}
		break;
		case BD_DTLS_ERROR:
		{
			bdDTLSError error;
			bdUInt offset = 0;
			bdBool ok = error.deserialize(data, val, offset, offset);

			if(ok)
			{
				bdSecurityID errorSecID;
				error.getSecID(errorSecID);
				//If an existing dtls association exists, don't create a new one
				if(!(tryToFindConnection(data, size, errorSecID, realAddr, addrHandle, val)))
				{
					bdDTLSAssociation serverDTLS(m_socket,
										&m_keyStore,
										&m_ECCKey,
										realAddr,
										addrHandle,
										m_localCommonAddr,
										&m_addrMap,
										m_config.m_DTLSAssociationReceiveTimeout);

					val = serverDTLS.receiveFrom(realAddr, data, static_cast<bdUInt>(val), addrHandle, data, size);
					createNewDTLS = (serverDTLS.getStatus() == BD_SOCKET_CONNECTED);
				}
			}
		}
		break;

		case BD_DTLS_INIT_ACK:
		{
			bdDTLSInitAck initAck;
			bdUInt offset = 0;
			bdBool ok = initAck.deserialize(data, val, offset, offset);
			if(ok)
			{
				bdSecurityID initAckSecID;
				initAck.getSecID(initAckSecID);

				/// We don't care if this returns false.
				tryDecryptPacket(data, size, initAckSecID, realAddr, addrHandle, val);
			}

		}
		break;

		case BD_DTLS_COOKIE_ACK:
		{
			bdDTLSCookieAck cookieAck;
			bdUInt offset = 0;
			bdBool ok = cookieAck.deserialize(data, val, offset, offset);
			if(ok)
			{
				bdSecurityID ackSecID;
				cookieAck.getSecID(ackSecID);

				/// We don't care if this returns false.
				tryDecryptPacket(data, size, ackSecID, realAddr, addrHandle, val);
			}

		}
		break;

		case BD_DTLS_DATA:
		{
			bdBool found = false;
			bdSecurityID fake_sec_id;
			found = tryDecryptPacket(data,size, fake_sec_id, realAddr,addrHandle, val, false);
			receiving = !found;
		}
		break;
		default:
		{
			// pass packet on to interceptors until it is accepted.
			bdBool accepted = false;
			for(bdUInt i = 0; !accepted && (i < m_interceptors.getSize()); i++)
			{
				accepted = m_interceptors[i]->acceptPacket(m_socket, realAddr, data, static_cast<bdUInt>(val) , type);
			} 
		}
		break;
	}

	if(createNewDTLS)
	{
		bdDTLSAssociation* dtlsSocket = new bdDTLSAssociation(m_socket,
											&m_keyStore,
											&m_ECCKey,
											realAddr,
											addrHandle,
											m_localCommonAddr,
											&m_addrMap,
											m_config.m_DTLSAssociationReceiveTimeout);

		val = dtlsSocket->receiveFrom(realAddr, data, static_cast<bdUInt>(val), addrHandle, data, size);

		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		realAddr.toString(addrString, BD_ADDR_STRING_SIZE);

		BD_INFO(BD_LOG_LEVEL, "New incoming DTLS connection from %s.", addrString);
		bdEndpoint endpoint(dtlsSocket->getAddrHandle()->m_endpoint.getCommonAddr(),
							dtlsSocket->getAddrHandle()->m_endpoint.getSecID());

		const bdBool dtlsAdded = m_dtls.put(endpoint, dtlsSocket);
		if(false == dtlsAdded)
		{
			bdNChar8 addrString [BD_COMMON_ADDR_INFO_SIZE];
			bdCommonAddrInfo::getInfo(endpoint.getCommonAddr(), addrString, BD_COMMON_ADDR_INFO_SIZE);
			bdNChar8 secIDString [BD_SECURITY_ID_STRING_LENGTH];
			bdSecurityInfo::toString(endpoint.getSecID(), secIDString, BD_SECURITY_ID_STRING_LENGTH);
			BD_ERR(BD_LOG_LEVEL, "New DTLS with secID %s NOT added to the list." , secIDString);
			BD_ERR(BD_LOG_LEVEL, "addrString %s" , addrString);
			// Ensure we don't leak memory here.
			delete dtlsSocket;
		}

		if(val >= 0)
		{
			// done receiving. we have something to pass it to the next level.
			receiving = false;
		}
	}
	return val;
}

void bdSocketRouter::processError( bdAddr &realAddr, bdInt val, bdUInt &resetCount, bdBool &receiving)
{
	// Once an error in encountered we will stop receiving..
	receiving = false;

	const bdSocketStatusCode error = static_cast<bdSocketStatusCode>(val);
	switch(error)
	{
		case BD_NET_CONNECTION_RESET:
		{
			// Connection reset is potentially harmful as it can be
			// spoofed and used to reset connections.
			// If a genuine connection reset occurs, DTLS layer
			// will correctly time out. by default in debug mode we
			// pay attention to it because time outs are very high.
			if(false == m_config.m_ignoreConnectionReset)
			{
				m_addrMap.unregisterRealAddr(realAddr);
			}
			else
			{
				const bdUWord addrBufferSize = 100;
				bdNChar8 addrBuffer[addrBufferSize];
				realAddr.toString(addrBuffer, addrBufferSize);
				BD_INFO(BD_LOG_LEVEL, "Connection reset packet received for %s. Ignoring." , addrBuffer);

			}
			// As this is about a specific connection, and does not need to be passed
			// to the higher levels we can continue receiving if this error is
			// encountered, however it will not loop forever.
			if (resetCount++ < m_config.m_maxConnectionResets)
			{
				receiving = true;
			}
			else
			{
				const bdUWord addrBufferSize = 100;
				bdNChar8 addrBuffer[addrBufferSize];
				realAddr.toString(addrBuffer, addrBufferSize);
				BD_WARN(BD_LOG_LEVEL, "Exiting receiveAll loop after receiving %u connection resets." , resetCount);
				BD_WARN(BD_LOG_LEVEL, "Last reset was received for addr %s " , addrBuffer);
			}
		}
		break;
		case BD_NET_WOULD_BLOCK:
		{
			// nothing left at this level.
		}
		break;
		case BD_NET_MSG_SIZE:
		{
			// packet cannot fit in the buffer
			BD_WARN(BD_LOG_LEVEL, "Couldn't receive message. Buffer too small?");
		}
		break;
		case BD_NET_ERROR:						/// unknown error.
		case BD_NET_SUBSYTEM_ERROR:				/// some part of the network subsystem has failed.
		case BD_NET_ADDRESS_IN_USE:				/// for binding, the address is already in use.
		case BD_NET_ADDRESS_INVALID:			/// the address given is invalid.
		case BD_NET_BLOCKING_CALL_CANCELED:	/// A blocking call was canceled.
		case BD_NET_NOT_BOUND:					/// The socket has yet to bind; therefore receiving won't work.
		case BD_NET_INVALID_HANDLE:				/// The socket handle is invalid.
		case BD_NET_NOT_CONNECTED:				/// The socket is not connected.
		{
			// TODO
			// Consider the appropriate action, if any, to take in each of the above cases
			BD_WARN(BD_LOG_LEVEL , "Encountered error: %i while receiving from the socket router " , error);
		}
		break;
		case BD_NET_SUCCESS: 					/// All good!!
		case BD_NET_CONNECTION_CLOSED:			/// Connection was closed gracefully
		{
			BD_ERR(BD_LOG_LEVEL , "These cannot occur");
		}
		break;
	}
}

void bdSocketRouter::onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr &realAddr)
{
	bdAddrHandleRef addrHandle;

	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot react to NAT addr discovery event before class has been initialized");
	}
	else
	{
		bdHashMap<bdEndpoint, bdAddrHandleRef>::Iterator iterator = m_endpointToAddrMap.getIterator();
		bdBool found = false;
		while(iterator)
		{
			bdEndpoint key = m_endpointToAddrMap.getKey(iterator);
			addrHandle = m_endpointToAddrMap.getValue(iterator);

			if(*(key).getCommonAddr() == *remote)
			{
				found = true;
				m_endpointToAddrMap.remove(iterator);
				addrHandle->setRealAddr(realAddr);
				bdEndpoint endpoint(addrHandle->m_endpoint.getCommonAddr(), addrHandle->m_endpoint.getSecID());

				bdDTLSAssociation *conn;
				if(!m_dtls.get(endpoint, conn))
				{
					bdNChar8 addrString[BD_ADDR_STRING_SIZE];
					realAddr.toString(addrString, BD_ADDR_STRING_SIZE);

					BD_INFO(BD_LOG_LEVEL, "Address lookup succeeded. Creating DTLS conn to %s.", addrString);

					conn = new bdDTLSAssociation(	m_socket,
													&m_keyStore,
													&m_ECCKey,
													realAddr,
													addrHandle,
													m_localCommonAddr,
													&m_addrMap,
													m_config.m_DTLSAssociationReceiveTimeout);
					conn->connect();

					const bdBool putResult = m_dtls.put(key, conn);
					if(false == putResult)
					{
						bdNChar8 addrString [BD_COMMON_ADDR_INFO_SIZE];
						bdCommonAddrInfo::getInfo(endpoint.getCommonAddr(), addrString, BD_COMMON_ADDR_INFO_SIZE);
						bdNChar8 secIDString [BD_SECURITY_ID_STRING_LENGTH];
						bdSecurityInfo::toString(endpoint.getSecID(), secIDString, BD_SECURITY_ID_STRING_LENGTH);

						BD_ERR(BD_LOG_LEVEL, "New DTLS with secID %s NOT added to the list.", secIDString);
						BD_ERR(BD_LOG_LEVEL, "Address: %s" , addrString);
						// Ensure we don't leak memory here.
						delete conn;
					}
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "already connected.");
				}
			}
			else
			{
				m_endpointToAddrMap.next(iterator);
			}
		}

		if(!found)
		{
			BD_ERR(BD_LOG_LEVEL, "address lookup failed.");
			bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
			bdCommonAddrInfo::getBriefInfo(remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
			BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

		}
	}
}

void bdSocketRouter::onNATAddrDiscoveryFailed(bdCommonAddrRef remote)
{
	BD_INFO(BD_LOG_LEVEL, "Address lookup failed from");

	bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
	BD_INFO(BD_LOG_LEVEL, "NAT Discovery Failed - Local addr:");
	bdCommonAddrInfo::getInfo(m_localCommonAddr, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
	BD_INFO(BD_LOG_LEVEL, commonAddrInfo);
	BD_INFO(BD_LOG_LEVEL, "NAT Discovery Failed - Remote addr:");
	bdCommonAddrInfo::getInfo(remote, commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
	BD_INFO(BD_LOG_LEVEL, commonAddrInfo);

	bdAddrHandleRef addrHandle;

	bdHashMap<bdEndpoint, bdAddrHandleRef>::Iterator iterator = m_endpointToAddrMap.getIterator();
	bdBool found = false;

	//FIXME CHECKME
	while(iterator)
	{
		bdEndpoint key = m_endpointToAddrMap.getKey(iterator);
		addrHandle = m_endpointToAddrMap.getValue(iterator);

		if(*key.getCommonAddr() == *remote)
		{
			found = true;
			m_endpointToAddrMap.remove(iterator);
			if (addrHandle->getStatus() == bdAddrHandle::BD_ADDR_NOT_RESOLVED)
			{
				addrHandle->setStatus(bdAddrHandle::BD_ADDR_ERROR);
			}
		}
		else
		{
			m_endpointToAddrMap.next(iterator);
		}
	}

	if (!found)
	{
		BD_ERR(BD_LOG_LEVEL, "address lookup failed.");
	}
}

void bdSocketRouter::registerInterceptor(bdPacketInterceptor *const interceptor)
{
	if(BD_SOCKET_ROUTER_INITIALIZED == m_status)
	{
#ifdef BD_DEBUG
		bdUInt32 index;
		if(m_interceptors.findFirst(interceptor, index))
		{
			BD_WARN(BD_LOG_LEVEL, "same listener registered multiple times.");
		}
	#endif // BD_DEBUG

		m_interceptors.pushBack(interceptor);
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Cannot register interceptors until this class is initialized");
	}
}

void bdSocketRouter::unregisterInterceptor(bdPacketInterceptor *const interceptor)
{
	m_interceptors.removeAllKeepOrder(interceptor);
}

bdQoSProbe* bdSocketRouter::getQoSProber()
{
	return &m_qosProber;
}

void bdSocketRouter::onSecurityKeyRemove(const bdSecurityID &id)
{
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while (it)
	{
		bdDTLSAssociation *conn = m_dtls.getValue(it);
		if (*(conn->getLocalSecurityId()) == id)
		{
			bdAddrHandleRef addrHandle = conn->getAddrHandle();
			addrHandle->setStatus(bdAddrHandle::BD_ADDR_UNRESOLVED);

			delete conn;
			m_dtls.remove(it);

			m_addrMap.unregisterAddr(addrHandle);

		}
		else
		{
			m_dtls.next(it);
		}
	}
	m_dtls.releaseIterator(it);
}

bdBool bdSocketRouter::setupIntroducers(const bdArray<bdAddr> &introducers)
{
	bdBool result = false;
	if(BD_SOCKET_ROUTER_INITIALIZED == m_status)
	{
		result = m_natTrav.setupIntroducers(introducers);
	}
	return result;
}

bdBool bdSocketRouter::updateLocalCommonAddr(const bdCommonAddrRef newLocalCommonAddr)
{
	bdBool result = false;
	if(BD_SOCKET_ROUTER_INITIALIZED == m_status)
	{
		if(m_localCommonAddr.isNull())
		{
			BD_ERR(BD_LOG_LEVEL, "Null common addr cannot be used to setup socket router");
		}
		else
		{
			m_localCommonAddr = newLocalCommonAddr;
			result = m_natTrav.updateLocalCommonAddr(newLocalCommonAddr);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Updating local common addr has no effect unless class is initialized");
	}
	return result;
}


bdAddressMap* bdSocketRouter::getAddressMap()
{
	return &m_addrMap;
}

bdSecurityKeyMap* bdSocketRouter::getKeyMap()
{
	return &m_keyStore;
}

void bdSocketRouter::getConfig(bdSocketRouterConfig &config) const
{
	config = m_config;
}

bdCommonAddrRef bdSocketRouter::getLocalCommonAddr() const
{
	return m_localCommonAddr;
}


bdBool bdSocketRouter::tryDecryptPacket(void *data,
							  const bdUInt& size,
							  const bdSecurityID& secID,
							  bdAddr& realAddr,
							  bdAddrHandleRef& addrHandle,
							  bdInt& val,
							  bdBool checkSecID)
{
	//iterator for dtls associations
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator iterator = m_dtls.getIterator();
	bdBool found = false;
	//Find a dtls association with a matching security ID and address
	while(iterator && !found)
	{
		//return an endpoint
		bdEndpoint key = m_dtls.getKey(iterator);
		//Return the association
		bdDTLSAssociation *dtls = m_dtls.getValue(iterator);

		bdAddrHandleRef dtlsAddrHandle;
		bdAddr dtlsRealAddr;
		//If we have a valid association, get its Address
		if(dtls)
		{
			dtlsAddrHandle = dtls->getAddrHandle();
		}
		if(dtlsAddrHandle.notNull())
		{
			dtlsRealAddr = dtlsAddrHandle->getRealAddr();
		}
		//If we have a valid address, check to see if it matches our
		//security ID and Address we want to receive from
		if(dtlsAddrHandle.notNull() &&
		   dtlsRealAddr == realAddr &&
		   (!checkSecID || key.getSecID() == secID))
		{
			//Receive the data on known address and security ID
			bdInt returnVal = dtls->receiveFrom(realAddr, data, static_cast<bdUInt>(val), addrHandle, data, size);

			//We got some valid data, move on
			if(returnVal >= 0)
			{
				// done receiving. we have something to pass it to the next level.
				found = true;
				val = returnVal;
				m_dtls.releaseIterator(iterator);
			}
		}
		//Data not found, continue iterating
		if(!found)
		{
			m_dtls.next(iterator);
		}
	}
	return found;
}

bdBool bdSocketRouter::tryToFindConnection(void *data,
							  const bdUInt& size,
							  const bdSecurityID& secID,
							  bdAddr& realAddr,
							  bdAddrHandleRef& addrHandle,
							  bdInt& val,
							  bdBool checkSecID)
{
	//iterator for dtls associations
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator iterator = m_dtls.getIterator();
	bdBool found = false;
	//Find a dtls association with a matching security ID and address
	while(iterator && !found)
	{
		//return an endpoint
		bdEndpoint key = m_dtls.getKey(iterator);
		//Return the association
		bdDTLSAssociation *dtls = m_dtls.getValue(iterator);

		bdAddrHandleRef dtlsAddrHandle;
		bdAddr dtlsRealAddr;
		//If we have a valid association, get its Address
		if(dtls)
		{
			dtlsAddrHandle = dtls->getAddrHandle();
		}
		if(dtlsAddrHandle.notNull())
		{
			dtlsRealAddr = dtlsAddrHandle->getRealAddr();
		}
		//If we have a valid address, check to see if it matches our
		//security ID and Address we want to receive from
		if(dtlsAddrHandle.notNull() &&
		   dtlsRealAddr == realAddr &&
		   (!checkSecID || key.getSecID() == secID))
		{
			//Receive the data on known address and security ID
				bdInt returnVal = dtls->receiveFrom(realAddr, data, static_cast<bdUInt>(val), addrHandle, data, size);

				//We got some valid data, move on
				if(returnVal >= 0)
				{
					// done receiving. we have something to pass it to the next level.
					found = true;
					val = returnVal;
					m_dtls.releaseIterator(iterator);
				}

				if(returnVal == BD_NET_WOULD_BLOCK)
				{
					found = true;
					val = returnVal;
					m_dtls.releaseIterator(iterator);
				}
		}
		//Data not found, continue iterating
		if(!found)
		{
			m_dtls.next(iterator);
		}
	}
	return found;
}

bdBool bdSocketRouter::isRelayEnabled() const
{
	return false;
}