#ifndef RENDER_TEXTURE_TEXTURE_UTILS_H
#define RENDER_TEXTURE_TEXTURE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Texture
{

struct C_TextureUtilsShaders
{
	void Create();
	void Destroy();

	Display::C_PixelShader			m_psNormalMap;
	Display::C_PixelShader			m_psGradientToNormal;

	Display::C_PixelShaderParameter	m_psNormalMap_Settings;
};

class C_TextureUtils
{
public:

	C_TextureUtils();
	~C_TextureUtils();

	void CopyTexture(Display::C_RenderTexture2D* pDstTexture, Display::C_RenderTexture2D* pSrcTexture);

	void GenerateNormalFromGradient(Display::C_RenderTexture2D* pRenderTarget, const Display::C_Texture2D* pTexture);
	void GenerateNormalMap(Display::C_RenderTexture2D* pRenderTarget, Display::C_Texture2D* pTexture, const F32 fScale = 1.0f); 

	void BuildMipMaps(const Display::C_Texture2D* pTexture, const bool blurMips = false);

private:

	static S32						m_RefCount;
	static C_TextureUtilsShaders	m_Shaders;
};

} // namespace Texture
} // namespace Render

#endif // RENDER_TEXTURE_TEXTURE_UTILS_H
