// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdRelay/bdPacket/bdRelayerInfo.h>

#define BD_LOG_LEVEL "bdSocket/relay"

bdRelayerInfo::bdRelayerInfo( bdUByte8 type )
	: bdRelayHeader( type )
{
	BD_ASSERT( (m_type == BD_RELAYER_INFO_REQUEST || type == BD_RELAYER_INFO_REPLY ), "Type not valid" );
}

bdUInt bdRelayerInfo::serialize( void *data,
									const bdUInt size,
									const bdUInt position,
									bdUInt &newPosition,
									bdBool buffered)
{
	bdBool status = true;
	bdUInt newOffset = BD_DTLS_PACKET_FIELD_SIZE;
	newPosition = position;

	if (buffered == false)
	{
		//Not buffering this packet. So, need to add on an extra 2 bytes for total encrypted size
		newOffset += BD_DTLS_PACKET_FIELD_SIZE;
	}

	// Add the header
	status = status && serializeHeader( data, size, newOffset, newOffset );

	// Add the load
	status = status && bdBytePacker::appendBasicType( data, size, newOffset, newOffset, m_relayerLoad );

	// Figure out how many targets we are going to serialize into this packet
	bdUInt numConnectionsLeft = m_relayerLatencies.getSize() - position;
	bdUInt maxConnections = (size - newOffset - sizeof(bdUInt16))/ BD_TARGET_LATENCY_SERIALIZED_SIZE;
	bdUInt numConnectionsToSerialize = (numConnectionsLeft > maxConnections ) ? maxConnections : numConnectionsLeft ;

	// The number of targets in this packet
	status = status && bdBytePacker::appendBasicType( data,
													size,
													newOffset,
													newOffset,
													static_cast<bdUInt16>( numConnectionsToSerialize ) );

	// The actual targets
	bdUInt i;
	for( i = position; status && i < (position + numConnectionsToSerialize); ++i )
	{
		status = status && m_relayerLatencies[i].m_target.serialize( data, size, newOffset, newOffset );
		status = status && bdBytePacker::appendBasicType( data,	size, newOffset, newOffset, m_relayerLatencies[i].m_latency );
	}
	newPosition = i;

	bdUInt encDataSizeOffset = 0;
	if (buffered == false)
	{
		//Not buffering this packet. So, write the total encrypted size to the packet
		const bdUInt16 totalEncDataSize = static_cast<bdUInt16>(newOffset - BD_DTLS_PACKET_FIELD_SIZE);
		bdUInt tempOffset;
		status = status && bdBytePacker::appendBasicType( data, size, 0, tempOffset, totalEncDataSize);
		//Compensate for the total encrypted size info at the start of the packet 
		newOffset -= BD_DTLS_PACKET_FIELD_SIZE;
		encDataSizeOffset = 2;
	}

	const bdUInt16 encDataSize16 = static_cast<bdUInt16>(newOffset - BD_DTLS_PACKET_FIELD_SIZE);
	bdUInt tempOffset;
	status = status && bdBytePacker::appendBasicType( data, size, encDataSizeOffset, tempOffset, encDataSize16);

	if (buffered == false)
	{
		//Add on the extra 2 bytes used by the total encrypted size if not buffering
		newOffset += 2;
	}

	return status? newOffset : 0;
}

bdBool bdRelayerInfo::deserialize(	void *data,
									const bdUInt size,
									bdCommonAddrRef me )
{
	bdBool status = true;
	bdUInt newOffset = 0;
	bdUInt16 encSize = 0;

	// Read out the size of the encrypted data
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, encSize);

	// Read out the packet type
	status = status && deserializeHeader( data, size, newOffset, newOffset );
	
	// Read out the load
	status = status && bdBytePacker::removeBasicType( data, size, newOffset, newOffset, m_relayerLoad );

	// Read out the connections
	bdUInt16 numConnections = 0;
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, numConnections);
	
	for( bdUInt i = 0; status && i < numConnections; ++i )
	{
		bdEndpoint tmpEndpoint;
		bdUInt16 latency = 0;
		status = status && tmpEndpoint.deserialize( me, data, size, newOffset, newOffset );
		status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, latency);
		m_relayerLatencies.pushBack( bdTargetLatency( tmpEndpoint, latency ) );
	}
	
	return status;
}

bdInt bdRelayerInfo::deserialize2(	void *data,
								 const bdUInt size,
								 bdCommonAddrRef me )
{
	bdBool status = true;
	bdUInt newOffset = 0;
	bdUInt16 encSize = 0;

	// Read out the size of the encrypted data
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, encSize);

	// Read out the packet type
	status = status && deserializeHeader( data, size, newOffset, newOffset );

	// Read out the load
	status = status && bdBytePacker::removeBasicType( data, size, newOffset, newOffset, m_relayerLoad );

	// Read out the connections
	bdUInt16 numConnections = 0;
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, numConnections);

	for( bdUInt i = 0; status && i < numConnections; ++i )
	{
		bdEndpoint tmpEndpoint;
		bdUInt16 latency = 0;
		status = status && tmpEndpoint.deserialize( me, data, size, newOffset, newOffset );
		status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, latency);
		m_relayerLatencies.pushBack( bdTargetLatency( tmpEndpoint, latency ) );
	}

	//((bdUByte8*)data) += newOffset;
	//bdUByte8* d = reinterpret_cast<bdUByte8 *>(data);
	//d += newOffset;
	//data = reinterpret_cast<void *>(d);
	if  (status == false)
	{
		newOffset = 0;
	}
	return newOffset;
}

void bdRelayerInfo::addLatencyRequest( const bdEndpoint &endpoint )
{
	m_relayerLatencies.pushBack( bdTargetLatency( endpoint, 0 ) );
}

void bdRelayerInfo::addLatencyRequest( const bdTargetLatency &targetLatency )
{
	m_relayerLatencies.pushBack( targetLatency );
}

void bdRelayerInfo::addLatencyResponse( const bdEndpoint &endpoint, bdUInt16 latency )
{
	m_relayerLatencies.pushBack( bdTargetLatency( endpoint, latency ) );
}

const bdArray<bdTargetLatency>& bdRelayerInfo::getRelayerLatencies() const
{
	return m_relayerLatencies;
}

/*bdRelayerInfo& bdRelayerInfo::operator=(const bdRelayerInfo& s)
{
	bdInt otherSize = s.m_relayerLatencies.getSize();
	bdInt mySize = m_relayerLatencies.getSize();
	if (otherSize>mySize)
	{
		bdInt difference = otherSize - mySize;
		m_relayerLatencies.increaseCapacity(difference);
	}

	m_relayerLatencies = s.m_relayerLatencies;
	return *this;
}*/


