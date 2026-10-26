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

namespace RingBuffer
{

Debug::Logger &g_Log = Debug::GetLogger("test");

static const U32 iAlign = 128;
static const U32 iBlockSize = 128;

/// constructor
MemorySet::MemorySet()
{
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

	// allocation only partially covers a single block
	if (iOffset+cSize <= iBlockSize)
	{
		BIZ_ASSERT(m_Set[iBlock] + cSize <= 255);
		m_Set[iBlock] += cSize;
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
}

/// free a set of memory from another set of memory
void MemorySet::Free(const MemorySet *pSet)
{
	U8 *pThis = &m_Set[0];
	const U8 *pThat = &pSet->m_Set[0];
	for (int i=0; i<RINGBUFFER_SET_SIZE; i++)
	{
		BIZ_ASSERT(*pThis >= *pThat || *pThis == 0);
		*pThis -= *pThat;
		pThis++;
		pThat++;
	}
}

/// clear the contents of the set
void MemorySet::Clear()
{
	memset(&m_Set, 0, sizeof(m_Set));
}

/// constructor
Allocator::Allocator()
: m_iAllocPtr(0)
, m_cSize(0)
, m_iHighBit(0x80)
{
}

/// initialise
void Allocator::Initialise(U8 *pMemory, U32 cSize)
{
	BIZ_ASSERT((cSize%(iBlockSize*128)) == 0);
	BIZ_ASSERT(cSize <= iBlockSize*128*RINGBUFFER_SET_SIZE);
	m_pBuffer = pMemory;
	m_iAllocPtr = 0;
	m_iFreePtr = 0;
	m_cSize = cSize;
}

/// allocate an amount of memory (size is rounded up to 128 byte aligned)
U32 Allocator::Allocate(U32 cSize, MemorySet *pSet)
{
	cSize = Maths::Align(cSize, 128);
	U32 iAlloc;
	U32 iFreePtr;
	while (1)
	{
		U64 v = MT::AtomicGet((U64*)&m_iAllocPtr);
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
		if (MT::AtomicCompareAndSwap(&m_iAllocPtr, iAlloc+cSize, iOrgAllocPtr))
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
		LOGERROR( g_Log, "OUT OF GRAPHICS RINGBUFFER MEMORY\n");
		LOGERROR( g_Log, "Req Size:  %08x\n", cSize);
		DumpMemoryUsage();
		DebugBreak();
		return 0;
	}

#ifdef DEBUG_CODE
	U32 xm = iAlloc%m_cSize;
	LOGINFO( g_Log, "Alloc [%x - %x]\n", xm, xm+cSize);
#endif

	return iAlloc;
}

/// free some memory
void Allocator::Free(const MemorySet *pSet)
{
#ifdef DEBUG_CODE
	LOGINFO( g_Log, "Free\n");
#endif

	// free memory set
	m_Set.Free(pSet);
}

/// dump memory usage (debug)
void Allocator::DumpMemoryUsage()
{
	LOGINFO( g_Log, "Alloc Ptr: %08x\n", m_iAllocPtr);
	LOGINFO( g_Log, "Free Ptr:  %08x\n", m_iFreePtr);
	LOGINFO( g_Log, "(diff):    %08x\n", m_iAllocPtr - m_iFreePtr);

	const U8 *pData = m_Set.GetRawDataPtr();
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
	const U8 *pData = m_Set.GetRawDataPtr();

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
			pData = m_Set.GetRawDataPtr();
		}
	}

	// atomically wrap the alloc and free pointers around when the free pointer reaches > 4x the ringbuffer size
	U32 cSizex4 = m_cSize * 4;
	U64 v; do { v = MT::AtomicGet((U64*)&m_iAllocPtr); }
	while ( ((U32*)&v)[1] > cSizex4 && !MT::AtomicCompareAndSwap((U64*)&m_iAllocPtr, v - ((U64)cSizex4 + (((U64)cSizex4)<<32)), v) );

	return m_iFreePtr;
}

}

}
