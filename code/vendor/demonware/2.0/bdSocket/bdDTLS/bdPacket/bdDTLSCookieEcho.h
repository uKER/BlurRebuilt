// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: DTLS cookie echo packet.


#ifndef BD_DTLS_COOKIE_ECHO_H
#define BD_DTLS_COOKIE_ECHO_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSInitAck.h>

class bdECCKey;

class bdDTLSCookieEcho : public bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdDTLSCookieEcho();

		bdDTLSCookieEcho(	bdUInt16 vtag,
							const bdDTLSInitAck &cookie,
							bdCommonAddrRef localCommonAddr,
							bdECCKey *const ECCKey);

		virtual ~bdDTLSCookieEcho();

		virtual bdBool serialize(	void *data,
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset) const;

		virtual bdBool deserialize(	const void* data, 
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset);

		const bdDTLSInitAck& getCookie() const;
		const bdUByte8* getCa() const;
		const bdUByte8* getSecID() const;
		const bdUByte8* getECCKey() const;

	protected:

		bdDTLSInitAck m_cookie;

		bdUByte8 m_ca[BD_COMMON_ADDR_SERIALIZED_SIZE];
		bdUByte8 m_secID[BD_SECURITY_ID_LENGTH];
		bdUByte8 m_ECCKey[BD_ECC_EXPORTED_KEY_SIZE];
};

#endif // BD_DTLS_COOKIE_ECHO_H


