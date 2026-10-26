#include "stdafx.h"
#include "Display/Buffer/GL/GLVertexBuffer.h"

namespace NewDisplay
{
namespace GL
{

C_VertexBuffer::C_VertexBuffer() : m_Stride(0), m_Name(0)
{
}

C_VertexBuffer::~C_VertexBuffer()
{
	assert(m_Name == 0 && "The gl vertex buffer has not been destroyed");
}

void* C_VertexBuffer::Lock(const U32 offset, const U32 size)
{
	assert(offset == 0 && "Cannot specify an offset when locking the vertex buffer!");
	//assert(size == 0 && "Cannot specify a size when locking the vertex buffer!");
	assert(m_Name != 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Name);
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void* C_VertexBuffer::Lock(const U32 size)
{
	return Lock(0, size);
}

void* C_VertexBuffer::Lock()
{
	return Lock(0, 0);
}

void C_VertexBuffer::Unlock()
{
	assert(m_Name != 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_Name);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

}; // namespace GL
}; // namespace NewDisplay
