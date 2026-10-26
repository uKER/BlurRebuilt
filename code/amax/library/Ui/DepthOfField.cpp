
#include "stdafx.h"

#include "Ui/DepthOfField.h"

#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psDoFSimple);

namespace UI
{

Debug::Float	g_UIFocalPoint("UI.PostProcess.DepthOfField.FocalPoint", 4.0f, Debug::Range(0.0f, 500.0f));
Debug::Float	g_UIFocalScale("UI.PostProcess.DepthOfField.FocalScale", 2.0f, Debug::Range(0.0f, 250.0f));

C_DepthOfField::C_DepthOfField() 
:	Display::I_Renderer("UIDepthOfField"),
	m_pColourTexture( NULL ),
	m_pDepthTexture( NULL )
{
	m_Width = 1280 / 2;
	m_Height = 720 / 2;
	m_Format = Display::E_Format_A8R8G8B8;
}

C_DepthOfField::~C_DepthOfField()
{
	DestroyRenderer();
}

bool C_DepthOfField::OnCreate()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDOFSimple", 0, &m_PixelShader, REF_SHADER(psDoFSimple)))
		return false;

	m_SettingsParameter	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_DepthParameter	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vDepth");

	return true;
}

bool C_DepthOfField::OnCreateVolatile()
{
	if (!Display::Device()->CreateRenderTexture2D( m_Width, m_Height, 1, 0, m_Format, Display::E_Multisample_None, 0, &m_BlurRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurRenderTexture, "_DepthOfFieldBlurRT");
	#endif

	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, 0, m_Format, Display::E_Multisample_None, 0, &m_TempRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_TempRenderTexture, "_DepthOfFieldTempRT");
	#endif

	return true;
}


void C_DepthOfField::RebuildSampleTexture( const U32 uWidth, const U32 uHeight )
{
	Display::Device()->Destroy(&m_BlurRenderTexture);

	Display::Device()->CreateRenderTexture2D( uWidth, uHeight, 1, 0, m_Format, Display::E_Multisample_None, 0, &m_BlurRenderTexture );

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurRenderTexture, "_DepthOfFieldBlurRT");
	#endif
	
	m_Width = uWidth;
	m_Width = uWidth;
}


void C_DepthOfField::OnDestroy()
{
	Display::Device()->Destroy(&m_PixelShader);
}

void C_DepthOfField::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_TempRenderTexture);
	Display::Device()->Destroy(&m_BlurRenderTexture);
}

void C_DepthOfField::Render()
{
	// Needs setting.
	assert( m_pColourTexture && m_pDepthTexture );

	PERF_EVENT("UI.DepthOfField");

	const Display::C_Surface* pColourSurface = Display::Device()->GetRenderTarget().GetColourSurface();
	const Display::C_Surface* pDepthStencilSurface = Display::Device()->GetRenderTarget().GetDepthStencilSurface();

	Display::Device()->SetBlendMode( Display::E_BlendMode_Solid );

#if 0
	m_DownsampleUtil.Downsample(&m_BlurRenderTexture, m_pColourTexture);
#else	
	Display::Device()->SetColourDepthStencilSurface( &m_BlurRenderTexture.GetSurface(), 0);
	Display::Quad::DrawFullScreen( m_pColourTexture );
	m_BlurRenderTexture.Resolve();
#endif

	m_GaussianBlurUtil.GaussBlurX(&m_TempRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_7);
	m_GaussianBlurUtil.GaussBlurY(&m_BlurRenderTexture, &m_TempRenderTexture, Render::Texture::E_GaussianBlurShader_7);

 	Display::Device()->SetColourDepthStencilSurface( pColourSurface, pDepthStencilSurface );
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Stencil, Maths::RGBA(0, 0, 0, 0));

	Display::Device()->SetTexture(0, m_pColourTexture);
	Display::Device()->SetTexture(1, &m_BlurRenderTexture.GetTexture());
	Display::Device()->SetTexture(2, m_pDepthTexture);

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);

	const Display::C_Float4 vSettings = Display::C_Float4(g_UIFocalPoint, 1.0f / g_UIFocalScale, 0.0f, 0.0f );
	const Display::C_Float4 vDepth = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_DepthParameter, (F32*)&vDepth);
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Device()->SetBlendMode( Display::E_BlendMode_Default );

	Display::Quad::DrawFullScreen();

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);
}

}; // namespace UI
