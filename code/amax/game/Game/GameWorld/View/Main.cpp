#include "stdafx.h"
#include <Render/Utils/Globals.h>	// [Blur2PC] GetGlobalStencilBuffer
#include "Deferred/Manager.h"	// [Blur2PC] prepass depth copy

#include <AuxFlares/AuxFlares.h>

#include <BlobShadows/BlobShadows.h>

#include <Dynamic/Renderer.h>

#include <Game/AmaxCam/CameraSystem.h>

#include <Game/GameWorld/View/Main.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/ViewType.h>
#include <Game/GameWorld/View/Reflection.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameWorld/GameWorld.h>
#include <SplashCamera/SplashCamera.h>

#include <Graphics/Lighting/Prepass.h>

#include <framework/view/View.h>
#include <fx/Fx.h>
#include <fx/RenderMutex.h>

#include <Graphics/GpuMarkers.h>
#include <Graphics/HDR.h>

#define SCENEGRAPH_ALLOW_INCLUDE
#include <scenegraph/light/Manager.h>
#include <scenegraph/SceneGraphImpl.h>
#include <SceneGraph/RenderGroupSettings.h>
#include <scenegraph/View.h>
#include <scenegraph/RenderNode.h>

#include <Render/ReducedRender/Manager.h>

#include <DropShadows/DropShadows.h>
#include <render/postprocess/Manager.h>
#include <render/postprocess/bloom/BloomFeedbackGhost.h>
#include <render/utils/Globals.h>
#include <Render/State/RenderState.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <crowd_system/Effects/CrowdFlags.h>

#include "Advertising\Advertising.h"

#include <Display/Device/DeviceMarkers.h>
#include "Game/GameActors/Vehicles/VehicleActor.h"
#include <shared_actors/Utility/VisibilityDetection.h>	// [Blur2PC] low detail car shadows
#include <Camera2/Camera.h>
#include <Camera2/stock/DebugFeed.h>
#include <Game/AmaxCam/VehicleFeed.h>

DEBUG_ENUM(Render::Reduced::E_Downsample)
	DE_OPTION(Render::Reduced::E_Downsample_None,			"None")
	DE_OPTION(Render::Reduced::E_Downsample_QuarterMSAA,	"QuarterMSAA")
	DE_OPTION(Render::Reduced::E_Downsample_Quarter,		"Quarter")
	DE_OPTION(Render::Reduced::E_Downsample_Sixteenth,		"Sixteenth")
END_DEBUG_ENUM

Debug::Enum<Render::Reduced::E_Downsample> g_MainReduceRender2D		("Amax.Views.Main.ReduceRender2D",		Render::Reduced::E_Downsample_Sixteenth);

enum E_AlphaPassRenderState
{
	E_AlphaPassRenderState_Default = 0,
	E_AlphaPassRenderState_PunchThrough,
	E_AlphaPassRenderState_AlphaDither,
	E_AlphaPassRenderState_Alpha,
	E_AlphaPassRenderState_Count
};

DEBUG_ENUM(E_AlphaPassRenderState)
	DE_OPTION(E_AlphaPassRenderState_Default,		"Default")
	DE_OPTION(E_AlphaPassRenderState_PunchThrough,	"PunchThrough")
	DE_OPTION(E_AlphaPassRenderState_AlphaDither,	"AlphaDither")
	DE_OPTION(E_AlphaPassRenderState_Alpha,			"Alpha")
END_DEBUG_ENUM

Debug::Enum<E_AlphaPassRenderState>	g_PunchPassRenderState		("Amax.Views.PunchPassRenderState",	E_AlphaPassRenderState_Default);
Debug::Enum<E_AlphaPassRenderState>	g_AlphaPassRenderState		("Amax.Views.AlphaPassRenderState",	E_AlphaPassRenderState_Default);
Debug::Int							g_AlphaPassAlphaRef			("Amax.Views.AlphaPassAlphaRef",	128, Debug::Range(0, 255));

Debug::Bool g_SceneGraphRenderTwoDimensional("Graphics.SceneGraph.RenderPasses.TwoDimensional", true);
Debug::Bool g_SceneGraphFreezeTwoDimensional("Amax.Views.FreezeTwoDimensional", false);

namespace PostProcess
{
	extern Debug::Bool g_debugPostProcessEnable;
}

extern Debug::Bool g_debugDisplayRestoreZcull;

namespace Game
{

struct GameWorldFxParams;
	
bool g_PrepassEnable = true;
#if PLATFORM_PC
extern Debug::Bool g_ReflectionEnable;	// [Blur2PC] View/Reflection.cpp
// [Blur2PC] Retail PC debug variable (Blur.exe 1.2 0x0111dffc, default true); set by the graphical detail
// preset (off at detail 1). GameWorld::BeginFrame folds it into g_PrepassEnable.
Debug::Bool g_MainPrepassEnable("Amax.Views.Main.PrepassEnable", true);
#endif

extern GameWorldFxParams	g_FxParams;
extern Debug::Bool g_MainReduceRenderOverride;
Debug::Bool g_MainReduceRenderCompositeToneMap("Amax.Views.Main.ReduceRenderCompositeToneMap", true);

Debug::Bool g_SceneGraphRenderSolid("Graphics.SceneGraph.RenderPasses.Solid", true);
Debug::Bool g_SceneGraphRenderVehicleSolid("Graphics.SceneGraph.RenderPasses.VehicleSolid", true);
Debug::Bool g_SceneGraphRenderAnimatedObjectSolid("Graphics.SceneGraph.RenderPasses.AnimatedObjectSolid", true);
Debug::Bool g_SceneGraphRenderAnimatedObjectPunch("Graphics.SceneGraph.RenderPasses.AnimatedObjectPunch", true);
Debug::Bool g_SceneGraphRenderCharacterSolid("Graphics.SceneGraph.RenderPasses.CharacterSolid", true);
Debug::Bool g_SceneGraphRenderCharacterPunch("Graphics.SceneGraph.RenderPasses.CharacterPunch", true);
Debug::Bool g_SceneGraphRenderCharacterAlpha("Graphics.SceneGraph.RenderPasses.CharacterAlpha", true);
Debug::Bool g_SceneGraphRenderEffectDecal("Graphics.SceneGraph.RenderPasses.EffectDecal", true);
Debug::Bool g_SceneGraphRenderRacification("Graphics.SceneGraph.RenderPasses.Racification", true);
Debug::Bool g_SceneGraphRenderDynamicObjectSolid("Graphics.SceneGraph.RenderPasses.DynamicObjectSolid", true);
Debug::Bool g_SceneGraphRenderPunch("Graphics.SceneGraph.RenderPasses.Punch", true);
Debug::Bool g_SceneGraphRenderPunchPrepass("Graphics.SceneGraph.RenderPasses.PunchInPrepass", true);
Debug::Bool g_SceneGraphRenderGhost("Graphics.SceneGraph.RenderPasses.Ghost", true);
Debug::Bool g_SceneGraphRenderVehicleAlpha("Graphics.SceneGraph.RenderPasses.VehicleAlpha", true);
Debug::Bool g_SceneGraphRenderSky("Graphics.SceneGraph.RenderPasses.Sky", true);
Debug::Bool g_SceneGraphRenderDecal("Graphics.SceneGraph.RenderPasses.Decal", true);
Debug::Bool g_SceneGraphRenderRefract("Graphics.SceneGraph.RenderPasses.Refract", true);
Debug::Bool g_SceneGraphRenderAlpha("Graphics.SceneGraph.RenderPasses.Alpha", true);
Debug::Bool g_SceneGraphRenderAlphaVehicleFlares("Graphics.SceneGraph.RenderPasses.AlphaVehicleFlares", true);
Debug::Bool g_SceneGraphRenderShadows("Graphics.SceneGraph.RenderPasses.Shadows", true);
#if PLATFORM_PS3
Debug::Int g_NumMainViewWorkers("Graphics.SceneGraph.NumMainViewWorkers", 10, Debug::Range(0, 20));
#endif

Debug::Bool g_debugEnableDropShadows("Amax.DropShadow.Enable", true);
//Debug::Float g_debugDropShadowsRadius("Amax.DropShadow.Radius", 1.0f, Debug::Range(0.0f, 5.0f));
//Debug::Float g_debugDropShadowsIntensity("Amax.DropShadow.Intensity", 0.5f, Debug::Range(0.0f, 1.0f));

//Debug::Float g_BlobShadowsQuality("Amax.BlobShadows.Quality", 1.f, Debug::Range(0.f, 1.f));

Debug::Bool g_EnableMainAlphaToCoverage("Amax.Views.MainAlphaToCoverage", true);

SceneGraph::RenderStates			g_PrepassSolidRenderStates;
SceneGraph::RenderStates			g_PrepassPunchRenderStates;
SceneGraph::RenderStates			g_MainPunchRenderStates;
SceneGraph::RenderStates			g_MainAlphaRenderStates;

enum E_RenderGroup
{
	E_RenderGroup_MainWithAA = 0,
	E_RenderGroup_MainWithoutAA
};

void C_MainView::Create(const C_View& gameView)
{
	m_pMain = m_pViewManager->GetSceneGraph()->AddView();
	m_pMain->EnableRenderPass(  SceneGraph::Flag<E_RenderPass_Solid>() |
								SceneGraph::Flag<E_RenderPass_Punch>() |
								SceneGraph::Flag<E_RenderPass_Decal>() |
							    SceneGraph::Flag<E_RenderPass_Refract>() |
								SceneGraph::Flag<E_RenderPass_Alpha>() |
								SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
								SceneGraph::Flag<E_RenderPass_VehicleSolid>() |
								SceneGraph::Flag<E_RenderPass_AnimatedObjectSolid>() |
								SceneGraph::Flag<E_RenderPass_AnimatedObjectPunch>() |
								SceneGraph::Flag<E_RenderPass_CharacterSolid>() |
								SceneGraph::Flag<E_RenderPass_CharacterPunch>() |
								SceneGraph::Flag<E_RenderPass_CharacterAlpha>() |
							    SceneGraph::Flag<E_RenderPass_Sky>() |
						 	 	SceneGraph::Flag<E_RenderPass_TwoDimensional>() |
								SceneGraph::Flag<E_RenderPass_EffectDecal>() | 
								SceneGraph::Flag<E_RenderPass_Ghost>() |
								SceneGraph::Flag<E_RenderPass_VehicleAlpha>() |
								SceneGraph::Flag<E_RenderPass_AlphaVehicleFlares>() |
								SceneGraph::Flag<E_RenderPass_Shadows>() |
								SceneGraph::Flag<E_RenderPass_RefractAlpha>() |
								SceneGraph::Flag<E_RenderPass_Racification>()
								);
	// set view type to be the main view, plus view index offset
	m_pMain->SetType(E_ViewType_Main1 + gameView.GetIndex());
	// disable gathering of lights since we do it manually
	m_pMain->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_DynamicLight>());
	m_pMain->DisableCullMethod(SceneGraph::E_CullMethod_Portal);
	m_pMain->Initialise();
	m_pMain->ReserveGroups(2);
	m_pMain->GetGroup(0)->SetName("MainView");
	m_pMain->GetGroup(1)->SetName("MainViewAlpha");

