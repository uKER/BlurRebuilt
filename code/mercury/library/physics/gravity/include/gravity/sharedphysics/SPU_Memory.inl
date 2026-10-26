#include "gravity/sharedphysics/SPU_Memory.h"

namespace SharedPhysics
{

	inline C_SPUMemoryMarker::C_SPUMemoryMarker(C_SPUMemoryPool* pPool) : m_pPool(pPool), 
			m_pMarker(pPool->m_pCurrentBlock), m_CurrentIndex(pPool->m_pCurrentBlock->m_CurrentIndex)
	{
	}

	inline void* C_SPUMemoryBlock::Alloc(const U32 size)
	{
		if((BlockSize - m_CurrentIndex) >= size)
		{
			void* thing = &m_pData[m_CurrentIndex];
			m_CurrentIndex += size;
			return thing;
		}
		return NULL;
	}

	inline C_SPUMemoryPool::C_SPUMemoryPool(C_StaticSPUMemoryAllocator* pPool) : m_pPool(pPool)
	{
#ifdef PLATFORM_SPU
		//m_pDummyBlocks = (C_SPUMemoryBlock*)Spu::Alloc(sizeof(C_SPUMemoryBlock) * pPool->m_NumElements);
		m_pPool->Alloc(&m_pDummyBlocks[0]);
		m_NumBlocks = 1;
		m_pCurrentBlock = m_pFirstBlock = &m_pDummyBlocks[0];
#else
		m_pCurrentBlock = m_pFirstBlock = m_pPool->Alloc();
#endif
	}

	inline C_SPUMemoryPool::~C_SPUMemoryPool()
	{
		m_pPool->ReleaseList(m_pFirstBlock);
		m_pFirstBlock = m_pCurrentBlock = NULL;
	}

#ifndef PLATFORM_SPU

	__forceinline void* C_SPUCache::FetchAllocData(void* ppuAddress, const U32 size)
	{
		return ppuAddress;
	}

	__forceinline void* C_SPUCache::FetchAlloc(void* ppuAddress, const U32 size)
	{
		return ppuAddress;
	}
	__forceinline void* C_SPUCache::FetchAllocAsync(void* ppuAddress, const U32 size, const U32 readMask)
	{
		return ppuAddress;
	}
	__forceinline void* C_SPUCache::CreateAlloc(void* ppuAddress, const U32 size)
	{
		return ppuAddress;
	}

	__forceinline void C_SPUCache::Store(void* ppuAddress, void* spuAddress, const U32 size)
	{
	}
#else
	inline void C_SPUCache::Store(void* ppuAddress, void* spuAddress, const U32 size)
	{
		//Spu::Store(ppuAddress, spuAddress, size);
	}
#endif

#ifndef PLATFORM_SPU

	__forceinline void* C_SmallSPUCache::FetchAllocData(void* ppuAddress, const U32 size)
	{
		return ppuAddress;
	}

	__forceinline void* C_SmallSPUCache::FetchAlloc(void* ppuAddress, const U32 size)
	{
		return ppuAddress;
	}
	__forceinline void* C_SmallSPUCache::FetchAllocAsync(void* ppuAddress, const U32 size, const U32 readMask)
	{
		return ppuAddress;
	}
	__forceinline void* C_SmallSPUCache::CreateAlloc(void* ppuAddress, const U32 size)
	{
		return ppuAddress;
	}

	__forceinline void C_SmallSPUCache::Store(void* ppuAddress, void* spuAddress, const U32 size)
	{
	}
#else
	inline void C_SmallSPUCache::Store(void* ppuAddress, void* spuAddress, const U32 size)
	{
		//Spu::Store(ppuAddress, spuAddress, size);
	}
#endif
};
