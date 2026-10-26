#include "StdAfx.h"
#include "Render/PostProcess/Bloom/BloomFeedbackGhost.h"

#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psBloomCombineFeedback);
EXTERN_SHADER(psBloomCombineFeedbackGamma);
EXTERN_SHADER(psBloomGhost);
EXTERN_SHADER(psBloomGhostGamma);

using namespace Display;

namespace PostProcess
{

extern Debug::Float g_HDRBloomFeedbackAdd;
extern Debug::Float g_HDRBloomFeedbackFade;
extern Debug::Float g_HDRBloomFeedbackOffset;

extern Debug::Float g_HDRBloomGhostScale0;
extern Debug::Float g_HDRBloomGhostScale1;
extern Debug::Float g_HDRBloomGhostScale2;
extern Debug::Float g_HDRBloomGhostScale3;

extern Debug::Bool g_HDRBloomFreeze;

extern Debug::Bool g_HDRPointFilter;

extern Debug::Bool g_HDRBloomGhost;

C_BloomFeedbackGhost::C_BloomFeedbackGhost(C_Manager& manager) : I_Bloom(manager)
{
	m_FeedbackClear = true;
	m_FeedbackFade = 0.90f;
}

C_BloomFeedbackGhost::~C_BloomFeedbackGhost()
{
}

bool C_BloomFeedbackGhost::Create()
{
	RESOURCE_MEMORY_ID("HDR");

	// feedback
	if(!Display::CreatePixelShaderFromFunc("BloomFeedbackCombine", 0, &m_BloomCombine, REF_SHADER(psBloomCombineFeedback)))
		return false;
	m_BloomCombineFeedbackBloomSettings = m_BloomCombine.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vBloomSettings");
	m_BloomCombineFeedbackFeedbackSettings = m_BloomCombine.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vFeedbackSettings");

	// feedback gamma
	if(!Display::CreatePixelShaderFromFunc("BloomCombineFeedbackGamma", 0, &m_BloomCombineGamma, REF_SHADER(psBloomCombineFeedbackGamma)))
		return false;
	m_BloomCombineFeedbackGammaBloomSettings = m_BloomCombineGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vBloomSettings");
	m_BloomCombineFeedbackGammaFeedbackSettings = m_BloomCombineGamma.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_3, "g_vFeedbackSettings");

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
	if(!Display::CreateTexture2DFromFile("Graphics/Textures/ghost_mask.dds", &m_MaskTexture))
		return false;

	return I_Bloom::Create();
}

bool C_BloomFeedbackGhost::CreateVolatile()
{
	RESOURCE_MEMORY_ID("HDR");

	// create a feedback texture for each posteffects viewport
	for(U32 i = 0; i < m_Manager.GetViewCount(); ++i)
	{
		C_View& view = m_Manager.GetView(i);
		const U32 iWidth = view.GetWidth() / 8;
		const U32 iHeight = view.GetHeight() / 8;
		Display::E_Format eFormat = Display::E_Format_A8R8G8B8;
		// [Blur2PC][HDR] Preserve the current bloom's range in next-frame feedback.
		if (Device()->GetBackBuffer().GetColourFormat() == Display::E_Format_A16B16G16R16F)
			eFormat = Display::E_Format_A16B16G16R16F;

		Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;
		if (!Device()->CreateRenderTexture2D(iWidth, iHeight, 1, Display::E_Usage_Colour, eFormat, E_Multisample_None, NULL, pRenderTexture))
		{
			delete pRenderTexture;
			return false;
		}

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_BloomFeedbackGhostRT");
		#endif

		m_FeedbackTextures.PushBack(pRenderTexture);
	}

	// clear textures
	Display::Device()->AcquireThreadOwnership();
	Display::Device()->BeginScene();
	for (S32 i = 0; i < m_FeedbackTextures.Size(); ++i)
	{
		const Display::C_Surface& colourSurface = m_FeedbackTextures[i]->GetSurface();
		Display::Device()->SetColourDepthStencilSurface(&colourSurface, 0);
		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = colourSurface.GetWidth();
		viewport.m_H = colourSurface.GetHeight();
		Display::Device()->SetViewport(viewport);
		Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0x00000000));
	}
	Display::Device()->EndScene();
	Display::Device()->ReleaseThreadOwnership();

	return I_Bloom::CreateVolatile();
}

