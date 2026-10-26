#ifndef DISPLAY2_GCM_BUFFER_SPU_H
#define DISPLAY2_GCM_BUFFER_SPU_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Context/GCMContextSPU.h"
#include <Core/Threads/Atomic.h>
#include <Core/Maths/Storage.h>

namespace Display2
{

U32 PadContextData(CellGcmContextData* pContextData, const U32 align);

#ifdef CELL_GCM_DEBUG

void DebugFinish(struct CellGcmContextData* pContextData);

void* DisableDebugFinish();

void EnableDebugFinish(void* pCellGcmDebugCallback);

#endif

class C_CommandBuffer : public C_CommandChain
{
public:

	C_CommandBuffer();
	~C_CommandBuffer();

	U8* GetStartAddress() const;
	U8* GetEndAddress() const;
	U32 GetStartOffset() const;
	U32 GetEndOffset() const;

	bool Run();

	#if PLATFORM_SPU
	void Flush(CellGcmContextData* pContextData);
	#endif

public:

	U32		m_EndOffset;
	U32		m_Destination;

	#if !CFG_GOLD
	char	m_DebugName[32];
	#endif
};

void BuildJumpToNextCommandBuffer(C_CommandBuffer* pCommandBuffer, const U32 offset);

class STOREALIGN16 C_RingBuffer
{
public:

	C_RingBuffer();

	bool IsCreated() const;

	U32 GetMaxSize() const;
	U32 GetCurSize() const;

	U32 GetStartOffset() const;
	U8* GetStartAddress() const;
	U8* GetAddressFromOffset(const U32 offset) const;

	bool Create(const U32 size, const U32 offset0, const U32 offset1, U8* pAddress0, U8* pAddress1);
	void Destroy();
	U32 Used();

	U32 Alloc(const U32 size);

	void Swap();

public:

	U32					m_CurOffset STOREALIGN16;
	U32*				m_pCurOffset;
	U32					m_CurFrame;
	U32					m_MaxSize;
	U32					m_Offset[2];
	U8*					m_pAddress[2];
	U32					m_AllocSize;
	U32					m_AllocCount;
	U32					m_AllocWasted;
};

template <class T_Context>
struct C_DisplayList
{
	C_DisplayList();

	U32 GetSize() const;

	bool Create(C_CommandChain* pCommandBuffer = NULL, CellGcmContextCallback contextCallback = NULL);
	void Destroy();

	bool Bind();
	void Unbind();
	void Unbind(const U32 jumpToOffset);

	bool						m_Bound;
	bool						m_Created;
	T_Context					m_Context;
	C_ContextData				m_ContextData;
	C_CommandChain*				m_pCommandBuffer;

	#ifdef CELL_GCM_DEBUG
	void*						m_pCellGcmDebugCallback;
	#endif
};

template <class T_Context>
struct C_CommandList : public T_Context
{
	C_CommandList();

	bool Create(C_CommandBuffer* pCommandBuffer = NULL, C_ContextData* pContextData = NULL);
	void Destroy();

	bool Bind();
	void Unbind();

	C_CommandBuffer*			m_pBuffer;

	#ifdef CELL_GCM_DEBUG
	void*						m_pCellGcmDebugCallback;
	#endif
};

} // namespace Display2

#include "Display2/Buffer/GCMBufferSPU.inl"

#endif // DISPLAY2_GCM_BUFFER_SPU_H
