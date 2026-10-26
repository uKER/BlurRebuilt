#ifndef DISPLAY2_GCM_BUFFER_H
#define DISPLAY2_GCM_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Buffer/GCMBufferSPU.h"
#include "Display2/Context/GCMContext.h"

namespace Display2
{

// C_GrowableCommandBuffer

class C_GrowableCommandBuffer
{
public:

	C_GrowableCommandBuffer();

	bool IsCreated() const;

	U32 GetTotalSize() const;
	U32 GetCurrentSize() const;

	bool Create(const U32 size, const U32 align, U8* pAddress0, U8* pAddress1);
	void Destroy();

	U8* Alloc(const U32 size, const U32 align);

	void Swap();

public:
	
	U32		m_CurFrame;
	U8*		m_pCurAddress[2];

private:

	U32		m_Size;
	U8*		m_pAddress[2];
};

extern C_GrowableCommandBuffer g_GrowableCommandBuffer;

//! C_DisplayList

int32_t DisplayListCallback(CellGcmContextData* pContext, uint32_t size);

#if !PLATFORM_SPU
U32 GetVertexShaderCommandBufferSize(CellCgbProgram* pCgbProgram, CellCgbVertexProgramConfiguration* pCgbProgramCondig);

bool BuildVertexShaderCommandBuffer(C_CommandChain* pCommandBuffer, CellCgbProgram* pCgbProgram, CellCgbVertexProgramConfiguration* pCgbProgramCondig);
#endif

} // namespace Display2

#include "Display2/Buffer/GCMBuffer.inl"

#endif // DISPLAY2_GCM_BUFFER_H
