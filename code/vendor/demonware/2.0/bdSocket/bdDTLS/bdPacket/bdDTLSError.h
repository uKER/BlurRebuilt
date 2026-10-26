// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: DTLS error packet.


#ifndef BD_DTLS_ERROR_H
#define BD_DTLS_ERROR_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSHeader.h>

class bdDTLSError : public bdDTLSHeader
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdDTLSErrorType
		{
			BD_DTLS_ERROR_BAD_SECID = 0,
			BD_DTLS_INVALID_STATE
		};

		bdDTLSError();

		bdDTLSError(bdUInt16 vtag, bdDTLSErrorType etype,
					const bdSecurityID& secID);

		virtual ~bdDTLSError();

		virtual bdBool serialize(	void *data,
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset) const;

		virtual bdBool deserialize(	const void* data, 
									const bdUInt size,
									const bdUInt offset, 
									bdUInt &newOffset);

		bdDTLSErrorType getEtype() const;

		/// A function to access the protected m_secID member variable
		/// \param secID[out] bdSecurityID that receives copy of m_secID
		void getSecID(bdSecurityID& secID) const;

	protected:

		bdDTLSErrorType m_etype;
		bdSecurityID m_secID;
};

#endif // BD_DTLS_ERROR_H


