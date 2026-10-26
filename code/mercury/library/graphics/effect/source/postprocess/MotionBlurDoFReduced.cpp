#include "stdafx.h"
#include "Effect/PostProcess/MotionBlurDoFReduced.h"

#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Texture/Downsample.h"
#include "Render/Utils/Globals.h"

#include "System/Misc/Random.h"

EXTERN_SHADER(psMotionBlurDoFReducedAlpha5);
EXTERN_SHADER(psMotionBlurDoFReducedAlpha7);
EXTERN_SHADER(psMotionBlurDoFReducedAlpha9);
EXTERN_SHADER(psMotionBlurDoFReducedAlpha11);
EXTERN_SHADER(psMotionBlurDoFReducedStencil5);
EXTERN_SHADER(psMotionBlurDoFReducedStencil7);
EXTERN_SHADER(psMotionBlurDoFReducedStencil9);
EXTERN_SHADER(psMotionBlurDoFReducedStencil11);
EXTERN_SHADER(psMotionBlurDoFReducedComposite);
EXTERN_SHADER(psMotionBlurDoFCopyStencil);
EXTERN_SHADER(psMotionBlurDoFDownsampleStencil);
EXTERN_SHADER(psMotionBlurDoFWriteMask);

namespace PostProcess
{

extern Debug::Bool g_debugMotionBlurDoF;
extern Debug::Bool g_debugMotionBlurDoFBlurDynamicObjects;

extern Debug::Bool g_debugMotionBlurDoFEnableDoF;

extern Debug::Float g_debugMotionBlurDoFBlurScale;
extern Debug::Float g_debugMotionBlurDoFFocalLength;
extern Debug::Float g_debugMotionBlurDoFMaxCoC;
extern Debug::Float g_debugMotionBlurDoFEdgeFactor;

extern Debug::Bool g_debugMotionBlurDoFUseAlternateBuffer;

extern Debug::Float g_debugMotionBlurDoFNoiseScale;
extern Debug::Float g_debugMotionBlurDoFCentreWeight;

extern Debug::Float g_debugMotionBlurDoFShutterAngle;

extern const U32 g_MotionBlurNoiseSize;

Debug::Float	g_debugMotionBlurDoFAlphaMotion	("Graphics.Render.PostProcess.MotionBlurDoF.MotionAlpha",	256.0f,	Debug::Range(0.0f, 512.0f));
Debug::Float	g_debugMotionBlurDoFAlphaDoF	("Graphics.Render.PostProcess.MotionBlurDoF.DoFAlpha",		0.20f,	Debug::Range(0.0f, 1.0f));

extern Util::C_Random g_Random;

extern Debug::Logger& g_Log;

C_MotionBlurDoFReduced::C_MotionBlurDoFReduced(C_Manager& manager) : I_PostProcess(manager), m_bEnabled(true), m_bComposite(true), m_pTextureBuffer(0), m_iTextureBufferSize(0)
{
	m_TextureFilter = Display::E_TextureFilter_Point;
	SetMethod(E_MotionBlurDoFMethod_Alpha);
	SetSamples(E_MotionBlurDoFSamples_5);
}

C_MotionBlurDoFReduced::~C_MotionBlurDoFReduced()
{
}

bool C_MotionBlurDoFReduced::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedAlpha5", 0, &m_BlurShader[0], REF_SHADER(psMotionBlurDoFReducedAlpha5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedAlpha7", 0, &m_BlurShader[1], REF_SHADER(psMotionBlurDoFReducedAlpha7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedAlpha9", 0, &m_BlurShader[2], REF_SHADER(psMotionBlurDoFReducedAlpha9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedAlpha11", 0, &m_BlurShader[3], REF_SHADER(psMotionBlurDoFReducedAlpha11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedStencil5", 0, &m_BlurShader[4], REF_SHADER(psMotionBlurDoFReducedStencil5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedStencil7", 0, &m_BlurShader[5], REF_SHADER(psMotionBlurDoFReducedStencil7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedStencil9", 0, &m_BlurShader[6], REF_SHADER(psMotionBlurDoFReducedStencil9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedStencil11", 0, &m_BlurShader[7], REF_SHADER(psMotionBlurDoFReducedStencil11)))
		return false;
	
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFCopyStencil", 0, &m_CopyStencilShader, REF_SHADER(psMotionBlurDoFCopyStencil)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFDownsampleStencil", 0, &m_DownsampleStencilShader, REF_SHADER(psMotionBlurDoFDownsampleStencil)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFWriteMask", 0, &m_WriteMaskShader, REF_SHADER(psMotionBlurDoFWriteMask)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFReducedComposite", 0, &m_CompositeShader, REF_SHADER(psMotionBlurDoFReducedComposite)))
		return false;

	for (U32 shaderNo = 0; shaderNo < kMotionBlurDoFShaderCount; ++shaderNo)
	{
		m_MatrixParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_mMatrix", 4);
		m_DepthScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_vDepthScale");
		m_StepScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_5, "g_vStepScale");
		m_PixelScaleParameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_6, "g_vPixelScale");
		m_Settings1Parameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_7, "g_vSettings1");
		m_Settings2Parameter[shaderNo]	= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_8, "g_vSettings2");
		m_NoiseParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_9, "g_vNoise");
		m_FilterParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_10, "g_vFilter", 6);
		m_AlphaParameter[shaderNo]		= m_BlurShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_16, "g_vAlpha");
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

