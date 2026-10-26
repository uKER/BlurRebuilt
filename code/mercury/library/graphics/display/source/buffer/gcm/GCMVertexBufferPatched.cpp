#include "stdafx.h"
#include "Display/Buffer/GCM/GCMVertexBufferPatched.h"
#include "Display/Buffer/GCM/GCMCommandBufferBuilder.h"

namespace Display
{
namespace GCM
{

C_VertexBufferPatched::C_VertexBufferPatched() :
	m_pVertexBuffer(NULL)
{
}

C_VertexBufferPatched::~C_VertexBufferPatched()
{
	DestroyCommandBuffer();
}

void C_VertexBufferPatched::SetCommandBuffer() const
{
	assert(m_CommandBuffer.IsCreated());
	m_CommandBuffer.Run();
}

bool C_VertexBufferPatched::CreateCommandBuffer(const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration)
{
	assert(vertexStride > 0);
	assert(pVertexDeclaration);
	assert(!m_CommandBuffer.IsCreated());
	
	C_VertexBuffer* pVertexBuffer = m_pVertexBuffer ? m_pVertexBuffer : this;

	Display::C_MeasureCommandBuffer measureCommandBuffer;
	measureCommandBuffer.Bind();
	#ifdef USE_GCM_MEASURE
	Display::GCM::GCMCreateCommandBufferSetVertexDeclaration(measureCommandBuffer.m_gcmContext, vertexStride, pVertexBuffer, pVertexDeclaration);
	#else
	return false;
	#endif
	U32 size = measureCommandBuffer.Unbind();

	size += sizeof(U32); // Add extra 1 word for command size so that callback won't be called at last command creation.

	m_CommandBuffer.Create(size);

	m_CommandBuffer.Bind();
	Display::GCM::GCMCreateCommandBufferSetVertexDeclaration(m_CommandBuffer.m_gcmContext, vertexStride, pVertexBuffer, pVertexDeclaration);
	m_CommandBuffer.Unbind();

	return true;
}

void C_VertexBufferPatched::DestroyCommandBuffer()
{
	m_CommandBuffer.Destroy();
}

}; // namespace GCM
}; // namespace Display
