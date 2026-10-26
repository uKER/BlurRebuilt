#include "StdAfx.h"
#include "Render/PostProcess/Scale/GaussianScale.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugGaussianScale("Graphics.Render.PostProcess.GaussianScale.Enable", true);

bool C_GaussianScale::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if(!m_pShader)
	{
		SetShader();
		SetParameters();
	}

	return true;
}

void C_GaussianScale::Destroy()
{
}

void C_GaussianScale::SetShader() const
{
	if(m_bGamma)
		I_PostProcess::SetShader(m_Manager.GetShader(E_Shader_WeightedFilter13Gamma));
	else
		I_PostProcess::SetShader(m_Manager.GetShader(E_Shader_WeightedFilter13));
}

void C_GaussianScale::SetParameters() const
{
	m_Taps = m_pShader->GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vTaps", 13);
}

void C_GaussianScale::Display() const
{
	if(m_Manager.Enabled() && g_debugGaussianScale)
	{
		PERF_EVENT("GaussianScale");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());

		// get the scaled viewport
		const Display::C_Viewport& viewport = m_Manager.GetViewport();
		Display::C_Viewport scaledViewport = viewport;
		scaledViewport.m_W /= (U32)m_Downsample;
		scaledViewport.m_H /= (U32)m_Downsample;
		m_Manager.SetViewport(scaledViewport);

		// get sample positions
		if(m_bConstants)
		{
			MathsStorage::Float4 vSamples[13];
			GetGaussianSamples<5, 5>(viewport.GetW(), viewport.GetH(), vSamples, m_fMultiplier);
			Display::Device()->SetPixelShaderParameterArrayF(m_Taps, (F32*)vSamples);
			Display::Device()->SetPixelShaderParameters(m_pShader);
		}

		// set register count
		const_cast<Display::C_PixelShader*>(m_pShader)->SetRegisterCount(24);

		DisplayFinal();
	}
}



}; // namespace PostProcess
