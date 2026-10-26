#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_BUFFERS_INL
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_BUFFERS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

// C_BackBuffer

INLINE const U32 C_BackBuffer::GetWidth() const
{
	return m_RenderBuffer.m_Width;
}

INLINE const U32 C_BackBuffer::GetHeight() const
{
	return m_RenderBuffer.m_Height;
}

INLINE const E_Multisample C_BackBuffer::GetMultisample() const
{
	return (E_Multisample)m_RenderBuffer.m_Multisample;
}

INLINE const E_Format C_BackBuffer::GetColourFormat() const
{
	return GetFormatFromSurfaceFormat(m_RenderBuffer.m_ColourSurfaceFormat[0]);
}

INLINE const C_Surface* C_BackBuffer::GetColourSurface() const
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetSurface();
}

INLINE const C_Texture2D* C_BackBuffer::GetColourTexture() const
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetTexture();
}

INLINE C_Surface* C_BackBuffer::GetColourSurface()
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetSurface();
}

INLINE C_Texture2D* C_BackBuffer::GetColourTexture()
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetTexture();
}

INLINE const E_Format C_BackBuffer::GetDepthStencilFormat() const
{
	return GetFormatFromSurfaceFormat(m_RenderBuffer.m_DepthStencilSurfaceFormat);
}

INLINE const C_Surface* C_BackBuffer::GetDepthStencilSurface() const
{
	return &m_RenderBuffer.m_DepthStencilBuffer.GetSurface();
}

INLINE const C_Texture2D* C_BackBuffer::GetDepthStencilTexture() const
{
	return &m_RenderBuffer.m_DepthStencilBuffer.GetTexture();
}

INLINE C_Surface* C_BackBuffer::GetDepthStencilSurface()
{
	return &m_RenderBuffer.m_DepthStencilBuffer.GetSurface();
}

INLINE C_Texture2D* C_BackBuffer::GetDepthStencilTexture()
{
	return &m_RenderBuffer.m_DepthStencilBuffer.GetTexture();
}

// C_FrontBuffer

INLINE const U32 C_FrontBuffer::GetWidth() const
{
	return m_RenderBuffer.m_Width;
}

INLINE const U32 C_FrontBuffer::GetHeight() const
{
	return m_RenderBuffer.m_Height;
}

INLINE const E_Format C_FrontBuffer::GetFormat() const
{
	return GetFormatFromSurfaceFormat(m_RenderBuffer.m_ColourSurfaceFormat[0]);
}

INLINE const C_Surface* C_FrontBuffer::GetSurface() const
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetSurface();
}

INLINE const C_Texture2D* C_FrontBuffer::GetTexture() const
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetTexture();
}

INLINE C_Surface* C_FrontBuffer::GetSurface()
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetSurface();
}

INLINE C_Texture2D* C_FrontBuffer::GetTexture()
{
	return &m_RenderBuffer.m_ColourBuffer[0].GetTexture();
}

// C_RenderTarget

INLINE const U32 C_RenderTarget::GetWidth() const
{
	return m_pColourSurface ? m_pColourSurface->GetWidth() : m_pDepthStencilSurface ? m_pDepthStencilSurface->GetWidth() : 0;
}

INLINE const U32 C_RenderTarget::GetHeight() const
{
	return m_pColourSurface ? m_pColourSurface->GetHeight() : m_pDepthStencilSurface ? m_pDepthStencilSurface->GetHeight() : 0;
}

INLINE const C_Surface* C_RenderTarget::GetColourSurface() const
{
	return m_pColourSurface;
}

INLINE const C_Surface* C_RenderTarget::GetDepthStencilSurface() const
{
	return m_pDepthStencilSurface;
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_BUFFERS_INL
