// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddrInfo.h>
#include <bdCore/bdSocket/bdSecurityInfo.h>

#include <bdSocket/bdRelay/bdRelaySocketRouter.h>

#define BD_LOG_LEVEL "bdSocket/socket"

void bdRelaySocketRouter::pump()
{	
	m_qosProber.pump();
	m_sendTokens = 20;

	// Check all pending connections
	bdQueue<bdAddrHandleRef> toDisconnect;
	bdQueue<bdEndpoint> toRemove;
	bdHashMap<bdEndpoint, bdAddrHandleRef, bdEndpointHashingClass>::Iterator it = m_endpointToAddrMap.getIterator();
	while(it)
	{
		const bdEndpoint &endpoint = m_endpointToAddrMap.getKey(it);
		const bdAddrHandleRef &addrHandle = m_endpointToAddrMap.getValue(it);
		BD_ASSERT( addrHandle.notNull(), "AddrHandle should not be null");

		switch( XNetGetConnectStatus(addrHandle->m_addr) )
		{
			case XNET_CONNECT_STATUS_IDLE:
				BD_WARN(BD_LOG_LEVEL, "Invalid state!");
				break;
			case XNET_CONNECT_STATUS_PENDING:
				break;
			case XNET_CONNECT_STATUS_CONNECTED:
			{
				bdBool added = m_relayRouteManager.addDirectRoute( endpoint );
				if( added )
				{
					m_relayRouteManager.updateRelayerLoad( endpoint, BD_MAX_RELAYER_LOAD );
					broadcastNewRoute( endpoint );
					sendConnections( endpoint );
				}
				toRemove.enqueue( endpoint );	
			}
			break;
			case XNET_CONNECT_STATUS_LOST:
			{
				// This connection needs to be removed from our direct connections
				toDisconnect.enqueue(addrHandle);
			}
			break;
		}
		m_endpointToAddrMap.next(it);
	}
	m_endpointToAddrMap.releaseIterator(it);

	while( toRemove.isEmpty() == false )
	{
		bdEndpoint finished = toRemove.peek();
		m_endpointToAddrMap.remove( finished );
		toRemove.dequeue();
	}

	// Disconnect any dead connections. This will also remove them 
	// from m_endpointToAddrMap if they exist in there
	bdBool newRouteAvailable = false;
	while(!toDisconnect.isEmpty())
	{
		bdAddrHandleRef addrHandle = toDisconnect.peek();
		BD_ASSERT( addrHandle.notNull(), "AddrHandle should not be null");

		// Remove from the pending list if it is in there
		if( m_endpointToAddrMap.containsKey( addrHandle->m_endpoint ) )
		{
			m_endpointToAddrMap.remove( addrHandle->m_endpoint );
		}
	
		// Remove from the direct connections if it exists
		if( m_relayRouteManager.removeDirectRoute( addrHandle->m_endpoint ) )
		{
			//Broadcast to others that we are no longer connected to this relayer
			broadcastRemovedRoute( addrHandle->m_endpoint );			
			newRouteAvailable = isRelaying(addrHandle->m_endpoint);
		}

		toDisconnect.dequeue();
	}
	BD_ASSERT( toDisconnect.isEmpty(), "Failed to dequeue all connections" );

	// Now check direct connections
	bdArray<bdEndpoint> directConnections = m_relayRouteManager.getDirectEndpoints();
	bdUInt directConnectionsSize = directConnections.getSize();
	if (m_lastStatusIndex < directConnectionsSize)
	{
		bdAddrHandleRef addrHandle;
		if( m_addrMap.findAddrHandle(	directConnections[m_lastStatusIndex].getCommonAddr(),
										directConnections[m_lastStatusIndex].getSecID(),
										addrHandle ) )
		{
			if( XNetGetConnectStatus(addrHandle->m_addr) == XNET_CONNECT_STATUS_LOST )
			{
				toDisconnect.enqueue( addrHandle );
			}			
		}
		else
		{
			BD_ASSERT( false , "We have a direct connection without an addrHandle");
		}	
	}
	if (directConnectionsSize > 0)
	{
		m_lastStatusIndex++;
		m_lastStatusIndex %= directConnectionsSize;
	}

	// Disconnect any dead connections. This will also remove them 
	// from m_endpointToAddrMap if they exist in there
	while(!toDisconnect.isEmpty())
	{
		bdAddrHandleRef addrHandle = toDisconnect.peek();
		BD_ASSERT( addrHandle.notNull(), "AddrHandle should not be null");

		// Remove from the pending list if it is in there
		if( m_endpointToAddrMap.containsKey( addrHandle->m_endpoint ) )
		{
			m_endpointToAddrMap.remove( addrHandle->m_endpoint );
		}
	
		// Remove from the direct connections if it exists
		if( m_relayRouteManager.removeDirectRoute( addrHandle->m_endpoint ) )
		{
			//Broadcast to others that we are no longer connected to this relayer
			broadcastRemovedRoute( addrHandle->m_endpoint );
			//Determine if we were relaying for this connection
			newRouteAvailable = isRelaying(addrHandle->m_endpoint);
		}

		toDisconnect.dequeue();
	}
	BD_ASSERT( toDisconnect.isEmpty(), "Failed to dequeue all connections" );

	if (newRouteAvailable)
	{
		broadcastNewRouteSlot();
	}

	pumpTimer();
	pumpRelayConnections();
}

