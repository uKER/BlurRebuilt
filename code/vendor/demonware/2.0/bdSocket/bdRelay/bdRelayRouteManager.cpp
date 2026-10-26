// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdRelay/bdRelayRouteManager.h>

#define BD_LOG_LEVEL "bdSocket/relay"

bdRelayRouteManager::bdRelayRouteManager()
{
}

bdRelayRouteManager::~bdRelayRouteManager()
{
}

void bdRelayRouteManager::quit()
{
	m_connections.clear();
	m_routes.clear();
	m_relayerLoads.clear();
}

void bdRelayRouteManager::pump( )
{
	// Rebuild m_routes from m_connections
	bdHashMap<bdEndpoint, bdArray<bdTargetLatency>, bdEndpointHashingClass >::Iterator connectionIter = m_connections.getIterator();
	while( connectionIter )
	{
		const bdEndpoint &relayerEndpoint = m_connections.getKey(connectionIter);
		bdArray<bdTargetLatency> relayedConnections = m_connections.getValue(connectionIter);

		// If they are in our connections map we have a direct connection to them.
		// Make sure that we are using this.
		if( m_routes.containsKey( relayerEndpoint ) )
		{
			bdEndpoint curRelayer;
			m_routes.get( relayerEndpoint, curRelayer );

			if( !( curRelayer == relayerEndpoint) )
			{
				m_routes.remove( relayerEndpoint );
				m_routes.put( relayerEndpoint, relayerEndpoint );
			}
		}


		for( bdUInt i = 0; i < relayedConnections.getSize(); ++i )
		{
			bdEndpoint curRelayer;
	
			if( !(m_routes.get( relayedConnections[i].m_target, curRelayer )) && !(curRelayer == relayedConnections[i].m_target) )
			{
				// No relayer exists for this target. Use this one if it meets the criteria.
				bdUInt newRelayerLoad = 0;
				bdBool newRelayerLoadExists = getRelayerLoad( relayerEndpoint, newRelayerLoad );
				
				if( newRelayerLoadExists)
				{
					//Only use this relayer if it is within the same session as the destination end point
					bdBool sameSession = false;
					if (bdMemcmp(relayerEndpoint.getSecID().ab, relayedConnections[i].m_target.getSecID().ab, BD_SECURITY_ID_LENGTH) == 0)
					{
						sameSession = true;
					}

					if (sameSession)
					{
						bdEndpoint relayerInOtherSession;
						//If we already have a route to this endpoint, but in another session (e.g. a party)
						//then make sure we use the existing relayer in the new session
						if (getRelayerFromAnotherSession(relayerInOtherSession, relayedConnections[i].m_target))
						{
							
							bdUInt newRelayerHash = relayerEndpoint.getCommonAddr()->getHash();	
							bdUInt existingRelayerHash = relayerInOtherSession.getCommonAddr()->getHash();

							if (newRelayerHash == existingRelayerHash)
							{
								m_routes.put( relayedConnections[i].m_target, relayerEndpoint );							
							}
						}
						else 
						{			
							m_routes.put( relayedConnections[i].m_target, relayerEndpoint );			
						}
					}
				}
			}

		}
		m_connections.next( connectionIter );
	}
	m_connections.releaseIterator( connectionIter );
}

