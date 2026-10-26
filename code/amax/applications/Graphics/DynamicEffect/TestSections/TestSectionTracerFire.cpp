
#include "StdAfx.h"
#include "TestSectionTracerFire.h"
#include "Framework/Application/Application.h"
#include "Framework/Application/UserControls.h"
#include "Input/GameInput.h"

// system
#include "System/Misc/Random.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/Debug/Charts/ChartRenderer.h"
#include "Render/Debug/Charts/LineChart.h"
#include "Render/Shaders/Constants.h"

// basic rendering
#include "Render/Utils/Helper.h"
#include "Render/Render/GridRender.h"

#include "Dynamic/Common.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Tracer/EffectInterface.h"
#include "Dynamic/Tracer/Manager.h"
#include "Dynamic/Tracer/Generic/XtInterface.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

#define USE_POST_EFFECTS
#define USE_POST_EFFECTS_HDR

C_TestSectionTracerFire::C_TestSectionTracerFire(const Util::C_String& name) : Framework::C_TestSection(name)
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

	m_pDynamicEffectRenderer = &DynamicEffect::C_Renderer::Instance();

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;

	m_pTracerManager = new DynamicEffect::Tracer::C_Manager(m_pDynamicEffectRenderer, DynamicEffect::E_ManagerTypeIndex_Tracer);

	DynamicEffect::XtInterface::Initialise(m_pDynamicEffectRenderer);
	DynamicEffect::Tracer::XtInterface::Initialise(m_pTracerManager);

	SetupPostProcess();
}

C_TestSectionTracerFire::~C_TestSectionTracerFire()
{
	DynamicEffect::Tracer::XtInterface::Shutdown();

	SAFE_DELETE_PTR(m_pPostProcessMgr);
	SAFE_DELETE_PTR(m_pDebugRenderSys);

	SAFE_DELETE_PTR(m_pDynamicEffectRenderer);
}

void C_TestSectionTracerFire::OnResize()
{
	if(!m_pPostProcessMgr)
		return;

	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();

	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

bool C_TestSectionTracerFire::OnCreate()
{
	return true;
}

bool C_TestSectionTracerFire::OnCreateVolatile()
{
	return true;
}

void C_TestSectionTracerFire::OnDestroyVolatile()
{
}

void C_TestSectionTracerFire::OnDestroy()
{
}

void C_TestSectionTracerFire::OnUpdate()
{
	DynamicEffect::Tracer::XtInterface::Update();
	m_pDynamicEffectRenderer->Update(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());

	if(!Framework::C_Application::GetInstance()->GetTimer().IsPaused())
	{
		for(ProjectileList::Iterator pIterator = m_Projectiles.Begin(); pIterator != m_Projectiles.End(); )
		{
			pIterator->Update(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime());

			Maths::HVec3 vPosition = pIterator->GetPosition();

			if(vPosition.Y() < 0.0f || Maths::Length(vPosition) > 100.0f)
			{
				C_Projectile* pProjectile = &(*pIterator);
				pIterator = m_Projectiles.Erase(pIterator);

				m_pDynamicEffectRenderer->CleanNode(pProjectile->GetNode());
				m_ProjectilePool.Free(pProjectile);
			}
			else
				++pIterator;
		}

		if(Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm) && !m_ProjectilePool.Empty())
		{
			if(C_Projectile* pProjectile = m_ProjectilePool.Allocate())
			{
				Display::C_View* pView = &Framework::C_Application::GetInstance()->GetSplitscreenView(0).m_View;
				const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
				const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();

				Maths::HVec3 vForward(viewMatrix._13, viewMatrix._23, viewMatrix._33);
				Maths::HVec3 vPosition(viewSettings3D.GetCameraPosition());

				DynamicEffect::C_Node node;
				node.SetPosition(vPosition);
				node.SetRotation(DynamicEffect::GetQuaternionFromTo<MathsStorage::Float4A16>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), vForward));
				
				Maths::HVec3 vVelocity = vForward * 25.0f;
				pProjectile->Initialise(vVelocity, node);

				// spawn here
				DynamicEffect::Tracer::I_EffectBase* pEffect = DynamicEffect::Tracer::XtInterface::FindEffect("Basic");
				if(pEffect)
				{
					pEffect->Spawn(pProjectile->GetNode(), Maths::C_RGBA_F32(8.0f, 6.0f, 2.0f, 1.0f));
				}

				m_Projectiles.PushBack(*pProjectile);
			}
		}
	}
}

void C_TestSectionTracerFire::OnPreRender()
{
	m_pDynamicEffectRenderer->BeginFrame();
}

void C_TestSectionTracerFire::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));
	
	m_pDynamicEffectRenderer->BeginView();

	Render::BeginPostEffectStates();

	m_SkyDome.Render();

	Render::DrawGrid();

	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);

	Render::EndPostEffectStates();
}

void C_TestSectionTracerFire::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	m_pDynamicEffectRenderer->BeginView();
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);

	for(ProjectileList::ConstIterator pIterator = m_Projectiles.Begin(); pIterator != m_Projectiles.End(); ++pIterator)
	{
		pIterator->Render();
	}
}

void C_TestSectionTracerFire::OnBeginPostEffects()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_TestSectionTracerFire::OnEndPostEffects()
{
	if(!m_pPostProcessMgr)
		return;

	if(m_pPostProcessHdr)
		m_pPostProcessHdr->Display();

	m_pPostProcessMgr->End();

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}

void C_TestSectionTracerFire::SetupPostProcess()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	const S32 viewCount = Display::Device()->GetViewCount();
	for(S32 viewIndex=0; viewIndex<viewCount; viewIndex++)
	{
		const Display::C_View* pView = Display::Device()->GetView(viewIndex);
		if(pView->GetUserIndex() != -1)
		{
			const Display::C_Viewport& viewport = pView->GetViewport();

			if(!m_pPostProcessHdr || m_pPostProcessHdr->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
			{
				m_pPostProcessMgr->AddView(
					viewport.GetW(),
					viewport.GetH(),
					pView->GetIndex(),
					PostProcess::E_RenderTargetFormat_RGBA_U8,
					Display::E_Multisample_None,
					5);
			}			
			else
			{
				m_pPostProcessMgr->AddView(
					viewport.GetW(),
					viewport.GetH(),
					pView->GetIndex(),
					PostProcess::E_RenderTargetFormat_RGBA_F10,
					Display::E_Multisample_None,
					5);
			}
		}
	}
}
