#ifndef __GCMRINGBUFFER_H__
#define __GCMRINGBUFFER_H__

#include "display/memory/RingBufferAllocator.h"

namespace Display
{

class ALIGN16 C_MemorySet
{
	friend class C_RingBuffer;
public:
	/// clear the contents of the set
	void Clear();

private:
	RingBufferAllocator::MemorySet m_Local, m_Main;
};

/// sub-buffer
class C_SubBuffer
{
	friend class C_RingBuffer;
private:
	/// initialise one of the sub-buffers
	void Initialise(U32 cSize, bool bMain);

	/// reset all internal variables
	void Reset();

	/// shutdown one of the sub-buffers
	void Shutdown();

	/// emit a fence
	void EmitFence(struct CellGcmContextData *pContext, const bool unsafeInline = false);

	/// calculate used size since last frame
	U32 CalcUsedSize();

	/// alloc
	void *Alloc(U32 cSize, RingBufferAllocator::MemorySet *pSet);

	/// free
	void Free(const RingBufferAllocator::MemorySet *pSet);

	/// convert EA address to GPU offset
	U32 GetOffsetFromAddress(void *pAddr);

private:
	U8 *m_pBuffer;
	U32 m_iOffset;
	U32 m_iLastAllocPtr;
	U32 m_iLastUsagePtr;

	Display::RingBufferAllocator::Allocator m_Allocator;

	U32 m_iFreeAddr;
	U32 m_iLabel;
	U32 *m_pLabel;
	bool m_bMain;
};

/// global graphics ringbuffer
class ALIGN16 C_RingBuffer
{
public:
	/// constructor
	C_RingBuffer();

	/// initialise ringbuffer
	void Initialise(U32 cLocalSize, U32 cMainSize = 0, U32 cDefaultCommandBufferSize = 1024*32);

	/// allocate some memory
	void *Alloc(U32 cSize, C_MemorySet *pSet) { return m_Local.Alloc(cSize, &pSet->m_Local); }

	/// allocate some main memory
	void *AllocMain(U32 cSize, C_MemorySet *pSet) { return m_Main.Alloc(cSize, &pSet->m_Main); }

	/// convert EA address to GPU offset
	U32 GetOffsetFromAddress(void *pAddr) { return m_Local.GetOffsetFromAddress(pAddr); }

	/// convert EA address to GPU offset
	U32 GetOffsetFromAddressMain(void *pAddr) { return m_Main.GetOffsetFromAddress(pAddr); }

	/// free a memory set
	void Free(const C_MemorySet *pSet, bool unsafeInline = false);

	/// free ringbuffer
	void Shutdown();

	/// returns the amount of local memory currently used
	U32 GetUsedSize();

	/// returns the amount of main memory currently used
	U32 GetMainUsedSize();

	/// called once at the end of each frame
	void EndFrame();

	/// emit a fence
	void EmitFence(struct CellGcmContextData *pContext = NULL, bool unsafeInline = false);

private:
	/// reset all internal variables
	void Reset();

	C_SubBuffer m_Local, m_Main;

	U32 m_iLocalUsedSize, m_iMainUsedSize;
	U32 m_iLastAllocPtr;
	bool m_bInitialised;

	/// for debugging
	void *m_pPrimaryBegin;
	void *m_pPrimaryEnd;
	U32 m_iPrimaryOffset;
};

extern C_RingBuffer g_RingBuffer;

/// get pointer to global graphics ringbuffer
inline C_RingBuffer *RingBuffer()
{
	return &g_RingBuffer;
}

}

#endif
