// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Common DTLS layer packet header.


#ifndef BD_DTLS_HEADER_H
#define BD_DTLS_HEADER_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdDTLS/bdDTLSPackets.h>

class bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdDTLSHeader();

		virtual ~bdDTLSHeader();

		bdDTLSHeader(	bdDTLSPacketTypes type, 
						bdUInt16 vtag, 
						bdUInt16 counter);

		virtual bdBool serialize(	void *data,
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset) const;

		virtual bdBool deserialize(	const void* data, 
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset);

		bdUByte8 getType() const;
		bdUInt16 getVtag() const;
		bdUInt16 getCounter() const;

		bdUInt getSerializedSize() const;

	protected:

		bdUByte8 m_type;
		bdUByte8 m_version;
		bdUInt16 m_vtag;
		bdUInt16 m_counter;

		static bdUInt serializedSize;
};

#endif