bool C_MotionBlurDoFReduced::CreateVolatile()
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

	if (!m_Manager.CreateUserRenderTextures(2, 2, flags, Display::E_Format_Unknown, &surfaceParams, m_ReducedRenderTextures, m_pTextureBuffer, m_iTextureBufferSize))
		return false;

	if (g_debugMotionBlurDoFUseAlternateBuffer)
		return CreateRenderTarget();

	return true;
}

void C_MotionBlurDoFReduced::Destroy()
{
	for (U32 shaderNo = 0; shaderNo < kMotionBlurDoFShaderCount; ++shaderNo)
	{
		Display::Device()->Destroy(&m_BlurShader[shaderNo]);
	}

	Display::Device()->Destroy(&m_CompositeShader);
	Display::Device()->Destroy(&m_CopyStencilShader);
	Display::Device()->Destroy(&m_DownsampleStencilShader);
	Display::Device()->Destroy(&m_WriteMaskShader);

	Display::Device()->Destroy(&m_BlackTexture);
	Display::Device()->Destroy(&m_NoiseTexture);
}

void C_MotionBlurDoFReduced::DestroyVolatile()
{
	m_Manager.DestroyUserRenderTextures(m_ReducedRenderTextures);

	if (g_debugMotionBlurDoFUseAlternateBuffer)
		DestroyRenderTarget();	
}

void C_MotionBlurDoFReduced::SetSamples(const E_MotionBlurDoFSamples eSamples) const
{
	m_eSamples = eSamples;
	switch (m_eSamples)
	{
		case E_MotionBlurDoFSamples_5:  m_nSamples = 5;  break;
		case E_MotionBlurDoFSamples_7:  m_nSamples = 7;  break;
		case E_MotionBlurDoFSamples_9:  m_nSamples = 9;  break;
		case E_MotionBlurDoFSamples_11: m_nSamples = 11; break;
	}

	m_fSamplesMinusOneRcp = 1.0f / (F32)(m_nSamples - 1);
}

void C_MotionBlurDoFReduced::SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize)
{
	m_pTextureBuffer = pTextureBuffer;
	m_iTextureBufferSize = iTextureBufferSize;
}

U32 C_MotionBlurDoFReduced::GetTextureBufferRequiredSize() const
{
#if defined(PLATFORM_PS3)
	return m_Manager.GetMaximumTextureSize(2, 2) * 2;
#else
	return m_Manager.GetMaximumTextureSize(2, 2);
#endif
}

