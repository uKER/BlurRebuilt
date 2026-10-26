#include "stdafx.h"
#include "Effect/PostProcess/MotionBlur/MotionBlurId.h"

#include <Render/Shaders/Constants.h>
#include <Render/State/RenderState.h>
#include <Render/Utils/Globals.h>

namespace PostProcess
{

Debug::Float	g_debugMotionBlurIdMaxVelocity("Graphics.Effect.PostProcess.MotionBlurId.MaxVelocity", 0.1f, Debug::Range(0.0f, 1.0f));

C_MotionBlurId::C_MotionBlurId(C_Manager& manager) : I_PostProcess(manager), m_MotionBlurVelocity(manager), m_VelocityBufferId(manager)
{
	m_MotionBlurVelocity.SetChainable(E_Chainable_False);
	m_MotionBlurVelocity.SetResolve(E_Resolve_False);
	m_VelocityBufferId.SetMethod(E_VelocityBufferIdMethod_Stencil);
}

C_MotionBlurId::~C_MotionBlurId()
{
}

bool C_MotionBlurId::Create()
{
	if (!m_VelocityBufferId.Create())
		return false;

	return true;
}

bool C_MotionBlurId::CreateVolatile()
{
	if (!m_VelocityBufferId.CreateVolatile())
		return false;

	U32 iWidth = Display::Device()->GetBackBuffer().GetWidth();
	U32 iHeight = Display::Device()->GetBackBuffer().GetHeight();
	if (m_Manager.GetRenderTargetCount() > 0)
	{
		const C_RenderTarget& renderTarget = m_Manager.GetRenderTarget(0);
		iWidth = renderTarget.iWidth;
		iHeight = renderTarget.iHeight;
	}

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_VelocityRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_VelocityRenderTexture, "_MotionBlurVelocityRT");
	#endif

	if (!Display::Device()->CreateRenderTexture2D(iWidth / 4, iHeight / 4, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_BlurRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurRenderTexture, "_MotionBlurRT");
	#endif

#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 4, iHeight / 4, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_BlurXRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurXRenderTexture, "_MotionBlurXRT");
	#endif
#endif

	return true;
}

void C_MotionBlurId::Destroy()
{
	m_VelocityBufferId.Destroy();
}

void C_MotionBlurId::DestroyVolatile()
{
	Display::Device()->Destroy(&m_VelocityRenderTexture);
	Display::Device()->Destroy(&m_BlurRenderTexture);
#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
	Display::Device()->Destroy(&m_BlurXRenderTexture);
#endif

	m_VelocityBufferId.DestroyVolatile();
}

void C_MotionBlurId::Update()
{
}

void C_MotionBlurId::Display() const
{
	if (!m_Manager.Enabled())
		return;

	PERF_EVENT("MotionBlurId");

	SetViewport();

	m_VelocityBufferId.SetMethod(E_VelocityBufferIdMethod_Stencil);
	m_VelocityBufferId.SetMaxVelocity(g_debugMotionBlurIdMaxVelocity);
	m_VelocityBufferId.SetVelocityBuffer(&m_VelocityRenderTexture);
	m_VelocityBufferId.Render();

	// TODO: make this work on PS3 and PC (separate render targets)
	m_Downsample.Downsample(&m_BlurRenderTexture, &m_VelocityRenderTexture.GetTexture());
#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
	m_GaussianBlur.GaussBlurX(&m_BlurXRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_13);
	m_GaussianBlur.GaussBlurY(&m_BlurRenderTexture, &m_BlurXRenderTexture, Render::Texture::E_GaussianBlurShader_13);
#else
	m_GaussianBlur.GaussBlurX(&m_BlurRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_13);
	m_GaussianBlur.GaussBlurY(&m_BlurRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_13);
#endif

	// motion blur using the generated velocity buffer
	m_MotionBlurVelocity.SetPixelShader(E_MotionBlurVelocityShader_11);
	m_MotionBlurVelocity.SetScaleBias(2.0f * g_debugMotionBlurIdMaxVelocity, -1.0f * g_debugMotionBlurIdMaxVelocity);
	m_MotionBlurVelocity.SetVelocityBuffer(&m_BlurRenderTexture.GetTexture());
	m_MotionBlurVelocity.Display();
}

void C_MotionBlurId::SetMethod(const E_VelocityBufferIdMethod eMethod)
{
	m_VelocityBufferId.SetMethod(eMethod);
}

void C_MotionBlurId::SetMatrix(const U32 id, const Maths::HMat44& mMovement)
{
	m_VelocityBufferId.SetMatrix(id, mMovement);
}

void C_MotionBlurId::ClearMatrixPalette()
{
	m_VelocityBufferId.ClearMatrixPalette();
}

}; // namespace PostProcess
