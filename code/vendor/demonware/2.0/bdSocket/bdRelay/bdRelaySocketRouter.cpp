// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddrInfo.h>
#include <bdCore/bdSocket/bdSecurityInfo.h>
#include <bdSocket/bdRelay/bdRelaySocketRouter.h>

#define BD_LOG_LEVEL "bdSocket/socket"

bdRelaySocketRouter::bdRelaySocketRouter()
	: m_bytesForwarded( 0 )
	, m_averageRelayBitrate( 0 )
	, m_bytesReceived( 0 )
	, m_averageBitrate( 0 )
	, m_relayerLoad( 0 )
	, m_sendTokens (2)
	, m_lastStatusIndex(0)
	, m_lastRelayStatusIndex(0)
	, m_maxRelayedConnections(-1)
{
	m_bitrateCalcTimer.start();
}

bdRelaySocketRouter::~bdRelaySocketRouter()
{
}

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
void bdRelaySocketRouter::pump()
{
	bdBool newRouteAvailable = false;

	m_natTrav.pump();
	m_qosProber.pump();

	// Add all the newly established DTLS connections to our map
	bdQueue<bdAddrHandleRef> toRemove;
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		const bdEndpoint &endpoint = m_dtls.getKey(it);
		bdDTLSAssociation *dtlsConn = m_dtls.getValue(it);
		dtlsConn->pump();

		switch( dtlsConn->getStatus())
		{
			case BD_SOCKET_IDLE:
				BD_WARN(BD_LOG_LEVEL, "Invalid state!");
				break;
			case BD_SOCKET_PENDING:
				break;
			case BD_SOCKET_CONNECTED:
			{
				bdBool added = m_relayRouteManager.addDirectRoute( endpoint );
				if( added )
				{
					m_relayRouteManager.updateRelayerLoad( endpoint, BD_MAX_RELAYER_LOAD );
					broadcastNewRoute( endpoint );
					sendConnections( endpoint );
				}
			}
			break;
			case BD_SOCKET_LOST:
			{
				toRemove.enqueue(dtlsConn->getAddrHandle());
				newRouteAvailable = isRelaying(endpoint);
			}
			break;
		}
		m_dtls.next(it);
	}
	m_dtls.releaseIterator(it);

	//Disconnect any dead connections
	while(!toRemove.isEmpty())
	{
		bdAddrHandleRef addrHandle = toRemove.peek();
		disconnect(addrHandle);
		toRemove.dequeue();
	}

	if (newRouteAvailable)
	{
		broadcastNewRouteSlot();
	}

	pumpTimer();
	pumpRelayConnections();

}
#endif

bdBool bdRelaySocketRouter::quit()
{
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
	m_endpointToAddrMap.clear();
#endif
	m_relayConnections.clear();
	m_relayRouteManager.quit();
	m_relayerLoad = 0;

	return bdSocketRouter::quit();
}

bdBool bdRelaySocketRouter::disconnect(bdAddrHandleRef &addrHandle, bdBool allowDisconnectBroadcast)
{
	bdBool success = false;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		success = false;
		BD_WARN(BD_LOG_LEVEL, "Cannot disconnect before class has been initialized");
	}
	else if(addrHandle.isNull())
	{
		success = false;
		BD_WARN(BD_LOG_LEVEL, "Cannot disconnect using null addr handle ref");
	}
	else
	{
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		// Remove from the pending list if it is in there
		if( m_endpointToAddrMap.containsKey( addrHandle->m_endpoint ) )
		{
			m_endpointToAddrMap.remove( addrHandle->m_endpoint );
		}		
#endif

		// Remove from the direct connections if it exists
		if( m_relayRouteManager.removeDirectRoute( addrHandle->m_endpoint ) )
		{
			if (allowDisconnectBroadcast)
			{
				//Broadcast to others that we are no longer connected to this relayer
				broadcastRemovedRoute( addrHandle->m_endpoint );
			}

			//If this connection was relaying through us, inform everyone that there is a new slot available
			if (isRelaying(addrHandle->m_endpoint))
			{
				broadcastNewRouteSlot();
			}
		}

		success = bdSocketRouter::disconnect( addrHandle );
	}
	return success;
}

bdDTLSAssociationStatus bdRelaySocketRouter::getStatus(const bdAddrHandleRef addrHandle)
{
		bdDTLSAssociationStatus retVal = BD_SOCKET_IDLE;
	if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot get status before class has been initialized");
	}
	else if(addrHandle.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot get status using null addr handle ref");
	}
	else
	{ 
		const bdEndpoint endpoint = addrHandle->m_endpoint;
		bdEndpoint tmp;

		if( m_relayRouteManager.findRoute( endpoint, tmp ) )
		{
			// If a relayer exists return connected.
			retVal = BD_SOCKET_CONNECTED;
		}
		else
		{
			// Otherwise return the direct dtls status
			retVal = bdSocketRouter::getStatus( addrHandle );
		}
	}
	return retVal;
}

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
bdInt bdRelaySocketRouter::sendTo(	const bdAddrHandleRef addrHandle,
									void *data,
									const bdUInt length)
{
	bdInt retVal = BD_NET_ERROR;

	// Sanity Checks
	BD_ASSERT( m_status == BD_SOCKET_ROUTER_INITIALIZED, "Socket Router Not Initialized");
	BD_ASSERT( addrHandle.notNull(), "Cannot send to null addrHandle" );

	// Check if a direct route exists
	bdEndpoint destEndpoint = addrHandle->m_endpoint;
	bdEndpoint relayEndpoint;

	if( m_relayRouteManager.findRoute( destEndpoint, relayEndpoint ) )
	{
		if( destEndpoint == relayEndpoint )
		{
			bdAddrHandleRef address = addrHandle;
			if( address->getRealAddr().getAddress().isValid() == false )
			{
				BD_INFO(BD_LOG_LEVEL, "Attempt to send to invalid real addr. Will try to set" );
				m_addrMap.findAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), address );
			}

			// A direct route exists. Send directly.
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
	else
	{
		// This connection may exist and not be in the relay route manager yet, if we have not
		// pumped since it was created.
		bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
		bdUInt found = 0;
		while (it)
		{
			bdDTLSAssociation *conn = m_dtls.getValue(it);
			if (*(conn->getAddrHandle()) == *addrHandle)
			{
				found = true;
				break;
			}
			else
			{
				m_dtls.next(it);
			}
		}
		m_dtls.releaseIterator(it);

		if (found)
		{
			// Just send it here. It will be added to our direct list on the next pump.
			retVal = sendWithBuffering(addrHandle, data, length);
		}
		else
		{
			// no route exists
			BD_ERR( BD_LOG_LEVEL, "No route exists to endpoint %u, either direct or indirect",
				addrHandle->m_endpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort()  );
		}
	}
	return retVal;
}