bdBool bdRelayRouteManager::getRelayerFromAnotherSession(bdEndpoint &relayer,  const bdEndpoint &destination  )
{
	bdUInt destinationHash = destination.getCommonAddr()->getHash();
	bdBool foundRouteFromOtherSession = false;

	
	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iter = m_routes.getIterator();
	while( iter )
	{
		//Check if we have a route to this destination, but in a different session inside of m_incomingRoutes
		const bdEndpoint &targetEndpoint = m_incomingRoutes.getKey(iter);
		const bdEndpoint &relayerEndpoint = m_incomingRoutes.getValue(iter);

		bdUInt targetEndpointHash = targetEndpoint.getCommonAddr()->getHash();

		if (destinationHash == targetEndpointHash)
		{
			if (bdMemcmp(destination.getSecID().ab, targetEndpoint.getSecID().ab, BD_SECURITY_ID_LENGTH) != 0)
			{			
				relayer = relayerEndpoint;
				foundRouteFromOtherSession = true;
				break;
			}
		}
		m_routes.next( iter );
	}
	m_routes.releaseIterator(iter);
	
	
	if (!foundRouteFromOtherSession)
	{
		//Check if we have a route to this destination, but in a different session inside of m_routes
		iter = m_routes.getIterator();
		while( iter )
		{
			const bdEndpoint &targetEndpoint = m_routes.getKey(iter);
			const bdEndpoint &relayerEndpoint = m_routes.getValue(iter);

			bdUInt targetEndpointHash = targetEndpoint.getCommonAddr()->getHash();

			if (destinationHash == targetEndpointHash)
			{
				if (bdMemcmp(destination.getSecID().ab, targetEndpoint.getSecID().ab, BD_SECURITY_ID_LENGTH) != 0)
				{				
					relayer = relayerEndpoint;
					foundRouteFromOtherSession = true;
					break;
				}
			}
			m_routes.next( iter );
		}
		m_routes.releaseIterator(iter);
	}
	return foundRouteFromOtherSession;
	
}

bdBool bdRelayRouteManager::addDirectRoute( const bdEndpoint &destinationEndpoint )
{
	bdBool retVal = false;

	bdEndpoint relayerEndpoint;
	// Add it if it doesn't exist at all
	if( !m_routes.get( destinationEndpoint, relayerEndpoint ) )
	{
		//BD_WARN("warn","N 1");
		retVal = m_routes.put( destinationEndpoint, destinationEndpoint );
	}
	// If it exists as a relayed connection, replace the relayed connection
	else if( !( relayerEndpoint == destinationEndpoint ) )
	{
		//BD_WARN("warn","N 2");		
		m_routes.remove( destinationEndpoint );

		retVal = m_routes.put( destinationEndpoint, destinationEndpoint );
	}

	//If it exists in m_incomingRoutes, then remove it
	if (m_incomingRoutes.get(destinationEndpoint, relayerEndpoint))
	{
		m_incomingRoutes.remove( destinationEndpoint );
	}
	
	// Check whether this direct route exists in the connections map
	bdArray<bdTargetLatency> relayerLatencies;
	bdBool found = false;
	if( m_connections.get( destinationEndpoint, relayerLatencies ) )
	{
		for( bdUInt i=0;i < relayerLatencies.getSize(); i++ )
		{
			if( relayerLatencies[i].m_target == destinationEndpoint )
			{
				found = true;
				break;
			}
		}
	}

	if( !found )
	{
		// If it doesn't exist add it.
		bdTargetLatency newTarget( destinationEndpoint, BD_RELAY_DEFAULT_LATENCY );
		relayerLatencies.pushBack( newTarget );

		m_connections.remove( destinationEndpoint );
		retVal = retVal && m_connections.put( destinationEndpoint, relayerLatencies );
	}

	return retVal;
}

bdBool bdRelayRouteManager::addRelayedRoute( const bdEndpoint &relayerEndpoint, const bdEndpoint &destinationEndpoint )
{
	bdBool retVal = true;

	bdArray<bdTargetLatency> connections;
	if( m_connections.containsKey( relayerEndpoint ) )
	{
		m_connections.get( relayerEndpoint, connections );
	}
	
	bdBool found = false;
	for( bdUInt i=0; i < connections.getSize(); i++ )
	{
		if( connections[i].m_target == destinationEndpoint )
		{
			found = true;
			break;
		}
	}

	// Add the route if it doesn't exist already
	if( !found )
	{
		connections.pushBack( bdTargetLatency( destinationEndpoint ) );
	}	
	m_connections.remove( relayerEndpoint );
	m_connections.put( relayerEndpoint, connections );

	return retVal;
}

