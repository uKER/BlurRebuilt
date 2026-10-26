#include "stdafx.h"
#include "Display/Buffer/GCM/GCMDefaultCommandBuffer.h"
#include "Display/Buffer/gcm/GCMDebugCommandBuffer.h"
#include "Display/Memory/GCM/GCMRingBuffer.h"
#include <Display2/Buffer/Buffer.h>
#include <Display2/Debug/Debug.h>

//#undef BIZ_ASSERT
//#define BIZ_ASSERT(X) if (!(X)) DebugBreak();

namespace Display
{

struct C_DefaultCommandBuffer
{
	C_DefaultCommandBuffer()
	{
		m_Pad = 0;
		m_Size = 0;
		m_Index = 0;
	}

	C_MemorySet* GetPrevMemorySet()
	{
		return &m_MemorySet[m_Index];
	}

	C_MemorySet* GetNextMemorySet()
	{
		return &m_MemorySet[!m_Index];
	}

	void SwapMemorySets()
	{
		m_Index = !m_Index;
	}

	U32					m_Pad;
	U32					m_Size;
	U32					m_Index;
	C_MemorySet			m_MemorySet[2];
};

C_DefaultCommandBuffer g_DefaultCommandBuffer;

int32_t DefaultCommandBufferCallback(CellGcmContextData* pContextData, uint32_t sizeInWords)
{
	BIZ_ASSERT(g_DefaultCommandBuffer.m_Size >= 20);
	
	BIZ_ASSERT(pContextData);

	BIZ_ASSERT(sizeInWords > 0);
	
	uint32_t sizeInBytes = sizeInWords * 4;

	BIZ_ASSERT(sizeInBytes < g_DefaultCommandBuffer.m_Size);

	// Tell the rsx to read up to the end of the old command buffer
	//cellGcmFlushUnsafeInline(pContextData);

	// Allocate the new command buffer address
	void* pNewAddress = RingBuffer()->AllocMain(g_DefaultCommandBuffer.m_Size, g_DefaultCommandBuffer.GetNextMemorySet());

	// Get the new command buffer offset from the new command buffer address
	U32 newOffset = RingBuffer()->GetOffsetFromAddressMain(pNewAddress);

	#ifdef CELL_GCM_DEBUG
	void* pCellGcmDebugCallback = Display2::DisableDebugFinish();
	#endif

	// Jump from the old command buffer to the new command buffer
	cellGcmSetJumpCommandUnsafeInline(pContextData, newOffset);

	#if 0
	Display2::C_CommandBufferWalkerInput input;
	input.m_OutputToLog		= true;
	input.m_MaxSizeInBytes	= 1024*512;
	Display2::C_CommandBufferWalkerOutput output;
	Display2::WalkCommandBuffer((U32*)pContextData->begin, (U32*)pContextData->current, input, output);
	BIZ_ASSERT((U32*)pContextData->current == output.m_pLastJumpAddress);
	#endif

	#if DEBUG_COMMAND_BUFFER
	g_MainDebugCommandBuffer.Wastage((U32)pContextData->end + 1 - (U32)pContextData->current);
	#endif
	
	// Make the new command buffer the default command buffer
	cellGcmSetCurrentBuffer((const U32*)pNewAddress, g_DefaultCommandBuffer.m_Size-g_DefaultCommandBuffer.m_Pad);

	// Write fences so the old command buffer can be freed once the rsx has finished with it
	RingBuffer()->Free(g_DefaultCommandBuffer.GetPrevMemorySet(), true);

	#ifdef CELL_GCM_DEBUG
	Display2::EnableDebugFinish(pCellGcmDebugCallback);
	#endif

	// Tell the rsx to read up to the fences in the new command buffer
	cellGcmFlush();

	// Tell this memory set to free up its associated memory
	g_DefaultCommandBuffer.GetPrevMemorySet()->Clear();
	
	// Swap the previous and next memory sets
	g_DefaultCommandBuffer.SwapMemorySets();

	#if TRACK_DISPLAY2_COMMAND_BUFFERS
	Display2::C_CommandBufferInstance* pInstance = Display2::g_CommandBufferTracker.AllocInstance("Default");
	pInstance->m_pHeadAddress = (U8*)pNewAddress;
	pInstance->m_pTailAddress = (U8*)pNewAddress + g_DefaultCommandBuffer.m_Size;
	#endif

	#if DEBUG_COMMAND_BUFFER
	g_MainDebugCommandBuffer.Realloc(g_DefaultCommandBuffer.m_Size);
	#endif
	
	return CELL_OK;
}

bool InitDefaultCommandBuffer(const U32 commandBufferSize)
{
	BIZ_ASSERT(g_DefaultCommandBuffer.m_Pad == 0);

	// For flush at end of command buffer
	#ifdef CELL_GCM_DEBUG
	//g_DefaultCommandBuffer.m_Pad = 16;
	#else
	//g_DefaultCommandBuffer.m_Pad = 4;
	#endif
	
	BIZ_ASSERT(g_DefaultCommandBuffer.m_Size == 0);

	g_DefaultCommandBuffer.m_Size = commandBufferSize;

	// Allocate the new default command buffer address
	void* pAddress = RingBuffer()->AllocMain(g_DefaultCommandBuffer.m_Size, g_DefaultCommandBuffer.GetPrevMemorySet());
	
	// Get the new default command buffer offset from the new default command buffer address
	U32 offset = RingBuffer()->GetOffsetFromAddressMain(pAddress);

	#ifdef CELL_GCM_DEBUG
	gCellGcmDebugCallback = Display2::DebugFinish;
	void* pCellGcmDebugCallback = Display2::DisableDebugFinish();
	#endif

	// Jump from the current default command buffer to the new default command buffer
	cellGcmSetJumpCommand(offset);

	// Set the current default command buffer to be the new default command buffer
	cellGcmSetCurrentBuffer((const U32*)pAddress, g_DefaultCommandBuffer.m_Size-g_DefaultCommandBuffer.m_Pad);
	
	// Set the overflow callback for the new default command buffer
	cellGcmSetUserCallback(DefaultCommandBufferCallback); 

	// Tell the rsx to read up to the start of the new default command buffer
	cellGcmFlush();

	#ifdef CELL_GCM_DEBUG
	Display2::EnableDebugFinish(pCellGcmDebugCallback);
	#endif

	#if TRACK_DISPLAY2_COMMAND_BUFFERS
	Display2::C_CommandBufferInstance* pInstance = Display2::g_CommandBufferTracker.AllocInstance("Default");
	pInstance->m_pHeadAddress = (U8*)pAddress;
	pInstance->m_pTailAddress = (U8*)pAddress + g_DefaultCommandBuffer.m_Size;
	#endif

	#if DEBUG_COMMAND_BUFFER
	g_MainDebugCommandBuffer.Alloc(g_DefaultCommandBuffer.m_Size);
	#endif

	return true;
}

}; // namespace Display
