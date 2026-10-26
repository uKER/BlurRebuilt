// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: DTLS data packet.


#ifndef BD_DTLS_DATA_H
#define BD_DTLS_DATA_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>

class bdCypher;

class bdDTLSData : public bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdDTLSData();

		virtual ~bdDTLSData();

		bdDTLSData(bdUInt16 vtag, bdUInt16 counter);

		bdBool serialize(	void *data,
							const bdUInt size,
							const bdUInt offset,
							bdUInt &newOffset,
							const bdSequenceNumber &lastSequenceNumber,
							const bdUByte8 sharedKey[BD_SHARED_KEY_SIZE],
							const bdUByte8 *const payload,
							const bdUInt payloadSize,
							bdCypher *const cypher);

		// This will attempt to treat all data in data buffer as being a part of this packet!
		bdBool deserialize(	const void *data,
							const bdUInt size,
							const bdUInt offset,
							bdUInt &newOffset,
							const bdSequenceNumber &lastSequenceNumber,
							const bdUByte8 sharedKey[BD_SHARED_KEY_SIZE],
							bdUByte8 *payload,
							const bdUInt maxPayloadSize,
							bdUInt &payloadSize,
							bdCypher *const cypher);

	protected:
		virtual bdBool serialize(void *data,const bdUInt size,
			const bdUInt offset, bdUInt &newOffset) const;
		virtual bdBool deserialize(const void* data, const bdUInt size,
			const bdUInt offset, bdUInt &newOffset);

	protected:
		bdUByte8 m_hmac[BD_DTLS_HMAC_SIZE];
};


#endif // BD_DTLS_DATA_H


