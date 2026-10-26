#include "StdAfx.h"
#include "Render/PostProcess/HDR/HDRInterface.h"

#include "Render/Debug/DebugRender.h"
#include "Render/State/RenderState.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"
#include "Render/Utils/Globals.h"
#include "System/Misc/Random.h"

EXTERN_SHADER(psLuminanceInitialHDRU32);
EXTERN_SHADER(psLuminanceInitialHDR);
EXTERN_SHADER(psLuminanceInitial);
EXTERN_SHADER(psWeightedFilter16);
EXTERN_SHADER(psLuminanceFinal);
EXTERN_SHADER(psLuminanceAdapted);
EXTERN_SHADER(psLuminanceResolve);
EXTERN_SHADER(psLuminanceClear);
EXTERN_SHADER(psLuminanceDisplay);
EXTERN_SHADER(psLuminanceDisplayExp);
EXTERN_SHADER(psLuminanceWeightDisplay);
EXTERN_SHADER(psDepthCentreTap);

using namespace Display;

namespace PostProcess
{

const U32 g_NoiseTextureSize = 128;

Debug::Bool g_HDRShowNoise("Graphics.Render.PostProcess.HDR.ShowNoise", false);

Debug::Bool g_HDRLuminanceShow64("Graphics.Render.PostProcess.HDR.Luminance.Show64x64", false);
Debug::Bool g_HDRLuminanceShow16("Graphics.Render.PostProcess.HDR.Luminance.Show16x16", false);
Debug::Bool g_HDRLuminanceShow4("Graphics.Render.PostProcess.HDR.Luminance.Show4x4", false);
Debug::Bool g_HDRLuminanceShow1("Graphics.Render.PostProcess.HDR.Luminance.Show1x1", false);
Debug::Bool g_HDRLuminanceShowAdapted("Graphics.Render.PostProcess.HDR.Luminance.ShowAdapted", false);
Debug::Bool g_HDRLuminanceShowWeight("Graphics.Render.PostProcess.HDR.Luminance.ShowWeight", false);

Debug::Bool g_HDRLuminanceCalculate("Graphics.Render.PostProcess.HDR.Luminance.Calculate", true);
Debug::Bool g_HDRLuminanceReset("Graphics.Render.PostProcess.HDR.Luminance.Reset", false);

Debug::Float g_HDRLuminanceWeightX("Graphics.Render.PostProcess.HDR.Luminance.WeightX", 0.5f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));
Debug::Float g_HDRLuminanceWeightY("Graphics.Render.PostProcess.HDR.Luminance.WeightY", 0.5f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));
Debug::Float g_HDRLuminanceWeightScale("Graphics.Render.PostProcess.HDR.Luminance.WeightScale", 1.0f, Debug::Range(0.f, 2.0f), Debug::Step(0.05f));
Debug::Float g_HDRLuminanceAdjustment("Graphics.Render.PostProcess.HDR.Luminance.Adjustment", 0.0f, Debug::Range(-2.f, 2.0f), Debug::Step(0.05f));

Debug::Float g_HDRExposureMin("Graphics.Render.PostProcess.HDR.Parameters.ExposureMin", 0.0f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));
Debug::Float g_HDRExposureMax("Graphics.Render.PostProcess.HDR.Parameters.ExposureMax", 2.0f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));
Debug::Float g_HDRExposureAdaptSpeed("Graphics.Render.PostProcess.HDR.Parameters.ExposureAdaptSpeed", 5.0f, Debug::Range(0.f, 10.0f), Debug::Step(0.1f));
Debug::Float g_HDRMiddleGrey("Graphics.Render.PostProcess.HDR.Parameters.MiddleGrey", 0.5f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));
Debug::Float g_HDRGamma("Graphics.Render.PostProcess.HDR.Parameters.Gamma", 2.2f, Debug::Range(0.f, 3.0f), Debug::Step(0.1f));

Debug::Bool g_HDRShowExposure("Graphics.Render.PostProcess.HDR.Exposure.Show", false);
Debug::Bool g_HDROverrideExposure("Graphics.Render.PostProcess.HDR.Exposure.Override", false);
Debug::Float g_HDRExposure("Graphics.Render.PostProcess.HDR.Exposure.Exposure", 1.0f, Debug::Range(0.f, 4.0f), Debug::Step(0.1f));

Debug::Float g_HDRLuminanceScale("Graphics.Render.PostProcess.HDR.LuminanceScale", 4.0f, Debug::Range(0.f, 2.0f), Debug::Step(0.1f));

