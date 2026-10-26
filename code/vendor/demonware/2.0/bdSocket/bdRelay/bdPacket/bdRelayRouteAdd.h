// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

/// Purpose: Describes a relay packet that contains relay connection update information

#ifndef BD_RELAY_ROUTE_ADD_H
#define BD_RELAY_ROUTE_ADD_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayHeader.h>

class bdRelayRouteAdd : public bdRelayHeader
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	bdRelayRouteAdd();

	bdUInt serialize(		void *data,
							const bdUInt size,
							const bdUInt position,
							bdUInt &newPosition,
							bdBool buffered = false);

	bdBool deserialize(		void *data,
							const bdUInt size,
							bdCommonAddrRef me);

	bdInt deserialize2(	void *data,
		const bdUInt size,
		bdCommonAddrRef me);

	void addEndpoint( const bdEndpoint &endpoint );

	const bdArray<bdEndpoint>& getEndpoints() const;

protected:

	bdArray<bdEndpoint> m_endpoints;

}; // class bdRelayRouteAdd

#endif // BD_RELAY_ROUTE_ADD_H
