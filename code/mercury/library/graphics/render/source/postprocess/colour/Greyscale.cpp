#include "StdAfx.h"
#include "Render/PostProcess/Colour/Greyscale.h"

#include "Render/PostProcess/Manager.h"

EXTERN_SHADER(psGreyscale);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/PostProcess/Colour/PS3/psGreyscale.shader"
#endif 
#endif

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugGreyscale("Graphics.Render.PostProcess.Greyscale.Enable", true);

bool C_Greyscale::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("Greyscale", 0, &m_Shader, REF_SHADER(psGreyscale));
	SetShader(&m_Shader);
	
	return true;
}

void C_Greyscale::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}

void C_Greyscale::Display() const
{
	if(g_debugGreyscale && m_Manager.Enabled())
	{
		PERF_EVENT("Greyscale");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		
		// display the effect
		DisplayFinal();
	}
}



}; // namespace PostProcess
