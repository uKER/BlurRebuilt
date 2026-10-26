// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdRelay/bdPacket/bdRelayFinal.h>

#define BD_LOG_LEVEL "bdSocket/relay"

bdRelayFinal::bdRelayFinal()
	: bdRelayHeader( BD_RELAY_FINAL )
{
}

bdRelayFinal::bdRelayFinal(	const bdEndpoint sourceEndpoint )
	: bdRelayHeader( BD_RELAY_FINAL )
	, m_sourceEndpoint( sourceEndpoint )
{
}

bdUInt bdRelayFinal::serialize(	void *data,
								const bdUInt size,
								const bdUByte8 *payload,
								const bdUInt payloadLength,
								bdBool buffered)
{
	bdBool status = true;
	bdUInt newOffset = BD_DTLS_PACKET_FIELD_SIZE;

	if (buffered == false)
	{
		//Not buffering this packet. So, need to add on an extra 2 bytes for total encrypted size
		newOffset += BD_DTLS_PACKET_FIELD_SIZE;
	}



	status = status && serializeHeader( data, size, newOffset, newOffset );

	if( status )
	{
		status = status && m_sourceEndpoint.serialize( data, size, newOffset, newOffset );
	}

	if( status )
	{
		if( payloadLength <= size - newOffset )
		{
			bdMemcpy( reinterpret_cast<bdUByte8 *>( data ) + newOffset, payload, payloadLength );
			newOffset += payloadLength;
		}
		else
		{
			status = false;
		}
	}

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


bdBool bdRelayFinal::deserialize(	void *data,
									const bdUInt size,
									bdUByte8 **payload,
									bdUInt &payloadLength,
									bdCommonAddrRef me )
{
	bdBool status = true;
	bdUInt newOffset = 0;
	bdUInt16 encSize = 0;

	// Read out the size of the encrypted data
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, encSize);
	BD_ASSERT( encSize == size - 2, "Error");

	status = status && deserializeHeader( data, size, newOffset, newOffset );
	BD_ASSERT( m_type == BD_RELAY_FINAL, "Packet is wrong type");

	if( status )
	{
		status = status && m_sourceEndpoint.deserialize( me, data, size, newOffset, newOffset );
	}

	payloadLength = size - newOffset;
	*payload = reinterpret_cast<bdUByte8*>( data ) + newOffset;

	return status;
}

bdInt bdRelayFinal::deserialize2(	void *data,
								 const bdUInt size,
								 bdUByte8 **payload,
								 bdUInt &payloadLength,
								 bdCommonAddrRef me )
{
	bdBool status = true;
	bdUInt newOffset = 0;
	bdUInt16 encSize = 0;

	// Read out the size of the encrypted data
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, encSize);
	BD_ASSERT( encSize == size - 2, "Error");

	status = status && deserializeHeader( data, size, newOffset, newOffset );
	BD_ASSERT( m_type == BD_RELAY_FINAL, "Packet is wrong type");

	if( status )
	{
		status = status && m_sourceEndpoint.deserialize( me, data, size, newOffset, newOffset );
	}

	payloadLength = size - newOffset;
	*payload = reinterpret_cast<bdUByte8*>( data ) + newOffset;
	
	if  (status == false)
	{
		newOffset = 0;
	}
	return newOffset;
}

const bdEndpoint& bdRelayFinal::getSourceEndpoint() const
{
	return m_sourceEndpoint;
}



