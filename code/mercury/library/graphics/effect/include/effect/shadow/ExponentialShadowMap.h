#ifndef EFFECT_EXPONENTIAL_SHADOW_MAP_H
#define EFFECT_EXPONENTIAL_SHADOW_MAP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Shadow/ShadowMapUtils.h"

namespace Effect
{

class C_ExponentialShadowMap
{
public:

	C_ExponentialShadowMap();
	~C_ExponentialShadowMap();

	bool Create();
	void Destroy();

	void Bind();
	void Unbind();

	const Display::C_Texture2D* GetTexture();

private:
	
	Display::C_RenderTexture2D	m_ColourRenderTexture;
	Display::C_RenderTexture2D	m_IntermediateRenderTexture;
	Display::C_RenderTexture2D	m_ExponentialRenderTexture;
	Display::C_Surface			m_DepthStencilSurface;

	const Display::C_Surface*	m_pSavedColourSurface;
	const Display::C_Surface*	m_pSavedDepthStencilSurface;
	Display::C_Viewport			m_SavedViewport;

	C_ExponentialFilter			m_ExponentialFilter;

	U32							m_iWidth;
	U32							m_iHeight;

};

INLINE const Display::C_Texture2D* C_ExponentialShadowMap::GetTexture()
{
	return &m_ExponentialRenderTexture.GetTexture();
}

}; // namespace Effect

#endif
