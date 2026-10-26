#ifndef RENDER_TEXTURE_RENDERUTILS_INL
#define RENDER_TEXTURE_RENDERUTILS_INL

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Texture
{

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayRGB(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture)
{
	MathsStorage::Float3 vScale(1.0f, 1.0f, 1.0f);
	MathsStorage::Float3 vBias(0.0f, 0.0f, 0.0f);
	DisplayRGBScaleBias(x, y, w, h, pTexture, vScale, vBias);
}

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayRGBScaleBias(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const MathsStorage::Float3& vScale, const MathsStorage::Float3& vBias)
{
	MathsStorage::Float4 vRed(vScale.x, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vGreen(0.0f, vScale.y, 0.0f, 0.0f);
	MathsStorage::Float4 vBlue(0.0f, 0.0f, vScale.z, 0.0f);
	MathsStorage::Float4 vAlpha(0.0f, 0.0f, 0.0f, 1.0f);
	MathsStorage::Float4 vNewBias(vBias.x, vBias.y, vBias.z, 1.0f);
	DisplayColour<Texture>(x, y, w, h, pTexture, vRed, vGreen, vBlue, vAlpha, vNewBias);
}

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayRed(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale, const F32 fBias)
{
	MathsStorage::Float4 vRed(fScale, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vGreen(0.0f, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vBlue(0.0f, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vAlpha(0.0f, 0.0f, 0.0f, 1.0f);
	MathsStorage::Float4 vBias(fBias, 0.0f, 0.0f, 1.0f);
	DisplayColour<Texture>(x, y, w, h, pTexture, vRed, vGreen, vBlue, vAlpha, vBias);
}

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayGreen(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale, const F32 fBias)
{
	MathsStorage::Float4 vRed(0.0f, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vGreen(0.0f, fScale, 0.0f, 0.0f);
	MathsStorage::Float4 vBlue(0.0f, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vAlpha(0.0f, 0.0f, 0.0f, 1.0f);
	MathsStorage::Float4 vBias(0.0f, fBias, 0.0f, 1.0f);
	DisplayColour<Texture>(x, y, w, h, pTexture, vRed, vGreen, vBlue, vAlpha, vBias);
}

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayBlue(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale, const F32 fBias)
{
	MathsStorage::Float4 vRed(0.0f, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vGreen(0.0f, 0.0f, 0.0f, 0.0f);
	MathsStorage::Float4 vBlue(0.0f, 0.0f, fScale, 0.0f);
	MathsStorage::Float4 vAlpha(0.0f, 0.0f, 0.0f, 1.0f);
	MathsStorage::Float4 vBias(0.0f, 0.0f, fBias, 1.0f);
	DisplayColour<Texture>(x, y, w, h, pTexture, vRed, vGreen, vBlue, vAlpha, vBias);
}

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayAlpha(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale, const F32 fBias)
{
	MathsStorage::Float4 vRed(0.0f, 0.0f, 0.0f, fScale);
	MathsStorage::Float4 vGreen(0.0f, 0.0f, 0.0f, fScale);
	MathsStorage::Float4 vBlue(0.0f, 0.0f, 0.0f, fScale);
	MathsStorage::Float4 vAlpha(0.0f, 0.0f, 0.0f, 1.0f);
	MathsStorage::Float4 vBias(fBias, fBias, fBias, 1.0f);
	DisplayColour<Texture>(x, y, w, h, pTexture, vRed, vGreen, vBlue, vAlpha, vBias);
}

template<typename Texture>
FORCE_INLINE void C_RenderUtils::DisplayColour(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4 &vAlpha, const MathsStorage::Float4& vBias)
{
	BIZ_STATIC_ASSERT(false);
}

template<>
FORCE_INLINE void C_RenderUtils::DisplayColour<Display::C_Texture2D>(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4 &vAlpha, const MathsStorage::Float4& vBias)
{
	DisplayTexture2DColour(x, y, w, h, pTexture, vRed, vGreen, vBlue, vAlpha, vBias);
}

}; // namespace Texture
}; // namespace Render

#endif // RENDER_TEXTURE_RENDERUTILS_INL
