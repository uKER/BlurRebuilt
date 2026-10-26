#include "StdAfx.h"
#include "Render/PostProcess/Misc/ShowDepth.h"

#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"
#include "Render/Utils/Globals.h"
#include "render/State/RenderState.h"

EXTERN_SHADER(psShowDepth);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/PostProcess/Misc/PS3/psShowDepth.shader"
#endif
#endif

using namespace Display;

namespace PostProcess
{

Debug::Bool g_debugShowDepth("Graphics.Render.PostProcess.Depth.Enable", true);

bool C_ShowDepth::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("ShowDepth", 0, &m_Shader, REF_SHADER(psShowDepth));
	SetShader(&m_Shader);
	
	return true;
}

void C_ShowDepth::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}

void C_ShowDepth::Display() const
{
	if(g_debugShowDepth && m_Manager.Enabled())
	{
		PERF_EVENT("ShowDepth");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);

		const Display::C_ViewSettings3D& viewSettings = Display::Device()->GetView()->GetViewSettings3D();

		MathsStorage::Float4 fvec;
		fvec = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_0, (F32*)&fvec, 1);

		// display the effect
		DisplayFinal();
	}
}



}; // namespace PostProcess