void C_MotionBlurDoFReduced::Display() const
{
	// this is broken on PC
#if !defined(PLATFORM_PC)
	if(!m_Manager.Enabled() || !g_debugMotionBlurDoF || !m_bEnabled)
#endif
		return;

	const S32 iRenderTarget = m_Manager.GetRenderTargetIndex();
	if(iRenderTarget < 0 || iRenderTarget >= m_ReducedRenderTextures.Size())
		return;

	Display::C_RenderTexture2D* pReducedRenderTexture = m_ReducedRenderTextures[iRenderTarget];
	
	PERF_EVENT("MotionBlurDoF");

	m_nShader = m_eMethod * E_MotionBlurDoFSamples_Count + m_eSamples;
	SetShader(&m_BlurShader[m_nShader]);

	// downsample
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	if (E_MotionBlurDoFMethod_Alpha == m_eMethod)
	{
		Display::Device()->SetColourDepthStencilSurface(&pReducedRenderTexture->GetSurface(), 0);
		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = pReducedRenderTexture->GetSurface().GetWidth();
		viewport.m_H = pReducedRenderTexture->GetSurface().GetHeight();
		Display::Device()->SetViewport(viewport);
		DownsampleStencilBufferToAlpha();
		pReducedRenderTexture->Resolve();
	}
	else
	{
		m_Downsample.Downsample(pReducedRenderTexture, m_Manager.GetMainTexture());
	}

	Display::Device()->SetColourDepthStencilSurface(&pReducedRenderTexture->GetSurface(), 0);

	// motion blur
	SetConstants();

	SetTextureUnexpanded(0, &pReducedRenderTexture->GetTexture());
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

	if (g_debugMotionBlurDoFUseAlternateBuffer && m_ColourMaskRenderTextures.Size() > 0)
	{
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Red | Display::E_ColourWriteEnable_Blue | Display::E_ColourWriteEnable_Green | Display::E_ColourWriteEnable_Alpha);
		m_Manager.SetRenderTexture(*m_ColourMaskRenderTextures[iRenderTarget]);
		DisplayIntermediate(&m_WriteMaskShader);

		Resolve();
		m_Manager.SetViewport();
		Display::Device()->SetTexture(0, &m_ColourMaskRenderTextures[iRenderTarget]->GetTexture());
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	}


	if (E_MotionBlurDoFMethod_Stencil == m_eMethod)
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

	#endif

		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);
	}

	DisplayFinal();

	pReducedRenderTexture->Resolve();

	// composite
	m_Manager.SetViewport();

	if(m_bComposite)
	{
		Display::Device()->SetTexture(0, &pReducedRenderTexture->GetTexture());
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

	if (E_MotionBlurDoFMethod_Stencil == m_eMethod)
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

void C_MotionBlurDoFReduced::SetConstants() const
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

	// step factor
	{
		F32 fShutterAngle = g_debugMotionBlurDoFShutterAngle;
		F32 fShutterAngleScale = 1.0f; 
		F32 fStep = fShutterAngle * fShutterAngleScale * m_fSamplesMinusOneRcp;

		MathsStorage::Float4 fvec(fStep, fStep, -fStep, -fStep);
		Display::Device()->SetPixelShaderParameterF(m_StepScaleParameter[m_nShader], (F32*)&fvec);
	}

	// pixel factor
	{
		F32 viewWidth = 1280;
		if(!Display::Device()->GetWideScreen())
			viewWidth = 960;
		F32 viewHeight = 720;

		F32 fWidth = viewWidth;
		F32 fHeight = viewHeight;

		F32 fWidth2 = 1.0f / viewWidth;
		F32 fHeight2 = 1.0f / viewHeight;
		
		if(!g_debugMotionBlurDoFBlurDynamicObjects)
		{
			fWidth = 0.0f;
			fHeight = 0.0f;
		}
		else
		{
			fWidth = 1.0f / fWidth;
			fHeight = 1.0f / fHeight;
		}

		MathsStorage::Float4 fvec(fWidth, fHeight, fWidth2, fHeight2);
		Display::Device()->SetPixelShaderParameterF(m_PixelScaleParameter[m_nShader], (F32*)&fvec);
	}

	{
		float fAperture = GetAperture(m_Manager.GetViewIndex());
		float fPlaneInFocus = GetPlaneInFocus(m_Manager.GetViewIndex());

		float fNoiseScale = g_debugMotionBlurDoFNoiseScale;
		float fCentreWeight = g_debugMotionBlurDoFCentreWeight;

		MathsStorage::Float4 fvec(fAperture, fPlaneInFocus, fNoiseScale, fCentreWeight);
		Display::Device()->SetPixelShaderParameterF(m_Settings1Parameter[m_nShader], (F32*)&fvec);
	}
	{
		float fMaxCoC = g_debugMotionBlurDoFMaxCoC;
		float fEdgeFactor = g_debugMotionBlurDoFEdgeFactor;
		float fBlurScale = g_debugMotionBlurDoFBlurScale;
		float fFocalLength = g_debugMotionBlurDoFFocalLength;

		MathsStorage::Float4 fvec(fMaxCoC, fEdgeFactor, fBlurScale, fFocalLength);
		Display::Device()->SetPixelShaderParameterF(m_Settings2Parameter[m_nShader], (F32*)&fvec);
	}
	{
		float fWidth = (F32)pView->GetViewport().GetW();
		float fHeight = (F32)pView->GetViewport().GetH();
		float fOffsetX = g_Random.FRand(0.0f, 1.0f);
		float fOffsetY = g_Random.FRand(0.0f, 1.0f);

		MathsStorage::Float4 fvec(fWidth / (F32)g_MotionBlurNoiseSize, fHeight / (F32)g_MotionBlurNoiseSize, fOffsetX, fOffsetY);
		Display::Device()->SetPixelShaderParameterF(m_NoiseParameter[m_nShader], (F32*)&fvec);
	}

	// set filter taps
	{
		F32 taps[24] =
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

		Display::Device()->SetPixelShaderParameterArrayF(m_FilterParameter[m_nShader], taps);
	}

	// set alpha scale
	{
		MathsStorage::Float4 fvec(g_debugMotionBlurDoFAlphaMotion, g_debugMotionBlurDoFAlphaDoF, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_AlphaParameter[m_nShader], (F32*)&fvec);
	}
}

