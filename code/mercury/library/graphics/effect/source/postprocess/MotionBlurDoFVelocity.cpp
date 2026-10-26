#include "stdafx.h"
#include "Effect/PostProcess/MotionBlurDoFVelocity.h"

#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psMotionBlurDoFVelocity5);
EXTERN_SHADER(psMotionBlurDoFVelocity7);
EXTERN_SHADER(psMotionBlurDoFVelocity9);
EXTERN_SHADER(psMotionBlurDoFVelocity11);

namespace PostProcess
{

extern Debug::Bool	g_debugMotionBlurDoF;
extern Debug::Float	g_debugMotionBlurDoFShutterAngle;
extern Debug::Bool	g_debugMotionBlurDoFEnableDoF;

extern Debug::Float	g_debugMotionBlurDoFBlurScale;
extern Debug::Float	g_debugMotionBlurDoFFocalLength;
extern Debug::Float	g_debugMotionBlurDoFMaxCoC;
extern Debug::Float	g_debugMotionBlurDoFEdgeFactor;

C_MotionBlurDoFVelocity::C_MotionBlurDoFVelocity(C_Manager& manager) : I_PostProcess( manager ), m_bEnabled(true), m_pViewportSettings(0), m_pVelocityBuffer(0)
{
	SetPixelShader(E_MotionBlurDoFVelocityShader_11);
	SetScaleBias(1.0f, 0.0f);
}

C_MotionBlurDoFVelocity::~C_MotionBlurDoFVelocity()
{
	SAFE_DELETE_ARY(m_pViewportSettings);
}

bool C_MotionBlurDoFVelocity::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFVelocity5", 0, &m_PixelShaders[E_MotionBlurDoFVelocityShader_5], REF_SHADER(psMotionBlurDoFVelocity5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFVelocity7", 0, &m_PixelShaders[E_MotionBlurDoFVelocityShader_7], REF_SHADER(psMotionBlurDoFVelocity7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFVelocity9", 0, &m_PixelShaders[E_MotionBlurDoFVelocityShader_9], REF_SHADER(psMotionBlurDoFVelocity9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFVelocity11", 0, &m_PixelShaders[E_MotionBlurDoFVelocityShader_11], REF_SHADER(psMotionBlurDoFVelocity11)))
		return false;

	for (U32 shaderNo = 0; shaderNo < E_MotionBlurDoFVelocityShader_Count; ++shaderNo)
	{
		m_DepthScaleParameter[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vDepthScale");
		m_StepScaleParameter[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vStepScale");
		m_PixelScaleParameter[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vPixelScale");
		m_Settings1Parameter[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vSettings1");
		m_Settings2Parameter[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_vSettings2");
		m_FilterParameter[shaderNo]		= m_PixelShaders[shaderNo].GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_5, "g_vFilter", 6);
	}
	
	m_pViewportSettings = new C_DepthOfFieldSettings[ m_Manager.GetViewCount() ];

	return true;
}

bool C_MotionBlurDoFVelocity::CreateVolatile()
{
	return true;
}

void C_MotionBlurDoFVelocity::Destroy()
{
	SAFE_DELETE_ARY(m_pViewportSettings);

	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurDoFVelocityShader_5]);
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurDoFVelocityShader_7]);
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurDoFVelocityShader_9]);
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurDoFVelocityShader_11]);
}

void C_MotionBlurDoFVelocity::DestroyVolatile()
{
}

void C_MotionBlurDoFVelocity::Display() const
{
	if (!m_Manager.Enabled() || !g_debugMotionBlurDoF || !m_bEnabled)
		return;

	PERF_EVENT("MotionBlurDoFVelocity");

	SetViewport();

	m_pVelocityBuffer = CreateVelocityBuffer();

	m_Manager.SetViewport();

	SetConstants();

	SetShader(&m_PixelShaders[m_eShader]);

	Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

	Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);

	Display::Device()->SetTexture(2, m_pVelocityBuffer);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);

	DisplayFinal();

	for (U32 i = 0; i <= 2; ++i)
	{
		Display::Device()->SetTexture(0, 0);
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	}
}

void C_MotionBlurDoFVelocity::SetConstants() const
{
	const Display::C_Float4 vDepthScale = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_DepthScaleParameter[m_eShader], (F32*)&vDepthScale);

	const F32 fStep = g_debugMotionBlurDoFShutterAngle * m_fSamplesMinusOneRcp;
	const Display::C_Float4 vStepScale(fStep, fStep, -fStep, -fStep);
	Display::Device()->SetPixelShaderParameterF(m_StepScaleParameter[m_eShader], (F32*)&vStepScale);

	const F32 fPixelWidth = 1.0f / (F32) Display::Device()->GetBackBuffer().GetWidth();
	const F32 fPixelHeight = 1.0f / (F32) Display::Device()->GetBackBuffer().GetHeight();
	const Display::C_Float4 vPixelScale(fPixelWidth, fPixelHeight, fPixelWidth, fPixelHeight);
	Display::Device()->SetPixelShaderParameterF(m_PixelScaleParameter[m_eShader], (F32*)&vPixelScale);

	{
		F32 fAperture = GetAperture(m_Manager.GetViewIndex());
		F32 fPlaneInFocus = GetPlaneInFocus(m_Manager.GetViewIndex());

		MathsStorage::Float4 vSettings1(fAperture, fPlaneInFocus, m_fScale, m_fBias);
		Display::Device()->SetPixelShaderParameterF(m_Settings1Parameter[m_eShader], (F32*)&vSettings1);
	}

	const Display::C_Float4 vSettings2(g_debugMotionBlurDoFMaxCoC, g_debugMotionBlurDoFEdgeFactor, g_debugMotionBlurDoFBlurScale, g_debugMotionBlurDoFFocalLength);
	Display::Device()->SetPixelShaderParameterF(m_Settings2Parameter[m_eShader], (F32*)&vSettings2);

	// set filter taps
	{
		const F32 fTaps[24] =
		{
			-0.326212f,	-0.405810f,
			+0.519456f,	+0.767022f,
			-0.840144f,	-0.073580f,
			+0.507431f,	+0.064425f,
			+0.473434f,	-0.480026f,
			-0.695914f,	+0.457137f,
			-0.203345f,	+0.620716f,
			+0.962340f,	-0.194983f,
			+0.896420f,	+0.412458f,
			-0.321940f,	-0.932615f,
			+0.185461f,	-0.893124f,
			-0.791559f,	-0.597710f
		};

		Display::Device()->SetPixelShaderParameterArrayF(m_FilterParameter[m_eShader], fTaps);
	}
}

const Display::C_Texture2D* C_MotionBlurDoFVelocity::CreateVelocityBuffer() const
{
	return m_pVelocityBuffer;
}

}; // namespace PostProcess
