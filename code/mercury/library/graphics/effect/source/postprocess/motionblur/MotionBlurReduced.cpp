#include "stdafx.h"
#include "Effect/PostProcess/MotionBlur/MotionBlurReduced.h"

#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Texture/Downsample.h"
#include "Render/Utils/Globals.h"

#include "System/Misc/Random.h"

EXTERN_SHADER(psMotionBlurReducedAlpha5);
EXTERN_SHADER(psMotionBlurReducedAlpha7);
EXTERN_SHADER(psMotionBlurReducedAlpha9);
EXTERN_SHADER(psMotionBlurReducedAlpha11);
EXTERN_SHADER(psMotionBlurReducedStencil5);
EXTERN_SHADER(psMotionBlurReducedStencil7);
EXTERN_SHADER(psMotionBlurReducedStencil9);
EXTERN_SHADER(psMotionBlurReducedStencil11);
EXTERN_SHADER(psMotionBlurReducedComposite);
EXTERN_SHADER(psMotionBlurCopyStencil);
EXTERN_SHADER(psMotionBlurDownsampleStencil);

namespace PostProcess
{

extern Debug::Bool g_debugMotionBlur;

extern Debug::Float g_debugMotionBlurShutterAngle;

extern Debug::Float g_debugMotionBlurNoiseScale;
extern Debug::Float g_debugMotionBlurCentreWeight;

extern const U32 g_MotionBlurNoiseSize;

Debug::Float	g_debugMotionBlurAlphaScale						("Graphics.Render.PostProcess.MotionBlur.AlphaScale",	256.0f,	Debug::Range(0.0f, 512.0f));
Debug::Int		g_debugMotionBlurDownsampleStencilRegisterCount	("Graphics.Render.PostProcess.MotionBlur.DownsampleStencilRegisterCount", 48, Debug::Range(2, 48));

extern Util::C_Random g_Random;

extern Debug::Logger& g_Log;

C_MotionBlurReduced::C_MotionBlurReduced(C_Manager& manager) : I_PostProcess(manager), m_bEnabled(true), m_bComposite(true), m_pTextureBuffer(0), m_iTextureBufferSize(0)
{
	m_TextureFilter = Display::E_TextureFilter_Point;
	SetMethod(E_MotionBlurMethod_Alpha);
	SetSamples(E_MotionBlurSamples_5);
}

C_MotionBlurReduced::~C_MotionBlurReduced()
{
}

bool C_MotionBlurReduced::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedAlpha5", 0, &m_BlurShader[0], REF_SHADER(psMotionBlurReducedAlpha5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedAlpha7", 0, &m_BlurShader[1], REF_SHADER(psMotionBlurReducedAlpha7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedAlpha9", 0, &m_BlurShader[2], REF_SHADER(psMotionBlurReducedAlpha9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedAlpha11", 0, &m_BlurShader[3], REF_SHADER(psMotionBlurReducedAlpha11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedStencil5", 0, &m_BlurShader[4], REF_SHADER(psMotionBlurReducedStencil5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedStencil7", 0, &m_BlurShader[5], REF_SHADER(psMotionBlurReducedStencil7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedStencil9", 0, &m_BlurShader[6], REF_SHADER(psMotionBlurReducedStencil9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedStencil11", 0, &m_BlurShader[7], REF_SHADER(psMotionBlurReducedStencil11)))
		return false;
	
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurCopyStencil", 0, &m_CopyStencilShader, REF_SHADER(psMotionBlurCopyStencil)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDownsampleStencil", 0, &m_DownsampleStencilShader, REF_SHADER(psMotionBlurDownsampleStencil)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurReducedComposite", 0, &m_CompositeShader, REF_SHADER(psMotionBlurReducedComposite)))
		return false;

	for (U32 shaderNo = 0; shaderNo < kMotionBlurShaderCount; ++shaderNo)
	{
		m_MatrixParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_mMatrix", 4);
		m_DepthScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_vDepthScale");
		m_StepScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_5, "g_vStepScale");
		m_SettingsParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_6, "g_vSettings");
		m_NoiseParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_7, "g_vNoise");
	}

	// noise texture
	{
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
	}

	// black texture
	{
		if (!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_BlackTexture))
			return false;

		Display::C_LockedRect rect = m_BlackTexture.Lock(0);
		if(rect.pBits)
		{
			U32 *pData = reinterpret_cast<U32*>(rect.pBits);
			*pData = 0x00000000;
		}
		m_BlackTexture.Unlock(0);

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_BlackTexture, "_MotionBlurBlack");
		#endif
	}

	return true;
}

