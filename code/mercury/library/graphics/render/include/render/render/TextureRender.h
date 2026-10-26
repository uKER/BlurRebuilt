#ifndef RENDER_TEXTURERENDER_H
#define RENDER_TEXTURERENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

class C_TextureRender : public Display::I_Renderer
{
public:

	C_TextureRender(const U32 width = 256, const U32 height = 256, const Display::E_Multisample multisample = Display::E_Multisample_None, const U32 numMips = 1);
	~C_TextureRender();

	bool	OnCreateVolatile();
	void	OnDestroyVolatile();

#if PLATFORM_PC
	// [Blur2PC] Retail PC (Blur.exe 1.2, inlined in 0x00b1fd20 and 0x00b20450): a new size and multisample for
	// the render target, one mip; used the next time the volatile resources are created (device reset).
	void	SetFormat(const U32 width, const U32 height, const Display::E_Multisample multisample);
#endif

	void	Bind() const;
	void	Unbind() const;

	void								DebugRender() const;
	const Display::C_RenderTexture2D&	GetRenderTexture() const;
	const Display::C_Surface&			GetColourSurface() const;
	const Display::C_Surface&			GetDepthStencilSurface() const;

private:

	U32							m_Width;
	U32							m_Height;
	Display::E_Multisample		m_MultiSample;
	U32							m_NumMips;
	Display::C_RenderTexture2D	m_RenderTexture;
	Display::C_Surface			m_DepthStencilSurface;
};

INLINE const Display::C_RenderTexture2D& C_TextureRender::GetRenderTexture() const
{
	return m_RenderTexture;
}

INLINE const Display::C_Surface& C_TextureRender::GetColourSurface() const
{
	return m_RenderTexture.GetSurface();
}

INLINE const Display::C_Surface& C_TextureRender::GetDepthStencilSurface() const
{
	return m_DepthStencilSurface;
}

} // namespace Render

#endif // RENDER_TEXTURERENDER_H
