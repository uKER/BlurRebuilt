// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdConnectionStore.h>

#include <bdConnection/bdConnection.h>
#include <bdSocket/bdAddressMap.h>
#include <bdConnection/bdPacket/bdDefaultPacket.h>
#include <bdConnection/bdPacket/bdCookieEchoChunk.h>
#include <bdConnection/bdPacket/bdInitChunk.h>
#include <bdConnection/bdPacket/bdInitAckChunk.h>
#include <bdConnection/bdLoopbackConnection.h>
#include <bdConnection/bdUnicastConnection.h>
#include <bdSocket/bdSocketRouter.h>

#define BD_LOG_LEVEL	"bdConnection/connectionstore"


bdConnectionStore::bdConnectionStore()
: m_socket(BD_NULL),
  m_status(BD_CONNECTION_STORE_UNINITIALIZED),
  m_addrMap(BD_NULL)
{
}

bdConnectionStore::~bdConnectionStore()
{
	m_connectionMap.clear();
	m_dispatcher.reset();
}

bdBool bdConnectionStore::init(bdSocketRouter *socket,
							   const bdConnectionStoreConfig &config)
{
	bdBool result = true;

	if(BD_CONNECTION_STORE_UNINITIALIZED != m_status)
	{
		result = false;
		BD_WARN(BD_LOG_LEVEL, "init() called multiple times.");
	}
	else if(BD_NULL == socket)
	{
		result = false;
		BD_ERR(BD_LOG_LEVEL, "A valid socket router pointer is required by this class");
	}
	else
	{
		m_socket = socket;
		m_addrMap = socket->getAddressMap();
		m_config = config;
		m_status = BD_CONNECTION_STORE_INITIALIZED;
	}

	return result;

}

