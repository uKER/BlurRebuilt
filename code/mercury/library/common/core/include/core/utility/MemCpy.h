#ifndef MERCURY_MEMCPY_H
#define MERCURY_MEMCPY_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include <vec_types.h>
#endif

INLINE void FastMemCpy( void*  __restrict dest, const void*  __restrict src, U32 size )
{
	#ifdef PLATFORM_PS3
	__dcbt( src );
	#endif

	U32 alignCheck = (U32)(INTPTR)dest | (U32)(INTPTR)src | size;	// [Blur2PC][x64] only the low bits matter

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
	#endif // PLATFORM_PS3
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

#endif // MERCURY_MEMCPY_H
