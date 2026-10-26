#include "stdafx.h"
#include "Effect/Shadow/ShadowMapUtils.h"
#include "Effect/Shaders/Constants.h"

EXTERN_SHADER(psExponentialFilterX5);
EXTERN_SHADER(psExponentialFilterX7);
EXTERN_SHADER(psExponentialFilterY5);
EXTERN_SHADER(psExponentialFilterY7);
EXTERN_SHADER(psVarianceFilterX5);
EXTERN_SHADER(psVarianceFilterX7);
EXTERN_SHADER(psVarianceFilterY5);
EXTERN_SHADER(psVarianceFilterY7);
EXTERN_SHADER(psVarianceResolve);

namespace Effect
{

bool C_ExponentialFilterShaders::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psExponentialFilterX5", 0, &m_ExponentialFilterX5Shader, REF_SHADER(psExponentialFilterX5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psExponentialFilterX7", 0, &m_ExponentialFilterX7Shader, REF_SHADER(psExponentialFilterX7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psExponentialFilterY5", 0, &m_ExponentialFilterY5Shader, REF_SHADER(psExponentialFilterY5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psExponentialFilterY7", 0, &m_ExponentialFilterY7Shader, REF_SHADER(psExponentialFilterY7)))
		return false;

	m_ExponentialFilterX5Parameter	= m_ExponentialFilterX5Shader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_ExponentialFilterX7Parameter	= m_ExponentialFilterX7Shader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_ExponentialFilterY5Parameter	= m_ExponentialFilterY5Shader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_ExponentialFilterY7Parameter	= m_ExponentialFilterY7Shader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");

	return true;
}

void C_ExponentialFilterShaders::Destroy()
{
	Display::Device()->Destroy(&m_ExponentialFilterX5Shader);
	Display::Device()->Destroy(&m_ExponentialFilterX7Shader);
	Display::Device()->Destroy(&m_ExponentialFilterY5Shader);
	Display::Device()->Destroy(&m_ExponentialFilterY7Shader);
}

S32 C_ExponentialFilter::m_RefCount = 0;
C_ExponentialFilterShaders C_ExponentialFilter::m_Shaders;