bdBool bdRelayRouteManager::removeDirectRoute( const bdEndpoint &destinationEndpoint )
{
	bdBool retVal = true;
	bdBool removedDirectRoute = false;
	// Remove from loads and from connections
	retVal = retVal && m_relayerLoads.remove( destinationEndpoint );
	retVal = retVal && m_connections.remove( destinationEndpoint );

	//When attempts to directlly conenct to a relayed connection times out, this method is called
	//Make sure its direct before we try remove any reference to this connection (see below)
	bdEndpoint currRelayer;
	if (m_routes.get(destinationEndpoint, currRelayer))
	{
		if (currRelayer == destinationEndpoint)
		{
			removedDirectRoute = true;
		}
	}

	// Remove all routes where he is the relayer or where he is being relayed to. 
	// The pump call afterwards will assign a new relayer if possible.
	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iter = m_routes.getIterator();
	while( iter )
	{
		const bdEndpoint &targetEndpoint = m_routes.getKey(iter);
		const bdEndpoint &curRelayerEndpoint = m_routes.getValue(iter);

		if( curRelayerEndpoint == destinationEndpoint || targetEndpoint == destinationEndpoint )
		{
			m_routes.remove( iter );
		}
		else
		{
			m_routes.next( iter );
		}
	}	
	m_routes.releaseIterator(iter);

	if (removedDirectRoute)
	{
		//Now remove any reference to this connection for other peers. We do this so we won't attempt to relay to this peer
		//if the game code attempts to send a game packet to the disconnecting peer
		bdHashMap<bdEndpoint, bdArray<bdTargetLatency>, bdEndpointHashingClass > tempConnections;
		bdHashMap<bdEndpoint, bdArray<bdTargetLatency>, bdEndpointHashingClass >::Iterator connectionIter = m_connections.getIterator();
		while( connectionIter != BD_NULL)
		{
			const bdEndpoint &relayerEndpoint = m_connections.getKey(connectionIter);
			bdArray<bdTargetLatency> relayedConnections = m_connections.getValue(connectionIter);

			//Check if this peer has a direct connection to the peer we are removing
			//If so, remove that reference
			for( bdUInt i = 0; i < relayedConnections.getSize(); ++i )
			{
				if(destinationEndpoint == relayedConnections[i].m_target )	
				{					
					relayedConnections.removeSection(i,i+1);
					break;
				}
			}			
		
			//Add the other connections back to the list
			tempConnections.put( relayerEndpoint, relayedConnections );

			m_connections.next( connectionIter );
		}
		m_connections.releaseIterator(connectionIter);

		//Now copy new hash map back into m_connections
		m_connections.clear();
		m_connections.putAll(tempConnections);
	
	}
	// Rebuild the routes immediately
	//pump();
	return retVal;
}

bdBool bdRelayRouteManager::removeRelayedRoute( const bdEndpoint &relayerEndpoint, const bdEndpoint &destinationEndpoint )
{
	bdBool retVal = true;

	// Remove this endpoint from the list of endpoints the relayer has available
	if( m_connections.containsKey( relayerEndpoint ) )
	{
		bdArray<bdTargetLatency> oldRelayedConnections;
		bdArray<bdTargetLatency> newRelayedConnections;

		retVal = retVal && m_connections.get( relayerEndpoint, oldRelayedConnections );
		for( bdUInt i = 0; i < oldRelayedConnections.getSize(); i++ )
		{
			if( !( oldRelayedConnections[i].m_target == destinationEndpoint ) )
			{
				newRelayedConnections.pushBack( oldRelayedConnections[i] );
			}
		}

		// Replace the relayedConnections for this relayer
		retVal = retVal && m_connections.remove( relayerEndpoint );
		retVal = retVal && m_connections.put( relayerEndpoint, newRelayedConnections );
	}

	// Remove all references to this destination from the routes map. If a connection
	// can still be made to this endpoint it will be set on the pump
	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iter = m_routes.getIterator();
	while( iter )
	{
		const bdEndpoint &targetEndpoint = m_routes.getKey(iter);
		const bdEndpoint &curRelayerEndpoint = m_routes.getValue(iter);
		
		if( targetEndpoint == destinationEndpoint || curRelayerEndpoint == destinationEndpoint)
		{
			retVal = retVal && m_routes.remove( iter );
		}
		else
		{
			m_routes.next( iter );
		}
	}
	m_routes.releaseIterator(iter);

	return retVal;
}

