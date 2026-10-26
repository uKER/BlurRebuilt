#include "stdafx.h"
#include "Effect/Sky/TexturedSkyDome.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include <display/context/Context.h>

EXTERN_SHADER(vsTexturedSkyDome);
EXTERN_SHADER(psTexturedSkyDome);
EXTERN_SHADER(psTexturedSkyDomeHDR);
EXTERN_SHADER(psTexturedSkyDomeHDRU32);
EXTERN_SHADER(psTexturedSkyDomeAlpha);
EXTERN_SHADER(psTexturedSkyDomeAlphaHDR);
EXTERN_SHADER(psTexturedSkyDomeAlphaHDRU32);

namespace Effect
{
namespace Sky
{

Debug::Float	g_debugTextureFogTop		("Graphics.Render.Sky.Textured.FogTop",		1.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugTextureFogAlpha		("Graphics.Render.Sky.Textured.FogAlpha",	1.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugTextureFogPower		("Graphics.Render.Sky.Textured.FogPower",	20.0f,	Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugTextureRotation		("Graphics.Render.Sky.Textured.Rotation",	0.0f,	Debug::Range(-1.0f, 1.0f ));
Debug::Float	g_debugTextureIntensity		("Graphics.Render.Sky.Textured.Intensity",	1.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugTextureBlackHeight	("Graphics.Render.Sky.Textured.BlackHeight",	3.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugTextureBlackSize		("Graphics.Render.Sky.Textured.BlackSize",	1.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugTextureContrast		("Graphics.Render.Sky.Textured.Contrast",	1.0f,	Debug::Range(0.0f, 4.0f));
Debug::Float	g_debugTextureSaturation	("Graphics.Render.Sky.Textured.Saturation",	1.0f,	Debug::Range(0.0f, 1.0f));

C_TexturedSkyDome::C_TexturedSkyDome() :
	m_TextureFileName("DontShip/Textures/Sunny.dds"),
	m_Rotation(0.0f),
	m_BaseRotation(0.0f),
	m_Intensity(-1.0f),
	m_FogColour(0.5f, 0.5f, 0.5f, 0.5f)
{
	SetAlpha(false);
}

C_TexturedSkyDome::~C_TexturedSkyDome()
{
	DestroyRenderer();
}

bool C_TexturedSkyDome::OnCreate()
{
	RESOURCE_MEMORY_ID( "TexturedSkyDome" );

	if (!C_SkyDomeBase::OnCreate())
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsTexturedSkyDome", 0, &m_VertexShader, REF_SHADER(vsTexturedSkyDome)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psTexturedSkyDomeHDRU32", 0, &m_PixelShaders[E_TexturedShader_Solid], REF_SHADER(psTexturedSkyDomeHDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psTexturedSkyDomeAlphaHDRU32", 0, &m_PixelShaders[E_TexturedShader_Alpha], REF_SHADER(psTexturedSkyDomeAlphaHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psTexturedSkyDomeHDR", 0, &m_PixelShaders[E_TexturedShader_Solid], REF_SHADER(psTexturedSkyDomeHDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("psTexturedSkyDomeAlphaHDR", 0, &m_PixelShaders[E_TexturedShader_Alpha], REF_SHADER(psTexturedSkyDomeAlphaHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psTexturedSkyDome", 0, &m_PixelShaders[E_TexturedShader_Solid], REF_SHADER(psTexturedSkyDome)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psTexturedSkyDomeAlpha", 0, &m_PixelShaders[E_TexturedShader_Alpha], REF_SHADER(psTexturedSkyDomeAlpha)))
			return false;
	}
	
	for (U32 shaderNo = 0; shaderNo < E_TexturedShader_Count; ++shaderNo)
	{
		m_psSkySettingsParam[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_SETTINGS,					"g_vSkySettings");
		m_psFogColourParam[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_SETTINGS_1,				"g_vFogColour");
		m_psLuminanceParam[shaderNo]	= m_PixelShaders[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");
	}

	if (Sys::GetDefaultFileSystem()->FileExists(m_TextureFileName))
	{
		Display::CreateTexture2DFromFile(m_TextureFileName, &m_Texture);

		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		if(m_Texture.IsCreated())
			Display2::Device()->InsertProtectedMemory(m_Texture.GetSize(), m_Texture.GetAddress(), "SkyDome");
		#endif
	}

	Display2::E_TextureFilter eFilter;
	const bool bBilinear = (m_Texture.GetFormat() == Display2::E_TextureFormat_A16B16G16R16F);
	if (bBilinear)
		eFilter = Display2::E_TextureFilter_Point;
	else
		eFilter = Display2::E_TextureFilter_Linear;

	m_SamplerState.SetDefaultStates();
	m_SamplerState.SetTextureFilter(eFilter);
	m_SamplerState.SetTextureAddress(Display2::E_TextureAddress_Wrap, Display2::E_TextureAddress_ClampToEdge, Display2::E_TextureAddress_ClampToEdge);

	if(m_TextureFileName.Search(".texture") == -1)
		m_Texture.SetSRGB();

	return true;
}

void C_TexturedSkyDome::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_TexturedShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShaders[shaderNo]);
	}

	#if TRACK_DISPLAY2_PROTECTED_MEMORY
	if(m_Texture.IsCreated())
		Display2::Device()->RemoveProtectedMemory(m_Texture.GetSize(), m_Texture.GetAddress());
	#endif

	Display::Device()->Destroy(&m_Texture);

	C_SkyDomeBase::OnDestroy();
}

void C_TexturedSkyDome::Bind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Bind(pContext); 
	
	const bool bAlpha = m_bAlpha;
	const bool bBilinear = (m_Texture.GetFormat() == Display2::E_TextureFormat_A16B16G16R16F);
	const U32 nType = Maths::BoolToInt(bAlpha) + 2 * Maths::BoolToInt(bBilinear);
	SetPixelShader(&m_PixelShaders[nType]);

	if (m_bAlpha)
	{
		//Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
	}
	else
	{
		pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
		pContext->SetBlendEnable(Display::E_Bool_False);
		pContext->SetBlendState(Display::E_Blend_One, Display::E_Blend_Zero, Display::E_BlendOp_Add);
		pContext->SetAlphaTestState(Display::E_Bool_False, 0);
		pContext->SetAlphaToMaskEnable(Display::E_Bool_False);
		pContext->SetCullMode(Display::E_CullMode_Default);
	}

	pContext->SetTexture(0, m_Texture.GetGPUTexture());
	Display::C_SamplerUsageFlags flags;
	flags.SetEnabled(0, 0);
	pContext->SetSamplerUsageFlags(&flags);

#if PLATFORM_PS3
	m_SamplerState.SetTextureGamma(m_Texture.GetFlags() & Display2::E_Flags_SRGB ? Display2::E_TextureGamma_R | Display2::E_TextureGamma_G | Display2::E_TextureGamma_B : 0);
#endif
	pContext->SetSamplerState(0, &m_SamplerState);

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		// get the luminance texture from the index of the view
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters(pContext->GetView()->GetUserIndex());
		pContext->SetPixelShaderParameterF(m_psLuminanceParam[nType], (F32*)&psLuminance);
	}
	else
	{
		Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		pContext->SetPixelShaderParameterF(m_psLuminanceParam[nType], (F32*)&psGain);
	}

	const F32 fRotation		= m_BaseRotation + m_Rotation + g_debugTextureRotation;
	const F32 fIntensity	= m_Intensity == -1.f ? g_debugTextureIntensity : m_Intensity;
	const F32 fBlackHeight	= g_debugTextureBlackHeight;
	const F32 fBlackSize	= g_debugTextureBlackSize;
	const Display::C_Float4 skySettings(fRotation, fIntensity, fBlackHeight, fBlackSize);
	pContext->SetVertexShaderConstantF(CV_RENDER_SETTINGS_1, (const F32*)&skySettings, 1);
	pContext->SetPixelShaderParameterF(m_psSkySettingsParam[nType], (const F32*)&skySettings);
	const Display::C_Float4 vsFogSettings(g_debugTextureFogAlpha, g_debugTextureFogTop, g_debugTextureFogPower, 0.0f);
	pContext->SetVertexShaderConstantF(CV_RENDER_SETTINGS_2, (const F32*)&vsFogSettings, 1);

	const Display::C_Float4 psFogColour(m_FogColour.R(), m_FogColour.G(), m_FogColour.B(), m_FogColour.A());
    pContext->SetPixelShaderParameterF(m_psFogColourParam[nType], (const F32*)&psFogColour);
}

void C_TexturedSkyDome::Unbind(Display::C_Context *pContext) const
{
	//if (m_bAlpha)
	//	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	C_SkyDomeBase::Unbind(pContext);
}

void C_TexturedSkyDome::ReloadTexture()
{
	RESOURCE_MEMORY_ID("TexturedSkyDome");

#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif

	#if TRACK_DISPLAY2_PROTECTED_MEMORY
	if(m_Texture.IsCreated())
		Display2::Device()->RemoveProtectedMemory(m_Texture.GetSize(), m_Texture.GetAddress());
	#endif

	Display::Device()->Destroy(&m_Texture);
	Display::CreateTexture2DFromFile(m_TextureFileName, &m_Texture);

	#if TRACK_DISPLAY2_PROTECTED_MEMORY
	if(m_Texture.IsCreated())
		Display2::Device()->InsertProtectedMemory(m_Texture.GetSize(), m_Texture.GetAddress(), "SkyDome");
	#endif

	if(m_TextureFileName.Search(".texture") == -1)
		m_Texture.SetSRGB();
}

}; // namespace Sky
}; // namespace Effect
