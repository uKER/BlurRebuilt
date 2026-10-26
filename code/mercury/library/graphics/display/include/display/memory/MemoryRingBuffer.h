#ifndef DISPLAY_MEMORY_MEMORY_RING_BUFFER_H
#define DISPLAY_MEMORY_MEMORY_RING_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(PLATFORM_PC)

#include <Btl/Allocators/Allocator.h>
#include <Btl/Allocators/Pool.h>
#include <Btl/Utility/Threads.h>
#include <Display/Device/Device.h>

namespace Display
{

class C_RingBufferBlock
{
public:
	C_RingBufferBlock();
	bool IsBusy() const;
public:
	C_RingBufferBlock*	m_pNext;
	U8*					m_pAddress;
	U32					m_Offset;
	U32					m_Fence;
};

class C_RingBuffer
{
public:

	C_RingBuffer();
	~C_RingBuffer();

	bool				Create(const U32 size, const U32 alignment, const E_AllocHeap eHeap);
	void				Destroy();	

	C_RingBufferBlock*	Allocate(const U32 size, const U32 alignment);

private:

	void	WaitForFreeSpace(const U8* pBase, const U32 size);
	void	Reset();

private:

	U8*						m_pBase;
	U8*						m_pTop;
	U8*						m_pPosition;
	U32						m_Offset;
	Display::E_AllocHeap	m_eHeap;

	C_RingBufferBlock*		m_pFirstBlock;
	C_RingBufferBlock*		m_pLastBlock;

	typedef Btl::DynamicPool<C_RingBufferBlock> RingBufferBlockPool;
	RingBufferBlockPool		m_RingBufferBlockPool;

};

}; // namespace Display

#endif // PLATFORM_PC

#endif // DISPLAY_MEMORY_MEMORY_RING_BUFFER_H
