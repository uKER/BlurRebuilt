#include "StdAfx.h"
#include "Render/PostProcess/Colour/Tint.h"
#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psTint);

using namespace Display;

namespace PostProcess
{

bool C_Tint::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("Tint", 0, &m_Shader, REF_SHADER(psTint));
	SetShader(&m_Shader);

	m_Parameter = m_Shader.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vColour");
	
	return true;
}

void C_Tint::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}

void C_Tint::Display() const
{
	if(m_Manager.Enabled())
	{
		PERF_EVENT("Tint");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		Display::Device()->SetPixelShaderParameterF(m_Parameter, (F32*)&m_vColour);
		
		// display the effect
		DisplayFinal();
	}
}



}; // namespace PostProcess
