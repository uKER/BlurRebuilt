#ifndef EFFECT_SHADOW_MAP_H
#define EFFECT_SHADOW_MAP_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{

class C_ShadowMap
{
public:

	C_ShadowMap();
	~C_ShadowMap();

	bool Create();
	void Destroy();

	void Bind();
	void Unbind();

	const Display::C_Texture2D* GetTexture();

private:

	Display::C_RenderTexture2D	m_ColourRenderTexture;
	Display::C_Surface			m_DepthStencilSurface;

	const Display::C_Surface*	m_pSavedColourSurface;
	const Display::C_Surface*	m_pSavedDepthStencilSurface;
	Display::C_Viewport			m_SavedViewport;

	U32							m_iWidth;
	U32							m_iHeight;

};

INLINE const Display::C_Texture2D* C_ShadowMap::GetTexture()
{
	return &m_ColourRenderTexture.GetTexture();
}

}; // namespace Effect

#endif
