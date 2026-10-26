#include "StdAfx.h"
#include "gravity/sharedphysics/SPU_Memory.h"
#include <system/perf.h>

namespace SharedPhysics
{

	void* C_SPUMemoryPool::Alloc(const U32 _size, const U32 align)
	{
		U32 size = (_size + align - 1) & ~(align-1);
		void* result = Alloc(size);
		INTPTR addy = (INTPTR)result;	// [Blur2PC][x64] full address
		addy = (addy + (align - 1)) & ~(INTPTR)(align-1);
		return (void*)addy;
	}

	void* C_SPUMemoryPool::Alloc(const U32 _size)
	{
		//See if we have space in our current thing!
		U32 size = (_size + 15) & 0xfffffff0;
		BIZ_ASSERT(size < m_pCurrentBlock->BlockSize);
		//Check to see if there's space in the current block!
		void* result = m_pCurrentBlock->Alloc(size);
		if(result)
			return result;

		if(!m_pCurrentBlock->m_pNext)
		{
#ifndef PLATFORM_SPU
			m_pCurrentBlock->m_pNext = m_pPool->Alloc();
#else
			BIZ_ASSERT(m_NumBlocks < NumDummyBlocks);
			m_pCurrentBlock->m_pNext = &m_pDummyBlocks[m_NumBlocks++];
			m_pPool->Alloc(m_pCurrentBlock->m_pNext);
#endif
		}
		m_pCurrentBlock = m_pCurrentBlock->m_pNext;
		m_pCurrentBlock->m_CurrentIndex = 0;
		return m_pCurrentBlock->Alloc(size);
	}

	void C_SPUMemoryMarker::Restore()
	{
		m_pMarker->m_CurrentIndex = m_CurrentIndex;
		m_pPool->m_pCurrentBlock = m_pMarker;
	}

#ifndef PLATFORM_SPU
	C_SPUMemoryBlock* C_StaticSPUMemoryAllocator::Alloc()
#else
	C_SPUMemoryBlock* C_StaticSPUMemoryAllocator::Alloc(C_SPUMemoryBlock* spuBlock)
#endif
	{
		//PERF_EVENT("gravity.sharedphysics.AllocateBlock");
		m_Mutex.Wait();
		//OK - in between here, we need to allocate on of our buffer objects!
		C_SPUMemoryBlock* ppuBlock = (C_SPUMemoryBlock*)Spu::Get((INTPTR*)&m_pFreeList->m_pFreeElems);
		BIZ_ASSERT(ppuBlock);
		//Got original ptr to memory block, now extract block!
#ifndef PLATFORM_SPU
		C_SPUMemoryBlock* spuBlock = ppuBlock;
#else
		Spu::Fetch(spuBlock, ppuBlock, sizeof(C_SPUMemoryBlock));
#endif
		Spu::Set((INTPTR*)&m_pFreeList->m_pFreeElems, (INTPTR)(spuBlock->m_pNext));	// [Blur2PC][x64] pointer-wide
		m_Mutex.Release();
		spuBlock->m_pNext = NULL;
		//spuBlock->m_pPPUPtr = ppuBlock;
		spuBlock->m_CurrentIndex = 0;
		return spuBlock;
	}

	void C_StaticSPUMemoryAllocator::Release(C_SPUMemoryBlock* pBlock)
	{
		//PERF_EVENT("gravity.sharedphysics.ReleaseBlock");
		m_Mutex.Wait();
		//Read in the first block!
		C_SPUMemoryBlock* ppuBlock = (C_SPUMemoryBlock*)Spu::Get((INTPTR*)&m_pFreeList->m_pFreeElems);
		pBlock->m_pNext = ppuBlock;
		pBlock->m_CurrentIndex = 0;
#ifdef PLATFORM_SPU
		Spu::Store(pBlock->m_pPPUPtr, pBlock, sizeof(C_SPUMemoryBlock));
#endif
		Spu::Set((INTPTR*)&m_pFreeList->m_pFreeElems, (INTPTR)(pBlock->m_pPPUPtr));	// [Blur2PC][x64] pointer-wide
		m_Mutex.Release();
	}

