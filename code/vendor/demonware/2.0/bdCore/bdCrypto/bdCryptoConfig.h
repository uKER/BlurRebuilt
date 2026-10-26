// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Config for bdCrypto.

#ifndef BD_CRYPTO_CONFIG_H
#define BD_CRYPTO_CONFIG_H

#include <bdPlatform/bdPlatformConfig/bdPlatformConfig.h>

#if !defined BD_PLATFORM_XBOX
// LibTomCrypt uses "export" as a function name, which is a keyword
// in C++. Here we redefine export so that C++ compilers won't
// complain.
#	define export pexport

// Use LibTomMath
#	define LTM_DESC
#	include <external/libtomcrypt/src/headers/tomcrypt.h>

#	undef export
#	undef byte
#else

typedef int	hmac_state;
typedef int ecc_key;

#endif //  !defined BD_PLATFORM_XBOX

#define BD_ENCRYPTION_KEY_SIZE		(24)
#define BD_SHARED_KEY_SIZE			(24)

#define BD_ECC_KEY_SIZE				(28u)
#define BD_ECC_EXPORTED_KEY_SIZE	(100)
#define BD_ECC_SHARED_SECRET_SIZE	(28)

#define BD_IV_SEED_SIZE				(8)
#define BD_CYPHER_BLOCK_SIZE		(8)
#define BD_HMAC_SIZE				(10)
#define BD_3DES_BLOCK_SIZE			(8)
#define BD_TIGER_HASH_SIZE			(24)

#define BD_RSA_SIGNATURE_LEN        (128)
#define MAX_RSA_DATA_SIZE           (86)

#endif // BD_CRYPTO_CONFIG_H