void C_BloomFeedbackGhost::Destroy()
{
	Display::Device()->Destroy(&m_BloomGhost);
	Display::Device()->Destroy(&m_BloomGhostGamma);
	Display::Device()->Destroy(&m_MaskTexture);

	I_Bloom::Destroy();
}

void C_BloomFeedbackGhost::DestroyVolatile()
{
	for (S32 i = 0; i < m_FeedbackTextures.Size(); ++i)
	{
		if (m_FeedbackTextures[i]->IsCreated())
			Display::Device()->Destroy(m_FeedbackTextures[i]);
		delete m_FeedbackTextures[i];
	}
	m_FeedbackTextures.Clear();

	I_Bloom::DestroyVolatile();
}

const Display::C_Texture* C_BloomFeedbackGhost::Display(const BloomParameters& parameters) const
{
	if(m_Manager.Enabled())
	{
		PERF_EVENT("Bloom");

		Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

		Display::C_RenderTexture2D* pFeedbackTexture = m_FeedbackTextures[m_Manager.GetViewIndex()];

		// Calculate bloom textures
		RenderTexturesConst bloomTextures;
		CalculateBloomTextures(parameters, bloomTextures);

		// Combine the bloom textures into a single render target
		{
			PERF_EVENT("Combine");
			const Display::C_PixelShader* pCombineShader;

			// set bloom textures to stage 3+
			for(S32 i = 0; i < bloomTextures.Size(); ++i)
			{
				Display::Device()->SetTexture(i, &bloomTextures[i]->GetTexture());
				Display::Device()->SetTextureAddressUVW(i, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
				if(g_HDRPointFilter)
					Display::Device()->SetTextureFilter(i, Display::E_TextureFilter_Point);
				else
					Display::Device()->SetTextureFilter(i, Display::E_TextureFilter_Linear);
			}
			Display::Device()->SetTexture(bloomTextures.Size(), &pFeedbackTexture->GetTexture());
			Display::Device()->SetTextureAddressUVW(bloomTextures.Size(), Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
			if(g_HDRPointFilter)
				Display::Device()->SetTextureFilter(bloomTextures.Size(), Display::E_TextureFilter_Point);
			else
				Display::Device()->SetTextureFilter(bloomTextures.Size(), Display::E_TextureFilter_Linear);

			MathsStorage::Float4 vBloomCombineBloomSettings(parameters.fCombinedScale[0], parameters.fCombinedScale[1], parameters.fCombinedScale[2], parameters.fCombinedScale[3]);
			
			if(g_HDRBloomFreeze)
				vBloomCombineBloomSettings = MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f);
			
			MathsStorage::Float4 vBloomCombineFeedbackSettings(g_HDRBloomFeedbackAdd, g_HDRBloomFeedbackFade * m_FeedbackFade, g_HDRBloomFeedbackOffset, 0.0f);

			if(m_bBloomGamma)
			{
				pCombineShader = &m_BloomCombineGamma;
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineFeedbackGammaBloomSettings, (F32*)&vBloomCombineBloomSettings);
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineFeedbackGammaFeedbackSettings, (F32*)&vBloomCombineFeedbackSettings);
			}
			else
			{
				pCombineShader = &m_BloomCombine;
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineFeedbackBloomSettings, (F32*)&vBloomCombineBloomSettings);
				Display::Device()->SetPixelShaderParameterF(m_BloomCombineFeedbackFeedbackSettings, (F32*)&vBloomCombineFeedbackSettings);
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

		// Copy the combined bloom into the feedback texture
		Display::Device()->SetColourDepthStencilSurface(&pFeedbackTexture->GetSurface(), 0);

		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = pFeedbackTexture->GetSurface().GetWidth();
		viewport.m_H = pFeedbackTexture->GetSurface().GetHeight();
		Display::Device()->SetViewport(viewport);

		if(m_FeedbackClear)
		{
			Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0x00000000));
			pFeedbackTexture->Resolve();
			m_FeedbackClear = false;
		}
		else
		{
			Quad::TextureFilter(Display::E_TextureFilter_Linear);
			if(!g_HDRBloomFreeze)
			{
				Quad::DrawFullScreen(&m_Manager.GetRenderTexture()->GetTexture());
				pFeedbackTexture->Resolve();
			}
		}

		// Ghost
		if (g_HDRBloomGhost)
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

}; // namespace PostProcess