bdInt bdRelaySocketRouter::sendRelayPacket( const bdEndpoint destEndpoint, void *data, const bdUInt length )
{
	bdInt retVal = BD_NET_ERROR;

	bdAddrHandleRef destAddrHandle;
	if( m_addrMap.findAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), destAddrHandle )  )
	{
		if(BD_SOCKET_ROUTER_INITIALIZED != m_status)
		{
			BD_WARN(BD_LOG_LEVEL, "Cannot send data before class has been initialized");
		}
		else if(destAddrHandle.isNull())
		{
			BD_WARN(BD_LOG_LEVEL, "Cannot send data to null addr handle ref");
		}
		else
		{
			const bdAddr &realAddr = destAddrHandle->getRealAddr();

			bdDTLSAssociation *dtlsSocket = BD_NULL;
			m_dtls.get(destAddrHandle->m_endpoint, dtlsSocket);

			if(dtlsSocket)
			{
				const bdUInt outDataSize = BD_MAX_DATAGRAM_SIZE;
				bdUByte8 outData[outDataSize];

				// Create the dtls packet exactly as a normal connection but do not send it yet.
				bdUInt packetLen = dtlsSocket->prepareData( data, length, outData, outDataSize );

				if( packetLen > 0 )
				{
					// Alter the dtls type byte to indicate a relay packet and send it off
					outData[0] = BD_RELAY;
					retVal = m_socket->sendTo( realAddr, outData, packetLen );
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "Error preparing data packet");
				}
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Not established! Call connect() first.");
				bdNChar8 commonAddrInfo[BD_COMMON_ADDR_INFO_SIZE];
				bdCommonAddrInfo::getBriefInfo(destAddrHandle->m_endpoint.getCommonAddr(), commonAddrInfo, BD_COMMON_ADDR_INFO_SIZE);
				BD_INFO(BD_LOG_LEVEL, commonAddrInfo);
			}
		}
	}
	else
	{
		BD_ERR( BD_LOG_LEVEL, "Relay endpoint has no addrHandle or the addrHandle is invalid. This should not happen." );
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
		bdAddr realAddr;

		bdUInt resetCount = 0;
		while(receiving)
		{

			val = m_socket->receiveFrom(realAddr, tempData, BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE);
			if(val > 0)
			{
				m_bytesReceived += val;

				// Check for the relay packet indicator. If it exists, replace it with
				// a normal BD_DTLS_DATA packet type so that the dtls connection can
				// decrypt it.
				const bdUByte8 realType = *(reinterpret_cast<const bdUByte8 *>(tempData));
				if( realType == BD_RELAY )
				{
					*(reinterpret_cast<bdUByte8 *>(tempData)) = (bdUByte8)BD_DTLS_DATA;
				}

				// Do the DTLS receive. This will decrypt the packet for us
				val = processPacket( addrHandle, realAddr, tempData, size, val, receiving);


				if( val > 0 && addrHandle.notNull() && realType == BD_RELAY )
				{
					receiving = true;
					if( m_relayRouteManager.addDirectRoute( addrHandle->m_endpoint ) )
					{
						m_relayRouteManager.updateRelayerLoad( addrHandle->m_endpoint, BD_MAX_RELAYER_LOAD );
						broadcastNewRoute( addrHandle->m_endpoint );
						sendConnections( addrHandle->m_endpoint );
					}

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
								addrHandle->m_endpoint.getCommonAddr()->getPublicAddr().getAddress().getInAddr().inUn.m_caddr.m_b1,
								addrHandle->m_endpoint.getCommonAddr()->getPublicAddr().getAddress().getInAddr().inUn.m_caddr.m_b2,
								addrHandle->m_endpoint.getCommonAddr()->getPublicAddr().getAddress().getInAddr().inUn.m_caddr.m_b3,
								addrHandle->m_endpoint.getCommonAddr()->getPublicAddr().getAddress().getInAddr().inUn.m_caddr.m_b4
								);

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
#endif

bdInt bdRelaySocketRouter::sendWithBuffering(const bdAddrHandleRef addrHandle,
											void *data,
											const bdUInt length)
{	
	bdInt retVal = BD_NET_ERROR; 

	const bdUInt outDataSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 outData[outDataSize];

	bdUByte8* writePtr = outData + sizeof(bdUInt16); //Moved forward 2 bytes to accomodate encrypted data size at the start


	bdUInt16 encDataSize = 0;
	bdUInt newOffset = 0;
	bdUInt32 totalLength = length;

	
	//Calculate how much available space is left in the game packet
	//available space     = total packet size - game packet length - 2 bytes for total encrypted size - 2 bytes for game packet encrypted size - 1 byte for game packet type
	bdInt availableSpace = outDataSize - length - sizeof(bdUInt16) - sizeof(bdUInt16) - sizeof(bdUByte8);
	
	//Write the relay data in to the remaining available space
	bdUInt16 relaySize = writeRelayData(addrHandle, availableSpace, writePtr);

	//Update packet length to consider relay data
	totalLength += relaySize;
						
	//Get the current size of the encrypted portion of the game packet
	bdBytePacker::removeBasicType(data, length, 0, newOffset, encDataSize);
	//Increase the size to accomadate addition of BD_DIRECT flag added below
	encDataSize++;
	totalLength++;

	//Write the total encrypted size, which is game packet size plus size of relay info, to the start of the packet    
	bdUByte8* packetStart = outData; 
	bdUInt16 totalEncryptedLength = encDataSize + relaySize + sizeof(bdUInt16);
	bdBytePacker::appendBasicType( packetStart, outDataSize, 0, newOffset, totalEncryptedLength );

	//Increase totalLength to accomodate addition of totalEncryptedLength
	totalLength += sizeof(bdUInt16);

	//Write the game packet encrypted size after the buffered relay related data
	bdBytePacker::appendBasicType( writePtr, outDataSize, 0, newOffset, encDataSize );
	writePtr += sizeof(bdUInt16);
			
	// Write the flag that indicates this is a direct packet
	bdUByte8 type = BD_DIRECT;			
	bdMemcpy( writePtr , &type, sizeof(bdUByte8) );	
	writePtr += sizeof(bdUByte8);

	// Write the game data after the flag
	bdMemcpy( writePtr,
				reinterpret_cast<bdUByte8*>(data) + sizeof(bdUInt16),
				length - sizeof(bdUInt16) );
	
	// A direct route exists. Send directly.
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON 

	retVal = bdSocketRouter::sendTo( addrHandle, outData, totalLength );
#else
	retVal = sendRelayPacket( addrHandle->m_endpoint, outData, totalLength );
#endif

	//Flush any existing relay messages that could not be appended to the game packet
	flushRelayMessages(addrHandle);

	return retVal;
}

void bdRelaySocketRouter::setRelayerLoad( bdUInt32 load )
{
	if( load > BD_MAX_RELAYER_LOAD )
	{
		load = BD_MAX_RELAYER_LOAD;
	}
	m_relayerLoad = load;
}

bdBool bdRelaySocketRouter::getRelayerStats(bdUInt32 &averageRelayBitrate,
											bdUInt32 &averageTotalBitrate,
											bdUInt32 &load,
											bdUInt32 &numRelayConnections )
{
	bdBool ok = false;
	if( m_status == BD_SOCKET_ROUTER_INITIALIZED )
	{
		ok = true;
		averageRelayBitrate = m_averageRelayBitrate;
		averageTotalBitrate = m_averageBitrate;
		load = m_relayerLoad;
		numRelayConnections = m_relayConnections.getSize();
	}
	return ok;
}

bdBool bdRelaySocketRouter::isRelayEnabled() const
{
	return true;
}

void bdRelaySocketRouter::print()
{
	BD_INFO( BD_LOG_LEVEL, "I am forwarding %u bits/s", m_averageRelayBitrate );
	m_relayRouteManager.print();
}

void bdRelaySocketRouter::pumpTimer()
{
	// Calculate the new bitrate forwarded and total
	if( m_bitrateCalcTimer.getElapsedTimeInSeconds() > BD_RELAY_BITRATE_CALC_PERIOD )
	{
		m_averageRelayBitrate = bdUInt32(m_averageRelayBitrate * ( 1 - BD_RELAY_BITRATE_CALC_ALPHA )
			+ BD_RELAY_BITRATE_CALC_ALPHA * m_bytesForwarded * 8 / BD_RELAY_BITRATE_CALC_PERIOD);
		m_bytesForwarded = 0;

		m_averageBitrate = bdUInt32(m_averageBitrate * ( 1 - BD_RELAY_BITRATE_CALC_ALPHA )
			+ BD_RELAY_BITRATE_CALC_ALPHA * m_bytesReceived * 8 / BD_RELAY_BITRATE_CALC_PERIOD);
		m_bytesReceived = 0;

		// We have calculated a new set of stats. Ask our peers what their stats are.
		broadcastInfoRequests();

		m_bitrateCalcTimer.start();
	}
}

void bdRelaySocketRouter::pumpRelayConnections()
{
	bdUInt relayConnectionsSize = m_relayConnections.getSize();

	if (m_lastRelayStatusIndex < relayConnectionsSize)
	{
		if( m_relayConnections[m_lastRelayStatusIndex].getTimeSinceLastUsed() > BD_RELAY_CONNECTION_TIMEOUT )
		{
			m_relayConnections.removeSection(m_lastRelayStatusIndex,m_lastRelayStatusIndex+1);

		}
	}

	if (relayConnectionsSize > 0)
	{
		m_lastRelayStatusIndex++;
		m_lastRelayStatusIndex %= relayConnectionsSize;
	}
}

bdInt bdRelaySocketRouter::processRelayPacket(	bdAddrHandleRef &addrHandle,
												void *data,
												const bdUInt bytesTransferred,
												bdBool &receiving,
												void *gameData,
												bdUInt& gameDataRead)
{
	bdInt retVal = BD_NET_WOULD_BLOCK;
	//receiving = true;
	
	// Read the header type and pass to the proper handler
	const bdUByte8 packetType = *(reinterpret_cast<bdUByte8 *>(data) + sizeof(bdUInt16));

	switch( packetType )
	{
		case BD_RELAY_FORWARD:
		{
			retVal = processRelayForward( addrHandle, data, bytesTransferred );
		}
		break;
		case BD_RELAY_FINAL:
		{
			retVal = processRelayFinal( addrHandle, data, bytesTransferred, gameData );
			gameDataRead = retVal;
			//receiving = false;
		}
		break;
		case BD_RELAY_ROUTE_ADD:
		{
			retVal = processRelayRouteAdd( addrHandle, data, bytesTransferred );
		}
		break;
		case BD_RELAY_ROUTE_REMOVE:
		{
			retVal = processRelayRouteRemove( addrHandle, data, bytesTransferred );
		}
		break;
		case BD_RELAYER_INFO_REQUEST:
		{
			retVal = processRelayInfoRequest( addrHandle, data, bytesTransferred );
		}
		break;
		case BD_RELAYER_INFO_REPLY:
		{
			retVal = processRelayInfoReply( addrHandle, data, bytesTransferred );
		}
		break;
		default:
		{
			BD_WARN(BD_LOG_LEVEL, "Unknown Packet Type");
		}
		break;
	}
	return retVal;
}

bdInt bdRelaySocketRouter::processRelayForward(bdAddrHandleRef &addrHandle,
												void *data,
												const bdUInt length )
{
	//BD_INFO( BD_LOG_LEVEL, "Processing a relay forward packet" );
	bdInt retVal = BD_NET_WOULD_BLOCK;
	bdRelayForward forwardPacket;

	bdUInt payloadSize = 0;
	bdUByte8 *payload = BD_NULL;
	bdUInt bytesRead = forwardPacket.deserialize2( data, length, &payload, payloadSize, m_localCommonAddr );
	
	if( bytesRead > 0)
	{
		bdBool canForwardPacket = checkRouteFowardSlotAvailable(forwardPacket);
		
		if (canForwardPacket)
		{
			// Build the packet header that marks this as the "final" step in a relay
			bdRelayFinal finalPacket( forwardPacket.getSourceEndpoint() );

			const bdUInt outBufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
			bdUByte8 outBuffer[outBufferSize];

			bdUInt packetLen = finalPacket.serialize( outBuffer, outBufferSize, payload, payloadSize );
			
			bdBool forwardFailed = false;
			if( packetLen > 0 )
			{
				// Send it off
				
				bdInt finalSize = sendRelayPacket( forwardPacket.getDestinationEndpoint(), outBuffer, packetLen );
				if( finalSize < 0 )
				{
					/// We do not have a connection to this destination. Inform the sender
					/// that we do not have this connection.
					BD_WARN("warn", "Forward failed");
					forwardFailed = true;
					sendRemovedRoute( forwardPacket.getSourceEndpoint(), forwardPacket.getDestinationEndpoint() );
				}
				else
				{
					///	Increment bytesForwareded appropriately
					m_bytesForwarded += packetLen;
				}
			}
			else
			{
				retVal = BD_NET_ERROR;
			}

			// Update our list of active relay connections. Restart the timer if the connection exists
			// and add it if it does not.
			bdBool found = false;
			bdUInt incomingSourceHash = forwardPacket.getSourceEndpoint().getCommonAddr()->getHash();
			bdUInt incomingDestinationHash = forwardPacket.getDestinationEndpoint().getCommonAddr()->getHash();
			for( bdUInt i=0; i<m_relayConnections.getSize() && !forwardFailed; i++)
			{
				bdUInt currentSourceHash =  m_relayConnections[i].getSourceEndpoint().getCommonAddr()->getHash();
				bdUInt currentDestinationHash = m_relayConnections[i].getDestinationEndpoint().getCommonAddr()->getHash();	

				if( currentSourceHash == incomingSourceHash
 				 && currentDestinationHash == incomingDestinationHash )
				{
					// We have received a forward packet for this connection, restart the timer.
					m_relayConnections[i].restartLastUsed();
					found = true;
				}

						//Also need to refresh the timer in the opposite direction
				if( currentSourceHash == incomingDestinationHash
 				 && currentDestinationHash == incomingSourceHash )
				{
					m_relayConnections[i].restartLastUsed();
				}
			}

			// If it does not exist, create it and put it in the list
			if( !found && !forwardFailed )
			{
				forwardPacket.restartLastUsed();
				m_relayConnections.pushBack( forwardPacket );

				//Mark both directions as active, as recipient will return in this direction
				bdRelayForward returnPacket(forwardPacket.getDestinationEndpoint(), forwardPacket.getSourceEndpoint());
				returnPacket.restartLastUsed();
				m_relayConnections.pushBack( returnPacket );
			}
		}
	}
	else
	{
		retVal = BD_NET_ERROR;
	}
	return bytesRead;
}

bdInt bdRelaySocketRouter::processRelayFinal(bdAddrHandleRef &addrHandle,
											void *data,
											const bdUInt length,
											void *gameData)
{
	bdEndpoint relayerEndpoint = addrHandle->m_endpoint;

	bdInt retVal = BD_NET_WOULD_BLOCK;

	bdRelayFinal finalPacket;
	bdUInt payloadSize = 0;
	bdUByte8 *payload = BD_NULL;
	bdUInt32 bytesRead = finalPacket.deserialize2( data, length, &payload, payloadSize, m_localCommonAddr );
	
	bdCommonAddrRef finalPacketSourceCommonAddr = finalPacket.getSourceEndpoint().getCommonAddr();

	if( bytesRead > 0 )	
	{
		m_relayRouteManager.addReturnRoute(relayerEndpoint, finalPacket.getSourceEndpoint(), getLocalCommonAddr());
		bdMemcpy( reinterpret_cast<bdUByte8 *>(gameData), payload, payloadSize );
		// Make it appear as if the packet came from the source, rather than from the relay
		if( m_addrMap.findAddrHandle( finalPacket.getSourceEndpoint().getCommonAddr(), finalPacket.getSourceEndpoint().getSecID(), addrHandle ) )
		{
			/// ...and that we received exactly as many bytes as the payload
			retVal = (bdInt)payloadSize;
		}
		else
		{
			// This addrHandle does not exist. Create it.
			m_addrMap.getAddrHandle( finalPacket.getSourceEndpoint().getCommonAddr(), 
									finalPacket.getSourceEndpoint().getSecID(), 
									addrHandle );
			if( addrHandle.notNull() )
			{
				retVal = (bdInt)payloadSize;
			}
			else
			{
				retVal = BD_NET_ERROR;
			}
		}
	}
	else
	{
		retVal = BD_NET_ERROR;
	}
	return retVal;
}

bdInt bdRelaySocketRouter::processRelayRouteAdd(bdAddrHandleRef &addrHandle,
												void *data,
												const bdUInt length )
{
	bdInt retVal = BD_NET_WOULD_BLOCK;
	bdRelayRouteAdd routeAdd;
	bdEndpoint relayerEndpoint = addrHandle->m_endpoint;

	bdUInt32 bytesRead = routeAdd.deserialize2( data, length, m_localCommonAddr );
	if( bytesRead > 0 )
	{
		const bdArray<bdEndpoint> newRoutes = routeAdd.getEndpoints();
		for( bdUInt i = 0; i < newRoutes.getSize(); ++i )
		{
			m_relayRouteManager.addRelayedRoute( relayerEndpoint, newRoutes[i] );
		}

		// Rebuild the routes table right away
		//m_relayRouteManager.pump();

		//m_relayRouteManager.print();
	}
	else
	{
		retVal = BD_NET_ERROR;
	}

	return bytesRead;
}

bdInt bdRelaySocketRouter::processRelayRouteRemove(bdAddrHandleRef &addrHandle,
													void *data,
													const bdUInt length )
{
	bdInt retVal = BD_NET_WOULD_BLOCK;
	bdRelayRouteRemove routeRemove;
	bdEndpoint relayerEndpoint = addrHandle->m_endpoint;

	bdUInt bytesRead = routeRemove.deserialize2( data, length, m_localCommonAddr );
	if( bytesRead > 0 )
	{
		const bdArray<bdEndpoint> removedRoutes = routeRemove.getEndpoints();
		for( bdUInt i = 0; i < removedRoutes.getSize(); ++i )
		{
			//BD_INFO( BD_LOG_LEVEL, "Removing connection to %u",
			//	removedRoutes[i].getCommonAddr()->getLocalAddrByIndex(0).getPort() );
			m_relayRouteManager.removeRelayedRoute( relayerEndpoint, removedRoutes[i] );
		}

		// Rebuild the routes table right away
		//m_relayRouteManager.pump();

		//m_relayRouteManager.print();
	}
	else
	{
		retVal = BD_NET_ERROR;
	}

	return bytesRead;
}

bdInt bdRelaySocketRouter::processRelayInfoRequest(bdAddrHandleRef &addrHandle,
													void *data,
													const bdUInt length )
{
	bdRelayerInfo infoRequest( BD_RELAYER_INFO_REQUEST );
	bdUInt numReplies = 0;

	bdUInt bytesRead = infoRequest.deserialize2( data, length, m_localCommonAddr );
	if(bytesRead > 0 )
	{
		bdRelayerInfo infoReply( BD_RELAYER_INFO_REPLY );

		// Search for the requested latencies and add them to the reply
		bdArray<bdTargetLatency> relayerLatencies = infoRequest.getRelayerLatencies();
		for(bdUInt i = 0; i < relayerLatencies.getSize(); i++)
		{
			bdEndpoint target = relayerLatencies[i].m_target;
			bdUInt16 latency = 0;
			if( m_relayRouteManager.getEndpointLatency( target, target, latency ) )
			{
				//BD_INFO( BD_LOG_LEVEL, "Reply for latency of peer %u is %u", target.getCommonAddr()->getLocalAddrByIndex(0).getPort(), latency);
				infoReply.addLatencyResponse( target, latency);
				numReplies++;
			}
		}

		// Inform the requester of our current load
		infoReply.setRelayerLoad( m_relayerLoad );

		const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
		bdUByte8 buffer[bufferSize];
		bdUInt position = 0;

		// We need to send a reply wether or not there were any latency requests contained in it
		// because the sender uses the reply to compute their latency to us.
		while( true )
		{
			bdUInt packetLen = infoReply.serialize( buffer, bufferSize, 0, position );
			if( packetLen > 0 )
			{
				//BD_INFO(BD_LOG_LEVEL, "Sending info reply to %u", addrHandle->m_endpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort() );
				sendRelayPacket( addrHandle->m_endpoint, buffer, packetLen );

				if( position >= numReplies )
				{
					break;
				}
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Did not serialize infoReply packet");
				break;
			}
		}
	}
	return bytesRead;
}

bdInt bdRelaySocketRouter::processRelayInfoReply(	bdAddrHandleRef &addrHandle,
													void *data,
													const bdUInt length )
{
	bdRelayerInfo infoReply( BD_RELAYER_INFO_REPLY );
	//BD_INFO( BD_LOG_LEVEL, "Processing a relay latency reply" );

	bdUInt bytesRead = infoReply.deserialize2( data, length, m_localCommonAddr );
	if(bytesRead > 0 )
	{
		// Only deal with this reply if it matches a pending request
		if( m_pendingInfoReplies.containsKey( addrHandle->m_endpoint ))
		{
			// Update the relayRouteManager with the new latencies that we requested from the relayer
			bdArray<bdTargetLatency> relayerEndpoints = infoReply.getRelayerLatencies();
			for( bdUInt i = 0; i < relayerEndpoints.getSize(); i++)
			{
				bdEndpoint target = relayerEndpoints[i].m_target;
				bdUInt latency = relayerEndpoints[i].m_latency;
				m_relayRouteManager.updateEndpointLatency( target, addrHandle->m_endpoint, latency );
			}

			// Compute the elapsed time between when we sent this packet and now
			bdUInt64 now = bdPlatformTiming::getHiResTimeStamp();
			bdUInt64 start = 0;
			m_pendingInfoReplies.get( addrHandle->m_endpoint, start );
			BD_ASSERT( now > start, "PlatformTiming error. now is less than start" );

			bdFloat32 elapsed = bdPlatformTiming::getElapsedTime( start, now );
			// Convert to milliseconds
			elapsed *= 1000.f;

			bdUInt16 currLatency = 0;
			bdUInt16 newLatency = 0;
			if( m_relayRouteManager.getEndpointLatency( addrHandle->m_endpoint, addrHandle->m_endpoint, currLatency ) )
			{
				newLatency = (bdUInt16)(currLatency * ( 1 - BD_RELAY_BITRATE_CALC_ALPHA )
									+ BD_RELAY_BITRATE_CALC_ALPHA * elapsed );
			}
			else
			{
				newLatency = (bdUInt16)elapsed;
			}

			//BD_INFO(BD_LOG_LEVEL, "New direct latency to %d.%d.%d.%d is %u (elapsed %.1fms)",
			//	addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b1,
			//	addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b2,
			//	addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b3,
			//	addrHandle->m_endpoint.getCommonAddr()->getXNAddr().ina.S_un.S_un_b.s_b4,
			//	newLatency, elapsed);

			m_relayRouteManager.updateEndpointLatency( addrHandle->m_endpoint, addrHandle->m_endpoint, newLatency);

			// Update the relayer load
			m_relayRouteManager.updateRelayerLoad( addrHandle->m_endpoint, infoReply.getRelayerLoad() );

			// We've received a reply for this update, we can remove it from the pending list
			m_pendingInfoReplies.remove( addrHandle->m_endpoint );
		}
		else
		{
			BD_INFO(BD_LOG_LEVEL, "No pending request corresponds to this reply");
		}
	}

	return bytesRead;
}

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
void bdRelaySocketRouter::broadcastPacket( bdUByte8* buffer, const bdUInt length, const bdEndpoint *dontSend )
{
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		const bdEndpoint &destEndpoint = m_dtls.getKey(it);
		bdDTLSAssociation *conn = m_dtls.getValue(it);

		// Inform everyone about the new route except the new route itself
		if( conn->getStatus() == BD_SOCKET_CONNECTED )
		{
			bdBool send = true;
			if( dontSend != BD_NULL )
			{
				send = !( *dontSend == destEndpoint );
			}

			if( send )
			{
				sendRelayPacket( destEndpoint, buffer, length );
			}
		}
		m_dtls.next(it);
	}
	m_dtls.releaseIterator(it);
}
#endif

void bdRelaySocketRouter::broadcastNewRoute( const bdEndpoint &newEndpoint )
{
	// Build the route add packet with the single newEndpoint
	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 buffer[bufferSize];

	bdRelayRouteAdd routeAdd;
	routeAdd.addEndpoint( newEndpoint );

	bdUInt newPosition;
	bdUInt packetLen = routeAdd.serialize( buffer, bufferSize, 0, newPosition );

	if( packetLen > 0 )
	{
		#if defined BD_BUFFER_ROUTE_ADD
			bufferRouteAddMessages(routeAdd, &newEndpoint);
		#else
			broadcastPacket( buffer, packetLen, &newEndpoint );
		#endif
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Did not serialize  packet");
	}	
}

void bdRelaySocketRouter::broadcastRemovedRoute( const bdEndpoint &deadEndpoint )
{
	// Build the route remove packet
	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 buffer[bufferSize];

	bdRelayRouteRemove routeRemove;
	routeRemove.addEndpoint( deadEndpoint );

	bdUInt newPosition;
	bdUInt packetLen = routeRemove.serialize( buffer, bufferSize, 0, newPosition );

	if( packetLen > 0 )
	{
		#if defined BD_BUFFER_ROUTE_REMOVE
				bufferRouteRemoveMessages(routeRemove, &deadEndpoint);
		#else
				broadcastPacket( buffer, packetLen, &deadEndpoint );
		#endif	
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Error serializing packet");
	}
}

void bdRelaySocketRouter::sendRemovedRoute( const bdEndpoint &recipientEndpoint, const bdEndpoint &deadEndpoint )
{
	// Build the route remove packet
	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 buffer[bufferSize];

	bdRelayRouteRemove routeRemove;
	routeRemove.addEndpoint( deadEndpoint );

	bdUInt newPosition;
	bdUInt packetLen = routeRemove.serialize( buffer, bufferSize, 0, newPosition );

	// Send it off
	if( packetLen > 0 )
	{
		sendRelayPacket( recipientEndpoint, buffer, packetLen );
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Error serializing packet");
	}
}

void bdRelaySocketRouter::sendConnections( const bdEndpoint &newEndpoint, bdBool buffered )
{
	// Build the route add packet. Add all of out direct connections except the one we're sending
	// this to.
	bdRelayRouteAdd routeAdd;
#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		const bdEndpoint &destEndpoint = m_dtls.getKey(it);
		bdDTLSAssociation *conn = m_dtls.getValue(it);

		if( conn->getStatus() == BD_SOCKET_CONNECTED && !( newEndpoint == destEndpoint ) )
		{
			routeAdd.addEndpoint( destEndpoint );
		}
		m_dtls.next( it );
	}
	m_dtls.releaseIterator(it);
#else
	bdArray<bdEndpoint> direct = m_relayRouteManager.getDirectEndpoints();
	for(bdUInt i=0; i< direct.getSize(); i++ )
	{
		if( !( newEndpoint == direct[i] ) && (bdMemcmp(newEndpoint.getSecID().ab, direct[i].getSecID().ab, BD_SECURITY_ID_LENGTH) == 0))
		{
			routeAdd.addEndpoint( direct[i] );
		}
	}
#endif

	if (buffered)
	{
		//Buffer the route add packets for sending on the next call to sendTo
		bdAddrHandleRef addrHandle;
		if (m_addrMap.getAddrHandle( newEndpoint.getCommonAddr(), newEndpoint.getSecID(), addrHandle ))
		{
			bdArray<bdRelayRouteAdd> bufferPktQueue;
			m_routeAddPackets.remove(bdAddrHandleRefWrapper(addrHandle), bufferPktQueue);
			
			//Split the packet so that its no bigger that each individual packet is no bigger
			//than the max payload size. This makes the buffering logic more straightforward.
			bdUInt position = 0;
			bdUInt startPosition = 0;
			bdUInt numEndpoints = routeAdd.getEndpoints().getSize();

			while ( position <  numEndpoints)
			{
				const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE - 2;
				bdUByte8 buffer[bufferSize];

				bdUInt packetLen = routeAdd.serialize( buffer, bufferSize, position, position );

				if( packetLen > 0 )
				{
					//Buffer the packet with the number of routes that will fit
					bdRelayRouteAdd bufferedPacket;
					bdArray< bdEndpoint > endPoints = routeAdd.getEndpoints();
					for( bdUInt i= startPosition ; i< position; i++ )
					{
						bufferedPacket.addEndpoint( endPoints[i] );
					}
					bufferPktQueue.pushBack(bufferedPacket);

					startPosition = position;
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "Error serializing packet");
				}
			}
			m_routeAddPackets.put(bdAddrHandleRefWrapper(addrHandle), bufferPktQueue);
		}
	}
	else
	{
		// Depending on the number of direct connections we have we may need to split this up into
		// several packets. Keep going until the position matches the number of routes we added.
		bdUInt position = 0;
		while ( position < routeAdd.getEndpoints().getSize() )
		{
			const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
			bdUByte8 buffer[bufferSize];

			bdUInt packetLen = routeAdd.serialize( buffer, bufferSize, position, position );

			if( packetLen > 0 )
			{
				sendRelayPacket( newEndpoint, buffer, packetLen );
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Error serializing packet");
			}
		}
	}
}

