#include "StdAfx.h"
#include "Render/PostProcess/Blur/Gaussian.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugGaussianBlur("Graphics.Render.PostProcess.GaussianBlur.Enable", true);

bool C_GaussianBlur::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if(!m_pShader)
	{
		SetShader();
		SetParameters();
	}

	return true;
}

void C_GaussianBlur::Destroy()
{
}

void C_GaussianBlur::SetBilinear(const bool bBilinear) const
{
	if(bBilinear != m_bBilinear)
	{
		m_bBilinear = bBilinear;
		m_TextureFilter = bBilinear ? Display::E_TextureFilter_Linear : Display::E_TextureFilter_Point;
		SetShader();
		SetParameters();
	}
}

void C_GaussianBlur::SetShader() const
{
	if(m_bGamma && m_bBilinear)
		I_PostProcess::SetShader(m_Manager.GetShader(E_Shader_WeightedFilter4Gamma));
	else if(m_bGamma)
		I_PostProcess::SetShader(m_Manager.GetShader(E_Shader_WeightedFilter9Gamma));
	else if(m_bBilinear)
		I_PostProcess::SetShader(m_Manager.GetShader(E_Shader_WeightedFilter4));
	else
		I_PostProcess::SetShader(m_Manager.GetShader(E_Shader_WeightedFilter9));
}

void C_GaussianBlur::SetParameters() const
{
	if(m_bBilinear)
		m_Taps = m_pShader->GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vTaps", 4);
	else
		m_Taps = m_pShader->GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vTaps", 9);
}

void C_GaussianBlur::Display() const
{
	if(m_Manager.Enabled() && g_debugGaussianBlur)
	{
		PERF_EVENT("PostProcess.GaussianBlur");

		SetViewport();

		const Display::C_Viewport& viewport = m_Manager.GetViewport();

		MathsStorage::Float4 vSamples[9];

		if(m_Manager.GetChainTexture()->GetFormat() == Display2::E_TextureFormat_A8R8G8B8)
			SetBilinear(true);
		else
			SetBilinear(false);

		// blur in X
		if(m_bBilinear)
		{
			GetHorizontalBilinearGaussianSamples<4>(viewport.GetW(), viewport.GetH(), vSamples, m_fDeviation);
			Display::Device()->SetPixelShaderParameterArrayF(m_Taps, (F32*)vSamples);
			Display::Device()->SetPixelShaderParameters(m_pShader);
			Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		}
		else
		{
			GetGaussianSamples<9>(viewport.GetW(), 0, vSamples, m_fDeviation);
			Display::Device()->SetPixelShaderParameterArrayF(m_Taps, (F32*)vSamples);
			Display::Device()->SetPixelShaderParameters(m_pShader);
			Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		}

		DisplayIntermediate();
		Resolve();

		// blur in Y
		if(m_bBilinear)
		{
			GetVerticalBilinearGaussianSamples<4>(viewport.GetW(), viewport.GetH(), vSamples, m_fDeviation);
			Display::Device()->SetPixelShaderParameterArrayF(m_Taps, (F32*)vSamples);
			Display::Device()->SetPixelShaderParameters(m_pShader);
			Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		}
		else
		{
			GetGaussianSamples<9>(0, viewport.GetH(), vSamples, m_fDeviation);
			Display::Device()->SetPixelShaderParameterArrayF(m_Taps, (F32*)vSamples);
			Display::Device()->SetPixelShaderParameters(m_pShader);
			Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		}
		
		DisplayFinal();
	}
}



}; // namespace PostProcess
