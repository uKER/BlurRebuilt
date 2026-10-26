#include "stdafx.h"

#include <Btl/Containers/Vector.h>
#include <core/Threads/Atomic.h>
#include "display/memory/RingBufferAllocator.h"
#include "display/memory/gcm/GCMRingBuffer.h"
#include "display/buffer/gcm/GCMDeferredCommandBufferManager.h"
#include <display2/context/GCMContext.h>
#include <system/scheduler.h>

#if !PLATFORM_SPU
#include "Display/Buffer/GCM/GCMDefaultCommandBuffer.h"
#include <Display2/Device/Device.h>
#endif

//#undef BIZ_ASSERT
//#define BIZ_ASSERT(X) if (!(X)) DebugBreak();

namespace Display
{

/// clear the contents of the set
void C_MemorySet::Clear()
{
	m_Main.Clear();
	m_Local.Clear();
}

/// alloc
void *C_SubBuffer::Alloc(U32 cSize, RingBufferAllocator::MemorySet *pSet)
{
	BIZ_ASSERT(m_pBuffer);

	// get allocation
	U32 iAddr = m_Allocator.Allocate(cSize, pSet);

	// check whether the GPU has finished with this bit of memory
	U32 iBufSize = m_Allocator.GetSize();
	
	while (1)
	{
#if PLATFORM_SPU
		U32 iFreeAddr;
		Spu::Fetch(&iFreeAddr, m_pLabel, 4);
#else
		U32 iFreeAddr = *m_pLabel;
#endif
		U32 iWrappedAddr = iAddr + cSize;
		if (iFreeAddr > iAddr)
			iWrappedAddr += 4*iBufSize;
		BIZ_ASSERT(iFreeAddr <= iWrappedAddr);
		if ((iFreeAddr + iBufSize) - iWrappedAddr<0x80000000)
			break;

#if !PLATFORM_SPU
		sys_timer_usleep(30);
#else
		// TODO: Print once per frame?
#endif
		if (m_bMain)
		{
			sLOGWARN( g_Log, "RUNNING OUT OF MAIN GRAPHICS RINGBUFFER MEMORY");
		}
		else
		{
			sLOGWARN( g_Log, "RUNNING OUT OF LOCAL GRAPHICS RINGBUFFER MEMORY");
		}
	}

	return m_pBuffer + (iAddr%m_Allocator.GetSize());
}

/// convert GPU address to EA address
U32 C_SubBuffer::GetOffsetFromAddress(void *pAddr)
{
	return ((U8*)pAddr - (U8*)m_pBuffer) + m_iOffset;
}

#if !PLATFORM_SPU

/// reset all of one the sub-buffers' internal variables
void C_SubBuffer::Reset()
{
	m_pBuffer = NULL;
	m_iFreeAddr = 0;
	m_iLastAllocPtr = 0;
	m_iLastUsagePtr = 0;
}

/// initialise one of the sub-buffers
void C_SubBuffer::Initialise(U32 cSize, bool bMain)
{
	bool bResult = Display2::Device()->AllocMemory(cSize, 128, bMain?Display2::E_Flags_System:Display2::E_Flags_Video, &m_pBuffer, Display2::E_MemoryType_CommandBuffer);
	BIZ_ASSERT(bResult);

	U32 hResult = cellGcmAddressToOffset(m_pBuffer, &m_iOffset);
	BIZ_ASSERT(hResult == CELL_OK);

	m_Allocator.Initialise(cSize);

	m_iLabel = bMain?Display2::E_Label_RingBuffer2:Display2::E_Label_RingBuffer1;
	m_pLabel = cellGcmGetLabelAddress(m_iLabel);
	*m_pLabel = 0;
	m_bMain = bMain;
}

/// shutdown one of the sub-buffers
void C_SubBuffer::Shutdown()
{
	Display2::Device()->FreeMemory(m_Allocator.GetSize(), m_pBuffer, Display2::E_MemoryType_CommandBuffer);
}

/// emit a fence
void C_SubBuffer::EmitFence(struct CellGcmContextData *pContext, const bool unsafeInline)
{
	U32 iAllocPtr = m_Allocator.GetFreeMemPtr();
	if (iAllocPtr == m_iLastAllocPtr)
		return;

	m_iLastAllocPtr = iAllocPtr;	

	if(unsafeInline)
		cellGcmSetWriteBackEndLabelUnsafeInline(pContext, m_iLabel, iAllocPtr);
	else
		GCM_NAMESPACE::cellGcmSetWriteBackEndLabel(pContext, m_iLabel, iAllocPtr);
}

/// calculate used size since last frame
U32 C_SubBuffer::CalcUsedSize()
{
	U32 x = m_Allocator.GetAllocPtr();
	U32 iUsedSize = x - m_iLastUsagePtr;
	m_iLastUsagePtr = x;
	if (iUsedSize > 0x80000000)
		iUsedSize += 4*m_Allocator.GetSize();

	return iUsedSize;
}

/// free
void C_SubBuffer::Free(const RingBufferAllocator::MemorySet *pSet)
{
	m_Allocator.Free(pSet);
}

// ------------

/// constructor
C_RingBuffer::C_RingBuffer()
{
	Reset();
};

/// allocate memory for ringbuffer
void C_RingBuffer::Initialise(U32 cLocalSize, U32 cMainSize, U32 cDefaultCommandBufferSize)
{
	BIZ_ASSERT(!m_bInitialised);
	m_bInitialised = true;

	if (cMainSize == 0)
		cMainSize = cLocalSize;

	MEMORY_TRACKER_PUSH("graphics.display.ringbuffer");

	m_Local.Initialise(cLocalSize, false);
	m_Main.Initialise(cMainSize, true);

	MEMORY_TRACKER_POP("graphics.display.ringbuffer");

	MEMORY_TRACKER_PUSH("graphics.display.defaultcommandbuffer");

	InitDefaultCommandBuffer(cDefaultCommandBufferSize);

	MEMORY_TRACKER_POP("graphics.display.defaultcommandbuffer");
}

/// free memory for ringbuffer
void C_RingBuffer::Shutdown()
{
	m_Local.Shutdown();
	m_Main.Shutdown();
	Reset();
}

/// reset all internal variables
void C_RingBuffer::Reset()
{
	m_Local.Reset();
	m_Main.Reset();
	m_bInitialised = false;
}

/// this function needs to be called every frame to clean up command buffers that were used the previous frame
void C_RingBuffer::EndFrame()
{
	BIZ_ASSERT( Display::Device()->HaveThreadOwnership() );

	if (m_bInitialised)
	{
		// wait until all deferred command buffers have been flushed
		while (!Display::g_DeferredCommandBufferManager.Flush())
		{
			sys_timer_usleep(30);
		}

		EmitFence();

		m_iLocalUsedSize = m_Local.CalcUsedSize();
		m_iMainUsedSize = m_Main.CalcUsedSize();

		// for debugging purposes, store the address and offset of the primary commandbuffer
		m_pPrimaryBegin = gCellGcmCurrentContext->begin;
		m_pPrimaryEnd = gCellGcmCurrentContext->end;
		U32 hResult = cellGcmAddressToOffset(m_pPrimaryBegin, &m_iPrimaryOffset);
		BIZ_ASSERT(hResult == CELL_OK);
	}
}

/// free memory
void C_RingBuffer::Free(const C_MemorySet *pSet, bool unsafeInline)
{
	m_Local.Free(&pSet->m_Local);
	m_Main.Free(&pSet->m_Main);
}

/// emit fence
void C_RingBuffer::EmitFence(struct CellGcmContextData *pContext, bool unsafeInline)
{
	if (!pContext)
	{
		pContext = gCellGcmCurrentContext;
		unsafeInline = false;
	}

	m_Local.EmitFence(pContext, unsafeInline);
	m_Main.EmitFence(pContext, unsafeInline);
}

/// returns the amount of memory currently used
U32 C_RingBuffer::GetUsedSize()
{
	return m_iLocalUsedSize;
}

/// returns the amount of memory currently used
U32 C_RingBuffer::GetMainUsedSize()
{
	return m_iMainUsedSize;
}

C_RingBuffer g_RingBuffer;

#endif

}	// namespace
