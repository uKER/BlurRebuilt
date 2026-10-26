// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: DTLS init ack packet.


#ifndef BD_DTLS_INIT_ACK_H
#define BD_DTLS_INIT_ACK_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>

class bdHMacSHA1;

class bdDTLSInitAck : public bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdDTLSInitAck();

		bdDTLSInitAck(bdUInt16 vtag,
			bdUInt16 initTag, 
			bdUInt16 localTag,
			bdUInt16 peerTag,
			bdUInt16 localTieTag,
			bdUInt16 peerTieTag,
			bdUInt32 timestamp,
			const bdAddr &peerAddr,
			bdSecurityID m_secID);

		virtual ~bdDTLSInitAck();

		void sign(bdHMacSHA1 &hmac);
		bdBool verify(bdHMacSHA1 &hmac) const;

		virtual bdBool serialize(void *data,const bdUInt size,
			const bdUInt offset, bdUInt &newOffset) const;

		virtual bdBool deserialize(const void* data, const bdUInt size,
			const bdUInt offset, bdUInt &newOffset);

		bdUInt32 getTimestamp() const;
		bdUInt16 getInitTag() const;
		bdUInt16 getLocalTag() const;
		bdUInt16 getPeerTag() const;
		bdUInt16 getLocalTieTag() const;
		bdUInt16 getPeerTieTag() const;
		const bdAddr &getPeerAddr() const;

		/// A function to access the protected m_secID member variable
		/// \param secID[out] bdSecurityID that receives copy of m_secID
		void getSecID(bdSecurityID& secID) const;

	protected:
		bdUInt32 m_timestamp;
		bdUInt32 m_signature;
		bdUInt16 m_initTag;
		bdUInt16 m_localTag;
		bdUInt16 m_peerTag;
		bdUInt16 m_localTieTag;
		bdUInt16 m_peerTieTag;
		bdAddr m_peerAddr;
		/// The bdSecurityID associated with this INIT Ack
		bdSecurityID m_secID;
};

#endif



