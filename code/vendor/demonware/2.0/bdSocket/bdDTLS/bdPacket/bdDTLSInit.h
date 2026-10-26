// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: DTLS init packet.


#ifndef BD_DTLS_INIT_H
#define BD_DTLS_INIT_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>

class bdDTLSInit : public bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdDTLSInit();

		bdDTLSInit(bdUInt16 initTag, 
				   const bdSecurityID& secID);

		virtual ~bdDTLSInit();

		virtual bdBool serialize(void *data,const bdUInt size,
			const bdUInt offset, bdUInt &newOffset) const;
		virtual bdBool deserialize(const void* data, const bdUInt size,
			const bdUInt offset, bdUInt &newOffset);

		bdUInt16 getInitTag() const;

		/// A function to access the protected m_secID member variable
		/// \param secID[out] bdSecurityID that receives copy of m_secID
		void getSecID(bdSecurityID& secID) const;

	protected:

		bdUInt16 m_initTag;

		/// The bdSecurityID associated with this INIT
		bdSecurityID m_secID;
		/// \todo Should have signature and addr
};

#endif // BD_DTLS_INIT_H


