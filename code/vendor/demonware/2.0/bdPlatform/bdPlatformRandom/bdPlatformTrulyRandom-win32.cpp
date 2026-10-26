// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformRandom/bdPlatformTrulyRandom.h>

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0400
#endif
#include <windows.h>
#include <wincrypt.h>

// The handle to the random generator.
static HCRYPTPROV s_provider;	// [Blur2PC][x64] ULONG_PTR; bdULong is 32-bit

class bdPlatformTrulyRandomInit
{
	public:

		// Repeated calls to acquire & release will leak memory on some versions of Windows
		//BD_DECORATOR("workaround" , "http://support.microsoft.com/default.aspx?scid=kb;en-us;258000");

		bdPlatformTrulyRandomInit()
		{
			CryptAcquireContext(&s_provider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
		}

		~bdPlatformTrulyRandomInit()
		{
			CryptReleaseContext(s_provider, 0);
		}
};

// Used to bind the handle to the generator.
static bdPlatformTrulyRandomInit s_randomInit;

void BD_CALL bdGetRandomUChar8(bdUByte8 in[], 
							   const bdUInt length)
{
	CryptGenRandom(s_provider, length, in);
}

