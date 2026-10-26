#include "LogicalFsPch.h"

#if defined(PLATFORM_PC)
#include <wincrypt.h>
#elif defined(PLATFORM_PS3)
#include <cell/hash/libsha1.h>
#endif

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakSha1.h>

namespace Pak
{
namespace Sha1
{

struct C_Context
{
	bool Create()
	{
		#if defined(PLATFORM_PC)

		BOOL bResult;
		bResult = CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
		if (bResult != TRUE)
		{
			BIZ_ASSERT_FAIL("Failed to acquire cryptography context");
			return false;
		}
		bResult = CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash);
		if (bResult != TRUE)
		{
			BIZ_ASSERT_FAIL("Failed to create cryptography hashing object");
			return false;
		}

		#elif defined(PLATFORM_PS3)
		cellSha1BlockInit(&workarea);
		#endif

		return true;
	}

	bool Destroy(U8* pSha1Hash)
	{
		bool result(true);

		const U32 kSha1Size = 20;
		memset(pSha1Hash, 0, sizeof(kSha1Size));

		#if defined(PLATFORM_PC)
		U32 hashLength, hashLengthLength=sizeof(U32);
		BOOL bResult = CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashLength, (DWORD*)&hashLengthLength, 0);
		if (bResult==TRUE && hashLength == kSha1Size)
			result = true;
		result &= CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)pSha1Hash, (DWORD*)&hashLength, 0) == TRUE;
		result &= CryptDestroyHash(hHash) == TRUE;
		#elif defined (PLATFORM_PS3)
		result = cellSha1BlockResult(&workarea, pSha1Hash) == CELL_OK;
		#endif
		BIZ_ASSERT(result && "Failed to destroy SHA-1 context!");
		return result;
	}

#if defined(PLATFORM_PC)
	HCRYPTPROV hProv;
	HCRYPTHASH hHash;
#elif defined (PLATFORM_PS3)
	CellSha1WorkArea workarea;
#endif
};

C_HashContext::C_HashContext() : m_reading(true), m_pHash(NULL)
{
	m_pContext = new C_Context;
	
	if (!m_pContext->Create())
		SAFE_DELETE_PTR(m_pContext);
}

C_HashContext::C_HashContext(bool reading, U8* hash, U32 sizeBytes)
{
	m_pContext = new C_Context;

	BIZ_ASSERT((hash == NULL) || (sizeBytes == 20));

	m_reading = reading;

	if (m_reading)
	{
		m_pHash = new U8[sizeBytes];
		memcpy(m_pHash, hash, sizeBytes);
	}
	else
	{
		m_pHash = hash;
	}

	if (!m_pContext->Create())
		SAFE_DELETE_PTR(m_pContext);
}

C_HashContext::~C_HashContext()
{
	// If we were writing, then this memory isn't ours
	if (!m_reading)
	{
		if (!GetHash(m_pHash))
		{
			memset(m_pHash, 0, 20);
		}
	}
	else
	{
		SAFE_DELETE_ARY(m_pHash);
	}

	SAFE_DELETE_PTR(m_pContext);
}

bool C_HashContext::UpdateHash(U32 value, bool endianSwap)
{
	if (endianSwap)
		Ut::DoEndianSwap::EndianSwap(value);

	return UpdateHash((U8*)&value, sizeof(value));
}

bool C_HashContext::UpdateHash(const U8* pData, const U32 sizeBytes)
{
	if (!m_pContext)
		return false;

#if defined(PLATFORM_PC)
	BOOL bResult = CryptHashData(m_pContext->hHash, (const BYTE*)pData, sizeBytes, 0);
	if (bResult!=TRUE)
	{
		BIZ_ASSERT_FAIL("Failed to calculate hash");
		return false;
	}
#elif defined (PLATFORM_PS3)
	int r = cellSha1BlockUpdate(&m_pContext->workarea, pData, sizeBytes);
	if (r != CELL_OK)
	{
		BIZ_ASSERT_FAIL("Failed to update hash");
		return false;
	}
#endif
	return true;
}

bool C_HashContext::VerifyHash(const U8* pData, const U32 sizeBytes)
{
	// Ensure we're reading a file to verify the hash
	BIZ_ASSERT(m_pHash != NULL);

	return (memcmp(m_pHash, pData, sizeBytes) == 0);
}

bool C_HashContext::GetHash(U8* pSha1Hash)
{
	if (m_pContext)
	{
		bool result = m_pContext->Destroy(pSha1Hash);
		SAFE_DELETE_PTR(m_pContext);
		return result;
	}

	BIZ_ASSERT_FAIL("Invalid SHA-1 context!");
	return false;
}

}
}