void bdRelaySocketRouter::bufferRelayInfoMessages(const bdRelayerInfo& packet)
{
#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		const bdEndpoint &destEndpoint = m_dtls.getKey(it);
		bdDTLSAssociation *conn = m_dtls.getValue(it);

		bdAddrHandleRef addrHandle;
		m_addrMap.getAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), addrHandle );

		if( conn->getStatus() == BD_SOCKET_CONNECTED )
		{
			m_infoPackets.put(bdAddrHandleRefWrapper(addrHandle), packet);
		}
		m_dtls.next(it);
	}
	m_dtls.releaseIterator(it);
#else
	bdArray<bdEndpoint> directConns = m_relayRouteManager.getDirectEndpoints();
	for(bdUInt i = 0; i < directConns.getSize(); i++ )
	{
		const bdEndpoint &destEndpoint = directConns[i];
		bdAddrHandleRef addrHandle;
		m_addrMap.getAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), addrHandle );

		if( addrHandle.notNull() )
		{
			//Get the list of buffer packets to this destination
			m_infoPackets.put(bdAddrHandleRefWrapper(addrHandle), packet);
			//BD_INFO(BD_LOG_LEVEL, "Adding relay info with %d values", packet.getRelayerLatencies().getSize());
		}
	}
#endif
}

