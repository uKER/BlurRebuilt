#include "StdAfx.h"
#include "Render/PostProcess/Bloom/Bloom.h"

#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psBloomCombine);
EXTERN_SHADER(psBloomCombineGamma);

using namespace Display;

namespace PostProcess
{

extern Debug::Bool g_HDRPointFilter;

C_Bloom::C_Bloom(C_Manager& manager) : I_Bloom(manager)
{
}

C_Bloom::~C_Bloom()
{
}

bool C_Bloom::Create()
{
	RESOURCE_MEMORY_ID("HDR");

	// bloom combine
	if(!Display::CreatePixelShaderFromFunc("BloomCombine", 0, &m_BloomCombine, REF_SHADER(psBloomCombine)))
		return false;
	m_BloomCombineBloomSettings = m_BloomCombine.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vBloomSettings");
		
	// gamma
	if(!Display::CreatePixelShaderFromFunc("BloomCombineGamma", 0, &m_BloomCombineGamma, REF_SHADER(psBloomCombineGamma)))
		return false;
	m_BloomCombineGammaBloomSettings = m_BloomCombineGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vBloomSettings");

	return I_Bloom::Create();
}

bool C_Bloom::CreateVolatile()
{
	return I_Bloom::CreateVolatile();
}

void C_Bloom::Destroy()
{
	I_Bloom::Destroy();
}

void C_Bloom::DestroyVolatile()
{
	I_Bloom::DestroyVolatile();
}

const Display::C_Texture* C_Bloom::Display(const BloomParameters& parameters) const
{
	if(m_Manager.Enabled())
	{
		PERF_EVENT("Bloom");

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

		// Calculate bloom textures
		RenderTexturesConst bloomTextures;
		CalculateBloomTextures(parameters, bloomTextures);

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

			MathsStorage::Float4 vBloomCombineBloomSettings(parameters.fCombinedScale[0], parameters.fCombinedScale[1], parameters.fCombinedScale[2], parameters.fCombinedScale[3]);
			
			if(m_bBloomGamma)
			{
				pCombineShader = &m_BloomCombineGamma;
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineGammaBloomSettings, (F32*)&vBloomCombineBloomSettings);
			}
			else
			{
				pCombineShader = &m_BloomCombine;
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineBloomSettings, (F32*)&vBloomCombineBloomSettings);
			}

			const Display::C_Viewport& viewport = m_Manager.GetViewportOrigin();
			Display::C_Viewport scaledViewport = viewport;
			scaledViewport.m_X = 0;
			scaledViewport.m_Y = 0;
			scaledViewport.m_W /= 4;
			scaledViewport.m_H /= 4;
			m_Manager.SetViewport(scaledViewport);
			DisplayIntermediate(pCombineShader, Display::E_TextureFilter_Linear);
			Resolve();
			return &(m_Manager.GetRenderTexture()->GetTexture());
		}
	}

	return NULL;
}

}; // namespace PostProcess