bdBool bdConnectionStore::flushAll()
{
	bdBool result = false;
	if( (BD_CONNECTION_STORE_INITIALIZED == m_status) ||
		(BD_CONNECTION_STORE_SHUTTING_DOWN == m_status))
	{
		result = true;
		bdQueue<bdConnectionRef> toDisconnect;
		// Pump socket.
		m_socket->pump();

		// Flush outgoing connections
		bdHashMap<bdAddrHandleRefWrapper, bdConnectionRef, bdAddrHandleRefWrapper>::Iterator it = m_connectionMap.getIterator();

		while(it)
		{
			bdConnectionRef connection = m_connectionMap.getValue(it);

			if(!connection->getAddress()->isLoopback())
			{
				const bdDTLSAssociationStatus &status = m_socket->getStatus(connection->getAddressHandle());
				bdUInt flushIters = 0;
				switch(status)
				{
					case BD_SOCKET_PENDING:
					{
						// do nothing
					}
					break;
					case BD_SOCKET_CONNECTED:
					{
						while((flushIters < m_config.m_maxFlushIterations) && (flush(*connection) > 0))
						{
							flushIters++;
						}

						if(connection->getStatus() == bdConnection::BD_DISCONNECTED)
						{
							const bdAddrHandleRef addrHandle = connection->getAddressHandle();
							bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
							bdAddressMap::addrToString(addrHandle, addrStr, sizeof(addrStr));

							BD_INFO(BD_LOG_LEVEL, "Connection state disconnected. Disconnecting %s.", addrStr);
							toDisconnect.enqueue(connection);
						}
					}
					break;
					case BD_SOCKET_LOST:
					{
						const bdAddrHandleRef addrHandle = connection->getAddressHandle();
						bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
						bdAddressMap::addrToString(addrHandle, addrStr, sizeof(addrStr));

						BD_INFO(BD_LOG_LEVEL, "Socket router reports socket lost. Disconnecting %s.", addrStr);
						toDisconnect.enqueue(connection);
					}
					break;
					case BD_SOCKET_IDLE:
					{
						const bdAddrHandleRef addrHandle = connection->getAddressHandle();
						bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
						bdAddressMap::addrToString(addrHandle, addrStr, sizeof(addrStr));

						BD_WARN(BD_LOG_LEVEL, "Socket router reports socket idle. Disconnecting %s.", addrStr);
						toDisconnect.enqueue(connection);
					}
					break;
				}
			}
			else
			{
				(reference_cast<bdLoopbackConnection>(connection))->updateStatus();

				if(connection->getStatus() == bdConnection::BD_DISCONNECTED)
				{
					toDisconnect.enqueue(connection);
				}
			}

			m_connectionMap.next(it);
		}


		// remove disconnected connections
		while(!toDisconnect.isEmpty())
		{
			bdConnectionRef connection = toDisconnect.peek();
			bdAddrHandleRef addr = connection->getAddressHandle();
			connection->close();

			// Remove from the connection map.
			const bdBool mapRemoveResult = m_connectionMap.remove(bdAddrHandleRefWrapper(addr));
			if(false == mapRemoveResult)
			{
				bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
				bdAddressMap::addrToString(addr, addrStr, sizeof(addrStr));
				BD_WARN(BD_LOG_LEVEL, "Failed to remove addr %s from connection map." , addrStr);
				result = false;
			}

			// Close the secure association.
			const bdBool socketDisconnectResult = m_socket->disconnect(addr);
			if(false == socketDisconnectResult)
			{
				bdNChar8 addrStr[BD_ADDR_STRING_SIZE];
				bdAddressMap::addrToString(addr, addrStr, sizeof(addrStr));
				BD_WARN(BD_LOG_LEVEL, "Failed to disconnect secure association for addr %s ." , addrStr);
				result = false;
			}

			toDisconnect.dequeue();
		}

		// Monitor the shutdown progress.
		if(BD_CONNECTION_STORE_SHUTTING_DOWN == m_status)
		{
			// If all connections are closed or we have waited long enough
			if( (m_connectionMap.getSize() == 0) ||
				(m_shutdownTimer.getElapsedTimeInSeconds() > m_config.m_maxShutdownDuration))
			{
				// Use immediate shutdown to preform remainder of required tidying
				result = startShutdown(BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);
			}
		}

	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot call flushAll unless class is initialized.");
	}
	return result;
}

