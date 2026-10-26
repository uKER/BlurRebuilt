#include "StdAfx.h"
#include "Render/PostProcess/Colour/Sepia.h"

#include "Render/PostProcess/Manager.h"

EXTERN_SHADER(psSepia);

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugSepia("Graphics.Render.PostProcess.Sepia.Enable", true);

bool C_Sepia::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("Sepia", 0, &m_Shader, REF_SHADER(psSepia));
	SetShader(&m_Shader);

	return true;
}

void C_Sepia::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}

void C_Sepia::Display() const
{
	if(g_debugSepia && m_Manager.Enabled())
	{
		PERF_EVENT("Sepia");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		
		// display the effect
		DisplayFinal();
	}
}



}; // namespace PostProcess
