#include "stdafx.h"
#include "Render/Texture/GaussianBlur.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psGaussBlur5);
EXTERN_SHADER(psGaussBlur7);
EXTERN_SHADER(psGaussBlur9);
EXTERN_SHADER(psGaussBlur11);
EXTERN_SHADER(psGaussBlur13);
EXTERN_SHADER(psBilinearGaussBlurX5);
EXTERN_SHADER(psBilinearGaussBlurY5);
EXTERN_SHADER(psBilinearGaussBlurX7);
EXTERN_SHADER(psBilinearGaussBlurY7);
EXTERN_SHADER(psBilinearGaussBlurX9);
EXTERN_SHADER(psBilinearGaussBlurY9);
EXTERN_SHADER(psBilinearGaussBlurX11);
EXTERN_SHADER(psBilinearGaussBlurY11);
EXTERN_SHADER(psBilinearGaussBlurX13);
EXTERN_SHADER(psBilinearGaussBlurY13);
EXTERN_SHADER(psSimpleGaussBlurX5);
EXTERN_SHADER(psSimpleGaussBlurY5);
EXTERN_SHADER(psSimpleGaussBlurX7);
EXTERN_SHADER(psSimpleGaussBlurY7);
EXTERN_SHADER(psSimpleGaussBlurX9);
EXTERN_SHADER(psSimpleGaussBlurY9);
EXTERN_SHADER(psSimpleGaussBlurX11);
EXTERN_SHADER(psSimpleGaussBlurY11);
EXTERN_SHADER(psSimpleGaussBlurX13);
EXTERN_SHADER(psSimpleGaussBlurY13);

