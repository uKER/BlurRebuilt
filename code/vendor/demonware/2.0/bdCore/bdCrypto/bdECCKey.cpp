// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCrypto/bdECCKey.h>
#include <bdCore/bdUtilities/bdTrulyRandom.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

#define BD_LOG_LEVEL "ecckey"

bdECCKey::bdECCKey()
: m_status(BD_ECC_KEY_UNINITIALIZED)
{
	ltc_mp = ltm_desc;
}

bdECCKey::~bdECCKey()
{
	if(BD_ECC_KEY_INITIALIZED == m_status)
	{
		ecc_free(&m_key);
	}
	m_status = BD_ECC_KEY_UNINITIALIZED;
}

bdBool bdECCKey::init()
{
	bdBool result = true;
	if(BD_ECC_KEY_UNINITIALIZED  == m_status)
	{
		// A non negative return value indicates success.
		bdInt error = 0;
		if(register_prng(&yarrow_desc) == -1)
		{
			BD_ERR(BD_LOG_LEVEL, "Register PRNG failed.");
			result = false;
		}

		const bdUInt size = 128;
		bdUByte8 random[size];
		bdTrulyRandom::getInstance()->getRandomUByte8(random, size);

		prng_state prng;
		if (result && 
			((error = yarrow_start(&prng)) != CRYPT_OK))
		{
			BD_ERR(BD_LOG_LEVEL, "Start error %s.", error_to_string(error));
			result = false;
		}

		if (result && 
			((error = yarrow_add_entropy(random, size, &prng)) != CRYPT_OK))
		{
			BD_ERR(BD_LOG_LEVEL, "Add_entropy error %s.", error_to_string(error));
			result = false;
		}

		if (result && 
			((error = yarrow_ready(&prng)) != CRYPT_OK))
		{
			BD_ERR(BD_LOG_LEVEL, "Ready error %s.", error_to_string(error));
			result = false;
		}

		if (result && 
			((error = ecc_make_key(&prng, find_prng("yarrow"), BD_ECC_KEY_SIZE, &m_key)) != CRYPT_OK))
		{
			BD_ERR(BD_LOG_LEVEL, "Unable to create private key %s.", error_to_string(error));
			result = false;
		}
		if(true == result)
		{
			m_status = BD_ECC_KEY_INITIALIZED;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot reinitialize key.");
		result = false;
	}
	return result;
}

bdBool bdECCKey::exportKey(bdUByte8 *const pubKey, 
						   bdUInt &keylen)
{
	bdBool result = false;
	if(BD_ECC_KEY_INITIALIZED == m_status)
	{
		unsigned long tmpKeylen = keylen;
		bdInt error = ecc_export(pubKey, &tmpKeylen, PK_PUBLIC, &m_key);
		if (error != CRYPT_OK)
		{
			BD_ERR(BD_LOG_LEVEL, "Unable to export public key. Error: %s", error_to_string(error));
			result = false;
		}
		else
		{
			keylen = tmpKeylen;
			if (keylen < BD_ECC_EXPORTED_KEY_SIZE)
			{
				bdMemset(pubKey + keylen, 0, BD_ECC_EXPORTED_KEY_SIZE - keylen);
			}

			keylen = BD_ECC_EXPORTED_KEY_SIZE;
			result = true;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot only export initialized private key.");
	}
	return result;
}

bdBool bdECCKey::generateSharedSecret(	const bdUByte8 *const publicKey,
										const bdUInt publicKeyLen,
										bdUByte8 *secretBuffer,
										const bdUInt secretBufferLen)
{
	bdBool result = false;

	bdECCKey importedKey;
	const bdInt error = ecc_import(publicKey, publicKeyLen, &importedKey.m_key);
	if (error != CRYPT_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Unable to import public key: %s.", error_to_string(error));
		result = false;
	}
	else
	{
		result = true;

		if(BD_ECC_KEY_INITIALIZED == m_status)
		{
			unsigned long tmpLen = secretBufferLen;

			const bdInt error = ecc_shared_secret(&(m_key), &(importedKey.m_key), secretBuffer, &tmpLen);
			if (error != CRYPT_OK)
			{
				BD_ERR(BD_LOG_LEVEL, "Unable to generate secret key. Error: %s", error_to_string(error));
				result = false;
			}
			else
			{
				// If the shared secret is less than the buffer size just fill the
				// rest with zeros.
				if(tmpLen < secretBufferLen)
				{
					bdMemset(secretBuffer + tmpLen, 0, secretBufferLen - tmpLen);
				}
				result = true;
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Cannot generate secret with uninitialized key.");
		}

		// Free the memory used.
		ecc_free(&(importedKey.m_key));

	}
	return result;
}

bdECCKey::bdECCKeyStatus bdECCKey::getStatus() const
{
	return m_status;
}
