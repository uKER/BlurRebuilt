#ifndef DISPLAY_BUFFER_DYNAMIC_VERTEX_BUFFER_H
#define DISPLAY_BUFFER_DYNAMIC_VERTEX_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/VertexDeclaration.h"
#include "Display/Buffer/VertexBuffer.h"

namespace Display
{

//! C_DynamicVertexBuffer

class C_DynamicVertexBuffer
{
public:

	C_DynamicVertexBuffer();
	~C_DynamicVertexBuffer();

	const bool IsCreated() const;

	const U32 GetSize() const;
	const U32 GetOffset() const;
	const U32 GetVertexSize() const;
	const U32 GetVertexCount() const;
	const U32 GetVertexStride() const;
	const E_PrimitiveType GetPrimitiveType() const;
	C_VertexBuffer* GetVertexBuffer();
	C_VertexDeclaration* GetVertexDeclaration();
	
	bool Create(const U32 size);
	void Destroy();

	void* Begin(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration);
    void End();

	void Swap();

private:

	bool					m_Locked;
	U32						m_Size;
	U32						m_Offset;
	U32						m_VertexSize;
	U32						m_VertexCount;
	U32						m_VertexStride;
	U32						m_VertexBuffer;
	E_PrimitiveType			m_PrimitiveType;
	C_VertexBuffer			m_VertexBuffers[2];
	C_VertexDeclaration*	m_pVertexDeclaration;
};

}; // namespace Display

#include "Display/Buffer/DynamicVertexBuffer.inl"

#endif // DISPLAY_BUFFER_DYNAMIC_VERTEX_BUFFER_H