bdBool bdRelaySocketRouter::connect(bdAddrHandleRef &addrHandle)
{
	bdBool success = false;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot connect before class has been initialized");
	}
	else if(addrHandle.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot connect using null addr handle ref");
	}
	else
	{ 
		success = (XNetConnect(addrHandle->m_addr) == 0);
		if( success )
		{
			if( m_endpointToAddrMap.containsKey( addrHandle->m_endpoint ) == false )
			{
				// Add to our pending connections list.
				m_endpointToAddrMap.put( addrHandle->m_endpoint, addrHandle );
			}
		}
	}
	return success;
}

bdInt bdRelaySocketRouter::sendTo(	const bdAddrHandleRef addrHandle,
									void *data,
									const bdUInt length)
{
	bdInt retVal = BD_NET_ERROR; 
	
	// Sanity Checks
	BD_ASSERT( m_status == BD_SOCKET_ROUTER_INITIALIZED, "Socket Router Not Initialized");
	BD_ASSERT( addrHandle.notNull(), "Cannot send to null addrHandle" );

	// Check if a route exists
	bdEndpoint destEndpoint = addrHandle->m_endpoint;
	bdEndpoint relayEndpoint;

	if( m_relayRouteManager.findRoute( destEndpoint, relayEndpoint ) )
	{
		if( destEndpoint == relayEndpoint )
		{
			retVal = sendWithBuffering(addrHandle, data, length);
		}
		else
		{

			// Send via relayer
			const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
			bdUByte8 buffer[bufferSize];

			bdRelayForward forwardPacket( bdEndpoint( m_localCommonAddr, relayEndpoint.getSecID() ), destEndpoint );
			bdUInt packetLen = forwardPacket.serialize( buffer, bufferSize, reinterpret_cast<bdUByte8 *>( data ), length );
			if( packetLen > 0  )
			{
				retVal = sendRelayPacket( relayEndpoint, buffer, packetLen );
			}
		}
	}
	else if( bdSocketRouter::getStatus( addrHandle ) == BD_SOCKET_CONNECTED )
	{
		BD_WARN("warn","Not found in list....attempting direction connection");
		// Just send it here. It will be added to our direct list on the next pump.
		retVal = sendWithBuffering(addrHandle, data, length);
	}

	return retVal;
}

bdInt bdRelaySocketRouter::sendRelayPacket( const bdEndpoint destEndpoint, void *data, const bdUInt length )
{
	bdInt retVal = BD_NET_ERROR;

	bdAddrHandleRef destAddrHandle;
	if( m_addrMap.findAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), destAddrHandle ) )
	{
		// Send it off
		retVal = bdSocketRouter::sendTo( destAddrHandle, data, length );
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "No Addr Handle could be found");
	}

	return retVal;
}

