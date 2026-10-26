// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdRelay/bdPacket/bdRelayRouteAdd.h>

#define BD_LOG_LEVEL "bdSocket/relay"

bdRelayRouteAdd::bdRelayRouteAdd()
	: bdRelayHeader( BD_RELAY_ROUTE_ADD )
{
}

bdUInt bdRelayRouteAdd::serialize(	void *data,
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

	status = status && serializeHeader( data, size, newOffset, newOffset );

	// Figure out how many endpoints we are going to serialize into this packet
	bdEndpoint temp;
	bdUInt numConnectionsLeft = m_endpoints.getSize() - position;
	bdUInt maxConnections = (size - newOffset - sizeof(bdUInt16))/temp.getSerializedLength();
	bdUInt numConnectionsToSerialize = (numConnectionsLeft > maxConnections ) ? maxConnections : numConnectionsLeft ;

	status = status && bdBytePacker::appendBasicType( data,
													size,
													newOffset,
													newOffset,
													static_cast<bdUInt16>( numConnectionsToSerialize ) );

	// Add the endpoints
	bdUInt i;
	for( i = position; status && i < (position + numConnectionsToSerialize); ++i )
	{
		const bdEndpoint &endpoint = m_endpoints[i];
		status = status && endpoint.serialize( data, size, newOffset, newOffset );
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

bdBool bdRelayRouteAdd::deserialize(	void *data,
											const bdUInt size,
											bdCommonAddrRef me )
{
	bdBool status = true;
	bdUInt newOffset = 0;
	bdUInt16 encSize = 0;

	// Read out the size of the encrypted data
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, encSize);

	status = status && deserializeHeader( data, size, newOffset, newOffset );
	BD_ASSERT( m_type == BD_RELAY_ROUTE_ADD, "Packet is wrong type");

	bdUInt16 numConnections = 0;
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, numConnections);

	for( bdUInt i = 0; status && i < numConnections; ++i )
	{
		bdEndpoint tmpEndpoint;
		status = status && tmpEndpoint.deserialize( me, data, size, newOffset, newOffset );
		m_endpoints.pushBack( tmpEndpoint );
	}
	return status;
}

bdInt bdRelayRouteAdd::deserialize2(	void *data,
									const bdUInt size,
									bdCommonAddrRef me )
{
	bdBool status = true;
	bdUInt newOffset = 0;
	bdUInt16 encSize = 0;

	// Read out the size of the encrypted data
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, encSize);

	status = status && deserializeHeader( data, size, newOffset, newOffset );
	BD_ASSERT( m_type == BD_RELAY_ROUTE_ADD, "Packet is wrong type");

	bdUInt16 numConnections = 0;
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, numConnections);

	for( bdUInt i = 0; status && i < numConnections; ++i )
	{
		bdEndpoint tmpEndpoint;
		status = status && tmpEndpoint.deserialize( me, data, size, newOffset, newOffset );
		m_endpoints.pushBack( tmpEndpoint );
	}
	if  (status == false)
	{
		newOffset = 0;
	}
	return newOffset;
}

void bdRelayRouteAdd::addEndpoint( const bdEndpoint &endpoint )
{
	m_endpoints.pushBack( endpoint );
}

const bdArray<bdEndpoint>& bdRelayRouteAdd::getEndpoints() const
{
	return m_endpoints;
}


