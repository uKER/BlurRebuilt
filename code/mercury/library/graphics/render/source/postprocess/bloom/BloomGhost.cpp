#include "StdAfx.h"
#include "Render/PostProcess/Bloom/BloomGhost.h"

#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psBloomCombine);
EXTERN_SHADER(psBloomCombineGamma);
EXTERN_SHADER(psBloomGhost);
EXTERN_SHADER(psBloomGhostGamma);

using namespace Display;

namespace PostProcess
{

Debug::Float g_HDRBloomGhostScale0("Graphics.Render.PostProcess.HDR.Bloom.Ghost.Scale0", -0.5f, Debug::Range(-8.0f, 8.0f));
Debug::Float g_HDRBloomGhostScale1("Graphics.Render.PostProcess.HDR.Bloom.Ghost.Scale1", -1.0f, Debug::Range(-8.0f, 8.0f));
Debug::Float g_HDRBloomGhostScale2("Graphics.Render.PostProcess.HDR.Bloom.Ghost.Scale2", -2.0f, Debug::Range(-8.0f, 8.0f));
Debug::Float g_BloomGhostScale3("Graphics.Render.PostProcess.HDR.Bloom.Ghost.Scale3", -4.0f, Debug::Range(-8.0f, 8.0f));

Debug::Bool g_HDRBloomGhost("Graphics.Render.PostProcess.HDR.Bloom.Ghost.Enable", true);

extern Debug::Bool g_HDRPointFilter;

C_BloomGhost::C_BloomGhost(C_Manager& manager) : I_Bloom(manager)
{
}

C_BloomGhost::~C_BloomGhost()
{
}

bool C_BloomGhost::Create()
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

	// bloom ghost
	if(!Display::CreatePixelShaderFromFunc("BloomGhost", 0, &m_BloomGhost, REF_SHADER(psBloomGhost)))
		return false;
	m_BloomGhostScale01 = m_BloomGhost.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vScale01");
	m_BloomGhostScale23 = m_BloomGhost.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vScale23");
	m_BloomGhostColour0 = m_BloomGhost.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vColour0");
	m_BloomGhostColour1 = m_BloomGhost.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vColour1");
	m_BloomGhostColour2 = m_BloomGhost.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_vColour2");
	m_BloomGhostColour3 = m_BloomGhost.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_5, "g_vColour3");
		
	// gamma
	if(!Display::CreatePixelShaderFromFunc("BloomGhostGamma", 0, &m_BloomGhostGamma, REF_SHADER(psBloomGhostGamma)))
		return false;
	m_BloomGhostGammaScale01 = m_BloomGhostGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vScale01");
	m_BloomGhostGammaScale23 = m_BloomGhostGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vScale23");
	m_BloomGhostGammaColour0 = m_BloomGhostGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vColour0");
	m_BloomGhostGammaColour1 = m_BloomGhostGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vColour1");
	m_BloomGhostGammaColour2 = m_BloomGhostGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_vColour2");
	m_BloomGhostGammaColour3 = m_BloomGhostGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_5, "g_vColour3");

	// mask
	if(!Display::CreateTexture2DFromFile("Graphics/Textures/Ghost_Mask.dds", &m_MaskTexture))
		return false;

	return I_Bloom::Create();
}

bool C_BloomGhost::CreateVolatile()
{
	return I_Bloom::CreateVolatile();
}

void C_BloomGhost::Destroy()
{
	Display::Device()->Destroy(&m_BloomGhost);
	Display::Device()->Destroy(&m_BloomGhostGamma);
	Display::Device()->Destroy(&m_MaskTexture);

	I_Bloom::Destroy();
}

void C_BloomGhost::DestroyVolatile()
{
	I_Bloom::DestroyVolatile();
}

const Display::C_Texture* C_BloomGhost::Display(const BloomParameters& parameters) const
{
	if(m_Manager.Enabled())
	{
		PERF_EVENT("Bloom");

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

		// Ghost
		if(g_HDRBloomGhost)
		{
			const Display::C_PixelShader* pGhostShader = 0;
			if(m_bBloomGamma)
			{
				pGhostShader = &m_BloomGhostGamma;
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostGammaScale01, (F32*)&m_Scales01);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostGammaScale23, (F32*)&m_Scales23);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostGammaColour0, (F32*)&m_Colour0);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostGammaColour1, (F32*)&m_Colour1);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostGammaColour2, (F32*)&m_Colour2);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostGammaColour3, (F32*)&m_Colour3);
			}
			else
			{
				pGhostShader = &m_BloomGhost;
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostScale01, (F32*)&m_Scales01);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostScale23, (F32*)&m_Scales23);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostColour0, (F32*)&m_Colour0);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostColour1, (F32*)&m_Colour1);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostColour2, (F32*)&m_Colour2);
				Display::Device()->SetPixelShaderParameterF(m_BloomGhostColour3, (F32*)&m_Colour3);
			}

			Display::Device()->SetTexture(0, &m_Manager.GetRenderTexture()->GetTexture());

			Display::Device()->SetTexture(1, &m_MaskTexture);
			Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
			Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
			Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

			const Display::C_Viewport& viewport = Display::Device()->GetView()->GetViewport();
			Display::C_Viewport scaledViewport = viewport;
			scaledViewport.m_X = 0;
			scaledViewport.m_Y = 0;
			scaledViewport.m_W /= 4;
			scaledViewport.m_H /= 4;
			m_Manager.SetViewport(scaledViewport);
			DisplayIntermediate(pGhostShader, Display::E_TextureFilter_Linear);
			Resolve();		
		}
	}

	return NULL;
}

void C_BloomGhost::SetScales(const Display::C_Float2& vScale0, const Display::C_Float2& vScale1, const Display::C_Float2& vScale2, const Display::C_Float2& vScale3) const
{
	m_Scales01.x = vScale0.x;
	m_Scales01.y = vScale0.y;
	m_Scales01.z = vScale1.x;
	m_Scales01.w = vScale1.y;
	m_Scales23.x = vScale2.x;
	m_Scales23.y = vScale2.y;
	m_Scales23.z = vScale3.x;
	m_Scales23.w = vScale3.y;
}

void C_BloomGhost::SetColours(const Display::C_Float4& vColour0, const Display::C_Float4& vColour1, const Display::C_Float4& vColour2, const Display::C_Float4& vColour3) const
{
	m_Colour0 = vColour0;
	m_Colour1 = vColour1;
	m_Colour2 = vColour2;
	m_Colour3 = vColour3;
}

}; // namespace PostProcess