bdBool bdRelayRouteManager::getRelayer(const bdEndpoint &targetEndpoint, bdEndpoint &relayerEndpoint) const
{
	bdBool val = m_routes.get( targetEndpoint, relayerEndpoint );

	return val;
}

void bdRelayRouteManager::updateRelayerLoad( const bdEndpoint &relayerEndpoint, bdUInt newLoad )
{
	if( m_relayerLoads.containsKey( relayerEndpoint ) )
	{
		m_relayerLoads.remove( relayerEndpoint );
	}
	m_relayerLoads.put( relayerEndpoint, newLoad );
}


bdBool bdRelayRouteManager::findRoute(const bdEndpoint &targetEndpoint,bdEndpoint &relayerEndpoint ) 
{
	//First, check if a route exists from a previously made incoming route. If so, return immediately and use the relayer
	if (m_incomingRoutes.get(targetEndpoint, relayerEndpoint))
	{
		return true;
	}
	
	//Next, check if a route exists as an outgoing route. If so, return immediately
	if (m_routes.get( targetEndpoint, relayerEndpoint))
	{
		return true;
	}
	
	//No route exists yet, co call pump() to build the route
	pump();

	//Now determine if a route exists
	return m_routes.get( targetEndpoint, relayerEndpoint);	
}

bdBool bdRelayRouteManager::getRelayerLoad( const bdEndpoint &relayerEndpoint, bdUInt &load )
{
	return m_relayerLoads.get( relayerEndpoint, load );
}

void bdRelayRouteManager::updateEndpointLatency( const bdEndpoint &targetEndpoint, const bdEndpoint &relayerEndpoint, bdUInt16 newLatency  )
{
	// Only update the latency if the direct connection exists
	if( m_connections.containsKey( relayerEndpoint ) )
	{
		bdArray<bdTargetLatency> oldRelayerLatencies;
		bdArray<bdTargetLatency> newRelayerLatencies;
		m_connections.get(relayerEndpoint, oldRelayerLatencies);
		// Pre-size the new array to avoid re-allocating, copying and other slowdown
		newRelayerLatencies.ensureCapacity(oldRelayerLatencies.getSize());
		for( bdUInt i = 0; i < oldRelayerLatencies.getSize(); i++ )
		{
			if( !(oldRelayerLatencies[i].m_target == targetEndpoint) )
			{
				newRelayerLatencies.pushBack( oldRelayerLatencies[i] );
			}
		}
		m_connections.remove( relayerEndpoint );
		bdTargetLatency newTargetLatency( targetEndpoint, newLatency);
		newRelayerLatencies.pushBack( newTargetLatency );

		m_connections.put( relayerEndpoint, newRelayerLatencies );
	}
}

bdBool bdRelayRouteManager::getEndpointLatency( const bdEndpoint &targetEndpoint, const bdEndpoint &relayerEndpoint, bdUInt16 &totalLatency )
{
	bdBool ok = false;

	bdArray<bdTargetLatency> relayerLatencies;
	if( m_connections.containsKey( relayerEndpoint ) )
	{
		m_connections.get( relayerEndpoint, relayerLatencies );

		for( bdUInt i = 0; i < relayerLatencies.getSize(); i++ )
		{
			if( relayerLatencies[i].m_target == targetEndpoint )
			{
				if( relayerEndpoint == targetEndpoint && relayerLatencies[i].m_isLatencyValid )
				{
					// This is a direct connection. The total is just the latency to the relayer
					ok = true;
					totalLatency = relayerLatencies[i].m_latency;
				}
				else if( relayerLatencies[i].m_isLatencyValid )
				{
					// This is a request for a relayed latency. Get the direct and add it to the relayed
					bdUInt16 directLatency = 0;
					if( getEndpointLatency( relayerEndpoint, relayerEndpoint, directLatency ) )
					{
						ok = true;
						totalLatency = relayerLatencies[i].m_latency + directLatency;
					}
					else
					{
						BD_INFO(BD_LOG_LEVEL, "Failed to get direct latency");
					}
				}
			}
		}
	}

	return ok;
}