	// add a seperate view for the prepass to allow for a shorter draw distance
	m_pMainPrepass = m_pViewManager->GetSceneGraph()->AddView();
#if PLATFORM_PC
	// [Blur2PC] Retail PC prepass view also gathers vehicles and ghosts (mask 0x80e5, Blur.exe 1.2 0x01f03efa),
	// scheduled only when m_PrepassVehicles is set by the graphical detail preset.
	m_pMainPrepass->EnableRenderPass(	SceneGraph::Flag<E_RenderPass_Solid>() |
											SceneGraph::Flag<E_RenderPass_Punch>() |
											SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
											SceneGraph::Flag<E_RenderPass_VehicleSolid>() |
											SceneGraph::Flag<E_RenderPass_AnimatedObjectSolid>() |
											SceneGraph::Flag<E_RenderPass_Ghost>());
#else
	m_pMainPrepass->EnableRenderPass(	SceneGraph::Flag<E_RenderPass_Solid>() |
											SceneGraph::Flag<E_RenderPass_Punch>() |
											SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
											SceneGraph::Flag<E_RenderPass_AnimatedObjectSolid>());
#endif

	m_pMainPrepass->SetTypeFlag(E_ViewType_Main1 + gameView.GetIndex(), E_ViewTypeFlag_Prepass + (1<<(E_ViewType_Main1+gameView.GetIndex())));

	// disable gathering of lights, dynamic effect, aux flares
	m_pMainPrepass->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_DynamicLight>());
	m_pMainPrepass->DisableRenderNode(m_pViewManager->m_pDynamicEffectRenderer->GetNodeFlag());
	m_pMainPrepass->DisableRenderNode(m_pViewManager->m_pAuxFlares->GetNodeFlag());

	m_pMainPrepass->DisableCullMethod(SceneGraph::E_CullMethod_Portal);
	m_pMainPrepass->Initialise();
	m_pMainPrepass->ReserveGroups(1);
	m_pMainPrepass->GetGroup(0)->SetName("PrepassView");

	m_PrepassSettings.SetSubtechnique(m_pViewManager->GetPrepassSubtechniqueHandle());
	// in split screen we change the number of workers per task and also task priorities
	if(m_pViewManager->GetNumViews() > 1)
	{
		m_PrepassSettings.SetNumWorkers(2);
		m_PrepassSettings.SetPriority(SceneGraph::E_RenderPriority_High);

		m_MainWithAASettings.SetNumWorkers(2);
		m_MainWithAASettings.SetPriority(SceneGraph::E_RenderPriority_High);

		m_MainWithoutAASettings.SetNumWorkers(2);

		m_MainWithAASettings.SetSubtechnique(m_pViewManager->GetLightsSplitScreenSubtechniqueHandle());
		m_MainWithAASettings.SetDistanceSubtechnique(m_pViewManager->GetLODSubtechniqueHandle());
	}
	else
	{
		m_MainWithAASettings.SetSubtechnique(m_pViewManager->GetLightsSubtechniqueHandle());
	}

	FX::LockSerialiseMutex();
	m_MirrorLinear = FX::GetSamplerState(
		Display::E_TextureAddress_Mirror, Display::E_TextureAddress_Mirror, Display::E_TextureAddress_Mirror,
		Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
		1, 0, 0);

	m_ClampPoint = FX::GetSamplerState(
		Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
		Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
		1, 0, 0);

	m_ClampLinear = FX::GetSamplerState(
		Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
		Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
		1, 0, 0);
	FX::ReleaseSerialiseMutex();

	// black texture
	{
		if (!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_BlackTexture))
			return;

		Display::C_LockedRect rect = m_BlackTexture.Lock(0);
		if(rect.pBits)
		{
			U32 *pData = reinterpret_cast<U32*>(rect.pBits);
			*pData = 0x00000000;
		}
		m_BlackTexture.Unlock(0);

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_BlackTexture, "_PrepassDisableBlack");
		#endif
	}
}

void C_MainView::Destroy()
{
	m_pViewManager->GetSceneGraph()->RemoveView(m_pMain);
	m_pViewManager->GetSceneGraph()->RemoveView(m_pMainPrepass);

	Display::Device()->Destroy(&m_BlackTexture);
}

Debug::Bool g_MainEnable("Amax.Views.MainEnable", true );
Debug::Bool g_MainFreeze("Amax.Views.MainFreeze", false );


