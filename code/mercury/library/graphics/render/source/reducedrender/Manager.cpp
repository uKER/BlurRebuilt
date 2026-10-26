#include "StdAfx.h"
#include "Render/ReducedRender/Manager.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/ReducedRender/MSAA.h"

EXTERN_SHADER(psComposite);
EXTERN_SHADER(psDownsampleDepth4);
EXTERN_SHADER(psDownsampleDepth16);

namespace Render
{
namespace Reduced
{

Debug::Bool	g_debugEnable					("Graphics.Render.Reduced.Enable",						true);
Debug::Int	g_debugDownsample4RegisterCount	("Graphics.Render.Reduced.Downsample4RegisterCount",	8,	Debug::Range(8, 48));
Debug::Int	g_debugDownsample16RegisterCount("Graphics.Render.Reduced.Downsample16RegisterCount",	8,	Debug::Range(8, 48));

C_Manager::C_Manager(const bool bFullPrecision)
	: Display::I_Renderer("ReducedRender")
	, m_bBegun(false)
	, m_bFullPrecision(bFullPrecision)
	, m_eDownsample(E_Downsample_Quarter)
	, m_iIndex(-1)
	, m_pMSAA(0)
{
}

C_Manager::~C_Manager()
{
	if(m_pMSAA)
		delete m_pMSAA;
	DestroyRenderer();
}

bool C_Manager::OnCreate()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psComposite", 0, &m_CompositeShader, REF_SHADER(psComposite)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shaders/psDownsampleDepth4", 0, &m_Downsample4Shader, REF_SHADER(psDownsampleDepth4)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDownsampleDepth16", 0, &m_Downsample16Shader, REF_SHADER(psDownsampleDepth16)))
			return false;

	m_Downsample4Parameter = m_Downsample4Shader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_Downsample16Parameter = m_Downsample16Shader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");

