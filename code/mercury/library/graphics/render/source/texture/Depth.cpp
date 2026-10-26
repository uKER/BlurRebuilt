#include "stdafx.h"
#include "Render/Texture/Depth.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psWorldNormal);

namespace Render
{
namespace Texture
{

bool C_DepthShaders::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psWorldNormal", 0, &m_WorldNormalShader, REF_SHADER(psWorldNormal)))
		return false;

	m_WorldNormalMatrixParameter	= m_WorldNormalShader.GetParameterArray(CP_RENDER_SETTINGS, "g_mToWorld", 4);
	m_WorldNormalScaleParameter		= m_WorldNormalShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vDepthScale");
	m_WorldNormalSettingsParameter	= m_WorldNormalShader.GetParameter(CP_RENDER_SETTINGS_5, "g_vSettings");

	return true;
}

void C_DepthShaders::Destroy()
{
	Display::Device()->Destroy(&m_WorldNormalShader);
}

S32 C_Depth::m_RefCount = 0;
C_DepthShaders C_Depth::m_Shaders;

C_Depth::C_Depth()
{
	RESOURCE_MEMORY_ID("Depth");

	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_Depth::~C_Depth()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_Depth::WorldNormal(const Display::C_RenderTexture2D* pDst, const Display::C_Texture2D* pSrc, const Display::C_Float4& vDepthScale, const Display::C_Matrix& mToWorld) const
{
	PERF_EVENT("Depth");

	const Display::C_Surface* pColourSurface = Display::Device()->GetRenderTarget().GetColourSurface();
	const Display::C_Surface* pDepthStencilSurface = Display::Device()->GetRenderTarget().GetDepthStencilSurface();
	const Display::C_Viewport savedViewport = Display::Device()->GetViewport();

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetTexture().GetWidth();
	viewport.m_H = pDst->GetTexture().GetHeight();
	Display::Device()->SetViewport(viewport);

	const Display::C_Float4 vSettings(1.0f / (F32) pSrc->GetWidth(), 1.0f / (F32) pSrc->GetHeight(), 0.0f, 0.0f);

	Display::Device()->SetPixelShaderParameterArrayF(m_Shaders.m_WorldNormalMatrixParameter, (F32*)&mToWorld);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_WorldNormalScaleParameter, (F32*)&vDepthScale);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_WorldNormalSettingsParameter, (F32*)&vSettings);

	Display::Device()->SetPixelShader(&m_Shaders.m_WorldNormalShader);
	Display::Device()->SetTexture(0, pSrc);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);

	Display::Device()->SetColourDepthStencilSurface(pColourSurface, pDepthStencilSurface);
	Display::Device()->SetViewport(savedViewport);
}

}; // namespace Texture
}; // namespace Render
