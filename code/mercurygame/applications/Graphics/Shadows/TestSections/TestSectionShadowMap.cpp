#include "stdafx.h"
#include "TestSections/TestSectionShadowMap.h"
#include <Render/Render/GridRender.h>
#include <Framework/Application/UserControls.h>
#include <Framework/Application/ApplicationBase.h>
#include <Camera2.h>
#include <Camera2/stock/FlyCamera.h>

// scene graph
#include <System/Serialise/C_FileSystem.h>
#include <System/Serialise/C_FileBlock.h>
#include <RenderingData/RenderingData.h>
#include <Scenegraph/Scenegraph.h>
#include <Scenegraph/Visible/Backfrontsorter.h>
#include <Fx/Fx.h>

// render
#include "Render/State/RenderState.h"

// shadow
#include "Effect/Shadow/ShadowMapSetup.h"
#include "Effect/Shadow/ShadowMapView.h"
#include "Effect/Shadow/CascadedShadowMap.h"
#include "Effect/Shadow/PerspectiveShadowMap.h"
#include "Effect/Shadow/StandardShadowMap.h"
#include "Effect/Shadow/ShadowMapDebug.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/View/ViewRender.h"

Debug::Enum<E_ShadowMapType>	g_debugTestSectionShadowMapType("TestSection.ShadowMap.Type", E_ShadowMapType_Standard);
Debug::Enum<E_ShadowMapView>	g_debugTestSectionShadowMapView("TestSection.ShadowMap.View", E_ShadowMapView_Standard);
Debug::Float	g_debugTestSectionShadowMapLightElevation("TestSection.ShadowMap.LightElevation", 45.0f, Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugTestSectionShadowMapLightHeading("TestSection.ShadowMap.LightHeading", 0.0f, Debug::Range(0.0f, 360.0f));
Debug::Float	g_debugTestSectionShadowMapLightOffset("TestSection.ShadowMap.LightOffset", 40.0f, Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugTestSectionShadowMapNearSplit("TestSection.ShadowMap.NearSplit", 1.0f, Debug::Range(0.0f, 1000.0f));
Debug::Float	g_debugTestSectionShadowMapFarSplit("TestSection.ShadowMap.FarSplit", 100.0f, Debug::Range(0.0f, 1000.0f));
Debug::Bool		g_debugTestSectionShadowMapFixView("TestSection.ShadowMap.FixView", false);

C_TestSectionShadowMap::C_TestSectionShadowMap(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
	m_pSceneGraph = new SceneGraph::C_SceneGraph();

	m_pScene = new RenderingData::Target::Scene();

	FX::Initialise();

	LoadScene();

	// insert the scene
	m_pSceneGraph->Insert(m_pScene);

	m_pSorter = new SceneGraph::C_BackFrontSorter;

	m_pSceneGraph->SetNumRenderPasses(3);

	// todo - set sorting strategies etc per render pass
	m_pSceneGraph->SetRenderPassType(0, SceneGraph::E_RenderPass_Solid);
	m_pSceneGraph->SetRenderPassType(1, SceneGraph::E_RenderPass_Punch);
	m_pSceneGraph->SetRenderPassType(2, SceneGraph::E_RenderPass_Alpha, m_pSorter);

	m_pEyeView = m_pSceneGraph->AddView(0);
	m_pEyeView->EnableRenderPass(	SceneGraph::PassFlag<SceneGraph::E_RenderPass_Solid>() |
									SceneGraph::PassFlag<SceneGraph::E_RenderPass_Punch>() |
									SceneGraph::PassFlag<SceneGraph::E_RenderPass_Alpha>() );
	
	m_pShadowView = m_pSceneGraph->AddView(1);
	m_pShadowView->EnableRenderPass(	SceneGraph::PassFlag<SceneGraph::E_RenderPass_Solid>() |
										SceneGraph::PassFlag<SceneGraph::E_RenderPass_Punch>() );

	//// Set the debug feed to fly camera:
	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetDebugType(Cam2::E_FlyCamera);
	
	// Position the fly camera:
	Cam2::C_Camera& camera = view.DebugFeed().Live();
	camera.SetPosition(Maths::HVec3(-30.f, 20.f, -30.f));
	camera.LookAt(Maths::HVec3());

	// debug renderer

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;
	
	m_pSceneGraph->CreateJobQueuePolicy();

	// start the scheduler
	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(24);

	m_pSceneGraph->CreateJobQueueGroups();

	// create the cube map
	if (!Display::CreateTextureCubeFromFile("Textures/Environment.dds", &m_TextureCube))
		Display::CreateTextureCubeFromFile("Dontship/Textures/Environment.dds", &m_TextureCube);;

	// create the shadow map
	m_ShadowMap.Create();
	m_ExponentialShadowMap.Create();
	m_VarianceShadowMap.Create();
}

C_TestSectionShadowMap::~C_TestSectionShadowMap()
{
	m_ShadowMap.Destroy();
	m_ExponentialShadowMap.Destroy();
	m_VarianceShadowMap.Destroy();

	SAFE_DELETE_PTR(m_pSorter);
	SAFE_DELETE_PTR(m_pSceneGraph);
	SAFE_DELETE_PTR(m_pScene);

	Scheduler::Shutdown();
}

void C_TestSectionShadowMap::OnUpdate()
{
}

void C_TestSectionShadowMap::OnPreRender()
{
	// render shadow map
	if (g_debugTestSectionShadowMapType == E_ShadowMapType_Standard)
		m_ShadowMap.Bind();
	else if (g_debugTestSectionShadowMapType == E_ShadowMapType_Exponential)
		m_ExponentialShadowMap.Bind();
	else if (g_debugTestSectionShadowMapType == E_ShadowMapType_Variance)
		m_VarianceShadowMap.Bind();
	
	RenderShadowPass();

	if (g_debugTestSectionShadowMapType == E_ShadowMapType_Standard)
		m_ShadowMap.Unbind();
	else if (g_debugTestSectionShadowMapType == E_ShadowMapType_Exponential)
		m_ExponentialShadowMap.Unbind();
	else if (g_debugTestSectionShadowMapType == E_ShadowMapType_Variance)
		m_VarianceShadowMap.Unbind();
}

void C_TestSectionShadowMap::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	// render the world
	RenderWorldPass();

	// reset states since world renderer will have trashed them
	Display::Device()->ResetStates();
	Display::Device()->DefaultStates();
	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);

	// render sky
	m_SkyDome.Render();

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();

	if (g_debugTestSectionShadowMapType == E_ShadowMapType_Standard)
		m_TextureRenderUtils.DisplayRed<Display::C_Texture2D>(0.0f, 0.0f, 512.0f, 384.0f, m_ShadowMap.GetTexture());
	else if (g_debugTestSectionShadowMapType == E_ShadowMapType_Exponential)
		m_TextureRenderUtils.DisplayRed<Display::C_Texture2D>(0.0f, 0.0f, 512.0f, 384.0f, m_ExponentialShadowMap.GetTexture());
	else if (g_debugTestSectionShadowMapType == E_ShadowMapType_Variance)
		m_TextureRenderUtils.DisplayRGB<Display::C_Texture2D>(0.0f, 0.0f, 512.0f, 384.0f, m_VarianceShadowMap.GetTexture());
}