Debug::Float g_MainPrepassDrawDistance("Amax.Views.Main.PrepassDrawDistance", 250.0f, Debug::Range(0.0f, 4000.0f));
Debug::Bool g_MainPrepassShow("Amax.Views.Main.PrepassShow",  false );
Debug::Float g_MainInstanceDrawDistanceScaleMin("Amax.Views.Main.InstanceDrawDistanceScaleMin", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Float g_MainInstanceDrawDistanceScaleMax("Amax.Views.Main.InstanceDrawDistanceScaleMax", 1.0f, Debug::Range(0.0f, 1.0f));
Debug::Float g_MainModelInstanceDrawDistanceScaleMin("Amax.Views.Main.ModelInstanceDrawDistanceScaleMin", 0.5f, Debug::Range(0.0f, 1.0f));
Debug::Float g_MainModelInstanceDrawDistanceScaleMax("Amax.Views.Main.ModelInstanceDrawDistanceScaleMax", 1.0f, Debug::Range(0.0f, 1.0f));
Debug::Bool g_MainDistanceLodSubtechique("Amax.Views.Main.DistanceLodSubtechique",  false );
Debug::Float g_MainDistanceLodDistance("Amax.Views.Main.DistanceLodDistance", 250.0f, Debug::Range(0.0f, 4000.0f));
Debug::Bool g_MainUseChaseCameraSplashEffects("Amax.Views.Main.MainUseChaseCameraSplashEffects", true);

void C_MainView::UpdateView(const C_View& gameView)
{
	const Framework::C_View* pFrameworkView = gameView.GetFrameworkView();
	if(!g_MainFreeze)
	{
	Display::C_TileSetup* pTileSetup = NULL;

	// if we are using tiling then tell the main view to calculate predication masks for us
	
	m_pMain->SetViewData(pFrameworkView->m_View.GetViewSettings3D());
	m_pMain->SetViewFrustum(pFrameworkView->m_View.GetViewFrustum(), pTileSetup);
	m_pMain->SetInstanceDrawDistanceScale(m_InstanceDrawDistanceScale);
	m_pMain->SetModelInstanceDrawDistanceScale(m_ModelInstanceDrawDistanceScale);
	m_pMain->SetDistanceSubtechniqueHandle(g_MainDistanceLodSubtechique, g_MainDistanceLodDistance, m_pViewManager->GetLODSubtechniqueHandle());
	m_pMain->SetFutureCamera(AmaxCam::CameraSystem().FutureFeed(pFrameworkView->m_View.m_UserIndex).Live().Mat43());
	
	m_pMainPrepass->SetViewData(pFrameworkView->m_View.GetViewSettings3D());
	m_pMainPrepass->SetFarClip(g_MainPrepassDrawDistance);
	m_pMainPrepass->SetViewFrustum(pFrameworkView->m_View.GetViewFrustum());
	m_pMainPrepass->SetInstanceDrawDistanceScale(m_InstanceDrawDistanceScale);
	m_pMainPrepass->SetModelInstanceDrawDistanceScale(m_ModelInstanceDrawDistanceScale);
	m_pMainPrepass->SetPvsCell(m_pMain->GetPvsCell());

	const Cam2::C_Camera* pCamera;

	if (g_MainUseChaseCameraSplashEffects)
		pCamera = AmaxCam::CameraSystem().VehicleFeed(pFrameworkView->m_View.m_UserIndex)->ChaseCamera();
	else
		pCamera = &pFrameworkView->DebugFeed().Live();

	if (!C_GameApp::GetInstance()->IsPaused())
		m_pViewManager->m_pGameWorld->GetSplashCamera()->Update(pFrameworkView->m_View.m_UserIndex + E_ViewType_Main1, C_GameApp::GetInstance()->GetTimeStep(), pCamera->Mat43(), pCamera->Velocity());
	}
	//Update lightcubemap to splash camera effects
	//Get current car
 
   	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
   	if (pRLC)
   	{
   		S8 vehicleIndex = pRLC->GetViewportFocusVehicleIndex(pFrameworkView->m_View.m_UserIndex);
   		if (vehicleIndex >= 0)
   		{
 			if (pRLC->GetVehicleActor(vehicleIndex))
   				m_pViewManager->m_pGameWorld->GetSplashCamera()->SetLightCubeTexture(pFrameworkView->m_View.m_UserIndex + E_ViewType_Main1, pRLC->GetVehicleActor(vehicleIndex)->GetCurrentLightCubeMap());
   		}
   		else
   			m_pViewManager->m_pGameWorld->GetSplashCamera()->SetLightCubeTexture(pFrameworkView->m_View.m_UserIndex + E_ViewType_Main1, NULL);
   	}
   	else
   		m_pViewManager->m_pGameWorld->GetSplashCamera()->SetLightCubeTexture(pFrameworkView->m_View.m_UserIndex + E_ViewType_Main1, NULL);


// 	if (m_pViewManager->GetCubeMap())
//  	{
//  		m_pViewManager->m_pGameWorld->GetSplashCamera()->SetLightCubeTexture(pFrameworkView->m_View.m_UserIndex + E_ViewType_Main1, const_cast<Display::C_TextureCube*>(m_pViewManager->GetCubeMap()->GetRenderTexture()));
//  	}
//  	else
//  		m_pViewManager->m_pGameWorld->GetSplashCamera()->SetLightCubeTexture(pFrameworkView->m_View.m_UserIndex + E_ViewType_Main1, NULL);
}

void C_MainView::SetDynamicDetail(F32 detail)
{
	m_InstanceDrawDistanceScale = Maths::Lerp(g_MainInstanceDrawDistanceScaleMin, g_MainInstanceDrawDistanceScaleMax, detail);
	m_ModelInstanceDrawDistanceScale = Maths::Lerp(g_MainModelInstanceDrawDistanceScaleMin, g_MainModelInstanceDrawDistanceScaleMax, detail);
}

#if PLATFORM_PC
} // namespace Game
namespace SceneGraph { EXTERN_DEBUG_FLOAT(g_fModelLODDistanceScale) }
namespace Game
{

// [Blur2PC] Retail PC graphical detail for the main view, ported from Blur.exe 1.2 0x00425450.
void C_MainView::SetGraphicalDetail(const U32 graphicalDetail, const bool inUI)
{
	// instance min, model instance min, instance max, model instance max, model LOD distance
	static const F32 s_Settings[4][5] =
	{
		{ 0.1f,  0.2f, 0.25f, 0.4f,  200.0f },
		{ 0.25f, 0.4f, 0.5f,  0.6f,  450.0f },
		{ 0.5f,  0.6f, 0.75f, 0.8f,  800.0f },
		{ 0.75f, 0.8f, 1.0f,  1.0f, 1000.0f },
	};

	m_PrepassVehicles = false;

	if(graphicalDetail >= 1 && graphicalDetail <= 4)
	{
		const F32* pSettings = s_Settings[graphicalDetail - 1];
		g_MainInstanceDrawDistanceScaleMin = pSettings[0];
		g_MainModelInstanceDrawDistanceScaleMin = pSettings[1];
		g_MainInstanceDrawDistanceScaleMax = pSettings[2];
		g_MainModelInstanceDrawDistanceScaleMax = pSettings[3];
		SceneGraph::g_fModelLODDistanceScale = pSettings[4];

		m_PrepassVehicles = graphicalDetail == 4;
		m_LightsSubtechniqueHandle = graphicalDetail == 4 ? m_pViewManager->GetLightsSubtechniqueHandle() : m_pViewManager->GetLightsSplitScreenSubtechniqueHandle();
		m_LowDetailShadows = graphicalDetail == 1;
		g_MainPrepassEnable = graphicalDetail != 1;
	}

	if(inUI)
	{
		m_PrepassVehicles = true;
		g_MainModelInstanceDrawDistanceScaleMax = 1.0f;
	}

	m_InstanceDrawDistanceScale = g_MainInstanceDrawDistanceScaleMax;
	m_ModelInstanceDrawDistanceScale = g_MainModelInstanceDrawDistanceScaleMax;
}
#endif

void C_MainView::Gather(C_View& gameView)
{
	if(!g_MainEnable)
		return;

	PERF_EVENT("Render.GatherMain");
	
	//Change the view flags in Photomode to make all visible
	if (AmaxCam::CameraSystem().IsPhotoCamera())
	{	
		//Not in splitscreen
		m_pMain->SetType(E_ViewType_Main2);
		m_pMainPrepass->SetTypeFlag(E_ViewType_Main2, E_ViewTypeFlag_Prepass + (1<<(E_ViewType_Main2)));
	}
	else
	{
		//Normal
		m_pMain->SetType(E_ViewType_Main1 + gameView.GetIndex());
		m_pMainPrepass->SetTypeFlag(E_ViewType_Main1 + gameView.GetIndex(), E_ViewTypeFlag_Prepass + (1<<(E_ViewType_Main1+gameView.GetIndex())));
	}

	UpdateView(gameView);

	gameView.m_pMultiView->AddView(m_pMain);
	gameView.m_pMultiView->AddView(m_pMainPrepass);
}

void C_MainView::SetupPrepassLightSpheres() const
{
	// gather lights manaually
	m_pViewManager->GetSceneGraph()->GetLightManager()->Gather(*m_pMain);

	// add light spheres to prepass view 

	// pointlights
	const SceneGraph::C_VisiblePointLightList& visiblePointLights = m_pMain->GetVisiblePointLightList();
	for(S32 i = 0; i < visiblePointLights.Size(); ++i)
	{
		m_pMainPrepass->AddLightSphere(visiblePointLights[i].m_pObject->GetBoundingSphere());
	}

	// spotlights
	const SceneGraph::C_VisibleSpotLightList& visibleSpotLights = m_pMain->GetVisibleSpotLightList();
	for(S32 i = 0; i < visibleSpotLights.Size(); ++i)
	{
		m_pMainPrepass->AddLightSphere(visibleSpotLights[i].m_pObject->GetBoundingSphere());
	}

	// dropshadows
	const SceneGraph::C_VisibleDropShadowList& visibleDropShadows = m_pMain->GetVisibleDropShadowsList();
	for(S32 i = 0; i < visibleDropShadows.Size(); ++i)
	{
		m_pMainPrepass->AddLightSphere(visibleDropShadows[i].m_pObject->GetBoundingSphere());
	}
}

Debug::Int	g_MainReduceRenderMinScreenSize("Amax.Views.Main.ReduceRenderMinScreenSize", 2000);

void C_MainView::Schedule(const C_View& gameView)
{
	if(!g_MainEnable)
		return;

	PERF_EVENT("Render.ScheduleMain");

#if PLATFORM_PS3
	m_MainWithAASettings.SetNumWorkers(g_NumMainViewWorkers);
#endif

	const S32 viewIndex = gameView.GetIndex();
	const Framework::C_View* pFrameworkView = gameView.GetFrameworkView();

	const SceneGraph::C_VisibleNodeList* pAlpha = m_pMain->GetVisibleNodeList(E_RenderPass_Alpha);
	m_ReduceRenderAlpha = m_pViewManager->GetReduceRenderAlpha();

	// the overhead for reduced rendering means its not worth doing it unless we have a reasonable amount of draw in the alpha pass
	if(m_ReduceRenderAlpha != Render::Reduced::E_Downsample_None && pAlpha->m_ScreenSize < (U32)g_MainReduceRenderMinScreenSize && !g_MainReduceRenderOverride)
		m_ReduceRenderAlpha = Render::Reduced::E_Downsample_None;

	{
		PERF_EVENT("Render.SetFxParams");

		// set exposure params
		const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
		if (Render::C_RenderState::Instance().ShaderHDR())
		{
			const Display::C_Float4	psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters(viewIndex);
			FX::SetParameter(g_FxParams.luminanceParameter, &psLuminance);
		}

		
		{
			if (gameView.GetReflection() && gameView.GetReflection()->GetRenderTexture())
			{
				const void* pReflectionTexAddress = gameView.GetReflection()->GetRenderTexture()->GetTexture().GetGPUTexture();
			#if PLATFORM_PC
				// [Blur2PC] Retail binds the black texture while the reflection is off (Blur.exe 1.2 0x0042a170).
				if (!gameView.GetReflection()->IsGraphicalDetailEnabled() || !g_ReflectionEnable)
					pReflectionTexAddress = m_BlackTexture.GetGPUTexture();
			#endif
				FX::SetParameter(g_FxParams.screenReflectionTexture, &pReflectionTexAddress);
				FX::SetSamplerState(g_FxParams.screenReflectionTexture, m_MirrorLinear);
			}
			if (gameView.GetReflection() && gameView.GetReflection()->GetBlurRenderTexture())
			{
				const void* pReflectionTexAddress = gameView.GetReflection()->GetBlurRenderTexture()->GetTexture().GetGPUTexture();
			#if PLATFORM_PC
				if (!gameView.GetReflection()->IsGraphicalDetailEnabled() || !g_ReflectionEnable)
					pReflectionTexAddress = m_BlackTexture.GetGPUTexture();
			#endif
				FX::SetParameter(g_FxParams.blurScreenReflectionTexture, &pReflectionTexAddress);
				FX::SetSamplerState(g_FxParams.blurScreenReflectionTexture, m_ClampLinear);
			}
		}
		if(m_pViewManager->m_pLightingPrepass)
		{
			{
				if (m_pViewManager->m_pLightingPrepass->GetNormalTexture(pFrameworkView->m_ViewIndex) && m_pViewManager->m_pLightingPrepass->GetNormalTexture(pFrameworkView->m_ViewIndex)->GetGPUTexture())
				{
					const void* pNormalBufferAddress = m_pViewManager->m_pLightingPrepass->GetNormalTexture(pFrameworkView->m_ViewIndex)->GetGPUTexture();
					FX::SetParameter(g_FxParams.normalBuffer, &pNormalBufferAddress);
					FX::SetSamplerState(g_FxParams.normalBuffer, m_ClampPoint);
				}

			}

			{
				if (m_pViewManager->m_pLightingPrepass->GetLightingTexture(pFrameworkView->m_ViewIndex) && m_pViewManager->m_pLightingPrepass->GetLightingTexture(pFrameworkView->m_ViewIndex)->GetGPUTexture())
				{
				#if PLATFORM_PC
					// [Blur2PC] retail 0x0042a170: the light buffer only outside the front end (mInUI, T115)
					if(g_PrepassEnable && !C_Game::GetInstance()->GetLevelState().mInUI)
				#else
					if(g_PrepassEnable)
				#endif
					{
						const void* pLightBufferAddress = m_pViewManager->m_pLightingPrepass->GetLightingTexture(pFrameworkView->m_ViewIndex)->GetGPUTexture();
						FX::SetParameter(g_FxParams.dynamicLights, &pLightBufferAddress);
						FX::SetSamplerState(g_FxParams.dynamicLights, m_ClampPoint);

						if (m_pViewManager->m_pCrowdFlagRenderer)
							m_pViewManager->m_pCrowdFlagRenderer->SetDynamicLightingTexture(pLightBufferAddress);
					}
					else
					{
						/* set it to the black texture */
						const void* pLightBufferAddress = m_BlackTexture.GetGPUTexture();

						FX::SetParameter(g_FxParams.dynamicLights, &pLightBufferAddress);
						FX::SetSamplerState(g_FxParams.dynamicLights, m_ClampPoint);

						if (m_pViewManager->m_pCrowdFlagRenderer)
							m_pViewManager->m_pCrowdFlagRenderer->SetDynamicLightingTexture(pLightBufferAddress);
					}
				}
			}
		}

		const U32 widthInPixels = pFrameworkView->m_View.GetViewport().m_W;
		const U32 heightInPixels = pFrameworkView->m_View.GetViewport().m_H;
		const MathsStorage::Float4 vHalfPixelOffset = MathsStorage::Float4(0.5f / widthInPixels, 0.5f / heightInPixels, 0.0f, 0.0f);
		FX::SetParameter(g_FxParams.halfPixelOffset, &vHalfPixelOffset);

		if (m_pViewManager->m_pCrowdFlagRenderer)
			m_pViewManager->m_pCrowdFlagRenderer->SetHalfPixelOffset(widthInPixels, heightInPixels);

		// set previous bloom buffer in-car window rendering
		if (m_pViewManager->m_pPostProcessBloom)
		{
			const S32 iView = m_pViewManager->m_pPostProcessManager->GetViewIndex(pFrameworkView->m_View.GetIndex());
			const Display::C_Texture2D* pFeedbackTexture = m_pViewManager->m_pPostProcessBloom->GetFeedbackTexture(iView);
			const void* pTexture = pFeedbackTexture ? pFeedbackTexture->GetGPUTexture() : NULL;

			FX::SetParameter(g_FxParams.bloomBuffer, &pTexture);
			FX::SetSamplerState(g_FxParams.bloomBuffer, m_ClampLinear);
		}

		//Set reflection view matrix
		if(gameView.GetReflection())
		{
			Display::C_Matrix reflectionMatrix = gameView.m_pReflection->GetMatrix();
			
			FX::SetParameter(g_FxParams.reflectionViewMatrix0, Display::C_PSConstant(reflectionMatrix._11,reflectionMatrix._21, reflectionMatrix._31, reflectionMatrix._41));
			FX::SetParameter(g_FxParams.reflectionViewMatrix1, Display::C_PSConstant(reflectionMatrix._12,reflectionMatrix._22, reflectionMatrix._32, reflectionMatrix._42));
			FX::SetParameter(g_FxParams.reflectionViewMatrix3, Display::C_PSConstant(reflectionMatrix._14,reflectionMatrix._24, reflectionMatrix._34, reflectionMatrix._44));

			// set the reflection plane for the main view
			FX::SetParameter(g_FxParams.planeReflection, &gameView.GetReflection()->GetPlane());
		}

		// set near far clip
		FX::SetParameter(g_FxParams.nearFar, Display::C_PSConstant(gameView.GetMainView().GetViewSettings3D().GetNearClip(), gameView.GetMainView().GetViewSettings3D().GetFarClip(), gameView.GetMainView().GetViewSettings3D().GetNearClip(), g_MainPrepassDrawDistance));

		// set pixel shader view constants
		const Display::C_Float4& vViewPosition = gameView.GetMainView().GetViewSettings3D().GetViewPosition();
		const Display::C_Matrix& mView = gameView.GetMainView().GetViewSettings3D().GetViewMatrix();
		const Display::C_Matrix& mViewProj = gameView.GetMainView().GetViewSettings3D().GetViewProjMatrix();
		FX::SetParameter(g_FxParams.viewPositionPS, &vViewPosition);
		FX::SetParameter(g_FxParams.viewMatrix0, Display::C_PSConstant(mView._11, mView._21, mView._31, mView._41));
		FX::SetParameter(g_FxParams.viewMatrix1, Display::C_PSConstant(mView._12, mView._22, mView._32, mView._42));
		FX::SetParameter(g_FxParams.viewMatrix2, Display::C_PSConstant(mView._13, mView._23, mView._33, mView._43));
		FX::SetParameter(g_FxParams.viewMatrix3, Display::C_PSConstant(mView._14, mView._24, mView._34, mView._44));
		FX::SetParameter(g_FxParams.viewProjMatrix0, Display::C_PSConstant(mViewProj._11, mViewProj._21, mViewProj._31, mViewProj._41));
		FX::SetParameter(g_FxParams.viewProjMatrix1, Display::C_PSConstant(mViewProj._12, mViewProj._22, mViewProj._32, mViewProj._42));
		FX::SetParameter(g_FxParams.viewProjMatrix2, Display::C_PSConstant(mViewProj._13, mViewProj._23, mViewProj._33, mViewProj._43));
		FX::SetParameter(g_FxParams.viewProjMatrix3, Display::C_PSConstant(mViewProj._14, mViewProj._24, mViewProj._34, mViewProj._44));

		if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore())
		{
			S8 vehicleIndex = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetViewportFocusVehicleIndex(viewIndex);
			Game::Actors::C_VehicleActor* pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetVehicleActor(vehicleIndex);
			if (pActor)
			{
				//const Display::C_Float4 vHeroPosition = Display::C_Float4(pActor->GetMatrix().AW().X(), pActor->GetMatrix().AW().Y(), pActor->GetMatrix().AW().Z(), 1.f);
				FX::SetParameter(g_FxParams.heroCarPos, Display::C_PSConstant(pActor->GetMatrix().AW().X(), pActor->GetMatrix().AW().Y(), pActor->GetMatrix().AW().Z(), 1.f));
			}
		} 
		m_pViewManager->SetLightFlareParameters();
	}