	return true;
}

bool C_Manager::OnCreateVolatile()
{
	MEMORY_TRACKER_PUSH("graphics.renderers.reducedrender");

	assert(m_RenderTargets4.Size() == 0);
	assert(m_RenderTargets16.Size() == 0);
	assert(m_RenderTargetMapping.Size() == 0);
	assert(m_ViewMapping.Size() == 0);

	for(S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		Display::E_Format eColourFormat = m_bFullPrecision ? Display::E_Format_A16B16G16R16F : Display::E_Format_A8R8G8B8;

		bool bCreateTexture = true;
		U32 iMapping = 0;
		
		// go through all current viewports and see if we have textures created for this view size and format already
		for(S32 i = 0; i < iView && bCreateTexture; ++i)
		{
			if(m_Views[i].iWidth == m_Views[iView].iWidth
				&& m_Views[i].iHeight == m_Views[iView].iHeight)
			{
				iMapping = i;
				bCreateTexture = false;
			}
		}

		if(bCreateTexture)
		{
			// we didn't match this view size and/or format so we need to create some new textures here
			RenderTarget renderTarget4;
			renderTarget4.pColourRenderTexture = new Display::C_RenderTexture2D;
			renderTarget4.pDepthStencilSurface = new Display::C_Surface;

			RenderTarget renderTarget16;
			renderTarget16.pColourRenderTexture = new Display::C_RenderTexture2D;
			renderTarget16.pDepthStencilSurface = new Display::C_Surface;

			#if PLATFORM_PC
			const Display::E_Format eDepthStencilFormat = Display::E_Format_D24S8;
			#else
			const Display::E_Format eDepthStencilFormat = Display::E_Format_D24FS8;
			#endif
			const U32 iWidth = m_Views[iView].iWidth;
			const U32 iHeight = m_Views[iView].iHeight;
			const U32 iWidth4 = m_Views[iView].iWidth / 2;
			const U32 iHeight4 = m_Views[iView].iHeight / 2;
			const U32 iWidth16 = m_Views[iView].iWidth / 4;
			const U32 iHeight16 = m_Views[iView].iHeight / 4;

			// colour surfaces
			const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
			Display::C_SurfaceParams surfaceParams;

			if (!Display::Device()->CreateRenderTexture2D(iWidth4, iHeight4, 1, Display::E_Usage_Colour | Display::E_Usage_Expand, eColourFormat, Display::E_Multisample_None, &surfaceParams, renderTarget4.pColourRenderTexture))
			{
				delete renderTarget4.pColourRenderTexture;
				MEMORY_TRACKER_POP("graphics.renderers.reducedrender");
				return false;
			}

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetRenderTexture2DName(renderTarget4.pColourRenderTexture, "_Reduced4RT");
			#endif

			const U32 iTextureFlags = Display::E_Usage_Colour | Display::E_Usage_Expand;
			const U32 iTextureSize = Display::GetRenderTexture2DSize(iWidth16, iHeight16, 1, iTextureFlags, eColourFormat, Display::E_Multisample_None);

			if (!Display::Device()->CreateRenderTexture2D(iWidth16, iHeight16, 1, iTextureFlags, eColourFormat, Display::E_Multisample_None, &surfaceParams, renderTarget16.pColourRenderTexture))
			{
				delete renderTarget16.pColourRenderTexture;
				MEMORY_TRACKER_POP("graphics.renderers.reducedrender");
				return false;
			}

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetRenderTexture2DName(renderTarget16.pColourRenderTexture, "_Reduced16RT");
			#endif

			// depth surfaces

			if (!Display::Device()->CreateDepthStencilSurface(iWidth4, iHeight4, 0, eDepthStencilFormat, Display::E_Multisample_None, &surfaceParams, renderTarget4.pDepthStencilSurface))
			{
				delete renderTarget4.pDepthStencilSurface;
				MEMORY_TRACKER_POP("graphics.renderers.reducedrender");
				return false;
			}

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetSurfaceName(renderTarget4.pDepthStencilSurface, "_Reduced4DS");
			#endif

			if (!Display::Device()->CreateDepthStencilSurface(iWidth16, iHeight16, 0, eDepthStencilFormat, Display::E_Multisample_None, &surfaceParams, renderTarget16.pDepthStencilSurface))
			{
				delete renderTarget16.pDepthStencilSurface;
				MEMORY_TRACKER_POP("graphics.renderers.reducedrender");
				return false;
			}

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetSurfaceName(renderTarget16.pDepthStencilSurface, "_Reduced16DS");
			#endif

			// store the mapping and add to viewport
			iMapping = m_RenderTargets4.Size();
			m_RenderTargets4.PushBack(renderTarget4);
			m_RenderTargets16.PushBack(renderTarget16);
		}

		while(m_RenderTargetMapping.Size() <= m_Views[iView].iIndex)
			m_RenderTargetMapping.PushBack(-1);

		m_RenderTargetMapping[m_Views[iView].iIndex] = iMapping;

		while(m_ViewMapping.Size() <= m_Views[iView].iIndex)
			m_ViewMapping.PushBack(-1);

		m_ViewMapping[m_Views[iView].iIndex] = iView;
	}

	MEMORY_TRACKER_POP("graphics.renderers.reducedrender");

	return true;
}

void C_Manager::OnDestroy()
{
	Display::Device()->Destroy(&m_CompositeShader);
	Display::Device()->Destroy(&m_Downsample4Shader);
	Display::Device()->Destroy(&m_Downsample16Shader);
}

void C_Manager::OnDestroyVolatile()
{
	// delete textures and render targets
	for(S32 i = 0; i < m_RenderTargets4.Size(); ++i)
	{
		if(m_RenderTargets4[i].pColourRenderTexture->IsCreated())
			Display::Device()->Destroy(m_RenderTargets4[i].pColourRenderTexture);

		delete m_RenderTargets4[i].pColourRenderTexture;

		if(m_RenderTargets4[i].pDepthStencilSurface->IsCreated())
			Display::Device()->Destroy(m_RenderTargets4[i].pDepthStencilSurface);

		delete m_RenderTargets4[i].pDepthStencilSurface;

		if(m_RenderTargets16[i].pColourRenderTexture->IsCreated())
			Display::Device()->Destroy(m_RenderTargets16[i].pColourRenderTexture);

		delete m_RenderTargets16[i].pColourRenderTexture;
		
		if(m_RenderTargets16[i].pDepthStencilSurface->IsCreated())
			Display::Device()->Destroy(m_RenderTargets16[i].pDepthStencilSurface);
	
		delete m_RenderTargets16[i].pDepthStencilSurface;
	}
	m_RenderTargets4.Clear();
	m_RenderTargets16.Clear();
	m_RenderTargetMapping.Clear();
	m_ViewMapping.Clear();
}

void C_Manager::SetDepth(const S32 iIndex, const E_Downsample eDownsample, const Display::C_Texture2D* pDepthTexture) const
{
	PERF_EVENT("ReducedRender.DownsampleDepth");

	if(eDownsample != E_Downsample_QuarterMSAA)
	{
		// get view and render target
		const S32 iView = m_ViewMapping[iIndex];
		const S32 iRenderTarget = m_RenderTargetMapping[iIndex];

		const Display::C_Surface* pColourSurface = 0;
		const Display::C_Surface* pDepthStencilSurface = 0;
		if (eDownsample == E_Downsample_Quarter)
		{
			pColourSurface = &m_RenderTargets4[iRenderTarget].pColourRenderTexture->GetSurface();
			pDepthStencilSurface = m_RenderTargets4[iRenderTarget].pDepthStencilSurface;
		}
		else if (eDownsample == E_Downsample_Sixteenth)
		{
			pColourSurface = &m_RenderTargets16[iRenderTarget].pColourRenderTexture->GetSurface();
			pDepthStencilSurface = m_RenderTargets16[iRenderTarget].pDepthStencilSurface;
		}
		const U32 iWidth = pColourSurface->GetWidth();
		const U32 iHeight = pColourSurface->GetHeight();

		// save current state
		const Display::C_RenderTarget renderTarget = Display::Device()->GetRenderTarget();
		m_pColourSurface = renderTarget.GetColourSurface();
		m_pDepthStencilSurface = renderTarget.GetDepthStencilSurface();
		m_SavedViewport = Display::Device()->GetViewport();

		// downsample depth
		Display::Device()->SetColourDepthStencilSurface(pColourSurface, pDepthStencilSurface);

		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = iWidth;
		viewport.m_H = iHeight;
		Display::Device()->SetViewport(viewport);

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_None);

		if (eDownsample == E_Downsample_Quarter)
		{
			const_cast<Display::C_PixelShader*>(&m_Downsample4Shader)->SetRegisterCount(g_debugDownsample4RegisterCount);
			Display::Device()->SetPixelShaderParameterF(m_Downsample4Parameter, Display::C_PSConstant(1.0f / (F32)m_SavedViewport.GetW(), 1.0f / (F32)m_SavedViewport.GetH(), 0.0f, 0.0f));
			Display::Quad::DrawFullScreenDepth(0, pDepthTexture, &m_Downsample4Shader);
		}
		else if (eDownsample == E_Downsample_Sixteenth)
		{
			const_cast<Display::C_PixelShader*>(&m_Downsample16Shader)->SetRegisterCount(g_debugDownsample16RegisterCount);
			Display::Device()->SetPixelShaderParameterF(m_Downsample16Parameter, Display::C_PSConstant(1.0f / (F32)m_SavedViewport.GetW(), 1.0f / (F32)m_SavedViewport.GetH(), 0.0f, 0.0f));
			Display::Quad::DrawFullScreenDepth(0, pDepthTexture, &m_Downsample16Shader);
		}

		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);

