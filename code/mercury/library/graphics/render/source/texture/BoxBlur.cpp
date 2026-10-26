#include "stdafx.h"
#include "Render/Texture/BoxBlur.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psBoxBlur5);
EXTERN_SHADER(psBoxBlur7);
EXTERN_SHADER(psBoxBlur9);
EXTERN_SHADER(psBoxBlur11);
EXTERN_SHADER(psBoxBlur13);
EXTERN_SHADER(psBilinearBoxBlurX5);
EXTERN_SHADER(psBilinearBoxBlurY5);
EXTERN_SHADER(psBilinearBoxBlurX7);
EXTERN_SHADER(psBilinearBoxBlurY7);
EXTERN_SHADER(psBilinearBoxBlurX9);
EXTERN_SHADER(psBilinearBoxBlurY9);
EXTERN_SHADER(psBilinearBoxBlurX11);
EXTERN_SHADER(psBilinearBoxBlurY11);
EXTERN_SHADER(psBilinearBoxBlurX13);
EXTERN_SHADER(psBilinearBoxBlurY13);
EXTERN_SHADER(psSimpleBoxBlurX5);
EXTERN_SHADER(psSimpleBoxBlurY5);
EXTERN_SHADER(psSimpleBoxBlurX7);
EXTERN_SHADER(psSimpleBoxBlurY7);
EXTERN_SHADER(psSimpleBoxBlurX9);
EXTERN_SHADER(psSimpleBoxBlurY9);
EXTERN_SHADER(psSimpleBoxBlurX11);
EXTERN_SHADER(psSimpleBoxBlurY11);
EXTERN_SHADER(psSimpleBoxBlurX13);
EXTERN_SHADER(psSimpleBoxBlurY13);