void C_MotionBlurDoFReduced::CopyStencilBufferToAlpha() const
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

void C_MotionBlurDoFReduced::DownsampleStencilBufferToAlpha() const
{
	Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);

	Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());	

	Display::Device()->SetPixelShader(&m_DownsampleStencilShader);

	Display::Quad::DrawFullScreen();

}

bool C_MotionBlurDoFReduced::CreateRenderTarget()
{
	if(m_ColourMaskRenderTextures.Size() > 0)
		return true;

	if(!m_Manager.CreateUserRenderTextures(1, 1, Display::E_Usage_Colour, Display::E_Format_Unknown, 0, m_ColourMaskRenderTextures, 0, 0))
		return false;

	return true;
}

void C_MotionBlurDoFReduced::DestroyRenderTarget()
{
	if(m_ColourMaskRenderTextures.Size() > 0)
		m_Manager.DestroyUserRenderTextures(m_ColourMaskRenderTextures);
}

Display::C_Texture2D* C_MotionBlurDoFReduced::GetTexture()
{
	const S32 iRenderTarget = m_Manager.GetRenderTargetIndex();
	if(g_debugMotionBlurDoF && iRenderTarget >= 0 && iRenderTarget < m_ReducedRenderTextures.Size())
		return &m_ReducedRenderTextures[iRenderTarget]->GetTexture();
	else
		return &m_BlackTexture;
}

}; // namespace PostProcess