		// restore state
		Display::Device()->SetColourDepthStencilSurface(m_pColourSurface, m_pDepthStencilSurface);
		Display::Device()->SetViewport(m_SavedViewport);
	}
}

Display::C_Surface*	C_Manager::GetColourSurface(const S32 iIndex, const E_Downsample eDownsample) const
{
	if(iIndex < 0 || iIndex >= m_RenderTargetMapping.Size())
		return 0;
	const S32 iRenderTarget = m_RenderTargetMapping[iIndex];
	if(eDownsample == E_Downsample_Quarter)
	{
		if(iRenderTarget < 0 || iRenderTarget >= m_RenderTargets4.Size())
			return 0;
		else
			return &m_RenderTargets4[iRenderTarget].pColourRenderTexture->GetSurface();
	}
	else if(eDownsample == E_Downsample_Sixteenth)
	{
		if(iRenderTarget < 0 || iRenderTarget >= m_RenderTargets16.Size())
			return 0;
		else
			return &m_RenderTargets16[iRenderTarget].pColourRenderTexture->GetSurface();
	}
	else
	{
		return 0;
	}
}

Display::C_Surface*	C_Manager::GetDepthStencilSurface(const S32 iIndex, const E_Downsample eDownsample) const
{
	if(iIndex < 0 || iIndex >= m_RenderTargetMapping.Size())
		return 0;
	const S32 iRenderTarget = m_RenderTargetMapping[iIndex];
	if(eDownsample == E_Downsample_Quarter)
	{
		if(iRenderTarget < 0 || iRenderTarget >= m_RenderTargets4.Size())
			return 0;
		else
			return m_RenderTargets4[iRenderTarget].pDepthStencilSurface;
	}
	else if(eDownsample == E_Downsample_Sixteenth)
	{
		if(iRenderTarget < 0 || iRenderTarget >= m_RenderTargets16.Size())
			return 0;
		else
			return m_RenderTargets16[iRenderTarget].pDepthStencilSurface;
	}
	else
	{
		return 0;
	}
}

