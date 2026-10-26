#include "stdafx.h"
#include "TestSections/TestSectionHDR.h"
#include "Render/Utils/Helper.h"
#include "Render/View/ViewManager.h"
#include "Render/PostProcess/Bloom/Bloom.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Render/PostProcess/Manager.h"

namespace Framework
{

C_TestSectionHDR::C_TestSectionHDR(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_pPostProcessMgr = new PostProcess::C_Manager();
	m_pPostProcessHdr = new PostProcess::C_HighDynamicRange(*m_pPostProcessMgr);
	m_pPostProcessBloom = new PostProcess::C_Bloom(*m_pPostProcessMgr);

	m_pPostProcessHdr->SetBloom(m_pPostProcessBloom);
	m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);

	SetupPostProcess();
}

C_TestSectionHDR::~C_TestSectionHDR()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

void C_TestSectionHDR::OnRender3DViewWithAA(const S32 viewIndex)
{
	m_pPostProcessMgr->SetView(Display::Device()->GetView()->GetIndex());
	Render::BeginPostEffectStates();

	OnRenderHdr3DViewWithAA(viewIndex);
	
	Render::EndPostEffectStates();
}

void C_TestSectionHDR::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	OnRenderHdr3DViewWithoutAA(viewIndex);
}

void C_TestSectionHDR::OnBeginRenderSplitscreenWithoutAA()
{
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());
}

void C_TestSectionHDR::OnEndRenderSplitscreenWithoutAA()
{
	m_pPostProcessHdr->Display();
	m_pPostProcessMgr->End();
}

void C_TestSectionHDR::OnResize()
{
	// destroy post process volatile and re-add the views
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();
	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

void C_TestSectionHDR::SetupPostProcess()
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

}; // namespace Framework
