// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Interface for Cypher implementations.

#ifndef BD_CYPHER_3DES_H
#define BD_CYPHER_3DES_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdCrypto/bdCypher.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

#include <bdCore/bdContainers/bdByteBuffer.h>

class bdCypher3Des : public bdCypher
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdCypher3Des();

		/// Virtual destructor
		virtual ~bdCypher3Des();

		/// Initializes the cipher with an encryption key
		/// \param key[in] : pointer to the encryption key
		/// \param keySize[in] : The length of the encryption key in bytes
		/// \return True  : The cipher was initialized successfully
		///			False : an error occurred
		bdBool init(const bdUByte8 *key,
					const bdUInt keySize);

		/// Encrypts a plain text message
		/// \param iv[in] : pointer to the IV seed. For 3Des the IV seed is 8 bytes
		/// \param pt[in] : pointer to the plain text buffer
		/// \param ct[in] : pointer to the cypher text buffer
		/// \param size[in] : length of the plain text buffer
		/// \note both plain text and cypher text buffer have to be a multiple of the
		///		cypher block size (8 bytes for 3DES)
		virtual bdBool encrypt(const bdUByte8 *iv,
							   const bdUByte8 *pt,
							   bdUByte8 *ct,
							   const bdUInt size);

		/// Decrypts a cypher  text message
		/// \param iv[in] : pointer to the IV seed. For 3Des the IV seed is 8 bytes
		/// \param ct[in] : pointer to the cypher text buffer
		/// \param pt[in] : pointer to the plain text buffer
		/// \param size[in] : length of the plain text buffer
		/// \note both plain text and cypher text buffer have to be a multiple of the
		///		cypher block size (8 bytes for 3DES)
		virtual bdBool decrypt(const bdUByte8 *iv,
							   const bdUByte8 *ct,
							   bdUByte8 *pt,
							   const bdUInt size);

		
	protected:

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON
		symmetric_CBC m_cbc;
#endif // !BD_PLATFORM_XBOX  && !BD_PLATFORM_XENON

};

#endif // BD_CYPHER_3DES_H