bdInt bdRelaySocketRouter::writeRelayInfoMessages(bdUByte8*& buffer, bdRelayerInfo& packet, bdUInt size, bdAddrHandleRef addrHandle)
{
	bdUInt bytesWritten = 0;

	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 pktBuffer[bufferSize];

	bdUInt pos = 0;
	bdUInt bytesLeft = size - bytesWritten;
	bdUInt32 packetSize = packet.serialize(pktBuffer, bytesLeft, pos, pos, true);			
	bdUInt numLatencyVals = packet.getRelayerLatencies().getSize();

	//If there's enough space, then write the data to the packet
	if (packetSize>0 && packetSize < bytesLeft)
	{			
		bdMemcpy(buffer, pktBuffer, packetSize);
		bytesWritten += packetSize;
		buffer += packetSize;

		m_pendingInfoReplies.put( addrHandle->m_endpoint, bdPlatformTiming::getHiResTimeStamp() );
		//BD_INFO(BD_LOG_LEVEL, "Sending %d relay info values out of %d", pos, numLatencyVals);
	}

	//If there are latency values left, then create a new packet 
	//with rest of latency info, and add to the list
	if (pos<numLatencyVals)
	{
		bdRelayerInfo infoRequest( BD_RELAYER_INFO_REQUEST );
		bdArray< bdTargetLatency > relayerLatencies = packet.getRelayerLatencies();
		for( bdUInt i= pos ; i< numLatencyVals; i++ )
		{
		   infoRequest.addLatencyRequest( relayerLatencies[i] );
		}
		m_infoPackets.put(bdAddrHandleRefWrapper(addrHandle), infoRequest);
	}
	return bytesWritten;
}