bdBool bdConnectionStore::receiveAll()
{
	bdBool result = true;
	if( (BD_CONNECTION_STORE_INITIALIZED == m_status) ||
		(BD_CONNECTION_STORE_SHUTTING_DOWN == m_status))
	{
		// Return once we have iterated "m_config.m_maxReceiveIterations" times
		// or if there is nothing left to receive
		bdUByte8 data[BD_MAX_DATAGRAM_SIZE];
		bdBool receivedData = false;

		bdUInt iterationCount = 0;
		do
		{
			bdAddrHandleRef address;
			bool didErrorOccur;
			const bdInt received = m_socket->receiveFrom(address, data, sizeof(data), didErrorOccur);

			//We will iterate do..while loop until no data is received.
			receivedData = (received > 0);

			// check the error conditions
			if(received < 0)
			{
				// loop should terminate
				receivedData = false;

				const bdSocketStatusCode error = static_cast<bdSocketStatusCode>(received);
				switch(error)
				{
					case BD_NET_WOULD_BLOCK:
					{
						// No data on the socket router, nothing else to do..
						result = true;
					}
					break;
					case BD_NET_CONNECTION_RESET:
					{
						/// connection reset. Usually occurs when the remote socket.
						/// is not in the listening state
						/// This error will be handled as necessary at the socket router
						/// level so it can safely be ignored here!
						result = true;

						/// As this indicates a problem with 1 specific
						/// connection it is ok & efficient to continue trying
						/// to receive data.
						receivedData = true;
					}
					break;
					case BD_NET_ERROR:						/// unknown error.
					case BD_NET_SUBSYTEM_ERROR:				/// some part of the network subsystem has failed.
					case BD_NET_ADDRESS_IN_USE:				/// for binding, the address is already in use.
					case BD_NET_MSG_SIZE:					/// the buffer passed was too small for the data.
					case BD_NET_ADDRESS_INVALID:			/// the address given is invalid.
					case BD_NET_BLOCKING_CALL_CANCELED:	/// A blocking call was canceled.
					case BD_NET_NOT_BOUND:					/// The socket has yet to bind; therefore receiving won't work.
					case BD_NET_INVALID_HANDLE:				/// The socket handle is invalid.
					case BD_NET_NOT_CONNECTED:				/// The socket is not connected.
					{
						// TODO
						// Consider the appropriate action, if any, to take in each of the above cases
						BD_WARN(BD_LOG_LEVEL , "Encountered error: %i while receiving from the socket router " , error);
						result = false;
					}
					break;
					case BD_NET_SUCCESS: 					/// All good!!
					case BD_NET_CONNECTION_CLOSED:			/// Connection was closed gracefully
					{
						BD_ERR(BD_LOG_LEVEL , "These cannot occur");
						result = false;
					}
					break;
				}
			}
			else
			{
				// we have received something (may be zero sized)

				// Safe to cast signed int to unsigned as received is >= 0
				const bdUInt bytesReceived = static_cast<bdUInt>(received);

				// lookup up the connection using address
				bdConnectionRef receivedConnection;

				// TODO
				// Consider what the exact conditions here are for the data
				// to be considered as a new connection.
				// Currently unless the data is successfully handled the data
				// will be passed on the the new connection code.
				if(true == address.notNull())
				{
					if(	(true == m_connectionMap.get(address, receivedConnection)) &&
						(true == receivedConnection.notNull()) &&
						(true == receivedConnection->receive(data, bytesReceived)))
					{
						// data has been handled by receivedConnection->receive()

						// If the received connection does not have a valid real addr set it here
						if( receivedConnection->getAddressHandle()->getRealAddr().getAddress().isValid() == false )
						{
							receivedConnection->setAddressHandle(address);
						}
					}
					else if(BD_CONNECTION_STORE_SHUTTING_DOWN == m_status)
					{
						// During shutdown we will ignore any new connections.
					}
					else
					{
						// TODO
						// This entire section can be massively refactored.. EoF
						// The idea would be to store connections in the map while
						// they are being initialized... as the traffic coming
						// from a trusted source, the socket router...
						//
						// if(notExistingConn)
						//		add new conn to map
						// process packet

						// TODO2
						// Do not have a handshake at the bdConnection layer


						// this is a new connection
						bdUnicastConnection newConnection(m_addrMap);
						newConnection.setAddressHandle(address);
						bdBool ok = newConnection.receive(data, bytesReceived);

						if(ok)
						{
							if(newConnection.getStatus() == bdConnection::BD_CONNECTED)
							{
								// this is an incoming connection that is now setup
								// so allocate it on the heap and record it
								const bdConnectionRef establishedConnection = new bdUnicastConnection(m_addrMap);
								establishedConnection->setAddressHandle(address);

								// process the packet
								ok = establishedConnection->receive(data, bytesReceived);

								if(ok && (establishedConnection->getStatus() == bdConnection::BD_CONNECTED))
								{
									if(m_connectionMap.put(address, establishedConnection))
									{
										BD_INFO(BD_LOG_LEVEL, "New incoming connection created.");
									}
								}
							}
							else
							{
								// give the connection a chance to send any connection packets
								flush(newConnection);
							}
						}
						else
						{
							result = false;
						}
					}
				}
				else
				{
					BD_ERR(BD_LOG_LEVEL, "Data & null addr handle ref received from socket route receiveFrom, ignoring.");
				}
			}
		}
		while(	(true == receivedData) &&
				(iterationCount++ < m_config.m_maxReceiveIterations ));
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot call receiveAll unless class is initialized.");
		result = false;
	}

	return result;
}

