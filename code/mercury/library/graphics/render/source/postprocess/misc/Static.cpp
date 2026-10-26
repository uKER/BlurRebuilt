#include "StdAfx.h"
#include "Render/PostProcess/Misc/Static.h"

#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psStatic);

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugStatic("Graphics.Render.PostProcess.Static.Enable", true);
Debug::Float g_debugStaticAmount("Graphics.Render.PostProcess.Static.Amount", 0.0f, Debug::Range(0.f, 1.0f), Debug::Step(0.1f));

bool C_Static::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("Static", 0, &m_Shader, REF_SHADER(psStatic));
	SetShader(&m_Shader);
	
	return true;
}

void C_Static::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}

void C_Static::Display() const
{
	if(g_debugStatic && m_Manager.Enabled())
	{
		PERF_EVENT("Static");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());

		MathsStorage::Float4 vAmount(g_debugStaticAmount, 0.0f, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_0, (F32*)&vAmount, 1);

		// display the effect
		DisplayFinal();
	}
}



}; // namespace PostProcess