void C_TestSectionShadowMap::RenderShadowPass()
{
	const F32 fCosHeading = cosf(g_debugTestSectionShadowMapLightHeading * Maths::Pi / 180.0f);
	const F32 fSinHeading = sinf(g_debugTestSectionShadowMapLightHeading * Maths::Pi / 180.0f);
	const F32 fCosElevation = cosf(g_debugTestSectionShadowMapLightElevation * Maths::Pi / 180.0f);
	const F32 fSinElevation = sinf(g_debugTestSectionShadowMapLightElevation * Maths::Pi / 180.0f);
	const Maths::HVec3 vLightDirection(fSinHeading * fCosElevation, fSinElevation, fCosHeading * fCosElevation);
	
	if (!g_debugTestSectionShadowMapFixView)
	{
		m_ShadowEyeView = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).m_View;
	}
	
	Display::C_View* pView = &m_ShadowEyeView;
	const Maths::HVec3 vCameraPosition(pView->GetViewSettings3D().GetCameraToWorld().AW());

	Effect::C_ShadowMapSetup shadowMapSetup;
	shadowMapSetup.m_pView			= pView;
	shadowMapSetup.m_vPosition		= vCameraPosition + g_debugTestSectionShadowMapLightOffset * vLightDirection;
	shadowMapSetup.m_vDirection		= -g_debugTestSectionShadowMapLightOffset * vLightDirection;
	shadowMapSetup.m_fNearClip		= 1.0f;
	shadowMapSetup.m_fFarClip		= 200.0f;
	shadowMapSetup.m_fPerspDist		= 1.0f;
	shadowMapSetup.m_vViewExtents	= Maths::C_Rect(Maths::HVec2(-25.0f, -25.0f), Maths::HVec2(+25.0f, +25.0f));
	shadowMapSetup.m_iWidth			= 1024;
	shadowMapSetup.m_iHeight		= 1024;

	const F32 fNearSplit = g_debugTestSectionShadowMapNearSplit;
	const F32 fFarSplit = g_debugTestSectionShadowMapFarSplit;

	Effect::C_ShadowMapView shadowMapView;
	if (g_debugTestSectionShadowMapView == E_ShadowMapView_Trapezoid)
		TrapezoidalShadowMapSetup(shadowMapView, fNearSplit, fFarSplit, shadowMapSetup);
	else if (g_debugTestSectionShadowMapView == E_ShadowMapView_ParallelSplit)
		ParallelSplitShadowMapSetup(shadowMapView, fNearSplit, fFarSplit, shadowMapSetup);
	else if (g_debugTestSectionShadowMapView == E_ShadowMapView_StableCascaded)
		StableCascadedShadowMapSetup(shadowMapView, fNearSplit, fFarSplit, shadowMapSetup);
	else
		DirectionalLightShadowMapSetup(shadowMapView, shadowMapSetup);

	m_pShadowView->SetViewFrustum(shadowMapView.m_View.GetViewFrustum());
	m_pShadowView->SetViewData(shadowMapView.m_View.GetViewSettings3D());

	Display::Device()->SetView(&shadowMapView.m_View, true);

	{
		PERF_EVENT("RenderShadowPass");
		m_pSceneGraph->GatherNodes(*m_pShadowView);

		m_pSceneGraph->Render(*m_pShadowView, SceneGraph::E_RenderPass_Solid);
		m_pSceneGraph->Render(*m_pShadowView, SceneGraph::E_RenderPass_Punch);
	}

	// queue up view data to render deferred
	Render::RenderViewFrustum(&shadowMapView.m_View, 0xFFFF00FF);
	Render::RenderViewFrustum(&m_ShadowEyeView, 0xFF00FF00);
}

