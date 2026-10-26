#ifndef LOGICALFS_PAKFS_PAK_SHA1_H
#define LOGICALFS_PAKFS_PAK_SHA1_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Pak
{
	namespace Sha1
	{
		struct C_Context;

		//Generate 20-byte SHA-1 hash.
		class C_HashContext
		{
		protected:
			C_Context* m_pContext;

			U8*		m_pHash;
			bool	m_reading;

		public:
			C_HashContext();
			C_HashContext(bool reading, U8* hash, U32 sizeBytes);
			~C_HashContext();

			bool Valid() const { return m_pContext != NULL; }

			bool UpdateHash(const U8* pData, const U32 sizeBytes);
			bool UpdateHash(U32 value, bool endianSwap);

			bool VerifyHash(const U8* pData, const U32 sizeBytes);

			bool GetHash(U8* pSha1Hash);
		};
	}
}

#endif
