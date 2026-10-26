#include "StdAfx.h"
#include "Render/PostProcess/Blur/DepthOfField.h"

#include "Render/PostProcess/Manager.h"
#include "Render/Shaders/Constants.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psDepthOfField);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/PostProcess/Blur/PS3/psDepthOfField.shader"
#endif
#endif

using namespace Display;

namespace PostProcess
{

#define SAM_COUNT 5
#define SAM_BIAS  (-2)

bool C_DepthOfField::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("DepthOfField", 0, &m_Shader, REF_SHADER(psDepthOfField));
	SetShader(&m_Shader);
	
	SetTextureFilter(Display::E_TextureFilter_Point);
	
	m_blurScale = 4.f;
	m_focusInvDepth = 1.f/10.f;
	
	return true;
}

void C_DepthOfField::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}


void C_DepthOfField::Display() const
{
	F32 taps[SAM_COUNT * 4] = { 0 };
	S32 i = 0;
	
	if(Manager()->Enabled())
	{
		PERF_EVENT("DepthOfField");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, Manager()->GetChainTexture());
		Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);

		const Display::C_View* pView = Display::Device()->GetView();
		F32 fWidth = pView->GetViewport().GetW() / 2.0f;
		F32 fHeight = pView->GetViewport().GetH() / 2.0f;
		const Display::C_ViewSettings3D& viewSettings = pView->GetViewSettings3D();
		
		MathsStorage::Float4 fvec;
		fvec.x = viewSettings.GetFarClip() * viewSettings.GetNearClip() / (viewSettings.GetNearClip() - viewSettings.GetFarClip());
		fvec.y = viewSettings.GetFarClip() / (viewSettings.GetFarClip() - viewSettings.GetNearClip());

		if(Display::Device()->GetDepthFlip())
			fvec.y = 1.0f - fvec.y;

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_0, (F32*)&fvec, 1);
		fvec.x=m_focusInvDepth;
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_1, (F32*)&fvec, 1);

		memset(taps,0,sizeof(taps));
		for (i=0;i<SAM_COUNT;i++)
		{
			taps[i<<2]=(float)(i+SAM_BIAS) / fWidth * m_blurScale;
		}
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_2, taps, SAM_COUNT);
		DisplayIntermediate();

		memset(taps,0,sizeof(taps));
		for (i=0;i<SAM_COUNT;i++)
		{
			taps[(i<<2)+1]=(float)(i+SAM_BIAS) / fHeight * m_blurScale;
		}
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_2, taps, SAM_COUNT);
		Resolve();
		DisplayFinal();
	}
}



}; // namespace PostProcess