namespace Render
{
namespace Texture
{

bool C_GaussianBlurShaders::Create()
{
	RESOURCE_MEMORY_ID("GaussianBlur");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psGaussBlur5", 0, &m_GaussBlurShader[E_GaussianBlurShader_5], REF_SHADER(psGaussBlur5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psGaussBlur7", 0, &m_GaussBlurShader[E_GaussianBlurShader_7], REF_SHADER(psGaussBlur7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psGaussBlur9", 0, &m_GaussBlurShader[E_GaussianBlurShader_9], REF_SHADER(psGaussBlur9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psGaussBlur11", 0, &m_GaussBlurShader[E_GaussianBlurShader_11], REF_SHADER(psGaussBlur11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psGaussBlur13", 0, &m_GaussBlurShader[E_GaussianBlurShader_13], REF_SHADER(psGaussBlur13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurX5", 0, &m_BilinearGaussBlurXShader[E_GaussianBlurShader_5], REF_SHADER(psBilinearGaussBlurX5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurY5", 0, &m_BilinearGaussBlurYShader[E_GaussianBlurShader_5], REF_SHADER(psBilinearGaussBlurY5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurX7", 0, &m_BilinearGaussBlurXShader[E_GaussianBlurShader_7], REF_SHADER(psBilinearGaussBlurX7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurY7", 0, &m_BilinearGaussBlurYShader[E_GaussianBlurShader_7], REF_SHADER(psBilinearGaussBlurY7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurX9", 0, &m_BilinearGaussBlurXShader[E_GaussianBlurShader_9], REF_SHADER(psBilinearGaussBlurX9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurY9", 0, &m_BilinearGaussBlurYShader[E_GaussianBlurShader_9], REF_SHADER(psBilinearGaussBlurY9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurX11", 0, &m_BilinearGaussBlurXShader[E_GaussianBlurShader_11], REF_SHADER(psBilinearGaussBlurX11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurY11", 0, &m_BilinearGaussBlurYShader[E_GaussianBlurShader_11], REF_SHADER(psBilinearGaussBlurY11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurX13", 0, &m_BilinearGaussBlurXShader[E_GaussianBlurShader_13], REF_SHADER(psBilinearGaussBlurX13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psBilinearGaussBlurY13", 0, &m_BilinearGaussBlurYShader[E_GaussianBlurShader_13], REF_SHADER(psBilinearGaussBlurY13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurX5", 0, &m_SimpleGaussBlurXShader[E_GaussianBlurShader_5], REF_SHADER(psSimpleGaussBlurX5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurY5", 0, &m_SimpleGaussBlurYShader[E_GaussianBlurShader_5], REF_SHADER(psSimpleGaussBlurY5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurX7", 0, &m_SimpleGaussBlurXShader[E_GaussianBlurShader_7], REF_SHADER(psSimpleGaussBlurX7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurY7", 0, &m_SimpleGaussBlurYShader[E_GaussianBlurShader_7], REF_SHADER(psSimpleGaussBlurY7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurX9", 0, &m_SimpleGaussBlurXShader[E_GaussianBlurShader_9], REF_SHADER(psSimpleGaussBlurX9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurY9", 0, &m_SimpleGaussBlurYShader[E_GaussianBlurShader_9], REF_SHADER(psSimpleGaussBlurY9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurX11", 0, &m_SimpleGaussBlurXShader[E_GaussianBlurShader_11], REF_SHADER(psSimpleGaussBlurX11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurY11", 0, &m_SimpleGaussBlurYShader[E_GaussianBlurShader_11], REF_SHADER(psSimpleGaussBlurY11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurX13", 0, &m_SimpleGaussBlurXShader[E_GaussianBlurShader_13], REF_SHADER(psSimpleGaussBlurX13)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSimpleGaussBlurY13", 0, &m_SimpleGaussBlurYShader[E_GaussianBlurShader_13], REF_SHADER(psSimpleGaussBlurY13)))
		return false;

	for (U32 shaderNo = 0; shaderNo < E_GaussianBlurShader_Count; ++shaderNo)
	{
		m_GaussBlurSettingsParameter[shaderNo]			= m_GaussBlurShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_GaussBlurWeights0Parameter[shaderNo]			= m_GaussBlurShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_1, "g_vWeights0");
		m_GaussBlurWeights1Parameter[shaderNo]			= m_GaussBlurShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_2, "g_vWeights1");
		m_BilinearGaussBlurXSettingsParameter[shaderNo]	= m_BilinearGaussBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_BilinearGaussBlurXWeights0Parameter[shaderNo]	= m_BilinearGaussBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_1, "g_vWeights0");
		m_BilinearGaussBlurXWeights1Parameter[shaderNo]	= m_BilinearGaussBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_2, "g_vWeights1");
		m_BilinearGaussBlurYSettingsParameter[shaderNo]	= m_BilinearGaussBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_BilinearGaussBlurYWeights0Parameter[shaderNo]	= m_BilinearGaussBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_1, "g_vWeights0");
		m_BilinearGaussBlurYWeights1Parameter[shaderNo]	= m_BilinearGaussBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_2, "g_vWeights1");
		m_SimpleGaussBlurXSettingsParameter[shaderNo]	= m_SimpleGaussBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_SimpleGaussBlurXWeights0Parameter[shaderNo]	= m_SimpleGaussBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_1, "g_vWeights0");
		m_SimpleGaussBlurXWeights1Parameter[shaderNo]	= m_SimpleGaussBlurXShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_2, "g_vWeights1");
		m_SimpleGaussBlurYSettingsParameter[shaderNo]	= m_SimpleGaussBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
		m_SimpleGaussBlurYWeights0Parameter[shaderNo]	= m_SimpleGaussBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_1, "g_vWeights0");
		m_SimpleGaussBlurYWeights1Parameter[shaderNo]	= m_SimpleGaussBlurYShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_2, "g_vWeights1");
	}

	return true;
}

void C_GaussianBlurShaders::Destroy()
{
	for (U32 shaderNo = 0; shaderNo < E_GaussianBlurShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_GaussBlurShader[shaderNo]);
		Display::Device()->Destroy(&m_BilinearGaussBlurXShader[shaderNo]);
		Display::Device()->Destroy(&m_BilinearGaussBlurYShader[shaderNo]);
		Display::Device()->Destroy(&m_SimpleGaussBlurXShader[shaderNo]);
		Display::Device()->Destroy(&m_SimpleGaussBlurYShader[shaderNo]);
	}
}

S32 C_GaussianBlur::m_RefCount = 0;
C_GaussianBlurShaders C_GaussianBlur::m_Shaders;

C_GaussianBlur::C_GaussianBlur()
{
	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_GaussianBlur::~C_GaussianBlur()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_GaussianBlur::GaussBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader, const F32 fScale, const F32 fDeviation) const
{
	PERF_EVENT("PostProcess.GaussianBlurX");

	F32 fWeights[8];
	GetGaussianWeights(GetGaussianSamples(eShader), fDeviation, fWeights);

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(fScale / (F32) pSrc->GetTexture().GetWidth(), 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_GaussBlurSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_GaussBlurWeights0Parameter[eShader], (F32*)&fWeights[0]);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_GaussBlurWeights1Parameter[eShader], (F32*)&fWeights[4]);
	Display::Device()->SetPixelShader(&m_Shaders.m_GaussBlurShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

#if PLATFORM_PS3
	if (eShader == Render::Texture::E_GaussianBlurShader_5)
		m_Shaders.m_GaussBlurShader[eShader].SetRegisterCount(7);
	if (eShader == Render::Texture::E_GaussianBlurShader_7)
		m_Shaders.m_GaussBlurShader[eShader].SetRegisterCount(31);
#endif
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_GaussianBlur::GaussBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader, const F32 fScale, const F32 fDeviation) const
{
	PERF_EVENT("PostProcess.GaussianBlurY");

	F32 fWeights[8];
	GetGaussianWeights(GetGaussianSamples(eShader), fDeviation, fWeights);

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(0.0f, fScale / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_GaussBlurSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_GaussBlurWeights0Parameter[eShader], (F32*)&fWeights[0]);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_GaussBlurWeights1Parameter[eShader], (F32*)&fWeights[4]);
	Display::Device()->SetPixelShader(&m_Shaders.m_GaussBlurShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_GaussianBlur::BilinearGaussBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const
{
	PERF_EVENT("BilinearGaussianBlurX");

	F32 fWeights[8];
	GetBilinearGaussianWeights(GetGaussianSamples(eShader), fWeights);

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearGaussBlurXSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearGaussBlurXWeights0Parameter[eShader], (F32*)&fWeights[0]);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearGaussBlurXWeights1Parameter[eShader], (F32*)&fWeights[4]);
	Display::Device()->SetPixelShader(&m_Shaders.m_BilinearGaussBlurXShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_GaussianBlur::BilinearGaussBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const
{
	PERF_EVENT("BilinearGaussianBlurY");

	F32 fWeights[8];
	GetBilinearGaussianWeights(GetGaussianSamples(eShader), fWeights);

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(0.0f, 1.0f / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearGaussBlurYSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearGaussBlurYWeights0Parameter[eShader], (F32*)&fWeights[0]);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_BilinearGaussBlurYWeights1Parameter[eShader], (F32*)&fWeights[4]);
	Display::Device()->SetPixelShader(&m_Shaders.m_BilinearGaussBlurYShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_GaussianBlur::SimpleGaussBlurX(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const
{
	PERF_EVENT("SimpleGaussianBlurX");

	F32 fWeights[8];
	GetGaussianWeights(GetGaussianSamples(eShader), fWeights);

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleGaussBlurXSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleGaussBlurXWeights0Parameter[eShader], (F32*)&fWeights[0]);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleGaussBlurXWeights1Parameter[eShader], (F32*)&fWeights[4]);
	Display::Device()->SetPixelShader(&m_Shaders.m_SimpleGaussBlurXShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_GaussianBlur::SimpleGaussBlurY(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc, const E_GaussianBlurShader eShader) const
{
	PERF_EVENT("SimpleGaussianBlurY");

	F32 fWeights[8];
	GetGaussianWeights(GetGaussianSamples(eShader), fWeights);

	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pDst->GetSurface().GetWidth();
	viewport.m_H = pDst->GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(0.0f, 1.0f / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleGaussBlurYSettingsParameter[eShader], (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleGaussBlurYWeights0Parameter[eShader], (F32*)&fWeights[0]);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_SimpleGaussBlurYWeights1Parameter[eShader], (F32*)&fWeights[4]);
	Display::Device()->SetPixelShader(&m_Shaders.m_SimpleGaussBlurYShader[eShader]);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_GaussianBlur::GetGaussianWeights(const U32 nSamples, F32* fWeights) const
{
	GetGaussianWeights(nSamples, 1.0f, fWeights);
}

void C_GaussianBlur::GetGaussianWeights(const U32 nSamples, const F32 fDeviation, F32* fWeights) const
{
	const U32 N = (nSamples + 1) / 2;

	fWeights[0]			= PostProcess::GaussianDistribution(0, 0, 2.0f * fDeviation);

	F32 fTotalWeight	= fWeights[0];
	for (U32 i = 1; i < N; ++i)
	{
		fWeights[i]		= PostProcess::GaussianDistribution((F32)i, 0, 2.0f * fDeviation);
		fTotalWeight	+= 2.0f * fWeights[i];
	}
	
	fTotalWeight = 1.0f / fTotalWeight;
	for (U32 i = 0; i < N; ++i)
	{
		fWeights[i] *= fTotalWeight;
	}
}

void C_GaussianBlur::GetBilinearGaussianWeights(const U32 nSamples, F32* fWeights) const
{
	const U32 N = (nSamples - 1) / 2;

	F32 fTotalWeight	= 0.0f;
	for (U32 i = 0; i < N; ++i)
	{
		fWeights[i]		= PostProcess::GaussianDistribution((F32)i + 0.5f, 0, 2.0f);
		fTotalWeight	+= fWeights[i];
	}
	
	fTotalWeight = 0.5f / fTotalWeight;
	for (U32 i = 0; i < N; ++i)
	{
		fWeights[i] *= fTotalWeight;
	}
}

U32 C_GaussianBlur::GetGaussianSamples(const E_GaussianBlurShader eShader) const
{
	switch (eShader)
	{
	case E_GaussianBlurShader_5:
		return 5;
	case E_GaussianBlurShader_7:
		return 7;
	case E_GaussianBlurShader_9:
		return 9;
	case E_GaussianBlurShader_11:
		return 11;
	case E_GaussianBlurShader_13:
		return 13;
	default:
		return 1;
	};
}

}; // namespace Texture
}; // namespace Render
