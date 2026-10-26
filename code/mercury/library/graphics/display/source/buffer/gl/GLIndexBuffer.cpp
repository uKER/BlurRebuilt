#include "stdafx.h"
#include "Display/Buffer/GL/GLIndexBuffer.h"

namespace NewDisplay
{
namespace GL
{

C_IndexBuffer::C_IndexBuffer() : m_Format(E_Format_Index16), m_Name(0)
{
}

C_IndexBuffer::~C_IndexBuffer()
{
	assert(m_Name == 0 && "The gl index buffer has not been destroyed");
}

void* C_IndexBuffer::Lock(const U32 offset, const U32 size)
{
	assert(offset == 0 && "Cannot specify an offset when locking the index buffer!");
	//assert(size == 0 && "Cannot specify a size when locking the index buffer!");
	assert(m_Name != 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Name);
	return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);	
}

void* C_IndexBuffer::Lock(const U32 size)
{
	return Lock(0, size);
}

void* C_IndexBuffer::Lock()
{
	return Lock(0, 0);
}

void C_IndexBuffer::Unlock()
{
	assert(m_Name != 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_Name);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

}; // namespace GL
}; // namespace NewDisplay
