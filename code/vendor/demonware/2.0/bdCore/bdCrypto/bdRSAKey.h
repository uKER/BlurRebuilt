// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PSP-INFRA PS2 PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_RSA_KEY_H
#define BD_RSA_KEY_H


#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

#define RSA_E_SIZE 65537
#define MAX_RSA_KEY_SIZE 1024
#define MAX_RSA_KEY_SIZE_BASE_64 1024*4
#define MAX_RSA_HASH_SIZE 128
#define MAX_RSA_HASH_SIZE_BASE_64 128 *4
#define OAEP_SYSTEM_TAG "DW-RSAENC"

class bdRSAKey
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdRSAKeyStatus
		{
			BD_RSA_KEY_UNINITIALIZED,
			BD_RSA_KEY_INITIALIZED
		};

		bdRSAKey();

		~bdRSAKey();

		/// Initialize an RSA key pair, very CPU intensive, hard coded to MAX_RSA_KEY_SIZE
		/// and exponent of RSA_E_SIZE
		bdBool init();

		/// Imports a Base64 encoded key
		bdBool import(bdNChar8 *keyBase64);

		bdBool verifyHash(bdNChar8 *signatureBase64, void* data, bdULong datalen);

		/// Exports an RSA Public key from our m_key
		/// \param key[out] buffer to store public key
		/// \param keySize[out] size of buffer out key is stored inf
		/// \return Returns True on success
		bdBool exportKey(bdUByte8 *key, bdULong &keySize);

		/// Imports an RSA Public/Private key into our member key
		/// \param key[in] Buffer with Public/Private key info 
		/// \param keySize[out] Buffer size
		/// \return Returns True on success
		bdBool importKey(const bdUByte8 *key, const bdULong keySize);

		/// Encrypts data with public RSA key stored in m_keyn
		/// \param dataToEncrypt[in] Buffer with data to encrypt
		/// \note according to libtom docs, with 1024 bit RSA key and using SHA-1 hash algorithm, 86 bytes is maximum
		/// \param dataSize[in] Size of data to encrypt
		/// \param out[out] the encrypted data
		/// \param outSize[out] Size of encrypted data NOTE!, does not equal datasize with RSA encryption
		/// \return Returns True on success
		bdBool encrypt(const bdUByte8 *dataToEncrypt, const bdULong &dataSize, bdUByte8 *out, bdULong &outSize);

		/// Decrypts data with private RSA key stored in m_key
		/// \param dataToDecrypt[in] Buffer with data to decrypt
		/// \param dataSize[in] Size of data to decrypt
		/// \param out[out] the decrypted data
		/// \param outSize[out] Size of decrypted
		/// \return Returns True on success
		bdBool decrypt(const bdUByte8 *dataToDecrypt, const bdULong &dataSize, bdUByte8 *out, bdULong &outSize);

protected:

		bdRSAKeyStatus m_status;

		/// Libtom rsa_key structure
		rsa_key m_key;

private:
	    /// Helper function to generate a yarrow prng state
	    /// \param *prng[out] PRNG state
	    /// \return Returns false if function failed
	    bdBool getStatePRNG(prng_state &prng);
};


#endif // BD_RSA_KEY_H
