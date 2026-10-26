#ifndef DISPLAY_BUFFER_DYNAMIC_VB_H
#define DISPLAY_BUFFER_DYNAMIC_VB_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/VertexBuffer.h"
#include "Display/Buffer/IndexBuffer.h"
#include "Display/Utils/Renderer.h"

#include "core/utility/singleton.h"

namespace Display
{

class C_DynamicVB : public I_Renderer, public Ut::Singleton<C_DynamicVB, Ut::CreateExternal>
{
public:

	C_DynamicVB();
	~C_DynamicVB();

	void SetBufferSize(U32 bufferSize, U32 bufferCount);
	
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void SwitchBuffer();	// for switching to next buffer
	void Begin();
	void End();

	void* BeginBlock(const char* debugName, const U32 vertexSize, const U32 vertexCount);
	U32 EndBlock(void* pPointer);

	void DrawBlock(const E_PrimitiveType primitiveType, const U32 blockIndex);
	void DrawIndexedBlock(const E_PrimitiveType primitiveType, const U32 blockIndex, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexCount);

private:

	struct C_VertexBlock
	{
		U32				m_VertexSize;
		U32				m_VertexCount;
		U32				m_BufferOffset;
		U32				m_BufferIndex;
	#ifndef CFG_SPEED
		char			m_DebugName[12];
	#endif
	};
	
	U32							m_BufferSize;
	U32							m_BufferCount;
	U32							m_BlockCount;
	bool						m_Created;
	bool						m_BeginBlock;

	U32							m_CurrentOffset;
	U32							m_CurrentBuffer;
	U32							m_CurrentBlock;
	void*						m_pCurrentPointer;
	void*						m_pStartPointer;
	Ut::Vector<C_VertexBuffer>	m_Buffers;
	Ut::Vector<C_VertexBlock>	m_Blocks;
};

}; // namespace Display

#include "Display/Buffer/DynamicVB.inl"

#endif // DISPLAY_BUFFER_DYNAMIC_VB_H