bool C_MotionBlurReduced::CreateVolatile()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::C_SurfaceParams surfaceParams;

	U32 flags = Display::E_Usage_Colour | Display::E_Usage_ReadWrite | Display::E_Usage_Expand;
	if(m_pTextureBuffer)
	{
		flags |= Display::E_Usage_Indirect;
		const U32 iMaximumTextureSize = m_Manager.GetMaximumTextureSize(2, 2);
		if(iMaximumTextureSize > m_iTextureBufferSize)
		{
			LOGERROR(g_Log, "Texture buffer size not large enough\n");
			return false;
		}
	}

	if(!m_Manager.CreateUserRenderTextures(2, 2, flags, Display::E_Format_Unknown, &surfaceParams, m_RenderTextures, m_pTextureBuffer, m_iTextureBufferSize))
		return false;

	return true;
}

void C_MotionBlurReduced::Destroy()
{
	for (U32 shaderNo = 0; shaderNo < kMotionBlurShaderCount; ++shaderNo)
	{
		Display::Device()->Destroy(&m_BlurShader[shaderNo]);
	}

	Display::Device()->Destroy(&m_CompositeShader);
	Display::Device()->Destroy(&m_CopyStencilShader);
	Display::Device()->Destroy(&m_DownsampleStencilShader);

	Display::Device()->Destroy(&m_BlackTexture);
	Display::Device()->Destroy(&m_NoiseTexture);
}

void C_MotionBlurReduced::DestroyVolatile()
{
	m_Manager.DestroyUserRenderTextures(m_RenderTextures);
}

void C_MotionBlurReduced::SetSamples(const E_MotionBlurSamples eSamples) const
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

void C_MotionBlurReduced::SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize)
{
	m_pTextureBuffer = pTextureBuffer;
	m_iTextureBufferSize = iTextureBufferSize;
}

U32 C_MotionBlurReduced::GetTextureBufferRequiredSize() const
{
#if defined(PLATFORM_PS3)
	return m_Manager.GetMaximumTextureSize(2, 2) * 2;
#else
	return m_Manager.GetMaximumTextureSize(2, 2);
#endif
}

