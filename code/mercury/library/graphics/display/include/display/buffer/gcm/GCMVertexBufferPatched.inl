#ifndef DISPLAY_BUFFER_GCM_GCM_VERTEX_BUFFER_PATCHED_INL
#define DISPLAY_BUFFER_GCM_GCM_VERTEX_BUFFER_PATCHED_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

INLINE const bool C_VertexBufferPatched::IsCommandBufferCreated() const
{
	return m_CommandBuffer.IsCreated();
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_BUFFER_GCM_GCM_VERTEX_BUFFER_PATCHED_INL
