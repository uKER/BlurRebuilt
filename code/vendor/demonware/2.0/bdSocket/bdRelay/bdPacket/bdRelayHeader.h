// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Relay layer packet header.


#ifndef BD_RELAY_HEADER_H
#define BD_RELAY_HEADER_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdRelay/bdRelayPacket.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdMessage/bdMessageTypes.h>
#include <bdCore/bdSocket/bdSocketConfig.h>

class bdRelayHeader
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdRelayHeader();

	bdRelayHeader( bdUInt type );

	virtual ~bdRelayHeader();

	bdBool serializeHeader(	void *data,
							const bdUInt size,
							const bdUInt offset,
							bdUInt &newOffset) const;

	bdBool deserializeHeader(	const void* data,
								const bdUInt size,
								const bdUInt offset,
								bdUInt &newOffset);

	bdUByte8 getType() const;

protected:

	bdUByte8 m_type;

	bdUByte8 m_version;
};

#endif


