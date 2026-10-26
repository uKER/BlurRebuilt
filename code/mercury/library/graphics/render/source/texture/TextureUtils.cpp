#include "stdafx.h"
#include "Render/Texture/TextureUtils.h"
#include "Render/Shaders/Constants.h"
#include "Render/Utils/DebugLog.h"

EXTERN_SHADER(psNormalMap);
EXTERN_SHADER(psGradientToNormal);

namespace Render
{
namespace Texture
{

void C_TextureUtilsShaders::Create()
{
	RESOURCE_MEMORY_ID("TextureUtils");

	Display::CreatePixelShaderFromFunc("Shaders/psNormalMap", 0, &m_psNormalMap, REF_SHADER(psNormalMap));
	Display::CreatePixelShaderFromFunc("Shaders/psGradientToNormal", 0, &m_psGradientToNormal, REF_SHADER(psGradientToNormal));

	m_psNormalMap_Settings = m_psNormalMap.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
}

void C_TextureUtilsShaders::Destroy()
{
	RESOURCE_MEMORY_ID("TextureUtils");

	Display::Device()->Destroy(&m_psGradientToNormal);
	Display::Device()->Destroy(&m_psNormalMap);
}

S32 C_TextureUtils::m_RefCount = 0;
C_TextureUtilsShaders C_TextureUtils::m_Shaders;

C_TextureUtils::C_TextureUtils()
{
	m_RefCount++;

	if(m_RefCount == 1)
	{
		m_Shaders.Create();
	}
}

C_TextureUtils::~C_TextureUtils()
{
	m_RefCount--;

	if(m_RefCount == 0)
	{
		m_Shaders.Destroy();
	}
}

void C_TextureUtils::CopyTexture(Display::C_RenderTexture2D* pDstTexture, Display::C_RenderTexture2D* pSrcTexture)
{
	PERF_EVENT("CopyTexture");

	if(!pDstTexture || !pSrcTexture)
		return;

	Display::Device()->SetColourDepthStencilSurface(&pDstTexture->GetSurface(), NULL);

	Display::C_Viewport viewport;
	viewport.m_W = pDstTexture->GetTexture().GetWidth();
	viewport.m_H = pDstTexture->GetTexture().GetHeight();
	Display::Device()->SetViewport(viewport);

	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShader);
	Display::Device()->SetPixelShader(&Display::PixelShader::m_PixelShaderTexture);
	
	Display::Device()->SetTexture(0, &pSrcTexture->GetTexture());
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);

	Display::Quad::DrawFullScreen();


	Display::Device()->SetColourDepthStencilSurface();
}

void C_TextureUtils::GenerateNormalFromGradient(Display::C_RenderTexture2D* pRenderTarget, const Display::C_Texture2D* pTexture)
{
	PERF_EVENT("GenerateNormalFromGradient");

	if(!pRenderTarget || !pTexture || !pTexture->IsCreated())
		return;

	Display::Device()->SetColourDepthStencilSurface(&pRenderTarget->GetSurface(), NULL);

	Display::C_Viewport viewport;
	viewport.m_W = pTexture->GetWidth();
	viewport.m_H = pTexture->GetHeight();
	Display::Device()->SetViewport(viewport);

	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShader);
	Display::Device()->SetPixelShader(&m_Shaders.m_psGradientToNormal);

	Display::Device()->SetTexture(0, pTexture);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);

	Display::Quad::DrawFullScreen();

}

void C_TextureUtils::GenerateNormalMap(Display::C_RenderTexture2D* pRenderTarget, Display::C_Texture2D* pTexture, const F32 fSlopeScale)
{
	PERF_EVENT("GenerateNormalMap");

	if(!pRenderTarget || !pTexture || !pTexture->IsCreated())
		return;

	Display::Device()->SetColourDepthStencilSurface(&pRenderTarget->GetSurface(), NULL);

	Display::C_Viewport viewport;
	viewport.m_W = pTexture->GetWidth();
	viewport.m_H = pTexture->GetHeight();
	Display::Device()->SetViewport(viewport);

	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShader);
	Display::Device()->SetPixelShader(&m_Shaders.m_psNormalMap);

	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_psNormalMap_Settings, Display::C_PSConstant(1.0f / (F32) pTexture->GetWidth(), 1.0f / (F32) pTexture->GetHeight(), fSlopeScale));

	Display::Device()->SetTexture(0, pTexture);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);

	Display::Quad::DrawFullScreen();

}

// this currently only works on 360
void C_TextureUtils::BuildMipMaps(const Display::C_Texture2D* pTexture, const bool blurMips)
{
	LOGWARN(Render::g_Log, "C_TextureUtils::BuildMipMaps is not implemented on this platform");
	return;
}

} // namespace Texture
} // namespace Render