	// we must set a solid blend mode here to set the colour write enable based on luminance writes
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	S32 totalLights = m_pMain->GetVisiblePointLightList().Size() + m_pMain->GetVisibleSpotLightList().Size() + m_pMain->GetVisibleDropShadowsList().Size();
#if PLATFORM_PC
	// [Blur2PC] retail Blur PC's C_MainView::Schedule (Blur.exe 1.2 0x0042a170) schedules the prepass under
	// Amax.Views.Main.PrepassEnable (0x0111e020) alone, whatever the light count, as RenderPrepass1 draws it
	// (T108). With no visible light the snapshot scheduled nothing and the depth target kept its far-plane
	// clear, which the blob shadows, fog and depth fades read (found under T115).
	(void)totalLights;
	bool prepassEnable = g_PrepassEnable;
#else
	bool prepassEnable = g_PrepassEnable && totalLights != 0;
#endif

	if(prepassEnable)
	{
		SceneGraph::RenderFlags renderFlags;
		renderFlags.setStencilState = 0;
		m_pViewManager->GetSceneGraph()->SetRenderFlags(renderFlags);

		if(g_MainPrepassShow)
			m_PrepassSettings.SetWindow(viewIndex, true);
		else
			m_PrepassSettings.SetColourDepthStencilSurface(NULL, NULL);
		
		SceneGraph::C_RenderGroup* pGroup = m_pMainPrepass->BeginSchedule(0, pFrameworkView->m_View, &m_PrepassSettings);
		pGroup->SetName("PrepassView");

		g_PrepassSolidRenderStates.colourWriteEnable = Display::E_ColourWriteEnable_All;

		g_PrepassPunchRenderStates.setBlendMode = true;
		g_PrepassPunchRenderStates.alphaTestEnable = Display::E_Bool_False;
		g_PrepassPunchRenderStates.alphaToMaskEnable = Display::E_Bool_False;
		g_PrepassPunchRenderStates.alphaRef = 0x80;
		g_PrepassPunchRenderStates.blendEnable = Display::E_Bool_False;
		g_PrepassPunchRenderStates.blendOp = Display::E_BlendOp_Add;
		g_PrepassPunchRenderStates.destBlend = Display::E_Blend_Zero;
		g_PrepassPunchRenderStates.srcBlend = Display::E_Blend_One;
		g_PrepassPunchRenderStates.renderFlagsFX.setAlphaToMask = 0;
		g_PrepassPunchRenderStates.renderFlagsFX.setBlendState = 0;
		g_PrepassPunchRenderStates.renderFlagsRD.setPass = 0;
		g_PrepassPunchRenderStates.colourWriteEnable = Display::E_ColourWriteEnable_All;
		
#if PLATFORM_PC
		// [Blur2PC] Retail PC main view schedule (Blur.exe 1.2 0x0042aa73): vehicles and ghosts in the prepass.
		if(m_PrepassVehicles)
		{
			if(g_SceneGraphRenderVehicleSolid)
				m_pMainPrepass->SchedulePass(E_RenderPass_VehicleSolid);
			if(g_SceneGraphRenderGhost)
				m_pMainPrepass->SchedulePass(E_RenderPass_Ghost, &m_pViewManager->GetGhostDepthStates());
		}
#endif
		if(g_SceneGraphRenderSolid)
			m_pMainPrepass->SchedulePass(E_RenderPass_Solid, &g_PrepassSolidRenderStates);
		if(g_SceneGraphRenderDynamicObjectSolid)
			m_pMainPrepass->SchedulePass(E_RenderPass_DynamicObjectSolid, &g_PrepassSolidRenderStates);
		if(g_SceneGraphRenderAnimatedObjectSolid)
			m_pMainPrepass->SchedulePass(E_RenderPass_AnimatedObjectSolid, &g_PrepassSolidRenderStates);
		if(g_SceneGraphRenderPunch && g_SceneGraphRenderPunchPrepass)
			m_pMainPrepass->SchedulePass(E_RenderPass_Punch, &g_PrepassPunchRenderStates);
		
		m_pMainPrepass->EndSchedule();

		renderFlags.setStencilState = 1;
		m_pViewManager->GetSceneGraph()->SetRenderFlags(renderFlags);
	}