bdInt bdRelaySocketRouter::receiveFrom(	bdAddrHandleRef &addrHandle,
										void *data,
										const bdUInt size,
										bool& didErrorOccurDummy)
{
	bdInt val = BD_NET_WOULD_BLOCK;
	bdUByte8 tempData[BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE];
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot receive data before class has been initialized");
	}
	else
	{
		bdBool receiving = true;
		while( receiving )
		{
			bool didErrorOccur;

			val = bdSocketRouter::receiveFrom( addrHandle, tempData, BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE, didErrorOccur );
			if(val > 0)
			{
				m_bytesReceived += val;

				// Check if this is a new direct connection
				if( addrHandle.notNull()  && !didErrorOccur )
				{
					// Add this connection to our direct connections. It will be removed
					// from the pending list on the next pump
					if( m_relayRouteManager.addDirectRoute( addrHandle->m_endpoint ) )
					{
						m_relayRouteManager.updateRelayerLoad( addrHandle->m_endpoint, BD_MAX_RELAYER_LOAD );

						broadcastNewRoute( addrHandle->m_endpoint );

						sendConnections( addrHandle->m_endpoint);
					}

					m_missedInfoReplies.remove( addrHandle->m_endpoint );
				}

				// Check for the relay packet indicator. If it exists, replace it with 
				// a normal BD_DTLS_DATA packet type so that the dtls connection can
				// decrypt it.
				bdUByte8* dataPtr = tempData;

				bdUInt16 totalEncryptedSize = 0;
				bdUInt newOffsetTemp = 0;
				bdBytePacker::removeBasicType(dataPtr, size, 0, newOffsetTemp, totalEncryptedSize);
				dataPtr += sizeof(bdUInt16);
				//Reduce the packet size by the total encrypted size
				val -= sizeof(bdUInt16);

				bdInt totalBytesRead = 0;
				bdBool chunkToProcess = true;
				while (chunkToProcess)
				{
					const bdUByte8 type = *(reinterpret_cast<bdUByte8 *>(dataPtr) + sizeof(bdUInt16));
					if( type == BD_DIRECT )
					{
						val = val - totalBytesRead;					
						bdMemcpy(data, dataPtr,val);
						// Decrement encoded size
						bdUInt16 encDataSize = 0;
						bdUInt newOffset = 0;
						bdBytePacker::removeBasicType(data, size, 0, newOffset, encDataSize);



						BD_ASSERT( encDataSize < val, "EncDataSize is too big");
						encDataSize--;
						bdBytePacker::appendBasicType( data, size, 0, newOffset, encDataSize );

						// remove byte
						bdMemmove(	reinterpret_cast<bdUByte8 *>(data)+sizeof(bdUInt16), 
							reinterpret_cast<const bdUByte8 *>(data)+sizeof(bdUInt16) + sizeof(bdUByte8),
							val - 1 );

						val--;
						
						receiving = false;
						chunkToProcess = false;
					}
					else
					{		
						BD_ASSERT( type >= BD_RELAY_FORWARD && type <= BD_RELAYER_INFO_REPLY, "Packet is of unknown type (%d) from %d.%d.%d.%d", type,
							addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b1,
							addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b2,
							addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b3,
							addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b4);

						bdUInt32 gameDataRead = 0;
						bdInt bytesRead = processRelayPacket( addrHandle, dataPtr, val, receiving, data, gameDataRead);
						dataPtr += bytesRead;
						totalBytesRead += bytesRead;
						if (val == -1 || bytesRead <= 0)
						{
							chunkToProcess = false;
							//receiving = false;
							val = BD_NET_ERROR;
						}
						else if (gameDataRead > 0)
						{
							val = bytesRead;
							receiving = false;
							chunkToProcess = false;
						}
						else if ( totalBytesRead >= val)
						{
							//receiving = false;
							chunkToProcess = false;
							//TODO: Add error checks for the val?
							val = BD_NET_WOULD_BLOCK;						
						}
					}
				}				
			}
			else
			{
				receiving = false;
			}
		}
	}
	return val;
}

void bdRelaySocketRouter::broadcastPacket( bdUByte8* buffer, const bdUInt length, const bdEndpoint *dontSend )
{
	bdArray<bdEndpoint> directConns = m_relayRouteManager.getDirectEndpoints();
	for(bdUInt i = 0; i < directConns.getSize(); i++ )
	{
		const bdEndpoint &destEndpoint = directConns[i];
		bdAddrHandleRef addrHandle;
		m_addrMap.getAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), addrHandle );
		
		// Inform everyone about the new route except the new route itself
		if( addrHandle.notNull() )
		{
			bdBool send = true;
			if( dontSend != BD_NULL )
			{
				send = !( *dontSend == destEndpoint );
			}

			if( send )
			{
				bdSocketRouter::sendTo( addrHandle, buffer, length );
			}
		}
	}
}

