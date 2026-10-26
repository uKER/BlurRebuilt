#include "stdafx.h"
#include "Effect/PostProcess/MotionBlur/MotionBlur.h"

#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

#include "System/Misc/Random.h"

EXTERN_SHADER(psMotionBlurAlpha5);
EXTERN_SHADER(psMotionBlurAlpha7);
EXTERN_SHADER(psMotionBlurAlpha9);
EXTERN_SHADER(psMotionBlurAlpha11);
EXTERN_SHADER(psMotionBlurStencil5);
EXTERN_SHADER(psMotionBlurStencil7);
EXTERN_SHADER(psMotionBlurStencil9);
EXTERN_SHADER(psMotionBlurStencil11);
EXTERN_SHADER(psMotionBlurCopyStencil);
EXTERN_SHADER(psMotionBlurWriteMask);

namespace PostProcess
{

#ifdef PLATFORM_PC
// [Blur2PC] T107: retail PC (Blur.exe 1.2) registers it on (0x00e19cb0, global 0x0114c7fc); the snapshot had it off
Debug::Bool		g_debugMotionBlur						("Graphics.Render.PostProcess.MotionBlur.Enable", true);
#else
Debug::Bool		g_debugMotionBlur						("Graphics.Render.PostProcess.MotionBlur.Enable", true);
#endif

Debug::Float	g_debugMotionBlurShutterAngle			("Graphics.Render.PostProcess.MotionBlur.ShutterAngle", 1.0f, Debug::Range(0.0f, 1.0f));

Debug::Bool		g_debugMotionBlurUseAlternateBuffer		("Graphics.Render.PostProcess.MotionBlur.UseAlternateBuffer", false);

Debug::Float	g_debugMotionBlurNoiseScale				("Graphics.Render.PostProcess.MotionBlur.NoiseScale", 1.0f, Debug::Range(0.0f, 5.0f));
Debug::Float	g_debugMotionBlurCentreWeight			("Graphics.Render.PostProcess.MotionBlur.CentreWeight", 0.001f, Debug::Range(0.0f, 5.0f));

#ifndef CFG_SPEED
Debug::Bool		g_debugMotionBlurUseRandomTaps			("Graphics.Render.PostProcess.MotionBlur.UseRandomTaps", false);
#endif

extern const U32 g_MotionBlurNoiseSize;

extern Util::C_Random g_Random;

C_MotionBlur::C_MotionBlur(C_Manager& manager) : I_PostProcess(manager), m_bEnabled(true)
{
	m_TextureFilter = Display::E_TextureFilter_Point;
	SetMethod(E_MotionBlurMethod_Alpha);
	SetSamples(E_MotionBlurSamples_5);
}

C_MotionBlur::~C_MotionBlur()
{
}

bool C_MotionBlur::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurAlpha5", 0, &m_BlurShader[0], REF_SHADER(psMotionBlurAlpha5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurAlpha7", 0, &m_BlurShader[1], REF_SHADER(psMotionBlurAlpha7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurAlpha9", 0, &m_BlurShader[2], REF_SHADER(psMotionBlurAlpha9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurAlpha11", 0, &m_BlurShader[3], REF_SHADER(psMotionBlurAlpha11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurStencil5", 0, &m_BlurShader[4], REF_SHADER(psMotionBlurStencil5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurStencil7", 0, &m_BlurShader[5], REF_SHADER(psMotionBlurStencil7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurStencil9", 0, &m_BlurShader[6], REF_SHADER(psMotionBlurStencil9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurStencil11", 0, &m_BlurShader[7], REF_SHADER(psMotionBlurStencil11)))
		return false;
	
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurCopyStencil", 0, &m_CopyStencilShader, REF_SHADER(psMotionBlurCopyStencil)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurWriteMask", 0, &m_WriteMaskShader, REF_SHADER(psMotionBlurWriteMask)))
		return false;

	for (U32 shaderNo = 0; shaderNo < kMotionBlurShaderCount; ++shaderNo)
	{
		m_MatrixParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_mMatrix", 4);
		m_DepthScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_vDepthScale");
		m_StepScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_5, "g_vStepScale");
		m_SettingsParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_6, "g_vSettings");
		m_NoiseParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_7, "g_vNoise");
	}

	if (!Display::Device()->CreateTexture2D(g_MotionBlurNoiseSize, g_MotionBlurNoiseSize, 1, Display::E_Format_Linear, Display::E_Format_L8, &m_NoiseTexture))
		return false;

	Display::C_LockedRect lockedRect = m_NoiseTexture.Lock(0);
	if (lockedRect.pBits)
	{
		U8* pData = lockedRect.pBits;
		for (U32 y = 0; y < g_MotionBlurNoiseSize; ++y)
		{
			for (U32 x = 0; x < g_MotionBlurNoiseSize; ++x)
				pData[x] = (U8)PostProcess::g_Random.Rand(255);
			pData += lockedRect.pitch;
		}
		m_NoiseTexture.Unlock(0);
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(&m_NoiseTexture, "_MotionBlurNoise");
	#endif

	return true;
}

