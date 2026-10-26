#include "StdAfx.h"
#include "Render/PostProcess/HDR/HDR.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

#include "System/Misc/Random.h"

EXTERN_SHADER(psToneMap);
EXTERN_SHADER(psToneMapHDR);
EXTERN_SHADER(psToneMapHDRU32);
EXTERN_SHADER(psBrightPass);
EXTERN_SHADER(psBrightPassHDR);
EXTERN_SHADER(psBrightPassHDRU32);
EXTERN_SHADER(psBrightPassGamma);
EXTERN_SHADER(psBrightPassGammaHDR);
EXTERN_SHADER(psBrightPassGammaHDRU32);
EXTERN_SHADER(psBrightPassLookup);
EXTERN_SHADER(psBrightPassLookupHDR);
EXTERN_SHADER(psBrightPassLookupHDRU32);
EXTERN_SHADER(psBrightPassLookupGamma);
EXTERN_SHADER(psBrightPassLookupGammaHDR);
EXTERN_SHADER(psBrightPassLookupGammaHDRU32);
EXTERN_SHADER(psBrightPassReducedAlphaLookupGamma);
EXTERN_SHADER(psBrightPassReducedAlphaLookupGammaHDR);
EXTERN_SHADER(psBrightPassReducedAlphaLookupGammaHDRU32);
EXTERN_SHADER(psBloomDisplay);
EXTERN_SHADER(psBloomDisplayGamma);

using namespace Display;

