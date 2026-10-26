#ifndef __I_BINARYSTREAM_INL__
#define __I_BINARYSTREAM_INL__

#include <core/utility/coredebug.h>
#ifdef PLATFORM_PS3
#if CELL_SDK_VERSION < 0x270000
#include "ppu_intrinsics_gcc.h"
#else
#include "ppu_intrinsics.h"
#endif
#include "vec_types.h"
#endif

namespace Sys
{

// write a block of raw binary data, checks buffer overflow after each byte
inline bool I_BinaryStream::Write(const void *pData_, U32 cData)
{
	BIZ_ASSERT(IsWriting());

	const U8 *pData = (const U8*)pData_;
	while (cData>0)
	{
		if (m_pCurBuffer >= m_pBufferEnd)
		{
			if (!Overflow())
				return false;
		}
		*(m_pCurBuffer++) = pData ? *(pData++) : 0;
		cData--;
	}

	return true;
}


inline void FasterMemCpy( void*  __restrict dest, void*  __restrict src, U32 size )
{
#ifdef PLATFORM_PS3
	__dcbt( src );
#endif
	U32 alignCheck = (U32&)dest | (U32&)src | size;
#ifdef PLATFORM_PS3
	if (!(alignCheck & 0xf ))
	{
		U32 nrQWords = size >> 4;
		qword* pSrc = (qword*)src;
		qword* pDest = (qword*)dest;

		for (U32 x = 0 ; x < nrQWords ; x++)
		{
			*pDest = *pSrc;
			pDest++;
			pSrc++;
		}
	}
	else
#endif
	if (!(alignCheck & 0x7 ))
	{
		U32 nr64bitInts = size >> 3;
		U64* pSrc = (U64*)src;
		U64* pDest = (U64*)dest;
		for (U32 x = 0 ; x < nr64bitInts ; x++)
		{
			*pDest = *pSrc;
			pDest++;
			pSrc++;
		}
	}
	else
	if (!(alignCheck & 0x3 ))
	{
		U32 nr32bitInts = size >> 2;
		U32* pSrc = (U32*)src;
		U32* pDest = (U32*)dest;
		for (U32 x = 0 ; x < nr32bitInts ; x++)
		{
			*pDest = *pSrc;
			pDest++;
			pSrc++;
		}
	}
	else
	{
		memcpy( dest, src, size );
	}
}

// read a block of raw binary data, checks buffer underflow after each byte
inline bool I_BinaryStream::Read(void *pData_, U32 cData)
{
	BIZ_ASSERT(!IsWriting());

	if (!pData_)
	{
		// skip data
		for (;;)
		{
			m_pCurBuffer += cData;
			S32 iOver = (S32)(m_pCurBuffer - m_pBufferEnd);
			if (iOver <= 0)
				return true;

			cData = iOver;
			m_pCurBuffer = m_pBufferEnd;

			UnderflowContext uc;

			if (!Underflow(uc))
				return false;
		}
	}
#define USE_FASTER_COPIER 
#if defined( USE_FASTER_COPIER ) 
	// This copies data faster, but is not currently the fastest implementation possible.
	// XMemCpy can be used, but will fail when Write-Combined Memory is Used for GPU Resources.
	// One way of doing this faster would be to XMemCpy by default, but to use a 
	// I_BinaryStream::ReadGPU for WriteCombined GPU Resource Addresses.
	U8 *pData = (U8*)pData_;
	while (cData > 0)
	{
		U32 nrBytesToEndOfBuffer = (U32)(m_pBufferEnd - m_pCurBuffer);
		if (nrBytesToEndOfBuffer < cData)
		{
			FasterMemCpy( pData, m_pCurBuffer, nrBytesToEndOfBuffer );
			pData = pData + nrBytesToEndOfBuffer;
			m_pCurBuffer =  m_pCurBuffer + nrBytesToEndOfBuffer;
			cData -= nrBytesToEndOfBuffer;

			UnderflowContext uc(pData, cData);

			if (!Underflow(uc))
				return false;

			pData += uc.bytesRead;
			cData -= uc.bytesRead;
		} 
		else
		{
			FasterMemCpy( pData, m_pCurBuffer, cData );
			pData = pData + cData;
			m_pCurBuffer = m_pCurBuffer + cData;
			cData -= cData;
		}
	}
#else
	U8 *pData = (U8*)pData_;
	while (cData>0)
	{
		// read data
		if (m_pCurBuffer >= m_pBufferEnd)
		{
			UnderflowContext uc(pData, cData);

			if (!Underflow(uc))
				return false;

			pData += uc.bytesRead;
			cData -= uc.bytesRead;
		}
		else
		{
			*pData = *m_pCurBuffer;
			m_pCurBuffer++;
			pData++;
			cData--;
		}
	}
#endif
	return true;
}

}

#endif
