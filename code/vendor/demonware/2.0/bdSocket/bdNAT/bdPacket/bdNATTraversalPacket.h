// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A NAT traversal protocol packet.

#ifndef BD_NAT_TRAVERSAL_PACKET_H
#define BD_NAT_TRAVERSAL_PACKET_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdNAT/bdNAT.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

class bdNATTraversalPacket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdNATTraversalPacket();

		bdNATTraversalPacket(const bdUByte8 &type,
							 const bdUInt32 ident,
							 const bdAddr &addrSrc, 
							 const bdAddr &addrDest);

		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

		bdUInt getSize() const;

		bdUByte8 getType() const;
		void setType(bdUByte8 type);

		bdUInt32 getIdentifier() const;
		const bdAddr& getAddrDest() const;
		const bdAddr& getAddrSrc() const;

		const bdUByte8* getHMAC() const;
		void setHMAC(const bdUByte8* hmac, const bdUInt size);

		bdUInt16 getProtocolVersion() const;

	protected:

		bdUByte8 m_type;
		bdUInt16 m_protocolVersion;

		bdUByte8 m_hmac[BD_NAT_TRAV_HMAC_SIZE];
		bdUInt32 m_ident;
		bdAddr m_addrSrc;
		bdAddr m_addrDest;

		static bdUInt serializedSize;
};

#endif // BD_NAT_TRAVERSAL_PACKET_H
