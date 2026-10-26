// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_HMAC_SHA1_H
#define BD_HMAC_SHA1_H

#include <bdCore/bdCrypto/bdHMac.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdCore/bdContainers/bdByteBuffer.h>

class bdHMacSHA1 : public bdHMac
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdHMacSHA1(const bdUByte8 *const key, const bdUInt keySize);

		/// Virtual destructor
		virtual ~bdHMacSHA1();

		virtual bdBool process(const bdUByte8 *const data,
							   const bdUInt length);

		virtual bdBool getData(bdUByte8 *dst,
							   bdUInt &length);

	protected:

		hmac_state m_state;
};

#endif // BD_HMAC_SHA1_H
