/** \file jobqueue_memutil.h
	SPU memory utility functions
*/
#ifndef JOBQUEUE_MEMUTIL_H
#define JOBQUEUE_MEMUTIL_H

#include <system/scheduler/jobqueue.h>

namespace Spu
{

#if PLATFORM_SPU

#define PREFETCHER_BLOCK_SIZE 128
#define PREFETCHER_MAX_BLOCKS 16
#define PREFETCHER_DMA_TAG 7

/// a low-overhead DMA utility that takes care of 128 byte alignment and prefetching
/// the next block is always being prefetched
class Prefetcher
{
public:
	Prefetcher()
	{
		m_iBlockStart = 0;
		m_iFetchStart = 0;
	}

	// fetch a block of data. each fetch is guaranteed to be after the next (although not necessarily directly after)
	// the size has to be smaller than the block size (PREFETCHER_BLOCK_SIZE)
	template<typename T>
	T *FetchNext(T *pEA)
	{
		U32 iEA = (U32)pEA;

		// try to find fetch from current block
		if ((iEA+sizeof(T))<=m_iFetchStart)
			return (T*)(iEA+m_iEABlockConv);

		return (T*)FetchMiss(iEA, sizeof(T));
	}

private:
	/// heavyweight fetch function for when a cache miss occurs
	U32 FetchMiss(U32 iEA, int cSize)
	{
		assert(cSize < PREFETCHER_BLOCK_SIZE);

		int x = iEA + cSize;
		// request is inside fetch area?
		if (x <= m_iFetchStart+PREFETCHER_BLOCK_SIZE)
		{
			// wait for outstanding prefetch to complete
			cellDmaWaitTagStatusAll(1<<PREFETCHER_DMA_TAG);

			// reached the end of m_data?
			if (m_iFetchStart+PREFETCHER_BLOCK_SIZE>=m_iBlockStart+sizeof(m_data))
			{
				// we've run out of memory. move the cache 
				int iHalf = PREFETCHER_BLOCK_SIZE*(PREFETCHER_MAX_BLOCKS/2);
				m_iBlockStart += iHalf;
				memmove(m_data, m_data+iHalf, iHalf);
				m_iEABlockConv = (U32)m_data - m_iBlockStart;
			}

			// don't prefetch into the next 64K page to prevent accidentally prefetching into unmapped pages which would cause an MFC error
			if ((m_iFetchStart+PREFETCHER_BLOCK_SIZE)&0xffff != 0)
			{
				// update pointers
				m_iFetchStart += PREFETCHER_BLOCK_SIZE;
				// kick off new prefetch
				Spu::FetchAsync((void*)((U32)m_data+m_iFetchStart-m_iBlockStart), (void*)m_iFetchStart, PREFETCHER_BLOCK_SIZE, PREFETCHER_DMA_TAG);
			}

			// return address
			return iEA+m_iEABlockConv;
		}
		else
		{
			// request is outside fetch area
			// clear cache and start again

			// wait for outstanding prefetch
			cellDmaWaitTagStatusAll(1<<PREFETCHER_DMA_TAG);

			m_iBlockStart = iEA & ~(PREFETCHER_BLOCK_SIZE-1);
			m_iFetchStart = m_iBlockStart;
			m_iEABlockConv = (U32)m_data-m_iBlockStart;

			Spu::FetchAsync(m_data, (void*)m_iBlockStart, PREFETCHER_BLOCK_SIZE, 1);
			if ((m_iFetchStart+PREFETCHER_BLOCK_SIZE)&0xffff != 0 || iEA + cSize > m_iBlockStart + PREFETCHER_BLOCK_SIZE)
			{
				m_iFetchStart += PREFETCHER_BLOCK_SIZE;
				Spu::FetchAsync((void*)(m_data+PREFETCHER_BLOCK_SIZE), (void*)m_iFetchStart, PREFETCHER_BLOCK_SIZE, PREFETCHER_DMA_TAG);
			}

			// wait for outstanding main fetch
			cellDmaWaitTagStatusAll(1<<1);

			return FetchMiss(iEA, cSize);
		}
	}

	int m_iBlockStart, m_iFetchStart, m_iEABlockConv;
	U8 ALIGN16 m_data[PREFETCHER_BLOCK_SIZE*PREFETCHER_MAX_BLOCKS];
};

#else

/// non-SPU stub implementation
class Prefetcher
{
public:
	template<typename T>
	T *FetchNext(T *_pEA) { return _pEA; }
};

#endif

}; // namespace Spu

#endif // JOBQUEUE_MEM_H