void bdRelaySocketRouter::bufferRouteAddMessages(bdRelayRouteAdd packet, const bdEndpoint *dontSend)
{

	bdArray<bdEndpoint> directConns = m_relayRouteManager.getDirectEndpoints();
	
	for(bdUInt i = 0; i < directConns.getSize(); i++ )
	{
		const bdEndpoint &destEndpoint = directConns[i];
		bdAddrHandleRef addrHandle;
		m_addrMap.getAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), addrHandle );

		if( addrHandle.notNull() )
		{
			bdBool send = true;
			if( dontSend != BD_NULL )
			{
				send = !( *dontSend == destEndpoint );

				if (bdMemcmp(dontSend->getSecID().ab, directConns[i].getSecID().ab, BD_SECURITY_ID_LENGTH) != 0)
				{
					send = false;
				}
			}

			if( send )
			{	
				//Get the list of buffer packets to this destination
				bdArray<bdRelayRouteAdd> bufferPktQueue;
				m_routeAddPackets.remove(bdAddrHandleRefWrapper(addrHandle), bufferPktQueue);
				bufferPktQueue.pushBack(packet);
				m_routeAddPackets.put(bdAddrHandleRefWrapper(addrHandle), bufferPktQueue);
			}
		}
	}
}


bdInt bdRelaySocketRouter::writeRouteAddMessages(bdUByte8*& buffer, bdArray<bdRelayRouteAdd>& packetList, bdUInt size)
{
	bdUInt bytesWritten = 0;
	bdUInt arrayReadStart = 0;
	bdUInt arrayReadPos = 0;
	bdUInt arraySize = packetList.getSize();

	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 pktBuffer[bufferSize];

	while (arrayReadPos < arraySize && bytesWritten < size)
	{
		bdRelayRouteAdd pkt;		
		if (packetList.get(arrayReadPos,pkt))
		{
			bdUInt pos = 0;
			bdUInt32 packetSize = pkt.serialize(pktBuffer, bufferSize, pos, pos, true);

			bdUInt bytesLeft = size - bytesWritten;

			//If there's enough space, then write the data to the packet
			if (packetSize>0 && packetSize < bytesLeft)
			{			
				bdMemcpy(buffer, pktBuffer, packetSize);

				bytesWritten += packetSize;
				buffer += packetSize;
				arrayReadPos++;
			}
			else
			{

				break;
			}
		}
		else
		{
			arrayReadPos++;
		}

	}
	if (bytesWritten > 0)
	{
		packetList.removeSection(arrayReadStart, arrayReadPos);
	}
	return bytesWritten;
}

