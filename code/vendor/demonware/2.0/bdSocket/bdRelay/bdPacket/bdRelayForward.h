// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

/// Purpose: Describes a relay packet whose final destination is not the recipient

#ifndef BD_RELAY_FORWARD_H
#define BD_RELAY_FORWARD_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayHeader.h>

class bdRelayForward : public bdRelayHeader
{

public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	// Constructor for deserialization
	bdRelayForward();

	// Constructor
	bdRelayForward( const bdEndpoint sourceEndpoint,
					const bdEndpoint destinationEndpoint );

	bdUInt serialize(	void *data,
						const bdUInt size,
						bdUByte8 *payload,
						const bdUInt payloadLength,
						bdBool buffered = false);

	bdBool deserialize(	void *data,
						const bdUInt size,
						bdUByte8 **payload,
						bdUInt &payloadLength,
						bdCommonAddrRef me );

	bdInt deserialize2(	void *data,
		const bdUInt size,
		bdUByte8 **payload,
		bdUInt &payloadLength,
		bdCommonAddrRef me );

	const bdEndpoint& getSourceEndpoint() const;

	const bdEndpoint& getDestinationEndpoint() const;

	bdFloat32 getTimeSinceLastUsed() const;

	void restartLastUsed();

protected:

	bdEndpoint m_sourceEndpoint;

	bdEndpoint m_destinationEndpoint;

	bdStopwatch m_lastUsed;
}; // class bdRelayForward

#endif // BD_RELAY_FORWARD_H