	void C_StaticSPUMemoryAllocator::ReleaseList(C_SPUMemoryBlock* _pBlock)
	{
		/*m_Mutex.Wait();
		C_SPUMemoryBlock* pBlock = _pBlock;
		C_SPUMemoryBlock* lastBlock(pBlock);
		while(pBlock)
		{
			lastBlock = pBlock;
#ifdef PLATFOR_SPU
			Spu::Store(pBlock->m_pPPUPtr, pBlock, sizeof(C_SPUMemoryBlock));	
#endif
			pBlock = pBlock->m_pNext;
		}

		//m_Mutex.Wait();
		lastBlock->m_pNext = (C_SPUMemoryBlock*)Spu::Get((INTPTR*)&m_pFreeList->m_pFreeElems);
#ifdef PLATFORM_SPU
		Spu::Store(lastBlock->m_pPPUPtr, lastBlock, sizeof(C_SPUMemoryBlock));
#endif
		Spu::Set((INTPTR*)&m_pFreeList->m_pFreeElems, (INTPTR)(_pBlock->m_pPPUPtr));
		m_Mutex.Release();*/

		
		m_Mutex.Wait();
		C_SPUMemoryBlock* pBlock = _pBlock;
		C_SPUMemoryBlock* ppuBlock = (C_SPUMemoryBlock*)Spu::Get((INTPTR*)&m_pFreeList->m_pFreeElems);
		while(pBlock)
		{
			C_SPUMemoryBlock *pNextBlock = pBlock->m_pNext;
			//Release(pBlock);
			pBlock->m_pNext = ppuBlock;
			ppuBlock = pBlock->m_pPPUPtr;
			pBlock->m_CurrentIndex = 0;
	#ifdef PLATFORM_SPU
			Spu::Store(pBlock->m_pPPUPtr, pBlock, sizeof(C_SPUMemoryBlock));
	#endif
			pBlock = pNextBlock;
		}
		
		Spu::Set((INTPTR*)&m_pFreeList->m_pFreeElems, (INTPTR)(ppuBlock));	// [Blur2PC][x64] pointer-wide
		m_Mutex.Release();
	}

	C_SPUCache::C_SPUCache()
	{
#ifdef PLATFORM_SPU
		m_Oldest = 0;
#endif
	}

	C_SPUCache::~C_SPUCache()
	{
#ifdef PLATFORM_SPU
		/*for(U32 a = 0; a < NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPU)
			{
				Spu::StoreAsync((void*)m_Headers[a].m_PPUStartAddress, &m_Headers[a].m_pPage->m_Data[0], CachePageSize, 1 << 3);
			}
		}
		Spu::Wait(1 << 3);*/
#endif
	}

#ifdef PLATFORM_SPU

	//If the pages are full, don't write back the data to the ppuAddress
	void* C_SPUCache::FetchAllocData(void* ppuAddress, const U32 size)
	{

		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%NumCachePages;
				return &m_Headers[a].m_pPage->m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress > 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_pPage->m_Data[0], CachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(CachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		Spu::Fetch(&m_Headers[fetchHeader].m_pPage->m_Data[0], (void*)ppuStartAddress, CachePageSize);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_pPage->m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];
	}

	//If the pages are full, write back the data to the ppuAddress
	void* C_SPUCache::FetchAlloc(void* ppuAddress, const U32 size)
	{
		//PERF_EVENT("cache_fetch");
		//DebugBreak();
		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%NumCachePages;
				return &m_Headers[a].m_pPage->m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress > 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_pPage->m_Data[0], CachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(CachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		Spu::Fetch(&m_Headers[fetchHeader].m_pPage->m_Data[0], (void*)ppuStartAddress, CachePageSize);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_pPage->m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];

		//return Spu::FetchAlloc(ppuAddress, size);
	}

	void* C_SPUCache::CreateAlloc(void* ppuAddress, const U32 size)
	{
		//PERF_EVENT("cache_fetch");
		//DebugBreak();
		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%NumCachePages;
				return &m_Headers[a].m_pPage->m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress > 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_pPage->m_Data[0], CachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(CachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		//Spu::Fetch(&m_Headers[fetchHeader].m_pPage->m_Data[0], (void*)ppuStartAddress, CachePageSize);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_pPage->m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];

		//return Spu::FetchAlloc(ppuAddress, size);
	}

	void* C_SPUCache::FetchAllocAsync(void* ppuAddress, const U32 size, const U32 readMask)
	{
		PERF_EVENT("cache_fetch_async");
		//DebugBreak();
		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%NumCachePages;
				return &m_Headers[a].m_pPage->m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress >= 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_pPage->m_Data[0], CachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(CachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		Spu::FetchAsync(&m_Headers[fetchHeader].m_pPage->m_Data[0], (void*)ppuStartAddress, CachePageSize, readMask);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_pPage->m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];
	}


#endif
	

#ifdef PLATFORM_SPU

	//If the pages are full, don't write back the data to the ppuAddress
	void* C_SmallSPUCache::FetchAllocData(void* ppuAddress, const U32 size)
	{

		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < m_NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%m_NumCachePages;
				return &m_Headers[a].m_Page.m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%m_NumCachePages;
		}

		
		const U32 ppuStartAddress = ppuAddy & ~(SmallCachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		Spu::Fetch(&m_Headers[fetchHeader].m_Page.m_Data[0], (void*)ppuStartAddress, SmallCachePageSize);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		BIZ_ASSERT(m_Headers[fetchHeader].IsPpuAddyFullyInRange(ppuAddress, size));
		return &m_Headers[fetchHeader].m_Page.m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];
	}

