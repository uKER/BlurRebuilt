#include "stdafx.h"
#include <Graphics/HDR.h>

#include <Graphics/PostFXLensFlare.h>

#include <Render/PostProcess/Manager.h>
#include <Render/Shaders/Constants.h>
#include <System/Misc/Random.h>

#if defined(PLATFORM_PS3) && defined(TONEMAP_COMBINE)
#include <Graphics/TonemapSPU.h>
#include "SchedulerConfig.h"
#endif

EXTERN_SHADER(psToneMapCustom);
EXTERN_SHADER(psToneMapCustomHDR);
EXTERN_SHADER(psToneMapCustomHDRU32);
EXTERN_SHADER(psToneMapCustomReducedAlpha);
EXTERN_SHADER(psToneMapCustomReducedAlphaHDR);
EXTERN_SHADER(psToneMapCustomReducedAlphaHDRU32);
EXTERN_SHADER(psToneMapCustomMotionBlur);
EXTERN_SHADER(psToneMapCustomMotionBlurHDR);
EXTERN_SHADER(psToneMapCustomMotionBlurHDRU32);
EXTERN_SHADER(psToneMapCustomReducedAlphaMotionBlur);
EXTERN_SHADER(psToneMapCustomReducedAlphaMotionBlurHDR);
EXTERN_SHADER(psToneMapCustomReducedAlphaMotionBlurHDRU32);
EXTERN_SHADER(psToneMapCustomDepthOfField);
EXTERN_SHADER(psToneMapCustomDepthOfFieldHDR);
EXTERN_SHADER(psToneMapCustomDepthOfFieldHDRU32);
EXTERN_SHADER(psToneMapCustomLargeScreenshot);
EXTERN_SHADER(psToneMapCustomLargeScreenshotHDR);
EXTERN_SHADER(psToneMapCustomLargeScreenshotHDRU32);
EXTERN_SHADER(psToneMapCustomExtraFX);
EXTERN_SHADER(psToneMapCustomExtraFXU32);

namespace PostProcess
{
	extern Util::C_Random g_Random;
};

namespace HDR
{
	Maths::Mat44		g_mColour				= Maths::Const<Maths::Mat44>::Identity();
	F32					g_fFilmGrainIntensity	= 0.15f;
	F32					g_fFilmGrainLuminance	= 0.00f;
	F32					g_fVignetteWeight		= 0.50f;
	F32					g_fTunnelAmount			= 0.00f;
	F32					g_ColourSep				= 0.00f;
	F32					g_Blockiness			= 0.00f;
	F32					g_ScreenOverlay			= 0.00f;
	Display::C_Float2	g_vFlareScales[4];
	Display::C_Float4	g_vFlareColours[4];
	bool				g_bReloadBloomCurve		= false;

	Debug::Float	g_ScreenZoom		("Graphics.Render.PostProcess.HDR.LargeScreenshot.Zoom",		1.0f,	Debug::Range( 1.0f, 5.0f));
	Debug::Float	g_ScreenMoveX		("Graphics.Render.PostProcess.HDR.LargeScreenshot.MoveX",		0.0f,	Debug::Range(-1.0f, 1.0f));
	Debug::Float	g_ScreenMoveY		("Graphics.Render.PostProcess.HDR.LargeScreenshot.MoveY",		0.0f,	Debug::Range(-1.0f, 1.0f));
	Debug::Float	g_BloomBright		("Graphics.Render.PostProcess.HDR.LargeScreenshot.BloomBright",	1.0f,	Debug::Range(0.0f, 2.0f));	
	Debug::Bool		g_ColourCorrection	("Graphics.Render.PostProcess.HDR.ColourCorrection",			true);
	Debug::Float	g_OverrideColourSep ("Graphics.Render.PostProcess.HDR.ColourSep",  0.f, Debug::Range(0.f, 1.f));
	Debug::Float	g_OverrideBlockiness("Graphics.Render.PostProcess.HDR.Blockiness", 0.f, Debug::Range(0.f, 1.f));
	Debug::Float	g_OverrideScreenOverlay("Graphics.Render.PostProcess.HDR.ScreenOverlay", 0.f, Debug::Range(0.f, 1.f));
};

