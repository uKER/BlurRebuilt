// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCrypto/bdCypher3Des.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>


#define BD_LOG_LEVEL "cypher3DES"

#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)


bdCypher3Des::bdCypher3Des()
{

	if (register_cipher(&des3_desc) == -1)
	{
		BD_ERR(BD_LOG_LEVEL, "Error registering cipher.");
	}
}

bdCypher3Des::~bdCypher3Des()
{
}

bdBool bdCypher3Des::init(const bdUByte8 *key, const bdUInt keySize)
{
	bdUByte8 iv[BD_IV_SEED_SIZE];
	bdMemset(iv, 0, BD_IV_SEED_SIZE);
	bdInt error = cbc_start(	find_cipher("3des"), // index of desired cipher
								iv, // the initial vector
								key, // the secret key
								keySize, // length of secret key
								0, // 0 == default # of rounds
								&m_cbc // where to store initialized CBC state
								);

	if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Error starting cipher.");
		return false;
	}
	return true;
}

bdBool bdCypher3Des::encrypt(const bdUByte8 *iv,
							 const bdUByte8 *pt,
							 bdUByte8 *ct,
							 const bdUInt size)
{

	bdInt error = cbc_setiv(iv, BD_IV_SEED_SIZE, &m_cbc);
	if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to set IV seed");
		BD_ERR(BD_LOG_LEVEL, "> %s", error_to_string(error));
		return false;
	}

	bdULong longSize = size;
	error = cbc_encrypt(
						pt, /* plaintext */
						ct, /* ciphertext */
						longSize, /* length of data to encrypt */
						&m_cbc /* previously initialized CBC state */
						);

	if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Error encrypting ");
		BD_ERR(BD_LOG_LEVEL, "> %s", error_to_string(error));
		return false;
	}

	return true;
}


bdBool bdCypher3Des::decrypt(const bdUByte8 *iv,
							 const bdUByte8 *ct,
							 bdUByte8 *pt,
							 const bdUInt size)
{

	bdInt error = cbc_setiv(iv, BD_IV_SEED_SIZE, &m_cbc);
	if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed to set IV seed");
		BD_ERR(BD_LOG_LEVEL, "> %s", error_to_string(error));
		return false;
	}

	bdULong longSize = size;
	error = cbc_decrypt(ct, /* ciphertext */
						pt, /* plaintext */
                        longSize, /* length of data to decrypt */
                        &m_cbc);

	if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Error decrypting.");
		return false;
	}

	return true;
}

#else // !XBOXes

bdCypher3Des::bdCypher3Des()
{
}

bdCypher3Des::~bdCypher3Des()
{
}

bdBool bdCypher3Des::init(const bdUByte8 *,
						  const bdUInt)
{
	return false;
}

bdBool bdCypher3Des::encrypt(const bdUByte8 *,
							 const bdUByte8 *,
							 bdUByte8 *,
							 const bdUInt)
{
	return false;
}


bdBool bdCypher3Des::decrypt(const bdUByte8 *,
							 const bdUByte8 *,
							 bdUByte8 *,
							 const bdUInt)
{
	return false;
}

#endif
