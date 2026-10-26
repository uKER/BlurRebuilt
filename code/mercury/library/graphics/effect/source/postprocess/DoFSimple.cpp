#include "stdafx.h"
#include "Effect/PostProcess/DoFSimple.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psDoFSimple);

namespace PostProcess
{

Debug::Float	g_debugSimpleFocalPoint("Graphics.Effect.PostProcess.DepthOfFieldSimple.FocalPoint", 5.0f, Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugSimpleFocalScale("Graphics.Effect.PostProcess.DepthOfFieldSimple.FocalScale", 2.0f, Debug::Range(0.0f, 100.0f));

C_DoFSimple::C_DoFSimple(C_Manager& manager) : I_PostProcess(manager)
{
}

C_DoFSimple::~C_DoFSimple()
{
}

bool C_DoFSimple::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFSimple", 0, &m_PixelShader, REF_SHADER(psDoFSimple)))
		return false;

	m_SettingsParameter	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_DepthParameter	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vDepth");

	return true;
}

bool C_DoFSimple::CreateVolatile()
{
	if(!m_Manager.CreateUserRenderTextures(4, 4, Display::E_Usage_Colour | Display::E_Usage_ReadWrite | Display::E_Usage_Expand, Display::E_Format_Unknown, 0, m_RenderTextures, 0, 0))
		return false;

	return true;
}

void C_DoFSimple::Destroy()
{
	Display::Device()->Destroy(&m_PixelShader);
}

void C_DoFSimple::DestroyVolatile()
{
	m_Manager.DestroyUserRenderTextures(m_RenderTextures);
}

void C_DoFSimple::Display() const
{
	if(!m_Manager.Enabled())
		return;

	const S32 iRenderTarget = m_Manager.GetRenderTargetIndex();
	if(iRenderTarget < 0 || iRenderTarget >= m_RenderTextures.Size())
		return;

	PERF_EVENT("DepthOfFieldSimple");

	SetViewport();

	const S32 iViewIndex = m_Manager.GetViewIndex();
	const Display::C_RenderTexture2D* pRenderTexture = m_RenderTextures[iRenderTarget];

	m_DownsampleUtil.Downsample(pRenderTexture, m_Manager.GetMainTexture());

	m_GaussianBlurUtil.GaussBlurX(pRenderTexture, pRenderTexture, Render::Texture::E_GaussianBlurShader_5);
	m_GaussianBlurUtil.GaussBlurY(pRenderTexture, pRenderTexture, Render::Texture::E_GaussianBlurShader_5);

	m_Manager.SetViewport();

	Display::Device()->SetTexture(0, m_Manager.GetMainTexture());
	Display::Device()->SetTexture(1, &pRenderTexture->GetTexture());
	Display::Device()->SetTexture(2, Render::GetGlobalDepthStencilBuffer());

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);

	const Display::C_Float4 vSettings = Display::C_Float4(g_debugSimpleFocalPoint, 1.0f / g_debugSimpleFocalScale, 0.0f, 0.0f);
	const Display::C_Float4 vDepth = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_DepthParameter, (F32*)&vDepth);
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Quad::DrawFullScreen();

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);

	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);
}

}; // namespace PostProcess
