#include "stdafx.h"
#include "Effect/Sky/LuminanceSkyDome.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"

EXTERN_SHADER(vsTexturedSkyDome);
EXTERN_SHADER(psLuminanceSkyDome);
EXTERN_SHADER(psLuminanceSkyDomeHDR);
EXTERN_SHADER(psLuminanceSkyDomeHDRU32);

namespace Effect
{
namespace Sky
{

Debug::Float    g_debugLuminanceFogTop		("Graphics.Render.Sky.Luminance.FogTop",		1.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float    g_debugLuminanceFogAlpha	("Graphics.Render.Sky.Luminance.FogAlpha",	1.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float    g_debugLuminanceFogPower	("Graphics.Render.Sky.Luminance.FogPower",	20.0f,	Debug::Range(0.0f, 100.0f));
Debug::Float    g_debugLuminanceRotation	("Graphics.Render.Sky.Luminance.Rotation",	0.0f,	Debug::Range(-1.0f, 1.0f ));
Debug::Float	g_debugLuminanceIntensity	("Graphics.Render.Sky.Luminance.Intensity",	1.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugLuminanceBlackHeight	("Graphics.Render.Sky.Luminance.BlackHeight",	3.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugLuminanceBlackSize	("Graphics.Render.Sky.Luminance.BlackSize",	1.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float	g_debugLuminanceContrast	("Graphics.Render.Sky.Luminance.Contrast",	1.0f,	Debug::Range(0.0f, 4.0f));
Debug::Float	g_debugLuminanceSaturation	("Graphics.Render.Sky.Luminance.Saturation",	1.0f,	Debug::Range(0.0f, 1.0f));

C_LuminanceSkyDome::C_LuminanceSkyDome() :
	m_TextureFileName("Textures/Sunny.dds"),
	m_Rotation(0.0f),
	m_BaseRotation(0.0f),
	m_Intensity(1.0f),
	m_FogColour(0.5f, 0.5f, 0.5f, 0.5f)
{
}

C_LuminanceSkyDome::~C_LuminanceSkyDome()
{
		DestroyRenderer();

}

bool C_LuminanceSkyDome::OnCreate()
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
			if (!Display::CreatePixelShaderFromFunc("psLuminanceSkyDomeHDRU32", 0, &m_PixelShader, REF_SHADER(psLuminanceSkyDomeHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psLuminanceSkyDomeHDR", 0, &m_PixelShader, REF_SHADER(psLuminanceSkyDomeHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psLuminanceSkyDome", 0, &m_PixelShader, REF_SHADER(psLuminanceSkyDome)))
			return false;
	}

	m_SkySettingsParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS,				"g_vSkySettings");
	m_FogColourParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_1,				"g_vFogColour");
	m_FogSettingsParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_2,				"g_vFogSettings");
	m_AdjustParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_3,				"g_vAdjust");
	m_SunColourParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_4,				"g_vSunColour");
	m_LuminanceParam	= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");

	if (!Display::CreateTexture2DFromFile(m_TextureFileName, &m_Texture))
		return false;

	return true;
}

void C_LuminanceSkyDome::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture);

	C_SkyDomeBase::OnDestroy();
}

void C_LuminanceSkyDome::Bind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Bind(pContext);
	
	Display::Device()->SetTexture(0, &m_Texture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

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

	const F32 fRotation		= m_BaseRotation + m_Rotation + g_debugLuminanceRotation;
	const F32 fIntensity	= m_Intensity == -1.f ? g_debugLuminanceIntensity : m_Intensity;
	const F32 fBlackHeight	= g_debugLuminanceBlackHeight;
	const F32 fBlackSize	= g_debugLuminanceBlackSize;

	const Display::C_Float4 psSkySettings(fRotation, fIntensity, fBlackHeight, fBlackSize);
    Display::Device()->SetPixelShaderParameterF(m_SkySettingsParam, (const F32*)&psSkySettings);

	const Display::C_Float4 psFogColour(m_FogColour.R(), m_FogColour.G(), m_FogColour.B(), m_FogColour.A());
    Display::Device()->SetPixelShaderParameterF(m_FogColourParam, (const F32*)&psFogColour);

	const Display::C_Float4 psFogSettings(g_debugLuminanceFogAlpha, g_debugLuminanceFogTop, g_debugLuminanceFogPower, 0.0f);
    Display::Device()->SetPixelShaderParameterF(m_FogSettingsParam, (const F32*)&psFogSettings);

	const Display::C_Float4 psAdjust(g_debugLuminanceSaturation, g_debugLuminanceSaturation, g_debugLuminanceSaturation, g_debugLuminanceContrast);
	Display::Device()->SetPixelShaderParameterF(m_AdjustParam, (const F32*)&psAdjust);

	const Display::C_Float4 psSunColour(m_SunColour.R(), m_SunColour.G(), m_SunColour.B(), m_SunColour.A());
	Display::Device()->SetPixelShaderParameterF(m_SunColourParam, (const F32*)&psSunColour);
}

void C_LuminanceSkyDome::Unbind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Unbind(pContext);
}

}; // namespace Sky
}; // namespace Effect