I_HighDynamicRange::I_HighDynamicRange(C_Manager& manager) : I_PostProcess(manager), m_eType(E_HighDynamicRangeType_PostProcess)
{
	Render::C_RenderState::Instance().m_pHDR = this;

	m_NoiseTextureRcp = 1.0f/(F32)g_NoiseTextureSize;
	
	m_bCalculateLuminance = true;
	m_bFastLuminance = false;
}

I_HighDynamicRange::~I_HighDynamicRange()
{
	Render::C_RenderState::Instance().m_pHDR = NULL;
}

void I_HighDynamicRange::SetCalculateLuminance(const bool bCalculateLuminance)
{
	m_bCalculateLuminance = bCalculateLuminance;
}


bool I_HighDynamicRange::Create()
{
	RESOURCE_MEMORY_ID("HDR");

	// luminance
	{
		if(m_eType == E_HighDynamicRangeType_Shader)
		{
			if(!Display::CreatePixelShaderFromFunc("LuminanceInitialHDR", 0, &m_LuminanceInitial, REF_SHADER(psLuminanceInitialHDR)))
				return false;
		}
		else if(m_eType == E_HighDynamicRangeType_ShaderU32)
		{
			if(!Display::CreatePixelShaderFromFunc("LuminanceInitialHDRU32", 0, &m_LuminanceInitial, REF_SHADER(psLuminanceInitialHDRU32)))
				return false;
		}
		else
		{
			// use 
			if(!Display::CreatePixelShaderFromFunc("LuminanceInitial", 0, &m_LuminanceInitial, REF_SHADER(psLuminanceInitial)))
				return false;
		}

		m_LuminanceInitialParam = m_LuminanceInitial.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
		m_LuminanceInitialWeightParam = m_LuminanceInitial.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vWeightSettings");

		if(!Display::CreatePixelShaderFromFunc("LuminanceIterative", 0, &m_LuminanceIterative, REF_SHADER(psWeightedFilter16)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceFinal", 0, &m_LuminanceFinal, REF_SHADER(psLuminanceFinal)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceAdapted", 0, &m_LuminanceAdapted, REF_SHADER(psLuminanceAdapted)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceClear", 0, &m_LuminanceClear, REF_SHADER(psLuminanceClear)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceResolve", 0, &m_LuminanceResolve, REF_SHADER(psLuminanceResolve)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceDisplay", 0, &m_LuminanceDisplay, REF_SHADER(psLuminanceDisplay)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceDisplayExp", 0, &m_LuminanceDisplayExp, REF_SHADER(psLuminanceDisplayExp)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("LuminanceWeightDisplay", 0, &m_LuminanceWeightDisplay, REF_SHADER(psLuminanceWeightDisplay)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("DepthCentreTap", 0, &m_DepthCentreTap, REF_SHADER(psDepthCentreTap)))
			return false;

		m_LuminanceIterativeTaps = m_LuminanceIterative.GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vTaps", 16);
		m_LuminanceFinalTaps = m_LuminanceFinal.GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vTaps", 16);
		m_LuminanceFinalParam = m_LuminanceFinal.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_16, "g_vLuminanceClamp");
		m_LuminanceAdaptedParam = m_LuminanceAdapted.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings");
	
		m_LuminanceClearParam = m_LuminanceClear.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vClear");

		m_LuminanceWeightDisplayParam = m_LuminanceWeightDisplay.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vWeightSettings");
	}

	// noise texture
	{
		if (!Display::Device()->CreateTexture2D(g_NoiseTextureSize, g_NoiseTextureSize, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_Noise))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_Noise, "_HDRNoise");
		#endif

		Util::C_Random random;
		random.Seed(123456);

		// fill the noise texture with random values
		Display::C_LockedRect rect = m_Noise.Lock(0);
		if(rect.pBits)
		{
			for(U32 y = 0; y < g_NoiseTextureSize; ++y)
			{	
				U32 *pData = reinterpret_cast<U32*>(rect.pBits);
				for(U32 x = 0; x < g_NoiseTextureSize; ++x)
				{
					pData[x] = (random.Rand(255) << 24) + (random.Rand(255) << 16) + (random.Rand(255) << 8) + random.Rand(255);
				}
				rect.pBits += rect.pitch;
			}
			m_Noise.Unlock(0);
		}
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
		Display2::Device()->SetTextureName(&m_BlackTexture, "_HDRBlack");
		#endif
	}

	return true;
}

