#include "StdAfx.h"
#include "Render/PostProcess/Bloom/BloomSun.h"

#include <Render/PostProcess/Manager.h>
#include <Render/PostProcess/Util.h>
#include <Render/Shaders/Constants.h>

EXTERN_SHADER(psBloomCombineSun);
EXTERN_SHADER(psBloomCombineSunGamma);

using namespace Display;

namespace PostProcess
{

Debug::Float g_HDRBloomSunScale("Graphics.Render.PostProcess.HDR.Bloom.SunScale", 1.0f, Debug::Range(0.f, 4.0f), Debug::Step(0.1f));

extern Debug::Float g_HDRGamma;
extern Debug::Bool g_HDRPointFilter;

C_BloomSun::C_BloomSun(C_Manager& manager) : I_Bloom(manager), m_SunRays(manager)
{
	m_bBloomSun = false;
}

C_BloomSun::~C_BloomSun()
{
}

bool C_BloomSun::Create()
{
	RESOURCE_MEMORY_ID("HDR");

	// sun
	if(!Display::CreatePixelShaderFromFunc("BloomSunCombine", 0, &m_BloomCombine, REF_SHADER(psBloomCombineSun)))
		return false;
	m_BloomCombineSunBloomSettings = m_BloomCombine.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vBloomSettings");
	m_BloomCombineSunSunSettings = m_BloomCombine.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vSunSettings");

	// sun gamma
	if(!Display::CreatePixelShaderFromFunc("BloomCombineSunGamma", 0, &m_BloomCombineGamma, REF_SHADER(psBloomCombineSunGamma)))
		return false;
	m_BloomCombineSunGammaBloomSettings = m_BloomCombineGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vBloomSettings");
	m_BloomCombineSunGammaSunSettings = m_BloomCombineGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vSunSettings");


	return I_Bloom::Create();
}

bool C_BloomSun::CreateVolatile()
{
	return I_Bloom::CreateVolatile();
}

void C_BloomSun::Destroy()
{
	I_Bloom::Destroy();
}

void C_BloomSun::DestroyVolatile()
{
	I_Bloom::DestroyVolatile();
}

void C_BloomSun::SetBloomSun(const bool bBloomSun) const
{
	m_bBloomSun = bBloomSun;
}

void C_BloomSun::SetBloomSunPosition(const Display::C_Float2& vSunPosition) const
{
	m_SunRays.SetSunPosition(vSunPosition);
}

void C_BloomSun::SetBloomSunSize(const Display::C_Float2& vSunSize) const
{
	m_SunRays.SetSunSize(vSunSize);
}

void C_BloomSun::SetBloomSunIntensity(const F32 fSunIntensity) const
{
	m_SunRays.SetSunIntensity(fSunIntensity);
}

const Display::C_Texture* C_BloomSun::Display(const BloomParameters& parameters) const
{
	if(m_Manager.Enabled())
	{
		PERF_EVENT("BloomSun");

		Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

		// Calculate bloom textures
		RenderTexturesConst bloomTextures;
		CalculateBloomTextures(parameters, bloomTextures);

		// Sun rays
		if(m_bBloomSun)
		{
			m_SunRays.SetTexture(&bloomTextures[0]->GetTexture());
			m_SunRays.Display();
		}

		// Combine the bloom textures into a single render target
		{
			PERF_EVENT("Combine");
			const Display::C_PixelShader* pCombineShader;

			// set bloom textures
			for(S32 i = 0; i < bloomTextures.Size(); ++i)
			{
				Display::Device()->SetTexture(i, &bloomTextures[i]->GetTexture());
				Display::Device()->SetTextureAddressUVW(i, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
				if(g_HDRPointFilter)
					Display::Device()->SetTextureFilter(i, Display::E_TextureFilter_Point);
				else
					Display::Device()->SetTextureFilter(i, Display::E_TextureFilter_Linear);
			}
			Display::Device()->SetTexture(bloomTextures.Size(), m_SunRays.GetTexture());
			Display::Device()->SetTextureAddressUVW(bloomTextures.Size(), Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
			if(g_HDRPointFilter)
				Display::Device()->SetTextureFilter(bloomTextures.Size(), Display::E_TextureFilter_Point);
			else
				Display::Device()->SetTextureFilter(bloomTextures.Size(), Display::E_TextureFilter_Linear);

			const F32 fSunScale = m_bBloomSun ? g_HDRBloomSunScale : 0.0f;
			MathsStorage::Float4 vBloomCombineBloomSettings(parameters.fCombinedScale[0], parameters.fCombinedScale[1], parameters.fCombinedScale[2], parameters.fCombinedScale[3]);
			MathsStorage::Float4 vBloomCombineSunSettings(fSunScale, fSunScale, fSunScale, fSunScale);

			if(m_bBloomGamma)
			{
				pCombineShader = &m_BloomCombineGamma;
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineSunGammaBloomSettings, (F32*)&vBloomCombineBloomSettings);
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineSunGammaSunSettings, (F32*)&vBloomCombineSunSettings);
			}
			else
			{
				pCombineShader = &m_BloomCombine;
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineSunBloomSettings, (F32*)&vBloomCombineBloomSettings);
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineSunSunSettings, (F32*)&vBloomCombineSunSettings);
			}

			const Display::C_Viewport& viewport = Display::Device()->GetView()->GetViewport();
			Display::C_Viewport scaledViewport = viewport;
			scaledViewport.m_X = 0;
			scaledViewport.m_Y = 0;
			scaledViewport.m_W /= 4;
			scaledViewport.m_H /= 4;
			m_Manager.SetViewport(scaledViewport);
			DisplayIntermediate(pCombineShader, Display::E_TextureFilter_Linear);
			Resolve();
		}
	}

	return NULL;
}

}; // namespace PostProcess
