#include "StdAfx.h"
#include "Render/PostProcess/Misc/EdgeAntiAliasing.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psEdgeAntiAliasingDepth);
EXTERN_SHADER(psEdgeAntiAliasingDepthNormal);

using namespace Display;

namespace PostProcess
{

Debug::Bool		g_debugEdgeAntiAliasing				("Graphics.Render.PostProcess.EdgeAntiAliasing.Enable",			true);
Debug::Float	g_debugEdgeAntiAliasingWeight		("Graphics.Render.PostProcess.EdgeAntiAliasing.Weight",			1.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugEdgeAntiAliasingDepthFactor	("Graphics.Render.PostProcess.EdgeAntiAliasing.DepthFactor",	25.0f,	Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugEdgeAntiAliasingNormalFactor	("Graphics.Render.PostProcess.EdgeAntiAliasing.NormalFactor",	0.4f,	Debug::Range(0.0f, 1.0f));

bool C_EdgeAntiAliasing::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if(!Display::CreatePixelShaderFromFunc("Shaders/psEdgeAntiAliasingDepth", 0, &m_PixelShader[E_EdgeAntiAliasingShader_Depth], REF_SHADER(psEdgeAntiAliasingDepth)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psEdgeAntiAliasingDepthNormal", 0, &m_PixelShader[E_EdgeAntiAliasingShader_DepthNormal], REF_SHADER(psEdgeAntiAliasingDepthNormal)))
		return false;

	for(S32 i = 0; i < E_EdgeAntiAliasingShader_Count; ++i)
	{
		m_PixelSizeParameter[i] = m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vPixelSize");
		m_SettingsParameter[i] = m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vSettings");
		m_DepthParameter[i] = m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vDepth");
	}

	SetShader(&m_PixelShader[E_EdgeAntiAliasingShader_Depth]);

	return true;
}

void C_EdgeAntiAliasing::Destroy()
{
	Display::Device()->Destroy(&m_PixelShader[E_EdgeAntiAliasingShader_Depth]);
	Display::Device()->Destroy(&m_PixelShader[E_EdgeAntiAliasingShader_DepthNormal]);
}

void C_EdgeAntiAliasing::Display() const
{
	if(g_debugEdgeAntiAliasing && m_Manager.Enabled())
	{
		PERF_EVENT("EdgeAntiAliasing");
		
		const E_EdgeAntiAliasingShader eShader = m_pNormalTexture ? E_EdgeAntiAliasingShader_DepthNormal : E_EdgeAntiAliasingShader_Depth;

		// set the shader
		SetShader(&m_PixelShader[eShader]);

		// get the current contents of render target and set as a texture
		SetViewport();

		SetTextureExpanded(0, const_cast<Display::C_Texture2D*>(m_Manager.GetChainTexture()));
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

		Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);

		Display::Device()->SetTexture(1, m_pNormalTexture);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);

		const Display::C_Float4 vPixelSize(1.0f / m_Manager.GetChainTexture()->GetWidth(), 1.0f / m_Manager.GetChainTexture()->GetHeight(), 0.0f, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_PixelSizeParameter[eShader], (F32*)&vPixelSize);

		const Display::C_Float4 vSettings(g_debugEdgeAntiAliasingDepthFactor, g_debugEdgeAntiAliasingNormalFactor, 0.5f * g_debugEdgeAntiAliasingWeight, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[eShader], (F32*)&vSettings);

		const Display::C_Float4 vDepth = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
		Display::Device()->SetPixelShaderParameterF(m_DepthParameter[eShader], (F32*)&vDepth);

		// display the effect
		DisplayFinal();
	}
}

}; // namespace PostProcess
