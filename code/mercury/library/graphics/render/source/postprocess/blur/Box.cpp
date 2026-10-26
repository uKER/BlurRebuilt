#include "StdAfx.h"
#include "Render/PostProcess/Blur/Box.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugBoxBlur("Graphics.Render.PostProcess.BoxBlur.Enable", true);

bool C_BoxBlur::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if(!m_pShader)
		SetShader(m_Manager.GetShader(E_Shader_WeightedFilter15));

	return true;
}

void C_BoxBlur::Destroy()
{
}

void C_BoxBlur::Display() const
{
	if(m_Manager.Enabled() && g_debugBoxBlur)
	{
		PERF_EVENT("PostProcess.GaussianBlur");

		// get the current contents of render target and set as a texture
		SetViewport();
		

		const Display::C_Viewport& viewport = m_Manager.GetViewport();

		MathsStorage::Float4 vSamples[15];

		// blur in X
		GetBoxSamples<15>(viewport.GetW(), 0, vSamples);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_0, (F32*)vSamples, 15);
		Display::Device()->SetPixelShaderParameters(m_pShader);
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		DisplayIntermediate();
		
		// blur in Y
		Resolve();
		GetBoxSamples<15>(0, viewport.GetH(), vSamples);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_0, (F32*)vSamples, 15);
		Display::Device()->SetPixelShaderParameters(m_pShader);
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		DisplayFinal();
	}
}



}; // namespace PostProcess
