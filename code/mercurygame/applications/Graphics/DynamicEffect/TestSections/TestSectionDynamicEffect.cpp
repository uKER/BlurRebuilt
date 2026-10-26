#include "StdAfx.h"
#include "TestSectionDynamicEffect.h"
#include "Framework/Application/Application.h"

#include "Render/Utils/Helper.h"
#include "Render/Render/GridRender.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/Minimal/Minimal.h"

#include "Render/View/ViewManager.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

#include <System/Misc/Random.h>

#define USE_POST_EFFECTS
#define USE_POST_EFFECTS_HDR

C_TestSectionDynamicEffect::C_TestSectionDynamicEffect(const Util::C_String& name) : Framework::C_TestSection(name)
{
	Util::C_Random random;

	#ifdef USE_POST_EFFECTS
	m_pPostProcessMgr	= new PostProcess::C_Manager();
	#ifdef USE_POST_EFFECTS_HDR
	m_pPostProcessHdr	= new PostProcess::C_HighDynamicRange();
	#else
	m_pPostProcessHdr	= NULL;
	#endif
	#else
	m_pPostProcessMgr	= NULL;
	m_pPostProcessHdr	= NULL;
	#endif

	if(m_pPostProcessHdr)
	{
		m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);

		#ifndef PLATFORM_XENON
			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
		#endif
	}

	m_pDynamicEffectRenderer = new DynamicEffect::C_Renderer(NULL);
	m_pDynamicEffectRenderer->SetAssetPath("Graphics/Textures/Particles/");
	
	// add minimal dynamic effect example
	{
		m_pMinimalDynamicEffect = new DynamicEffect::C_Minimal(m_pDynamicEffectRenderer, 0, 1234);

		S32 randomObjects = 20;

		for(S32 i = 0; i < randomObjects; ++i)
		{
			Maths::C_MatrixRotateRPH rph(random.FRand(-Maths::Pi, Maths::Pi),
											random.FRand(-Maths::Pi, Maths::Pi),
											random.FRand(-Maths::Pi, Maths::Pi));
			Maths::HMat33 rotation(rph);
			Maths::HMat43 transform;
			transform.Translation(Maths::HVec3(random.FRand(-20.0f, 0.0f), random.FRand(10.0f, 0.0f), random.FRand(-20.0f, 0.0f)));
			transform.Rotation(rotation);
			Maths::C_RGBA_F32 colour(random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f), 1.0f);

			m_pMinimalDynamicEffect->AddObject(colour, transform, false);
		}
	}

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;

	SetupPostProcess();
}

C_TestSectionDynamicEffect::~C_TestSectionDynamicEffect()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
	SAFE_DELETE_PTR(m_pDebugRenderSys);

	SAFE_DELETE_PTR(m_pMinimalDynamicEffect);
	SAFE_DELETE_PTR(m_pDynamicEffectRenderer);
}

void C_TestSectionDynamicEffect::OnResize()
{
	if(!m_pPostProcessMgr)
		return;

	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();

	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

bool C_TestSectionDynamicEffect::OnCreate()
{
	return true;
}

bool C_TestSectionDynamicEffect::OnCreateVolatile()
{
	return true;
}

void C_TestSectionDynamicEffect::OnDestroyVolatile()
{
}

void C_TestSectionDynamicEffect::OnDestroy()
{
}

void C_TestSectionDynamicEffect::OnUpdate()
{
	m_pDynamicEffectRenderer->Update(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());
}

void C_TestSectionDynamicEffect::OnPreRender()
{
	//m_pDynamicEffectRenderer->BeginFrame();
}

void C_TestSectionDynamicEffect::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	//m_pDynamicEffectRenderer->BeginView();

	Render::BeginPostEffectStates();

	m_SkyDome.Render();

	Render::DrawGrid();

//	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
//	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);

	Render::EndPostEffectStates();
}

void C_TestSectionDynamicEffect::OnRender3DViewWithoutAA(const S32 viewIndex)
{
//	m_pDynamicEffectRenderer->BeginView();
//	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);
}

void C_TestSectionDynamicEffect::OnBeginPostEffects()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_TestSectionDynamicEffect::OnEndPostEffects()
{
	if(!m_pPostProcessMgr)
		return;

	if(m_pPostProcessHdr)
		m_pPostProcessHdr->Display();

	m_pPostProcessMgr->End();

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}

void C_TestSectionDynamicEffect::SetupPostProcess()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	for (U32 viewNo = 0; viewNo < viewCount; ++viewNo)
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
