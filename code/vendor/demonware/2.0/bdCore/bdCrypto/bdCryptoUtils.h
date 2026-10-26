// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PS2 PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_CRYPTO_UTILS_H
#define BD_CRYPTO_UTILS_H

#include <bdCore/bdMemory/bdMemory.h>

/// This class provides common crypto utility functions
/// Note: These functions incur a high CPU overhead.
class bdCryptoUtils
{
	public:
		/// Get a new truly random number to use as a seed for an encryption
		/// initial vector.
		/// \return	A 32-bit unsigned initial vector seed.
		static bdUInt32 BD_CALL getNewIVSeed();

		/// Get an initial vector using a particular seed.
		/// \param	seed[in]	The seed to calculate the initial vector from.
		/// \param	iv[out]		A pointer to an array to fill in with the calculate
		///						initial vector. This array should be of size
		///						BD_TIGER_HASH_SIZE
		static void BD_CALL calculateInitialVector(const bdUInt32 seed, bdUByte8* iv);

		/// Encrypt some data with 3DES.
		/// \note	The data size must be a multiple of 8.
		/// \param	key[in]				Pointer to the key to encrypt with. Must
		///								be of size BD_ENCRYPTION_KEY_SIZE.
		/// \param	initialVector[in]	The initial vector to use for encryption.
		///								This must be of size BD_IV_SEED_SIZE. The
		///								initial vector must be shared between encryption
		///								and decryption, and should not be re-used for
		///								encrypting different data.
		///	\param	source[in]			The data to encrypt.
		/// \param	dest[out]			Pointer to a buffer to place the encrypted data.
		///	\param	size[in]			The size of the source data to encrypt. This must
		///								be a multiple of 8. The destination data will
		///								also be of this size.
		static void BD_CALL encrypt(const void *key,
									const void *initialVector,
									const void *source,
									void *dest,
									const bdUInt size);

		/// Decrypt some data with 3DES.
		/// \note	The data size must be a multiple of 8.
		/// \param	key[in]				Pointer to the key to decrypt with. Must
		///								be of size BD_ENCRYPTION_KEY_SIZE.
		/// \param	initialVector[in]	The initial vector to use for decryption.
		///								This must be of size BD_IV_SEED_SIZE. The
		///								initial vector must be shared between encryption
		///								and decryption, and should not be re-used for
		///								encrypting different data.
		///	\param	source[in]			The data to decrypt.
		/// \param	dest[out]			Pointer to a buffer to place the decrypted data.
		///	\param	size[in]			The size of the source data to decrypt. This must
		///								be a multiple of 8. The destination data will
		///								also be of this size.
		static void BD_CALL decrypt(const void *key,
								    const void *initialVector,
									const void *source,
									void *dest,
									const bdUInt size);
};


#endif // BD_CRYPTO_UTILS_H
