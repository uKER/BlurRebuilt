#ifndef DISPLAY2_GCM_BUFFER_SPU_INL
#define DISPLAY2_GCM_BUFFER_SPU_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE U8* C_CommandBuffer::GetStartAddress() const
{
	return m_pAddress;
}

INLINE U8* C_CommandBuffer::GetEndAddress() const
{
	return m_pAddress+(m_EndOffset-m_Offset);
}

INLINE U32 C_CommandBuffer::GetStartOffset() const
{
	return m_Offset;
}

INLINE U32 C_CommandBuffer::GetEndOffset() const
{
	return m_EndOffset;
}

INLINE bool C_RingBuffer::IsCreated() const
{
	return m_MaxSize == 0 ? false : true;
}

INLINE U32 C_RingBuffer::GetMaxSize() const
{
	return m_MaxSize;
}

INLINE U32 C_RingBuffer::GetCurSize() const
{
	return m_CurOffset - (U32)m_pAddress[m_CurFrame];
}

template <class T_Context>
INLINE C_DisplayList<T_Context>::C_DisplayList() :
	m_Bound(false),
	m_Created(false),
	m_pCommandBuffer(NULL)
{
	m_Context.m_pData = &m_ContextData;

	#ifdef CELL_GCM_DEBUG
	m_pCellGcmDebugCallback = NULL;
	#endif
}

template <class T_Context>
INLINE U32 C_DisplayList<T_Context>::GetSize() const
{
	return (U32)(m_ContextData.current - m_ContextData.begin) * sizeof(U32);
}

template <class T_Context>
INLINE bool C_DisplayList<T_Context>::Create(C_CommandChain* pCommandBuffer, CellGcmContextCallback contextCallback)
{
	assert(!m_Bound && !m_Created);

	m_pCommandBuffer = pCommandBuffer;

	if(m_pCommandBuffer)
	{
		cellGcmSetupContextData(&m_ContextData, (U32*)m_pCommandBuffer->m_pAddress, m_pCommandBuffer->m_Size, contextCallback);

		//assert((m_pCommandBuffer->m_Offset & 0x3) == 0);
	}
	else
	{
		cellGcmSetupContextData(&m_ContextData, NULL, 0, NULL);
	}
	
	m_Created = true;

	return true;
}

template <class T_Context>
INLINE void C_DisplayList<T_Context>::Destroy()
{
	assert(!m_Bound);

	m_pCommandBuffer = NULL;

	m_Created = false;
}

template <class T_Context>
INLINE bool C_DisplayList<T_Context>::Bind()
{
	assert(!m_Bound && m_Created);

	#ifdef CELL_GCM_DEBUG
	BIZ_ASSERT(m_pCellGcmDebugCallback == NULL);
	m_pCellGcmDebugCallback = DisableDebugFinish();
	#endif

	m_Bound = true;

	return true;
}

template <class T_Context>
INLINE void C_DisplayList<T_Context>::Unbind()
{
	assert(m_Bound && m_Created);

	m_Context.SetReturnCommand();

	#ifdef CELL_GCM_DEBUG
	BIZ_ASSERT(m_pCellGcmDebugCallback != NULL);
	EnableDebugFinish(m_pCellGcmDebugCallback);
	m_pCellGcmDebugCallback = NULL;
	#endif

	m_Bound = false;
}

template <class T_Context>
INLINE void C_DisplayList<T_Context>::Unbind(const U32 jumpToOffset)
{
	assert(m_Bound && m_Created);

	m_Context.SetJumpCommand(jumpToOffset);

	#ifdef CELL_GCM_DEBUG
	BIZ_ASSERT(m_pCellGcmDebugCallback != NULL);
	EnableDebugFinish(m_pCellGcmDebugCallback);
	m_pCellGcmDebugCallback = NULL;
	#endif

	m_Bound = false;
}

template <class T_Context>
INLINE C_CommandList<T_Context>::C_CommandList() :
	m_pBuffer(NULL)
{
	#ifdef CELL_GCM_DEBUG
	m_pCellGcmDebugCallback = NULL;
	#endif
}

template <class T_Context>
INLINE bool C_CommandList<T_Context>::Create(C_CommandBuffer* pCommandBuffer, C_ContextData* pContextData)
{
	BIZ_ASSERT(pCommandBuffer);
	BIZ_ASSERT(pContextData);

	T_Context::m_pData = pContextData;
	m_pBuffer = pCommandBuffer;

	return true;
}

template <class T_Context>
INLINE void C_CommandList<T_Context>::Destroy()
{
	T_Context::m_pData = NULL;
	m_pBuffer = NULL;
}

template <class T_Context>
INLINE bool C_CommandList<T_Context>::Bind()
{
	BIZ_ASSERT(T_Context::m_pData);
	BIZ_ASSERT(m_pBuffer);
	BIZ_ASSERT(m_pBuffer->IsCreated());

	#ifdef CELL_GCM_DEBUG
	m_pCellGcmDebugCallback = DisableDebugFinish(); // Don't disable debug finish if rendering to the default command buffer
	#endif

	return true;
}

template <class T_Context>
INLINE void C_CommandList<T_Context>::Unbind()
{
	BIZ_ASSERT(T_Context::m_pData);
	BIZ_ASSERT(m_pBuffer);
	BIZ_ASSERT(m_pBuffer->IsCreated());

	#ifdef CELL_GCM_DEBUG
	EnableDebugFinish(m_pCellGcmDebugCallback); // Don't disable debug finish if rendering to the default command buffer
	#endif
}

} // namespace Display2

#endif // DISPLAY2_GCM_BUFFER_SPU_INL