bdArray <bdEndpoint> bdRelayRouteManager::getRelayedEndpoints()
{
	bdArray <bdEndpoint> relayedEndpoints;
	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass >::Iterator routesIter = m_routes.getIterator();

	while( routesIter )
	{
		const bdEndpoint &destEndpoint = m_routes.getKey(routesIter);
		const bdEndpoint &relayerEndpoint = m_routes.getValue(routesIter);

		// If this is not a direct connection it is a relayed connection.
		if( !(destEndpoint == relayerEndpoint) )
		{
			relayedEndpoints.pushBack( destEndpoint );
		}

		m_routes.next( routesIter );
	}
	m_routes.releaseIterator(routesIter);

	return relayedEndpoints;
}

bdArray <bdEndpoint> bdRelayRouteManager::getDirectEndpoints()
{
	bdArray <bdEndpoint> directEndpoints;
	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass >::Iterator routesIter = m_routes.getIterator();

	while( routesIter )
	{
		const bdEndpoint &destEndpoint = m_routes.getKey(routesIter);
		const bdEndpoint &relayerEndpoint = m_routes.getValue(routesIter);

		// If this is not a direce connection it is a relayed connection.
		if( destEndpoint == relayerEndpoint )
		{
			directEndpoints.pushBack( destEndpoint );
		}

		m_routes.next( routesIter );
	}
	m_routes.releaseIterator(routesIter);

	return directEndpoints;
}

