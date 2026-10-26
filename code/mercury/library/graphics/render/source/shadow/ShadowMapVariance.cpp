#include "stdafx.h"
#include "Render/Shadow/ShadowMapVariance.h"

EXTERN_SHADER(vsShadowMapVarianceDepth);
EXTERN_SHADER(psShadowMapVarianceDepth);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Shadow/PS3/vsShadowMapVarianceDepth.shader"
#include "Render/Shaders/Shadow/PS3/psShadowMapVarianceDepth.shader"
#endif
#endif

namespace Render
{
namespace Shadow
{

::Debug::Float g_debugVarianceEpilson("Graphics.Render.ShadowMap.Epsilon", 0.001f, ::Debug::Range(0.0f, 1.0f));
::Debug::Float g_debugVariancePower("Graphics.Render.ShadowMap.Power", 16.f, ::Debug::Range(0.1f, 100.f));
::Debug::Float g_debugVarianceBias("Graphics.Render.ShadowMap.Bias", 0.0f, ::Debug::Range(-1.0f, 1.0f));

C_VarianceMapShaders C_VarianceMap::m_Shaders;

C_VarianceMap::C_VarianceMap()
{
	m_Levels				= 1;
	#ifdef PLATFORM_PS3
	m_ColourFormat			= Display::E_Format_A8R8G8B8;
	#else
	m_ColourFormat			= Display::E_Format_G16R16;
	#endif
	m_DepthStencilFormat	= Display::Device()->GetBackBuffer().GetDepthStencilFormat();

	Display::MatrixSetIdentity(&m_ShadowMatrix);
	m_hTextureMatrix.AX(Maths::HVec4(0.5f,  0.0f, 0.0f, 0.0f));
	m_hTextureMatrix.AY(Maths::HVec4(0.0f, -0.5f, 0.0f, 0.0f));
	m_hTextureMatrix.AZ(Maths::HVec4(0.0f,  0.0f, 1.0f, 0.0f));
	m_hTextureMatrix.AW(Maths::HVec4(0.5f,  0.5f, 0.0f, 1.0f));

	g_debugUseDepthStencilTexture = true;

	g_pVarShadowMap = this;
}

C_VarianceMap::~C_VarianceMap()
{
	g_pVarShadowMap = NULL;
}

void C_VarianceMap::Initialise(const U32 width, const U32 height)
{
	C_ShadowMap::Initialise(width, height);
}

bool C_VarianceMap::OnCreate()
{
	//if(!CreateShaders())
		//return false;

	return true;
}

bool C_VarianceMap::OnCreateVolatile()
{
	if(!C_ShadowMap::OnCreateVolatile())
		return false;

	if(!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, Display::E_Usage_Colour, m_ColourFormat, Display::E_Multisample_None, NULL, &m_ColourTexture))
		return false;
	if(!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, Display::E_Usage_DepthStencil, m_DepthStencilFormat, Display::E_Multisample_None, NULL, &m_DepthStencilTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_ColourTexture, "_VarianceMapColourRT");
	Display2::Device()->SetRenderTexture2DName(&m_DepthStencilTexture, "_VarianceMapDepthStencilRT");
	#endif
	
	return true;
}

void C_VarianceMap::OnDestroyVolatile()
{
	C_ShadowMap::OnDestroyVolatile();

	Display::Device()->Destroy(&m_ColourTexture);
	Display::Device()->Destroy(&m_DepthStencilTexture);
}

void C_VarianceMap::OnDestroy()
{
	//DestroyShaders();
}

bool C_VarianceMap::CreateShaders()
{
	if(!Display::CreateVertexShaderFromFunc("Shaders/vsShadowMapVarianceDepth", 0, &m_Shaders.m_vsDepth, REF_SHADER(vsShadowMapVarianceDepth)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psShadowMapVarianceDepth", 0, &m_Shaders.m_psDepth, REF_SHADER(psShadowMapVarianceDepth)))
		return false;
	
	return true;
}

void C_VarianceMap::DestroyShaders()
{
	Display::Device()->Destroy(&m_Shaders.m_vsDepth);
	Display::Device()->Destroy(&m_Shaders.m_psDepth);
}

void C_VarianceMap::SetShaders()
{
	Display::Device()->SetVertexShader(&m_Shaders.m_vsDepth);

	Display::Device()->SetPixelShader(&m_Shaders.m_psDepth);
}

void C_VarianceMap::SetTexture(const U32 sampler, const U32 shadowMatrixRegister)
{
//	Display::C_Float4 psSettings(g_debugVarianceEpilson, g_debugVariancePower, g_debugVarianceBias, 0.0f);
//	Display::Device()->SetPixelShaderConstantF(100, (F32*)&psSettings, 1);

	Display::Device()->SetVertexShaderConstantF(shadowMatrixRegister, (F32*)&m_ShadowMatrix, 4);

	Display::Device()->SetTexture(sampler, &m_RenderTexture.GetTexture());
	Display::Device()->SetTextureFilterMagMinMip(sampler, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddressUVW(sampler, Display::E_TextureAddress_Border, Display::E_TextureAddress_Border, Display::E_TextureAddress_Border);
	Display::Device()->SetTextureBorderColour(sampler, 0xFFFFFFFF);
}

void C_VarianceMap::Bind()
{
	C_ShadowMap::Bind();

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	Maths::Store(viewSettings3D.m_WorldToDevice * m_hTextureMatrix, &m_ShadowMatrix);

	SetShaders();
}

void C_VarianceMap::Unbind()
{

	C_ShadowMap::Unbind();
}

void C_VarianceMap::DebugRender()
{
	if(!m_RenderTexture.IsCreated())
		return;

	Display::BasicRender()->DrawTextured2DRect(1024.f - 300.f - 20.f, 20.f, 300.f, 300.f, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture.GetTexture());
}

C_VarianceMap* g_pVarShadowMap = NULL;

} // namespace Shadow
} // namespace Render