void bdRelaySocketRouter::bufferRouteRemoveMessages(bdRelayRouteRemove packet, const bdEndpoint *dontSend)
{
	bdArray<bdEndpoint> directConns = m_relayRouteManager.getDirectEndpoints();
	for(bdUInt i = 0; i < directConns.getSize(); i++ )
	{
		const bdEndpoint &destEndpoint = directConns[i];
		bdAddrHandleRef addrHandle;
		m_addrMap.getAddrHandle( destEndpoint.getCommonAddr(), destEndpoint.getSecID(), addrHandle );

		if( addrHandle.notNull() )
		{
			bdBool send = true;
			if( dontSend != BD_NULL )
			{
				send = !( *dontSend == destEndpoint );
			}

			if( send )
			{	
				//Get the list of buffer packets to this destination
				bdArray<bdRelayRouteRemove> bufferPktQueue;
				m_routeRemovePackets.remove(bdAddrHandleRefWrapper(addrHandle), bufferPktQueue);
				bufferPktQueue.pushBack(packet);
				m_routeRemovePackets.put(bdAddrHandleRefWrapper(addrHandle), bufferPktQueue);
			}
		}
	}


}

bdInt bdRelaySocketRouter::writeRouteRemoveMessages(bdUByte8*& buffer, bdArray<bdRelayRouteRemove>& packetList, bdUInt size)
{
	bdUInt bytesWritten = 0;
	bdUInt arrayReadStart = 0;
	bdUInt arrayReadPos = 0;
	bdUInt arraySize = packetList.getSize();

	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 pktBuffer[bufferSize];

	while (arrayReadPos < arraySize && bytesWritten < size)
	{
		bdRelayRouteRemove pkt;		
		if (packetList.get(arrayReadPos,pkt))
		{
			bdUInt pos = 0;
			bdUInt32 packetSize = pkt.serialize(pktBuffer, bufferSize, pos, pos, true);

			bdUInt bytesLeft = size - bytesWritten;

			//If there's enough space, then write the data to the packet
			if (packetSize>0 && packetSize < bytesLeft)
			{			
				bdMemcpy(buffer, pktBuffer, packetSize);

				bytesWritten += packetSize;
				buffer += packetSize;
				arrayReadPos++;
			}
			else
			{
				break;
			}
		}
		else
		{
			arrayReadPos++;
		}

	}
	if (bytesWritten > 0)
	{
		packetList.removeSection(arrayReadStart, arrayReadPos);
	}
	return bytesWritten;
}