void C_Manager::Begin(const S32 iIndex, const E_Downsample eDownsample) const
{
	PERF_EVENT("ReducedRender.Begin");

#if !defined(CFG_SPEED)
	if (!g_debugEnable)
		return;
#endif

	assert(!m_bBegun && "Reduced render Begin() called without matching End()");

	// begin
	m_bBegun = true;
	m_eDownsample = eDownsample;
	m_iIndex = iIndex;

	// save current state
	const Display::C_RenderTarget renderTarget = Display::Device()->GetRenderTarget();
	m_pColourSurface = renderTarget.GetColourSurface();
	m_pDepthStencilSurface = renderTarget.GetDepthStencilSurface();
	m_SavedViewport = Display::Device()->GetViewport();

	if(eDownsample != E_Downsample_QuarterMSAA)
	{
		// get view and render target
		const S32 iView = m_ViewMapping[iIndex];
		const S32 iRenderTarget = m_RenderTargetMapping[iIndex];

		const Display::C_Surface* pColourSurface = 0;
		const Display::C_Surface* pDepthStencilSurface = 0;
		if (eDownsample == E_Downsample_Quarter)
		{
			pColourSurface = &m_RenderTargets4[iRenderTarget].pColourRenderTexture->GetSurface();
			pDepthStencilSurface = m_RenderTargets4[iRenderTarget].pDepthStencilSurface;
		}
		else if (eDownsample == E_Downsample_Sixteenth)
		{
			pColourSurface = &m_RenderTargets16[iRenderTarget].pColourRenderTexture->GetSurface();
			pDepthStencilSurface = m_RenderTargets16[iRenderTarget].pDepthStencilSurface;
		}
		const U32 iWidth = pColourSurface->GetWidth();
		const U32 iHeight = pColourSurface->GetHeight();

		// set surfaces and viewport
		Display::Device()->SetColourDepthStencilSurface(pColourSurface, pDepthStencilSurface);

		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = iWidth;
		viewport.m_H = iHeight;
		Display::Device()->SetViewport(viewport);

		Maths::RGBA colour;

		// turn off luminance writes
		Display::Device()->SetLuminanceWrite(Display::E_LuminanceWrite_Disabled);
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);

		// clear colour surface
		Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0xFF000000));
	}
	else
	{
		if(!m_pMSAA)
			m_pMSAA = new C_MSAA;

		m_pMSAA->Begin();

		Display::C_Viewport viewport;
		viewport.m_X = 0;
		viewport.m_Y = 0;
		viewport.m_W = m_SavedViewport.m_W / 2;
		viewport.m_H = m_SavedViewport.m_H / 2;
		Display::Device()->SetViewport(viewport);
	}
}