bool C_MotionBlur::CreateVolatile()
{
	if (g_debugMotionBlurUseAlternateBuffer)
		return CreateRenderTarget();

	return true;
}

void C_MotionBlur::Destroy()
{
	for (U32 shaderNo = 0; shaderNo < kMotionBlurShaderCount; ++shaderNo)
	{
		Display::Device()->Destroy(&m_BlurShader[shaderNo]);
	}

	Display::Device()->Destroy(&m_CopyStencilShader);
	Display::Device()->Destroy(&m_WriteMaskShader);

	Display::Device()->Destroy(&m_NoiseTexture);
}

void C_MotionBlur::DestroyVolatile()
{
	if (g_debugMotionBlurUseAlternateBuffer)
		DestroyRenderTarget();	
}

void C_MotionBlur::SetSamples(const E_MotionBlurSamples eSamples) const
{
	m_eSamples = eSamples;
	switch (m_eSamples)
	{
		case E_MotionBlurSamples_5:  m_nSamples = 5;  break;
		case E_MotionBlurSamples_7:  m_nSamples = 7;  break;
		case E_MotionBlurSamples_9:  m_nSamples = 9;  break;
		case E_MotionBlurSamples_11: m_nSamples = 11; break;
	}

	m_fSamplesMinusOneRcp = 1.0f / (F32)(m_nSamples - 1);
}

void C_MotionBlur::Display() const
{
	if (!m_Manager.Enabled() || !g_debugMotionBlur || !m_bEnabled)
		return;

	PERF_EVENT("MotionBlur");

	m_nShader = m_eMethod * E_MotionBlurSamples_Count + m_eSamples;
	SetShader(&m_BlurShader[m_nShader]);

	if (E_MotionBlurMethod_Alpha == m_eMethod)
	{
		CopyStencilBufferToAlpha();
	}

	SetViewport();
	SetConstants();
	
	Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	Display::Device()->SetTexture(3, &m_NoiseTexture);
	Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilter(3, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(3, Display::E_Bool_False);

	if (g_debugMotionBlurUseAlternateBuffer && m_ColourMaskRenderTexture.IsCreated())
	{
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Red | Display::E_ColourWriteEnable_Blue | Display::E_ColourWriteEnable_Green | Display::E_ColourWriteEnable_Alpha);
		m_Manager.SetRenderTexture(m_ColourMaskRenderTexture);
		DisplayIntermediate(&m_WriteMaskShader);

		Resolve();
		m_Manager.SetViewport();
		Display::Device()->SetTexture(0, &m_ColourMaskRenderTexture.GetTexture());
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	}

	if (E_MotionBlurMethod_Stencil == m_eMethod)
	{
	#if PLATFORM_PS3
	
		CellGcmTexture* pSrcGcmTexture = (CellGcmTexture*) Render::GetGlobalDepthStencilBuffer()->GetGPUTexture();

		CellGcmTexture gcmTexture;
		gcmTexture.format		= CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
		gcmTexture.mipmap		= 1;
		gcmTexture.dimension	= CELL_GCM_TEXTURE_DIMENSION_2;
		gcmTexture.cubemap		= CELL_GCM_FALSE;
		gcmTexture.remap		=	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
									CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
									CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
									CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
									CELL_GCM_TEXTURE_REMAP_FROM_A;
		gcmTexture.width		= pSrcGcmTexture->width;
		gcmTexture.height		= pSrcGcmTexture->height;
		gcmTexture.depth		= pSrcGcmTexture->depth;
		gcmTexture.location		= pSrcGcmTexture->location;
		gcmTexture.pitch		= pSrcGcmTexture->pitch;
		gcmTexture.offset		= pSrcGcmTexture->offset;
		
		Display::Device()->SetTextureUncached(2, &gcmTexture);

	#endif

		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);
	}

	DisplayFinal();

	if (E_MotionBlurMethod_Stencil == m_eMethod)
	{
		Display::Device()->SetTexture(2, 0);
		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);
	}

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Display::Device()->SetTexture(3, 0);
	Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(3, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(3, Display::E_Bool_Default);
}

