#include "stdafx.h"
#include "Effect/Sky/HDRSkyDome.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"

EXTERN_SHADER(vsTexturedSkyDome);
EXTERN_SHADER(psHDRSkyDome);
EXTERN_SHADER(psHDRSkyDomeHDR);
EXTERN_SHADER(psHDRSkyDomeHDRU32);

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

C_HDRSkyDome::C_HDRSkyDome() :
	m_TextureLDRFileName("Textures/Sky_LDR.xet"),
	m_TextureHDRFileName("Textures/Sky_HDR.xet"),
	m_Rotation(0.0f),
	m_BaseRotation(0.0f),
	m_Intensity(-1.0f),
	m_FogColour(0.5f, 0.5f, 0.5f, 0.5f)
{
}

C_HDRSkyDome::~C_HDRSkyDome()
{
	DestroyRenderer();
}

bool C_HDRSkyDome::OnCreate()
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
			if (!Display::CreatePixelShaderFromFunc("psHDRSkyDomeHDRU32", 0, &m_PixelShader, REF_SHADER(psHDRSkyDomeHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psHDRSkyDomeHDR", 0, &m_PixelShader, REF_SHADER(psHDRSkyDomeHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psHDRSkyDome", 0, &m_PixelShader, REF_SHADER(psHDRSkyDome)))
			return false;
	}
	
	m_SkySettingsParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS,				"g_vSkySettings");
	m_FogColourParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_1,				"g_vFogColour");
	m_FogSettingsParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_2,				"g_vFogSettings");
	m_AdjustParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_3,				"g_vAdjust");
	m_PixelSizeParam	= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_4,				"g_vPixelSize");
	m_LuminanceParam	= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");

	Display::CreateTexture2DFromFile(m_TextureLDRFileName, &m_TextureLDR);
	Display::CreateTexture2DFromFile(m_TextureHDRFileName, &m_TextureHDR);

	return true;
}

void C_HDRSkyDome::OnDestroy()
{
	Display::Device()->Destroy(&m_TextureHDR);
	Display::Device()->Destroy(&m_TextureLDR);

	C_SkyDomeBase::OnDestroy();
}

void C_HDRSkyDome::Bind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Bind(pContext);
	
	Display::Device()->SetTexture(0, &m_TextureLDR);
	Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point);
	
	Display::Device()->SetTexture(1, &m_TextureHDR);
	Display::Device()->SetTextureAddressUVW(1, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilterMagMinMip(1, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point);

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

	const F32 fRotation		= m_BaseRotation + m_Rotation + g_debugTextureRotation;
	const F32 fIntensity	= m_Intensity == -1.f ? g_debugTextureIntensity : m_Intensity;
	const F32 fBlackHeight	= g_debugTextureBlackHeight;
	const F32 fBlackSize	= g_debugTextureBlackSize;
	const Display::C_Float4 psSkySettings(fRotation, fIntensity, fBlackHeight, fBlackSize);
    Display::Device()->SetPixelShaderParameterF(m_SkySettingsParam, (const F32*)&psSkySettings);

	const Display::C_Float4 psFogColour(m_FogColour.R(), m_FogColour.G(), m_FogColour.B(), m_FogColour.A());
    Display::Device()->SetPixelShaderParameterF(m_FogColourParam, (const F32*)&psFogColour);

	const Display::C_Float4 psFogSettings(g_debugTextureFogAlpha, g_debugTextureFogTop, g_debugTextureFogPower, 0.0f);
    Display::Device()->SetPixelShaderParameterF(m_FogSettingsParam, (const F32*)&psFogSettings);

	const Display::C_Float4 psAdjust(g_debugTextureSaturation, g_debugTextureSaturation, g_debugTextureSaturation, g_debugTextureContrast);
	Display::Device()->SetPixelShaderParameterF(m_AdjustParam, (const F32*)&psAdjust);

	const Display::C_Float4 psPixelSize((F32) m_TextureLDR.GetWidth(), (F32) m_TextureLDR.GetHeight(), 1.0f / (F32) m_TextureLDR.GetWidth(), 1.0f / (F32) m_TextureLDR.GetHeight());
	Display::Device()->SetPixelShaderParameterF(m_PixelSizeParam, (const F32*)&psPixelSize);
}

void C_HDRSkyDome::Unbind(Display::C_Context *pContext) const
{
	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMagMinMip(1, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);

	C_SkyDomeBase::Unbind(pContext);
}

void C_HDRSkyDome::ReloadTextures()
{
#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif

	Display::Device()->Destroy(&m_TextureLDR);
	Display::Device()->Destroy(&m_TextureHDR);

	Display::CreateTexture2DFromFile(m_TextureLDRFileName, &m_TextureLDR);
	Display::CreateTexture2DFromFile(m_TextureHDRFileName, &m_TextureHDR);
}

}; // namespace Sky
}; // namespace Effect