void bdRelaySocketRouter::broadcastInfoRequests()
{
	//BD_INFO(BD_LOG_LEVEL, "Broadcasting info requests" );

	// Prepare the latency request packet
	bdRelayerInfo infoRequest( BD_RELAYER_INFO_REQUEST );
	bdArray< bdEndpoint > relayedEndpoints = m_relayRouteManager.getRelayedEndpoints();

	if( relayedEndpoints.isEmpty() == false )
	{
		//BD_INFO(BD_LOG_LEVEL, "Sending requests for %u relayed endpoints", relayedEndpoints.getSize() );
		for( bdUInt i= 0 ; i< relayedEndpoints.getSize(); i++ )
		{
			infoRequest.addLatencyRequest( relayedEndpoints[i] );
		}
	}

	// We are broadcasting new info request packets. Find all the requests that did not receive a response
	// and update their latencies to a high value.
	bdHashMap<bdEndpoint, bdUInt64, bdEndpointHashingClass>::Iterator pendingIt = m_pendingInfoReplies.getIterator();
	while( pendingIt )
	{
		const bdEndpoint &endpoint = m_pendingInfoReplies.getKey( pendingIt );

		bdUInt16 currLatency = 0;
		bdUInt16 newLatency = 0;
		if( m_relayRouteManager.getEndpointLatency( endpoint, endpoint, currLatency ) )
		{
			newLatency = (bdUInt)(currLatency * ( 1 - BD_RELAY_BITRATE_CALC_ALPHA )
								+ BD_RELAY_BITRATE_CALC_ALPHA * BD_RELAY_DEFAULT_LATENCY );
		}
		else
		{
			newLatency = BD_RELAY_DEFAULT_LATENCY;
		}

		//BD_INFO(BD_LOG_LEVEL, "Failed to receive relayInfo from %u", endpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort() );
		m_relayRouteManager.updateEndpointLatency( endpoint, endpoint, newLatency);

		// Check how many times this connection has failed to reply and disconnect 
		// if it exceeds the max
		bdUInt missedTimes = 0;
		if( m_missedInfoReplies.containsKey( endpoint ) )
		{
			m_missedInfoReplies.get( endpoint, missedTimes );
			m_missedInfoReplies.remove( endpoint );
		}
		missedTimes++;
		
		if( missedTimes >= BD_DIRECT_MAX_MISSED_REPLIES )
		{
			// Remove from the pending list if it is in there
			if( m_endpointToAddrMap.containsKey( endpoint ) )
			{
				m_endpointToAddrMap.remove( endpoint );
			}

			// Remove from the direct connections if it exists
			if( m_relayRouteManager.removeDirectRoute( endpoint ) )
			{
				//Broadcast to others that we are no longer connected to this relayer
				broadcastRemovedRoute( endpoint );

				//If this connection was relaying through us, inform everyone that there is a new slot available
				if (isRelaying(endpoint))
				{
					broadcastNewRouteSlot();
				}
			}			
		}
		else
		{
			m_missedInfoReplies.put( endpoint, missedTimes );
		}

		m_pendingInfoReplies.next(pendingIt);
	}
	m_pendingInfoReplies.clear();


	// Send off the requests
	#if defined BD_BUFFER_INFO_REQUESTS
		bufferRelayInfoMessages(infoRequest);
	#else
		bdArray<bdEndpoint> directConns = m_relayRouteManager.getDirectEndpoints();
		for(bdUInt i = 0; i < directConns.getSize(); i++ )
		{
			const bdEndpoint &destEndpoint = directConns[i];
			bdAddrHandleRef addrHandle;
			m_addrMap.getAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), addrHandle );

			bdUInt position = 0;
					
			while ( true && addrHandle.notNull() )
			{
					const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
					bdUByte8 buffer[bufferSize];

					bdUInt packetLen = infoRequest.serialize( buffer, bufferSize, position, position );

					if( packetLen > 0 )
					{
						//BD_INFO(BD_LOG_LEVEL, "Sending infoRequest to %u", destEndpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort() ); 
						bdSocketRouter::sendTo( addrHandle, buffer, packetLen );

						// Store the pending request along with the request time in milliseconds
						m_pendingInfoReplies.put( destEndpoint, bdPlatformTiming::getHiResTimeStamp() );

						if( position >= infoRequest.getRelayerLatencies().getSize() )
						{
							break;
						}
					}
					else
					{
						BD_WARN(BD_LOG_LEVEL, "Error serializing packet");
						break;
					}
			}			
		}
	#endif
}


