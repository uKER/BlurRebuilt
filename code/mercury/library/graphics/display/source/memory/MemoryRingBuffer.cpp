#include "stdafx.h"
#include "Display/Memory/MemoryRingBuffer.h"

namespace Display
{

C_RingBufferBlock::C_RingBufferBlock() : m_pNext(0), m_pAddress(0), m_Offset(0), m_Fence(0)
{
}

bool C_RingBufferBlock::IsBusy() const
{
	return !Device()->IsFencePassed(m_Fence);
}

C_RingBuffer::C_RingBuffer()
{
	Reset();
}

C_RingBuffer::~C_RingBuffer()
{
	Destroy();
}

bool C_RingBuffer::Create(const U32 size, const U32 alignment, const E_AllocHeap eHeap)
{
	if (!Device()->AllocMemory(alignment, size, &m_pBase, eHeap, E_AllocType_Start))
		return false;

	m_eHeap		= eHeap;
	m_pTop		= m_pBase + size;
	m_pPosition	= m_pBase;
#if defined(PLATFORM_PS3)
	cellGcmAddressToOffset(m_pBase, &m_Offset);
#else
	m_Offset	= (U32)m_pTop;
#endif

	return true;
}

void C_RingBuffer::Destroy()
{
	if (m_pBase)
	{
		Device()->FreeMemory(m_pTop - m_pBase, m_pBase, m_eHeap);
		Reset();
	}
}

C_RingBufferBlock* C_RingBuffer::Allocate(const U32 size, const U32 alignment)
{
	U32 allocSize	= size + alignment;
	U8* pAllocPos	= m_pPosition;

	WaitForFreeSpace(pAllocPos, allocSize);

	// check if we need to wrap around to the beginning
	U8* pAllocTop = pAllocPos + allocSize;
	if (pAllocTop > m_pTop)
	{
		pAllocPos	= m_pBase;
		pAllocTop	= pAllocPos + allocSize;

		WaitForFreeSpace(pAllocPos, allocSize);
	}

	const INTPTR alignmentMask = alignment - 1;	// [Blur2PC][x64] align the full address
	pAllocPos = (U8*)((INTPTR)(pAllocPos + alignmentMask) & ~alignmentMask);

	m_pPosition	= pAllocTop;

	// allocate a new block for this memory
	C_RingBufferBlock* pBlock	= m_RingBufferBlockPool.Allocate();
	pBlock->m_pAddress			= pAllocPos;
	pBlock->m_Offset			= (U32)(pAllocPos - m_pBase + m_Offset);
	pBlock->m_pNext				= 0;
	pBlock->m_Fence				= Device()->GetCurrentFence() + 0x0FFFFFFF; // default to expiring in a very long time

	if (m_pLastBlock)
		m_pLastBlock->m_pNext = pBlock;
	if (!m_pFirstBlock)
		m_pFirstBlock = pBlock;

	m_pLastBlock = pBlock;
	return pBlock;
}

void C_RingBuffer::WaitForFreeSpace(const U8* pBase, const U32 size)
{
	C_RingBufferBlock* pFirstBlock = m_pFirstBlock;
	while (pFirstBlock && ((U32)(pFirstBlock->m_pAddress - pBase) < size))
	{
		Device()->BlockOnFence(pFirstBlock->m_Fence);
		// might need to do a flush of the push buffer to ensure commands are getting processed

		C_RingBufferBlock* pNextBlock = pFirstBlock->m_pNext;
		m_RingBufferBlockPool.Free(pFirstBlock);
		pFirstBlock = pNextBlock;
	}

	m_pFirstBlock = pFirstBlock;
	if (!m_pFirstBlock)
		m_pLastBlock = 0;
}

void C_RingBuffer::Reset()
{
	m_pBase			= 0;
	m_pTop			= 0;
	m_pPosition		= 0;
	m_Offset		= 0;
	m_pFirstBlock	= 0;
	m_pLastBlock	= 0;
	// clear out the block pool when a function is available
}

}; // namespace Display
