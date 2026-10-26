#include "StdAfx.h"
#include "Display2/Buffer/GCMBufferSPU.h"
#include "Display2/Debug/Debug.h"
#include <System/Scheduler.h>

//#define WALK_COMMAND_BUFFER 1

namespace Display2
{

U32 PadContextData(CellGcmContextData* pContextData, const U32 align)
{
	BIZ_ASSERT(pContextData);
	BIZ_ASSERT(pContextData->end);
	BIZ_ASSERT(pContextData->begin);
	BIZ_ASSERT(pContextData->current);
	BIZ_ASSERT(pContextData->end > pContextData->begin);
	BIZ_ASSERT((align & 3) == 0);

	const U32 size = (U32)pContextData->current - (U32)pContextData->begin;

	const U32 alignedSize = ALIGN_SIZE(size, align);

	const U32 diff = alignedSize - size;
	
	BIZ_ASSERT(pContextData->current >= pContextData->begin && pContextData->current <= pContextData->end);
	
	if(diff > 0)
	{
		C_ContextNoCheckNoTrackUnsafeInline context(pContextData);
		
		context.SetNopCommand(diff / 4);
	}

	BIZ_ASSERT(pContextData->current >= pContextData->begin && pContextData->current <= pContextData->end);
	
	return alignedSize;
}

#ifdef CELL_GCM_DEBUG

void DebugFinish(struct CellGcmContextData* pContextData)
{
	static U32 sLabelValue = 0;

	void* pCellGcmDebugCallback = DisableDebugFinish();

	cellGcmSetWriteBackEndLabelUnsafeInline(pContextData, E_Label_DebugFinish, sLabelValue);

	cellGcmFlush(pContextData);

	volatile U32 iLabelValue = *cellGcmGetLabelAddress(E_Label_DebugFinish);

	while(iLabelValue != sLabelValue)
	{
		sys_timer_usleep(100);	

		iLabelValue = *cellGcmGetLabelAddress(E_Label_DebugFinish);
	}

	sLabelValue++;

	EnableDebugFinish(pCellGcmDebugCallback);
}

void* DisableDebugFinish()
{
	void (*pCellGcmDebugCallback)(struct CellGcmContextData*) = gCellGcmDebugCallback;
	gCellGcmDebugCallback = NULL;
	return (void*)pCellGcmDebugCallback;
}

void EnableDebugFinish(void* pCellGcmDebugCallback)
{
	gCellGcmDebugCallback = (void (*)(CellGcmContextData*))pCellGcmDebugCallback;
}

#endif

C_CommandBuffer::C_CommandBuffer() :
	m_EndOffset(UINT_MAX),
	m_Destination(UINT_MAX)
{
	#if !CFG_GOLD
	m_DebugName[0] = '\0';
	#endif
}

C_CommandBuffer::~C_CommandBuffer()
{
}

bool C_CommandBuffer::Run()
{
#if PLATFORM_SPU
	BIZ_ASSERT_FAIL("Cannot be called from an SPU!");
#else
	cellGcmReserveMethodSize(gCellGcmCurrentContext, 1); // Reserve enough space in the default command buffer for the j2n
	
	void* pEndAddress = NULL;
	S32 result = cellGcmIoOffsetToAddress(m_EndOffset, &pEndAddress); // Where our command buffer ends in main memory
	BIZ_ASSERT(result == CELL_OK);
	
	Display2::C_ContextData contextData;
	cellGcmSetupContextData(&contextData, (U32*)pEndAddress, 4, NULL);
	Display2::C_ContextNoCheckNoTrackUnsafeInline context(&contextData); // Create a context to render to the end of our command buffer

	U32 currentOffset = 0;
	result = cellGcmAddressToOffset(gCellGcmCurrentContext->current, &currentOffset); // Get the current offset of the default command buffer
	BIZ_ASSERT(result == CELL_OK);

	#ifdef CELL_GCM_DEBUG
	void* pCellGcmDebugCallback = Display2::DisableDebugFinish();
	#endif

	context.SetJumpCommand(currentOffset+4); // Add a jump back to the default command buffer to the end of our command buffer

#if WALK_COMMAND_BUFFER && !PLATFORM_SPU
	void* pHeadAddress = NULL;
	result = cellGcmIoOffsetToAddress(m_Offset, &pHeadAddress); // Where our command buffer starts in main memory
	BIZ_ASSERT(result == CELL_OK);
	
	void* pTailAddress = NULL;
	result = cellGcmIoOffsetToAddress(currentOffset+4, &pTailAddress); // Where our command buffer ends in main memory (including the j2n)
	BIZ_ASSERT(result == CELL_OK);

	C_CommandBufferWalkerInput input;
	input.m_OutputToLog			= true;
	input.m_MaxSizeInBytes		= 1024*512; // Assuming no command chain will be bigger than this!
	C_CommandBufferWalkerOutput output;
	
	WalkCommandBuffer((U32*)pHeadAddress, (U32*)pTailAddress, input, output);

	BIZ_ASSERT((U32*)pTailAddress == output.m_pLastJumpAddress); // Make sure our command buffer jumps back to the default command buffer
#endif
	
	cellGcmSetJumpCommandUnsafeInline(gCellGcmCurrentContext, m_Offset); // Jump to our command buffer

	#ifdef CELL_GCM_DEBUG
	Display2::EnableDebugFinish(pCellGcmDebugCallback);
	#endif

#endif

	return true;
}

#if PLATFORM_SPU
void C_CommandBuffer::Flush(CellGcmContextData* pContextData)
{
	BIZ_ASSERT(m_Offset != UINT_MAX);
	BIZ_ASSERT(m_pAddress != NULL);
	BIZ_ASSERT(m_EndOffset != UINT_MAX);
	BIZ_ASSERT(pContextData != NULL);
	BIZ_ASSERT(pContextData->current >= pContextData->begin);
	BIZ_ASSERT(pContextData->current <= pContextData->end);
	BIZ_ASSERT(m_EndOffset >= m_Offset);

	const U32 conDatUsed = (U32)pContextData->current - (U32)pContextData->begin;
	const U32 conDatFree = (U32)pContextData->end - (U32)pContextData->current;
	
	const U32 comBufUsed = m_EndOffset - m_Offset;
	const U32 comBufFree = (m_Offset + m_Size) - m_EndOffset;

	const U32 conDatSize = PadContextData(pContextData, 16);
	
	BIZ_ASSERT(comBufFree >= 16);
	BIZ_ASSERT(conDatSize <= (comBufFree - 16));
	Spu::Store(m_pAddress + comBufUsed, pContextData->begin, conDatSize);

	BIZ_ASSERT(m_EndOffset <= (m_Offset + (m_Size - 16)));
	m_EndOffset += conDatSize;
	BIZ_ASSERT(m_EndOffset <= (m_Offset + (m_Size - 16)));
}
#endif

void BuildJumpToNextCommandBuffer(C_CommandBuffer* pCommandBuffer, const U32 offset)
{
	BIZ_ASSERT(pCommandBuffer);
	BIZ_ASSERT(pCommandBuffer->m_Size >= 16);
	
	C_ContextData contextData;
		
	pCommandBuffer->Setup(&contextData);

	C_ContextNoCheckNoTrackUnsafeInline context(&contextData);

	context.SetJumpCommand(offset);

	context.SetNopCommand(3);

	BIZ_ASSERT(contextData.current >= contextData.begin && contextData.current <= contextData.end + 1);
}

C_RingBuffer::C_RingBuffer()
{
	m_CurOffset		= 0;
	m_pCurOffset	= &m_CurOffset;
	m_CurFrame		= 0;
	m_MaxSize		= 0;
	m_pAddress[0]	= NULL;
	m_pAddress[1]	= NULL;
	m_Offset[0]		= 0;
	m_Offset[1]		= 0;
	m_AllocSize		= 0;
	m_AllocCount	= 0;
	m_AllocWasted	= 0;
}

U32 C_RingBuffer::GetStartOffset() const
{
	return m_Offset[m_CurFrame];
}

U8* C_RingBuffer::GetStartAddress() const
{
	return m_pAddress[m_CurFrame];
}

U8* C_RingBuffer::GetAddressFromOffset(const U32 offset) const
{
	return m_pAddress[m_CurFrame] + (offset - m_Offset[m_CurFrame]);
}

U32 C_RingBuffer::Used()
{
	return m_CurOffset - m_Offset[m_CurFrame];
}

bool C_RingBuffer::Create(const U32 size, const U32 offset0, const U32 offset1, U8* pAddress0, U8* pAddress1)
{
	m_MaxSize		= size;

	m_CurFrame		= 0;
	m_CurOffset		= offset0;

	m_Offset[0] 	= offset0;
	m_Offset[1] 	= offset1;

	m_pAddress[0] 	= pAddress0;
	m_pAddress[1] 	= pAddress1;

	return true;
}

void C_RingBuffer::Destroy()
{
	m_MaxSize = 0;
}

void C_RingBuffer::Swap()
{
	m_CurFrame		^= 1;

	m_CurOffset		= m_Offset[m_CurFrame];

	m_AllocSize  	= 0;
	m_AllocCount 	= 0;
	m_AllocWasted	= 0;
}

U32 C_RingBuffer::Alloc(const U32 size)
{
	const U32 alignedSize = ALIGN_SIZE(size, 128);
	
	const U32 startOffset = MT::AtomicAdd((volatile U32*)m_pCurOffset, alignedSize);		

	if(((startOffset+alignedSize)-m_Offset[m_CurFrame]) > m_MaxSize)
	{
		sLOGERROR(g_Log, "Failed to alloc %i bytes out of ring buffer\n", size);
		DebugBreak();
		
		return 0;
	}
	else
	{
		m_AllocSize += alignedSize;
		m_AllocCount++;

		return startOffset;
	}
}

} // namespace Display2