void C_MotionBlurReduced::Display() const
{
	// [Blur2PC] T107: the snapshot returned here unconditionally on PC ("this is broken on PC"); retail PC
	// (Blur.exe 1.2, 0x00668d10) runs the pass under the same test as the consoles
	if(!m_Manager.Enabled() || !g_debugMotionBlur || !m_bEnabled)
		return;

	const S32 iRenderTarget = m_Manager.GetRenderTargetIndex();
	if(iRenderTarget < 0 || iRenderTarget >= m_RenderTextures.Size())
		return;
	
	Display::C_RenderTexture2D* pRenderTexture = m_RenderTextures[iRenderTarget];

	PERF_EVENT("MotionBlur");

#if defined(PLATFORM_PC)
	// [Blur2PC] T107: retail PC (0x00668d59, 0x00668d64) resets the device's state cache first
	Display::Device()->InvalidateState();
	Display::Device()->DefaultStates();
#endif

	m_nShader = m_eMethod * E_MotionBlurSamples_Count + m_eSamples;
	SetShader(&m_BlurShader[m_nShader]);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

#if defined(PLATFORM_PC)
	// [Blur2PC] T107: retail PC (0x00668d92) calls I_PostProcess::SetViewport here, which resolves the frame
	// into the manager's render texture and makes it the chain texture that DownsampleStencilBufferToAlpha
	// reads. Without it the chain texture still held the previous frame's last bloom level, and the pass
	// blended the near road toward black.
	SetViewport();
#endif

	// downsample
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	if (E_MotionBlurMethod_Alpha == m_eMethod)
	{
		Display::Device()->SetColourDepthStencilSurface(&pRenderTexture->GetSurface(), 0);
		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = pRenderTexture->GetSurface().GetWidth();
		viewport.m_H = pRenderTexture->GetSurface().GetHeight();
		Display::Device()->SetViewport(viewport);
		DownsampleStencilBufferToAlpha();
		pRenderTexture->Resolve();
	}
	else
	{
		m_Downsample.Downsample(pRenderTexture, m_Manager.GetMainTexture());
	}

	Display::Device()->SetColourDepthStencilSurface(&pRenderTexture->GetSurface(), 0);

	// motion blur
	SetConstants();

	SetTextureUnexpanded(0, &pRenderTexture->GetTexture());
	//Display::Device()->SetTexture(0, &pRenderTexture->GetTexture());
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


	if (E_MotionBlurMethod_Stencil == m_eMethod)
	{
	#if defined(PLATFORM_PS3)
	
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

		const Display::C_TextureControl textureControl(CELL_GCM_TRUE, 0, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
		Display::Device()->SetTextureControlUncached(2, textureControl);

	#endif

		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);
	}

	DisplayFinal();

	pRenderTexture->Resolve();

	// composite
	m_Manager.SetViewport();

	if(m_bComposite)
	{
		Display::Device()->SetTexture(0, &pRenderTexture->GetTexture());
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

		Display::Device()->SetPixelShader(&m_CompositeShader);

		Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);

		Display::Quad::DrawFullScreen();

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	}

	// reset states
	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	if (E_MotionBlurMethod_Stencil == m_eMethod)
	{
		Display::Device()->SetTexture(2, 0);
		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);
	}

	Display::Device()->SetTexture(3, 0);
	Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(3, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(3, Display::E_Bool_Default);
}

void C_MotionBlurReduced::SetConstants() const
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
			mTemp._41 = -1.0f;
			mTemp._22 = -2.0f;
			mTemp._42 = 1.0f;
			
			mTexCoord0 = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// second matrix takes into account field of view and aspect ratio to get position in view space
		Maths::HMat44 mFovAspect;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);

			Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

//			mTemp._11 = 1.f / viewSettings3D.GetProjMatrix()._11;
//			mTemp._22 = 1.f / viewSettings3D.GetProjMatrix()._22;

			// if the viewport has been flipped we need to negate these
//			if(m_Manager.GetView().bFlip)
//			{
//				mTemp._11 *= -1.0f;
//				mTemp._22 *= -1.0f;
//			}

			mFovAspect = mProjection.Inverse();//Maths::Load<Maths::HMat44>(&mTemp);
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
		MathsStorage::Float4 fvec(g_debugMotionBlurNoiseScale, g_debugMotionBlurCentreWeight, g_debugMotionBlurAlphaScale, 0.0f);
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

void C_MotionBlurReduced::CopyStencilBufferToAlpha() const
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

void C_MotionBlurReduced::DownsampleStencilBufferToAlpha() const
{
	Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);

#if defined(PLATFORM_PC)
	// [Blur2PC] T107: retail PC (Blur.exe 1.2, 0x0065d680) reads the stencil from the display window's stencil
	// texture (Render::GetGlobalStencilBuffer, 0x006a1940; T089), not the depth buffer
	Display::Device()->SetTexture(1, Render::GetGlobalStencilBuffer());
#else
	Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());	
#endif

	Display::Device()->SetPixelShader(&m_DownsampleStencilShader);
	const_cast<Display::C_PixelShader*>(&m_DownsampleStencilShader)->SetRegisterCount(g_debugMotionBlurDownsampleStencilRegisterCount);

	Display::Quad::DrawFullScreen();

}

Display::C_Texture2D* C_MotionBlurReduced::GetTexture()
{
	const S32 iRenderTarget = m_Manager.GetRenderTargetIndex();
	if(g_debugMotionBlur && iRenderTarget >= 0 && iRenderTarget < m_RenderTextures.Size())
		return &m_RenderTextures[iRenderTarget]->GetTexture();
	else
		return &m_BlackTexture;
}

}; // namespace PostProcess
