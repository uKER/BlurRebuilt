#include "stdafx.h"
#include "ModelRenderSection.h"
#include "modelrenderer.h"
#include <Framework/Application/UserControls.h>
#include <Framework/Application/ApplicationBase.h>
#include <Render/Render/GridRender.h>
#include <Render/Sky/SkyDome.h>
#include <Render/State/RenderState.h>
#include <Render/Utils/Helper.h>
#include <Render/View/ViewManager.h>
#include <camera2.h>
#include <camera2/debug/DebugTools.h>
#include <camera2/stock/FlyCamera.h>

#include <fx/fx.h>

namespace PostProcess
{
extern Debug::Bool g_debugPostProcessEnable;
}

static Debug::Int g_ActiveFlyCamera("ModelRenderApp.ActiveFlyCamera", 0, Debug::Range(0, 2));
static S32 g_LastFlyCamera = -1;

C_ModelRenderSection::C_ModelRenderSection(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
#ifndef PLATFORM_XENON
	//PostProcess::g_debugPostProcessEnable = false; // disable post-processing on PS3 and PC for now because it's broken [LloydW] This should work now...
#endif
	FX::Initialise();

	m_pModelRendererApp = new ModelRendererApp;
	m_pModelRendererApp->Create();
	m_pSkyDome = new Render::Sky::C_SkyDome();

	// Set the debug feed to fly camera:
	// Set the debug feed to fly camera:
	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetDebugType(Cam2::E_FlyCamera);
	
	// Position the fly camera:
	Cam2::C_Camera camera;
	camera.SetPosition(Maths::HVec3(-30.f, 20.f, -30.f));
	camera.LookAt(Maths::HVec3());
	view.DebugFeed().CopyFrom(camera);

	m_FlyCameras.PushBack(new Cam2::C_FlyCamera);
	m_FlyCameras.PushBack(new Cam2::C_FlyCamera);
	m_FlyCameras.PushBack(new Cam2::C_FlyCamera);
	
	m_FlyCameras[0]->SetPosition(Maths::HVec3(-30.f, 20.f, -30.f)); //djh
	m_FlyCameras[0]->LookAt(Maths::HVec3());

	m_pDebugTools = new Cam2::C_DebugTools;
	m_pDebugTools->Init();

	m_pCubemap = new Display::C_TextureCube;
	if (!Display::CreateTextureCubeFromFile("Textures/Environment.dds", m_pCubemap))
		Display::CreateTextureCubeFromFile("Dontship/Textures/Environment.dds", m_pCubemap);;
}

C_ModelRenderSection::~C_ModelRenderSection()
{
	SAFE_DELETE_PTR(m_pDebugTools)

	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().ShowFeed(0);
	for (S32 i = 0; i < m_FlyCameras.Size(); i++)
		delete m_FlyCameras[i];
	m_FlyCameras.Clear();

	m_pModelRendererApp->Destroy();
	delete m_pModelRendererApp;
	delete m_pSkyDome;
	delete m_pCubemap;

	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

void C_ModelRenderSection::OnUpdate()
{
	F32 period = 1.f / 60.f;

	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetInput( Framework::GetFlyCameraControls(0) );

	m_pDebugTools->SetLocalCamera(view.DebugFeed().Live());
	m_pDebugTools->Update(period);
	switch (m_pDebugTools->Notify())
	{
		case Cam2::E_NetNotify_SetPos:
			// Set camera position:
			view.DebugFeed().CopyFrom(m_pDebugTools->RemoteCamera());
		break;
	}

	S32 active = g_ActiveFlyCamera;
	if (active >= 0 && active < m_FlyCameras.Size())
	{
		Cam2::C_FlyCamera& activeFly = *m_FlyCameras[active];
		activeFly.SetInput(Framework::GetFlyCameraControls(0));

		if (active != g_LastFlyCamera)
		{
			view.DebugFeed().ShowFeed(&activeFly);
			g_LastFlyCamera = active;
		}
	}

	for (S32 i = 0; i < m_FlyCameras.Size(); i++)
		m_FlyCameras[i]->Update(period);

	m_pModelRendererApp->Update(view);
}

void C_ModelRenderSection::OnRenderHdr3DViewWithAA(const S32 iView)
{
	FX::SetSharedParameterByName("g_tEnvironment", &m_pCubemap->GetGPUTexture());

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		FX::Parameter*	pTextureParameter	= FX::GetSharedParameter("g_tLuminance");
		
		const Display::C_Float4	pVector		= Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		const void*				pTexture	= Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture()->GetGPUTexture();
		
		FX::SetSharedParameterByName("g_vLuminance", &pVector);
		FX::SetParameter(pTextureParameter, &pTexture);
		FX::SetSamplerState(pTextureParameter,
							Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
							Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
							1, 0, 0.0f);
	}

	m_pSkyDome->Render();
	m_pModelRendererApp->Render();
	Display::Device()->Reset();
}
