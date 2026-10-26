// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: DTLS cookie ack packet.


#ifndef BD_DTLS_COOKIE_ACK_H
#define BD_DTLS_COOKIE_ACK_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>

class bdDTLSCookieAck : public bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdDTLSCookieAck();

		virtual ~bdDTLSCookieAck();

		bdDTLSCookieAck(bdUInt16 vtag, 
						const bdUByte8 key[BD_ECC_EXPORTED_KEY_SIZE],
						const bdSecurityID secID);

		virtual bdBool serialize(	void *data,
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset) const;

		virtual bdBool deserialize(	const void* data, 
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset);

		const bdUByte8 *getECCKey() const;

		/// A function to access the protected m_secID member variable
		/// \param secID[out] bdSecurityID that receives copy of m_secID
		void getSecID(bdSecurityID& secID) const;

	protected:

		bdUByte8 m_ECCKey[BD_ECC_EXPORTED_KEY_SIZE];

		/// The bdSecurityID associated with this CookieAck
		bdSecurityID m_secID;
};

#endif // BD_DTLS_COOKIE_ACK_H



