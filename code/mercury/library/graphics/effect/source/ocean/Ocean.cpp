#include "stdafx.h"
#include "Effect/Ocean/Ocean.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"

EXTERN_SHADER(vsOcean);
EXTERN_SHADER(psOcean);
EXTERN_SHADER(psOceanHDR);
EXTERN_SHADER(psOceanHDRU32);

namespace Effect
{
namespace Ocean
{

Debug::Bool		g_debugWireframe("Graphics.Effect.Ocean.Wireframe", false);
Debug::Float	g_debugHeightMapSize("Graphics.Effect.Ocean.HeightMapSize", 32.0f, Debug::Range(0.0f, 128.0f));
Debug::Float	g_debugChoppiness("Graphics.Effect.Ocean.Choppiness", 0.00f, Debug::Range(0.00f, 1.00f));
Debug::Float	g_debugTimeScale("Graphics.Effect.Ocean.TimeScale", 0.0025f, Debug::Range(0.00f, 2.00f));

C_Ocean::C_Ocean() : Display::I_Renderer("Ocean")
{
	m_fHeight = 0.0f;
}

C_Ocean::~C_Ocean()
{
	DestroyRenderer();
}

bool C_Ocean::OnCreate()
{
	if (!m_GeometryPolicy.Create())
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsOcean", 0, &m_VertexShader, REF_SHADER(vsOcean)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("Shaders/psOceanHDRU32", 0, &m_PixelShader, REF_SHADER(psOceanHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("Shaders/psOceanHDR", 0, &m_PixelShader, REF_SHADER(psOceanHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shaders/psOcean", 0, &m_PixelShader, REF_SHADER(psOcean)))
			return false;
	}

	m_LuminanceParam = m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");

	return true;
}

void C_Ocean::OnDestroy()
{
	Display::Device()->Destroy(&m_PixelShader);
	Display::Device()->Destroy(&m_VertexShader);

	m_GeometryPolicy.Destroy();
}

void C_Ocean::Update(const F32 fTimeElapsed, const F32 fTimeStep)
{
	m_fTimeElapsed = fTimeElapsed;
}

void C_Ocean::PreRender()
{
	m_WavesPolicy.Render(m_fTimeElapsed * g_debugTimeScale);
}

void C_Ocean::Render()
{
	const U32 hdrNo = Render::C_RenderState::Instance().ShaderHDR() ? 1 : 0;

	Display::C_Texture2D * pHeightTexture = const_cast<Display::C_Texture2D *>(m_WavesPolicy.GetHeightMap());
	Display::C_Texture2D * pGradientTexture = const_cast<Display::C_Texture2D *>(m_WavesPolicy.GetGradientMap());


	Display::Device()->SetTexture(0, m_WavesPolicy.GetNormalMap());
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);

	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Device()->SetVertexShaderConstantF(CV_EFFECT_OCEAN_HEIGHTMAP_SETTINGS, Display::C_VSConstant(1.0f / g_debugHeightMapSize, 1.0f, g_debugChoppiness), 1);

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

	if (g_debugWireframe)
		Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_None);

	m_GeometryPolicy.Render();

	if (g_debugWireframe)
		Display::Device()->SetFillMode(Display::E_FillMode_Solid);
}

}; // namespace Ocean
}; // namespace Effect