bool I_HighDynamicRange::CreateVolatile()
{
	RESOURCE_MEMORY_ID("HDR");
	assert(m_AdaptedLuminances.Size() == 0);
	assert(m_AdaptedInitialised.Size() == 0);
	assert(m_LuminanceResolves.Size() == 0);

	// create the luminance rendertargets
	if (!Device()->CreateRenderTexture2D(64, 64, 1, Display::E_Usage_Colour, E_Format_A16B16G16R16F, E_Multisample_None, NULL, &m_Luminance64))
		return false;
	if (!Device()->CreateRenderTexture2D(16, 16, 1, Display::E_Usage_Colour, E_Format_A16B16G16R16F, E_Multisample_None, NULL, &m_Luminance16))
		return false;
	if (!Device()->CreateRenderTexture2D(4, 4, 1, Display::E_Usage_Colour, E_Format_A16B16G16R16F, E_Multisample_None, NULL, &m_Luminance4))
		return false;
	if (!Device()->CreateRenderTexture2D(1, 1, 1, Display::E_Usage_Colour, E_Format_R32F, E_Multisample_None, NULL, &m_CurrentLuminance))
		return false;

	// made this in system memory as it's slow to lock and read from GDDR3
#ifdef PLATFORM_PS3
	const Display::E_Format depthFormat = E_Format_A8R8G8B8;
#else
	const Display::E_Format depthFormat = E_Format_R32F;
#endif

	if (!Device()->CreateRenderTexture2D(1, 1, 1, Display::E_Usage_Colour | Display2::E_Flags_System, depthFormat, E_Multisample_None, NULL, &m_DepthBufferCentre))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_Luminance64, "_HDRLuminance64RT");
	Display2::Device()->SetRenderTexture2DName(&m_Luminance16, "_HDRLuminance16RT");
	Display2::Device()->SetRenderTexture2DName(&m_Luminance4, "_HDRLuminance4RT");
	Display2::Device()->SetRenderTexture2DName(&m_CurrentLuminance, "_HDRLuminanceRT");
	#endif

	// create an adapted luminance texture for each post effects viewport
	for(U32 i = 0; i < m_Manager.GetViewCount(); ++i)
	{
		Display::C_RenderTexture2D* pAdaptedLuminance = new Display::C_RenderTexture2D;
		if (!Device()->CreateRenderTexture2D(1, 1, 1, Display::E_Usage_Colour, E_Format_R32F, E_Multisample_None, NULL, pAdaptedLuminance))
		{
			delete pAdaptedLuminance;
			return false;
		}

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(pAdaptedLuminance, "_HDRAdaptedLuminanceRT");
		#endif

		m_AdaptedLuminances.PushBack(pAdaptedLuminance);
		m_AdaptedInitialised.PushBack(false);
		m_AdaptedExposures.PushBack(1.0f);
	}

	// create a luminance resolve structure for each viewport
	for(U32 i = 0; i < m_Manager.GetViewCount(); ++i)
	{
		LuminanceResolve* pLuminanceResolve = new LuminanceResolve;
		if (!pLuminanceResolve->Create())
			return false;
		m_LuminanceResolves.PushBack(pLuminanceResolve);
	}

	// create parameters for each viewport
	if (m_Parameters.Size() != m_Manager.GetViewCount())
	{
		for (S32 i = 0; i < m_Parameters.Size(); ++i)
			delete m_Parameters[i];
		m_Parameters.Clear();

		for (U32 i = 0; i < m_Manager.GetViewCount(); ++i)
			m_Parameters.PushBack(new LuminanceParameters);
	}

	// initialise luminance parameter values
	for (S32 i = 0; i < m_Parameters.Size(); ++i)
	{
		LuminanceParameters* pParameters = m_Parameters[i];
		pParameters->fMiddleGrey = g_HDRMiddleGrey;
		pParameters->fExposureMin = g_HDRExposureMin;
		pParameters->fExposureMax = g_HDRExposureMax;
		pParameters->fGamma = 1.0f / g_HDRGamma;
	}

	return true;
}

