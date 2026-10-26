#include "stdafx.h"
#include "Effect/Fog/AtmosphericScatter.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psAtmosphericScatter);
EXTERN_SHADER(psAtmosphericScatterHDR);
EXTERN_SHADER(psAtmosphericScatterHDRU32);

namespace Effect
{
namespace Fog
{

Debug::Colour	g_debugAtmosphericColour		("Render.Fog.Colour",						0.25f, 0.25f, 0.25f, 1.0f);
Debug::Float	g_debugAtmosphericHeightFalloff	("Render.Fog.HeightFalloff",				1.0f,		Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugAtmosphericGlobalDensity	("Render.Fog.GlobalDensity",				0.1f,		Debug::Range(0.0f, 8.0f));

Debug::Float	g_debugAtmosphericSunIntensity	("Render.Fog.Atmospheric.SunIntensity",		20.0f,		Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugAtmosphericMieCoeff		("Render.Fog.Atmospheric.MieCoeff",			0.0010f,	Debug::Range(0.0f, 0.01f));
Debug::Float	g_debugAtmosphericMieG			("Render.Fog.Atmospheric.MieG",				-0.99f,		Debug::Range(-2.0f, 2.0f));
Debug::Float	g_debugAtmosphericRayleighCoeff	("Render.Fog.Atmospheric.RayleighCoeff",	0.0025f,	Debug::Range(0.0f, 0.01f));

C_AtmosphericScatter::C_AtmosphericScatter() : Display::I_Renderer("AtmosphericScatter")
{
	m_vSunDirection = Display::C_Float4(0.0f, 1.0f, 0.0f, 0.0f);
}

C_AtmosphericScatter::~C_AtmosphericScatter()
{
	DestroyRenderer();
}

bool C_AtmosphericScatter::OnCreate()
{
	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psAtmosphericScatterHDRU32", 0, &m_PixelShader, REF_SHADER(psAtmosphericScatterHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psAtmosphericScatterHDR", 0, &m_PixelShader, REF_SHADER(psAtmosphericScatterHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psAtmosphericScatter", 0, &m_PixelShader, REF_SHADER(psAtmosphericScatter)))
			return false;
	}

	m_LuminanceParam		= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
	m_DeviceToWorldParam	= m_PixelShader.GetParameterArray(CP_RENDER_SETTINGS, "g_mDeviceToWorld", 4);
	m_ViewPositionParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vViewPosition");
	m_FogSettingsParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_5, "g_vFogSettings");
	m_FogColourParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_6, "g_vFogColour");
	m_DepthScaleParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_7, "g_vDepthScale");
	m_SunDirectionParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_8, "g_vSunDirection");
	m_SunIntensityParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_9, "g_vSunIntensity");
	m_MieGParam				= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_10, "g_vMieG");

	return true;
}

bool C_AtmosphericScatter::OnCreateVolatile()
{
	return true;
}

void C_AtmosphericScatter::OnDestroy()
{
	Display::Device()->Destroy(&m_PixelShader);
}

void C_AtmosphericScatter::OnDestroyVolatile()
{
}

void C_AtmosphericScatter::Render()
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	// set pixel shader parameters
	Maths::HMat44 mTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = +2.0f;
		mTemp._22 = -2.0f;
		mTemp._42 = +1.0f;
		mTemp._41 = -1.0f;
		mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}

	Display::C_Matrix mDeviceToWorld;
	Maths::Store(mTexture * viewSettings3D.GetWorldToDevice().Inverse(), &mDeviceToWorld);

	const F32 fGlobalDensity = g_debugAtmosphericGlobalDensity;
	const F32 fHeightFalloff = g_debugAtmosphericHeightFalloff;
	const F32 fHeightDensity = expf(-g_debugAtmosphericHeightFalloff * viewSettings3D.GetViewPosition().y);
	const MathsStorage::Float4 depthScaleParam = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	const MathsStorage::Float4 fogSettingsParam = MathsStorage::Float4(fHeightDensity, fHeightFalloff, fGlobalDensity, 0.0f);
	const MathsStorage::Float4 gainParam = Render::C_RenderState::Instance().ShaderHDR() ? Render::C_RenderState::Instance().HDR()->GetShaderParameters() : Render::C_RenderState::Instance().GetGainShaderParameters();

	const F32 fRayleighSunIntensity	= g_debugAtmosphericSunIntensity * g_debugAtmosphericRayleighCoeff;
	const F32 fMieSunIntensity		= g_debugAtmosphericSunIntensity * g_debugAtmosphericMieCoeff;
	const Display::C_Float4 sunIntensityParam(fRayleighSunIntensity / powf(0.650f, 4.0f), fRayleighSunIntensity / powf(0.570f, 4.0f), fRayleighSunIntensity / powf(0.475f, 4.0f), fMieSunIntensity);

	const F32 fG = g_debugAtmosphericMieG;
	const F32 fG2 = fG * fG;
	const Display::C_Float4 mieGParam(1.5f * ((1.0f - fG2) / (2.0f + fG2)), 1.0f + fG2, 2.0f * fG, 0.0f);

	Display::Device()->SetPixelShaderParameterArrayF(m_DeviceToWorldParam, (F32*)&mDeviceToWorld);
	Display::Device()->SetPixelShaderParameterF(m_ViewPositionParam, (F32*)&viewSettings3D.GetViewPosition());
	Display::Device()->SetPixelShaderParameterF(m_FogSettingsParam, (F32*)&fogSettingsParam);
	Display::Device()->SetPixelShaderParameterF(m_FogColourParam, (F32*)&g_debugAtmosphericColour.Value());
	Display::Device()->SetPixelShaderParameterF(m_DepthScaleParam, (F32*)&depthScaleParam);
	Display::Device()->SetPixelShaderParameterF(m_SunDirectionParam, (F32*)&m_vSunDirection);
	Display::Device()->SetPixelShaderParameterF(m_SunIntensityParam, (F32*)&sunIntensityParam);
	Display::Device()->SetPixelShaderParameterF(m_MieGParam, (F32*)&mieGParam);
	Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&gainParam);

	// hdr states
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}

	// draw
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTexture(0, Render::GetGlobalDepthStencilBuffer());

	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Red | Display::E_ColourWriteEnable_Green | Display::E_ColourWriteEnable_Blue);
	
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Quad::DrawFullScreen();

	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
}

}; // namespace Fog
}; // namespace Effect
