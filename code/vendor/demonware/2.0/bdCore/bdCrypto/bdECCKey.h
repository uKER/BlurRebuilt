// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Wrapper around key exchange functionality.

#ifndef BD_ECCKEY_H
#define BD_ECCKEY_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>

/// Representation of an ECC key.
/// [ECC : Elliptic Curve Cryptography]
/// This class is used as the game 'key'. 
/// It exposes functionality to export the public key
/// and generate a shared secret based on this private key 
/// and a received public one.
class bdECCKey
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdECCKeyStatus
		{
			BD_ECC_KEY_UNINITIALIZED,
			BD_ECC_KEY_INITIALIZED
		};

 		/// Default constructor
		bdECCKey();

		/// Virtual destructor
		~bdECCKey();

		/// Initialize this class.
		/// An instance of this class can only be 
		/// initialized once. Subsequent calls to init
		/// will fail, but the class will remain valid.
		/// \return
		///  - True, if the key was successfully initialized.
		///  - False otherwise.
		bdBool init();

		/// Export the public key into the supplied buffer.
		/// Only an initialized instance of this class can be
		/// exported from.
		/// \param pubKey [out] The key buffer to export into.
		/// \param keylen [in|out] 
		///  - in: The size of the key buffer.
		///  - out: The size of the key added to the buffer.
		/// \return
		///  - True, if the key was successfully exported to the buffer.
		///  - False otherwise.
		/// \todo This function should be const, but the LibTomCrypt API
		///       prevents it currently. cf ecc_export(..)
		bdBool exportKey(	bdUByte8 *const pubKey,
							bdUInt &keylen);

		/// Generate a shared secret based on a local private ECC key
		/// & a buffer containing a received public key.
		/// \param publicKey [in] The buffer containing the received public key.
		/// \param publicKeyLen [in] The size of the public key buffer.
		/// \param secretBuffer [out] The buffer to which the secret will be written.
		/// \param secretBufferLen [in] The size of the secret buffer.
		/// \return
		///  - True, if the secret was successfully generated.
		///  - False otherwise.
		/// \todo This function should be const, but the LibTomCrypt API
		///       prevents it currently. cf ecc_shared_secret(..)
		bdBool generateSharedSecret(const bdUByte8 *const publicKey,
									const bdUInt publicKeyLen,
									bdUByte8 *secretBuffer,
									const bdUInt secretBufferLen);

		/// Get the status of the ECC key.
		/// This is mainly used to determine whether the class has been 
		/// initialized already.
		/// \return
		///  - The current status of the key.
		bdECCKeyStatus getStatus() const;

	protected:

		bdECCKeyStatus m_status;

		ecc_key m_key;
};



#endif // BD_ECCKEY_H

