#ifndef RENDER_RENDER_CUBE_TEXTURE_RENDER_H
#define RENDER_RENDER_CUBE_TEXTURE_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Render/CubeTextureInterface.h"

namespace Render
{

class C_CubeTextureRender : public I_CubeTexture, public Display::I_Renderer
{
public:

	C_CubeTextureRender(const U32 width = 256, Display::E_Multisample multiSample = Display::E_Multisample_None);
	~C_CubeTextureRender();

	const Display::C_TextureCube* GetTexture();
	
	bool OnCreateVolatile();
	void OnDestroyVolatile();

	void Bind(const Display::E_CubeTextureFace face);
	void Unbind(const Display::E_CubeTextureFace face);

	const Display::C_Surface&		GetSurface(const Display::E_CubeTextureFace face) const;
	const Display::C_Surface&		GetDepthStencilSurface() const;

	void DebugRender();

private:
	U32								m_Width;
	Display::E_Multisample			m_MultiSample;

	Display::C_RenderTextureCube	m_RenderTexture;
	Display::C_Surface				m_DepthStencilSurface;
};

INLINE const Display::C_Surface& C_CubeTextureRender::GetSurface(const Display::E_CubeTextureFace face) const
{
	return m_RenderTexture.GetSurface(face);
}

INLINE const Display::C_Surface& C_CubeTextureRender::GetDepthStencilSurface() const
{
	return m_DepthStencilSurface;
}

} // namespace Render

#endif // RENDER_RENDER_CUBE_TEXTURE_RENDER_H