C_ExponentialFilter::C_ExponentialFilter()
{
	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_ExponentialFilter::~C_ExponentialFilter()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_ExponentialFilter::ExponentialFilter5(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const
{
	PERF_EVENT("ExponentialFilter5");

	// set states
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	F32 fNearClip = 0.0f;
	F32 fFarClip = 0.0f;
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = Display::Device()->GetViewSettings3D().GetFarClip();
		fFarClip = Display::Device()->GetViewSettings3D().GetNearClip();
	}
	else
	{
		fNearClip = Display::Device()->GetViewSettings3D().GetNearClip();
		fFarClip = Display::Device()->GetViewSettings3D().GetFarClip();
	}
	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 1.0f / (F32) pSrc->GetTexture().GetHeight(), fFarClip - fNearClip, fNearClip);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ExponentialFilterX5Parameter, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ExponentialFilterY5Parameter, (F32*)&vSettings);

	// gaussian blur x
	Display::Device()->SetColourDepthStencilSurface(&pInt->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_ExponentialFilterX5Shader);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pInt->Resolve();

	// gaussian blur y
	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_ExponentialFilterY5Shader);
	Display::Device()->SetTexture(0, &pInt->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	// reset states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_ExponentialFilter::ExponentialFilter7(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const
{
	PERF_EVENT("ExponentialFilter7");

	// set states
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	F32 fNearClip = 0.0f;
	F32 fFarClip = 0.0f;
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = Display::Device()->GetViewSettings3D().GetFarClip();
		fFarClip = Display::Device()->GetViewSettings3D().GetNearClip();
	}
	else
	{
		fNearClip = Display::Device()->GetViewSettings3D().GetNearClip();
		fFarClip = Display::Device()->GetViewSettings3D().GetFarClip();
	}
	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 1.0f / (F32) pSrc->GetTexture().GetHeight(), fFarClip - fNearClip, fNearClip);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ExponentialFilterX7Parameter, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ExponentialFilterY7Parameter, (F32*)&vSettings);

	// gaussian blur x
	Display::Device()->SetColourDepthStencilSurface(&pInt->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_ExponentialFilterX7Shader);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pInt->Resolve();

	// gaussian blur y
	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_ExponentialFilterY7Shader);
	Display::Device()->SetTexture(0, &pInt->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	// reset states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

bool C_VarianceFilterShaders::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psVarianceFilterX5", 0, &m_VarianceFilterX5Shader, REF_SHADER(psVarianceFilterX5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psVarianceFilterX7", 0, &m_VarianceFilterX7Shader, REF_SHADER(psVarianceFilterX7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psVarianceFilterY5", 0, &m_VarianceFilterY5Shader, REF_SHADER(psVarianceFilterY5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psVarianceFilterY7", 0, &m_VarianceFilterY7Shader, REF_SHADER(psVarianceFilterY7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psVarianceResolve", 0, &m_VarianceResolveShader, REF_SHADER(psVarianceResolve)))
		return false;

	m_VarianceFilterX5Parameter	= m_VarianceFilterX5Shader.GetParameter(CP_RENDER_SETTINGS, "g_vPixelSize");
	m_VarianceFilterX7Parameter	= m_VarianceFilterX7Shader.GetParameter(CP_RENDER_SETTINGS, "g_vPixelSize");
	m_VarianceFilterY5Parameter	= m_VarianceFilterY5Shader.GetParameter(CP_RENDER_SETTINGS, "g_vPixelSize");
	m_VarianceFilterY7Parameter	= m_VarianceFilterY7Shader.GetParameter(CP_RENDER_SETTINGS, "g_vPixelSize");

	return true;
}

void C_VarianceFilterShaders::Destroy()
{
	Display::Device()->Destroy(&m_VarianceFilterX5Shader);
	Display::Device()->Destroy(&m_VarianceFilterX7Shader);
	Display::Device()->Destroy(&m_VarianceFilterY5Shader);
	Display::Device()->Destroy(&m_VarianceFilterY7Shader);
	Display::Device()->Destroy(&m_VarianceResolveShader);
}

S32 C_VarianceFilter::m_RefCount = 0;
C_VarianceFilterShaders C_VarianceFilter::m_Shaders;

C_VarianceFilter::C_VarianceFilter()
{
	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_VarianceFilter::~C_VarianceFilter()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_VarianceFilter::VarianceFilter5(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const
{
	PERF_EVENT("VarianceFilter5");

	// set states
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 1.0f / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_VarianceFilterX5Parameter, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_VarianceFilterY5Parameter, (F32*)&vSettings);

	// gaussian blur x
	Display::Device()->SetColourDepthStencilSurface(&pInt->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_VarianceFilterX5Shader);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pInt->Resolve();

	// gaussian blur y
	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_VarianceFilterY5Shader);
	Display::Device()->SetTexture(0, &pInt->GetTexture());

	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	// reset states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_VarianceFilter::VarianceFilter7(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const
{
	PERF_EVENT("VarianceFilter7");

	// set states
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	const MathsStorage::Float4 vSettings = MathsStorage::Float4(1.0f / (F32) pSrc->GetTexture().GetWidth(), 1.0f / (F32) pSrc->GetTexture().GetHeight(), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_VarianceFilterX7Parameter, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_VarianceFilterY7Parameter, (F32*)&vSettings);

	// gaussian blur x
	Display::Device()->SetColourDepthStencilSurface(&pInt->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_VarianceFilterX7Shader);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pInt->Resolve();

	// gaussian blur y
	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_VarianceFilterY7Shader);
	Display::Device()->SetTexture(0, &pInt->GetTexture());

	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	// reset states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

void C_VarianceFilter::VarianceResolve(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc) const
{
	PERF_EVENT("VarianceResolve");

	// set states
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	// resolve
	Display::Device()->SetColourDepthStencilSurface(&pDst->GetSurface(), 0);

	Display::Device()->SetPixelShader(&m_Shaders.m_VarianceResolveShader);
	Display::Device()->SetTexture(0, &pSrc->GetTexture());
	
	Display::Quad::DrawFullScreen();

	pDst->Resolve();

	// reset states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTexture(0, 0);
}

}; // namespace Effect
