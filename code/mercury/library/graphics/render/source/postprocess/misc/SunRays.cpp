#include "StdAfx.h"
#include "Render/PostProcess/Misc/SunRays.h"

#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psSunRays);
EXTERN_SHADER(psSunRaysGamma);

using namespace Display;

namespace PostProcess
{

Debug::Bool		g_debugSunRays("Graphics.Render.PostProcess.SunRays.Enable", true);
Debug::Float	g_debugSunMaskScale("Graphics.Render.PostProcess.SunRays.MaskScale", 4.0f, Debug::Range(0.0f, 4.0f));

bool C_SunRays::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	if(!Display::CreatePixelShaderFromFunc("Shaders/psSunRays", 0, &m_PixelShader, REF_SHADER(psSunRays)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psSunRaysGamma", 0, &m_PixelShaderGamma, REF_SHADER(psSunRaysGamma)))
		return false;

	if(!m_pShader)
	{
		SetShader();
		SetParameters();
	}

	return true;
}

bool C_SunRays::CreateVolatile()
{
	RESOURCE_MEMORY_ID("PostProcess");

	// create a sun ray texture for each different size of view
	for(U32 i = 0; i < m_Manager.GetViewCount(); ++i)
	{
		C_View& view = m_Manager.GetView(i);

		for(U32 j = 0; j < i; ++j)
		{
			C_View& prevView = m_Manager.GetView(j);
			if (view.GetWidth() == prevView.GetWidth() && view.GetHeight() == prevView.GetHeight())
			{
				m_SunRayMapping[i] = m_SunRayMapping[j];
				continue;
			}
		}

		m_SunRayMapping[i] = m_SunRayTextures.Size();
		
		const U32 iWidth = view.GetWidth() / 4;
		const U32 iHeight = view.GetHeight() / 4;
		const Display::E_Format eFormat = Display::E_Format_A8R8G8B8;

		Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;
		if (!Device()->CreateRenderTexture2D(iWidth, iHeight, 1, Display::E_Usage_Colour, eFormat, E_Multisample_None, NULL, pRenderTexture))
		{
			delete pRenderTexture;
			return false;
		}

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_SunRaysRT");
		#endif

		m_SunRayTextures.PushBack(pRenderTexture);
	}

	return true;
}

void C_SunRays::Destroy()
{
	Display::Device()->Destroy(&m_PixelShader);
	Display::Device()->Destroy(&m_PixelShaderGamma);
}

void C_SunRays::DestroyVolatile()
{
	for(S32 i = 0; i < m_SunRayTextures.Size(); ++i)
	{
		Display::Device()->Destroy(m_SunRayTextures[i]);
		delete m_SunRayTextures[i];
	}
	m_SunRayTextures.Clear();
	m_SunRayMapping.Clear();
}

void C_SunRays::SetShader() const
{
	if(m_bGamma)
		I_PostProcess::SetShader(&m_PixelShaderGamma);
	else
		I_PostProcess::SetShader(&m_PixelShader);
}

void C_SunRays::SetParameters() const
{
	m_Settings1	= m_pShader->GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings1");
	m_Settings2	= m_pShader->GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vSettings2");
}

void C_SunRays::Display() const
{
	if(m_Manager.Enabled() && g_debugSunRays && m_fSunIntensity > 0.0001f)
	{
		PERF_EVENT("SunRays");

		const Display::C_Surface* pColourSurface = Display::Device()->GetRenderTarget().GetColourSurface();
		const Display::C_Surface* pDepthStencilSurface = Display::Device()->GetRenderTarget().GetDepthStencilSurface();

		const S32 iViewIndex = m_Manager.GetViewIndex();
		const S32 iTextureIndex = m_SunRayMapping[iViewIndex];
		Display::C_RenderTexture2D* pRenderTexture = m_SunRayTextures[iTextureIndex];

		Display::Device()->SetColourDepthStencilSurface(&pRenderTexture->GetSurface(), pDepthStencilSurface);

		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = pRenderTexture->GetTexture().GetWidth();
		viewport.m_H = pRenderTexture->GetTexture().GetHeight();
		Display::Device()->SetViewport(viewport);

		const MathsStorage::Float4 psSettings1(m_vSunPosition.x, m_vSunPosition.y, 1.0f / m_vSunSize.x, 1.0f / m_vSunSize.y);
		Display::Device()->SetPixelShaderParameterF(m_Settings1, (const F32*)&psSettings1);

		const MathsStorage::Float4 psSettings2(m_fSunIntensity, 0.0f, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_Settings2, (const F32*)&psSettings2);

		Display::Device()->SetPixelShaderParameters(m_pShader);
		Display::Device()->SetTexture(0, m_pTexture);

		DisplayFinal();

		pRenderTexture->Resolve();
	}
}

}; // namespace PostProcess