	// don't schedule main if we are showing prepass
	if(!g_MainPrepassShow)
	{
	#if PLATFORM_PC
		// [Blur2PC] The lights subtechnique is the one the graphical detail preset picked (retail stores it in
		// m_MainWithAASettings once); kept at 0 while the prepass is off as the snapshot does.
		m_MainWithAASettings.SetSubtechnique(g_PrepassEnable ? m_LightsSubtechniqueHandle : 0);
	#endif
		
		m_MainWithAASettings.SetWindow(viewIndex, true);

		if(g_PunchPassRenderState == E_AlphaPassRenderState_Default)
		{
			g_MainPunchRenderStates.setBlendMode = false;
			g_MainPunchRenderStates.renderFlagsFX.setAlphaToMask = 1;
			g_MainPunchRenderStates.renderFlagsFX.setBlendState = 1;
			g_MainPunchRenderStates.renderFlagsRD.setPass = 1;
			g_MainPunchRenderStates.colourWriteEnable = Display::E_ColourWriteEnable_All;
		}
		else if(g_PunchPassRenderState == E_AlphaPassRenderState_PunchThrough)
		{
			g_MainPunchRenderStates.setBlendMode = true;
			g_MainPunchRenderStates.alphaTestEnable = Display::E_Bool_True;
			g_MainPunchRenderStates.alphaToMaskEnable = Display::E_Bool_False;
			g_MainPunchRenderStates.alphaRef = g_AlphaPassAlphaRef;
			g_MainPunchRenderStates.blendEnable = Display::E_Bool_False;
			g_MainPunchRenderStates.blendOp = Display::E_BlendOp_Add;
			g_MainPunchRenderStates.destBlend = Display::E_Blend_Zero;
			g_MainPunchRenderStates.srcBlend = Display::E_Blend_One;
			g_MainPunchRenderStates.renderFlagsFX.setAlphaToMask = 0;
			g_MainPunchRenderStates.renderFlagsFX.setBlendState = 0;
			g_MainPunchRenderStates.renderFlagsRD.setPass = 0;			
		}
		else if(g_PunchPassRenderState == E_AlphaPassRenderState_AlphaDither)
		{
			g_MainPunchRenderStates.setBlendMode = true;
			g_MainPunchRenderStates.alphaTestEnable = Display::E_Bool_True;
			g_MainPunchRenderStates.alphaToMaskEnable = Display::E_Bool_True;
			g_MainPunchRenderStates.alphaRef = 0;
			g_MainPunchRenderStates.blendEnable = Display::E_Bool_False;
			g_MainPunchRenderStates.blendOp = Display::E_BlendOp_Add;
			g_MainPunchRenderStates.destBlend = Display::E_Blend_Zero;
			g_MainPunchRenderStates.srcBlend = Display::E_Blend_One;
			g_MainPunchRenderStates.renderFlagsFX.setAlphaToMask = 0;
			g_MainPunchRenderStates.renderFlagsFX.setBlendState = 0;
			g_MainPunchRenderStates.renderFlagsRD.setPass = 0;
		}
		else if(g_PunchPassRenderState == E_AlphaPassRenderState_Alpha)
		{
			g_MainPunchRenderStates.setBlendMode = true;
			g_MainPunchRenderStates.alphaTestEnable = Display::E_Bool_True;
			g_MainPunchRenderStates.alphaToMaskEnable = Display::E_Bool_False;
			g_MainPunchRenderStates.alphaRef = 0;
			g_MainPunchRenderStates.blendEnable = Display::E_Bool_True;
			g_MainPunchRenderStates.blendOp = Display::E_BlendOp_Add;
			g_MainPunchRenderStates.destBlend = Display::E_Blend_InvSrcAlpha;
			g_MainPunchRenderStates.srcBlend = Display::E_Blend_SrcAlpha;
			g_MainPunchRenderStates.renderFlagsFX.setAlphaToMask = 0;
			g_MainPunchRenderStates.renderFlagsFX.setBlendState = 0;
			g_MainPunchRenderStates.renderFlagsRD.setPass = 0;
		}

		m_pMain->BeginSchedule(E_RenderGroup_MainWithAA, pFrameworkView->m_View, &m_MainWithAASettings);
		
		if(g_SceneGraphRenderVehicleSolid)
			m_pMain->SchedulePass(E_RenderPass_VehicleSolid);
		if(g_SceneGraphRenderCharacterSolid)
			m_pMain->SchedulePass(E_RenderPass_CharacterSolid);
		if(g_SceneGraphRenderCharacterPunch)
			m_pMain->SchedulePass(E_RenderPass_CharacterPunch, &g_MainPunchRenderStates);
		if(g_SceneGraphRenderSolid)
			m_pMain->SchedulePass(E_RenderPass_Solid);
		if(g_SceneGraphRenderDynamicObjectSolid)
			m_pMain->SchedulePass(E_RenderPass_DynamicObjectSolid);
		if(g_SceneGraphRenderAnimatedObjectSolid)
			m_pMain->SchedulePass(E_RenderPass_AnimatedObjectSolid);
		if(g_SceneGraphRenderAnimatedObjectPunch)
			m_pMain->SchedulePass(E_RenderPass_AnimatedObjectPunch, &g_MainPunchRenderStates);
		if(g_SceneGraphRenderPunch)
			m_pMain->SchedulePass(E_RenderPass_Punch, &g_MainPunchRenderStates);
		if(g_SceneGraphRenderSky)
			m_pMain->SchedulePass(E_RenderPass_Sky);
		if(g_SceneGraphRenderDecal)
			m_pMain->SchedulePass(E_RenderPass_Decal);
		if(g_SceneGraphRenderEffectDecal)
			m_pMain->SchedulePass(E_RenderPass_EffectDecal);
		if(g_SceneGraphRenderRacification)
			m_pMain->SchedulePass(E_RenderPass_Racification);
		if(g_SceneGraphRenderGhost)
		{
			m_pMain->SchedulePass(E_RenderPass_Ghost, &m_pViewManager->GetGhostDepthStates());
			m_pMain->SchedulePass(E_RenderPass_Ghost, &m_pViewManager->GetGhostColourStates());
		}
		if(g_SceneGraphRenderCharacterAlpha)
			m_pMain->SchedulePass(E_RenderPass_CharacterAlpha);
		if(g_SceneGraphRenderVehicleAlpha)
			m_pMain->SchedulePass(E_RenderPass_VehicleAlpha);
		
		m_pMain->EndSchedule();
	}