void I_HighDynamicRange::Destroy()
{
	if(m_LuminanceInitial.IsCreated())
		Display::Device()->Destroy(&m_LuminanceInitial);
	if(m_LuminanceIterative.IsCreated())
		Display::Device()->Destroy(&m_LuminanceIterative);
	if(m_LuminanceFinal.IsCreated())
		Display::Device()->Destroy(&m_LuminanceFinal);
	if(m_LuminanceAdapted.IsCreated())
		Display::Device()->Destroy(&m_LuminanceAdapted);
	if(m_LuminanceClear.IsCreated())
		Display::Device()->Destroy(&m_LuminanceClear);
	if(m_LuminanceResolve.IsCreated())
		Display::Device()->Destroy(&m_LuminanceResolve);
	if(m_LuminanceDisplayExp.IsCreated())
		Display::Device()->Destroy(&m_LuminanceDisplayExp);
	if(m_LuminanceDisplay.IsCreated())
		Display::Device()->Destroy(&m_LuminanceDisplay);
	if(m_LuminanceWeightDisplay.IsCreated())
		Display::Device()->Destroy(&m_LuminanceWeightDisplay);
	if(m_DepthCentreTap.IsCreated())
		Display::Device()->Destroy(&m_DepthCentreTap);

	if(m_Noise.IsCreated())
		Display::Device()->Destroy(&m_Noise);
	if(m_BlackTexture.IsCreated())
		Display::Device()->Destroy(&m_BlackTexture);
}

void I_HighDynamicRange::DestroyVolatile()
{
	if(m_Luminance64.IsCreated())
		Display::Device()->Destroy(&m_Luminance64);
	if(m_Luminance16.IsCreated())
		Display::Device()->Destroy(&m_Luminance16);
	if(m_Luminance4.IsCreated())
		Display::Device()->Destroy(&m_Luminance4);
	if(m_CurrentLuminance.IsCreated())
		Display::Device()->Destroy(&m_CurrentLuminance);
	if(m_DepthBufferCentre.IsCreated())
		Display::Device()->Destroy(&m_DepthBufferCentre);

	for(S32 i = 0; i < m_AdaptedLuminances.Size(); ++i)
	{
		if(m_AdaptedLuminances[i]->IsCreated())
			Display::Device()->Destroy(m_AdaptedLuminances[i]);

		delete m_AdaptedLuminances[i];
	}
	m_AdaptedLuminances.Clear();
	m_AdaptedInitialised.Clear();
	m_AdaptedExposures.Clear();

	for(S32 i = 0; i < m_LuminanceResolves.Size(); ++i)
	{
		LuminanceResolve* pLuminanceResolve = m_LuminanceResolves[i];
		pLuminanceResolve->Destroy();
		delete pLuminanceResolve;
	}
	m_LuminanceResolves.Clear();

	for(S32 i = 0; i < m_Parameters.Size(); ++i)
		delete m_Parameters[i];
	m_Parameters.Clear();
}

const Display::C_Texture2D*	I_HighDynamicRange::GetAdaptedLuminanceTexture(S32 index) const
{
	if(index >= 0 && index < m_AdaptedLuminances.Size())
		return &m_AdaptedLuminances[index]->GetTexture();
	else
		return NULL;
}

const Display::C_Texture2D*	I_HighDynamicRange::GetAdaptedLuminanceTexture() const
{
	return GetAdaptedLuminanceTexture(m_Manager.GetViewIndex());
}

const Display::C_Texture2D*	I_HighDynamicRange::GetDepthBufferCentreTexture() const
{
	return &m_DepthBufferCentre.GetTexture();
}

F32 I_HighDynamicRange::GetAdaptedExposure() const
{
	S32 iCurrentView = m_Manager.GetViewIndex();
	if(iCurrentView >= 0 && iCurrentView < m_AdaptedExposures.Size())
		return m_AdaptedExposures[iCurrentView];
	else
		return 1.0f;	
}

void I_HighDynamicRange::SetAdaptedExposure(const F32 fAdaptedExposure)
{
	const S32 iCurrentView = m_Manager.GetViewIndex();
	if(iCurrentView >= 0 && iCurrentView < m_AdaptedExposures.Size())
		m_AdaptedExposures[iCurrentView] = fAdaptedExposure;
}

void I_HighDynamicRange::SetAdaptedExposure(const S32 iViewIndex, const F32 fAdaptedExposure)
{
	if(iViewIndex >= 0 && iViewIndex < m_AdaptedExposures.Size())
		m_AdaptedExposures[iViewIndex] = fAdaptedExposure;
}

void I_HighDynamicRange::SetGamma(const F32 fGamma)
{
	const S32 iCurrentView = m_Manager.GetViewIndex();
	if(iCurrentView >= 0 && iCurrentView < m_Parameters.Size())
		m_Parameters[iCurrentView]->fGamma = 1.0f / fGamma;
}

