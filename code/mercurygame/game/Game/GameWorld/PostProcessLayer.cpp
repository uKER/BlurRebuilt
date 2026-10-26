#include "stdafx.h"

#include <Game/Game.h>

#include <framework/application/ApplicationBase.h>

#include <Game/GameWorld/PostProcessLayer.h>

#include <Render/PostProcess/Manager.h>
#include <Render/PostProcess/Misc/EdgeAntiAliasing.h>
#include <Render/PostProcess/HDR/HDR.h>
#include <Render/PostProcess/Bloom/Bloom.h>
#include <Effect/PostProcess/MotionBlur/MotionBlurReduced.h>
#include <Effect/PostProcess/MotionBlurDoFReduced.h>
#include <Render/Utils/Helper.h>
#include <Render/View/ViewManager.h>

#include <Effect/PostProcess/DoFClassify.h>
#include <Effect/PostProcess/DoFFgBg.h>
#include <Effect/PostProcess/DoFReduced.h>
#include <Effect/PostProcess/DoFSimple.h>

namespace Game
{

Debug::Bool g_debugPostProcessMotionBlur	("Game.PostProcess.MotionBlur",		true);
Debug::Bool g_debugPostProcessDepthOfField	("Game.PostProcess.DepthOfField",	true);

C_PostProcessLayer::C_PostProcessLayer()
	: m_pPostProcessMgr(NULL)
	, m_pPostProcessHdr(NULL)
	, m_pPostProcessBloom(NULL)
	, m_pPostProcessMotionBlur(NULL)
	, m_pPostProcessMotionBlurDoF(NULL)
	, m_pPostProcessDoF(NULL)
	, m_fFocusPlane(0.0f)
	, m_fAperture(0.0f)
	, m_pTextureBuffer(0)
	, m_iTextureBufferSize(0)
{
	m_pPostProcessMgr = new PostProcess::C_Manager();

	m_pPostProcessEdgeAA = new PostProcess::C_EdgeAntiAliasing(*m_pPostProcessMgr);

	m_pPostProcessMotionBlur = new PostProcess::C_MotionBlurReduced(*m_pPostProcessMgr);
	m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_11);

	m_pPostProcessMotionBlurDoF = new PostProcess::C_MotionBlurDoFReduced(*m_pPostProcessMgr);
	m_pPostProcessMotionBlurDoF->SetSamples(PostProcess::E_MotionBlurDoFSamples_11);

	m_pPostProcessDoF = new PostProcess::C_DoFFgBg(*m_pPostProcessMgr);

	m_pPostProcessHdr = new PostProcess::C_HighDynamicRange(*m_pPostProcessMgr);
	m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);
	#ifdef PLATFORM_PS3
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	#else
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_Shader);
	#endif
	
	m_pPostProcessBloom = new PostProcess::C_Bloom(*m_pPostProcessMgr);

	SetupPostProcess();
}

C_PostProcessLayer::~C_PostProcessLayer()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

HRESULT C_PostProcessLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;

		case Framework::E_LayerMessage_Resize:
			return Resize();
		break;

		case Framework::E_LayerMessage_BeginRenderSplitscreenWithoutAA:
			return BeginRenderSplitscreenWithoutAA();
		break;

		case Framework::E_LayerMessage_EndRenderSplitscreenWithoutAA:
			return EndRenderSplitscreenWithoutAA();
		break;

		case Framework::E_LayerMessage_RenderSplitscreenWithAA:
			return RenderSplitscreenWithAA();
		break;
	}

	return S_OK;
}

HRESULT C_PostProcessLayer::RenderSplitscreenWithAA()
{
	//if(Display::Device()->GetView()->m_ViewFlags && Render::E_ViewFlag_EnablePostProcess)
	{
		m_pPostProcessMgr->SetView(Display::Device()->GetView()->GetIndex());
	}

	return S_OK;
}