	if(g_SceneGraphRenderAlpha || g_SceneGraphRenderAlphaVehicleFlares)
	{
		const C_LightFlareSettings carSettings = m_pViewManager->m_pGameWorld->GetCarLightFlareSettings();
		m_pViewManager->m_pGameWorld->GetAuxFlares()->SetViewSettings(E_ViewType_Main1 + gameView.GetIndex(), carSettings.m_GlobalSize * carSettings.m_GlobalSizeX, carSettings.m_GlobalSize * carSettings.m_GlobalSizeY, carSettings.m_GlobalColour);
			
		if(m_ReduceRenderAlpha == Render::Reduced::E_Downsample_None)
		{
			if(PostProcess::g_debugPostProcessEnable && m_pViewManager->m_pPostProcessManager->IsRendererCreated())
			{
				const PostProcess::C_RenderTarget& renderTarget = m_pViewManager->m_pPostProcessManager->GetRenderTargetMapped(pFrameworkView->m_View.GetIndex());
				m_MainWithoutAASettings.SetColourDepthStencilSurface(&renderTarget.m_RenderTextures[0]->GetSurface(), renderTarget.m_pDepthStencilSurface);
			}
			else
			{
				m_MainWithoutAASettings.SetWindow(viewIndex, false);
			}
		}
		else
		{
			S32 index = pFrameworkView->m_View.GetIndex();
			const Display::C_Surface* pColourSurface = m_pViewManager->m_pGameWorld->GetReducedRender()->GetColourSurface(index, m_ReduceRenderAlpha);
			const Display::C_Surface* pDepthStencilSurface = m_pViewManager->m_pGameWorld->GetReducedRender()->GetDepthStencilSurface(index, m_ReduceRenderAlpha);
			m_MainWithoutAASettings.SetColourDepthStencilSurface(pColourSurface, pDepthStencilSurface);
		}
		m_MainWithoutAASettings.SetColourDepthStencilTexture(Render::GetGlobalColourBuffer(), Render::GetGlobalDepthStencilBuffer());
#if PLATFORM_PC
		m_MainWithoutAASettings.SetStencilTexture(Render::GetGlobalStencilBuffer());	// [Blur2PC] retail 0x0042aeb0 (T089)
#endif
		
		//Prepare Blob shadows
		m_pViewManager->m_pBlobShadows->SetDepthTexture(Render::GetGlobalDepthStencilBuffer());

		//Prepare DropShadows
		if (g_debugEnableDropShadows
#ifdef PLATFORM_PS3
			&& (m_pViewManager->GetNumViews() < 3) //Disabled in 3/4 splitscreen
#endif
			)
		{
			const S32 displayViewIndex = m_pMain->GetDisplayView()->GetIndex();
			const SceneGraph::C_VisibleDropShadowList& visibleDropShadows = m_pMain->GetVisibleDropShadowsList();
			for (S32 i = 0; i < visibleDropShadows.Size(); ++i)
			{
				const SceneGraph::C_VisibleDropShadow& visibleDropShadow = visibleDropShadows[i];

				MathsStorage::Float3	position	= visibleDropShadow.m_pObject->m_Position;
				F32						radius		= visibleDropShadow.m_pObject->m_Radius;
				F32						intensity	= visibleDropShadow.m_pObject->m_ShadowIntensity * visibleDropShadow.m_Intensity;

				m_pViewManager->m_pDropShadows->AddShadow(displayViewIndex, position, radius, intensity);
			}
		}

		if(g_AlphaPassRenderState == E_AlphaPassRenderState_Default)
		{
			g_MainAlphaRenderStates.setBlendMode = false;
			g_MainAlphaRenderStates.renderFlagsFX.setAlphaToMask = 1;
			g_MainAlphaRenderStates.renderFlagsFX.setBlendState = 1;
			g_MainAlphaRenderStates.renderFlagsRD.setPass = 1;
		}
		else if(g_AlphaPassRenderState == E_AlphaPassRenderState_PunchThrough)
		{
			g_MainAlphaRenderStates.setBlendMode = true;
			g_MainAlphaRenderStates.alphaTestEnable = Display::E_Bool_True;
			g_MainAlphaRenderStates.alphaToMaskEnable = Display::E_Bool_False;
			g_MainAlphaRenderStates.alphaRef = g_AlphaPassAlphaRef;
			g_MainAlphaRenderStates.blendEnable = Display::E_Bool_False;
			g_MainAlphaRenderStates.blendOp = Display::E_BlendOp_Add;
			g_MainAlphaRenderStates.destBlend = Display::E_Blend_Zero;
			g_MainAlphaRenderStates.srcBlend = Display::E_Blend_One;
			g_MainAlphaRenderStates.renderFlagsFX.setAlphaToMask = 0;
			g_MainAlphaRenderStates.renderFlagsFX.setBlendState = 0;
			g_MainAlphaRenderStates.renderFlagsRD.setPass = 0;			
		}
		else if(g_AlphaPassRenderState == E_AlphaPassRenderState_AlphaDither)
		{
			g_MainAlphaRenderStates.setBlendMode = true;
			g_MainAlphaRenderStates.alphaTestEnable = Display::E_Bool_True;
			g_MainAlphaRenderStates.alphaToMaskEnable = Display::E_Bool_True;
			g_MainAlphaRenderStates.alphaRef = g_AlphaPassAlphaRef;
			g_MainAlphaRenderStates.blendEnable = Display::E_Bool_False;
			g_MainAlphaRenderStates.blendOp = Display::E_BlendOp_Add;
			g_MainAlphaRenderStates.destBlend = Display::E_Blend_Zero;
			g_MainAlphaRenderStates.srcBlend = Display::E_Blend_One;
			g_MainAlphaRenderStates.renderFlagsFX.setAlphaToMask = 0;
			g_MainAlphaRenderStates.renderFlagsFX.setBlendState = 0;
			g_MainAlphaRenderStates.renderFlagsRD.setPass = 0;
		}
		else if(g_AlphaPassRenderState == E_AlphaPassRenderState_Alpha)
		{
			g_MainAlphaRenderStates.setBlendMode = true;
			g_MainAlphaRenderStates.alphaTestEnable = Display::E_Bool_True;
			g_MainAlphaRenderStates.alphaToMaskEnable = Display::E_Bool_False;
			g_MainAlphaRenderStates.alphaRef = 0;
			g_MainAlphaRenderStates.blendEnable = Display::E_Bool_True;
			g_MainAlphaRenderStates.blendOp = Display::E_BlendOp_Add;
			g_MainAlphaRenderStates.destBlend = Display::E_Blend_InvSrcAlpha;
			g_MainAlphaRenderStates.srcBlend = Display::E_Blend_One;
			g_MainAlphaRenderStates.renderFlagsFX.setAlphaToMask = 0;
			g_MainAlphaRenderStates.renderFlagsFX.setBlendState = 0;
			g_MainAlphaRenderStates.renderFlagsRD.setPass = 0;
		}

		SceneGraph::RenderFlags renderFlags;
		if(m_ReduceRenderAlpha != Render::Reduced::E_Downsample_None)
			renderFlags.setColourWrite = 0;
		m_pViewManager->GetSceneGraph()->SetRenderFlags(renderFlags);
		
		m_pMain->BeginSchedule(E_RenderGroup_MainWithoutAA, pFrameworkView->m_View, &m_MainWithoutAASettings);
		if (g_SceneGraphRenderShadows)
			m_pMain->SchedulePass(E_RenderPass_Shadows);
		if(g_SceneGraphRenderAlpha)
			m_pMain->SchedulePass(E_RenderPass_Alpha, &g_MainAlphaRenderStates);
		if(g_SceneGraphRenderAlphaVehicleFlares)
			m_pMain->SchedulePass(E_RenderPass_AlphaVehicleFlares);
		m_pMain->EndSchedule();

		renderFlags.setColourWrite = 1;
		m_pViewManager->GetSceneGraph()->SetRenderFlags(renderFlags);
	}
}

