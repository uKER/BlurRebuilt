#ifndef RENDER_TEXTURE_GAUSSIAN_BLUR_H
#define RENDER_TEXTURE_GAUSSIAN_BLUR_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Texture
{

enum E_GaussianBlurShader
{
	E_GaussianBlurShader_5 = 0,
	E_GaussianBlurShader_7,
	E_GaussianBlurShader_9,
	E_GaussianBlurShader_11,
	E_GaussianBlurShader_13,
	E_GaussianBlurShader_Count
};

struct C_GaussianBlurShaders
{
	bool Create();
	void Destroy();

	Display::C_PixelShader	m_GaussBlurShader[E_GaussianBlurShader_Count];
	Display::C_PixelShader	m_BilinearGaussBlurXShader[E_GaussianBlurShader_Count];
	Display::C_PixelShader	m_BilinearGaussBlurYShader[E_GaussianBlurShader_Count];
	Display::C_PixelShader	m_SimpleGaussBlurXShader[E_GaussianBlurShader_Count];
	Display::C_PixelShader	m_SimpleGaussBlurYShader[E_GaussianBlurShader_Count];

	Display::C_PixelShaderParameter	m_GaussBlurSettingsParameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_GaussBlurWeights0Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_GaussBlurWeights1Parameter[E_GaussianBlurShader_Count];

	Display::C_PixelShaderParameter	m_BilinearGaussBlurXSettingsParameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_BilinearGaussBlurXWeights0Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_BilinearGaussBlurXWeights1Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_BilinearGaussBlurYSettingsParameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_BilinearGaussBlurYWeights0Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_BilinearGaussBlurYWeights1Parameter[E_GaussianBlurShader_Count];

	Display::C_PixelShaderParameter	m_SimpleGaussBlurXSettingsParameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_SimpleGaussBlurXWeights0Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_SimpleGaussBlurXWeights1Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_SimpleGaussBlurYSettingsParameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_SimpleGaussBlurYWeights0Parameter[E_GaussianBlurShader_Count];
	Display::C_PixelShaderParameter	m_SimpleGaussBlurYWeights1Parameter[E_GaussianBlurShader_Count];
};

class C_GaussianBlur
{
public:

	C_GaussianBlur();
	~C_GaussianBlur();

	void	GaussBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader, const F32 fScale = 1.0f, const F32 fDeviation = 1.0f) const;
	void	GaussBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader, const F32 fScale = 1.0f, const F32 fDeviation = 1.0f) const;
	void	BilinearGaussBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const;
	void	BilinearGaussBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const;
	void	SimpleGaussBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const;
	void	SimpleGaussBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const;

private:

	void	GetGaussianWeights(const U32 nSamples, F32* fWeights) const;
	void	GetGaussianWeights(const U32 nSamples, const F32 fDeviation, F32* fWeights) const;
	void	GetBilinearGaussianWeights(const U32 nSamples, F32* fWeights) const;
	U32		GetGaussianSamples(const E_GaussianBlurShader eShader) const;

private:

	static S32						m_RefCount;
	static C_GaussianBlurShaders	m_Shaders;

};

}; // namespace Texture
}; // namespace Render

#endif // RENDER_TEXTURE_GAUSSIAN_BLUR_H
