#include "stdafx.h"
#include "Effect/Fog/Fog.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psVolumetricFog);
EXTERN_SHADER(psVolumetricFogHDR);
EXTERN_SHADER(psVolumetricFogHDRU32);

namespace Effect
{
namespace Fog
{

Debug::Colour	g_debugColour("Render.Fog.Colour", 0.25f, 0.25f, 0.25f, 1.0f);
Debug::Float	g_debugHeightFalloff("Render.Fog.HeightFalloff", 1.0f, Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugGlobalDensity("Render.Fog.GlobalDensity", 0.1f, Debug::Range(0.0f, 8.0f));

C_Fog::C_Fog() : Display::I_Renderer("Fog")
{
}

C_Fog::~C_Fog()
{
	DestroyRenderer();
}

bool C_Fog::OnCreate()
{
	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psVolumetricFogHDRU32", 0, &m_PixelShader, REF_SHADER(psVolumetricFogHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psVolumetricFogHDR", 0, &m_PixelShader, REF_SHADER(psVolumetricFogHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psVolumetricFog", 0, &m_PixelShader, REF_SHADER(psVolumetricFog)))
			return false;
	}

	m_LuminanceParam		= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
	m_DeviceToWorldParam	= m_PixelShader.GetParameterArray(CP_RENDER_SETTINGS, "g_mDeviceToWorld", 4);
	m_ViewPositionParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vViewPosition");
	m_FogSettingsParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_5, "g_vFogSettings");
	m_FogColourParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_6, "g_vFogColour");
	m_DepthScaleParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_7, "g_vDepthScale");

	return true;
}

bool C_Fog::OnCreateVolatile()
{
	return true;
}

void C_Fog::OnDestroy()
{
	Display::Device()->Destroy(&m_PixelShader);
}

void C_Fog::OnDestroyVolatile()
{
}

void C_Fog::Render()
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

	const F32 fGlobalDensity = g_debugGlobalDensity;
	const F32 fHeightFalloff = g_debugHeightFalloff;
	const F32 fHeightDensity = expf(-g_debugHeightFalloff * viewSettings3D.GetViewPosition().y);
	const MathsStorage::Float4 depthScaleParam = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	const MathsStorage::Float4 fogSettingsParam = MathsStorage::Float4(fHeightDensity, fHeightFalloff, fGlobalDensity, 0.0f);
	const MathsStorage::Float4 gainParam = Render::C_RenderState::Instance().ShaderHDR() ? Render::C_RenderState::Instance().HDR()->GetShaderParameters() : Render::C_RenderState::Instance().GetGainShaderParameters();

	Display::Device()->SetPixelShaderParameterArrayF(m_DeviceToWorldParam, (F32*)&mDeviceToWorld);
	Display::Device()->SetPixelShaderParameterF(m_ViewPositionParam, (F32*)&viewSettings3D.GetViewPosition());
	Display::Device()->SetPixelShaderParameterF(m_FogSettingsParam, (F32*)&fogSettingsParam);
	Display::Device()->SetPixelShaderParameterF(m_FogColourParam, (F32*)&g_debugColour.Value());
	Display::Device()->SetPixelShaderParameterF(m_DepthScaleParam, (F32*)&depthScaleParam);
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
