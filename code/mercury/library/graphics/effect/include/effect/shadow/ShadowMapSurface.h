#ifndef EFFECT_SHADOW_MAP_SURFACE_H
#define EFFECT_SHADOW_MAP_SURFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{

#if PLATFORM_PS3
bool PS3CreateRenderTextureFromSurface(const Display::C_Surface* pSurface, Display::C_RenderTexture2D* pRenderTexture);
bool PS3CreateDepthTextureFromSurface(const Display::C_Surface* pSurface, Display::C_RenderTexture2D* pRenderTexture);
#endif

}; // namespace Effect

#endif // EFFECT_SHADOW_MAP_SURFACE_H
