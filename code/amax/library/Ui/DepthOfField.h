
#ifndef UI_DEPTH_OF_FIELD_H
#define UI_DEPTH_OF_FIELD_H

#include "Render/Texture/Downsample.h"
#include "Render/Texture/GaussianBlur.h"

namespace UI
{

class C_DepthOfField : public Display::I_Renderer
{
public:
	C_DepthOfField();
	~C_DepthOfField();

	bool 	OnCreate();
	bool 	OnCreateVolatile();
	void 	OnDestroy();
	void 	OnDestroyVolatile();

	void 	Render();

	void 	Setup( const U32 uWidth, const U32 uHeight, const Display::E_Format eFormat )
			{
				m_Width = uWidth;
				m_Height = uHeight;
				m_Format = eFormat;
			}

	void	RebuildSampleTexture( const U32 uWidth, const U32 uHeight );

	void	SetColourDepthStencilBuffers( const Display::C_RenderTexture2D* pColourTexture, const Display::C_Texture2D* pDepthTexture )
			{
				m_pColourTexture = &pColourTexture->GetTexture();
				m_pDepthTexture = pDepthTexture;
			}

private:
	const Display::C_Texture2D*			m_pColourTexture;
	const Display::C_Texture2D*			m_pDepthTexture;

	Display::C_RenderTexture2D			m_BlurRenderTexture;
	Display::C_RenderTexture2D			m_TempRenderTexture;

	Display::C_PixelShader				m_PixelShader;
	Display::C_PixelShaderParameter		m_SettingsParameter;
	Display::C_PixelShaderParameter		m_DepthParameter;

	Render::Texture::C_Downsample		m_DownsampleUtil;
	Render::Texture::C_GaussianBlur		m_GaussianBlurUtil;

	U32									m_Width;
	U32									m_Height;
	Display::E_Format					m_Format;
};

} // namespace UI

#endif // UI_DEPTH_OF_FIELD_H