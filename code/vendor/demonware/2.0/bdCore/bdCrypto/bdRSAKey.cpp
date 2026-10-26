// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PSP-INFRA PS2 PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCrypto/bdRSAKey.h>
#include <bdCore/bdUtilities/bdBase64.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdCore/bdUtilities/bdTrulyRandom.h>

#define BD_LOG_LEVEL "rsakey"

bdRSAKey::bdRSAKey()
:m_status(BD_RSA_KEY_UNINITIALIZED)
{
	ltc_mp = ltm_desc;
}

bdRSAKey::~bdRSAKey()
{
	//Watch out for mem-leaks here
	if(BD_RSA_KEY_INITIALIZED == m_status)
	{
		rsa_free(&m_key);
	}
}

bdBool bdRSAKey::init()
{

	bdBool result = true;
	bdInt error = 0;

	if(BD_RSA_KEY_UNINITIALIZED  == m_status)
	{
		// A non negative return value indicates success.

		if(register_prng(&yarrow_desc) == -1)
		{
			BD_ERR(BD_LOG_LEVEL, "Register PRNG failed.");
			result = false;
		}

		prng_state prng;
		result = getStatePRNG(prng);
		
		if (result && 
			((error = rsa_make_key(&prng, find_prng("yarrow"), MAX_RSA_KEY_SIZE/8, RSA_E_SIZE, &m_key)) != CRYPT_OK))
		{
			BD_ERR(BD_LOG_LEVEL, "Unable to create private key %s.", error_to_string(error));
			result = false;
		}

		if(true == result)
		{
			m_status = BD_RSA_KEY_INITIALIZED;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot reinitialize key.");
		result = false;
	}
	return result;
}
//If this code is ported to WII, may require to set status here so mem doesn't leak
bdBool bdRSAKey::import(bdNChar8 *keyBase64)
{
	bdNChar8 key[MAX_RSA_KEY_SIZE];
	bdInt keySize;
	keySize= bdBase64::decode(keyBase64, (bdByte8*)key, 
							  bdStrnlen(keyBase64,MAX_RSA_KEY_SIZE_BASE_64));
	bdInt error;
	ltc_mp = ltm_desc;
	error = rsa_import((unsigned char*)key, keySize, &m_key);
	if (error != CRYPT_OK)
	{
		printf("rsa_import %s", error_to_string(error));
		return false;
	}
	m_status = BD_RSA_KEY_INITIALIZED;
	return true;
}

bdBool bdRSAKey::verifyHash(bdNChar8 *signature, void* data, bdULong datalen)
{
	bdInt error;
	ltc_mp = ltm_desc;

	bdHashTiger192 tigerHash;
	bdUByte8 hash[BD_TIGER_HASH_SIZE];
	bdUInt hashSize = BD_TIGER_HASH_SIZE;
	tigerHash.hash((bdUByte8*)data, datalen ,hash, hashSize);

	int hash_idx = find_hash("tiger");
	bdInt status = 0;
	bdInt signatureSize = BD_RSA_SIGNATURE_LEN;


	error = rsa_verify_hash((unsigned char*)signature, signatureSize, hash, hashSize, hash_idx, 0, &status, &m_key);
	if (error != CRYPT_OK)
	{
		printf("rsa_verify_hash %s", error_to_string(error));
		return false;
	}
	return (status == 1);
}

bdBool bdRSAKey::exportKey(bdUByte8 *key, bdULong &keySize)
{
	bdBool result = true;
	bdInt error = 0;

	if(BD_RSA_KEY_INITIALIZED  == m_status)
	{
		error = rsa_export(key, &keySize, PK_PUBLIC, &m_key);
		if (error != CRYPT_OK)
		{
			printf("rsa_export %s", error_to_string(error));
			result = false;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Exporting uninitialized private key.");
		result = false;
	}
	return result;
}

bdBool bdRSAKey::importKey(const bdUByte8 *key, const bdULong keySize)
{
	bdBool result = true;
	bdInt error = 0;
	ltc_mp = ltm_desc;
	if(BD_RSA_KEY_UNINITIALIZED  == m_status)
	{
		error = rsa_import(key, keySize, &m_key);
		if (error != CRYPT_OK)
		{
			printf("rsa_import %s", error_to_string(error));
			result = false;
		}
		m_status = BD_RSA_KEY_INITIALIZED;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Importing into initialized public key.");
		result = false;
	}

	return result;
}
bdBool bdRSAKey::encrypt(const bdUByte8 *dataToEncrypt, const bdULong &dataSize, bdUByte8 *out, bdULong &outSize)
{
	bdBool result = true;
	if (dataSize > MAX_RSA_DATA_SIZE)
	{
		BD_WARN(BD_LOG_LEVEL, "Size of data to encrypt is too large.");
		result = false;
	}

	if(BD_RSA_KEY_INITIALIZED  == m_status)
	{
		bdInt error = 0;

		if (register_hash(&sha1_desc) == -1) {
			printf("Error registering sha1");
			return EXIT_FAILURE;
		}

		bdInt hash_idx = find_hash("sha1");

		prng_state prng;
		result = getStatePRNG(prng);
		bdInt prng_idx = find_prng("yarrow");


		if ((error = rsa_encrypt_key(dataToEncrypt, // data we wish to encrypt
			dataSize, // data is 16 bytes long 
			out, // where to store ciphertext 
			&outSize, // length of ciphertext 
			(const unsigned char*)OAEP_SYSTEM_TAG, // our lparam for this program - this is a system specific tag
			bdStrlen(OAEP_SYSTEM_TAG)+1, // lparam size
			&prng, // PRNG state 
			prng_idx, // prng idx 
			hash_idx, // hash idx 
			&(m_key)) // our RSA key 
			) != CRYPT_OK) {
				printf("rsa_encrypt_key %s", error_to_string(error));
				result = false;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Can't encrypt uninitialized public key.");
		result = false;
	}
	return result;
}

bdBool bdRSAKey::decrypt(const bdUByte8 *dataToDecrypt, const bdULong &dataSize, bdUByte8 *out, bdULong &outSize)
{
	bdBool result = true;

	if (register_hash(&sha1_desc) == -1) {
		printf("Error registering sha1");
		return EXIT_FAILURE;
	}

	ltc_mp = ltm_desc;
	bdInt hash_idx = find_hash("sha1");
	bdInt err = 0,res = 0;

	if ((err = rsa_decrypt_key(dataToDecrypt, // encrypted data
		dataSize, // length of ciphertext
		out, // where to put plaintext
		&outSize, // plaintext length
		(const unsigned char*)OAEP_SYSTEM_TAG, // lparam for this program
		bdStrlen(OAEP_SYSTEM_TAG)+1, // lparam is 7 bytes long
		hash_idx, // hash idx
		&res, // validity of data
		&(m_key)) // our RSA key
		) != CRYPT_OK) {
			printf("rsa_decrypt_key %s", error_to_string(err));
			result = false;
	}

	return result;
}

bdBool bdRSAKey::getStatePRNG(prng_state &prng)
{
	bdBool result = true;
	bdInt error = 0;
	const bdUInt size = 128;
	bdUByte8 random[size];
	bdTrulyRandom::getInstance()->getRandomUByte8(random, size);

	// A non negative return value indicates success.

	if(register_prng(&yarrow_desc) == -1)
	{
		BD_ERR(BD_LOG_LEVEL, "Register PRNG failed.");
		result = false;
	}

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
	return result;
}