HRESULT C_PostProcessLayer::Create()
{
	// allocate shared memory buffers for effects
	const U32 iMotionBlurBufferSize		= m_pPostProcessMotionBlur->GetTextureBufferRequiredSize();
	const U32 iDepthOfFieldBufferSize	= m_pPostProcessDoF->GetTextureBufferRequiredSize();
	const U32 iMotionBlurDoFBufferSize	= m_pPostProcessMotionBlurDoF->GetTextureBufferRequiredSize();

	m_iTextureBufferSize = Maths::Max(iMotionBlurBufferSize, Maths::Max(iDepthOfFieldBufferSize, iMotionBlurDoFBufferSize));

#if defined(PLATFORM_XENON)
	if (!Display::Device()->AllocMemory(128, m_iTextureBufferSize, &m_pTextureBuffer, Display::E_AllocHeap_Large, Display::E_AllocType_Start, Display2::E_MemoryType_Texture))
		return E_FAIL;
#elif defined(PLATFORM_PS3)
	if (!Display::Device()->AllocMemory(128, m_iTextureBufferSize, &m_pTextureBuffer, Display::E_AllocHeap_Video, Display::E_AllocType_Default, Display2::E_MemoryType_Texture))
		return E_FAIL;
#endif

//	m_pPostProcessMotionBlurDoF->SetTextureBuffer(m_pTextureBuffer, m_iTextureBufferSize);
	m_pPostProcessMotionBlur->SetTextureBuffer(m_pTextureBuffer, m_iTextureBufferSize);
	m_pPostProcessDoF->SetTextureBuffer(m_pTextureBuffer, m_iTextureBufferSize);

	if( !m_pPostProcessMgr->Create() )
		return E_FAIL;

	if( !m_pPostProcessMgr->CreateVolatile() )
		return E_FAIL;

	m_pPostProcessHdr->SetBloom(m_pPostProcessBloom);
	m_pPostProcessHdr->SetBloomGamma(true);

	return S_OK;
}

HRESULT C_PostProcessLayer::Destroy()
{
	m_pPostProcessMgr->Destroy();
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMotionBlurDoF->SetTextureBuffer(0, 0);
	m_pPostProcessMotionBlur->SetTextureBuffer(0, 0);
	m_pPostProcessDoF->SetTextureBuffer(0, 0);

#if defined(PLATFORM_XENON)
	Display::Device()->FreeMemory(m_iTextureBufferSize, m_pTextureBuffer, Display::E_AllocHeap_Large, Display2::E_MemoryType_Texture);
#elif defined(PLATFORM_PS3)
	Display::Device()->FreeMemory(m_iTextureBufferSize, m_pTextureBuffer, Display::E_AllocHeap_Video, Display2::E_MemoryType_Texture);
#endif
	m_pTextureBuffer = 0;
	m_iTextureBufferSize = 0;

	return S_OK;
}

HRESULT C_PostProcessLayer::BeginRenderSplitscreenWithoutAA()
{
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	return S_OK;
}

Debug::Bool g_ResetHack("Game.ResetHack", true);

HRESULT C_PostProcessLayer::EndRenderSplitscreenWithoutAA()
{
	if(g_ResetHack)
		Display::Device()->Reset();

#if defined(PLATFORM_PS3)
	// on PS3, do the luminance calculation before the motionblur, because the motion blur blats the alpha channel
	m_pPostProcessHdr->SetCalculateLuminance(true);
	m_pPostProcessHdr->CalculateLuminance();
	m_pPostProcessHdr->SetCalculateLuminance(false);
#endif

	if(g_debugPostProcessMotionBlur && g_debugPostProcessDepthOfField)
	{
		m_pPostProcessMotionBlurDoF->SetDepthOfField(0, m_fFocusPlane, m_fAperture);
		m_pPostProcessMotionBlurDoF->Display();
	}
	else if(g_debugPostProcessMotionBlur)
	{
		m_pPostProcessMotionBlur->Display();
	}
	else if(g_debugPostProcessDepthOfField)
	{
		m_pPostProcessDoF->SetDepthOfField(0, m_fFocusPlane, m_fAperture);
		m_pPostProcessDoF->Display();
	}
	//m_pPostProcessEdgeAA->Display();
	m_pPostProcessHdr->Display();
	m_pPostProcessMgr->End();

	return S_OK;
}

HRESULT C_PostProcessLayer::Resize()
{
	// destroy post process volatile and re-add the views
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();
	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();

	return S_OK;
}

void C_PostProcessLayer::SetupPostProcess()
{
	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	for (S32 viewNo = 0; viewNo < viewCount; ++viewNo)
	{
		const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(viewNo);
		if (pView && pView->m_ViewFlags & Render::E_ViewFlag_EnablePostProcess)
		{
			const Display::C_Viewport& viewport = pView->m_View.GetViewport();
			if(m_pPostProcessHdr->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
				m_pPostProcessMgr->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
			else
				m_pPostProcessMgr->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_F10, Display::E_Multisample_None, 5);
		}
	}
}

void C_PostProcessLayer::SetDofInfo(F32 fFocusPlane, F32 fAperture)
{
	m_fFocusPlane = fFocusPlane;
	m_fAperture = fAperture;
}



} // namespace Game