void C_MainView::RenderWithAA(C_View& view)
{
	if(!g_MainEnable)
		return;

	m_pMain->BeginRender();

	Display::Device()->InvalidateState();
	Display::Device()->DefaultStates();


	S32 totalLights = m_pMain->GetVisiblePointLightList().Size() + m_pMain->GetVisibleSpotLightList().Size() + m_pMain->GetVisibleDropShadowsList().Size();
	bool prepassEnable = g_PrepassEnable && totalLights != 0;

	if(g_MainPrepassShow)
	{
		if(prepassEnable)		
			m_pMainPrepass->RenderGroup(0);
	}
	else
	{
		DEVICE_MARKER("RenderSplitscreenWithAA.MainWithAA");
		m_pMain->RenderGroup(E_RenderGroup_MainWithAA);
	}
}

Debug::Bool	g_MainRestoreZCull("Amax.Views.Main.RestoreZCull", true);
Debug::Int	g_MainRestoreZCullScreenSize("Amax.Views.Main.RestoreZCullScreenSize", 1000);

void C_MainView::RenderWithoutAA(C_View& gameView)
{
	if(!g_MainEnable)
		return;

#ifdef PLATFORM_PS3
	// on PS3 we restore Z cull if theres a decent amount of stuff on screen in the alpha pass and we haven't already restored it
	const SceneGraph::C_VisibleNodeList* pAlpha = m_pMain->GetVisibleNodeList(E_RenderPass_Alpha);
	if(pAlpha->m_ScreenSize > g_MainRestoreZCullScreenSize && g_MainRestoreZCull && !g_debugDisplayRestoreZcull)
		Display::Device()->ReloadZCull(false);
#endif

	// if more than 1 view we need to set the view matrices etc again
	if(m_pViewManager->GetNumViews() != 1)
		m_pViewManager->GetSceneGraph()->SetSharedParameters(*Display::Device()->GetView());

#if defined(PLATFORM_PS3)
	// [StephenM] Turn depth reads back on after resolving MSAA buffer
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
#endif

	DEVICE_MARKER("RenderSplitscreenWithoutAA.Advertising");
	Advertising::Render(&gameView.m_pFrameworkView->m_View);

	// render the rest of the scene
	if(g_SceneGraphRenderRefract)
	{
		PERF_EVENT("Render.Refract");

		U32 refractionSubTechnique = 0;
		if (m_pViewManager->GetNumViews() > 1)
			refractionSubTechnique = m_pViewManager->GetLightsSplitScreenSubtechniqueHandle();

		DEVICE_MARKER("RenderSplitscreenWithoutAA.Refract");
		m_pMain->RenderPass(E_RenderPass_Refract, NULL, refractionSubTechnique);

		DEVICE_MARKER("RenderSplitscreenWithoutAA.RefractAlpha");
		m_pMain->RenderPass(E_RenderPass_RefractAlpha, NULL, refractionSubTechnique);
	}
	if(g_SceneGraphRenderAlpha || g_SceneGraphRenderAlphaVehicleFlares)
	{
		PERF_EVENT("Render.Alpha");

		S32 index = gameView.GetFrameworkView()->m_View.GetIndex();
		
		if(m_ReduceRenderAlpha != Render::Reduced::E_Downsample_None)
		{
			Display::Device()->InvalidateState();
			Display::Device()->SetTexture(1, 0); // ensure that depth texture gets set
			#if PLATFORM_PS3
			if(m_pViewManager->GetNumViews() == 1)
			{
				m_pViewManager->m_pReducedRender->SetDepth(index, m_ReduceRenderAlpha, Display::Device()->GetBackBuffer().GetDepthStencilTexture());
			}
			else
			#endif
			{
				m_pViewManager->m_pReducedRender->SetDepth(index, m_ReduceRenderAlpha, Render::GetGlobalDepthStencilBuffer());
			}
			m_pViewManager->m_pReducedRender->Begin(index, m_ReduceRenderAlpha);
		}

#if PLATFORM_PC
		// [Blur2PC] Retail PC detail 1 (blob shadows off): each car visible in this view gets its
		// collision-mesh shadow drawn here (Blur.exe 1.2 0x00427120, same call as the mirror's shadows).
		if(m_LowDetailShadows && (g_SceneGraphRenderAlpha || g_SceneGraphRenderAlphaVehicleFlares))
		{
			Display::Device()->InvalidateState();
			Game::Actors::RaceLogicCore* pRLC = m_pViewManager->m_pAmaxWorldLayer->GetRaceLogicCore();
			for(U32 iVehicle = 0; iVehicle < pRLC->GetNumVehicles(); iVehicle++)
			{
				Game::Actors::C_VehicleActor* pVehicleActor = const_cast<Game::Actors::C_VehicleActor*>(pRLC->GetVehicleActor(iVehicle));
				if(pVehicleActor && pVehicleActor->GetVisibleDetect() && (pVehicleActor->GetVisibleDetect()->flag & m_pMain->GetTypeFlag()))
				{
					const Btl::Vector<Maths::HVec3>& collisionVertices = pVehicleActor->GetCollisionCacheVertices();
					if(collisionVertices.Size() > 0)
						m_pViewManager->m_pBlobShadows->RenderCollisionMeshShadow(pVehicleActor->GetMatrix(), pVehicleActor->GetGroundPosition(), 1.f, &collisionVertices[0], collisionVertices.Size());
				}
			}
		}
#endif
		if(g_SceneGraphRenderAlpha || g_SceneGraphRenderAlphaVehicleFlares)
		{
			DEVICE_MARKER("RenderSplitscreenWithoutAA.MainWithoutAA");
			m_pMain->RenderGroup(E_RenderGroup_MainWithoutAA);
		}

		if(m_ReduceRenderAlpha != Render::Reduced::E_Downsample_None)
		{
			DEVICE_MARKER("RenderSplitscreenWithoutAA.ReducedRender");
			Display::Device()->InvalidateState();
			m_pViewManager->m_pReducedRender->End();

			if(g_MainReduceRenderCompositeToneMap && PostProcess::g_debugPostProcessEnable)
				m_pViewManager->m_pPostProcessHdr->SetReducedAlphaTexture(m_pViewManager->m_pReducedRender->GetTexture(index, m_ReduceRenderAlpha));
			else
			{
				m_pViewManager->m_pPostProcessHdr->SetReducedAlphaTexture(NULL);
				m_pViewManager->m_pReducedRender->Render(index, m_ReduceRenderAlpha, Render::Reduced::E_BlendMode_Alpha);
			}
		}
		else
			m_pViewManager->m_pPostProcessHdr->SetReducedAlphaTexture(NULL);
	}

	if(g_SceneGraphRenderTwoDimensional)
	{
		PERF_EVENT("Render.TwoDimensional");
		
		Display::Device()->InvalidateState(); // I hate putting stuff like this in here, but it fixes another bug...

#if PLATFORM_PS3
		// HACK - this is to fix an intermittent GPU hang where the display state is incorrect
		//        after coming out of the renderingdata
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
#endif

		S32 index = gameView.GetFrameworkView()->m_View.GetIndex();

		if(g_MainReduceRender2D != Render::Reduced::E_Downsample_None)
			m_pViewManager->m_pReducedRender->Begin(index, g_MainReduceRender2D);
		
		DEVICE_MARKER("RenderSplitscreenWithoutAA.TwoDimensional");
		m_pMain->RenderPass(E_RenderPass_TwoDimensional);
		
		if(g_MainReduceRender2D != Render::Reduced::E_Downsample_None)
		{
			m_pViewManager->m_pReducedRender->End();
			m_pViewManager->m_pPostProcessHdr->SetLensEffectTexture(m_pViewManager->m_pReducedRender->GetTexture(index, g_MainReduceRender2D));
		}
		else
			m_pViewManager->m_pPostProcessHdr->SetLensEffectTexture(0);
	}
	else if(!g_SceneGraphFreezeTwoDimensional)
		m_pViewManager->m_pPostProcessHdr->SetLensEffectTexture(0);

	m_pMain->EndRender();

	// render prepass
	m_pViewManager->m_pLightingPrepass->Show(gameView.GetIndex() + 1);
}

