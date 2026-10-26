#include "stdafx.h"

#include <GameApp.h>
#include <game/AmaxCam/CameraSystem.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/View/Mirror.h>
#include <Game/GameWorld/View/Reflection.h>

#include <Fx/EnvironmentSettings.h>

#include <render/debug/debugrender.h>

#include <SchedulerConfig.h>

#define SCENEGRAPH_ALLOW_INCLUDE // just for GameWorld.cpp because it sets up the SceneGraph
#include <scenegraph/SceneGraphImpl.h>
#include <SceneGraph/MultiView.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/RenderPass.h>

#include <Graphics/HDR.h>

#include <Display/Device/DeviceMarkers.h>
#include <Camera2/stock/DebugFeed.h>

namespace Game
{
Debug::Bool g_bShowViewReferencePosition("Amax.ShowViewReferencePosition", false);

extern GameWorldFxParams	g_FxParams;

// we overload begin gather in order to perform some game specific passing of light spheres to be inclusion spheres
void C_MultiView::BeginGather() const
{
	if(!m_Gathering)
	{
		if(m_Views.Size() == 0)
			return;

		MEMORY_TRACKER_PUSH("graphics.scenegraph");

		PERF_EVENT("MultiView.BeginGather");

		// clear all nodes
		ClearNodes();

		// manually gather the lightmanager. this is because we need the results for the inclusion spheres for the prepass view
		m_pView->m_MainView.SetupPrepassLightSpheres();
		Gather();

		MEMORY_TRACKER_POP("graphics.scenegraph");

		m_Gathering = true;
	}
}

C_View::C_View(C_ViewManager* pViewManager, const S32 index) :
	m_Index(index),
	m_pViewManager(pViewManager),
	m_pMirror(NULL),
	m_pReflection(NULL),
	m_MainView(pViewManager),
	m_pEnvironmentSettings(NULL)
{

	m_pFrameworkView = &Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(index);
}

C_View::~C_View()
{

}

void C_View::Create(const bool createMirror, const bool createReflection)
{
	// we always have a main view so create
	m_MainView.Create(*this);
	
	if(createMirror)
	{
		m_pMirror = new C_MirrorView(m_pViewManager);
		m_pMirror->Create();
	}
	else
		m_pMirror = NULL;

	if(createReflection)
	{
		m_pReflection = new C_ReflectionView(m_pViewManager);
		m_pReflection->Create();
		m_pReflection->CreateVolatile();
	}
	else
		m_pReflection = NULL;

	// use a multiview to gather main, prepass, mirror and reflection views simultaneously
	m_pMultiView = new C_MultiView(m_pViewManager->GetSceneGraph(), this);
}

void C_View::Destroy()
{
	m_MainView.Destroy();
	
	if(m_pMirror)
	{
		m_pMirror->Destroy();
		delete m_pMirror;
	}

	if(m_pReflection)
	{
		m_pReflection->DestroyVolatile();
		m_pReflection->Destroy();
		delete m_pReflection;
	}

	delete m_pMultiView;

	SAFE_DELETE_PTR(m_pEnvironmentSettings);
}

void C_View::UpdateView()
{
	m_MainView.UpdateView(*this);
}

void C_View::Begin()
{
	// clear all views from multivew
	m_pMultiView->ClearViews();

	//Calculate view reference position (not in slomo)
	if (m_GroundRay.m_result.Hit() && (C_GameApp::GetInstance()->GetTimeScale() == 1.0f))
	{
		m_ReferencePosition = m_GroundRay.m_result.HitPoint();
		if (g_bShowViewReferencePosition)
			Render::Debug::Deferred()->Draw3dArrow(m_ReferencePosition, Maths::HVec3(0.f, 1.f, 0.f), 3.f, 1.f, Maths::RGBA(255, 0, 0 , 255));
	}

	//Create collision task to calculate reference position
	Maths::HVec3 rayPos = GetFrameworkView()->DebugFeed().Position();
	Collision::C_CollisionFilter filter(Game::PolyFlags::kReflectionPlane, Game::PolyFlags::kReflectionPlane, 0, 0, 0);
	Collision::C_Ray ray;
	Maths::HVec3 rayDir(AmaxCam::CameraSystem().DebugFeed().Mat43().AZ().X(), -0.3f, GetFrameworkView()->DebugFeed().Mat43().AZ().Z());
	m_GroundRay.InitialiseRay(rayPos, rayPos + rayDir * 100.0f, &filter);
	m_pViewManager->m_pGameWorld->GetPhysicsScene()->DoDeferredRayTrace(&m_GroundRay, NULL, m_pViewManager->m_pGameWorld->GetRaytraceGroup(), C_SchedulerConfig::GetInstance()->GetCarDynamicEffectsPolicy());


	if(m_pReflection)
		m_pReflection->Gather(*this);
	
	if(m_pMirror && !C_GameApp::GetInstance()->IsPaused())
		m_pMirror->Gather(*this);

	m_MainView.Gather(*this);

	m_pMultiView->BeginGather();
}

void C_View::Schedule()
{
	m_pMultiView->EndGather();
	
	if(m_pReflection)
		m_pReflection->Schedule(*this);
	
	if(m_pMirror && !C_GameApp::GetInstance()->IsPaused())
		m_pMirror->Schedule(*this);

	m_MainView.Schedule(*this);
}

void C_View::PreRenderSplitscreen()
{
	// Render the prepass first
	// (This needs to be done on the PS3 so that when we render out the lights on the SPUs,
	// the RSX is busy rendering other things before the sync and the main scene)

	// Render the first stage of the prepass
	// 1. Geometry
	// 2. Point lights (potentially on SPU)
	// 3. Spot lights (only if we are NOT doing SPU lighting, otherwise do it in second pass)
	{
		DEVICE_MARKER("PreRenderSplitscreen.RenderPrepass1");
		m_MainView.RenderPrepass1(*this);
	}

	const S32 viewCount = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	if(viewCount == 1)
	{
		if(m_pViewManager->GetCubeMap())
		{
			DEVICE_MARKER("PreRenderSplitscreen.RenderCubeMap");
			m_pViewManager->GetCubeMap()->Render();
		}
	}

	if(m_pReflection)
	{
		DEVICE_MARKER("PreRenderSplitscreen.RenderReflection");
		m_pReflection->Render(*this);
	}
	if(m_pMirror && !C_GameApp::GetInstance()->IsPaused())
	{
		DEVICE_MARKER("PreRenderSplitscreen.RenderMirror");
		m_pMirror->Render(*this);
	}

	// Render the second stage of the prepass
	// 1. Sync SPU tasks (PS3 using SPU lighting only)
	// 2. Spot lights (PS3 using SPU lighting only)
	{
		DEVICE_MARKER("PreRenderSplitscreen.RenderPrepass2");
		m_MainView.RenderPrepass2(*this);
	}
}

void C_View::RenderWithAA()
{
	m_MainView.RenderWithAA(*this);
}


void C_View::RenderWithoutAA()
{
	m_MainView.RenderWithoutAA(*this);
}

void C_View::ResetEnvironmentSettings( FX::I_EnvironmentSettings* sorceryEnvironmentSettings )
{
	SAFE_DELETE_PTR(m_pEnvironmentSettings);
	if (sorceryEnvironmentSettings)
	{
		//Create environment settings per view
		m_pEnvironmentSettings = sorceryEnvironmentSettings->Lerp(sorceryEnvironmentSettings, 1.f);
	}
}

void C_View::SetDynamicDetail(const F32 detail)
{
	m_MainView.SetDynamicDetail(detail);
	if (m_pMirror)
		m_pMirror->SetDynamicDetail(detail);
	if (m_pReflection)
		m_pReflection->SetDynamicDetail(detail);
}

#if PLATFORM_PC
// [Blur2PC] Retail PC graphical detail for one view (Blur.exe 1.2 0x00419620).
void C_View::SetGraphicalDetail(const U32 graphicalDetail, const bool inUI)
{
	m_MainView.SetGraphicalDetail(graphicalDetail, inUI);
	if (m_pMirror)
		m_pMirror->SetGraphicalDetail(graphicalDetail);
	if (m_pReflection)
		m_pReflection->SetGraphicalDetail(graphicalDetail);
}
#endif

} // namespace Game