	//If the pages are full, write back the data to the ppuAddress
	void* C_SmallSPUCache::FetchAlloc(void* ppuAddress, const U32 size)
	{
		//PERF_EVENT("cache_fetch");
		//DebugBreak();
		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < m_NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%m_NumCachePages;
				return &m_Headers[a].m_Page.m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%m_NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress > 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_Page.m_Data[0], SmallCachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(SmallCachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		Spu::Fetch(&m_Headers[fetchHeader].m_Page.m_Data[0], (void*)ppuStartAddress, SmallCachePageSize);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_Page.m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];

		//return Spu::FetchAlloc(ppuAddress, size);
	}

	void* C_SmallSPUCache::CreateAlloc(void* ppuAddress, const U32 size)
	{
		//PERF_EVENT("cache_fetch");
		//DebugBreak();
		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < m_NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%m_NumCachePages;
				return &m_Headers[a].m_Page.m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%m_NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress > 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_Page.m_Data[0], SmallCachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(SmallCachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		//Spu::Fetch(&m_Headers[fetchHeader].m_Page.m_Data[0], (void*)ppuStartAddress, SmallCachePageSize);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_Page.m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];

		//return Spu::FetchAlloc(ppuAddress, size);
	}

	void* C_SmallSPUCache::FetchAllocAsync(void* ppuAddress, const U32 size, const U32 readMask)
	{
		PERF_EVENT("cache_fetch_async");
		//DebugBreak();
		const U32 ppuAddy = (U32)ppuAddress;
		S32 fetchHeader = -1;

		for(U32 a = 0; a < m_NumCachePages; ++a)
		{
			if(m_Headers[a].m_PPUStartAddress < 0)
			{
				fetchHeader = a;
				break;
			}
			if (m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				m_Oldest = (a + 1)%m_NumCachePages;
				return &m_Headers[a].m_Page.m_Data[ppuAddy - m_Headers[a].m_PPUStartAddress];
			}
		}
		//If down here, we either couldn't find a hit, or 

		if(fetchHeader < 0)
		{
			fetchHeader = m_Oldest;
			m_Oldest = (m_Oldest + 1)%m_NumCachePages;
		}

		if(m_Headers[fetchHeader].m_PPUStartAddress >= 0)
		{
			//DebugBreak();
			Spu::Store((void*)m_Headers[fetchHeader].m_PPUStartAddress, &m_Headers[fetchHeader].m_Page.m_Data[0], SmallCachePageSize);
		}

		const U32 ppuStartAddress = ppuAddy & ~(SmallCachePageSize - 1);
		//Only read from 256-byte aligned memory addresses! That way, we don't get any page clashes!
		m_Headers[fetchHeader].m_bTouched = false;
		Spu::FetchAsync(&m_Headers[fetchHeader].m_Page.m_Data[0], (void*)ppuStartAddress, SmallCachePageSize, readMask);
		m_Headers[fetchHeader].m_PPUStartAddress = ppuStartAddress;
		return &m_Headers[fetchHeader].m_Page.m_Data[ppuAddy - m_Headers[fetchHeader].m_PPUStartAddress];
	}

	void PreFetchMessage(U32 fetchElements)
	{
		//sLOGINFO(0, "Pre-fetch index %i", fetchElements);
	}
	void NextElementMessage(U32 fetchElements)
	{
		//sLOGINFO(0, "Next Stream Element %i", fetchElements);
	}
	void WaitMessage(U32 nextStreamElements, U32 index)
	{
		//sLOGINFO(0, "Wait for index %i : element %i ", index, nextStreamElements);
		//DebugBreak();
	}
	void StreamMessage(U32 nextStreamElements, U32 index)
	{
		//sLOGINFO(0, "Streaming index %i : element %i", index, nextStreamElements);
	}
	void WriteBackMessage(U32 currentIndex, U32 m_WriteIndex)
	{
		//sLOGINFO(0, "Write back index %i : element %i : ", m_WriteIndex, currentIndex );
		//DebugBreak();
	}


#endif
	
	/*void C_SPUCache::Touch(void* spuAddress, const U32 size)
	{
#ifdef PLATFORM_SPU
		for(U32 a = 0; a < NumCachePages; ++a)
		{
			if(m_Headers[a].IsPpuAddyFullyInRange(ppuAddress, size))
			{
				//DebugBreak();
				m_Headers[a].m_bTouched = true;
				break;
			}
		}
#endif
	}*/

}