bdUInt bdRelayRouteManager::serializeRoutes( void * buffer, const bdUInt bufferSize ) const
{
	bdUInt offset = 0;
	bdBool status = true;

	if( m_routes.getSize() == 0 )
	{
		status = false;
	}

	// Serialize the number of peers we have a connection with, direct or indirect
	if( status )
	{
		status = status && bdBytePacker::appendBasicType( buffer, bufferSize, offset, offset, m_routes.getSize() ); 
	}

	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iter = m_routes.getIterator();
	while( iter && status )
	{
		const bdEndpoint &destEndpoint = m_routes.getKey(iter);
		const bdEndpoint &relayerEndpoint = m_routes.getValue(iter);
		
		destEndpoint.getCommonAddr()->serialize( reinterpret_cast<bdUByte8 *>( buffer ) + offset );
		offset = offset + BD_COMMON_ADDR_SERIALIZED_SIZE;

		relayerEndpoint.getCommonAddr()->serialize( reinterpret_cast<bdUByte8 *>( buffer ) + offset );
		offset = offset + BD_COMMON_ADDR_SERIALIZED_SIZE;

		m_routes.next( iter );
	}	
	m_routes.releaseIterator(iter);
/*
	bdHashMap<bdEndpoint, bdArray<bdEndpoint>,bdEndpointHashingClass >::Iterator connectionIter = m_connections.getIterator();
	while( connectionIter && status )
	{
		const bdEndpoint &relayerEndpoint = m_connections.getKey(connectionIter);
		bdArray<bdEndpoint> relayedConnections = m_connections.getValue(connectionIter);
		for( bdUInt i = 0; i < relayedConnections.getSize(); ++i )
		{
			status = status && relayedConnections[i].serialize( buffer, bufferSize, offset, offset );
			status = status && relayerEndpoint.serialize( buffer, bufferSize, offset, offset );
		}
		m_connections.next( connectionIter );
	}
	m_connections.releaseIterator(connectionIter);
*/
	if( !status )
	{
		offset = 0;
	}

	return offset;	
}

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON 
void bdRelayRouteManager::print()
{
	if ( m_routes.getSize() == 0 ) return;

	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iter = m_routes.getIterator();
	BD_INFO( BD_LOG_LEVEL, "Routes map. %u peers", m_routes.getSize() );
	BD_INFO( BD_LOG_LEVEL, "--------------------------------------" );
	while( iter )
	{
		const bdEndpoint &destEndpoint = m_routes.getKey(iter);
		const bdEndpoint &relayerEndpoint = m_routes.getValue(iter);

		//bdNChar8 destIP[15];
		//destEndpoint.getCommonAddr()->getPublicAddr().toString(destIP, 15);
		//bdPort destPort = destEndpoint.getCommonAddr()->getPublicAddr().getPort();
		bdPort destPort = destEndpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort();

		bdUInt16 destLat = 999;
		getEndpointLatency( destEndpoint, relayerEndpoint, destLat );
		bdUInt destLoad = 999;
		getRelayerLoad( relayerEndpoint, destLoad );

		//bdNChar8 relayerIP[15];
		//relayerEndpoint.getCommonAddr()->getPublicAddr().toString(relayerIP, 15);
		//bdPort relayerPort = relayerEndpoint.getCommonAddr()->getPublicAddr().getPort();
		bdPort relayerPort = relayerEndpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort();

		if( relayerEndpoint == destEndpoint )
		{
			BD_INFO( BD_LOG_LEVEL, "Direct: %u (%u %u)", destPort, destLat, destLoad );
		}
		else
		{
			BD_INFO( BD_LOG_LEVEL, "Relay: %u == %u (%u NA)", relayerPort, destPort, destLat );
		}

		m_routes.next( iter );
	}	
	m_routes.releaseIterator(iter);
	
	BD_INFO( BD_LOG_LEVEL, "" );
	BD_INFO( BD_LOG_LEVEL, "All available connections" );
	BD_INFO( BD_LOG_LEVEL, "--------------------------------------" );
	bdHashMap<bdEndpoint, bdArray<bdEndpoint>, bdEndpointHashingClass >::Iterator connectionIter = m_connections.getIterator();
	while( connectionIter )
	{
		const bdEndpoint &relayerEndpoint = m_connections.getKey(connectionIter);
		bdArray<bdTargetLatency> relayedConnections = m_connections.getValue(connectionIter);

		//bdNChar8 relayerIP[15];
		//relayerEndpoint.getCommonAddr()->getPublicAddr().toString(relayerIP, 15);
		bdPort relayerPort = relayerEndpoint.getCommonAddr()->getLocalAddrByIndex(0).getPort();

		
		for( bdUInt i = 0; i < relayedConnections.getSize(); ++i )
		{
			//bdNChar8 relayedIP[15];
			//relayedConnections[i].getCommonAddr()->getPublicAddr().toString(relayedIP, 15);
			BD_INFO( BD_LOG_LEVEL, "%u -- %u (%u)", relayerPort, 
				relayedConnections[i].m_target.getCommonAddr()->getLocalAddrByIndex(0).getPort(), 
				relayedConnections[i].m_latency );
		}

		m_connections.next( connectionIter );
	}
	m_connections.releaseIterator(connectionIter);
	BD_INFO( BD_LOG_LEVEL, "" );
}
#else
void bdRelayRouteManager::print()
{
	if ( m_routes.getSize() == 0 ) return;

	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iter = m_routes.getIterator();
	BD_INFO( BD_LOG_LEVEL, "Routes map. %u peers", m_routes.getSize() );
	BD_INFO( BD_LOG_LEVEL, "--------------------------------------" );
	while( iter )
	{
		const bdEndpoint &destEndpoint = m_routes.getKey(iter);
		const bdEndpoint &relayerEndpoint = m_routes.getValue(iter);

		bdNChar8 destIP[16];
		XNADDR dest = destEndpoint.getCommonAddr()->getXNAddr();
		sprintf( destIP, "%u.%u.%u.%u", dest.ina.S_un.S_un_b.s_b1, 
									dest.ina.S_un.S_un_b.s_b2,
									dest.ina.S_un.S_un_b.s_b3,
									dest.ina.S_un.S_un_b.s_b4 );
		//bdPort destPort = destEndpoint.getCommonAddr()->getPublicAddr().getPort();

		bdUInt16 destLat = 999;
		getEndpointLatency( destEndpoint, relayerEndpoint, destLat );
		bdUInt destLoad = 999;
		getRelayerLoad( relayerEndpoint, destLoad );

		bdNChar8 relayerIP[15];
		XNADDR relayer = relayerEndpoint.getCommonAddr()->getXNAddr();
		sprintf( relayerIP, "%u.%u.%u.%u", relayer.ina.S_un.S_un_b.s_b1, 
									relayer.ina.S_un.S_un_b.s_b2,
									relayer.ina.S_un.S_un_b.s_b3,
									relayer.ina.S_un.S_un_b.s_b4 );		
		//bdPort relayerPort = relayerEndpoint.getCommonAddr()->getPublicAddr().getPort();
		//bdUInt relayer = relayerEndpoint.getCommonAddr()->getHash();

		if( relayerEndpoint == destEndpoint )
		{
			BD_INFO( BD_LOG_LEVEL, "Direct: %s (%u %u)", destIP, destLat, destLoad );
		}
		else
		{
			BD_INFO( BD_LOG_LEVEL, "Relay: %s == %s (%u NA)", relayerIP, destIP, destLat );
		}

		m_routes.next( iter );
	}	
	m_routes.releaseIterator(iter);

	bdHashMap<bdEndpoint, bdEndpoint, bdEndpointHashingClass>::Iterator iterIncoming = m_incomingRoutes.getIterator();
	BD_INFO( BD_LOG_LEVEL, "Incoming routes map. %u peers", m_incomingRoutes.getSize() );
	BD_INFO( BD_LOG_LEVEL, "--------------------------------------" );
	while( iterIncoming )
	{
		const bdEndpoint &destEndpoint = m_incomingRoutes.getKey(iterIncoming);
		const bdEndpoint &relayerEndpoint = m_incomingRoutes.getValue(iterIncoming);

		bdNChar8 destIP[16];
		XNADDR dest = destEndpoint.getCommonAddr()->getXNAddr();
		sprintf( destIP, "%u.%u.%u.%u", dest.ina.S_un.S_un_b.s_b1, 
									dest.ina.S_un.S_un_b.s_b2,
									dest.ina.S_un.S_un_b.s_b3,
									dest.ina.S_un.S_un_b.s_b4 );
		//bdPort destPort = destEndpoint.getCommonAddr()->getPublicAddr().getPort();

		bdUInt16 destLat = 999;
		getEndpointLatency( destEndpoint, relayerEndpoint, destLat );
		bdUInt destLoad = 999;
		getRelayerLoad( relayerEndpoint, destLoad );

		bdNChar8 relayerIP[15];
		XNADDR relayer = relayerEndpoint.getCommonAddr()->getXNAddr();
		sprintf( relayerIP, "%u.%u.%u.%u", relayer.ina.S_un.S_un_b.s_b1, 
									relayer.ina.S_un.S_un_b.s_b2,
									relayer.ina.S_un.S_un_b.s_b3,
									relayer.ina.S_un.S_un_b.s_b4 );		
		//bdPort relayerPort = relayerEndpoint.getCommonAddr()->getPublicAddr().getPort();
		//bdUInt relayer = relayerEndpoint.getCommonAddr()->getHash();

		if( relayerEndpoint == destEndpoint )
		{
			BD_INFO( BD_LOG_LEVEL, "Direct: %s (%u %u)", destIP, destLat, destLoad );
		}
		else
		{
			BD_INFO( BD_LOG_LEVEL, "Relay: %s == %s (%u NA)", relayerIP, destIP, destLat );
		}

		m_incomingRoutes.next( iterIncoming );
	}	
	m_incomingRoutes.releaseIterator(iterIncoming);

	BD_INFO( BD_LOG_LEVEL, "" );
	BD_INFO( BD_LOG_LEVEL, "All available connections" );
	BD_INFO( BD_LOG_LEVEL, "--------------------------------------" );
	bdHashMap<bdEndpoint, bdArray<bdEndpoint>, bdEndpointHashingClass >::Iterator connectionIter = m_connections.getIterator();
	while( connectionIter )
	{
		const bdEndpoint &relayerEndpoint = m_connections.getKey(connectionIter);
		bdArray<bdTargetLatency> relayedConnections = m_connections.getValue(connectionIter);

		bdNChar8 relayerIP[15];
		XNADDR relayer = relayerEndpoint.getCommonAddr()->getXNAddr();
		sprintf( relayerIP, "%u.%u.%u.%u", relayer.ina.S_un.S_un_b.s_b1, 
									relayer.ina.S_un.S_un_b.s_b2,
									relayer.ina.S_un.S_un_b.s_b3,
									relayer.ina.S_un.S_un_b.s_b4 );	
		//relayerEndpoint.getCommonAddr()->getPublicAddr().toString(relayerIP, 15);
		//bdUInt relayer = relayerEndpoint.getCommonAddr()->getHash();

		
		for( bdUInt i = 0; i < relayedConnections.getSize(); ++i )
		{
			bdNChar8 destIP[16];
			XNADDR dest = relayedConnections[i].m_target.getCommonAddr()->getXNAddr();
			sprintf( destIP, "%u.%u.%u.%u", dest.ina.S_un.S_un_b.s_b1, 
										dest.ina.S_un.S_un_b.s_b2,
										dest.ina.S_un.S_un_b.s_b3,
										dest.ina.S_un.S_un_b.s_b4 );

			BD_INFO( BD_LOG_LEVEL, "%s -- %s (%u)", relayerIP, destIP, 
				relayedConnections[i].m_latency );
		}

		m_connections.next( connectionIter );
	}
	m_connections.releaseIterator( connectionIter );

	BD_INFO( BD_LOG_LEVEL, "" );
}
#endif

