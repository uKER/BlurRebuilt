#ifndef DISPLAY_BUFFER_DYNAMIC_VERTEX_BUFFER_INL
#define DISPLAY_BUFFER_DYNAMIC_VERTEX_BUFFER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE const bool C_DynamicVertexBuffer::IsCreated() const
{
	return m_VertexBuffers[0].IsCreated() && m_VertexBuffers[1].IsCreated();
}

INLINE const U32 C_DynamicVertexBuffer::GetSize() const
{
	return m_Size;
}

INLINE const U32 C_DynamicVertexBuffer::GetOffset() const
{
	return m_Offset;
}

INLINE const U32 C_DynamicVertexBuffer::GetVertexSize() const
{
	return m_VertexSize;
}

INLINE const U32 C_DynamicVertexBuffer::GetVertexCount() const
{
	return m_VertexCount;
}

INLINE const U32 C_DynamicVertexBuffer::GetVertexStride() const
{
	return m_VertexStride;
}

INLINE const E_PrimitiveType C_DynamicVertexBuffer::GetPrimitiveType() const
{
	return m_PrimitiveType;
}

INLINE C_VertexBuffer* C_DynamicVertexBuffer::GetVertexBuffer()
{
	assert(m_VertexBuffer < 2);
	return &m_VertexBuffers[m_VertexBuffer];
}

INLINE C_VertexDeclaration* C_DynamicVertexBuffer::GetVertexDeclaration()
{
	assert(m_pVertexDeclaration);
	return m_pVertexDeclaration;
}

}; // namespace Display

#endif // DISPLAY_BUFFER_DYNAMIC_VERTEX_BUFFER_INL
