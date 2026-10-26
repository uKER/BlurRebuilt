#include "stdafx.h"
#include "Display/Buffer/GCM/GCMCommandBuffer.h"
#include "Display/Manager/GCM/GCMManager.h"
#include "Display/Device/Device.h"
#include "Display/Buffer/Gcm/GCMUserFifo.h"
#include "Display/Buffer/gcm/GCMDeferredCommandBufferManager.h"
#include <Display2/Debug/Debug.h>

#if !CFG_SPEED && !PLATFORM_SPU
Debug::Bool g_DisplayWalkDeferredCommandBuffers("Graphics.Display.WalkDeferredCommandBuffers", false);
#endif

#if !PLATFORM_SPU
#ifndef CFG_GOLD
Debug::Bool g_DisplayWaitForVertexBuildTasks("Graphics.Display.WaitForVertexBuildTasks", true);
#else
const bool g_DisplayWaitForVertexBuildTasks = true;
#endif
#endif

namespace Display
{

C_DeferredCommandBufferManager g_DeferredCommandBufferManager;
S32	g_VertexBuildingJobs = 0;

/// constructor
C_DeferredCommandBufferManager::C_DeferredCommandBufferManager()
{
	m_iRead = m_iWrite = 0;
}

extern Debug::Logger &g_Log;

#define COMMAND_BUFFER_HOLE_SIZE (1*4)+(3*4)+(2*16) // 1 jump, 3 nops, 2 labels

/// add a command buffer to be patched into the main command buffer later on
void C_DeferredCommandBufferManager::AddCommandBuffer(C_CommandBuffer *p)
{
	U32 i = m_iWrite;
	assert((i+1)%MAX_DEFERRED_COMMAND_BUFFERS != m_iRead%MAX_DEFERRED_COMMAND_BUFFERS);
	m_Entries[i%MAX_DEFERRED_COMMAND_BUFFERS] = p;

	// make hole in main commandbuffer
	// store offset of hole in commandbuffer
	U32 iSize = COMMAND_BUFFER_HOLE_SIZE;
	p->m_Destination = GCMCreateCommandBufferHole(iSize);

	BIZ_ASSERT((U32)gCellGcmCurrentContext->current == (U32)p->m_Destination + COMMAND_BUFFER_HOLE_SIZE);

	m_iWrite++;

	cellGcmFlush();
}

/// patch in any command buffers that are ready
/// returns true if all commandbuffers were flushed, or false if some commandbuffers haven't finished building yet
bool C_DeferredCommandBufferManager::Flush()
{
	while (m_iRead - m_iWrite > 0x80000000 )
	{
		C_CommandBuffer *pCommandBuffer = m_Entries[m_iRead%MAX_DEFERRED_COMMAND_BUFFERS];

		if (MT::AtomicGet(&pCommandBuffer->m_EndOffset) == UINT_MAX)
			return false;

		BIZ_ASSERT(pCommandBuffer->m_Offset != UINT_MAX);

		if(g_DisplayWaitForVertexBuildTasks && g_VertexBuildingJobs != 0)
			return false;
			
		// Where our command buffer ends in main memory
		void* pEndAddress = NULL;
		S32 result = cellGcmIoOffsetToAddress(pCommandBuffer->m_EndOffset, &pEndAddress);
		BIZ_ASSERT(result == CELL_OK);
		
		// Offset of our command buffer hole in the default command buffer
		U32 holeOffset = 0;
		result = cellGcmAddressToOffset((void*)pCommandBuffer->m_Destination, &holeOffset);
		BIZ_ASSERT(result == CELL_OK);

		// Write a jump back to the default command buffer to the end of our command buffer
		Display2::C_ContextData cbContextData;
		cellGcmSetupContextData(&cbContextData, (U32*)pEndAddress, 4+4, NULL);
		Display2::C_ContextNoCheckNoTrackUnsafeInline cbContext(&cbContextData);
		cbContext.SetJumpCommand(holeOffset+4);
		MT::MemoryFence();

		#if !CFG_SPEED && !PLATFORM_SPU
		if(g_DisplayWalkDeferredCommandBuffers)
			Display2::WalkCommandBuffer(pCommandBuffer->m_Offset, pCommandBuffer->m_EndOffset, false, false);
		#endif

		// Write a jump to our command buffer to our command buffer hole in the default command buffer
		U32 j2nBuffer[COMMAND_BUFFER_HOLE_SIZE/4];
		Display2::C_ContextData j2nContextData;
		cellGcmSetupContextData(&j2nContextData, j2nBuffer, COMMAND_BUFFER_HOLE_SIZE+4, NULL);
		Display2::C_ContextNoCheckNoTrackUnsafeInline context2(&j2nContextData);
		context2.SetJumpCommand(pCommandBuffer->m_Offset);
		context2.SetNopCommand(3);
		
		BIZ_ASSERT((j2nContextData.current - j2nContextData.begin) == 4);

		Display::RingBuffer()->Free(&pCommandBuffer->m_UsedMemory);
		Display::RingBuffer()->EmitFence(&j2nContextData, true);

		BIZ_ASSERT(j2nContextData.current <= j2nContextData.end);

		// Pad out the jump to next command buffer with nops
		S32 j2nNopCount = j2nContextData.end - j2nContextData.current;
		BIZ_ASSERT(j2nNopCount>=0 && j2nNopCount<(COMMAND_BUFFER_HOLE_SIZE/4));
		if(j2nNopCount > 0)
			context2.SetNopCommand(j2nNopCount);

		BIZ_ASSERT(j2nContextData.current == j2nContextData.end);
		BIZ_ASSERT((U32)j2nContextData.end - (U32)j2nContextData.begin == COMMAND_BUFFER_HOLE_SIZE);

		// Write the jump to next to our command buffer hole in the default command buffer
		MathsStorage::UInt4 *j2sBuffer = (MathsStorage::UInt4*)(pCommandBuffer->m_Destination);
		j2sBuffer[2] = MathsStorage::UInt4(j2nBuffer[8], j2nBuffer[9], j2nBuffer[10], j2nBuffer[11]);
		j2sBuffer[1] = MathsStorage::UInt4(j2nBuffer[4], j2nBuffer[5], j2nBuffer[ 6], j2nBuffer[ 7]);
		MT::MemoryFence();
		j2sBuffer[0] = MathsStorage::UInt4(j2nBuffer[0], j2nBuffer[1], j2nBuffer[ 2], j2nBuffer[ 3]);
		
		m_iRead++;

		cellGcmFlush();
	}

	return true;
}

}; // namespace Display
