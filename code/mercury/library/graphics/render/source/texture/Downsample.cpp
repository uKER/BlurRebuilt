#include "stdafx.h"
#include "Render/Texture/Downsample.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psDownsample1Tap);
EXTERN_SHADER(psDownsample4Tap);
EXTERN_SHADER(psDownsample16Tap);

namespace Render
{
namespace Texture
{

bool C_DownsampleShaders::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDownsample1Tap", 0, &m_Downsample1TapShader, REF_SHADER(psDownsample1Tap)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDownsample4Tap", 0, &m_Downsample4TapShader, REF_SHADER(psDownsample4Tap)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDownsample16Tap", 0, &m_Downsample16TapShader, REF_SHADER(psDownsample16Tap)))
		return false;

	m_Downsample1TapParameter = m_Downsample1TapShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_Downsample4TapParameter = m_Downsample4TapShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_Downsample16TapParameter = m_Downsample16TapShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");

	return true;
}

void C_DownsampleShaders::Destroy()
{
	Display::Device()->Destroy(&m_Downsample1TapShader);
	Display::Device()->Destroy(&m_Downsample4TapShader);
	Display::Device()->Destroy(&m_Downsample16TapShader);
}

S32 C_Downsample::m_RefCount = 0;
C_DownsampleShaders C_Downsample::m_Shaders;

C_Downsample::C_Downsample()
{
	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_Downsample::~C_Downsample()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_Downsample::Downsample(const Display::C_RenderTexture2D* pDst, const Display::C_Texture2D* pSrc) const
{
	PERF_EVENT("Downsample");

	const U32 iSrcWidth		= pSrc->GetWidth();
	const U32 iSrcHeight	= pSrc->GetHeight();
	const U32 iDstWidth		= pDst->GetTexture().GetWidth();
	const U32 iDstHeight	= pDst->GetTexture().GetHeight();

	const Display::C_PixelShader* pShader = 0;
	const Display::C_PixelShaderParameter* pParameter = 0;
	if (iSrcWidth / iDstWidth == 8)
	{
		pShader = &m_Shaders.m_Downsample16TapShader;
		pParameter = &m_Shaders.m_Downsample16TapParameter;
	}
	if (iSrcWidth / iDstWidth == 4)
	{
		pShader = &m_Shaders.m_Downsample4TapShader;
		pParameter = &m_Shaders.m_Downsample4TapParameter;
	}
	else
	{
		pShader = &m_Shaders.m_Downsample1TapShader;
		pParameter = &m_Shaders.m_Downsample1TapParameter;
	}

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = iDstWidth;
	viewport.m_H = iDstHeight;
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) iSrcWidth, 1.0f / (F32) iSrcHeight, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(*pParameter, (F32*)&vSettings);
	Display::Device()->SetPixelShader(pShader);
	Display::Device()->SetTexture(0, pSrc);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

}; // namespace Texture
}; // namespace Render