C_HighDynamicRange::C_HighDynamicRange(PostProcess::C_Manager& manager) : PostProcess::C_HighDynamicRange(manager), m_NoiseTextureName("Graphics/Textures/PostProcess/Noise.dds"), m_LookupTextureName("")
{
	SetHighDynamicRange(this);
#ifdef HDR_USE_U32
	m_eType = PostProcess::E_HighDynamicRangeType_ShaderU32;
#else
	m_eType = PostProcess::E_HighDynamicRangeType_Shader;
#endif
#ifdef PLATFORM_PS3
	m_bCalculateLuminanceAuto = false;
#endif
	m_eToneMapShader = E_ToneMapShader_Default;
	m_pMotionBlurTexture = 0;
	m_pLensEffectTexture = 0;
	m_UID = 0;
	memset(&m_OverlayAmount, 0, sizeof(m_OverlayAmount));
}

C_HighDynamicRange::~C_HighDynamicRange()
{
	SetHighDynamicRange(NULL);
}

bool C_HighDynamicRange::Create()
{
#ifndef HDR_USE_U32
	if(m_eType == PostProcess::E_HighDynamicRangeType_Shader)
	{
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomHDR", 0, &m_PixelShader[E_ToneMapShader_Default], REF_SHADER(psToneMapCustomHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomMotionBlurHDR", 0, &m_PixelShader[E_ToneMapShader_MotionBlur], REF_SHADER(psToneMapCustomMotionBlurHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomReducedAlphaHDR", 0, &m_PixelShader[E_ToneMapShader_ReducedAlpha], REF_SHADER(psToneMapCustomReducedAlphaHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomReducedAlphaMotionBlurHDR", 0, &m_PixelShader[E_ToneMapShader_ReducedAlphaMotionBlur], REF_SHADER(psToneMapCustomReducedAlphaMotionBlurHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomDepthOfFieldHDR", 0, &m_PixelShader[E_ToneMapShader_DepthOfField], REF_SHADER(psToneMapCustomDepthOfFieldHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomLargeScreenshotHDR", 0, &m_PixelShader[E_ToneMapShader_LargeScreenshot], REF_SHADER(psToneMapCustomLargeScreenshotHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomExtraFX", 0, &m_PixelShader[E_ToneMapShader_ExtraFX], REF_SHADER(psToneMapCustomExtraFX)))
			return false;
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustom", 0, &m_PixelShader[E_ToneMapShader_Default], REF_SHADER(psToneMapCustom)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomMotionBlur", 0, &m_PixelShader[E_ToneMapShader_MotionBlur], REF_SHADER(psToneMapCustomMotionBlur)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomReducedAlpha", 0, &m_PixelShader[E_ToneMapShader_ReducedAlpha], REF_SHADER(psToneMapCustomReducedAlpha)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomReducedAlphaMotionBlur", 0, &m_PixelShader[E_ToneMapShader_ReducedAlphaMotionBlur], REF_SHADER(psToneMapCustomReducedAlphaMotionBlur)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomDepthOfField", 0, &m_PixelShader[E_ToneMapShader_DepthOfField], REF_SHADER(psToneMapCustomDepthOfField)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomLargeScreenshot", 0, &m_PixelShader[E_ToneMapShader_LargeScreenshot], REF_SHADER(psToneMapCustomLargeScreenshot)))
			return false;

		// This doesn't seem to be used, so just use Motion Blur shader if ExtraFX is requested:
		if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomMotionBlur", 0, &m_PixelShader[E_ToneMapShader_ExtraFX], REF_SHADER(psToneMapCustomMotionBlur)))
			return false;
	}
#else
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomHDRU32", 0, &m_PixelShader[E_ToneMapShader_Default], REF_SHADER(psToneMapCustomHDRU32)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomMotionBlurHDRU32", 0, &m_PixelShader[E_ToneMapShader_MotionBlur], REF_SHADER(psToneMapCustomMotionBlurHDRU32)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomReducedAlphaHDRU32", 0, &m_PixelShader[E_ToneMapShader_ReducedAlpha], REF_SHADER(psToneMapCustomReducedAlphaHDRU32)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomReducedAlphaMotionBlurHDRU32", 0, &m_PixelShader[E_ToneMapShader_ReducedAlphaMotionBlur], REF_SHADER(psToneMapCustomReducedAlphaMotionBlurHDRU32)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomDepthOfFieldHDRU32", 0, &m_PixelShader[E_ToneMapShader_DepthOfField], REF_SHADER(psToneMapCustomDepthOfFieldHDRU32)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomLargeScreenshotHDRU32", 0, &m_PixelShader[E_ToneMapShader_LargeScreenshot], REF_SHADER(psToneMapCustomLargeScreenshotHDRU32)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psToneMapCustomExtraFXU32", 0, &m_PixelShader[E_ToneMapShader_ExtraFX], REF_SHADER(psToneMapCustomExtraFXU32)))
		return false;
#endif

	for(S32 i = 0; i < E_ToneMapShader_Count; ++i)
	{
		m_Settings1Parameter[i]		= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings1");
		m_Settings2Parameter[i]		= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vSettings2");
		m_ColourMatrixParameter[i]	= m_PixelShader[i].GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_mColour", 4);
		m_FilmGrainParameter[i]		= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_6, "g_vFilmGrain");
		m_SpecialFXParameter[i]		= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_7, "g_vSpecialFX");
		m_BloomScaleParameter[i]	= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_8, "g_vBloomScale");
		m_ColourSepParameter[i]		= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_9, "g_vColourSep");
	}

	SetShader(&m_PixelShader[m_eToneMapShader]);

	if(!Display::CreateTexture2DFromFile(m_NoiseTextureName, &m_NoiseTexture))
		Display::CreateTexture2DFromFile("Graphics/Textures/PostProcess/Noise.dds", &m_NoiseTexture);

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
		Display2::Device()->SetTextureName(&m_BlackTexture, "_HdrBlack");
		#endif
	}

	if(!CreateLookupTexture())
		return false;

