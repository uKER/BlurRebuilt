#include "stdafx.h"
#include "Effect/PostProcess/MotionBlurDoFId.h"

#include <Render/Shaders/Constants.h>
#include <Render/State/RenderState.h>
#include <Render/Utils/Globals.h>

namespace PostProcess
{

Debug::Float	g_debugMotionBlurDoFIdMaxVelocity	("Graphics.Render.PostProcess.MotionBlurDoFId.MaxVelocity",	0.1f, Debug::Range(0.0f, 1.0f));
Debug::Bool		g_debugMotionBlurDoFIdDownsample	("Graphics.Render.PostProcess.MotionBlurDoFId.Downsample",	true);
Debug::Bool		g_debugMotionBlurDoFIdBlur			("Graphics.Render.PostProcess.MotionBlurDoFId.Blur",		true);

C_MotionBlurDoFId::C_MotionBlurDoFId(C_Manager& manager) : C_MotionBlurDoFVelocity( manager ), m_VelocityBufferId( manager )
{
	m_VelocityBufferId.SetMethod(E_VelocityBufferIdMethod_Stencil);
}

C_MotionBlurDoFId::~C_MotionBlurDoFId()
{
}

bool C_MotionBlurDoFId::Create()
{
	if (!m_VelocityBufferId.Create())
		return false;

	return C_MotionBlurDoFVelocity::Create();
}

void C_MotionBlurDoFId::Update()
{
}

bool C_MotionBlurDoFId::CreateVolatile()
{
	if (!m_VelocityBufferId.CreateVolatile())
		return false;

	U32 iWidth = Display::Device()->GetBackBuffer().GetWidth();
	U32 iHeight = Display::Device()->GetBackBuffer().GetHeight();
	if (Manager().GetRenderTargetCount() > 0)
	{
		const C_RenderTarget& renderTarget = m_Manager.GetRenderTarget(0);
		iWidth = renderTarget.iWidth;
		iHeight = renderTarget.iHeight;
	}

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_VelocityRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 4, iHeight / 4, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_BlurRenderTexture))
		return false;
	
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_VelocityRenderTexture, "_MotionBlurDofVelocityRT");
	Display2::Device()->SetRenderTexture2DName(&m_BlurRenderTexture, "_MotionBlurDofBlurRT");
	#endif

#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 4, iHeight / 4, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_BlurXRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurXRenderTexture, "_MotionBlurDofBlurXRT");
	#endif
#endif

	return C_MotionBlurDoFVelocity::CreateVolatile();
}

void C_MotionBlurDoFId::Destroy()
{
	m_VelocityBufferId.Destroy();

	C_MotionBlurDoFVelocity::Destroy();
}

void C_MotionBlurDoFId::DestroyVolatile()
{
	Display::Device()->Destroy(&m_VelocityRenderTexture);
	Display::Device()->Destroy(&m_BlurRenderTexture);
#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
	Display::Device()->Destroy(&m_BlurXRenderTexture);
#endif

	m_VelocityBufferId.DestroyVolatile();

	C_MotionBlurDoFVelocity::DestroyVolatile();
}

const Display::C_Texture2D* C_MotionBlurDoFId::CreateVelocityBuffer() const
{
	if (!m_Manager.Enabled())
		return NULL;

	PERF_EVENT("CreateVelocityBuffer");

	m_VelocityBufferId.SetMethod(E_VelocityBufferIdMethod_Stencil);
	m_VelocityBufferId.SetMaxVelocity(g_debugMotionBlurDoFIdMaxVelocity);
	m_VelocityBufferId.SetVelocityBuffer(&m_VelocityRenderTexture);
	m_VelocityBufferId.Render();

	SetScaleBias(2.0f * g_debugMotionBlurDoFIdMaxVelocity, -1.0f * g_debugMotionBlurDoFIdMaxVelocity);

	if (g_debugMotionBlurDoFIdDownsample)
	{
		m_Downsample.Downsample(&m_BlurRenderTexture, &m_VelocityRenderTexture.GetTexture());
		if (g_debugMotionBlurDoFIdBlur)
		{
#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
			m_GaussianBlur.GaussBlurX(&m_BlurXRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_13);
			m_GaussianBlur.GaussBlurY(&m_BlurRenderTexture, &m_BlurXRenderTexture, Render::Texture::E_GaussianBlurShader_13);
#else
			m_GaussianBlur.GaussBlurX(&m_BlurRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_13);
			m_GaussianBlur.GaussBlurY(&m_BlurRenderTexture, &m_BlurRenderTexture, Render::Texture::E_GaussianBlurShader_13);
#endif
		}
		SetVelocityBuffer(&m_BlurRenderTexture.GetTexture());
		return &m_BlurRenderTexture.GetTexture();
	}
	else
	{
		SetVelocityBuffer(&m_VelocityRenderTexture.GetTexture());
		return &m_VelocityRenderTexture.GetTexture();
	}
}

void C_MotionBlurDoFId::SetMethod(const E_VelocityBufferIdMethod eMethod)
{
	m_VelocityBufferId.SetMethod(eMethod);
}

void C_MotionBlurDoFId::SetMatrix(const U32 id, const Maths::HMat44& mMovement)
{
	m_VelocityBufferId.SetMatrix(id, mMovement);
}

void C_MotionBlurDoFId::ClearMatrixPalette()
{
	m_VelocityBufferId.ClearMatrixPalette();
}

}; // namespace PostProcess
