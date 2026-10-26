// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

/// Purpose: Describes a relay packet that contains relay connection update information

#ifndef BD_RELAYER_INFO_H
#define BD_RELAYER_INFO_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayHeader.h>

#define BD_TARGET_LATENCY_SERIALIZED_SIZE ( BD_COMMON_ADDR_SERIALIZED_SIZE + BD_SECURITY_ID_LENGTH + 2 )

struct bdTargetLatency
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	bdTargetLatency( bdEndpoint target, bdUInt16 latency )
		: m_target( target )
		, m_latency( latency )
		, m_isLatencyValid( true )
	{}

	bdTargetLatency( bdEndpoint target )
		: m_target( target )
		, m_latency( 0 )
		, m_isLatencyValid( false )
	{}

	bdEndpoint	m_target;
	bdUInt16	m_latency;
	bdBool		m_isLatencyValid;
};

class bdRelayerInfo : public bdRelayHeader
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	bdRelayerInfo( bdUByte8 type );

	bdUInt serialize(	void *data,
						const bdUInt size,
						const bdUInt position,
						bdUInt &newPosition,
						bdBool buffered = false);

	bdBool deserialize(	void *data,
						const bdUInt size,
						bdCommonAddrRef me);

	bdInt deserialize2(	void *data,
		const bdUInt size,
		bdCommonAddrRef me);


	void addLatencyRequest( const bdEndpoint &endpoint );

	void addLatencyRequest( const bdTargetLatency &targetLatency );

	void addLatencyResponse( const bdEndpoint &endpoint, bdUInt16 latency );

	const bdArray<bdTargetLatency>& getRelayerLatencies() const;

	void setRelayerLoad( bdUInt32 newLoad ) { m_relayerLoad = newLoad; }

	bdUInt32 getRelayerLoad() { return m_relayerLoad; }

	void increaseCapacity(bdUInt newCapacity) { m_relayerLatencies.increaseCapacity(newCapacity);}

	//bdRelayerInfo& operator=(const bdRelayerInfo& s);

protected:

	bdArray<bdTargetLatency> m_relayerLatencies;

	bdUInt32 m_relayerLoad;

}; // class bdRelayerInfo

#endif // BD_RELAYER_INFO_H
