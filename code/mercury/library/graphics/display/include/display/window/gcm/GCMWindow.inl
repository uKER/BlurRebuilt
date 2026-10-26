#ifndef DISPLAY_WINDOW_GCM_GCM_WINDOW_INL
#define DISPLAY_WINDOW_GCM_GCM_WINDOW_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

INLINE U32 C_Window::GetWidth() const
{
	return m_RenderBuffer.m_Width;
}

INLINE U32 C_Window::GetHeight() const
{
	return m_RenderBuffer.m_Height;
}

INLINE E_Multisample C_Window::GetMultisample() const
{
	return (E_Multisample)m_RenderBuffer.m_Multisample;
}

INLINE E_Format C_Window::GetColourFormat() const
{
	return GetFormatFromSurfaceFormat(m_RenderBuffer.m_ColourSurfaceFormat[0]);
}

INLINE E_Format C_Window::GetDepthStencilFormat() const
{
	return GetFormatFromSurfaceFormat(m_RenderBuffer.m_DepthStencilSurfaceFormat);
}

INLINE C_Surface* C_Window::GetColourSurface(const U32 i)
{
	if(m_pColourSurface)
		return m_pColourSurface;

	BIZ_ASSERT(i < 4);
	return &m_RenderBuffer.m_ColourBuffer[i].GetSurface();
}

INLINE C_Surface* C_Window::GetDepthStencilSurface()
{
	if(m_pDepthStencilSurface)
		return m_pDepthStencilSurface;

	return &m_RenderBuffer.m_DepthStencilBuffer.GetSurface();
}

INLINE C_Texture2D* C_Window::GetColourTexture(const U32 i)
{
	if(m_pColourTexture)
		return m_pColourTexture;

	BIZ_ASSERT(i < 4);
	return &m_RenderBuffer.m_ColourBuffer[i].GetTexture();
}

INLINE C_Texture2D* C_Window::GetDepthStencilTexture()
{
	if(m_pDepthStencilTexture)
		return m_pDepthStencilTexture;

	return &m_RenderBuffer.m_DepthStencilBuffer.GetTexture();
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_WINDOW_GCM_GCM_WINDOW_INL
