#ifndef RENDER_TEXTURE_BOX_BLUR_H
#define RENDER_TEXTURE_BOX_BLUR_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Texture
{

enum E_BoxBlurShader
{
	E_BoxBlurShader_5 = 0,
	E_BoxBlurShader_7,
	E_BoxBlurShader_9,
	E_BoxBlurShader_11,
	E_BoxBlurShader_13,
	E_BoxBlurShader_Count
};

struct C_BoxBlurShaders
{
	bool Create();
	void Destroy();

	Display::C_PixelShader	m_BoxBlurShader[E_BoxBlurShader_Count];
	Display::C_PixelShader	m_BilinearBoxBlurXShader[E_BoxBlurShader_Count];
	Display::C_PixelShader	m_BilinearBoxBlurYShader[E_BoxBlurShader_Count];
	Display::C_PixelShader	m_SimpleBoxBlurXShader[E_BoxBlurShader_Count];
	Display::C_PixelShader	m_SimpleBoxBlurYShader[E_BoxBlurShader_Count];

	Display::C_PixelShaderParameter	m_BoxBlurSettingsParameter[E_BoxBlurShader_Count];

	Display::C_PixelShaderParameter	m_BilinearBoxBlurXSettingsParameter[E_BoxBlurShader_Count];
	Display::C_PixelShaderParameter	m_BilinearBoxBlurYSettingsParameter[E_BoxBlurShader_Count];

	Display::C_PixelShaderParameter	m_SimpleBoxBlurXSettingsParameter[E_BoxBlurShader_Count];
	Display::C_PixelShaderParameter	m_SimpleBoxBlurYSettingsParameter[E_BoxBlurShader_Count];
};

class C_BoxBlur
{
public:

	C_BoxBlur();
	~C_BoxBlur();

	void	BoxBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader, const F32 fScale = 1.0f, const F32 fDeviation = 1.0f) const;
	void	BoxBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader, const F32 fScale = 1.0f, const F32 fDeviation = 1.0f) const;
	void	BilinearBoxBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const;
	void	BilinearBoxBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const;
	void	SimpleBoxBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const;
	void	SimpleBoxBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const;

private:

	static S32				m_RefCount;
	static C_BoxBlurShaders	m_Shaders;

};

}; // namespace Texture
}; // namespace Render

#endif // RENDER_TEXTURE_BOX_BLUR_H