bdInt bdRelaySocketRouter::writeRelayData(const bdAddrHandleRef addrHandle, bdInt totalSize, bdUByte8*& buffer)
{
	bdInt totalBytesWritten = 0;
	bdInt bytesWritten = 0;
	
	//Get the list of each type of packet
	bdRelayerInfo routeInfo(BD_RELAYER_INFO_REQUEST);
	bdArray<bdRelayRouteAdd> routeAddList;
	bdArray<bdRelayRouteRemove> routeRemoveList;

	m_routeAddPackets.remove(bdAddrHandleRefWrapper(addrHandle), routeAddList);
	m_routeRemovePackets.remove(bdAddrHandleRefWrapper(addrHandle), routeRemoveList);

	bdBool foundInfo = m_infoPackets.remove(bdAddrHandleRefWrapper(addrHandle), routeInfo);

	//Write route add requests
	if (routeAddList.isEmpty() == false)
	{
		if (totalSize > 0)
		{				
			bytesWritten = writeRouteAddMessages(buffer, routeAddList, totalSize);
			//Add list back to dictionary in case there are any left
			m_routeAddPackets.put(bdAddrHandleRefWrapper(addrHandle), routeAddList);
			totalBytesWritten += bytesWritten;
			totalSize -= bytesWritten;
		}
		else
		{
			// We didn't have space to write this time, so re-add to the list and wait for the next time around
			m_routeAddPackets.put(bdAddrHandleRefWrapper(addrHandle), routeAddList);
		}
	}

	//Write route remove requests
	if (routeRemoveList.isEmpty() == false)
	{
		if (totalSize > 0)
		{			
			bytesWritten = writeRouteRemoveMessages(buffer, routeRemoveList, totalSize);
			//Add list back to dictionary in case there are any left
			m_routeRemovePackets.put(bdAddrHandleRefWrapper(addrHandle), routeRemoveList);
			totalBytesWritten += bytesWritten;
			totalSize -= bytesWritten;
		}
		else
		{
			// We didn't have space to write this time, so re-add to the list and wait for the next time around
			m_routeRemovePackets.put(bdAddrHandleRefWrapper(addrHandle), routeRemoveList);
		}
	}

	//Write route info requests
	if (foundInfo)
	{
		if (totalSize > 0)
		{			
			bytesWritten = writeRelayInfoMessages(buffer, routeInfo, totalSize, addrHandle);
			if (bytesWritten <= 0)
			{
				//Could not fit in current packet, so add to list again
				m_infoPackets.put(bdAddrHandleRefWrapper(addrHandle), routeInfo);
			}
			else
			{
				totalBytesWritten += bytesWritten;
				totalSize -= bytesWritten;
			}
		}
		else
		{
			//Could not fit in current packet, so add to list again
			m_infoPackets.put(bdAddrHandleRefWrapper(addrHandle), routeInfo);
		}
	}

	return totalBytesWritten;

}