namespace PostProcess
{

Util::C_Random g_Random;

Debug::Bool g_HDR("Graphics.Render.PostProcess.HDR.Enable", true);
Debug::Bool g_HDRTemporalDither("Graphics.Render.PostProcess.HDR.TemporalDither", true);
Debug::Bool g_HDRPointFilter("Graphics.Render.PostProcess.HDR.Bloom.PointFilter", false);

// game modifiable parameters

Debug::Float g_HDRBloomThreshold("Graphics.Render.PostProcess.HDR.Parameters.BloomThreshold", 1.0f, Debug::Range(0.f, 10.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomOffset("Graphics.Render.PostProcess.HDR.Parameters.BloomOffset", 2.0f, Debug::Range(0.f, 10.0f), Debug::Step(0.1f));

Debug::Bool g_HDROverride("Graphics.Render.PostProcess.HDR.Parameters.Override", false);

Debug::Bool g_HDRBloomEnable("Graphics.Render.PostProcess.HDR.Bloom.Enable", true);
Debug::Bool g_HDRBloomShow("Graphics.Render.PostProcess.HDR.Bloom.Show", false);

#if defined(PLATFORM_PS3)
Debug::Bool g_HDRFakeAA("Graphics.Render.PostProcess.HDR.FakeAA", true);
#else
Debug::Bool g_HDRFakeAA("Graphics.Render.PostProcess.HDR.FakeAA", false);
#endif
Debug::Float g_HDRFakeAABiasX("Graphics.Render.PostProcess.HDR.FakeAABiasX", 1.0f, Debug::Range(-4.0f, 4.0f));
Debug::Float g_HDRFakeAABiasY("Graphics.Render.PostProcess.HDR.FakeAABiasY", 1.0f, Debug::Range(-4.0f, 4.0f));

extern Debug::Float g_HDRBloomDownsampleScale0;

extern Debug::Float g_HDRMiddleGrey;
extern Debug::Float g_HDRExposureMin;
extern Debug::Float g_HDRExposureMax;
extern Debug::Float g_HDRGamma;
extern Debug::Float g_HDRLuminanceScale;

C_HighDynamicRange::C_HighDynamicRange(C_Manager& manager) : I_HighDynamicRange(manager), m_BrightPass(manager)
{
	m_TextureFilter = Display::E_TextureFilter_Point;

	// bright pass doesn't need to resolve since the frame texture has already been resolved at start of HDR
	// it also is not chainable so should revert to full sized viewport
	m_BrightPass.SetResolve(E_Resolve_False);
	m_BrightPass.SetChainable(E_Chainable_False);
	m_BrightPass.I_PostProcess::SetShader(&m_BrightPassShader[0]);
	m_BrightPass.SetConstants(false); // don't need to set filter constants
	m_BrightPass.SetTextureFilter(Display::E_TextureFilter_Point);

	m_pBloom = 0;
	m_pBloomCurveTexture = 0;

	m_bCalculateLuminanceAuto = true;

	m_pReducedAlphaTexture = NULL;
}

C_HighDynamicRange::~C_HighDynamicRange()
{
}

bool C_HighDynamicRange::Create()
{
	RESOURCE_MEMORY_ID("HDR");

	// tonemap
	{
		if(!m_pShader)
		{
			if(m_eType == E_HighDynamicRangeType_Shader)
			{
				if(!Display::CreatePixelShaderFromFunc("TonemapHDR", 0, &m_ToneMap, REF_SHADER(psToneMapHDR)))
					return false;
			}
			else if(m_eType == E_HighDynamicRangeType_ShaderU32)
			{
				if(!Display::CreatePixelShaderFromFunc("TonemapHDRU32", 0, &m_ToneMap, REF_SHADER(psToneMapHDRU32)))
					return false;
			}
			else
			{
				if(!Display::CreatePixelShaderFromFunc("Tonemap", 0, &m_ToneMap, REF_SHADER(psToneMap)))
					return false;
			}
			SetShader(&m_ToneMap);
		}
	}

	// bloom
	{
		// bright pass
		if(m_eType == E_HighDynamicRangeType_Shader)
		{
			if(!Display::CreatePixelShaderFromFunc("BrightPassHDR", 0, &m_BrightPassShader[E_BrightPassShader_Default], REF_SHADER(psBrightPassHDR)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassGammaHDR", 0, &m_BrightPassShader[E_BrightPassShader_Gamma], REF_SHADER(psBrightPassGammaHDR)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassLookupHDR", 0, &m_BrightPassShader[E_BrightPassShader_Lookup], REF_SHADER(psBrightPassLookupHDR)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassLookupGammaHDR", 0, &m_BrightPassShader[E_BrightPassShader_LookupGamma], REF_SHADER(psBrightPassLookupGammaHDR)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassReducedAlphaLookupGammaHDR", 0, &m_BrightPassShader[E_BrightPassShader_ReducedAlphaLookupGamma], REF_SHADER(psBrightPassReducedAlphaLookupGammaHDR)))
				return false;
		}
		else if(m_eType == E_HighDynamicRangeType_ShaderU32)
		{
			if(!Display::CreatePixelShaderFromFunc("BrightPassHDRU32", 0, &m_BrightPassShader[E_BrightPassShader_Default], REF_SHADER(psBrightPassHDRU32)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassGammaHDRU32", 0, &m_BrightPassShader[E_BrightPassShader_Gamma], REF_SHADER(psBrightPassGammaHDRU32)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassLookupHDRU32", 0, &m_BrightPassShader[E_BrightPassShader_Lookup], REF_SHADER(psBrightPassLookupHDRU32)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassLookupGammaHDRU32", 0, &m_BrightPassShader[E_BrightPassShader_LookupGamma], REF_SHADER(psBrightPassLookupGammaHDRU32)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassReducedAlphaLookupGammaHDRU32", 0, &m_BrightPassShader[E_BrightPassShader_ReducedAlphaLookupGamma], REF_SHADER(psBrightPassReducedAlphaLookupGammaHDRU32)))
				return false;
		}
		else
		{
			if(!Display::CreatePixelShaderFromFunc("BrightPass", 0, &m_BrightPassShader[E_BrightPassShader_Default], REF_SHADER(psBrightPass)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassGamma", 0, &m_BrightPassShader[E_BrightPassShader_Gamma], REF_SHADER(psBrightPassGamma)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassLookup", 0, &m_BrightPassShader[E_BrightPassShader_Lookup], REF_SHADER(psBrightPassLookup)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassLookupGamma", 0, &m_BrightPassShader[E_BrightPassShader_LookupGamma], REF_SHADER(psBrightPassLookupGamma)))
				return false;
			if(!Display::CreatePixelShaderFromFunc("BrightPassReducedAlphaLookupGamma", 0, &m_BrightPassShader[E_BrightPassShader_ReducedAlphaLookupGamma], REF_SHADER(psBrightPassReducedAlphaLookupGamma)))
				return false;
		}

		for(S32 i = 0; i < E_BrightPassShader_Count; ++i)
		{
			m_BrightPassSettings1[i] = m_BrightPassShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_14, "g_vSettings1");
			m_BrightPassSettings2[i] = m_BrightPassShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_15, "g_vSettings2");
		}

		// bloom display
		if(!Display::CreatePixelShaderFromFunc("BloomDisplay", 0, &m_BloomDisplay, REF_SHADER(psBloomDisplay)))
			return false;
		if(!Display::CreatePixelShaderFromFunc("BloomDisplayGamma", 0, &m_BloomDisplayGamma, REF_SHADER(psBloomDisplayGamma)))
			return false;

		m_BloomDisplayParam = m_BloomDisplay.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings1");
		m_BloomDisplayGammaParam = m_BloomDisplayGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings1");
	}

	return I_HighDynamicRange::Create();
}

bool C_HighDynamicRange::CreateVolatile()
{
	if (m_Parameters.Size() != m_Manager.GetViewCount())
	{
		for (S32 i = 0; i < m_Parameters.Size(); ++i)
			delete m_Parameters[i];
		m_Parameters.Clear();

		for (U32 i = 0; i < m_Manager.GetViewCount(); ++i)
			m_Parameters.PushBack(new HighDynamicRangeParameters);
	}

	for (S32 i = 0; i < m_Parameters.Size(); ++i)
	{
		HighDynamicRangeParameters* pParameters = (HighDynamicRangeParameters*)m_Parameters[i];
		pParameters->fMiddleGrey = g_HDRMiddleGrey;
		pParameters->fExposureMin = g_HDRExposureMin;
		pParameters->fExposureMax = g_HDRExposureMax;
		pParameters->fGamma = 1.0f / g_HDRGamma;
		pParameters->fBloomThreshold = g_HDRBloomThreshold;
		
		// dont do first downscale
		pParameters->bDownsample[0] = false;
	}

	return I_HighDynamicRange::CreateVolatile();
}

void C_HighDynamicRange::Destroy()
{
	for(S32 i = 0; i < E_BrightPassShader_Count; ++i)
	{
		if(m_BrightPassShader[i].IsCreated())
			Display::Device()->Destroy(&m_BrightPassShader[i]);
	}
	if(m_BloomDisplay.IsCreated())
		Display::Device()->Destroy(&m_BloomDisplay);
	if(m_BloomDisplayGamma.IsCreated())
		Display::Device()->Destroy(&m_BloomDisplayGamma);
	if(m_ToneMap.IsCreated())
		Display::Device()->Destroy(&m_ToneMap);

	I_HighDynamicRange::Destroy();
}

void C_HighDynamicRange::DestroyVolatile()
{
	I_HighDynamicRange::DestroyVolatile();
}

void C_HighDynamicRange::SetBloom(I_Bloom* pBloom)
{
	assert(pBloom);
	m_pBloom = pBloom;
}

void C_HighDynamicRange::SetBloomGamma(const bool bBloomGamma) const
{
	m_pBloom->SetBloomGamma(bBloomGamma);
}

void C_HighDynamicRange::SetBloomCurve(const Display::C_Texture2D* pTexture) const
{
	m_pBloomCurveTexture = pTexture;
}

void C_HighDynamicRange::SetUserParameters(const MathsStorage::Float4& settings1, const MathsStorage::Float4& settings2) const
{
}

void C_HighDynamicRange::UnsetUserParameters() const
{
}

void C_HighDynamicRange::UpdateParameters() const
{
#if !defined(CFG_SPEED)
	if (g_HDROverride)
	{	
		for (S32 i = 0; i < m_Parameters.Size(); ++i)
		{
			HighDynamicRangeParameters* pParameters = (HighDynamicRangeParameters*)m_Parameters[i];
			pParameters->fMiddleGrey = g_HDRMiddleGrey;
			pParameters->fExposureMin = g_HDRExposureMin;
			pParameters->fExposureMax = g_HDRExposureMax;
			pParameters->fGamma = 1.0f / g_HDRGamma;
			pParameters->fBloomThreshold = g_HDRBloomThreshold;
		}
	}
	// clamp values in range
	for (S32 i = 0; i < m_Parameters.Size(); ++i)
	{
		HighDynamicRangeParameters* pParameters = (HighDynamicRangeParameters*)m_Parameters[i];
		if (pParameters->fMiddleGrey < 0.0001f)
			pParameters->fMiddleGrey = 0.0001f;
	}	
#endif
}

HighDynamicRangeParameters* C_HighDynamicRange::GetParameters(const S32 iViewIndex)
{
	if(iViewIndex < m_Parameters.Size())
		return (HighDynamicRangeParameters*)m_Parameters[iViewIndex];
	else
		return NULL;
}

const Display::C_Texture2D* C_HighDynamicRange::CreateBloom() const
{
	PERF_EVENT("Bloom");

	if (!m_pBloom)
		return &m_BlackTexture;

	E_BrightPassShader	shader;

	if(m_pBloom->GetBloomGamma())
	{
		if(m_pBloomCurveTexture)
		{
			if(m_pReducedAlphaTexture)
				shader = E_BrightPassShader_ReducedAlphaLookupGamma;
			else
				shader = E_BrightPassShader_LookupGamma;
		}
		else
			shader = E_BrightPassShader_Gamma;
	}
	else
	{
		if(m_pBloomCurveTexture)
			shader = E_BrightPassShader_Lookup;
		else
			shader = E_BrightPassShader_Default;
	}

	m_BrightPass.I_PostProcess::SetShader(&m_BrightPassShader[shader]);
	HighDynamicRangeParameters* pParameters = (HighDynamicRangeParameters*)m_Parameters[m_Manager.GetViewIndex()];
	
	const F32 fBloomOffset = (g_HDRBloomOffset < pParameters->fBloomThreshold) ? pParameters->fBloomThreshold : g_HDRBloomOffset;
	const F32 fBloomDownsample = g_HDRBloomDownsampleScale0;

	// set main frame buffer to texture 0
	Display::Device()->SetTexture(0, m_Manager.GetMainTexture());

	MathsStorage::Float4 vSettings1(pParameters->fMiddleGrey, pParameters->fBloomThreshold, fBloomOffset, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_BrightPassSettings1[shader], (F32*)&vSettings1);

	const F32 fAdaptedExposure = m_AdaptedExposures[m_Manager.GetViewIndex()];
	MathsStorage::Float4 vSettings2(fBloomDownsample, 0.2125f * g_HDRLuminanceScale / fAdaptedExposure, 0.7154f * g_HDRLuminanceScale / fAdaptedExposure, 0.0721f * g_HDRLuminanceScale / fAdaptedExposure);
	Display::Device()->SetPixelShaderParameterF(m_BrightPassSettings2[shader], (F32*)&vSettings2);

	Display::Device()->SetTexture(1, m_pBloomCurveTexture);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);

	if(m_pReducedAlphaTexture)
	{
		Display::Device()->SetTexture(2, m_pReducedAlphaTexture);
		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);
	}

	Display::Device()->SetTexture(15, &m_AdaptedLuminances[m_Manager.GetViewIndex()]->GetTexture());
	Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);