void C_Manager::End() const
{
#if !defined(CFG_SPEED)
	if (!g_debugEnable)
		return;
#endif

	assert(m_bBegun && "Reduced render End() called without matching Begin()");

	if(m_eDownsample != E_Downsample_QuarterMSAA)
	{
		PERF_EVENT("ReducedRender.End");
		// resolve
		const S32 iView = m_ViewMapping[m_iIndex];
		const S32 iRenderTarget = m_RenderTargetMapping[m_iIndex];

		if(m_eDownsample == E_Downsample_Quarter)
			m_RenderTargets4[iRenderTarget].pColourRenderTexture->Resolve();
		else if(m_eDownsample == E_Downsample_Sixteenth)
			m_RenderTargets16[iRenderTarget].pColourRenderTexture->Resolve();

		// if we are using hdr u32 we need to enable luminance alpha writes
		const Render::C_RenderState* pRenderState = &Render::C_RenderState::Instance();
		if(pRenderState->HDR() && pRenderState->HDR()->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			Display::Device()->SetLuminanceWrite(Display::E_LuminanceWrite_Enabled);
		}
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	}

	// restore state
	Display::Device()->SetColourDepthStencilSurface(m_pColourSurface, m_pDepthStencilSurface);
	Display::Device()->SetViewport(m_SavedViewport);

	// clear states
	m_bBegun = false;
	m_iIndex = -1;
}

void C_Manager::Render(const S32 iIndex, const E_Downsample eDownsample, const E_BlendMode eBlendMode) const
{
	if(eDownsample != E_Downsample_QuarterMSAA)
	{
		if(const Display::C_Texture2D* pTexture = GetTexture(iIndex, eDownsample))
		{
			PERF_EVENT("ReducedRender.Composite");
			if(eBlendMode == E_BlendMode_Additive)
			{
				Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);
			}
			else if(eBlendMode == E_BlendMode_Alpha)
			{
				Display::Device()->SetBlendMode(Display::E_BlendMode_Unknown);
				Display::Device()->SetBlendEnable(Display::E_Bool_True);
				Display::Device()->SetAlphaTestEnable(Display::E_Bool_False);
				Display::Device()->SetBlend(Display::E_Blend_One, Display::E_Blend_SrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_Zero, Display::E_Blend_One, Display::E_BlendOp_Add);
			}
			if (eDownsample == E_Downsample_Quarter)
			{
				Display::Quad::SetQuadSettings(MathsStorage::UInt2(1,1), 2.0f, 2.0f);
				Display::Quad::DrawFullScreen(pTexture, &m_CompositeShader);
			}
			else if (eDownsample == E_Downsample_Sixteenth)
			{
				Display::Quad::SetQuadSettings(MathsStorage::UInt2(1,1), 4.0f, 4.0f);
				Display::Quad::DrawFullScreen(pTexture, &m_CompositeShader);
			}
			Display::Quad::SetQuadSettings(MathsStorage::UInt2(1,1), 1.0f, 1.0f);
			Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		}
	}
}

const Display::C_Texture2D* C_Manager::GetTexture(const S32 iIndex, const E_Downsample eDownsample) const
{
	const S32 iRenderTarget = m_RenderTargetMapping[iIndex];

	if (eDownsample == E_Downsample_Quarter)
	{
		return &m_RenderTargets4[iRenderTarget].pColourRenderTexture->GetTexture();
	}
	else if (eDownsample == E_Downsample_Sixteenth)
	{
		return &m_RenderTargets16[iRenderTarget].pColourRenderTexture->GetTexture();
	}
	return NULL;
}

void C_Manager::AddView(const S32 iIndex, const U32 iWidth, const U32 iHeight)
{
#ifndef CFG_SPEED
	for(S32 i = 0; i < m_Views.Size(); ++i)
		assert(m_Views[i].iIndex != iIndex);
#endif

	m_Views.PushBack();
	m_Views.Back().iWidth = iWidth;
	m_Views.Back().iHeight = iHeight;
	m_Views.Back().iIndex = iIndex;
}

} // namespace Reduced
} // namespace Render
