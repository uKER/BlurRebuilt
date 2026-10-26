// [Blur1PCDX11] Native D3D11 game facade declarations.
#ifndef DISPLAY_WINDOW_D3D11_D3D11_WINDOW_INL
#define DISPLAY_WINDOW_D3D11_D3D11_WINDOW_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace D3D11
{

#ifdef PLATFORM_PC
INLINE E_Multisample C_Window::GetMultisample() const
{
	return m_Multisample;
}

INLINE C_Surface* C_Window::GetColourSurface(const U32 i)
{
	return &m_ColourRenderTexture.GetSurface();
}

INLINE C_Surface* C_Window::GetDepthStencilSurface()
{
	return &m_DepthStencilSurface;
}

INLINE C_Texture2D* C_Window::GetColourTexture(const U32 i)
{
	return &m_ColourRenderTexture.GetTexture();
}

INLINE const C_Texture2D* C_Window::GetDepthStencilTexture()
{
	return m_pDepthTexture;
}

INLINE C_Texture2D* C_Window::GetMaskTexture()
{
	return &m_MaskRenderTexture.GetTexture();
}
#else
INLINE E_Multisample C_Window::GetMultisample() const
{
	return m_Multisample;
}

INLINE C_Surface* C_Window::GetColourSurface(const U32 i)
{
	return &m_ColourSurfaceWithAA;
}

INLINE C_Surface* C_Window::GetDepthStencilSurface()
{
	return &m_DepthStencilSurfaceWithAA;
}

INLINE C_Texture2D* C_Window::GetColourTexture(const U32 i)
{
	return &m_ColourTexture;
}

INLINE C_Texture2D* C_Window::GetDepthStencilTexture()
{
	return &m_DepthStencilTexture;
}
#endif // PLATFORM_PC

}; // namespace D3D11
}; // namespace Display

#endif // DISPLAY_WINDOW_D3D11_D3D11_WINDOW_INL