void I_HighDynamicRange::SetGamma(const S32 iViewIndex, const F32 fGamma)
{
	if(iViewIndex >= 0 && iViewIndex < m_Parameters.Size())
		m_Parameters[iViewIndex]->fGamma = 1.0f / fGamma;
}

MathsStorage::Float4 I_HighDynamicRange::GetShaderParameters(const S32 iViewIndex) const
{
	F32 fExposure = iViewIndex < m_AdaptedExposures.Size() ? m_AdaptedExposures[iViewIndex] : 1.0f;
	if (g_HDROverrideExposure)
		fExposure = g_HDRExposure;

	return MathsStorage::Float4(fExposure, 1.0f / fExposure, g_HDRLuminanceScale, 0.0f);
}

MathsStorage::Float4 I_HighDynamicRange::GetShaderParameters() const
{
	// hack to get around the fact that the cube map is being rendered
	// using HDR parameters but does not have an HDR view
	S32 iViewIndex = m_Manager.GetViewIndex();
	if (iViewIndex < 0 || iViewIndex >= m_Parameters.Size())
		iViewIndex = 0;

	return GetShaderParameters(iViewIndex);
}

void I_HighDynamicRange::CalculateLuminance() const
{
	// get the current contents of render target and set as a texture
	SetViewport();

	if(g_HDRLuminanceReset)
	{
		for(S32 i = 0; i < m_AdaptedInitialised.Size(); ++i)
		{
			m_AdaptedInitialised[i] = false;
		}
		for(S32 i = 0; i < m_LuminanceResolves.Size(); ++i)
		{
			m_LuminanceResolves[i]->Reset();
		}
		g_HDRLuminanceReset = false;
	}

	if(!(m_bCalculateLuminance && g_HDRLuminanceCalculate))
		return;

	assert(m_AdaptedLuminances.Size() == (S32)m_Manager.GetViewCount());

	const Display::C_Viewport& viewport = m_Manager.GetViewport();
	// make sure solid is set so inverse gamma is correct
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	MathsStorage::Float4 shaderParameters = GetShaderParameters();
	shaderParameters.z = 1.0f / shaderParameters.z; // invert luminance scale

	PERF_EVENT("PostProcess.Luminance");
	// calculate initial luminance into 64x64
	if(!m_bFastLuminance)
	{
		PERF_EVENT("PostProcess.Luminance.64x64");

		Display::Device()->SetPixelShaderParameterF(m_LuminanceInitialParam, (F32*)&shaderParameters);
		MathsStorage::Float4 vLuminanceInitialWeightParam(g_HDRLuminanceWeightX, g_HDRLuminanceWeightY, g_HDRLuminanceWeightScale, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_LuminanceInitialWeightParam, (F32*)&vLuminanceInitialWeightParam);

		m_Manager.SetRenderTexture(m_Luminance64);
		Display::Device()->SetTexture(0, m_Manager.GetMainTexture());
		DisplayIntermediate(&m_LuminanceInitial, Display::E_TextureFilter_Point);
		Resolve();
	}
	else	
	{
		PERF_EVENT("PostProcess.Luminance.16x16");

		Display::Device()->SetPixelShaderParameterF(m_LuminanceInitialParam, (F32*)&shaderParameters);
		MathsStorage::Float4 vLuminanceInitialWeightParam(g_HDRLuminanceWeightX, g_HDRLuminanceWeightY, g_HDRLuminanceWeightScale, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_LuminanceInitialWeightParam, (F32*)&vLuminanceInitialWeightParam);

		m_Manager.SetRenderTexture(m_Luminance16);
		Display::Device()->SetTexture(0, m_Manager.GetMainTexture());
		DisplayIntermediate(&m_LuminanceInitial, Display::E_TextureFilter_Point);
		Resolve();
	}
	
	// downsample 64x64 to 16x16
	if(!m_bFastLuminance)
	{
		PERF_EVENT("PostProcess.Luminance.16x16");
		MathsStorage::Float4 vTaps[16];
		GetBoxSamples<4, 4>(64, 64, vTaps);
		Display::Device()->SetPixelShaderParameterArrayF(m_LuminanceIterativeTaps, (F32*)&vTaps);
		Display::Device()->SetPixelShaderParameters(&m_LuminanceIterative);

		m_Manager.SetRenderTexture(m_Luminance16);
		Display::Device()->SetTexture(0, &m_Luminance64.GetTexture());
		DisplayIntermediate(&m_LuminanceIterative, Display::E_TextureFilter_Point);
		Resolve();
	}

	// downsample 16x16 to 4x4
	{
		PERF_EVENT("PostProcess.Luminance.4x4");
		MathsStorage::Float4 vTaps[16];
		GetBoxSamples<4, 4>(16, 16, vTaps);
		Display::Device()->SetPixelShaderParameterArrayF(m_LuminanceIterativeTaps, (F32*)&vTaps);
		Display::Device()->SetPixelShaderParameters(&m_LuminanceIterative);
		
		m_Manager.SetRenderTexture(m_Luminance4);
		Display::Device()->SetTexture(0, &m_Luminance16.GetTexture());
		DisplayIntermediate(&m_LuminanceIterative, Display::E_TextureFilter_Point);
		Resolve();
	}

	U32 iCurrentView = m_Manager.GetViewIndex();
	// downsample 4x4 to 1x1
	{
		PERF_EVENT("PostProcess.Luminance.1x1");
		MathsStorage::Float4 vTaps[16];
		GetBoxSamples<4, 4>(4, 4, vTaps);
		Display::Device()->SetPixelShaderParameterArrayF(m_LuminanceFinalTaps, (F32*)&vTaps);

		LuminanceParameters* pParameters = m_Parameters[iCurrentView];

		MathsStorage::Float4 vLuminanceParam;
		vLuminanceParam.x = (pParameters->fMiddleGrey - 0.001f * pParameters->fExposureMax) / pParameters->fExposureMax;
		vLuminanceParam.y = (pParameters->fMiddleGrey - 0.001f * pParameters->fExposureMin) / pParameters->fExposureMin;
		vLuminanceParam.z = g_HDRLuminanceAdjustment;
		Display::Device()->SetPixelShaderParameterF(m_LuminanceFinalParam, (F32*)&vLuminanceParam);
		
		m_Manager.SetRenderTexture(m_CurrentLuminance);
		Display::Device()->SetTexture(0, &m_Luminance4.GetTexture());
		DisplayIntermediate(&m_LuminanceFinal, Display::E_TextureFilter_Point);
		Resolve();
	}
	
	// tap centre depth buffer texel.
	{
		PERF_EVENT("PostProcess.DepthBufferTap1x1");
		m_Manager.SetRenderTexture(m_DepthBufferCentre);		// for now need to handle double buffer
		Display::Device()->SetTexture(0, Render::GetGlobalDepthStencilBuffer());
		DisplayIntermediate(&m_DepthCentreTap, Display::E_TextureFilter_Point);
		Resolve();
	}

	if(!m_AdaptedInitialised[iCurrentView])
	{
		PERF_EVENT("PostProcess.Luminance.ClearAdapted");
		m_Manager.SetRenderTexture(*m_AdaptedLuminances[iCurrentView]);

		LuminanceParameters* pParameters = m_Parameters[iCurrentView];

		// set the luminance to be the average of the min and max exposures
		F32 fAvgExposure = (pParameters->fExposureMin + pParameters->fExposureMax) * 0.5f;
		F32 fAvgLuminance = (pParameters->fMiddleGrey - 0.001f * fAvgExposure) / fAvgExposure;
		
		MathsStorage::Float4 vLuminanceClearParam(fAvgLuminance, fAvgLuminance, fAvgLuminance, fAvgLuminance);
		Display::Device()->SetPixelShaderParameterF(m_LuminanceClearParam, (F32*)&vLuminanceClearParam);
		
		DisplayIntermediate(&m_LuminanceClear, Display::E_TextureFilter_Point);
		m_AdaptedInitialised[iCurrentView] = true;
		Resolve();
	}
	else
	{
		PERF_EVENT("PostProcess.Luminance.Adapted");
		MathsStorage::Float4 vSettings;
		vSettings.x = m_Manager.GetFrameTime() * g_HDRExposureAdaptSpeed;
		vSettings.y = g_HDRLuminanceAdjustment;
		Display::Device()->SetPixelShaderParameterF(m_LuminanceAdaptedParam, (F32*)&vSettings);

		m_Manager.SetRenderTexture(*m_AdaptedLuminances[iCurrentView]);
		Display::Device()->SetTexture(0, &m_CurrentLuminance.GetTexture());
		Display::Device()->SetTexture(1, &m_AdaptedLuminances[iCurrentView]->GetTexture());
		Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureAddressUVW(1, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);	
		DisplayIntermediate(&m_LuminanceAdapted, Display::E_TextureFilter_Point);
		Resolve();
	}

	ResolveLuminance();

	Display::Device()->SetTexture(0, NULL);	
	Display::Device()->SetTexture(1, NULL);	
	Display::Device()->SetTexture(2, NULL);
}