namespace Render
{
namespace Texture
{

bool C_BoxBlurShaders::Create()
{
	RESOURCE_MEMORY_ID("BoxBlur");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psBoxBlur5", 0, &m_BoxBlurShader[E_BoxBlurShader_5], REF_SHADER(psBoxBlur5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBoxBlur7", 0, &m_BoxBlurShader[E_BoxBlurShader_7], REF_SHADER(psBoxBlur7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBoxBlur9", 0, &m_BoxBlurShader[E_BoxBlurShader_9], REF_SHADER(psBoxBlur9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBoxBlur11", 0, &m_BoxBlurShader[E_BoxBlurShader_11], REF_SHADER(psBoxBlur11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBoxBlur13", 0, &m_BoxBlurShader[E_BoxBlurShader_13], REF_SHADER(psBoxBlur13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurX5", 0, &m_BilinearBoxBlurXShader[E_BoxBlurShader_5], REF_SHADER(psBilinearBoxBlurX5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurY5", 0, &m_BilinearBoxBlurYShader[E_BoxBlurShader_5], REF_SHADER(psBilinearBoxBlurY5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurX7", 0, &m_BilinearBoxBlurXShader[E_BoxBlurShader_7], REF_SHADER(psBilinearBoxBlurX7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurY7", 0, &m_BilinearBoxBlurYShader[E_BoxBlurShader_7], REF_SHADER(psBilinearBoxBlurY7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurX9", 0, &m_BilinearBoxBlurXShader[E_BoxBlurShader_9], REF_SHADER(psBilinearBoxBlurX9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurY9", 0, &m_BilinearBoxBlurYShader[E_BoxBlurShader_9], REF_SHADER(psBilinearBoxBlurY9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurX11", 0, &m_BilinearBoxBlurXShader[E_BoxBlurShader_11], REF_SHADER(psBilinearBoxBlurX11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurY11", 0, &m_BilinearBoxBlurYShader[E_BoxBlurShader_11], REF_SHADER(psBilinearBoxBlurY11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurX13", 0, &m_BilinearBoxBlurXShader[E_BoxBlurShader_13], REF_SHADER(psBilinearBoxBlurX13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearBoxBlurY13", 0, &m_BilinearBoxBlurYShader[E_BoxBlurShader_13], REF_SHADER(psBilinearBoxBlurY13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurX5", 0, &m_SimpleBoxBlurXShader[E_BoxBlurShader_5], REF_SHADER(psSimpleBoxBlurX5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurY5", 0, &m_SimpleBoxBlurYShader[E_BoxBlurShader_5], REF_SHADER(psSimpleBoxBlurY5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurX7", 0, &m_SimpleBoxBlurXShader[E_BoxBlurShader_7], REF_SHADER(psSimpleBoxBlurX7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurY7", 0, &m_SimpleBoxBlurYShader[E_BoxBlurShader_7], REF_SHADER(psSimpleBoxBlurY7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurX9", 0, &m_SimpleBoxBlurXShader[E_BoxBlurShader_9], REF_SHADER(psSimpleBoxBlurX9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurY9", 0, &m_SimpleBoxBlurYShader[E_BoxBlurShader_9], REF_SHADER(psSimpleBoxBlurY9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurX11", 0, &m_SimpleBoxBlurXShader[E_BoxBlurShader_11], REF_SHADER(psSimpleBoxBlurX11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurY11", 0, &m_SimpleBoxBlurYShader[E_BoxBlurShader_11], REF_SHADER(psSimpleBoxBlurY11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurX13", 0, &m_SimpleBoxBlurXShader[E_BoxBlurShader_13], REF_SHADER(psSimpleBoxBlurX13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleBoxBlurY13", 0, &m_SimpleBoxBlurYShader[E_BoxBlurShader_13], REF_SHADER(psSimpleBoxBlurY13)))
		return false;

	for (U32 shaderNo = 0; shaderNo < E_BoxBlurShader_Count; ++shaderNo)
	{
		m_BoxBlurSettingsParameter[shaderNo]			= m_BoxBlurShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_BilinearBoxBlurXSettingsParameter[shaderNo]	= m_BilinearBoxBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_BilinearBoxBlurYSettingsParameter[shaderNo]	= m_BilinearBoxBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_SimpleBoxBlurXSettingsParameter[shaderNo]	= m_SimpleBoxBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_SimpleBoxBlurYSettingsParameter[shaderNo]	= m_SimpleBoxBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	}

	return true;
}

void C_BoxBlurShaders::Destroy()
{
	for (U32 shaderNo = 0; shaderNo < E_BoxBlurShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_BoxBlurShader[shaderNo]);
		Display::Device()->Destroy(&m_BilinearBoxBlurXShader[shaderNo]);
		Display::Device()->Destroy(&m_BilinearBoxBlurYShader[shaderNo]);
		Display::Device()->Destroy(&m_SimpleBoxBlurXShader[shaderNo]);
		Display::Device()->Destroy(&m_SimpleBoxBlurYShader[shaderNo]);
	}
}

S32 C_BoxBlur::m_RefCount = 0;
C_BoxBlurShaders C_BoxBlur::m_Shaders;

C_BoxBlur::C_BoxBlur()
{
	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_BoxBlur::~C_BoxBlur()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_BoxBlur::BoxBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader, const F32 fScale, const F32 fDeviation) const
{
	PERF_EVENT("BoxBlurX");

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(fScale / (F32) pSrc->GetTexture().GetWidth(), 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BoxBlurSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShader(&m_Shaders.m_BoxBlurShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_BoxBlur::BoxBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader, const F32 fScale, const F32 fDeviation) const
{
	PERF_EVENT("BoxBlurY");

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(0.0f, fScale / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BoxBlurSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShader(&m_Shaders.m_BoxBlurShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_BoxBlur::BilinearBoxBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const
{
	PERF_EVENT("BilinearBoxBlurX");

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearBoxBlurXSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShader(&m_Shaders.m_BilinearBoxBlurXShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_BoxBlur::BilinearBoxBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const
{
	PERF_EVENT("BilinearBoxBlurY");

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(0.0f, 1.0f / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearBoxBlurYSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShader(&m_Shaders.m_BilinearBoxBlurYShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_BoxBlur::SimpleBoxBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const
{
	PERF_EVENT("SimpleBoxBlurX");

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleBoxBlurXSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShader(&m_Shaders.m_SimpleBoxBlurXShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_BoxBlur::SimpleBoxBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_BoxBlurShader eShader) const
{
	PERF_EVENT("SimpleBoxBlurY");

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(0.0f, 1.0f / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleBoxBlurYSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShader(&m_Shaders.m_SimpleBoxBlurYShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

}; // namespace Texture
}; // namespace Render
