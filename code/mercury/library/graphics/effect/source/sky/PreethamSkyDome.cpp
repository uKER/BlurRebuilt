#include "stdafx.h"
#include "Effect/Sky/PreethamSkyDome.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Helper.h"

EXTERN_SHADER(vsPreethamSkyDome);
EXTERN_SHADER(psPreethamSkyDome);
EXTERN_SHADER(psPreethamSkyDomeHDR);
EXTERN_SHADER(psPreethamSkyDomeHDRU32);

namespace Effect
{
namespace Sky
{

Debug::Float	g_debugPreethamTurbidity	("Graphics.Render.Sky.Preetham.Turbidity",		1.00f,	Debug::Range(0.0f, 30.0f));
Debug::Float	g_debugPreethamIntensity	("Graphics.Render.Sky.Preetham.Intensity",		1.00f,	Debug::Range(0.0f, 10.0f));

C_PreethamSkyDome::C_PreethamSkyDome() :
	m_SunHeading(0.0f),
	m_SunElevation(0.0f)
{
}

C_PreethamSkyDome::~C_PreethamSkyDome()
{
		DestroyRenderer();

}

bool C_PreethamSkyDome::OnCreate()
{
	if (!C_SkyDomeBase::OnCreate())
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPreethamSkyDome", 0, &m_VertexShader, REF_SHADER(vsPreethamSkyDome)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psPreethamSkyDomeHDRU32", 0, &m_PixelShader, REF_SHADER(psPreethamSkyDomeHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psPreethamSkyDomeHDR", 0, &m_PixelShader, REF_SHADER(psPreethamSkyDomeHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psPreethamSkyDome", 0, &m_PixelShader, REF_SHADER(psPreethamSkyDome)))
			return false;
	}

	m_SettingsParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS,				"g_vSettings");
	m_LuminanceParam	= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");

	return true;
}

void C_PreethamSkyDome::OnDestroy()
{
	C_SkyDomeBase::OnDestroy();
}

void C_PreethamSkyDome::Bind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Bind(pContext);

	const F32 T = g_debugPreethamTurbidity;

	Display::C_Float4 vsA(-0.0193f * T - 0.2592f, -0.0167f * T - 0.2608f, +0.1787f * T - 1.4630f, 0.0f);
	Display::C_Float4 vsB(-0.0665f * T + 0.0008f, -0.0950f * T + 0.0092f, -0.3554f * T + 0.4275f, 0.0f);
	Display::C_Float4 vsC(-0.0004f * T + 0.2125f, -0.0079f * T + 0.2102f, -0.2270f * T + 5.3251f, 0.0f);
	Display::C_Float4 vsD(-0.0641f * T - 0.8989f, -0.0441f * T - 1.6537f, +0.1206f * T - 2.5771f, 0.0f);
	Display::C_Float4 vsE(-0.0033f * T + 0.0452f, -0.0109f * T + 0.0529f, -0.0670f * T + 0.3703f, 0.0f);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_1, (F32 *) &vsA, 1);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_2, (F32 *) &vsB, 1);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_3, (F32 *) &vsC, 1);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_4, (F32 *) &vsD, 1);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_5, (F32 *) &vsE, 1);

	const F32 fTheta = m_SunElevation * Maths::Pi / 180.0f;
	const F32 fCosH = cosf(m_SunHeading * Maths::Pi / 180.0f);
	const F32 fCosE = cosf(fTheta);
	const F32 fSinH = sinf(m_SunHeading * Maths::Pi / 180.0f);
	const F32 fSinE = sinf(fTheta);

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_6, Display::C_VSConstant(fSinH * fSinE, fCosE, fCosH * fSinE), 1);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_7, Display::C_VSConstant(fTheta, fCosE * fCosE), 1);

	const F32 T2 = T * T;
	const F32 t2 = fTheta * fTheta;
	const F32 t3 = t2 * fTheta;
	const F32 fChi = (4.0f / 9.0f - T / 120.0f) * (Maths::Pi - 2.0f * fTheta);

	Display::C_Float4 vsZenithColour;
	vsZenithColour.x =
		(T2 * +0.00166f + T * -0.02903f + 0.11693f) * t3 +
		(T2 * -0.00375f + T * +0.06377f - 0.21196f) * t2 +
		(T2 * +0.00209f + T * -0.03203f + 0.06052f) * fTheta +
		(T2 * +0.00000f + T * +0.00394f + 0.25886f);
	vsZenithColour.y =
		(T2 * +0.00275f + T * -0.04214f + 0.15346f) * t3 +
		(T2 * -0.00610f + T * +0.08970f - 0.26756f) * t2 +
		(T2 * +0.00317f + T * -0.04153f + 0.06670f) * fTheta +
		(T2 * +0.00000f + T * +0.00516f + 0.26688f);
	vsZenithColour.z = Maths::Abs((4.0453f * T - 4.9710f) * tanf(fChi) - 0.2155f * T + 2.4192f);

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_8, (F32*)&vsZenithColour, 1);

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		const Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (const F32*)&psLuminance);

		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}
	else
	{
		const Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (const F32*)&psGain);
	}

	const Display::C_Float4 psSettings(g_debugPreethamIntensity, 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParam, (const F32*)&psSettings);
}

void C_PreethamSkyDome::Unbind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Unbind(pContext);
}

} // namespace Sky
} // namespace Effect