void I_HighDynamicRange::ResolveLuminance() const
{
	assert(m_LuminanceResolves.Size() == m_Manager.GetViewCount());

	const U32 iCurrentView = m_Manager.GetViewIndex();
	LuminanceResolve* pLuminanceResolve = m_LuminanceResolves[iCurrentView];
	LuminanceParameters* pParameters = m_Parameters[iCurrentView];

	// read back last luminance
	if (pLuminanceResolve->IsInitialised())
	{
		#if !defined(PLATFORM_PC)
		Display::C_LockedRect lockedRect = pLuminanceResolve->GetRenderTexture()->GetTexture().Lock(0);
		#else
		Display::C_LockedRect lockedRect = pLuminanceResolve->GetTexture()->Lock(0);
		#endif
		if (lockedRect.pBits)
		{
			const F32 fAdaptedLuminance = *((F32*)lockedRect.pBits);
			const F32 fAdaptedExposure = pParameters->fMiddleGrey / (fAdaptedLuminance + 0.001f);
			m_AdaptedExposures[iCurrentView] = fAdaptedExposure;
		}
		#if !defined(PLATFORM_PC)
		pLuminanceResolve->GetRenderTexture()->GetTexture().Unlock(0);
		#else
		pLuminanceResolve->GetTexture()->Unlock(0);
		#endif
	}
	else
	{
		// set the exposure to be the average of the min and max exposures
		m_AdaptedExposures[iCurrentView] = (pParameters->fExposureMin + pParameters->fExposureMax) * 0.5f;
	}

	// calculate new luminance
	{
		#if !defined(PLATFORM_PC)
		PERF_EVENT("ResolveLuminance");
		m_Manager.SetRenderTexture(*pLuminanceResolve->GetRenderTexture());
		Display::Device()->SetTexture(0, &m_AdaptedLuminances[iCurrentView]->GetTexture());
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);	
		DisplayIntermediate(&m_LuminanceResolve, Display::E_TextureFilter_Point);
		Resolve();
		#else // [Blur1PCDX11] Native luminance resolve copy.
		ID3D11Texture2D* source = m_AdaptedLuminances[iCurrentView]->GetSurface().m_pd3dSurface;
		ID3D11Texture2D* destination = pLuminanceResolve->GetTexture()->GetGPUTexture2D();
		if (source && destination)
		{
			ID3D11DeviceContext* context = nullptr;
			Display::Device()->GetD3DDevice()->GetImmediateContext(&context);
			if (context)
			{
				context->CopySubresourceRegion(destination, 0, 0, 0, 0, source, 0, nullptr);
				Display2::D3D11ResourceWritten(destination, context); // [Blur1PCDX11] system-memory readback ring
				context->Release();
			}
		}
		#endif
	}
	
	pLuminanceResolve->Next();
}