	// perform brightpass
	m_BrightPass.SetDownsample(4);
	m_BrightPass.Display();

	m_pBloom->Display(*pParameters);

	return &m_Manager.GetRenderTexture()->GetTexture();
}

void C_HighDynamicRange::Display() const
{
	if(g_HDR && m_Manager.Enabled())
	{
		PERF_EVENT("HDR");
		Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

		UpdateParameters();

		// do luminance calculation
		if(m_bCalculateLuminanceAuto)
			CalculateLuminance();
		else
			SetViewport();

		// create bloom textures
		const Display::C_Texture2D* pBloomTexture;
		
		if(g_HDRBloomEnable)
			pBloomTexture = CreateBloom();
		else
			pBloomTexture = &m_BlackTexture;
		
		{
			PERF_EVENT("Tonemap");

			U32 iWidth = 0;
			U32 iHeight = 0;
			// set original framebuffer to stage 0
			{
				const Display::C_View& view = *Display::Device()->GetView();
				iWidth = view.GetViewport().GetW();
				iHeight = view.GetViewport().GetH();
				Display::C_Viewport viewport = view.GetViewport();
				viewport.m_X = 0;
				viewport.m_Y = 0;
				m_Manager.SetViewport(viewport);
				Display::Device()->SetTexture(0, m_Manager.GetMainTexture());
				Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
				Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
			}

			// set luminance texture to stage 1
			{
				Display::Device()->SetTexture(1, &m_AdaptedLuminances[m_Manager.GetViewIndex()]->GetTexture());
				Display::Device()->SetTextureAddressUVW(1, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
				Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
			}

			// set noise texture to stage 2
			Display::Device()->SetTexture(2, &m_Noise);
			Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Wrap);
			Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);

			// set bloom texture to stage 3
			Display::Device()->SetTexture(3, pBloomTexture);
			Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Clamp);
			Display::Device()->SetTextureFilter(3, g_HDRPointFilter ? Display::E_TextureFilter_Point : Display::E_TextureFilter_Linear);

			const F32 fInvBloomGamma = (m_pBloom && m_pBloom->GetBloomGamma()) ? 2.0f : 1.0f;

			HighDynamicRangeParameters* pParameters = (HighDynamicRangeParameters*)m_Parameters[m_Manager.GetViewIndex()];
			MathsStorage::Float4 vSettings1(pParameters->fMiddleGrey, pParameters->fGamma, 1.0f / pParameters->fGamma, fInvBloomGamma);
			MathsStorage::Float4 vSettings2((F32)iWidth * m_NoiseTextureRcp, (F32)iHeight * m_NoiseTextureRcp, g_HDRTemporalDither ? 1.0f/255.0f : 0.f, 0.f);
			
			SetUserParameters(vSettings1, vSettings2);
			
			if(g_HDRFakeAA)
			{
				Display::Quad::SetQuadBias(g_HDRFakeAABiasX, g_HDRFakeAABiasY);
				m_TextureFilter = Display::E_TextureFilter_Linear;
			}

			if(g_HDRBloomShow && m_pBloom)
			{
				const Display::C_PixelShader* pBloomDisplay;
				const Display::C_PixelShaderParameter* pBloomDisplayParam;

				if(m_pBloom->GetBloomGamma())
				{
					pBloomDisplay = &m_BloomDisplayGamma;
					pBloomDisplayParam = &m_BloomDisplayGammaParam;
				}
				else
				{
					pBloomDisplay = &m_BloomDisplay;
					pBloomDisplayParam = &m_BloomDisplayParam;
				}

				Display::Device()->SetPixelShaderParameterF(*pBloomDisplayParam, (F32*)&vSettings1);
				Quad::TextureFilter(Display::E_TextureFilter_Linear);
				Quad::DrawFullScreen(NULL, pBloomDisplay);
			}
			else
				DisplayFinal();

			I_HighDynamicRange::Display();

			if(g_HDRFakeAA)
			{
				Display::Quad::SetQuadBias();
				m_TextureFilter = Display::E_TextureFilter_Point;
			}

			Display::Device()->SetTexture(0, NULL);
			Display::Device()->SetTexture(1, NULL);
			Display::Device()->SetTexture(2, NULL);

			UnsetUserParameters();
		}

		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}
}

}; // namespace PostProcess
