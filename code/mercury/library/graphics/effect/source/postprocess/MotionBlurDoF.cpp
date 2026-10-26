#include "stdafx.h"
#include "Effect/PostProcess/MotionBlurDoF.h"

#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

#include "System/Misc/Random.h"

EXTERN_SHADER(psMotionBlurDoFAlpha5);
EXTERN_SHADER(psMotionBlurDoFAlpha7);
EXTERN_SHADER(psMotionBlurDoFAlpha9);
EXTERN_SHADER(psMotionBlurDoFAlpha11);
EXTERN_SHADER(psMotionBlurDoFStencil5);
EXTERN_SHADER(psMotionBlurDoFStencil7);
EXTERN_SHADER(psMotionBlurDoFStencil9);
EXTERN_SHADER(psMotionBlurDoFStencil11);
EXTERN_SHADER(psMotionBlurDoFCopyStencil);
EXTERN_SHADER(psMotionBlurDoFWriteMask);

namespace PostProcess
{

#ifdef PLATFORM_PC
Debug::Bool		g_debugMotionBlurDoF						("Graphics.Render.PostProcess.MotionBlurDoF.Enable", false);
#else
Debug::Bool		g_debugMotionBlurDoF						("Graphics.Render.PostProcess.MotionBlurDoF.Enable", true);
#endif

Debug::Bool		g_debugMotionBlurDoFBlurDynamicObjects		("Graphics.Render.PostProcess.MotionBlurDoF.BlurDynamicObjects", false);
Debug::Float	g_debugMotionBlurDoFShutterAngle			("Graphics.Render.PostProcess.MotionBlurDoF.ShutterAngle", 1.0f, Debug::Range(0.0f, 1.0f),
	Debug::Description("Please leave this value set to 1.0, change Camera2.ShutterAngle instead")
);

Debug::Bool		g_debugMotionBlurDoFEnableDoF				("Graphics.Render.PostProcess.MotionBlurDoF.EnableDoF", true);

Debug::Float	g_debugMotionBlurDoFBlurScale				("Graphics.Render.PostProcess.MotionBlurDoF.BlurScale", 100.0f, Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugMotionBlurDoFFocalLength				("Graphics.Render.PostProcess.MotionBlurDoF.FocalLength", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugMotionBlurDoFMaxCoC					("Graphics.Render.PostProcess.MotionBlurDoF.MaxCoC", 5.0f, Debug::Range(0.0f, 16.0f) );
Debug::Float	g_debugMotionBlurDoFEdgeFactor				("Graphics.Render.PostProcess.MotionBlurDoF.EdgeFactor", 0.1f, Debug::Range(0.0f, 16.0f) );

Debug::Bool		g_debugMotionBlurDoFUseAlternateBuffer		("Graphics.Render.PostProcess.MotionBlurDoF.UseAlternateBuffer", false);

Debug::Float	g_debugMotionBlurDoFNoiseScale				("Graphics.Render.PostProcess.MotionBlurDoF.NoiseScale", 1.0f, Debug::Range(0.0f, 5.0f));
Debug::Float	g_debugMotionBlurDoFCentreWeight			("Graphics.Render.PostProcess.MotionBlurDoF.CentreWeight", 0.001f, Debug::Range(0.0f, 5.0f));

#ifndef CFG_SPEED
Debug::Bool		g_debugMotionBlurDoFUseRandomTaps			("Graphics.Render.PostProcess.MotionBlurDoF.UseRandomTaps", false);
#endif

extern const U32 g_MotionBlurNoiseSize = 64;

extern Util::C_Random g_Random;

C_MotionBlurDoF::C_MotionBlurDoF(C_Manager& manager) : I_PostProcess(manager), m_bEnabled(true)
{
	m_TextureFilter = Display::E_TextureFilter_Point;
	SetMethod(E_MotionBlurDoFMethod_Alpha);
	SetSamples(E_MotionBlurDoFSamples_5);
}

C_MotionBlurDoF::~C_MotionBlurDoF()
{
	m_ViewportSettings.Clear();
}

bool C_MotionBlurDoF::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFAlpha5", 0, &m_BlurShader[0], REF_SHADER(psMotionBlurDoFAlpha5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFAlpha7", 0, &m_BlurShader[1], REF_SHADER(psMotionBlurDoFAlpha7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFAlpha9", 0, &m_BlurShader[2], REF_SHADER(psMotionBlurDoFAlpha9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFAlpha11", 0, &m_BlurShader[3], REF_SHADER(psMotionBlurDoFAlpha11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFStencil5", 0, &m_BlurShader[4], REF_SHADER(psMotionBlurDoFStencil5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFStencil7", 0, &m_BlurShader[5], REF_SHADER(psMotionBlurDoFStencil7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFStencil9", 0, &m_BlurShader[6], REF_SHADER(psMotionBlurDoFStencil9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFStencil11", 0, &m_BlurShader[7], REF_SHADER(psMotionBlurDoFStencil11)))
		return false;
	
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFCopyStencil", 0, &m_CopyStencilShader, REF_SHADER(psMotionBlurDoFCopyStencil)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psMotionBlurDoFWriteMask", 0, &m_WriteMaskShader, REF_SHADER(psMotionBlurDoFWriteMask)))
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

bool C_MotionBlurDoF::CreateVolatile()
{
	if (g_debugMotionBlurDoFUseAlternateBuffer)
		return CreateRenderTarget();

	return true;
}

void C_MotionBlurDoF::Destroy()
{
	m_ViewportSettings.Clear();

	for (U32 shaderNo = 0; shaderNo < kMotionBlurDoFShaderCount; ++shaderNo)
	{
		Display::Device()->Destroy(&m_BlurShader[shaderNo]);
	}

	Display::Device()->Destroy(&m_CopyStencilShader);
	Display::Device()->Destroy(&m_WriteMaskShader);

	Display::Device()->Destroy(&m_NoiseTexture);
}

void C_MotionBlurDoF::DestroyVolatile()
{
	if (g_debugMotionBlurDoFUseAlternateBuffer)
		DestroyRenderTarget();	
}

void C_MotionBlurDoF::SetSamples(const E_MotionBlurDoFSamples eSamples) const
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

void C_MotionBlurDoF::Display() const
{
	if (!m_Manager.Enabled() || !g_debugMotionBlurDoF || !m_bEnabled)
		return;

	PERF_EVENT("MotionBlurDoF");

	m_nShader = m_eMethod * E_MotionBlurDoFSamples_Count + m_eSamples;
	SetShader(&m_BlurShader[m_nShader]);

	if (E_MotionBlurDoFMethod_Alpha == m_eMethod)
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

	if (g_debugMotionBlurDoFUseAlternateBuffer && m_ColourMaskRenderTexture.IsCreated())
	{
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Red | Display::E_ColourWriteEnable_Blue | Display::E_ColourWriteEnable_Green | Display::E_ColourWriteEnable_Alpha);
		m_Manager.SetRenderTexture(m_ColourMaskRenderTexture);
		DisplayIntermediate(&m_WriteMaskShader);

		Resolve();
		m_Manager.SetViewport();
		Display::Device()->SetTexture(0, &m_ColourMaskRenderTexture.GetTexture());
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

	if (E_MotionBlurDoFMethod_Stencil == m_eMethod)
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

void C_MotionBlurDoF::SetConstants() const
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

		#ifndef CFG_SPEED
		if(g_debugMotionBlurDoFUseRandomTaps)
		{
			F32 taps2[24];

			//F32 angle = ((rand() % 256) / 256) * Maths::Pi * 2.0f;

			static F32 angle = 0.0f;
			angle += 0.1f;

			F32 s = sin(angle);
			F32 c = cos(angle);

			for(S32 i=0; i<16; i++)
			{
				F32 srcX = taps[i*2];
				F32 srcY = taps[i*2 + 1];

				F32 dstX = srcX * s + srcY * c;
				F32 dstY = srcX * c - srcY * s;

				taps2[i * 2] = dstX;
				taps2[i * 2 + 1] = dstY;
			}

			Display::Device()->SetPixelShaderParameterArrayF(m_FilterParameter[m_nShader], taps2);
		}
		else
		#endif
		{
			Display::Device()->SetPixelShaderParameterArrayF(m_FilterParameter[m_nShader], taps);
		}
	}
}

void C_MotionBlurDoF::CopyStencilBufferToAlpha() const
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

bool C_MotionBlurDoF::CreateRenderTarget()
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
		Display2::Device()->SetRenderTexture2DName(&m_ColourMaskRenderTexture, "_MotionBlurDoFColourMaskRT");
		#endif
	}

	return true;
}

void C_MotionBlurDoF::DestroyRenderTarget()
{
	if (m_ColourMaskRenderTexture.IsCreated())
	{
		Display::Device()->Destroy(&m_ColourMaskRenderTexture);
	}
}

}; // namespace PostProcess
