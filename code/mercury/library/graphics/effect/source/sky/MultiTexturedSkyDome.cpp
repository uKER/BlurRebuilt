#include "stdafx.h"
#include "Effect/Sky/MultiTexturedSkyDome.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"

EXTERN_SHADER(vsTexturedSkyDome);
EXTERN_SHADER(psMultiTexturedSkyDome);
EXTERN_SHADER(psMultiTexturedSkyDomeHDR);
EXTERN_SHADER(psMultiTexturedSkyDomeHDRU32);

namespace Effect
{
namespace Sky
{

extern Debug::Float	g_debugTextureFogTop;
extern Debug::Float	g_debugTextureFogAlpha;
extern Debug::Float	g_debugTextureFogPower;
extern Debug::Float	g_debugTextureRotation;
extern Debug::Float	g_debugTextureIntensity;
extern Debug::Float	g_debugTextureBlackHeight;
extern Debug::Float	g_debugTextureBlackSize;
extern Debug::Float	g_debugTextureContrast;
extern Debug::Float	g_debugTextureSaturation;

Debug::Float	g_debugMultiTextureFgIntensity	("Graphics.Render.Sky.MultiTextured.FgIntensity",	1.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugMultiTextureBgIntensity	("Graphics.Render.Sky.MultiTextured.BgIntensity",	1.0f,	Debug::Range(0.0f, 10.0f));

C_MultiTexturedSkyDome::C_MultiTexturedSkyDome() :
	m_BgTextureFileName("Textures/SkyBg.dds"),
	m_FgTextureFileName("Textures/SkyFg.dds"),
	m_BaseRotation(0.0f),
	m_BgRotation(0.0f),
	m_FgRotation(0.0f),
	m_BgIntensity(-1.0f),
	m_FgIntensity(-1.0f),
	m_FogColour(0.5f, 0.5f, 0.5f, 0.5f)
{
}

C_MultiTexturedSkyDome::~C_MultiTexturedSkyDome()
{
		DestroyRenderer();

}

bool C_MultiTexturedSkyDome::OnCreate()
{
	if (!C_SkyDomeBase::OnCreate())
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsBaseSkyDome", 0, &m_VertexShader, REF_SHADER(vsTexturedSkyDome)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psMultiTexturedSkyDomeHDRU32", 0, &m_PixelShader, REF_SHADER(psMultiTexturedSkyDomeHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psMultiTexturedSkyDomeHDR", 0, &m_PixelShader, REF_SHADER(psMultiTexturedSkyDomeHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psMultiTexturedSkyDome", 0, &m_PixelShader, REF_SHADER(psMultiTexturedSkyDome)))
			return false;
	}
	
	m_SkySettingsParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS,				"g_vSkySettings");
	m_FogColourParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_1,				"g_vFogColour");
	m_FogSettingsParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_2,				"g_vFogSettings");
	m_AdjustParam			= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_3,				"g_vAdjust");
	m_BlackSettingsParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_4,				"g_vBlackSettings");
	m_LuminanceParam		= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");

	Display::CreateTexture2DFromFile(m_BgTextureFileName, &m_BgTexture);
	Display::CreateTexture2DFromFile(m_FgTextureFileName, &m_FgTexture);

	return true;
}

void C_MultiTexturedSkyDome::OnDestroy()
{
	Display::Device()->Destroy(&m_BgTexture);
	Display::Device()->Destroy(&m_FgTexture);

	C_SkyDomeBase::OnDestroy();
}

void C_MultiTexturedSkyDome::Bind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Bind(pContext);
	
	if (m_BgTexture.IsCreated())
	{
		Display::Device()->SetTexture(0, &m_BgTexture);
		Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
	}
	if (m_FgTexture.IsCreated())
	{
		Display::Device()->SetTexture(1, &m_FgTexture);
		Display::Device()->SetTextureAddressUVW(1, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilterMagMinMip(1, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
	}

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&psLuminance);
		
		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}
	else
	{
		Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&psGain);
	}

	const F32 fBgRotation	= m_BaseRotation + m_BgRotation + g_debugTextureRotation;
	const F32 fBgIntensity	= m_BgIntensity == -1.f ? g_debugTextureIntensity * g_debugMultiTextureBgIntensity : m_BgIntensity;
	const F32 fFgRotation	= m_BaseRotation + m_FgRotation + g_debugTextureRotation;
	const F32 fFgIntensity	= m_FgIntensity == -1.f ? g_debugTextureIntensity * g_debugMultiTextureFgIntensity : m_FgIntensity;

	const Display::C_Float4 psSkySettings(fBgRotation, fBgIntensity, fFgRotation, fFgIntensity);
    Display::Device()->SetPixelShaderParameterF(m_SkySettingsParam, (const F32*)&psSkySettings);

	const Display::C_Float4 psFogColour(m_FogColour.R(), m_FogColour.G(), m_FogColour.B(), m_FogColour.A());
    Display::Device()->SetPixelShaderParameterF(m_FogColourParam, (const F32*)&psFogColour);

	const Display::C_Float4 psFogSettings(g_debugTextureFogAlpha, g_debugTextureFogTop, g_debugTextureFogPower, 0.0f);
    Display::Device()->SetPixelShaderParameterF(m_FogSettingsParam, (const F32*)&psFogSettings);

	const Display::C_Float4 psAdjust(g_debugTextureSaturation, g_debugTextureSaturation, g_debugTextureSaturation, g_debugTextureContrast);
	Display::Device()->SetPixelShaderParameterF(m_AdjustParam, (const F32*)&psAdjust);

	const Display::C_Float4	psBlackSettings(g_debugTextureBlackHeight, g_debugTextureBlackSize, 0.0f, 0.0f);
    Display::Device()->SetPixelShaderParameterF(m_BlackSettingsParam, (const F32*)&psBlackSettings);
}

void C_MultiTexturedSkyDome::Unbind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Unbind(pContext);
}

void C_MultiTexturedSkyDome::ReloadTextures()
{
#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif

	Display::Device()->Destroy(&m_BgTexture);
	Display::Device()->Destroy(&m_FgTexture);

	Display::CreateTexture2DFromFile(m_BgTextureFileName, &m_BgTexture);
	Display::CreateTexture2DFromFile(m_FgTextureFileName, &m_FgTexture);
}

}; // namespace Sky
}; // namespace Effect
