#ifndef RENDER_DYNAMICEFFECT_BLOCKMANAGER_H
#define RENDER_DYNAMICEFFECT_BLOCKMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system\memory.h>
#include <Dynamic/Atomic.h>

namespace DynamicEffect
{

extern Debug::Logger &g_Log;

class BlockManagerBase
{
public:
	BlockManagerBase() : m_iBlockSize(1), m_iMax(32), m_iAllocated(0), m_iElementSize(0) { m_pAllocated = &m_iAllocated; }
	
	void		SetBlockSize(S32 iBlockSize) { m_iBlockSize = iBlockSize; }
	U32			GetBlockSize() const { return m_iBlockSize; }

	void		SetMaxBlocks(S32 iMax) { m_iMax = iMax; }
	U32			GetMaxBlocks() const { return m_iMax; }

	void		SetElementSize(U32 iElementSize) { m_iElementSize = iElementSize; }
	U32			GetElementSize() const { return m_iElementSize; }
	U32			GetAllocatedBlocks() const { return m_iAllocated; }

protected:
	U32			m_iBlockSize;
	S32			m_iMax;
	S32			m_iAllocated;
	S32*		m_pAllocated;
	U32			m_iElementSize;
};

// manages vector blocks of the same type, reserves space for number of blocks and elements per block
template <class BlockType>
class ALIGN16 VectorBlockManager : public BlockManagerBase
{
public:
	VectorBlockManager() : BlockManagerBase() {  };

	typedef		AtomicDynamicPool<BlockType>			BlockPool;
	
	// gives a block of particle storage to an Emitter class
	BlockType*	NewBlock()
	{
		BlockType* pBlock = NULL;

		S32 allocated = MT::AtomicIncrement(m_pAllocated);
		if(allocated < m_iMax)
		{
			pBlock = m_BlockPool.AtomicAllocate();
		}
		return pBlock;
	}

	void Update()
	{
		MEMORY_TRACKER_PUSH("graphics.dynamic.particlestorage");
		if(m_iAllocated > m_iMax)
			m_iAllocated = m_iMax;

		S32 currentSize = m_BlockPool.m_PoolElements < 0 ? 0 : m_BlockPool.m_PoolElements;
		m_BlockPool.Update();

		// reserve the element and block size
		for(S32 i = currentSize; i < m_BlockPool.m_PoolElements; ++i)
		{
			BlockType* pBlock = m_BlockPool.m_Pool[i];
			pBlock->m_Element.Reserve(m_iElementSize, m_iBlockSize);
		}
		MEMORY_TRACKER_POP("graphics.dynamic.particlestorage");
	}

	// returns the block of storage back to the block manager when it's finished
	void FreeBlock(BlockType* pBlock)
	{
		//assert(m_iAllocated <= m_iMax);
		m_BlockPool.Free(pBlock);
		--m_iAllocated;
	}
	
	const BlockPool&	GetBlock() { return m_BlockPool; }
	
private:
	// static storage for each instantiated block manager
	BlockPool	m_BlockPool;

};

}

#endif // RENDER_DYNAMICEFFECT_BLOCKMANAGER_H