#if !defined(CFG_GOLD)
	if(!Display::Device()->CreateTexture3D(32, 32, 32, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &m_IdentityTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(&m_IdentityTexture, "_HdrIdentity");
	#endif

	Display::C_LockedBox lockedBox = m_IdentityTexture.LockBox3D(0);
	if(!lockedBox.pBits)
		return false;

	U32* pBits = (U32*)lockedBox.pBits;
	for(U32 w = 0; w < 32; ++w)
	{
		for(U32 v = 0; v < 32; ++v)
		{
			for(U32 u = 0; u < 32; ++u)
			{
				U8 a = 0;
				U8 r = (U8)(u * 255.0f / 31.0f);
				U8 g = (U8)(v * 255.0f / 31.0f);
				U8 b = (U8)(w * 255.0f / 31.0f);
				pBits[u + v * lockedBox.rowPitch / 4] = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}
		pBits += lockedBox.slicePitch / 4;
	}

	m_IdentityTexture.UnlockBox3D(0);
#endif

	{
		if(!Display::Device()->CreateTexture2D(256, 1, 1, Display::E_Format_Linear, Display::E_Format_L8, &m_BloomCurveTexture))
			return false;

#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_BloomCurveTexture, "_HdrBloomThreshold");
#endif

		Display2::C_LockedRect lockedRect = m_BloomCurveTexture.LockRect2D(0);
		if(!lockedRect.pBits)
			return false;

		U8* pBits = (U8*)lockedRect.pBits;
		for(U32 x = 0; x < 256; ++x)
			*pBits++ = 0xFF;

		m_BloomCurveTexture.UnlockRect2D(0);

		SetBloomCurve(&m_BloomCurveTexture);
	}

#ifdef TONEMAP_COMBINE
	for (int i = 0; i < TONEMAP_COMBINE_BUFFER_SIZE; i++)
	{
		if(!Display::Device()->CreateTexture3D(32, 32, 32, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &m_LookupTextureArray[i]))
			return false;

#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_LookupTextureArray[i], "_HdrLookup");
#endif
	}

	if(!Display::Device()->CreateTexture3D(32, 32, 32, 1, Display::E_Format_Linear | Display::E_Usage_System, Display::E_Format_A8R8G8B8, &m_LookupTextureGamma))
		return false;

	m_LookupTextureGammaValue = 0.0f;
#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_LookupTextureGamma, "_HdrLookup");
#endif
#endif

	Display::CreateTexture2DFromFile("Graphics/Textures/PostProcess/ScreenSmash.texture", &m_OverlayTexture);

	return PostProcess::C_HighDynamicRange::Create();
}

bool C_HighDynamicRange::CreateVolatile()
{
	if (m_Parameters.Size() != m_Manager.GetViewCount())
	{
		for (S32 i = 0; i < m_Parameters.Size(); ++i)
			delete m_Parameters[i];
		m_Parameters.Clear();

		for (U32 i = 0; i < m_Manager.GetViewCount(); ++i)
			m_Parameters.PushBack(new ToneMapParameters);
	}

	return PostProcess::C_HighDynamicRange::CreateVolatile();
}

void C_HighDynamicRange::Destroy()
{
	PostProcess::C_HighDynamicRange::Destroy();

#if !defined(CFG_GOLD)
	Display::Device()->Destroy(&m_IdentityTexture);
#endif
	Display::Device()->Destroy(&m_LookupTexture);
#ifdef TONEMAP_COMBINE
	for (int i = 0; i < TONEMAP_COMBINE_BUFFER_SIZE; i++)
		Display::Device()->Destroy(&m_LookupTextureArray[i]);
	Display::Device()->Destroy(&m_LookupTextureGamma);
#endif
	Display::Device()->Destroy(&m_NoiseTexture);
	Display::Device()->Destroy(&m_BlackTexture);
	Display::Device()->Destroy(&m_BloomCurveTexture);
	for(S32 i = 0; i < E_ToneMapShader_Count; ++i)
		Display::Device()->Destroy(&m_PixelShader[i]);

	Display::Device()->Destroy(&m_OverlayTexture);
}

void C_HighDynamicRange::DestroyVolatile()
{
	PostProcess::C_HighDynamicRange::DestroyVolatile();
}

void C_HighDynamicRange::ManualCalculateLuminance() const
{
#ifdef PLATFORM_PS3
	CalculateLuminance();

	// need to restore the main viewport at 0,0
	const Display::C_View& view = *Display::Device()->GetView();
	Display::C_Viewport viewportOrigin = view.GetViewport();
	viewportOrigin.m_X = 0;
	viewportOrigin.m_Y = 0;
	m_Manager.SetViewport(viewportOrigin);
#endif
}

void C_HighDynamicRange::SetUserParameters(const MathsStorage::Float4& settings1, const MathsStorage::Float4& settings2) const
{
	Display::Device()->SetPixelShaderParameterF(m_Settings1Parameter[m_eToneMapShader], (F32*)&settings1);
	Display::Device()->SetPixelShaderParameterF(m_Settings2Parameter[m_eToneMapShader], (F32*)&settings2);

	MathsStorage::MatrixRow44 mColour;
	if (HDR::g_ColourCorrection)
	{
		Maths::Store(HDR::g_mColour, &mColour);
	}
	else
	{
		Maths::Store(Maths::Const<Maths::Mat44>::Identity(), &mColour);
	}
	Display::Device()->SetPixelShaderParameterArrayF(m_ColourMatrixParameter[m_eToneMapShader], (F32*)&mColour);

	const F32 fFilmGrainLuminance = HDR::g_fFilmGrainIntensity * HDR::g_fFilmGrainLuminance;
	const Display::C_Float4 vFilmGrain(0.4250f * fFilmGrainLuminance, 1.4308f * fFilmGrainLuminance, 0.1442f * fFilmGrainLuminance, HDR::g_fFilmGrainIntensity + fFilmGrainLuminance);
	Display::Device()->SetPixelShaderParameterF(m_FilmGrainParameter[m_eToneMapShader], (F32*)&vFilmGrain);

	const F32 fOffsetX = PostProcess::g_Random.FRand(0.0f, 1.0f);
	const F32 fOffsetY = PostProcess::g_Random.FRand(0.0f, 1.0f);
	const Display::C_Float4 vSpecialFX(1.0f - 2.0f * HDR::g_fVignetteWeight, HDR::g_fTunnelAmount, fOffsetX, fOffsetY);
	Display::Device()->SetPixelShaderParameterF(m_SpecialFXParameter[m_eToneMapShader], (F32*)&vSpecialFX);

	Display::C_Float4 vBloomScale;
	vBloomScale.x = HDR::g_ScreenMoveX * 0.5f;
	vBloomScale.y = HDR::g_ScreenMoveY * -0.5f;
	vBloomScale.z = (HDR::g_ScreenZoom > 0.001f) ? 1.0f / HDR::g_ScreenZoom : 0.0f;
	vBloomScale.w = HDR::g_BloomBright;
	Display::Device()->SetPixelShaderParameterF(m_BloomScaleParameter[m_eToneMapShader], (F32*)&vBloomScale);

	// Set colour separation/blockiness values:
	F32 extraFx[4] = {ColourSep() * 0.02f, Blockiness(), ScreenOverlay()};
	Display::Device()->SetPixelShaderParameterF(m_ColourSepParameter[m_eToneMapShader], extraFx);

	Display::Device()->SetTexture(5, &m_NoiseTexture);
	Display::Device()->SetTextureAddress(5, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilter(5, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(5, Display::E_Bool_False);	
	
	Display::Device()->SetTexture(6, m_pMotionBlurTexture);
	Display::Device()->SetTextureAddress(6, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(6, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(6, Display::E_Bool_False);

	if(m_pLensEffectTexture)
		Display::Device()->SetTexture(7, m_pLensEffectTexture);
	else
		Display::Device()->SetTexture(7, &m_BlackTexture);
	Display::Device()->SetTextureAddress(7, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(7, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(7, Display::E_Bool_False);

#if !defined(CFG_GOLD)
	if (!HDR::g_ColourCorrection)
	{
		Display::Device()->SetTexture(8, &m_IdentityTexture);
	}
	else
#endif
	{
#ifdef TONEMAP_COMBINE
		int ViewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
		int ViewCount = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();

		static int BufferBase;
		if (ViewIndex == 0)
			BufferBase = (BufferBase + 1) & 0x1;
		int BufferIndex = BufferBase * TONEMAP_COMBINE_BUFFER_SIZE / 2 + ViewIndex;

		Display::C_LockedBox lockedBoxSrc = ((Display::C_Texture3D&)m_LookupTexture).LockBox3D(0);
		Display::C_LockedBox lockedBoxGamma = ((Display::C_Texture3D&)m_LookupTextureGamma).LockBox3D(0);
		Display::C_LockedBox lockedBoxDst = ((Display::C_Texture3D&)m_LookupTextureArray[BufferIndex]).LockBox3D(0);

		m_LookupTexture.UnlockBox3D(0);
		m_LookupTextureGamma.UnlockBox3D(0);
		m_LookupTextureArray[BufferIndex].UnlockBox3D(0);

		PostProcess::HighDynamicRangeParameters* pParameters = (PostProcess::HighDynamicRangeParameters*)m_Parameters[m_Manager.GetViewIndex()];
		float fGamma = pParameters->fGamma;

		((Maths::Mat44*)m_ColourMatArray)[BufferIndex] = HDR::g_mColour;

		Tonemap::TaskData Data;
		Data.lockedBoxSrc = lockedBoxSrc;
		Data.lockedBoxGamma = lockedBoxGamma;
		Data.lockedBoxDst = lockedBoxDst;
		Data.mColour = (Maths::Mat44*)&m_ColourMatArray[BufferIndex];
		Data.fGamma = fGamma;
		Data.UpdateGamma = (fGamma == m_LookupTextureGammaValue) ? 0 : 1;
		*(float*)&m_LookupTextureGammaValue = fGamma;

		Scheduler::AddTask(TonemapModule, NULL, NULL, Scheduler::JQ_GROUP_NONE, Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy(), &Data, sizeof(Data));

		Display::Device()->SetTexture(8, &m_LookupTextureArray[BufferIndex]);
#else
		Display::Device()->SetTexture(8, &m_LookupTexture);
#endif
	}
	Display::Device()->SetTextureAddress(8, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(8, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(8, Display::E_Bool_False);

	if(m_pReducedAlphaTexture)
		Display::Device()->SetTexture(9, m_pReducedAlphaTexture);
	else
		Display::Device()->SetTexture(9, &m_BlackTexture);
	Display::Device()->SetTextureAddress(9, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(9, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(9, Display::E_Bool_False);

	Display::Device()->SetTexture(10, &m_OverlayTexture);
}

ToneMapParameters* C_HighDynamicRange::GetParameters(const S32 iViewIndex)
{
	return (ToneMapParameters*)m_Parameters[iViewIndex];
}

void C_HighDynamicRange::Display() const
{
	if(m_pReducedAlphaTexture)
	{
		if(m_eToneMapShader == E_ToneMapShader_Default)
			SetToneMapShader(E_ToneMapShader_ReducedAlpha);
		else if(m_eToneMapShader == E_ToneMapShader_MotionBlur)
			SetToneMapShader(E_ToneMapShader_ReducedAlphaMotionBlur);
	}
	else
	{
		if(m_eToneMapShader == E_ToneMapShader_ReducedAlpha)
			SetToneMapShader(E_ToneMapShader_Default);
		else if(m_eToneMapShader == E_ToneMapShader_ReducedAlphaMotionBlur)
			SetToneMapShader(E_ToneMapShader_MotionBlur);
	}

	bool efx = NeedExtraFX();
	if (m_eToneMapShader == E_ToneMapShader_MotionBlur && efx)
		SetToneMapShader(E_ToneMapShader_ExtraFX);
	else if (m_eToneMapShader == E_ToneMapShader_ExtraFX && !efx)
		SetToneMapShader(E_ToneMapShader_MotionBlur);

	PostProcess::C_HighDynamicRange::Display();
}

void C_HighDynamicRange::SetToneMapShader(const E_ToneMapShader eToneMapShader) const
{
	m_eToneMapShader = eToneMapShader;
	SetShader(&m_PixelShader[eToneMapShader]);
}

E_ToneMapShader C_HighDynamicRange::GetToneMapShader() const
{
	return m_eToneMapShader;
}

void C_HighDynamicRange::SetLargeScreenshotBloomScale(const F32 fZoom, const F32 fX, const F32 fY, const F32 fBright)
{
	HDR::g_ScreenZoom = fZoom;
	HDR::g_ScreenMoveX = fX;
	HDR::g_ScreenMoveY = fY;
	HDR::g_BloomBright = fBright;
}

void C_HighDynamicRange::ReloadNoiseTexture()
{
#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif
	Display::Device()->Destroy(&m_NoiseTexture);
	if(!Display::CreateTexture2DFromFile(m_NoiseTextureName, &m_NoiseTexture))
		Display::CreateTexture2DFromFile("Graphics/Textures/PostProcess/Noise.dds", &m_NoiseTexture);
}

bool C_HighDynamicRange::CreateLookupTexture()
{
#ifdef TONEMAP_COMBINE
	m_LookupTextureGammaValue = 0.0f;
#endif
#ifdef TONEMAP_COMBINE
	// the LUT is only read on the CPU and baked into m_LookupTextureArray each frame
	const U32 lookupUsage = Display::E_Usage_System;
#else
	// [Blur2PC] without TONEMAP_COMBINE (PC) this texture is bound to sampler 8 directly; a system-memory
	// texture cannot be sampled, so every LUT fetch returned black and the whole frame went black.
	const U32 lookupUsage = Display::E_Usage_Managed;
#endif
	if(m_LookupTextureName.Length() == 0 || !Display::CreateTexture3DFromFile(m_LookupTextureName, &m_LookupTexture, lookupUsage))
	{
		if(!Display::Device()->CreateTexture3D(32, 32, 32, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &m_LookupTexture))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_LookupTexture, "_HdrLookup");
		#endif

		Display::C_LockedBox lockedBox = m_LookupTexture.LockBox3D(0);
		if(!lockedBox.pBits)
			return false;

		U32* pBits = (U32*)lockedBox.pBits;
		for(U32 w = 0; w < 32; ++w)
		{
			for(U32 v = 0; v < 32; ++v)
			{
				for(U32 u = 0; u < 32; ++u)
				{
					U8 a = 0;
					U8 r = (U8)(u * 255.0f / 31.0f);
					U8 g = (U8)(v * 255.0f / 31.0f);
					U8 b = (U8)(w * 255.0f / 31.0f);
					pBits[u + v * lockedBox.rowPitch / 4] = (a << 24) + (r << 16) + (g << 8) + b;
				}
			}
			pBits += lockedBox.slicePitch / 4;
		}

		m_LookupTexture.UnlockBox3D(0);
	}
	return true;
}

void C_HighDynamicRange::ReloadLookupTexture()
{
#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif
	Display::Device()->Destroy(&m_LookupTexture);
	CreateLookupTexture();
}

void C_HighDynamicRange::ReloadBloomCurve(const Render::C_HermiteCurve& curve, const U32 uid)
{
	if(uid != m_UID || HDR::g_bReloadBloomCurve)
	{
#if defined(PLATFORM_PS3)
		Display::Device()->BlockUntilIdle();
#endif
	
		Display2::C_LockedRect lockedRect = m_BloomCurveTexture.LockRect2D(0);
		if(!lockedRect.pBits)
			return;

		U8* pBits = (U8*)lockedRect.pBits;
		for(U32 x = 0; x < 256; ++x)
		{
			const F32 fTime = (F32)x / 256.0f;
			const F32 fValue = curve.Evaluate(fTime, 1.0f);
			*pBits++ = (U8)(fValue * 255.0f);
		}

		m_BloomCurveTexture.UnlockRect2D(0);

		m_UID = uid;

		HDR::g_bReloadBloomCurve = false;
	}
}

F32 C_HighDynamicRange::ColourSep() const
{
	return HDR::g_OverrideColourSep > 0.f  ? HDR::g_OverrideColourSep  : HDR::g_ColourSep;
}

F32 C_HighDynamicRange::Blockiness() const
{
	return HDR::g_OverrideBlockiness > 0.f ? HDR::g_OverrideBlockiness : HDR::g_Blockiness;
}

// Return a float for the intensity of the 'screen smash' effect:
F32 C_HighDynamicRange::ScreenOverlay() const
{
	if (!m_OverlayTexture.IsCreated())
		return 0.f;

	float overlayFx = HDR::g_ScreenOverlay; // Overlay set via Jose's FX system
	int view = m_Manager.GetViewIndex();
	float overlayDirect = m_OverlayAmount[view & 3]; // Amount of overlay set directly
	float overlay = Maths::Max(overlayFx, overlayDirect);
	
	// Debug menu override of overlay amount:
	if (HDR::g_OverrideScreenOverlay > 0.f)
		overlay = HDR::g_OverrideScreenOverlay;
	
	return overlay;
}

// Return whether we need the more expensive 'ExtraFX' version of the ToneMapping shader:
bool C_HighDynamicRange::NeedExtraFX() const
{
	return ColourSep() > 0.f || Blockiness() > 0.f || ScreenOverlay() > 0.f;
}

void C_HighDynamicRange::SetOverlay(int viewport, float amount)
{
	m_OverlayAmount[viewport & 3] = amount;
}

C_HighDynamicRange* g_pHighDynamicRange = NULL;

C_HighDynamicRange* GetHighDynamicRange()
{
	assert(g_pHighDynamicRange);
	return g_pHighDynamicRange;
}

void SetHighDynamicRange(C_HighDynamicRange* pHighDynamicRange)
{
	g_pHighDynamicRange = pHighDynamicRange;
}

bool HighDynamicRangeExists()
{
	return g_pHighDynamicRange != NULL;
}