void C_MainView::RenderPrepass1(C_View& gameView)
{
	if(g_MainPrepassShow || !g_PrepassEnable || !m_pViewManager->m_pLightingPrepass)
		return;

	PERF_EVENT("Render.RenderPrepass");

	{
		CPU_TIMING(5, Maths::RGBA(255, 128, 255, 255), "CPU.RenderPrepass");
		GPU_TIMING(5, Maths::RGBA(255, 128, 255, 255), "GPU.RenderPrepass");

		Display::Device()->InvalidateState();

		S32 totalLights = m_pMain->GetVisiblePointLightList().Size() + m_pMain->GetVisibleSpotLightList().Size() + m_pMain->GetVisibleDropShadowsList().Size();

		m_pViewManager->m_pLightingPrepass->Begin();
		
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);

#if PLATFORM_PC
		// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x00426390) draws the prepass whatever the light count: its
		// depth target feeds the depth-fade effects, blob shadows and fog as well as the lights
		(void)totalLights;
		if(true)
#else
		if(totalLights != 0)
#endif
		{
			m_pMainPrepass->BeginRender();
			m_pMainPrepass->RenderGroup(0);
			m_pMainPrepass->EndRender();

			Display::Device()->InvalidateState();
		}
 
		m_pViewManager->m_pLightingPrepass->End();
		
	}

#if defined(PLATFORM_PC)
	// Since the depth texture (m_pDepthTexture) is set as RenderTarget #1, we cannot safely use it as an input texture, so
	// we must reset the appropriate target...
	
	 ID3D11DeviceContext* context = nullptr;
	 Display::Device()->GetD3DDevice()->GetImmediateContext(&context);
	 if (context)
	 {
		 ID3D11RenderTargetView* colour = nullptr;
		 ID3D11DepthStencilView* depth = nullptr;
		 context->OMGetRenderTargets(1, &colour, &depth);
		 context->OMSetRenderTargets(1, &colour, depth);
		 if (colour) colour->Release();
		 if (depth) depth->Release();
		 context->Release();
	 }

	 // We also need to unbind any other textures used as render targets...
	 //  - DX release will not report a problem but results are undefined.
	 //    DX debug gives a warning message from 'DrawPrimitive' about not being able to render to a target which is used as a texture.

	 Display::Device()->SetTexture(13, NULL); 			 

#endif

#if PLATFORM_PC
	// [Blur2PC] retail 0x00426390 renders the lights only outside the front end (mInUI); the prepass depth
	// above is drawn in the front end too (T115)
	if(C_Game::GetInstance()->GetLevelState().mInUI)
		return;
#endif

	CPU_TIMING(5, Maths::RGBA(255, 255, 128, 255), "CPU.RenderLights");
	GPU_TIMING(5, Maths::RGBA(255, 255, 128, 255), "GPU.RenderLights");
 	m_pViewManager->m_pDropShadows->SetFarClip(g_MainPrepassDrawDistance);
	m_pViewManager->m_pLightingPrepass->SetFarClip(g_MainPrepassDrawDistance);
	m_pViewManager->m_pLightingPrepass->RenderLights1(m_pMain->GetVisiblePointLightList(), m_pMain->GetVisibleSpotLightList());
}

void C_MainView::RenderPrepass2(C_View& gameView)
{
	if(g_MainPrepassShow || !g_PrepassEnable || !m_pViewManager->m_pLightingPrepass)
		return;
#if PLATFORM_PC
	// [Blur2PC] retail 0x00554c10: no lights in the front end (mInUI, T115)
	if(C_Game::GetInstance()->GetLevelState().mInUI)
		return;
#endif

	PERF_EVENT("Render.RenderPrepass");
	CPU_TIMING(5, Maths::RGBA(255, 255, 128, 255), "CPU.RenderLights");
	GPU_TIMING(5, Maths::RGBA(255, 255, 128, 255), "GPU.RenderLights");
	m_pViewManager->m_pLightingPrepass->RenderLights2(m_pMain->GetVisiblePointLightList(), m_pMain->GetVisibleSpotLightList());
}

} // namespace Game