void bdRelayRouteManager::addReturnRoute(const bdEndpoint &relayerEndpoint, const bdEndpoint &destinationEndpoint, const bdCommonAddrRef localCommonAddr)
{
	bdEndpoint currRelayer;
	bdBool isAuthorative = isAuthorativeOverRoute(localCommonAddr, destinationEndpoint.getCommonAddr());
	if (!m_routes.get( destinationEndpoint, currRelayer ) && !m_incomingRoutes.get( destinationEndpoint, currRelayer ) )
	{
		//This is a brand new incoming connection. Determine who has authority over the route
		//and add to the appropriate data structure.
		if (isAuthorative)
		{
			m_routes.put( destinationEndpoint, relayerEndpoint );
		}
		else
		{
			m_incomingRoutes.put( destinationEndpoint, relayerEndpoint );
		}
	}
	else if (!isAuthorative)
	{
		//If the route has changed then switch routes to the new route
		if (!m_incomingRoutes.get( destinationEndpoint, currRelayer ))
		{
			//No route yet, so use this one
			m_incomingRoutes.put( destinationEndpoint, relayerEndpoint );
		}
		else
		{
			if (!(currRelayer == relayerEndpoint))
			{	
				//Route has changed, so switch to the new route
				m_incomingRoutes.remove( destinationEndpoint );
				m_incomingRoutes.put( destinationEndpoint, relayerEndpoint );
			}
		}
	}
}

bdBool bdRelayRouteManager::isAuthorativeOverRoute(const bdCommonAddrRef localCommonAddr, const bdCommonAddrRef remoteCommonAddr)
{
	bdBool isAuthorative = false;
	bdUInt localHash = localCommonAddr->getHash();
	bdUInt incomingHash = remoteCommonAddr->getHash();

	if (incomingHash > localHash)
	{
		isAuthorative = true;
	}
	return isAuthorative;
}