void bdRelaySocketRouter::flushRelayMessages(const bdAddrHandleRef addr)
{	
	const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
	bdUByte8 pktBuffer[bufferSize];
	bdUByte8* writePtr = pktBuffer;
	bdUByte8* bufferStart = pktBuffer;
	bdUInt bytesWritten = 0;
	bdInt retVal = 0;
	do 
	{
		//Move forward 2 bytes to accomodate total encrypted size		
		writePtr += sizeof(bdUInt16);

		//size is -2 bytes to accomodate the encrypted data size
		bytesWritten = writeRelayData(addr, bufferSize-2, writePtr);

		if (bytesWritten > 0)
		{
			//Add the total encrypted data size to the start of the message
			bdUInt newOffset = 0;
			bdUInt16 totalEncryptedSize = bytesWritten;
			bdBytePacker::appendBasicType( bufferStart, bufferSize, 0, newOffset, totalEncryptedSize );
			//Increase packet size to accomodate the addition of total encrypted packet size
			bytesWritten += sizeof(bdUInt16);

			#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON 
				retVal = bdSocketRouter::sendTo( addr, pktBuffer, bytesWritten );
			#else				
				retVal = sendRelayPacket( addr->m_endpoint, pktBuffer, bytesWritten );
			#endif
			
		}		

		//Reset pointer to write new data
		writePtr = pktBuffer;
	} while (bytesWritten > 0 && retVal>0);
	 
}

bdBool bdRelaySocketRouter::checkRouteFowardSlotAvailable(bdRelayForward& forwardPacket)
{
	bdBool canForwardPacket = false;
	bdInt numRelayConnections = m_relayConnections.getSize();

	bdEndpoint source = forwardPacket.getSourceEndpoint();
	bdEndpoint destination = forwardPacket.getDestinationEndpoint();
	bdUInt incomingSourceHash = source.getCommonAddr()->getHash();
	bdUInt incomingDestinationHash = destination.getCommonAddr()->getHash();

	if (m_maxRelayedConnections < 0)
	{
		canForwardPacket = true;
	}
	else
	{
		for( bdInt i=0; i<numRelayConnections; i++)
		{
			//If we've forwarded a packet in this direction before
			//or this is a packet being returned, then allow to be forwarded

			bdUInt currentSourceHash =  m_relayConnections[i].getSourceEndpoint().getCommonAddr()->getHash();
			bdUInt currentDestinationHash = m_relayConnections[i].getDestinationEndpoint().getCommonAddr()->getHash();	

			if( currentSourceHash == incomingSourceHash
 				 && currentDestinationHash == incomingDestinationHash )
			{
				canForwardPacket = true;
				break;
			}

		}
	}

	if (!canForwardPacket)
	{
		//Its a brand new route, so check are we allowed to forward 
		if (numRelayConnections < m_maxRelayedConnections)
		{
			canForwardPacket = true;
		}
		else
		{
			sendRemovedRoute(source, destination);
		}	
	}

	return canForwardPacket;
}

void bdRelaySocketRouter::broadcastNewRouteSlot()
{
	bdArray<bdEndpoint> directConnections = m_relayRouteManager.getDirectEndpoints();
	for( bdUInt i = 0; i < directConnections.getSize(); i++ )
	{
		bdAddrHandleRef addrHandle;
		if( m_addrMap.findAddrHandle(	directConnections[i].getCommonAddr(),
											directConnections[i].getSecID(),
											addrHandle ) )
		{
			//Buffer these messages, as they are being broadcast
			//They will be transmitted on the next sendTo
			sendConnections(addrHandle->m_endpoint, true);
		}
	}
}

bdBool bdRelaySocketRouter::isRelaying(const bdEndpoint& endpoint)
{
	bdBool relaying = false;
	bdUInt numRelayConnections = m_relayConnections.getSize();
	for( bdUInt i=0; i<numRelayConnections; i++)
	{
			if( m_relayConnections[i].getSourceEndpoint() == endpoint)
			{
				relaying = true;
				break;
			}
	}
	return relaying;
}

void bdRelaySocketRouter::setMaxRelayedConnections(bdInt numConnections)
{
	m_maxRelayedConnections = numConnections;

	if (m_maxRelayedConnections % 2 != 0)
	{
		BD_WARN("warn","Max num relayed connections should be configured to allow for bi-directional connections"); 
	}
}

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
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

	// We are broadcasting new info request packets. Find all the requests that did not receive a responese
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

		m_pendingInfoReplies.next(pendingIt);
	}
	m_pendingInfoReplies.releaseIterator(pendingIt);
	m_pendingInfoReplies.clear();

	// Send off the requests
#if defined BD_BUFFER_INFO_REQUESTS
		bufferRelayInfoMessages(infoRequest);
#else
	// Send off the requests
	bdHashMap<bdEndpoint, bdDTLSAssociation *, bdEndpointHashingClass>::Iterator it = m_dtls.getIterator();
	while(it)
	{
		const bdEndpoint &destEndpoint = m_dtls.getKey(it);
		bdDTLSAssociation *conn = m_dtls.getValue(it);

		if( conn->getStatus() == BD_SOCKET_CONNECTED )
		{
			bdUInt position = 0;
			while ( true )
			{
				const bdUInt bufferSize = BD_MAX_SOCKET_ROUTER_PAYLOAD_SIZE;
				bdUByte8 buffer[bufferSize];

				bdUInt packetLen = infoRequest.serialize( buffer, bufferSize, position, position );

				if( packetLen > 0 )
				{
					//BD_INFO(BD_LOG_LEVEL, "Sending infoRequest to %u", destEndpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort() );
					sendRelayPacket( destEndpoint, buffer, packetLen );

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
		m_dtls.next(it);
	}
	m_dtls.releaseIterator(it);
#endif
}
#endif
