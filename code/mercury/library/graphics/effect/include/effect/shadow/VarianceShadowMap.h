#ifndef EFFECT_VARIANCE_SHADOW_MAP_H
#define EFFECT_VARIANCE_SHADOW_MAP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Shadow/ShadowMapSurface.h"
#include "Effect/Shadow/ShadowMapUtils.h"

namespace Effect
{

class C_VarianceShadowMap
{
public:

	C_VarianceShadowMap();
	~C_VarianceShadowMap();

	bool Create();
	void Destroy();

	void Bind();
	void Unbind();

	const Display::C_Texture2D* GetTexture();

private:
	
	Display::C_RenderTexture2D	m_ColourRenderTexture;
	Display::C_RenderTexture2D	m_IntermediateRenderTexture;
	Display::C_RenderTexture2D	m_VarianceRenderTexture;
	Display::C_Surface			m_DepthStencilSurface;

	const Display::C_Surface*	m_pSavedColourSurface;
	const Display::C_Surface*	m_pSavedDepthStencilSurface;
	Display::C_Viewport			m_SavedViewport;

	C_VarianceFilter			m_VarianceFilter;

	U32							m_iWidth;
	U32							m_iHeight;

};

INLINE const Display::C_Texture2D* C_VarianceShadowMap::GetTexture()
{
	return &m_VarianceRenderTexture.GetTexture();
}

}; // namespace Effect

#endif // EFFECT_VARIANCE_SHADOW_MAP_H
