#ifndef DISPLAY_BUFFER_GCM_GCM_VERTEX_BUFFER_PATCHED_H
#define DISPLAY_BUFFER_GCM_GCM_VERTEX_BUFFER_PATCHED_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/GCM/GCMVertexBuffer.h"
#include "Display/Buffer/GCM/GCMVertexDeclaration.h"
#include "Display/Buffer/GCM/GCMCommandBuffer.h"

namespace Display
{
namespace GCM
{

//! C_VertexBufferPatched

class C_VertexBufferPatched : public C_VertexBuffer
{
public:

	C_VertexBufferPatched();
	~C_VertexBufferPatched();

	const bool IsCommandBufferCreated() const;
	
	void SetCommandBuffer() const;
	
	bool CreateCommandBuffer(const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration);
	void DestroyCommandBuffer();

public:

	Display::C_VertexBuffer*	m_pVertexBuffer;

private:

	C_CommandBuffer				m_CommandBuffer;
};

}; // namespace GCM
}; // namespace Display

#include "Display/Buffer/GCM/GCMVertexBufferPatched.inl"

#endif // DISPLAY_BUFFER_GCM_GCM_VERTEX_BUFFER_PATCHED_H
