#include "stdafx.h"

#include <core/maths/MathsUtil.h>
#include "Display/memory/RingBufferAllocator.h"
#include <core/threads/atomic.h>

/*
#define DEBUG_CODE

#undef optimize
#pragma optimize("", off)

#ifdef DEBUG_CODE
#include <core\containers\String.h>

#include <System\debuglog.h>
#include <system\debug\log\AutoConfigure.h>
#endif
*/
//#undef BIZ_ASSERT
//#define BIZ_ASSERT(X) if (!(X)) DebugBreak();

namespace Display
{

namespace RingBufferAllocator
{

#if !PLATFORM_SPU
Debug::Logger &g_Log = Debug::GetLogger("test");
#endif

static const U32 iAlign = 128;
static const U32 iBlockSize = 128;

/// constructor
MemorySet::MemorySet()
{
	m_iMin = 0;
	m_iMax = RINGBUFFER_SET_SIZE;
	Clear();
}

/// add a range of memory to the set of allocations
void MemorySet::Add(U32 iAddr, U32 cSize)
{
	// divide everything by the align size
	BIZ_ASSERT((iAddr & (iAlign-1)) == 0 && (cSize & (iAlign-1)) == 0);
	iAddr/=iAlign;
	cSize/=iAlign;

	U32 iBlock = iAddr/iBlockSize;
	U32 iOffset = iAddr%iBlockSize;

	BIZ_ASSERT(iBlock>=0 && iBlock<RINGBUFFER_SET_SIZE);

	m_iMin = Maths::Min(m_iMin, iBlock);

	// allocation only partially covers a single block
	if (iOffset+cSize <= iBlockSize)
	{
		BIZ_ASSERT(m_Set[iBlock] + cSize <= 255);
		m_Set[iBlock] += cSize;
		m_iMax = Maths::Max(m_iMax, iBlock+1);
		return;
	}

	// allocation covers first partial block
	U32 d = iBlockSize-iOffset;
	BIZ_ASSERT(m_Set[iBlock] + d <= 255);
	m_Set[iBlock] += d;
	cSize -= d;
	iBlock++;

	// process the rest of the allocation
	while (cSize > 0)
	{
		U32 d = Maths::Min(iBlockSize, cSize);
		if (iBlock >= RINGBUFFER_SET_SIZE)
			iBlock = 0;
		BIZ_ASSERT(m_Set[iBlock] + d <= 255);
		m_Set[iBlock] += d;
		cSize -= d;
		iBlock++;
	}

	m_iMax = Maths::Max(m_iMax, iBlock);
}

/// clear the contents of the set
void MemorySet::Clear()
{
	int n = m_iMax-m_iMin;
	if (n <= 0)
		return;

	memset(&m_Set[m_iMin], 0, n);
	m_iMin = RINGBUFFER_SET_SIZE;
	m_iMax = 0;
}

#if !PLATFORM_SPU
/// free a set of memory from another set of memory
void MemorySet::Free(const MemorySet *pSet)
{
	int n = pSet->m_iMax-pSet->m_iMin;
	if (n <= 0)
		return;

	BIZ_ASSERT(pSet->m_iMin<RINGBUFFER_SET_SIZE && pSet->m_iMax<=RINGBUFFER_SET_SIZE && pSet->m_iMin+n<=RINGBUFFER_SET_SIZE);

	U8 *pThis = &m_Set[pSet->m_iMin];
	const U8 *pThat = &pSet->m_Set[pSet->m_iMin];
	for (int i=0; i<n; i++)
	{
		BIZ_ASSERT(*pThis >= *pThat || *pThis == 0);
		*pThis -= *pThat;
		pThis++;
		pThat++;
	}
}

/// constructor
Allocator::Allocator()
: m_iAllocPtr(0)
, m_cSize(0)
, m_iHighBit(0x80)
{
	m_pSet = new MemorySet;
	m_pThis = this;
}

/// destructor
Allocator::~Allocator()
{
	delete m_pSet;
}

/// initialise
void Allocator::Initialise(U32 cSize)
{
	BIZ_ASSERT((cSize%(iBlockSize*128)) == 0);
	BIZ_ASSERT(cSize <= iBlockSize*128*RINGBUFFER_SET_SIZE);
	m_iAllocPtr = 0;
	m_iFreePtr = 0;
	m_cSize = cSize;
}
#endif

/// allocate an amount of memory (size is rounded up to 128 byte aligned)
U32 Allocator::Allocate(U32 cSize, MemorySet *pSet)
{
	cSize = Maths::Align(cSize, 128);
	U32 iAlloc;
	U32 iFreePtr;
	while (1)
	{
		U64 v = MT::AtomicGet((U64*)&m_pThis->m_iAllocPtr);
		U32 iOrgAllocPtr = iAlloc = ((U32*)&v)[0];
		iFreePtr = ((U32*)&v)[1];

		// check whether the allocation goes off the end of the ringbuffer, and if so skip the last bit of memory
		U32 iMod = iOrgAllocPtr%m_cSize;
		U32 c = 0;
		if (iMod + cSize > m_cSize)
		{
			c = m_cSize - iMod;
			iAlloc += c;
		}

		// atomically allocate the memory. iAlloc contains the allocation address
		if (MT::AtomicCompareAndSwap(&m_pThis->m_iAllocPtr, iAlloc+cSize, iOrgAllocPtr))
		{
			if (pSet)
			{
				if (c)
				{
					pSet->Add(iMod, c);
					iMod = 0;
				}
				pSet->Add(iMod, cSize);
			}
			break;
		}
	}

	// this checks whether the alloc pointer has wrapped around and hit the free pointer,
	// which would indicate we used all of the ringbuffer and ar
	if ((((iAlloc + cSize) - (iFreePtr + m_cSize)))<0x80000000)
	{
		sLOGERROR( g_Log, "OUT OF GRAPHICS RINGBUFFER MEMORY\n");
		sLOGERROR( g_Log, "Req Size:  %08x\n", cSize);
#if !PLATFORM_SPU
		DumpMemoryUsage();
#endif
		DebugBreak();
		return 0;
	}

#ifdef DEBUG_CODE
	U32 xm = iAlloc%m_cSize;
	LOGINFO( g_Log, "Alloc [%x - %x]\n", xm, xm+cSize);
#endif

	return iAlloc;
}

#if !PLATFORM_SPU

/// free some memory
void Allocator::Free(const MemorySet *pSet)
{
#ifdef DEBUG_CODE
	LOGINFO( g_Log, "Free\n");
#endif

	// free memory set
	m_pSet->Free(pSet);
}

/// dump memory usage (debug)
void Allocator::DumpMemoryUsage()
{
	LOGINFO( g_Log, "Alloc Ptr: %08x\n", m_iAllocPtr);
	LOGINFO( g_Log, "Free Ptr:  %08x\n", m_iFreePtr);
	LOGINFO( g_Log, "(diff):    %08x\n", m_iAllocPtr - m_iFreePtr);

	const U8 *pData = m_pSet->GetRawDataPtr();
	Util::C_String sMem, sTmp;
	for (int i=0; i<RINGBUFFER_SET_SIZE; i++)
	{
		sTmp.Printf("%02x", pData[i]);
		sMem += sTmp;
		if (i%32 == 31)
		{
			LOGINFO( g_Log, "%08x: %s\n", (i-31)*128*128, sMem.Chars());
			sMem = "";
		}
	}
}

/// return the allocation pointer (for debugging purposes)
U32 Allocator::GetAllocPtr()
{
	return m_iAllocPtr;
}

/// get updated 'free memory' pointer
U32 Allocator::GetFreeMemPtr()
{
	U32 iWrap = m_cSize/(iBlockSize*128);

	// update 'free memory' pointer
	U32 x = MT::AtomicGet(&m_iFreePtr)/(128*iBlockSize);
	x = x % iWrap;
	const U8 *pData = m_pSet->GetRawDataPtr();

	// check free memory
	while (1)
	{
		if (pData[x] != m_iHighBit)
			break;

#ifndef CFG_SPEED
		U64 ptrs = MT::AtomicGet((U64*)&m_iAllocPtr);
		BIZ_ASSERT(((U32*)&ptrs)[0] - ((U32*)&ptrs)[1] < 0x80000000);	// atomically check that m_iAllocPtr > m_iFreePtr
#endif

		MT::AtomicAdd(&m_iFreePtr, 128*iBlockSize);	// making this atomic means it won't interfere with the wrap guard in the multithreaded alloc function
		x++;
		if (x >= iWrap)
		{
			x = 0;
			m_iHighBit ^= 0x80;
			pData = m_pSet->GetRawDataPtr();
		}
	}

	// atomically wrap the alloc and free pointers around when the free pointer reaches > 4x the ringbuffer size
	U32 cSizex4 = m_cSize * 4;
	U64 v; do { v = MT::AtomicGet((U64*)&m_iAllocPtr); }
	while ( ((U32*)&v)[1] > cSizex4 && !MT::AtomicCompareAndSwap((U64*)&m_iAllocPtr, v - ((U64)cSizex4 + (((U64)cSizex4)<<32)), v) );

	return m_iFreePtr;
}
#endif

}

}
