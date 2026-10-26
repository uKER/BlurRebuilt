#include "stdafx.h"
#include "Effect/PostProcess/MotionBlur/MotionBlurVelocity.h"

#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psMotionBlurVelocity5);
EXTERN_SHADER(psMotionBlurVelocity7);
EXTERN_SHADER(psMotionBlurVelocity9);
EXTERN_SHADER(psMotionBlurVelocity11);

namespace PostProcess
{

Debug::Float	g_debugMotionBlurVelocityScale("Graphics.Effect.PostProcess.MotionBlurVelocity.Scale", 1.0f, Debug::Range(0.0f, 4.0f));

C_MotionBlurVelocity::C_MotionBlurVelocity( C_Manager& manager ): I_PostProcess( manager )
{
	m_pVelocityBuffer	= 0;
	m_eShader			= E_MotionBlurVelocityShader_5;
}

C_MotionBlurVelocity::~C_MotionBlurVelocity()
{
}

bool C_MotionBlurVelocity::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shader/psMotionBlurVelocity5", 0, &m_PixelShaders[E_MotionBlurVelocityShader_5], REF_SHADER(psMotionBlurVelocity5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psMotionBlurVelocity7", 0, &m_PixelShaders[E_MotionBlurVelocityShader_7], REF_SHADER(psMotionBlurVelocity7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psMotionBlurVelocity9", 0, &m_PixelShaders[E_MotionBlurVelocityShader_9], REF_SHADER(psMotionBlurVelocity9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psMotionBlurVelocity11", 0, &m_PixelShaders[E_MotionBlurVelocityShader_11], REF_SHADER(psMotionBlurVelocity11)))
		return false;

	m_Parameters[E_MotionBlurVelocityShader_5]	= m_PixelShaders[E_MotionBlurVelocityShader_5].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings");
	m_Parameters[E_MotionBlurVelocityShader_7]	= m_PixelShaders[E_MotionBlurVelocityShader_7].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings");
	m_Parameters[E_MotionBlurVelocityShader_9]	= m_PixelShaders[E_MotionBlurVelocityShader_9].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings");
	m_Parameters[E_MotionBlurVelocityShader_11]	= m_PixelShaders[E_MotionBlurVelocityShader_11].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings");

	return true;
}

bool C_MotionBlurVelocity::CreateVolatile()
{
	return true;
}

void C_MotionBlurVelocity::Destroy()
{
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurVelocityShader_5]);
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurVelocityShader_7]);
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurVelocityShader_9]);
	Display::Device()->Destroy(&m_PixelShaders[E_MotionBlurVelocityShader_11]);
}

void C_MotionBlurVelocity::DestroyVolatile()
{
}

void C_MotionBlurVelocity::Update()
{
}

void C_MotionBlurVelocity::Display() const
{
	if (!m_Manager.Enabled())
		return;

	PERF_EVENT("MotionBlurVelocity");

	SetViewport();

	Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

	// TODO: change texture filter state based on velocity buffer format
	Display::Device()->SetTexture(1, m_pVelocityBuffer);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);

	const Display::C_Float4 vParameter(m_fScale * g_debugMotionBlurVelocityScale, m_fBias * g_debugMotionBlurVelocityScale, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_Parameters[m_eShader], (F32*)&vParameter);

	SetShader(&m_PixelShaders[m_eShader]);

	DisplayFinal();
}

void C_MotionBlurVelocity::SetPixelShader(const E_MotionBlurVelocityShader eShader) const
{
	m_eShader = eShader;
}

void C_MotionBlurVelocity::SetScaleBias(const F32 fScale, const F32 fBias) const
{
	m_fScale = fScale;
	m_fBias = fBias;
}

void C_MotionBlurVelocity::SetVelocityBuffer(const Display::C_Texture2D* pVelocityBuffer) const
{
	m_pVelocityBuffer = pVelocityBuffer;
}

}; // namespace PostProcess