void I_HighDynamicRange::ClearLuminance() const
{
	U32 iCurrentView = m_Manager.GetViewIndex();
	if(!m_AdaptedInitialised[iCurrentView])
	{
		PERF_EVENT("ClearAdapted");

		// set the luminance to be the average of the min and max exposures
		F32 fAvgExposure = (g_HDRExposureMin + g_HDRExposureMax) * 0.5f;
		F32 fAvgLuminance = (g_HDRMiddleGrey - 0.001f * fAvgExposure) / fAvgExposure;
		
		MathsStorage::Float4 vLuminanceClearParam(fAvgLuminance, fAvgLuminance, fAvgLuminance, fAvgLuminance);
		Display::Device()->SetPixelShaderParameterF(m_LuminanceClearParam, (F32*)&vLuminanceClearParam);

		// depending on whether we are inside a Begin/End PostProcess
		if(m_Manager.Enabled())
		{
			const Display::C_RenderTexture2D* pRenderTexture = m_Manager.GetRenderTexture();
			m_Manager.SetRenderTexture(*m_AdaptedLuminances[iCurrentView]);
			DisplayIntermediate(&m_LuminanceClear, Display::E_TextureFilter_Point);
			Resolve();
			m_Manager.SetRenderTexture(*pRenderTexture);
		}
		else
		{
			const Display::C_Surface* pColourSurface = Display::Device()->GetRenderTarget().GetColourSurface();
			const Display::C_Surface* pDepthStencilSurface = Display::Device()->GetRenderTarget().GetDepthStencilSurface();
			Display::C_Viewport oldViewport = Display::Device()->GetViewport();

			// set luminance texture
			Display::Device()->SetColourDepthStencilSurface(&m_AdaptedLuminances[iCurrentView]->GetSurface(), NULL);
			Display::C_Viewport viewport;
			viewport.m_W = 1;
			viewport.m_H = 1;
			Display::Device()->SetViewport(viewport);

			DisplayIntermediate(&m_LuminanceClear, Display::E_TextureFilter_Point);


			Display::Device()->SetColourDepthStencilSurface(pColourSurface, pDepthStencilSurface);
			Display::Device()->SetViewport(oldViewport);
		}		

		m_AdaptedInitialised[iCurrentView] = true;
	}
}

