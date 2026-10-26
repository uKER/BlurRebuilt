// [Blur1PCDX11] Native D3D11 game facade declarations.
#ifndef DISPLAY_DEVICE_D3D11_DEVICE_BUFFERS_INL
#define DISPLAY_DEVICE_D3D11_DEVICE_BUFFERS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace D3D11
{

// C_BackBuffer

INLINE const U32 C_BackBuffer::GetWidth() const
{
	return m_Width;
}

INLINE const U32 C_BackBuffer::GetHeight() const
{
	return m_Height;
}

INLINE const E_Multisample C_BackBuffer::GetMultisample() const
{
	return m_Multisample;
}

INLINE const E_Format C_BackBuffer::GetColourFormat() const
{
	return m_ColourFormat;
}

INLINE const E_Format C_BackBuffer::GetDepthStencilFormat() const
{
	return m_DepthStencilFormat;
}

INLINE const C_Surface* C_BackBuffer::GetColourSurface() const
{
	return &m_ColourSurface;
}

INLINE const C_Surface* C_BackBuffer::GetDepthStencilSurface() const
{
	return &m_DepthStencilSurface;
}

// C_FrontBuffer

INLINE const U32 C_FrontBuffer::GetWidth() const
{
	return m_Width;
}

INLINE const U32 C_FrontBuffer::GetHeight() const
{
	return m_Height;
}

INLINE const E_Format C_FrontBuffer::GetFormat() const
{
	return m_Format;
}

INLINE const C_Texture2D* C_FrontBuffer::GetTexture() const
{
	return &m_Texture;
}

// C_RenderTarget

INLINE const U32 C_RenderTarget::GetColourWidth() const
{
	assert(m_pColourSurface);
	return m_pColourSurface->GetWidth();
}

INLINE const U32 C_RenderTarget::GetColourHeight() const
{
	assert(m_pColourSurface);
	return m_pColourSurface->GetHeight();
}

INLINE const E_Format C_RenderTarget::GetColourFormat() const
{
	assert(m_pColourSurface);
	return GetFormatFromSurfaceFormat(m_pColourSurface->GetFormat());
}

INLINE const E_Multisample C_RenderTarget::GetColourMultisample() const
{
	assert(m_pColourSurface);
	return (E_Multisample)m_pColourSurface->GetMultisample();
}
	
INLINE const C_Surface* C_RenderTarget::GetColourSurface() const
{
	return m_pColourSurface;
}

INLINE const U32 C_RenderTarget::GetDepthStencilWidth() const
{
	assert(m_pDepthStencilSurface);
	return m_pDepthStencilSurface->GetWidth();
}

INLINE const U32 C_RenderTarget::GetDepthStencilHeight() const
{
	assert(m_pDepthStencilSurface);
	return m_pDepthStencilSurface->GetHeight();
}

INLINE const E_Format C_RenderTarget::GetDepthStencilFormat() const
{
	assert(m_pDepthStencilSurface);
	return GetFormatFromSurfaceFormat(m_pDepthStencilSurface->GetFormat());
}

INLINE const E_Multisample C_RenderTarget::GetDepthStencilMultisample() const
{
	assert(m_pDepthStencilSurface);
	return (E_Multisample)m_pDepthStencilSurface->GetMultisample();
}

INLINE const C_Surface* C_RenderTarget::GetDepthStencilSurface() const
{
	return m_pDepthStencilSurface;
}

}; // namespace D3D11
}; // namespace Display

#endif // DISPLAY_DEVICE_D3D11_DEVICE_BUFFERS_INL
