// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

/// Purpose: Describes a relay packet whose final destination is the recipient

#ifndef BD_RELAY_FINAL_H
#define BD_RELAY_FINAL_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayHeader.h>

class bdRelayFinal : public bdRelayHeader
{

public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	// Constructor for deserialization
	bdRelayFinal();

	// Constructor for serialization
	bdRelayFinal( const bdEndpoint sourceEndpoint );

	bdUInt serialize(	void *data,
						const bdUInt size,
						const bdUByte8 *payload,
						const bdUInt payloadLength,
						bdBool buffered = false);

	bdBool deserialize(	void *data,
						const bdUInt length,
						bdUByte8 **payload,
						bdUInt &payloadLength,
						bdCommonAddrRef me );

	bdInt deserialize2(	void *data,
		const bdUInt length,
		bdUByte8 **payload,
		bdUInt &payloadLength,
		bdCommonAddrRef me );

	const bdEndpoint& getSourceEndpoint() const;

protected:

	bdEndpoint m_sourceEndpoint;

}; // class bdRelayFinal

#endif // BD_RELAY_FINAL_H