void bdConnectionStore::dispatchAll()
{
	// YIKES !! FIXME !! We can't call bdDispatcher::process() while iterating a hashmap
	bdArray<bdConnectionRef> connections (m_connectionMap.getSize());

	// First iterate hashmap to fill array of connection refs
	bdHashMap<bdAddrHandleRefWrapper, bdConnectionRef, bdAddrHandleRefWrapper>::Iterator it = m_connectionMap.getIterator();
	while(it)
	{
		connections.pushBack(m_connectionMap.getValue(it));
		m_connectionMap.next(it);
	}

	// process messages in each connection.
	for(bdUInt i=0 ; i< connections.getSize() ; i++)
	{
		m_dispatcher.process(connections[i]);
	}
}

bdBool bdConnectionStore::startShutdown(const bdConnectionStoreShutdownType shutdownType)
{
	bdBool result = false;

	if(BD_CONNECTION_STORE_UNINITIALIZED != m_status)
	{
		// There are several types of shutdown!
		// 1) BD_CONNECTION_STORE_SHUTDOWN_ELEGANT - perform elegant shutdown
		// 2) BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE - perform shutdown RIGHT NOW
		switch (shutdownType)
		{
			case BD_CONNECTION_STORE_SHUTDOWN_ELEGANT:
			{
				if(	(BD_CONNECTION_STORE_INITIALIZED == m_status) ||
					(BD_CONNECTION_STORE_ERROR == m_status))
				{
					bdHashMap<bdAddrHandleRefWrapper, bdConnectionRef, bdAddrHandleRefWrapper>::Iterator it = m_connectionMap.getIterator();
					while(it)
					{
						bdConnectionRef conn = m_connectionMap.getValue(it);
						conn->disconnect();
						m_connectionMap.next(it);
					}
					m_status = BD_CONNECTION_STORE_SHUTTING_DOWN;
					m_shutdownTimer.start();
					result = true;
				}
				else if(BD_CONNECTION_STORE_SHUTTING_DOWN == m_status)
				{
					BD_WARN(BD_LOG_LEVEL, "Already shutting down; Shutdown call ignored.");
				}
			}
			break;
			case BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE:
			{
				// be very quick about it..
				bdHashMap<bdAddrHandleRefWrapper, bdConnectionRef, bdAddrHandleRefWrapper>::Iterator it = m_connectionMap.getIterator();
				while(it)
				{
					bdConnectionRef conn = m_connectionMap.getValue(it);
					conn->disconnect();
					conn->close();
					m_connectionMap.next(it);
				}
				m_connectionMap.clear();
				m_dispatcher.reset();
				m_socket  = BD_NULL;
				m_status = BD_CONNECTION_STORE_UNINITIALIZED;
				m_shutdownTimer.reset();
				m_addrMap = BD_NULL;
				m_config.reset();
				result = true;
			}
			break;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Class is uninitialized, nothing for shutdown to do.");
	}
	return result;
}

bdConnectionRef bdConnectionStore::create(bdCommonAddrRef addr, const bdSecurityID &secID)
{
	bdConnectionRef connection;
	if(BD_CONNECTION_STORE_INITIALIZED == m_status)
	{
		if(addr.isNull())
		{
			BD_ERR(BD_LOG_LEVEL, "An invalid common addr ref used to create connection.");
		}
		else
		{
			bdAddrHandleRef addrHandle;
			if(BD_NULL != m_addrMap)
			{
				// first get the addr handle
				const bdBool getHandleResult = m_addrMap->getAddrHandle(addr, secID, addrHandle);
				if(true == getHandleResult)
				{
					// next check to make sure the connection isn't already in the map.
					if(m_connectionMap.get(addrHandle, connection))
					{
						BD_INFO(BD_LOG_LEVEL, "a connection already exists to host.");
#if (!defined(BD_PLATFORM_XBOX)) && (!defined(BD_PLATFORM_XENON))
						if( addrHandle->getStatus() == bdAddrHandle::BD_ADDR_NOT_RESOLVED &&
							m_socket->isRelayEnabled() )
						{
							// This connection exists but has not been constructed. This means
							// that they have sent us messages through a relay but no bdDTLS has
							// been set up. Try to set one up here.
							m_socket->connect(addrHandle);
						}
#endif
					}
					else
					{
						if (addr->isLoopback())
						{
							BD_INFO(BD_LOG_LEVEL, "Creating new loopback connection.");
							connection = new bdLoopbackConnection(addr);
							connection->setAddressHandle(addrHandle);
							if(false == m_connectionMap.put(addrHandle, connection))
							{
								// If we failed to add the con to the map then we will delete it.
								connection = BD_NULL;
							}
						}
						else
						{
							BD_INFO(BD_LOG_LEVEL, "Creating new unicast connection.");
							connection = new bdUnicastConnection(	addr,
																	m_addrMap);
							connection->setAddressHandle(addrHandle);
							bdBool ok = m_socket->connect(addrHandle);
							ok = ok && m_connectionMap.put(addrHandle, connection);
							if(false == ok)
							{
								// If we failed to start connecting or adding the
								// conn to the map then we will close & delete it.
								connection->disconnect();
								connection->close();
								connection = BD_NULL;
							}
						}
					}
				}
				else
				{
					// addr handle retrieval has failed.
					BD_WARN(BD_LOG_LEVEL, "Failed to create connection as addr handle retrieval failed.");
					connection = BD_NULL;
				}
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Addr map pointer not initialized.");
			}
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot create a connection when store is not initialized.");
	}

	return connection;
}

bdConnectionStore::bdConnectionStoreStatus bdConnectionStore::getStatus() const
{
	return m_status;
}

void bdConnectionStore::registerDispatchInterceptor(bdDispatchInterceptor *const interceptor)
{
	m_dispatcher.registerInterceptor(interceptor);
}

void bdConnectionStore::unregisterDispatchInterceptor(bdDispatchInterceptor *const interceptor)
{
	m_dispatcher.unregisterInterceptor(interceptor);
}

bdUInt bdConnectionStore::flush(bdConnection& connection)
{

	bdUByte8 data[BD_MAX_PAYLOAD_SIZE];
	const bdUInt size = connection.getDataToSend(data, sizeof(data));

	if(size)
	{
		// send to socket
		const bdAddrHandleRef addrHandle = connection.getAddressHandle();
		const bdInt status = m_socket->sendTo(addrHandle, data, size);

		if(status < 0)
		{
			const bdSocketStatusCode error = static_cast<bdSocketStatusCode>(status);
			switch(error)
			{
				case BD_NET_SUBSYTEM_ERROR:
				{
					// something very wrong.
					BD_WARN(BD_LOG_LEVEL, "net subsystem error!");
				}
				break;
				case BD_NET_ADDRESS_INVALID:
				{
					BD_INFO(BD_LOG_LEVEL, "Invalid address. Closing connection.");
					connection.close();
				}
				break;
				case BD_NET_ERROR:
				{
					// unknown error..
					BD_WARN(BD_LOG_LEVEL, "unknown error.");
				}
				break;
				case BD_NET_WOULD_BLOCK:
				{
					// doesn't happen.
					BD_WARN(BD_LOG_LEVEL, "would block.");
				}
				break;
				case BD_NET_ADDRESS_IN_USE:
				{
					// can't happen.. not binding
				}
				break;
				case BD_NET_CONNECTION_RESET:
				case BD_NET_MSG_SIZE:
				{
					// can't happen.. not receiving
				}
				break;
				default:
				{
					// TODO: it is better not to have a default here, and to simply handle.
					// all the possible values of the enum sensibly
				}
				break;
			}
		}
	}

	return size;
}

void bdConnectionStore::getConfig(bdConnectionStoreConfig &config) const
{
	config = m_config;
}