void I_HighDynamicRange::Display() const
{
	if(g_HDRLuminanceShow64)
	{
		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(&m_Luminance64.GetTexture(), &m_LuminanceDisplayExp);
	}
	else if(g_HDRLuminanceShow16)
	{
		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(&m_Luminance16.GetTexture(), &m_LuminanceDisplayExp);
	}
	else if(g_HDRLuminanceShow4)
	{
		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(&m_Luminance4.GetTexture(), &m_LuminanceDisplayExp);
	}
	else if(g_HDRLuminanceShow1)
	{
		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(&m_CurrentLuminance.GetTexture(), &m_LuminanceDisplay);
	}
	else if(g_HDRLuminanceShowAdapted)
	{
		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(&m_AdaptedLuminances[m_Manager.GetViewIndex()]->GetTexture(), &m_LuminanceDisplay);
	}
	else if(g_HDRShowNoise)
	{
		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(&m_Noise);
	}
	else if(g_HDRLuminanceShowWeight)
	{
		MathsStorage::Float4 vLuminanceWeightDisplayParam(g_HDRLuminanceWeightX, g_HDRLuminanceWeightY, g_HDRLuminanceWeightScale, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_LuminanceWeightDisplayParam, (F32*)&vLuminanceWeightDisplayParam);

		Quad::TextureFilter(Display::E_TextureFilter_Point);
		Quad::DrawFullScreen(NULL, &m_LuminanceWeightDisplay);
	}
	// debug text
	if (g_HDRShowExposure)
	{
		const U32 iCurrentView		= m_Manager.GetViewIndex();
		const F32 fAdaptedExposure	= m_AdaptedExposures[iCurrentView];
		const F32 fX				= 64.0f;
		const F32 fY				= 64.0f + 64.0f * iCurrentView;

		Util::C_String sExposure;
		sExposure.Printf("%2.4f", fAdaptedExposure);
		DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dText(Maths::HVec2(fX, fY), sExposure);
	}
}

void I_HighDynamicRange::SetHighDynamicRangeType(const E_HighDynamicRangeType eType)
{
	m_eType = eType;
}

LuminanceResolve::LuminanceResolve() : m_Index(0), m_Count(0)
{
}

bool LuminanceResolve::Create()
{
	for(S32 j = 0; j < MAX_LUMINANCE_BUFFERS; ++j)
	{
	#if !defined(PLATFORM_PC)
		Display::Device()->CreateRenderTexture2D(1, 1, 1, Display::E_Usage_Colour, Display::E_Format_R32F, Display::E_Multisample_None, NULL, &m_RenderTextures[j]);
		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(&m_RenderTextures[j], "_DHRLuminanceResolveRT");
		#endif
	#else
		Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_System, Display::E_Format_R32F, &m_Textures[j]);
		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_Textures[j], "_HDRLuminanceResolveRT");
		#endif
	#endif
	}
	m_Index = 0;
	m_Count = 0;
	return true;
}

void LuminanceResolve::Destroy()
{
	for(S32 j = 0; j < MAX_LUMINANCE_BUFFERS; ++j)
	{
		#if !defined(PLATFORM_PC)
		Display::Device()->Destroy(&m_RenderTextures[j]);
		#else
		Display::Device()->Destroy(&m_Textures[j]);
		#endif
	}
}

bool LuminanceResolve::IsInitialised() const
{
	return (m_Count >= MAX_LUMINANCE_BUFFERS * 2);
}

}; // namespace PostProcess
