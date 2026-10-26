#ifndef RENDER_TEXTURE_RENDERUTILS_H
#define RENDER_TEXTURE_RENDERUTILS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Texture
{

enum E_CubeMapRender
{
	E_CubeMapRender_Face = 0,
	E_CubeMapRender_All = 1,
	E_CubeMapRender_Count
};

struct C_RenderUtilsShaders
{
	void Create();
	void Destroy();

	Display::C_PixelShader			m_Colour2DPixelShader;
	Display::C_PixelShader			m_Colour3DPixelShader;
	Display::C_PixelShader			m_ColourCubePixelShader;
	Display::C_PixelShader			m_Depth2DPixelShader;
	
	Display::C_PixelShaderParameter	m_Colour2DParameterRed;
	Display::C_PixelShaderParameter	m_Colour2DParameterGreen;
	Display::C_PixelShaderParameter	m_Colour2DParameterBlue;
	Display::C_PixelShaderParameter	m_Colour2DParameterAlpha;
	Display::C_PixelShaderParameter	m_Colour2DParameterBias;
	
	Display::C_PixelShaderParameter	m_Colour3DParameterRed;
	Display::C_PixelShaderParameter	m_Colour3DParameterGreen;
	Display::C_PixelShaderParameter	m_Colour3DParameterBlue;
	Display::C_PixelShaderParameter	m_Colour3DParameterAlpha;
	Display::C_PixelShaderParameter	m_Colour3DParameterBias;
	Display::C_PixelShaderParameter	m_Colour3DParameterSlice;
	
	Display::C_PixelShaderParameter	m_ColourCubeParameterRed;
	Display::C_PixelShaderParameter	m_ColourCubeParameterGreen;
	Display::C_PixelShaderParameter	m_ColourCubeParameterBlue;
	Display::C_PixelShaderParameter	m_ColourCubeParameterAlpha;
	Display::C_PixelShaderParameter	m_ColourCubeParameterBias;
	Display::C_PixelShaderParameter	m_ColourCubeParameterTopLeft;
	Display::C_PixelShaderParameter	m_ColourCubeParameterTopRight;
	Display::C_PixelShaderParameter	m_ColourCubeParameterBottomLeft;
	Display::C_PixelShaderParameter	m_ColourCubeParameterBottomRight;

	Display::C_PixelShaderParameter	m_Depth2DParameterScale;
};

class C_RenderUtils
{
public:

	C_RenderUtils();
	~C_RenderUtils();

	// generic texture functions
	template<typename Texture> void DisplayRGB(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture);
	template<typename Texture> void DisplayRGBScaleBias(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const MathsStorage::Float3& vScale, const MathsStorage::Float3& vBias);
	template<typename Texture> void DisplayRed(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale = 1.0f, const F32 fBias = 0.0f);
	template<typename Texture> void DisplayGreen(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale = 1.0f, const F32 fBias = 0.0f);
	template<typename Texture> void DisplayBlue(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale = 1.0f, const F32 fBias = 0.0f);
	template<typename Texture> void DisplayAlpha(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const F32 fScale = 1.0f, const F32 fBias = 0.0f);

	template<typename Texture> void DisplayColour(const F32 x, const F32 y, const F32 w, const F32 h, const Texture* pTexture, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4 &vAlpha, const MathsStorage::Float4& vBias);

	// depth texture functions
	void DisplayTexture2DDepth(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture2D);

	// base functions
	void DisplayTexture2DColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture2D, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias);
	void DisplayTexture3DColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture3D* pTexture3D, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias);
	void DisplayTextureCubeColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias);
	void DisplayTextureCubeAllColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias);
	void DisplayTextureCubeFaceColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias);
	void DisplayTextureCubeColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vTopLeft, const MathsStorage::Float4& vTopRight, const MathsStorage::Float4& vBottomLeft, const MathsStorage::Float4& vBottomRight, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias);
	void DisplayTexture2DDepth(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture2D, const MathsStorage::Float4& vScale);

private:

	void DrawTextured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture* pTexture, Display::C_PixelShader* pPixelShader);

private:
	
	static C_RenderUtilsShaders	m_Shaders;
	static S32					m_RefCount;

};

}; // namespace Texture
}; // namespace Render

#include "Render/Texture/RenderUtils.inl"

#endif // RENDER_TEXTURE_RENDERUTILS_H