void C_MotionBlur::SetConstants() const
{
	const Display::C_View* pView = Display::Device()->GetView();
	const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	// first we compute our motion blur matrix
	{
		// first matrix transforms from uv * depth space to projected space
		Maths::HMat44 mTexCoord0;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);
			
			mTemp._11 = 2.0f;
			mTemp._31 = -1.0f;
			mTemp._22 = -2.0f;
			mTemp._32 = 1.0f;
			
			mTexCoord0 = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// second matrix takes into account field of view and aspect ratio to get position in view space
		Maths::HMat44 mFovAspect;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);

			Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

			mTemp._11 = 1.f / viewSettings3D.GetProjMatrix()._11;
			mTemp._22 = 1.f / viewSettings3D.GetProjMatrix()._22;

			// if the viewport has been flipped we need to negate these
			if(m_Manager.GetView().bFlip)
			{
				mTemp._11 *= -1.0f;
				mTemp._22 *= -1.0f;
			}

			mFovAspect = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// third matrix is inverse view times previous matrix
		Maths::HMat44 mMove;
		{
			Maths::HMat44 mViewInverse(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix()));
			Maths::HMat44 mViewPrev(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrixPrev()));
			mMove = mViewInverse.Inverse() * mViewPrev;

			// make sure pixel always gets projected into screen not outside it
			Maths::HVec4 vZero;
			vZero.W(Maths::Constants<Maths::HF32>::One());

			vZero = vZero * mMove;
			m_vMotionAmount = Maths::HVec3(vZero);
			if(vZero.Z() < Maths::Constants<Maths::HF32>::Zero())
			{
				mMove = mMove.Inverse();
			}
		}

		// fourth matrix is just the projection matrix
		Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrixPrev()));

		// fifth matrix is projected space to texture * depth space transform (sorta - depth in W)
		Maths::HMat44 mTexCoord1;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);
			mTemp._11 = 0.5f;
			mTemp._41 = 0.5f;
			mTemp._22 = -0.5f;
			mTemp._42 = 0.5f;

			mTexCoord1 = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// combine them all together to get final matrix
		Maths::HMat44 mFinal;
		mFinal = mFinal * mTexCoord0;
		mFinal = mFinal * mFovAspect;
		mFinal = mFinal * mMove;
		mFinal = mFinal * mProjection;
		mFinal = mFinal * mTexCoord1;

		Display::C_Matrix mFinalStore;
		Maths::Store(mFinal, &mFinalStore);

		Display::Device()->SetPixelShaderParameterArrayF(m_MatrixParameter[m_nShader], reinterpret_cast<F32 *>(&mFinalStore));
	}

	// now compute scaling factors to get linear depth
	{
		MathsStorage::Float4 fvec;

		F32 zNear;
		F32 zFar;

		if (Display::Device()->GetDepthFlip())
		{
			zNear = viewSettings3D.GetFarClip();
			zFar = viewSettings3D.GetNearClip();
		}
		else
		{
			zNear = viewSettings3D.GetNearClip();
			zFar = viewSettings3D.GetFarClip();
		}

		fvec.x = zFar / (zFar - zNear);
		fvec.y = zFar * zNear / (zNear - zFar);
		fvec.z = 0.0f;
		fvec.w = 0.0f;

		Display::Device()->SetPixelShaderParameterF(m_DepthScaleParameter[m_nShader], (F32*)&fvec);
	}

	{
		F32 fShutterAngle = g_debugMotionBlurShutterAngle;
		F32 fShutterAngleScale = 1.0f; 
		F32 fStep = fShutterAngle * fShutterAngleScale * m_fSamplesMinusOneRcp;

		MathsStorage::Float4 fvec(fStep, fStep, -fStep, -fStep);
		Display::Device()->SetPixelShaderParameterF(m_StepScaleParameter[m_nShader], (F32*)&fvec);
	}

	{
		MathsStorage::Float4 fvec(g_debugMotionBlurNoiseScale, g_debugMotionBlurCentreWeight, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[m_nShader], (F32*)&fvec);
	}

	{
		float fWidth = (F32)pView->GetViewport().GetW();
		float fHeight = (F32)pView->GetViewport().GetH();
		float fOffsetX = g_Random.FRand(0.0f, 1.0f);
		float fOffsetY = g_Random.FRand(0.0f, 1.0f);

		MathsStorage::Float4 fvec(fWidth / (F32)g_MotionBlurNoiseSize, fHeight / (F32)g_MotionBlurNoiseSize, fOffsetX, fOffsetY);
		Display::Device()->SetPixelShaderParameterF(m_NoiseParameter[m_nShader], (F32*)&fvec);
	}
}

void C_MotionBlur::CopyStencilBufferToAlpha() const
{
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Alpha);

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	Display::Device()->SetTexture(0, Render::GetGlobalDepthStencilBuffer());	

	Display::Device()->SetPixelShader(&m_CopyStencilShader);

	Display::Quad::DrawFullScreen();

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);


	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
}

bool C_MotionBlur::CreateRenderTarget()
{
	if (m_ColourMaskRenderTexture.IsCreated())
		return true;

	if (m_Manager.GetRenderTargetCount() > 0)
	{
		const C_RenderTarget& renderTarget = m_Manager.GetRenderTarget(0);

		Display::E_Format format = Display::GetFormatFromSurfaceFormat(renderTarget.m_RenderTextures[0]->GetSurface().GetFormat());
		if (!Display::Device()->CreateRenderTexture2D(renderTarget.iWidth, renderTarget.iHeight, 1, Display::E_Usage_Colour, format, Display::E_Multisample_None, NULL, &m_ColourMaskRenderTexture))
		{
			return false;
		}

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(&m_ColourMaskRenderTexture, "_MotionBlurColourMaskRT");
		#endif
	}

	return true;
}

void C_MotionBlur::DestroyRenderTarget()
{
	if (m_ColourMaskRenderTexture.IsCreated())
	{
		Display::Device()->Destroy(&m_ColourMaskRenderTexture);
	}
}

}; // namespace PostProcess
