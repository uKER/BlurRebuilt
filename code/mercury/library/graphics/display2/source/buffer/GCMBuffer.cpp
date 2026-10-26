#include "StdAfx.h"
#include "Display2/Buffer/GCMBuffer.h"

namespace Display2
{

// C_GrowableCommandBuffer

C_GrowableCommandBuffer::C_GrowableCommandBuffer()
{
	m_CurFrame = 0;
	m_pCurAddress[0] = m_pCurAddress[1] = NULL;

	m_Size = 0;
	m_pAddress[0] = m_pAddress[1] = NULL;
}

bool C_GrowableCommandBuffer::IsCreated() const
{
	return m_Size == 0 ? false : true;
}

U32 C_GrowableCommandBuffer::GetTotalSize() const
{
	return m_Size;
}

U32 C_GrowableCommandBuffer::GetCurrentSize() const
{
	return (U32)m_pCurAddress[m_CurFrame] - (U32)m_pAddress[m_CurFrame];
}

bool C_GrowableCommandBuffer::Create(const U32 size, const U32 align, U8* pAddress0, U8* pAddress1)
{
	m_Size = size;

	m_CurFrame = 0;

	m_pCurAddress[0] = m_pAddress[0] = pAddress0;
	m_pCurAddress[1] = m_pAddress[1] = pAddress1;

	return true;
}

void C_GrowableCommandBuffer::Destroy()
{
	m_Size = 0;
}

U8* C_GrowableCommandBuffer::Alloc(const U32 size, const U32 align)
{
	// align address
	U8* pAddress = (U8*)ALIGN_SIZE((U32)m_pCurAddress[m_CurFrame], align);

	// have we run out of memory?
	if((U32)pAddress + size > (U32)m_pAddress[m_CurFrame] + m_Size)
		return NULL;

	// move current to next free address
	m_pCurAddress[m_CurFrame] = pAddress + size;

	return pAddress;
}

void C_GrowableCommandBuffer::Swap()
{
	m_CurFrame ^= 1;

	m_pCurAddress[m_CurFrame] = m_pAddress[m_CurFrame];
}

C_GrowableCommandBuffer g_GrowableCommandBuffer;

// C_DisplayList

int32_t DisplayListCallback(CellGcmContextData* pContext, uint32_t size)
{
	#if defined(CELL_GCM_DEBUG)
	size += 4;
	#endif
	
	U8* pAddress = g_GrowableCommandBuffer.Alloc(size*sizeof(U32), 4);
	if(pAddress == NULL)
	{
		LOGFATAL(g_Log, "Failed to alloc %i bytes from growable command buffer\n", size);
		return 1;
	}

	U32 offset = 0;
	S32 result = cellGcmAddressToOffset(pAddress, &offset);
	if(result != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed convert address %i to offset for growable command buffer\n", pAddress);
		return 1;
	}

	#ifdef CELL_GCM_DEBUG
	void* pCellGcmDebugCallback = Display2::DisableDebugFinish();
	#endif

	cellGcmSetJumpCommandUnsafeInline(pContext, offset);
	
	#ifdef CELL_GCM_DEBUG
	Display2::EnableDebugFinish(pCellGcmDebugCallback);
	#endif

	pContext->begin		= (U32*)pAddress;
	pContext->end		= pContext->begin+size;
	pContext->current	= pContext->begin;

	return CELL_OK;
}

U32 GetVertexShaderCommandBufferSize(CellCgbProgram* pCgbProgram, CellCgbVertexProgramConfiguration* pCgbProgramConfig)
{
	BIZ_ASSERT(pCgbProgram);
	BIZ_ASSERT(pCgbProgramConfig);
	
	C_DisplayList<C_ContextNoCheckNoTrackMeasure> displayList;

	if(!displayList.Create())
		return false;

	if(!displayList.Bind())
		return false;

	const U32 count = cellCgbGetVertexConstantCount(pCgbProgram);
	const F32* value = NULL;
	U16 reg = 0;
	U32 i = 0;
	while(i < count)
	{
		cellCgbGetVertexConstantValues(pCgbProgram, i, &reg, &value);
		displayList.m_Context.SetVertexProgramParameterBlock(reg, 1, value);
		i++;
	}

	const void* ucode = cellCgbGetUCode(pCgbProgram);
	displayList.m_Context.SetVertexProgramLoad(pCgbProgramConfig, ucode);

	displayList.Unbind();

	return displayList.GetSize();
}

bool BuildVertexShaderCommandBuffer(C_CommandChain* pCommandBuffer, CellCgbProgram* pCgbProgram, CellCgbVertexProgramConfiguration* pCgbProgramConfig)
{
	BIZ_ASSERT(pCommandBuffer);
	BIZ_ASSERT(pCommandBuffer->IsCreated());
	BIZ_ASSERT(pCgbProgram);
	BIZ_ASSERT(pCgbProgramConfig);

	C_DisplayList<C_ContextNoCheckNoTrackUnsafeInline> displayList;

	if(!displayList.Create(pCommandBuffer))
		return false;

	if(!displayList.Bind())
		return false;

	const U32 count = cellCgbGetVertexConstantCount(pCgbProgram);
	const F32* value = NULL;
	U16 reg = 0;
	for(U32 i=0; i<count; i++)
	{
		cellCgbGetVertexConstantValues(pCgbProgram, i, &reg, &value);
		displayList.m_Context.SetVertexProgramParameterBlock(reg, 1, value);
	}

	const void* ucode = cellCgbGetUCode(pCgbProgram);
	displayList.m_Context.SetVertexProgramLoad(pCgbProgramConfig, ucode);

	displayList.Unbind();

	BIZ_ASSERT(pCommandBuffer->m_Size == ((U32)displayList.m_ContextData.current - (U32)displayList.m_ContextData.begin));

	return true;
}

} // namespace Display2