void C_TestSectionShadowMap::RenderWorldPass()
{
	// set parameters
	FX::Parameter *pCubemap = FX::GetSharedParameter("g_tEnvironment");
	const void *pAddress = m_TextureCube.GetGPUTexture();
	FX::SetParameter(pCubemap, &pAddress);
	FX::SetSamplerState(pCubemap,
						Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
						Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
						1, 0, 0.0f);
	
	const F32 fCosHeading = cosf(g_debugTestSectionShadowMapLightHeading * Maths::Pi / 180.0f);
	const F32 fSinHeading = sinf(g_debugTestSectionShadowMapLightHeading * Maths::Pi / 180.0f);
	const F32 fCosElevation = cosf(g_debugTestSectionShadowMapLightElevation * Maths::Pi / 180.0f);
	const F32 fSinElevation = sinf(g_debugTestSectionShadowMapLightElevation * Maths::Pi / 180.0f);
	const MathsStorage::Float4 vSunDirection(fSinHeading * fCosElevation, fSinElevation, fCosHeading * fCosElevation, 0.0f);
	FX::SetSharedParameterByName("g_vSunDirection", &vSunDirection);
	FX::SetSharedParameterByName("g_vSunColour", Display::C_PSConstant(1.0f, 1.0f, 1.0f, 1.0f));
	FX::SetSharedParameterByName("g_vSkyColour", Display::C_PSConstant(0.5f, 0.5f, 0.5f, 1.0f));
	FX::SetSharedParameterByName("g_vGroundColour", Display::C_PSConstant(0.2f, 0.2f, 0.2f, 1.0f));

	// set hdr parameters
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		FX::Parameter*	pVectorParameter	= FX::GetSharedParameter("g_vLuminance");
		FX::Parameter*	pTextureParameter	= FX::GetSharedParameter("g_tLuminance");
		
		const Display::C_Float4	pVector		= Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		const void*				pTexture	= Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture()->GetGPUTexture();
		
		FX::SetParameter(pVectorParameter, &pVector);
		FX::SetParameter(pTextureParameter, &pTexture);
		FX::SetSamplerState(pTextureParameter,
							Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
							Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
							1, 0, 0.0f);
	}

	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	m_pEyeView->SetViewFrustum(view.m_View.GetViewFrustum());
	m_pEyeView->SetViewData(view.m_View.GetViewSettings3D());

	Display::Device()->SetView(&view.m_View, true);

	{
		PERF_EVENT("RenderWorldPass");
		m_pSceneGraph->GatherNodes(*m_pEyeView);

		m_pSceneGraph->Render(*m_pEyeView, SceneGraph::E_RenderPass_Solid);
		m_pSceneGraph->Render(*m_pEyeView, SceneGraph::E_RenderPass_Punch);
		m_pSceneGraph->Render(*m_pEyeView, SceneGraph::E_RenderPass_Alpha);
	}
}

void C_TestSectionShadowMap::LoadScene()
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("Models/Model.dat", Sys::E_FileAccess_Read);

	if (!pFile)
	{
		assert(!"Could not open file.");
		return;
	}
	else
	{
		Sys::C_FileBlock rootBlock( pFile );
		 
		// keep reading child blocks
		while ( rootBlock.GetPosition() < rootBlock.Length() )
		{
			Sys::C_FileBlock block( &rootBlock );
 
			if( block.HeaderId() == "Scene" )
			{
				Arch::BinaryInputArchive ar(block);
				ar >> *m_pScene;

				for (int i = 0; i < m_pScene->GetNumResourceBlocks(); i++)
				{
					ar >> m_pScene->GetResourceBlock(i);
				}
			}
		}
	}

	delete pFile;

	m_pScene->UpdateResourcePointers();
}
