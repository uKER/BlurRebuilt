#ifndef RENDER_DYNAMIC2_DYNAMIC_VERTEX_BUFFER_H
#define RENDER_DYNAMIC2_DYNAMIC_VERTEX_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/VertexBuffer.h"
#include "Display/Buffer/IndexBuffer.h"

namespace Display
{
	class C_Context;
}

namespace DynamicEffect
{

class C_DynamicVertexBufferBlock
{
public:
	C_DynamicVertexBufferBlock() : pVertices(NULL), pIndices(NULL), blockIndex(0xFFFFFFFF) {}
	~C_DynamicVertexBufferBlock() {}
	void*	pVertices;
	void*	pIndices;

	U32		blockIndex;
};

class C_VertexBuffer : public Display::C_VertexBuffer
{
	friend class C_DynamicVertexBuffer;
public:
	C_VertexBuffer() : frame(0), Display::C_VertexBuffer() {}
	~C_VertexBuffer() {}

private:
	void*		pVertices;
	U32			locks;
	U32			frame;
};

class C_IndexBuffer : public Display::C_IndexBuffer
{
	friend class C_DynamicVertexBuffer;
private:
	void*		pIndices;
	U32			locks;
};

class C_DynamicVertexBuffer
{
public:

	C_DynamicVertexBuffer();
	~C_DynamicVertexBuffer();

	void SetVertexBufferSize(U32 bufferSize, U32 bufferCount);
	void SetIndexBufferSize(U32 bufferSize, U32 bufferCount);
	void ReserveBlocks(U32 blocks);
	
	bool OnCreateVolatile();
	void OnDestroyVolatile();

	void Begin();
	
	C_DynamicVertexBufferBlock	Begin(const Util::C_String128& debugName, const U32 vertexSize, const U32 vertexCount, const U32 indexCount = 0xFFFFFFFF);
	
	void						End(C_DynamicVertexBufferBlock block);
	void						End(U32 blockIndex);
	void						SwitchVertexBuffer();
	
	// draw non-indexed primitives from dynamic vertices
	void						DrawPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const C_DynamicVertexBufferBlock block) const;
	void						DrawPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const U32 blockIndex) const;
	
	// draw indexed primitives from dynamic vertices and dynamic indices
	void						DrawIndexedPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const C_DynamicVertexBufferBlock block) const;
	void						DrawIndexedPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const U32 blockIndex) const;

	// draw indexed primitives from dynamic vertices and static indices
	void						DrawIndexedPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const C_DynamicVertexBufferBlock block, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexCount) const;
	void						DrawIndexedPrimitiveBlock(Display::C_Context* pContext, const Display::E_PrimitiveType primitiveType, const U32 blockIndex, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexCount) const;

private:

	struct C_VertexBlock
	{
		U32						vertexSize;
		U32						vertexCount;
		U32						vertexBufferOffset;
		U32						vertexBufferIndex;

		U32						indexCount;
		U32						indexBufferOffset;
		U32						indexBufferIndex;

		bool					bInUse;
	#ifndef CFG_SPEED
		Util::C_String128		debugName;
	#endif
	};


	void						SwitchIndexBuffer();
	
	U32							m_VertexBufferSize;
	U32							m_VertexBufferCount;

	U32							m_IndexBufferSize;
	U32							m_IndexBufferCount;

	U32							m_BlockCount;
	U32							m_BeginBlocks;
	

	U32							m_CurrentVertexOffset;
	U32							m_CurrentVertexBuffer;
	U32							m_BeginVertexBuffer;
	U32							m_CurrentIndexOffset;
	U32							m_CurrentIndexBuffer;
	U32							m_BeginIndexBuffer;
	U32							m_CurrentFrame;
	
	U32							m_CurrentBlock;
	void*						m_pVertexPointer;

	Ut::Vector<C_VertexBuffer>	m_VertexBuffers;
	Ut::Vector<C_IndexBuffer>	m_IndexBuffers;
	Ut::Vector<C_VertexBlock>	m_Blocks;
	bool						m_BlocksGrow;
	bool						m_BufferWarning;

	bool						m_Created;
};

}; // namespace DynamicEffect

#include "Dynamic/DynamicVertexBuffer.inl"

#endif // DISPLAY_BUFFER_DYNAMIC_VB_H
