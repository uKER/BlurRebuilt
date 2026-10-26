#include "stdafx.h"
#include <system/debug/Blur2PCAutomation.h>
#include <system/time/scopedtimer.h>
#include <OverlayFs/OverlayFs.h>
#include <GameFs/GameHelpers.h>
#include <PakFs/PakFsManager.h>

#define RENDERINGDATA_ALLOW_INCLUDE // just for GameWorld.cpp because it deals directly with the renderingdata library

#include "system/time/simpletimer.h"

#include <btl/containers/List.h>
#include <btl/containers/BitVector.h>

#include <fx/Fx.h>
#include <fx/RenderMutex.h>
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

#define SCENEGRAPH_ALLOW_INCLUDE // just for GameWorld.cpp because it sets up the SceneGraph
#include <scenegraph/SceneGraphImpl.h>
#include <scenegraph/Visible/BackFrontSorter.h>
#include <scenegraph/Visible/FrontBackSorter.h>
#include <scenegraph/Visible/DepthSorter.h>
#include <SceneGraph/RenderNode.h>
#include <scenegraph/view.h>
#include <scenegraph/light/Manager.h>

#include <Lod/LodManager.h>

#include "renderingdata/renderingdata.h"

#include <game_resource/LevelResource.h>
#include <game_resource/LevelSceneManager.h>

#include <Game/GameWorld/GameWorld.h>
#include <Game/GameDef/Materials.h>
#include <Game/GameDef/UserData.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

#include <Render/View/ViewManager.h>
#include <Render/ReducedRender/Manager.h>

#include <GameApp.h>
#include <SchedulerConfig.h>
#include <render/debug/DebugRender.h>

#include <Graphics/Lighting/Prepass.h>
#include <Graphics/CutSceneLights.h>
#include <Graphics/Wind.h>
#include <BlobShadows/BlobShadows.h>
#include <DropShadows/DropShadows.h>
#include <AuxFlares/AuxFlares.h>
#include <LightTrails/LightTrails.h>

#include <Dynamic/Renderer.h>
#include <Dynamic/Light/Manager.h>
#include <Dynamic/Light/Generic/XtInterface.h>
#include <Dynamic/Particle/Manager.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/UserParticleLibrary.h>
#include <Dynamic/Track/Track.h>
#include <Dynamic/Track/XtInterface.h>

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared/Messaging/MessageTypeList.h>
#include <misc_shared/Messaging/GameMessaging.h>

#include <world_render/LightList.h>
#include <game/AmaxCam/CameraSystem.h>
#include <Game/AmaxCam/CameraSwitcher.h>

#include <Game/GameDef/Polyflags.h>
#include <Game/GameDef/PhysicsConstants.h>

#include <racingphysics/racingphysics.h>
#include <racingphysics/debug/draw.h>
#include <racingphysics/debug/colour.h>
#include <racingphysics/physics/surfaces.h>

#include <Vehicle/VehicleModelManager.h>
#include <Vehicle/VehicleModel.h>
#include <game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <Game/GameWorld/GameWorldContextProvider.h>

#include "Game\Messaging\gamemessages.h"
#include "Game\Messaging\GameflowMessages.h"
#include <Game/Game.h>
#include <AI/AI_Amax.h>

#include <Fx/EnvironmentSettings.h>
#include <xt/xtgame.h>
#include <Game/GameWorld/EnvironmentSettings.h>
#include <Effect\Sky\TexturedSkyDome.h>
#include <Game\Messaging\VehicleActorMessages.h>

#include "Framework/Application/ApplicationBase.h"

#include <GameLogic\RaceIntroOutro\RaceIntroOutro.h>

#include <shared_actors/SharedActorUtils.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/utility/VisibilityDetection.h>
#include <shared_actors/Components/Impl/AnimatedObjectComponents.h>

#include <crowd_system/CrowdSystem.h>
#include <crowd_system/Effects/CrowdFlags.h>
#include <game/GameActors/Crowds/CrowdGameActor.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/Drivers/DriverManager.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_PlayerRubberBanding.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>

#include <Ui/UiLayer.h>
#include "Ui/Game/UiPhotoMode.h"
#include <UI/UIMessages.h>

#include <animation/task/TaskManager.h>
#include <animated_model/TaskManager.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <game_debug/ShaderDebugToggles.h>

#include <Game/GameWorld/ViewType.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View/CubeMap.h>
#include <Game/GameWorld/View/Mirror.h>
#include <Game/GameWorld/View/Reflection.h>

#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>
#include <Game/GameUtil/ResourceBag.h>

#include "Automation\GamePerfLogging.h"
#include "Game\GameUtil\LoadingPerf.h"
#include "Game\PerfStats\PerfStats.h"

#include <Graphics/HDR.h>

#include <SplashCamera/SplashCamera.h>

#include "GameAudio/PickupSubsystem.h"
#include "GameAudio/UISubsystem.h"
#include "GameAudio/GameAudioLayer.h"

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#if PLATFORM_PS3
#include "vehicle\damage\SPU\SPUDamageContext.h"
#include <PS3Installer/PS3InstallData.h>
#endif

#include <Display/Device/DeviceMarkers.h>


#include <system/serialise/I_FileSystem.h>

#include <Effect/PostProcess/MotionBlur/MotionBlurReduced.h>
#include <GameLogic/GameData/GameEffectData.h>
#include <Camera2/stock/DebugFeed.h>

Debug::Bool g_debugPhysicsPlane("Game.EnableGroundPlane", false);
Debug::Float g_debugPhysicsPlaneY("Game.DebugPlaneY", 1.0f, Debug::Range(0.f, 1.f));
Debug::Bool g_useFutureCamera("Game.Streaming.UseFutureCamera", true);
Debug::Bool g_useStreamBehind("Game.Streaming.StreamBehind", true);
Debug::Bool g_bRenderEverythingInCubemap("Amax.Views.CubemapRenderEverything", false);

Debug::Bool g_PunchInstancesFirst("Amax.Views.Punch.InstancesFirst", false);
Debug::Bool g_PunchPrimitivesFrontBack("Amax.Views.Punch.PrimitivesFrontBack", false);
Debug::Bool g_PunchInstancesFrontBack("Amax.Views.Punch.InstancesFrontBack", false);
Debug::Bool g_PunchModelInstancesFrontBack("Amax.Views.Punch.ModelInstancesFrontBack", false);

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

namespace Lod
{
	extern Debug::Bool g_LodEnabled;
}

#if CFG_PROFILE
Debug::Int g_maxPhysicsUpdates("Game.MaxPhysicsUpdates", 2);
#else
Debug::Int g_maxPhysicsUpdates("Game.MaxPhysicsUpdates", 4);
#endif

#if !CFG_GOLD
Debug::Bool g_bDisableVehicleCubemaps("Vehicle.DisableCubemaps", false);
#endif

#if !CFG_GOLD
Debug::Bool g_debugDisableGlobalFade("Game.DisableGlobalFade", false);
#endif

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include "world_render/RenderLevel.h"

#include "Graphics/GpuMarkers.h"

#include "SpecialFx/AnimatedMesh.h"

extern Scheduler::jqModule CarDynamicEffectsTaskModule;
extern Scheduler::jqModule CarUpdateLightCubeMapTaskModule;

Debug::Logger &g_GameDbgMsgLog( Debug::GetLogger("amax.debugmessaging") );

namespace RacingPhysics
{
EXTERN_DEBUG_BOOL (g_displayDebugVehicle);
}

namespace GameWorld_Impl
{

// messages that get dispatched to all components in an actor regardless of I_ActorComponent::GetActorMessageTypeList()
const Messaging::C_MessageTypeRange g_ActorComponentGenericMessageTypes [] = 
{
	// message types handled by I_ActorComponent
	{GameMessaging::E_GameMsg_DebugRenderSpecific, GameMessaging::E_GameMsg_DebugRenderSpecific},

	// misc message types

	// add message types before this terminator!
	{INT_MAX, INT_MIN}
};

const Messaging::C_MessageTypeListFixed g_ActorComponentGenericMessageTypeList(g_ActorComponentGenericMessageTypes);

} // namespace GameWorld_Impl

namespace PostProcess
{
	extern Debug::Bool	g_debugPostProcessEnable;
	extern Debug::Bool g_debugMotionBlurDoF;
}

namespace Game
{

extern bool g_PrepassEnable;
#if PLATFORM_PC
extern Debug::Bool g_MainPrepassEnable;	// [Blur2PC] View/Main.cpp
#endif

#ifndef CFG_GOLD
Debug::Bool g_DebugGameWorldDebugActorRender("ActorDebug.DebugRender", false);
#endif
Debug::Bool g_RenderGravityScene( "Gravity.Debug.RenderScene", false );
Debug::Float g_GravityDrawDistance( "Gravity.Debug.Settings.DrawDistance", 50.0f, Debug::Range(0.0f, 500.0f));

Debug::Bool g_DynamicLightEnable("Amax.Lights.Enable", true);
Debug::Float g_DynamicLightScale("Amax.Lights.Scale", 1.0f, Debug::Range(0.0f, 8.0f));
Debug::Float g_LightMaskDistance("Amax.Lights.MaskDistance", 256.0f);
#if defined(PLATFORM_PC)
Debug::Float g_SpotlightScreenSizeStencilThreshold("Amax.Lights.SpotlightScreenSizeStencilThreshold", 0.7f, Debug::Range(0.0f, 1.0f));
#else
Debug::Float g_SpotlightScreenSizeStencilThreshold("Amax.Lights.SpotlightScreenSizeStencilThreshold", 8.0f, Debug::Range(0.0f, 1.0f));
#endif

Debug::Bool g_EnableHeadlights("Amax.Lights.Headlight.Enable", true);
Debug::Bool g_TiledHeadlights("Amax.Lights.Headlight.Tiled", false);

Debug::Float g_debugTextureHeadlightMinOffset("Amax.Lights.Headlight.MinOffset", 0.050f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debugTextureHeadlightMaxOffset("Amax.Lights.Headlight.MaxOffset", 0.400f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debugTextureHeadlightMinSigma("Amax.Lights.Headlight.MinSigma", 0.050f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debugTextureHeadlightMaxSigma("Amax.Lights.Headlight.MaxSigma", 0.500f, Debug::Range(0.0f, 1.0f));
Debug::Bool g_debugRecreateHeadlightTexture("Amax.Lights.Headlight.ReloadTexture", false);
Debug::Float g_fHeadlightTexturePow("Amax.Lights.Headlight.Pow", 1.f, Debug::Range(0.0f, 1.f));
Debug::Float g_fHeadlightVScalePow("Amax.Lights.Headlight.VPow", 1.f, Debug::Range(0.f, 1.f));
Debug::Float g_fHeadlightTextureOffset("Amax.Lights.Headlight.Offset", 0.f, Debug::Range(-0.1f, 0.f));

Debug::String g_HeadlightTextureFileName("Amax.Lights.Headlight.Filename", "graphics/textures/headlight.raw");

Debug::Bool g_FixedReflectionNormal("Amax.FixedReflectionNormal", true);

Debug::Bool g_DebugDynamicObjectsRender("DynamicObjects.Objects.Render",false);

Debug::Float g_DebugPlaybackRate("Replays.PlayBackRate", 1.0f, Debug::Range(0.1f, 10.0f));

Debug::Bool g_MakeSlowmoCaptureWork("Camera2.MakeSlowmoCaptureWork", false);
static Debug::Bool g_TimeStep50Hz("Game.TimeStep50Hz", false);
extern Debug::Bool g_debugStreamerBlockLoad;

#ifndef CFG_GOLD
//Reload specialFX bag
Debug::String g_ReloadSpecialFXBagResource("Amax.ReloadSpecialFX.Resource", "", Debug::Persist());
static void ReloadSpecialFXREsource(void*)
{
	C_GameApp::GetInstance()->GetSpecialFXResourceBag()->ReloadResource( MakeResourceBagTag(g_ReloadSpecialFXBagResource.Value()));
}
Debug::Function g_ReloadSpecialFXBagReload("Amax.ReloadSpecialFX.Reload", ReloadSpecialFXREsource, 0);
#endif
//#pragma optimize("", off)

extern bool g_SorceryRecordsModified;

GameWorldFxParams	g_FxParams;

C_GameWorld::C_GameWorld()
	: m_pPhysicsScene( NULL )
	, m_pSceneGraph( NULL )
	, m_pActorList( NULL )
//	, m_pActorUtils( NULL )
	, m_pContextProvider( NULL )
	, m_pLevelSceneManager( NULL )
	, m_pSorceryEnvironmentSettings( NULL )
	, m_TempBodyCollisionFilter( 0, PolyFlags::kDynamicObjectSolid )
	, m_pCrowdGameManager( NULL )
	, m_pCrowdSystem( NULL )
	, m_pCrowdFlagRenderer( NULL )
	, m_pViewManager( NULL )
	, m_GameFrame( 0 )
	, m_RaceLoaded(false)
	, m_cutsceneVideoPlaying(false)
	, m_allowedToStream(false)
	, m_waitForStreamer(false)
	, m_streamerStartedLoading(false)
	, m_streamerFinishedLoading(false)
	, m_startStreamer(false)
	, m_bCubeMapOverridden(false)
	, m_pSceneEnvMapTexture(NULL)
	, m_pSceneEnvMapGPUTexture(NULL)
	, m_pLightTrails(NULL)
	, m_letsGoRacing(false)
{
	//Can't get at this...
	//Framework::C_ApplicationBase::GetInstance()->m_DisplayDeviceParams
	//So...

	//Tasks

	//Create group
	m_CarDynamicEffectsTaskGroup = Scheduler::CreateGroup();
	m_CarUpdateLightCubeMapTaskGroup = Scheduler::CreateGroup();
	m_LogicActorUpdateGroup = Scheduler::CreateGroup();
	m_ImpactProcessGroup = Scheduler::CreateGroup();
	m_RaytraceGroup = Scheduler::CreateGroup();
	Scheduler::AddGroupDependency(m_CarUpdateLightCubeMapTaskGroup, m_CarDynamicEffectsTaskGroup);
	m_PickupSubsystemGroup = Scheduler::CreateGroup();
	m_VehicleAudioGroup = Scheduler::CreateGroup();

	Game::Actors::C_VehicleLogicActor::SetSchedulerGroup(m_LogicActorUpdateGroup);
	Game::Actors::C_VehicleActor::SetAudioSchedulerGroup(m_VehicleAudioGroup);
	Game::Actors::C_VehicleActor::SetRaytraceGroup(m_RaytraceGroup);
	PickUpSubsystem::SetSchedulerGroup(m_PickupSubsystemGroup);
	
	// get shared parameter handles
	FX::LockSerialiseMutex();
	g_FxParams.luminanceParameter = FX::GetSharedParameter("g_vLuminance");
	g_FxParams.luminanceTexture = FX::GetSharedParameter("g_tLuminance");
	g_FxParams.colourBuffer = FX::GetSharedParameter("g_tColourBuffer");
	g_FxParams.depthBuffer = FX::GetSharedParameter("g_tDepthBuffer");
	g_FxParams.depthScale = FX::GetSharedParameter("g_vDepthScale");
	g_FxParams.screenReflectionTexture = FX::GetSharedParameter("g_tScreenReflectionTexture");
	g_FxParams.blurScreenReflectionTexture = FX::GetSharedParameter("g_tBlurScreenReflectionTexture");
	g_FxParams.normalBuffer = FX::GetSharedParameter("g_tNormalBuffer");
	g_FxParams.dynamicLights = FX::GetSharedParameter("g_tDynamicLights");
	g_FxParams.bloomBuffer = FX::GetSharedParameter("g_tBloomBuffer");
	g_FxParams.vehicleEnvTex = FX::GetSharedParameter("g_sVehicleEnvironmentTexture");
	g_FxParams.cutSceneLightColour0 = FX::GetSharedParameter("g_vCutSceneLightColour0");
	g_FxParams.cutSceneLightColour1 = FX::GetSharedParameter("g_vCutSceneLightColour1");
	g_FxParams.cutSceneLightColour2 = FX::GetSharedParameter("g_vCutSceneLightColour2");
	g_FxParams.cutSceneLightColour3 = FX::GetSharedParameter("g_vCutSceneLightColour3");
	g_FxParams.cutSceneLightDir0 = FX::GetSharedParameter("g_vCutSceneLightDir0");
	g_FxParams.cutSceneLightDir1 = FX::GetSharedParameter("g_vCutSceneLightDir1");
	g_FxParams.cutSceneLightDir2 = FX::GetSharedParameter("g_vCutSceneLightDir2");
	g_FxParams.cutSceneLightDir3 = FX::GetSharedParameter("g_vCutSceneLightDir3");
	g_FxParams.time = FX::GetSharedParameter("g_fTime");
	g_FxParams.sunDirection = FX::GetSharedParameter("g_vSunDirection");
	g_FxParams.dynamicScale = FX::GetSharedParameter("g_fDynamicScale");
	g_FxParams.shockTexture = FX::GetSharedParameter("g_tShockTexture");
	g_FxParams.screenSettingsVS = FX::GetSharedParameter("g_vScreenSettingsVS");
	g_FxParams.screenSettingsPS = FX::GetSharedParameter("g_vScreenSettingsPS");
	g_FxParams.globalLightFlareSize = FX::GetSharedParameter("g_vGlobalLightFlareSize");
	g_FxParams.globalLightFlareColour = FX::GetSharedParameter("g_vGlobalLightFlareColour");
	g_FxParams.carGlobalLightFlareSize = FX::GetSharedParameter("g_vCarGlobalLightFlareSize");
	g_FxParams.carGlobalLightFlareColour = FX::GetSharedParameter("g_vCarGlobalLightFlareColour");
	g_FxParams.planeReflection = FX::GetSharedParameter("g_vPlaneReflection");
	g_FxParams.halfPixelOffset = FX::GetSharedParameter("g_vHalfPixelOffset");
	g_FxParams.bloomBuffer = FX::GetSharedParameter("g_tBloomBuffer");
	g_FxParams.reflectionViewMatrix0 = FX::GetSharedParameter("g_ReflectionViewMatrix0");
	g_FxParams.reflectionViewMatrix1 = FX::GetSharedParameter("g_ReflectionViewMatrix1");
	g_FxParams.reflectionViewMatrix3 = FX::GetSharedParameter("g_ReflectionViewMatrix3");
	g_FxParams.viewPositionPS = FX::GetSharedParameter("g_vViewPositionPS");
	g_FxParams.viewMatrix0 = FX::GetSharedParameter("g_vViewMatrix0");
	g_FxParams.viewMatrix1 = FX::GetSharedParameter("g_vViewMatrix1");
	g_FxParams.viewMatrix2 = FX::GetSharedParameter("g_vViewMatrix2");
	g_FxParams.viewMatrix3 = FX::GetSharedParameter("g_vViewMatrix3");
	g_FxParams.viewProjMatrix0 = FX::GetSharedParameter("g_vViewProjMatrix0");
	g_FxParams.viewProjMatrix1 = FX::GetSharedParameter("g_vViewProjMatrix1");
	g_FxParams.viewProjMatrix2 = FX::GetSharedParameter("g_vViewProjMatrix2");
	g_FxParams.viewProjMatrix3 = FX::GetSharedParameter("g_vViewProjMatrix3");
	g_FxParams.windMatrices = FX::GetSharedParameter("g_mWind");
	g_FxParams.leafMatrices = FX::GetSharedParameter("g_mLeaf");
	g_FxParams.nearFar = FX::GetSharedParameter("g_vNearFar");
	g_FxParams.nearFarLightFlare = FX::GetSharedParameter("g_vNearFarLightFlareColour");
	g_FxParams.heroCarPos = FX::GetSharedParameter("g_vHeroCarPos");

	g_FxParams.m_ClampLinear = FX::GetSamplerState(
		Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
		Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
		1, 0, 0);

	g_FxParams.m_WrapLinear = FX::GetSamplerState(
		Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap,
		Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
		1, 0, 0);
	
	g_FxParams.m_ClampPoint = FX::GetSamplerState(
		Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
		Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
		1, 0, 0);
	
	g_FxParams.m_MirrorPoint = FX::GetSamplerState(
		Display::E_TextureAddress_Mirror, Display::E_TextureAddress_Mirror, Display::E_TextureAddress_Mirror,
		Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, 
		1, 0, 0);
	FX::ReleaseSerialiseMutex();

	m_VehicleParticleEmitScale = 1.0f;
	m_VehicleParticleLifeScale = 1.0f;
	m_GlobalFade = 1.0f;			// Need to set this at start of each race 0.0f

	InitialiseEnvironmentSettingsParameters();

	m_bRunningCarEffectTasks = false;
#if PLATFORM_PC
	m_GraphicalDetail = 0; // [Blur2PC] set by SetGraphicalDetail at the end of Create
#endif

	m_HandleUISceneReady = false;
	m_paksSwitched = false;

	m_GameStep = 0;
	m_GameFraction = 0.f;

	m_MaxBlobShadowCasters = 6;

	m_pLightingPrepass = NULL;
}

C_GameWorld::~C_GameWorld()
{
	delete m_pSorceryEnvironmentSettings;
	delete m_pDropShadows;
	m_pBlobShadows->Destroy();

	delete m_pBlobShadows;
	
	Scheduler::DestroyGroup(m_CarDynamicEffectsTaskGroup);
	Scheduler::DestroyGroup(m_CarUpdateLightCubeMapTaskGroup);
	Scheduler::DestroyGroup(m_LogicActorUpdateGroup);
	Scheduler::DestroyGroup(m_ImpactProcessGroup);
	Scheduler::DestroyGroup(m_RaytraceGroup);
	Scheduler::DestroyGroup(m_PickupSubsystemGroup);
	Scheduler::DestroyGroup(m_VehicleAudioGroup);
}


void C_GameWorld::CreateEnvironmentSettingsFromSorcery(bool do_flashback, Game::EnvironmentSettings *override_settings)
{
	if (m_pSorceryEnvironmentSettings)
		delete m_pSorceryEnvironmentSettings;

	Game::EnvironmentSettings *pEnvSettings = NULL;

	if (override_settings)
	{
		pEnvSettings = override_settings;
	}
	else
	{
		AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity];
		if (pCity)
			pEnvSettings = &pCity->environmentSettings;

		if(C_Game::GetInstance()->GetLevelState().mEnvironmentOveride)
		{
			pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mEnvironmentOveride];
			if (pCity)
				pEnvSettings = &pCity->environmentSettings;
		}

		if(do_flashback)
			pEnvSettings = &pCity->flashbackSettings;
	}

	if (pEnvSettings)
	{
		m_pSorceryEnvironmentSettings = CreateEnvironmentSettings(pEnvSettings, m_pSky, m_pWind);
	}
	else
		m_pSorceryEnvironmentSettings = new FX::EnvironmentSettings;

	ResetEnvironmentSettings();
}

void C_GameWorld::ResetEnvironmentSettings()
{
	//Reset environment settings per view
	for (S32 i = 0; i < m_pViewManager->GetNumViews(); i++)
	{
		m_pViewManager->GetView(i)->ResetEnvironmentSettings(m_pSorceryEnvironmentSettings);
		
		FX::I_EnvironmentSettings* pEnvironmentSettings = m_pViewManager->GetView(i)->GetEnvironmentSettings();
		Game::PostFxEnvSettings* pPostFXEnvSettings = (Game::PostFxEnvSettings*)pEnvironmentSettings->GetSettingsBlockByName("PostFX");

#if !CFG_GOLD
		if (g_debugDisableGlobalFade)
			m_GlobalFade = 1.0f;
#endif
		if (pPostFXEnvSettings)
			pPostFXEnvSettings->m_ColourMatrix = pPostFXEnvSettings->m_ColourMatrix * Maths::MakeVF32(m_GlobalFade);
	}
	
	if (m_pSorceryEnvironmentSettings)
		m_pSorceryEnvironmentSettings->Set(); 
}

FX::I_EnvironmentSettings* C_GameWorld::GetEnvironmentSettings( S32 viewport )
{
	if(viewport >= 0 && viewport < m_pViewManager->GetNumViews())
		return m_pViewManager->GetView(viewport)->GetEnvironmentSettings();
	else
		return 0;
}

bool C_GameWorld::CreatePhysics()
{
	MEMORY_TRACKER_PUSH("gravity.application");
	C_PhysicsMaterials::SetupMaterials();
	Gravity::C_PhysicsSDK::PhysicsSDK()->SetShareVertexData(true);
	m_pPhysicsScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateScene( C_PhysicsMaterials::GetMaterialMatrix(), C_PhysicsMaterials::GetMaterialCount() );
	
	//Kier - reduce number of iterations in solver to try and speed solving up
	m_pPhysicsScene->SetNumSolverIterations(20);
	m_pPhysicsScene->SetJointIterationFrequency(2);
	m_PhysicsExternalConstants.m_Gravity = Maths::HVec3(0, -9.8f, 0);

	//m_pScene->RegisterCollisionCallback(&drawer);

	m_pPhysicsScene->SetInternalTaskPolicy( C_SchedulerConfig::GetInstance()->GetCollisionPolicy() );
	m_pPhysicsScene->SetInternalGroupHandle( C_SchedulerConfig::GetInstance()->GetGroup( E_SchedulerGroup_PhysicsInternal ) );
	m_pPhysicsScene->SetBroadPhaseGroupHandle( C_SchedulerConfig::GetInstance()->GetGroup( E_SchedulerGroup_PhysicsInternal2 ) );
	//m_pPhysicsScene->RegisterRaytracingTaskPolicy( C_SchedulerConfig::GetInstance()->GetGroup( E_SchedulerGroup_PhysicsRaytrace ) );

	MEMORY_TRACKER_POP("gravity.application");

	AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity];
	if (pCity && pCity->surfaceMappings)
	{
		RacingPhysics::Surfaces::RP_SurfaceMappingList* pSurfaceMappings = pCity->surfaceMappings.Object();
		RacingPhysics::Surfaces::BindSurfaceMappings(pCity->surfaceMappings.Record(), pSurfaceMappings);
	}
	else
		BIZ_ASSERT_FAIL("City has no RP_SurfaceMappingList setup! Will use default or previous settings!");

	//RacingPhysics::VehicleManager::Instance().Initialise();
	RacingPhysics::PhysicsTimer::Instance().InitialiseTimers();

	MEMORY_TRACKER_PUSH("application.ai");
	// intialise the AI
	AIAmax::AmaxAIR::Instance().Initialise(&C_Game::GetInstance()->GetSubscriberSystem());
	MEMORY_TRACKER_POP("application.ai");

	if (m_pPhysicsScene->GetTriggerSystem() && C_GameApp::GetInstance()->GetRaceBookLayer() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
	{
		m_pPhysicsScene->GetTriggerSystem()->SetCallback(&C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->m_TriggerEventCallback);
		m_pPhysicsScene->GetTriggerSystem()->DestroyLayers();
		for (S32 iLayer = 0; iLayer < E_TriggerLayer_Max; iLayer++)
		{
			//Hard-coded the world-bounds as those coming from the physics scene are total garbage...
			Gravity::C_TriggerLayer* pLayer = m_pPhysicsScene->GetTriggerSystem()->CreateLayer(	Maths::HVec3(-10000.0f, -5000.0f, -10000.0f), Maths::HVec3(10000.0f, 5000.0f, 10000.0f), 
																								Maths::HVec3(20.f, 20.f, 20.f), 10, iLayer);
		}
	}	

	return true;
}

//=========================================================================

bool C_GameWorld::CreateActorList( Actor::C_ActorFactory* pActorFactory )
{
//	const U32 maxNoActors = 10000;
	const U32 noMsgQueues = 8;

	if (m_pSceneGraph == NULL)
		return false;

	if (m_pPhysicsScene == NULL)
		return false;

	if(m_pLevelSceneManager == NULL)
		return false;

	m_pContextProvider = new C_GameWorldContextProvider(m_pSceneGraph, m_pLevelSceneManager, m_pPhysicsScene, &m_PhysicsExternalConstants, m_pLightEffectManager);

	m_pActorList = new Game::C_GameActorList(MAX_NUM_ACTORS, noMsgQueues, pActorFactory, m_pContextProvider);
	m_pActorList->AddComponentGenericMessageTypes(GameWorld_Impl::g_ActorComponentGenericMessageTypeList);
	::Actors::C_ActorUtils::Instance().SetGameWorldPtr(this);

	m_pVisibilityDetector = new ::Actors::C_VisibilityDetector( m_pActorList );
	
	return true;
}

bool C_GameWorld::CreateSceneGraph()
{
	MEMORY_TRACKER_PUSH("gravity.application.scenegraph");

	m_pSceneGraph = new SceneGraph::C_SceneGraph();

	m_pViewManager = new C_ViewManager(m_pSceneGraph, this);

	// set rendering policies
	m_pSceneGraph->SetJobQueuePolicyRender( C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy(),
											C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy(),
											C_SchedulerConfig::GetInstance()->GetCollisionPolicy() );
	m_pSceneGraph->SetJobQueuePolicyGather( C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy() );


	m_pBackFront = new SceneGraph::C_BackFrontSorter;
	m_pVehicleSorter = new SceneGraph::C_DepthSorter(SceneGraph::E_DepthSortOrder_FrontBack, SceneGraph::E_DepthSortMethod_PerType);
	m_pPunchDepthSorter = new SceneGraph::C_DepthSorter(SceneGraph::E_DepthSortOrder_FrontBack, SceneGraph::E_DepthSortMethod_PerType);

	// set render passes and sorting strategies etc
	m_pSceneGraph->SetNumRenderPasses(18);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Solid, NULL, m_pViewManager->GetSolidSubtechniqueHandle() + m_pViewManager->GetAlphaLuminanceSubtechniqueHandle());
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Punch, m_pPunchDepthSorter);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Decal);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_EffectDecal);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Refract);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Alpha, m_pBackFront);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_VehicleSolid, m_pVehicleSorter, m_pViewManager->GetAlphaLuminanceSubtechniqueHandle());
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_VehicleAlpha, m_pBackFront);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_CharacterSolid, NULL, 0,  m_pViewManager->GetLightsSubtechniqueHandle());
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_CharacterPunch, NULL, 0,  m_pViewManager->GetLightsSubtechniqueHandle());
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_CharacterAlpha, m_pBackFront);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_DynamicObjectSolid);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_AnimatedObjectSolid);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_AnimatedObjectPunch);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Sky, NULL);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_TwoDimensional);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Ghost);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_AlphaVehicleFlares);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Shadows);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_RefractAlpha, m_pBackFront);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Racification);
	
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_Solid);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, E_RenderPass_Punch);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OVERLAY, E_RenderPass_Decal);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_POST_RESOLVE, E_RenderPass_Refract);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, E_RenderPass_Alpha);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_RACIFICATION, E_RenderPass_Racification);

	// tell the following passes to work out predication
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Solid))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Punch))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Decal))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Racification))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_VehicleSolid))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_VehicleAlpha))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_CharacterSolid))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_CharacterPunch))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_CharacterAlpha))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_DynamicObjectSolid))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_AnimatedObjectSolid))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_AnimatedObjectPunch))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_Predication);
	
	// tell alpha pass to work out an estimated total screen size of gathered nodes. this allows us to reduce render if it goes high
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Alpha))
		pRenderPass->EnableVolatileInfoFlag(SceneGraph::E_VolatileNodeInfo_ScreenSize);

	// states to write stencil to mask motion blur
	m_NoMotionBlurStates.stencilState.m_StencilEnable		= Display::E_Bool_True;
	m_NoMotionBlurStates.stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	m_NoMotionBlurStates.stencilState.m_StencilRef			= 0xFF;
	m_NoMotionBlurStates.stencilState.m_StencilMask			= 0x00;
	m_NoMotionBlurStates.stencilState.m_StencilWriteMask	= 0xFF;
	m_NoMotionBlurStates.stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	m_NoMotionBlurStates.stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_NoMotionBlurStates.stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;

	m_NoMotionBlurSolidStates = m_NoMotionBlurStates;
	m_NoMotionBlurSolidStates.setBlendMode = true;
	m_NoMotionBlurSolidStates.blendEnable = Display::E_Bool_False;
	m_NoMotionBlurSolidStates.alphaTestEnable = Display::E_Bool_False;
	m_NoMotionBlurSolidStates.alphaRef = 0;
	m_NoMotionBlurSolidStates.alphaToMaskEnable = Display::E_Bool_False;
	m_NoMotionBlurSolidStates.blendHighPrecisionEnable = Display::E_Bool_False;
	m_NoMotionBlurSolidStates.blendOp = Display::E_BlendOp_Add;
	m_NoMotionBlurSolidStates.srcBlend = Display::E_Blend_One;
	m_NoMotionBlurSolidStates.destBlend = Display::E_Blend_Zero;
	m_NoMotionBlurSolidStates.blendOp = Display::E_BlendOp_Add;
	
	m_NoMotionBlurSolidStates.renderFlagsRD.setPass = 0;
		
	m_NoMotionBlurSolidStates.setDepth =true;
	m_NoMotionBlurSolidStates.depthWriteEnable = Display::E_Bool_True;
	m_NoMotionBlurSolidStates.depthEnable = Display::E_Bool_True;
	m_NoMotionBlurSolidStates.depthFunc = Display::E_CmpFunc_GreaterEqual;
	m_NoMotionBlurSolidStates.colourWriteEnable = Display::E_ColourWriteEnable_All;

	m_MotionBlurColourWriteAllStates.stencilState.m_StencilEnable		= m_MotionBlurStates.stencilState.m_StencilEnable		= Display::E_Bool_True;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilFunc			= m_MotionBlurStates.stencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilRef			= m_MotionBlurStates.stencilState.m_StencilRef		= 0x00;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilMask			= m_MotionBlurStates.stencilState.m_StencilMask		= 0x00;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilWriteMask	= m_MotionBlurStates.stencilState.m_StencilWriteMask	= 0xFF;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilFail			= m_MotionBlurStates.stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilDepthFail	= m_MotionBlurStates.stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_MotionBlurColourWriteAllStates.stencilState.m_StencilDepthPass	= m_MotionBlurStates.stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;

	// states for full colour write
	m_MotionBlurColourWriteAllStates.colourWriteEnable = m_ColourWriteAllStates.colourWriteEnable = Display::E_ColourWriteEnable_All;

	// no motion blur for vehicle, character
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_VehicleSolid))
		pRenderPass->SetRenderStates(&m_NoMotionBlurSolidStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_CharacterSolid))
		pRenderPass->SetRenderStates(&m_NoMotionBlurStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_CharacterPunch))
		pRenderPass->SetRenderStates(&m_NoMotionBlurStates);
		
	//motion blur for solids
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_DynamicObjectSolid))
		pRenderPass->SetRenderStates(&m_MotionBlurStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Punch))
		pRenderPass->SetRenderStates(&m_MotionBlurStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Decal))
		pRenderPass->SetRenderStates(&m_MotionBlurStates);
	//if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Racification))
	//	pRenderPass->SetRenderStates(&m_MotionBlurStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_AnimatedObjectSolid))
		pRenderPass->SetRenderStates(&m_MotionBlurStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_AnimatedObjectPunch))
		pRenderPass->SetRenderStates(&m_MotionBlurStates);

	// Set up render passes to be used by animated object actor. 
	Components::C_AnimatedModelComponent::SetSolidRenderPass(E_RenderPass_AnimatedObjectSolid);	// Custom render pass to exclude from cubemaps but include in prepass.
	Components::C_AnimatedModelComponent::SetPunchRenderPass(E_RenderPass_AnimatedObjectPunch);
	Components::C_AnimatedModelComponent::SetAlphaRenderPass(E_RenderPass_CharacterAlpha);		// Use character alpha to ensure correct sorting with car glass when cutscene characters are inside cars.

	// full colour writes for solid and sky - this isn't doing anything on Xenon atm
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Solid))
		pRenderPass->SetRenderStates(&m_MotionBlurColourWriteAllStates);
	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Sky))
		pRenderPass->SetRenderStates(&m_ColourWriteAllStates);

	m_RefractAlphaStates.setBlendMode = true;
	m_RefractAlphaStates.blendEnable = Display::E_Bool_False;
	m_RefractAlphaStates.alphaTestEnable = Display::E_Bool_False;
	m_RefractAlphaStates.alphaRef = 0;
	m_RefractAlphaStates.alphaToMaskEnable = Display::E_Bool_False;
	m_RefractAlphaStates.blendHighPrecisionEnable = Display::E_Bool_False;
	m_RefractAlphaStates.blendOp = Display::E_BlendOp_Add;
	m_RefractAlphaStates.srcBlend = Display::E_Blend_One;
	m_RefractAlphaStates.destBlend = Display::E_Blend_Zero;
	m_RefractAlphaStates.blendOp = Display::E_BlendOp_Add;
	
	m_RefractAlphaStates.renderFlagsFX.setBlendState = 0;
	m_RefractAlphaStates.renderFlagsRD.setPass = 0;
		
	m_RefractAlphaStates.setDepth =true;
	m_RefractAlphaStates.depthWriteEnable = Display::E_Bool_False;
	m_RefractAlphaStates.depthEnable = Display::E_Bool_True;
	m_RefractAlphaStates.depthFunc = Display::E_CmpFunc_GreaterEqual;

	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_RefractAlpha))
		pRenderPass->SetRenderStates(&m_RefractAlphaStates);

	// vehicle alpha overrides blend mode to be alpha with z writes enabled, and stencil to mask motion blur
	m_VehicleAlphaStates.renderFlagsRD.setPass = 0;
	m_VehicleAlphaStates.renderFlagsFX.setBlendState = 0;
	m_VehicleAlphaStates.colourWriteEnable = Display::E_ColourWriteEnable_Red|Display::E_ColourWriteEnable_Green|Display::E_ColourWriteEnable_Blue;
	m_VehicleAlphaStates.setBlendMode = true;
	m_VehicleAlphaStates.blendEnable = Display::E_Bool_True;
	m_VehicleAlphaStates.blendHighPrecisionEnable = Display::E_Bool_True;
	m_VehicleAlphaStates.srcBlend = Display::E_Blend_One;
	m_VehicleAlphaStates.destBlend = Display::E_Blend_InvSrcAlpha;
	m_VehicleAlphaStates.blendOp = Display::E_BlendOp_Add;
	m_VehicleAlphaStates.alphaToMaskEnable = Display::E_Bool_False;
	m_VehicleAlphaStates.alphaTestEnable = Display::E_Bool_True;
	m_VehicleAlphaStates.alphaRef = 0;
	m_VehicleAlphaStates.setDepth = true;
	m_VehicleAlphaStates.depthEnable = Display::E_Bool_True;
	m_VehicleAlphaStates.depthWriteEnable = Display::E_Bool_True;
	m_VehicleAlphaStates.depthFunc = Display::E_CmpFunc_GreaterEqual;
	m_VehicleAlphaStates.stencilState.m_StencilEnable		= Display::E_Bool_True;
	m_VehicleAlphaStates.stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	m_VehicleAlphaStates.stencilState.m_StencilRef			= 0xFF;
	m_VehicleAlphaStates.stencilState.m_StencilMask			= 0x00;
	m_VehicleAlphaStates.stencilState.m_StencilWriteMask	= 0xFF;
	m_VehicleAlphaStates.stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	m_VehicleAlphaStates.stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_VehicleAlphaStates.stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;

	if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_VehicleAlpha))
		pRenderPass->SetRenderStates(&m_VehicleAlphaStates);

	//Init SpecialFX
	SpecialFX::C_SpecialFXManager::SetInitParam(SpecialFX::E_InitParamID_PostResolveRenderPass, (void*)E_RenderPass_RefractAlpha);
	SpecialFX::C_SpecialFXManager::SetInitParam(SpecialFX::E_InitParamID_CubemapViewFlag, (void*)E_ViewTypeFlag_CubeMap);
	SpecialFX::C_SpecialFXManager::SetInitParam(SpecialFX::E_InitParamID_ReflectionViewFlag, (void*)E_ViewTypeFlag_Reflection);
	SpecialFX::C_SpecialFXManager::SetInitParam(SpecialFX::E_InitParamID_MirrorViewFlag, (void*)E_ViewTypeFlag_Mirror);

	MEMORY_TRACKER_POP("gravity.application.scenegraph");

	return true;
}

Debug::Bool g_debugEnableMirrorView("Amax.Views.CreateMirrorViewTheReturn", true);

bool C_GameWorld::CreateViews(const S32 views)
{
	bool enableMirror;
	bool enableReflection;
	bool enableCubeMap;

	switch(views)
	{
	case 2:
		if(GetHDR())
			GetHDR()->SetFastLuminance(false);
		PostProcess::g_debugMotionBlurDoF = false;
		enableMirror = false;
		enableReflection = false;
		enableCubeMap = false;
		break;
	case 3:
		if(GetHDR())
			GetHDR()->SetFastLuminance(true);
		PostProcess::g_debugMotionBlurDoF = false;
		enableMirror = false;
		enableReflection = false;
		enableCubeMap = false;
		break;
	case 4:
		if(GetHDR())
			GetHDR()->SetFastLuminance(true);
		PostProcess::g_debugMotionBlurDoF = false;
		enableMirror = false;
		enableReflection = false;
		enableCubeMap = false;
		break;
	default:
		if(GetHDR())
			GetHDR()->SetFastLuminance(false);
		PostProcess::g_debugMotionBlurDoF = true;
		enableMirror = g_debugEnableMirrorView;
		enableReflection = true;
		enableCubeMap = true;
		break;
	}

	if (C_Game::GetInstance()->GetLevelState().mInUI)
	{
		enableMirror = false;			// disable mirror in the UI
		enableCubeMap = false;
		Lod::g_LodEnabled = false;
	}
	else
	{
#if defined(CFG_SPEED) && !defined(CFG_PROFILE)
		Lod::g_LodEnabled = true;
#else
		Lod::g_LodEnabled = false;
#endif
	}

	m_pViewManager->Create(views, enableMirror, enableReflection, enableCubeMap);
	

	return true;
}

XtPack* OpenSpecialFxPack()
{
	DynamicEffect::XtInterface::PackOpen(C_GameApp::GetInstance()->GetGlobalXtDatabase(), "gamedata\\Xt\\");
	return DynamicEffect::XtInterface::GetPack();
}

void CloseSpecialFxPack()
{
	DynamicEffect::XtInterface::PackClose();
}

bool C_GameWorld::CreateRenderers()
{
	// sky renderer
	{
		MEMORY_TRACKER_PUSH_SUB("sky");
		LOADING_PERF("Sky");
		m_pSky = new Effect::Sky::C_TexturedSkyDome;
		m_pSky->SetRenderPass(E_RenderPass_Sky);
		MEMORY_TRACKER_POP_SUB("sky");
	}

	// dynamic effect renderer
	{
		MEMORY_TRACKER_PUSH_SUB("dynamiceffect");
		LOADING_PERF("DynamicEffect");
		// TODO: move this out of gameworld so it is reloaded all the time
		m_pDynamicEffectRenderer = new DynamicEffect::C_Renderer(m_pSceneGraph);
		m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Opaque, E_RenderPass_Solid);
		m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Decal, E_RenderPass_EffectDecal);
		m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent, E_RenderPass_Alpha);
		m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_TwoDimensional, E_RenderPass_TwoDimensional);
		m_pDynamicEffectRenderer->ReserveVertexBuffers(128 * 1024, 36, 640);	// 4.5 megs
		m_pDynamicEffectRenderer->SetSRGB(true);
		m_pDynamicEffectRenderer->SetAssetPath("graphics\\textures\\particles\\");
		m_pDynamicEffectRenderer->SetExternalTextures(C_GameApp::GetInstance()->GetParticleTextures());
		m_pDynamicEffectRenderer->SetJobQueuePolicyHandle(C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy());
		MEMORY_TRACKER_POP_SUB("dynamiceffect");

		m_pViewManager->m_pDynamicEffectRenderer = m_pDynamicEffectRenderer;

		MEMORY_TRACKER_PUSH_SUB("lighteffect");
		m_pLightEffectManager = new DynamicEffect::Light::C_Manager(m_pDynamicEffectRenderer);
		m_pLightEffectManager->ReserveLights(512);
		MEMORY_TRACKER_POP_SUB("lighteffect");

		MEMORY_TRACKER_PUSH_SUB("particle");
		m_pParticleManager = new DynamicEffect::Particle::C_ParticleManager(m_pDynamicEffectRenderer);
		m_pParticleManager->ReserveEmitters(2048);
		m_pParticleManager->ReserveSuperEmitters(256);
		m_pParticleManager->ReserveEventInstances(768);//512);
		m_pParticleManager->SetParticleColour(MathsStorage::Float4A16(2.0f, 2.0f, 2.0f, 2.0f));
		m_pParticleManager->SetEmissiveColour(MathsStorage::Float4A16(2.0f, 2.0f, 2.0f, 2.0f));
		m_pParticleManager->SetDefaultSpawnDirection(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ());
		m_pParticleManager->SetCullDistance(100.0f);
		m_pParticleManager->SetAutoUpdate(false);
		//Now loading in GameAppResources
		//DynamicEffect::Particle::C_UserParticleLibrary::Instance().Open("graphics\\models\\particles\\Amax_Particle_Shapes.particleMesh");
		MEMORY_TRACKER_POP_SUB("particle");

		MEMORY_TRACKER_PUSH_SUB("track");
		m_pTrackRenderer = new DynamicEffect::Track::C_TrackRenderer(m_pDynamicEffectRenderer);
		m_pTrackRenderer->ReserveEmitters(80);
		MEMORY_TRACKER_POP_SUB("track");

		MEMORY_TRACKER_PUSH("dynamiceffectxt");
		// open the effects pack and initialise xt interface
		// now done in preloadspecialfxpack called from uilayer.
		DynamicEffect::XtInterface::PackOpen(C_GameApp::GetInstance()->GetGlobalXtDatabase(), "gamedata\\Xt\\");
		DynamicEffect::XtInterface::Initialise(m_pDynamicEffectRenderer);
		DynamicEffect::Light::XtInterface::Initialise(m_pLightEffectManager);
		DynamicEffect::Particle::XtInterface::Initialise(m_pParticleManager);
		DynamicEffect::Track::XtInterface::Initialise(m_pTrackRenderer);
		MEMORY_TRACKER_POP("dynamiceffectxt");
	}

	// aux flare renderer
	{
		MEMORY_TRACKER_PUSH_SUB("auxflare");

		LOADING_PERF("AuxFlare");
		m_pAuxFlares = new AuxFlares::C_AuxFlares(m_pSceneGraph, "graphics/textures/flares/LightFlare_DIFF.texture");
		// for now we put aux flares in their own render pass. we could put them into the alpha pass if we wanted them sorted though
		m_pAuxFlares->SetRenderPass(E_RenderPass_AlphaVehicleFlares);
		m_pAuxFlares->SetNumViewSettings(E_ViewType_Count);
		m_pViewManager->m_pAuxFlares = m_pAuxFlares;
		MEMORY_TRACKER_POP_SUB("auxflare");
	}

	// light trails renderer
	{
		MEMORY_TRACKER_PUSH_SUB("lighttrails");
		LOADING_PERF("LightTrails");
		m_pLightTrails = new LightTrails::C_LightTrails(m_pSceneGraph);
		m_pLightTrails->SetRenderPass(E_RenderPass_Alpha);
		m_pLightTrails->SetViewFlags(E_ViewTypeFlag_Main);

		//Fill the curve value
		Curve::C_HermiteCurveFixed3 curve;
		curve.Clear();
		for(S32 i = 0; i < C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailIntensity.count && i < 3; i++)
		{
			DynamicEffect::XtInterface::HermiteKeyFrame& keyFrame = C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailIntensity[i];
			curve.AddKeyFrame(keyFrame.time, keyFrame.value, keyFrame.tangentIn, keyFrame.tangentOut);
		}

		for (S32 i = 0; i < LIGHTTRAILS_SIZE; i++)
		{
			float t = Maths::Clamp(F32(i) / F32(LIGHTTRAILS_SIZE - 2), 0.f, 1.f);
			m_pLightTrails->SetIntensityVector(i, curve.Evaluate(t));
		}


		MEMORY_TRACKER_POP_SUB("lighttrails");
	}

#ifdef CREATE_CROWD_FLAG_RENDERER
	// crowd flag renderer
	{
		MEMORY_TRACKER_PUSH_SUB("crowdflags");
		LOADING_PERF("CrowdFlags");
		m_pCrowdFlagRenderer = new AnimatedCrowd::C_CrowdFlags(m_pSceneGraph, m_pViewManager->GetPrepassSubtechniqueHandle());
		m_pCrowdFlagRenderer->SetRenderPass(E_RenderPass_Punch);
		m_pCrowdFlagRenderer->SetViewFlags(E_ViewTypeFlag_Main | E_ViewTypeFlag_Mirror);
		m_pViewManager->m_pCrowdFlagRenderer = m_pCrowdFlagRenderer;
		MEMORY_TRACKER_POP_SUB("crowdflags");
	}
#endif

	m_pSceneGraph->Insert(m_pDynamicEffectRenderer);
	m_pSceneGraph->Insert(m_pSky);
	m_pSceneGraph->Insert(m_pAuxFlares);
	m_pSceneGraph->Insert(m_pLightTrails);
#ifdef CREATE_CROWD_FLAG_RENDERER
	m_pSceneGraph->Insert(m_pCrowdFlagRenderer);
#endif

	return true;
}

bool C_GameWorld::CreateLevelSceneManager()
{
	m_pLevelSceneManager = new GameResource::C_LevelSceneManager;
	m_pLevelSceneManager->Initialise( m_pSceneGraph, m_pPhysicsScene );
	return true;
}

bool C_GameWorld::CreateAnimationTaskManager()
{
	const U32 kMaxAsyncTasksPerFrame = 250;
	bool ok = Animation::CreateTaskManager(kMaxAsyncTasksPerFrame, C_SchedulerConfig::GetInstance()->GetAnimationPolicy());
	return ok;
}

bool C_GameWorld::CreateAnimatedModelTaskManager()
{
	const U32 kMaxAsyncTasksPerFrame = 250;
	bool ok = AnimatedModel::CreateTaskManager(kMaxAsyncTasksPerFrame, C_SchedulerConfig::GetInstance()->GetAnimationPolicy());
	return ok;
}

bool C_GameWorld::CreateDriverManager()
{
	m_pDriverManager = new C_DriverManager;
	m_pDriverManager->SetSceneGraph(m_pSceneGraph);
	m_pDriverManager->SubscribeToMessages(&m_pActorList->GetSubscriberSystem());
	return true;
}

bool C_GameWorld::CreateCrowdSystem()
{
	if (m_pCrowdSystem)
		return false;

	if (!AnimatedCrowd::C_CrowdSystem::DisableCreation())
	{
		m_pCrowdSystem = new AnimatedCrowd::C_CrowdSystem();
		m_pCrowdSystem->SetJobQueuePolicyHandle(C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy());
		m_pCrowdSystem->SetFlagRenderer(m_pCrowdFlagRenderer);

		m_pCrowdGameManager = new C_CrowdGameManager();
		m_pCrowdGameManager->Create(m_pCrowdSystem, &m_pActorList->GetSubscriberSystem());
	}

	return true;
}

bool C_GameWorld::CreateVehicleLightCubeMaps()
{
	bool success = true;
	Display::E_Format lightCubeFormat = Display::E_Format_A8R8G8B8;
	// [Blur2PC][HDR] Floating RGBM permits the multiplier to exceed the retail ceiling.
	if (Display::Device()->GetBackBuffer().GetColourFormat() == Display::E_Format_A16B16G16R16F)
		lightCubeFormat = Display::E_Format_A16B16G16R16F;

	//Create LightCubemaps
	U32 flags = Display::E_Format_Linear | Display::E_Usage_Dynamic;
#ifdef PLATFORM_PS3
	flags |= Display::E_Usage_System;
#endif
	{
		RESOURCE_MEMORY_ID("LightCubemap");

		//Get Num Vehicles
		m_NumVehicleLightCubeMapTexture = C_Game::GetInstance()->GetLevelState().mInUI ? 1 : C_Game::GetInstance()->GetLevelState().mNumCars + C_Game::GetInstance()->GetLevelState().mNumAdditionalCars;

		for (U32 i = 0; i < m_NumVehicleLightCubeMapTexture; i++)
		{
			
			success = success && Display::Device()->CreateTextureCube(GameWorld_LightCubeMapSize, 1,  flags , lightCubeFormat, &m_VehicleLightCubeMapTexture[i][0]);
			BIZ_ASSERT(success);
			success = success &&  Display::Device()->CreateTextureCube(GameWorld_LightCubeMapSize, 1, flags , lightCubeFormat, &m_VehicleLightCubeMapTexture[i][1]);
			BIZ_ASSERT(success);

			//Clean to black the textures
#ifndef PLATFORM_PC
			for (U32 j = 0; j < 2; j++)
			{
				BIZ_ASSERT(m_VehicleLightCubeMapTexture[i][j].IsCreated());
				for (U32 k = 0; k < 6; k++)
				{
					Display2::C_LockedRect lockedRect;
					lockedRect = Display2::LockTextureRectCube(&m_VehicleLightCubeMapTexture[i][j], Display2::E_CubeMapFace(k), 0);
					BIZ_ASSERT(lockedRect.GetBits());
					memset(lockedRect.GetBits(), 0, lockedRect.GetPitch() * GameWorld_LightCubeMapSize);
					Display2::UnlockTextureRectCube(&m_VehicleLightCubeMapTexture[i][j], Display2::E_CubeMapFace(k), 0);
				}
			}
#endif
		}
	}


	return success;
}

void C_GameWorld::AddSubscribers()
{
	SetSubscriberSystem( &m_pActorList->GetSubscriberSystem() );
	AddSubscriber(Game::E_VehicleMessage_VehicleFocusPosition);
	
	SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber(Game::E_GameflowMessage_RaceLoaded);
	AddSubscriber(Game::E_GameflowMessage_RestartLevel);	
	AddSubscriber(Game::E_GameflowMessage_StartGameRendering);
	AddSubscriber(Game::E_GameflowMessage_StopGameRendering);
	AddSubscriber(Game::E_GameflowMessage_FillStreamer);	

	AddSubscriber(Game::E_GameflowMessage_CutsceneVideoFinished );
	AddSubscriber(Game::E_GameflowMessage_CutsceneVideoStarted);
}

void C_GameWorld::RemoveSubscribers()
{
	SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
	RemoveSubscriber(Game::E_GameflowMessage_StartGameRendering);
	RemoveSubscriber(Game::E_GameflowMessage_StopGameRendering);
	RemoveSubscriber(Game::E_GameflowMessage_RaceLoaded);
	RemoveSubscriber(Game::E_GameflowMessage_RestartLevel);

	RemoveSubscriber(Game::E_GameflowMessage_CutsceneVideoFinished);
	RemoveSubscriber(Game::E_GameflowMessage_CutsceneVideoStarted);

	SetSubscriberSystem( &m_pActorList->GetSubscriberSystem() );
	RemoveSubscriber(Game::E_VehicleMessage_VehicleFocusPosition);
	SetSubscriberSystem(NULL);
}

void C_GameWorld::SetNumViews(const S32 numViews )
{
	// tell framework which view setup we want
	switch(numViews)
	{
	case 1:
		Framework::C_ApplicationBase::GetInstance()->SetViewSetup(Framework::E_ViewSetup_1);
		break;
	case 2:
		Framework::C_ApplicationBase::GetInstance()->SetViewSetup(Framework::E_ViewSetup_2h);
		break;
	case 3:
		// Use custom split screen 3 player setup
		Framework::C_ApplicationBase::GetInstance()->SetViewSetup(Framework::E_ViewSetup_3);
		break;
	case 4:
		Framework::C_ApplicationBase::GetInstance()->SetViewSetup(Framework::E_ViewSetup_4);
		break;
	}

	C_Game::GetInstance()->GetLevelState().mNumViewports = numViews;
}

bool C_GameWorld::Create( Actor::C_ActorFactory* pActorFactory, const S32 numViews )
{
	bool success = true;

	m_GameFrame = 0;

	C_GameApp::GetInstance()->ResetElapsedRaceTime();

	Lod::C_LodManager::Instance().Initialise(30.0f);
	
	SetNumViews(numViews);
	AmaxCam::CameraSystem().SetViewConfig(numViews);

	Resource::C_Manager::GetInstance()->PauseLoadingThread();

	{
		LOADING_PERF("CreatePostProcess");
		C_GameWorldPostProcess::Create();
	}

	{
		LOADING_PERF("CreatePhysics");
		success = CreatePhysics();
		if(!success)
			return false;
	}

	{
		LOADING_PERF("CreateSceneGraph");
		success = CreateSceneGraph();
		if(!success)
			return false;
	}

	{
		LOADING_PERF("CreateRenderers");
		success = CreateRenderers();
		if(!success)
			return false;
	}
	Resource::C_Manager::GetInstance()->ResumeLoadingThread();

//	success = CreateStreamer();
//	if(!success)
//		return false;

	{
		LOADING_PERF("CreateLevelSceneManager");
		success = CreateLevelSceneManager();
		if(!success)
			return false;
	}

	{
		LOADING_PERF("CreateTextureHeadlight");
		success = CreateTextureHeadlight();
		if(!success)
			return false;
	}

	{
		LOADING_PERF("CreateAnimationTaskManager");
		success = CreateAnimationTaskManager();
		if (!success)
			return false;
	}

	{
		LOADING_PERF("CreateAnimatedModelTaskManager");
		success = CreateAnimatedModelTaskManager();
		if (!success)
			return false;
	}

	Resource::C_Manager::GetInstance()->PauseLoadingThread();
	{
		LOADING_PERF("CreateActorList");
		success = CreateActorList( pActorFactory );
		if(!success)
			return false;
	}

	{
		LOADING_PERF("CreateDriverManager");
		success = CreateDriverManager();
		if (!success)
			return false;
	}

	{
		LOADING_PERF("CreateCrowdSystem");
		success = CreateCrowdSystem();
		if (!success)
			return false;
	}

	{
		LOADING_PERF("CreateVehicleLightCubeMaps");
		success = CreateVehicleLightCubeMaps();
		if (!success)
			return false;
	}

	{
		LOADING_PERF("Damage CreateTextureResources");
		//Get Num Vehicles
		U32 numVehicles = C_Game::GetInstance()->GetLevelState().mInUI ? 1 : C_Game::GetInstance()->GetLevelState().mNumCars + C_Game::GetInstance()->GetLevelState().mNumAdditionalCars;
		success = Vehicle::C_DamageLayer::CreateTextureResources(numVehicles);
		if (!success)
			return false;
	}
	{
		LOADING_PERF("Vehicle PaintTextures");
		//Get Num Vehicles
		U32 numVehicles = C_Game::GetInstance()->GetLevelState().mInUI ? 1 : C_Game::GetInstance()->GetLevelState().mNumCars + C_Game::GetInstance()->GetLevelState().mNumAdditionalCars;
		success = Vehicle::C_PaintJob::CreatePaintTextures(numVehicles);
		if (!success)
			return false;
	}

	{
		LOADING_PERF("CreateLightingPrepass");
		m_pLightingPrepass = new C_LightingPrepass;
		success = m_pLightingPrepass->Create();
		if (!success)
			return false;
	}

	{
		LOADING_PERF("CreateDynamicShadows");
		//create dynamic lights, drop shadows and blob shadows
		m_pDropShadows = new DropShadows::C_DropShadows(m_pLightingPrepass->GetDeferredManager(), Game::E_ViewTypeFlag_Main);
		m_pDropShadows->SetRenderPass(E_RenderPass_Shadows);
		m_pSceneGraph->Insert(m_pDropShadows);

		m_pBlobShadows = new BlobShadows::C_BlobShadows();
		BlobShadows::C_BlobShadows::C_BlobShadowsInitSettings blobShadowsInitSettings;
		m_pBlobShadows->Create(blobShadowsInitSettings, m_pSceneGraph->GetContext());
		m_pBlobShadows->SetRenderPass(E_RenderPass_Shadows);
		m_pBlobShadows->SetVisibilityFlags(Game::E_ViewTypeFlag_Main);

		m_pSceneGraph->Insert(m_pBlobShadows);
	
	}

	{
		LOADING_PERF("CreateSplashCamera");
		
		m_pSplashCamera = new SplashCamera::C_SplashCamera();
		m_pSplashCamera->SetRenderPass(E_RenderPass_TwoDimensional);
		m_pSplashCamera->SetNumViewSettings(E_ViewType_Count);
		m_pSplashCamera->SetVisibilityFlags(E_ViewTypeFlag_Main);
		m_pSceneGraph->Insert(m_pSplashCamera);
	}

	{
		LOADING_PERF("CreateReducedRender");
		// [Blur2PC][HDR] Particles and lens/light flares are composed through
		// these reduced targets BEFORE the final tone mapper. UNORM storage
		// clips their exposed radiance, even with an FP16 main scene/output.
		const bool hdrStorage = Display::Device()->GetBackBuffer().GetColourFormat() == Display::E_Format_A16B16G16R16F;
		m_pReducedRender = new Render::Reduced::C_Manager(hdrStorage);

		const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
		for (S32 viewNo = 0; viewNo < viewCount; ++viewNo)
		{
			const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(viewNo);
			if (pView && pView->m_ViewFlags & Render::E_ViewFlag_EnableDeferred)
			{
				const Display::C_Viewport& viewport = pView->m_View.GetViewport();
				m_pReducedRender->AddView(pView->m_ViewIndex, viewport.GetW(), viewport.GetH());
			}
		}
	}

#if PLATFORM_PC
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x00423054, in C_GameWorld::Create)
	{
		if (!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_R32F, &m_WindowDepthTexture))
			return false;

		Display::C_LockedRect rect = m_WindowDepthTexture.Lock(0);
		if(rect.pBits)
			*reinterpret_cast<F32*>(rect.pBits) = 0.0f;
		m_WindowDepthTexture.Unlock(0);
	}
#endif

	m_pWind = new C_Wind(m_pParticleManager);

	{
		LOADING_PERF("CreateViews");
		// do this after creating renderers
		success = CreateViews(numViews);
		if(!success)
			return false;
	}

	AddSubscribers();
	
	//Read shock texture
#ifndef CFG_GOLD
	if (!g_debugSuperSpecialCutSceneVideoMode)
#endif
	{
		LOADING_PERF("CreateShockTexture");
		if (!Display::CreateTexture2DFromFile("graphics/textures/shock/Shock.dds", &m_ShockTexture))
			return false;
	}

	Resource::C_Manager::GetInstance()->ResumeLoadingThread();

#ifndef CFG_GOLD
	Game::C_PerfStats::Instance().Setup();
#endif
	
	//WaitForFXResourceBag();

	SpecialFX::C_AnimatedMeshSpecialFX::CreatePostUpdateSystem();

#if PLATFORM_PC
	// [Blur2PC] Retail C_GameWorld::Create ends by applying the player's graphical detail (Blur.exe 1.2 0x0042311e).
	SetGraphicalDetail(C_GameApp::GetInstance()->GetGraphicalDetail());
#endif

	return true;
}

bool C_GameWorld::Reset()
{		
	#ifdef ENABLE_LOAD_PERF
	LoadPerf::Push("Loading");
	#endif

	m_letsGoRacing = false;

	C_DriverManager::Instance()->Clear();
	
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_ClearStoredActorPtrs);
	m_pActorList->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_ClearStoredActorPtrs );

	m_pActorList->Reset();

	//Reset all active particles
	m_pDynamicEffectRenderer->Reset();

	AIAmax::AmaxAIR::Instance().ShutDown();

	::Actors::C_ActorUtils::Instance().RemoveDestroyedTriggers();
	if(m_pPhysicsScene)
	{
		if (m_pPhysicsScene->GetTriggerSystem())
			m_pPhysicsScene->GetTriggerSystem()->DestroyLayers();
		Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene( m_pPhysicsScene );
		m_pPhysicsScene = NULL;
	}

	bool success = true;
	success = CreatePhysics();
	if(!success)
		return false;

	if (m_pLevelSceneManager)
		m_pLevelSceneManager->SetPhysicsScene(m_pPhysicsScene);

	m_pContextProvider->SetPhysicsContext(m_pPhysicsScene, &m_PhysicsExternalConstants);

	//m_waitForStreamer =  !g_debugStreamerBlockLoad;;

	m_cutsceneVideoPlaying		= false;
	m_allowedToStream			= false;
	m_waitForStreamer			= true;
	m_streamerStartedLoading	= false;
	m_streamerFinishedLoading	= false;
	m_startStreamer				= false;

	m_pSplashCamera->Clear();

	if (m_pCrowdSystem)
		m_pCrowdSystem->Reset();

	return true;
}

bool C_GameWorld::Destroy()
{
	C_GameWorldPostProcess::Destroy();

	m_pSceneEnvMapGPUTexture = NULL;
	if (m_pSceneEnvMapTexture)
		WorldRender::C_LocalisedCubemapData::ReleaseGPUTexture(m_pSceneEnvMapTexture);

	m_pParticleManager->UpdateMemory();

	m_cutsceneVideoPlaying = false;
	m_allowedToStream = false;
	m_waitForStreamer = false;
	m_streamerStartedLoading = false;
	m_streamerFinishedLoading = false;
	m_startStreamer = false;

	_UI_MESSAGE( Game::E_GameflowMessage_ClearStoredActorPtrs );
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_ClearStoredActorPtrs);
	m_pActorList->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_ClearStoredActorPtrs );

	RemoveSubscribers();

	if (m_pDriverManager)
	{
		delete m_pDriverManager;
		m_pDriverManager = NULL;
	}

	m_pViewManager->Destroy();
	delete m_pViewManager;

	if (m_pCrowdSystem)
		m_pCrowdSystem->Destroy();

	if (m_pCrowdGameManager)
	{
		delete m_pCrowdGameManager;
		m_pCrowdGameManager = NULL;
	}

	// close pack and shutdown
	//DynamicEffect::XtInterface::PackClose();
	DynamicEffect::Light::XtInterface::Shutdown();
	DynamicEffect::Particle::XtInterface::Shutdown();
	DynamicEffect::Track::XtInterface::Shutdown();

	if (m_pSplashCamera)
		m_pSplashCamera->Destroy();
	SAFE_DELETE_PTR( m_pSplashCamera );

	if(m_pLightingPrepass)
		m_pLightingPrepass->Destroy();
	
	SAFE_DELETE_PTR( m_pLightingPrepass );
	SAFE_DELETE_PTR( m_pWind );
	SAFE_DELETE_PTR( m_pSky );

#ifndef CFG_SPEED
	Messaging::SetDefaultLog(&g_GameDbgMsgLog);
#endif
	SAFE_DELETE_PTR( m_pActorList );
		
	AnimatedModel::DestroyTaskManager();
	Animation::DestroyTaskManager();

#ifndef CFG_SPEED
	Messaging::SetDefaultLog(NULL);
#endif
	SAFE_DELETE_PTR( m_pVisibilityDetector );
	SAFE_DELETE_PTR( m_pDynamicEffectRenderer );
	SAFE_DELETE_PTR( m_pAuxFlares );
	SAFE_DELETE_PTR( m_pLightTrails );
	SAFE_DELETE_PTR( m_pCrowdFlagRenderer );
	SAFE_DELETE_PTR( m_pContextProvider );
	SAFE_DELETE_PTR( m_pLevelSceneManager );
	SAFE_DELETE_PTR( m_pCrowdSystem );
	SAFE_DELETE_PTR( m_pSceneGraph );
	SAFE_DELETE_PTR( m_pBackFront );
	SAFE_DELETE_PTR( m_pVehicleSorter );
	SAFE_DELETE_PTR( m_pPunchDepthSorter );

	::Actors::C_ActorUtils::Instance().RemoveDestroyedTriggers();

	if(m_pPhysicsScene)
	{
		if (m_pPhysicsScene->GetTriggerSystem())
			m_pPhysicsScene->GetTriggerSystem()->DestroyLayers();
		Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene( m_pPhysicsScene );
		m_pPhysicsScene = NULL;
	}

	SAFE_DELETE_PTR( m_pReducedRender );

	DestroyTextureHeadlight();


	Display::Device()->Destroy( &m_ShockTexture );
#if PLATFORM_PC
	Display::Device()->Destroy( &m_WindowDepthTexture );	// [Blur2PC] retail 0x0041a363
#endif

	AIAmax::AmaxAIR::Instance().ShutDown();

#ifndef CFG_GOLD
	Game::C_PerfStats::Instance().Shutdown();
#endif


	//Destroy Vehicle Light Cubemaps
	for (U32 i = 0; i < m_NumVehicleLightCubeMapTexture; i++)
	{
		Display::Device()->Destroy( &m_VehicleLightCubeMapTexture[i][0]);
		Display::Device()->Destroy( &m_VehicleLightCubeMapTexture[i][1]);
	}

	//Destroy Damage Texture Resources
	Vehicle::C_DamageLayer::DestroyTextureResources();

	//Destroy Paint Textures Resources
	Vehicle::C_PaintJob::DestroyPaintTextures();

	SpecialFX::C_AnimatedMeshSpecialFX::DestroyPostUpdateSystem();


	return true;
}

U32 C_GameWorld::GetElectricSubTechniqueHandle() const
{
	return m_pViewManager->GetElectricSubtechniqueHandle();
}

Gravity::C_ExternalConstants& C_GameWorld::GetPhysicsExternalConstants()
{
	return m_PhysicsExternalConstants;
}

Sys::I_ContextProvider* C_GameWorld::GetContextProvider()
{
	return (m_pContextProvider != NULL) ? m_pContextProvider : NULL;
}

Gravity::C_TriggerSystem* C_GameWorld::GetTriggerSystem()
{
	if (!m_pPhysicsScene)
		return NULL;

	return m_pPhysicsScene->GetTriggerSystem();
}

Gravity::C_TriggerLayer* C_GameWorld::GetTriggerLayer(E_TriggerLayer p_Layer)
{
	if (!m_pPhysicsScene || !m_pPhysicsScene->GetTriggerSystem())
		return NULL;

	return m_pPhysicsScene->GetTriggerSystem()->GetLayer((U32)p_Layer);
}

void C_GameWorld::DebugRender( DbUtil::I_DebugRender* pDebugRender )
{
	//#if defined (PLATFORM_XENON)
	const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewIndex);
	Maths::HMat43 mCameraToWorld = view.m_View.GetViewSettings3D().GetCameraToWorld();
	
	if( g_RenderGravityScene )
	{
		Gravity::C_Scene* pScene = GetPhysicsScene();
		if( pScene ) 
		{
			pDebugRender->SetLightDirection( Maths::HVec3( 0.2f, 0.7f, 0.3f ).Normalised().Normal() );
			Collision::C_CRAxisBox box;

			F32 drawDist = g_GravityDrawDistance;
			const Maths::HVec3 centre = mCameraToWorld.AW() + mCameraToWorld.AZ() * drawDist * 0.5f;
			const Maths::HVec3 extents = Maths::HVec3(drawDist, drawDist, drawDist);
			box.SetExtents( extents );
			box.SetCentre( centre );
			pScene->DebugRenderScene( pDebugRender, box, pScene->m_Stack );
		}
	}

	//RacingPhysics::VehicleManager::Instance().DisplayDebug();
	
	WorldRender::C_Light::SetDebugRenderCameraMatrix( mCameraToWorld );

	if( GetLevelSceneManager() )
	{
		GetLevelSceneManager()->DebugRender( pDebugRender );
	}
	
	#ifndef CFG_GOLD
	if (g_DebugGameWorldDebugActorRender)
	{
		PERF_EVENT("Render.ActorsDebugRender");
		GetActorList()->DebugRender(pDebugRender);
		m_pDriverManager->DebugRender(pDebugRender);
	}
	#endif
	
//#endif
}

Debug::Bool g_debugLoadDebugCubeMaps("Amax.Debug.LoadDebugCubeMaps", false);

bool C_GameWorld::CreateTextureHeadlight()
{
	RESOURCE_MEMORY_ID("Headlight");

	const U32 iVolumeWidth	= 64;
	const U32 iVolumeHeight	= 16;
	const U32 iVolumeDepth	= 64;

	#if PLATFORM_PC
	const U32 flags = Display::E_Usage_Managed;
	#else
	const U32 flags = Display::E_Usage_Video | Display::E_Format_Linear;
	#endif

	if (!Display::Device()->CreateTexture3D(iVolumeWidth, iVolumeHeight, iVolumeDepth, 1, flags, Display::E_Format_L8, &m_HeadlightTexture))
		return false;

	Display::C_LockedBox lockedBox = m_HeadlightTexture.LockBox3D(0);
	if (!lockedBox.pBits)
		return false;

	U8* pBits = (U8*)lockedBox.pBits;

	bool defaultTexture = false;
	//Look if texture exists
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	if (fs.FileExists(g_HeadlightTextureFileName.Value()))
	{
		//Open head light texture
		Sys::C_FileInfo fileInfo;
		fs.FileInfo(g_HeadlightTextureFileName.Value(), &fileInfo);

		if (fileInfo.m_FileSize == (iVolumeWidth * iVolumeHeight * iVolumeDepth))
		{
			//Open
			Sys::I_File* pFile = fs.Open(g_HeadlightTextureFileName.Value());
			if (pFile)
			{
				//Fill the data
				char textureBuffer[iVolumeWidth * iVolumeHeight * iVolumeDepth];
				pFile->Read(textureBuffer, iVolumeWidth * iVolumeHeight * iVolumeDepth);

				//Read blocks of iVolumeWidth*iVolumeHeight
				for (U32 i = 0; i < iVolumeDepth; i++)
				{
					U8* pBitsExtra = pBits;
					for (U32 j = 0; j < iVolumeHeight; j++)
					{
						memcpy(pBitsExtra, &textureBuffer[j * iVolumeWidth * iVolumeDepth + (iVolumeDepth - (i + 1)) * iVolumeWidth], iVolumeWidth);
						pBitsExtra += lockedBox.rowPitch;
					}
					
					pBits += lockedBox.slicePitch;
				}

				delete pFile;
			}
			else
			{
				defaultTexture = true;
				LOGWARN(g_Log, "Error opening headlight texture <%s>, loading default", g_HeadlightTextureFileName.Value());
			}
		}
		else
		{
			defaultTexture = true;
			LOGWARN(g_Log, "Headlight texture <%s> size is not correct, loading default", g_HeadlightTextureFileName.Value());
		}
	}
	else
	{
		defaultTexture = true;
		LOGINFO(g_Log, "Headlight texture <%s> doesn't exist, loading default", g_HeadlightTextureFileName.Value());
	}
	
	//If fails
	if (defaultTexture)
	{
		//Create default one
		const F32 fMinOffset = g_debugTextureHeadlightMinOffset;
		const F32 fMaxOffset = g_debugTextureHeadlightMaxOffset;
		const F32 fMinSigma  = g_debugTextureHeadlightMinSigma;
		const F32 fMaxSigma  = g_debugTextureHeadlightMaxSigma;
		const F32 fScale     = 3.0f;

		for (U32 w = 0; w < iVolumeDepth; ++w)
		{
			const F32 z				= (1.0f * w) / (iVolumeDepth - 1);
			const F32 fIScale		= (1.0f - z > 1.0f) ? 1.0f : (1.0f - z);
			const F32 fOffset		= fMinOffset + (fMaxOffset - fMinOffset) * z;
			const F32 fSigma		= fMinSigma + (fMaxSigma - fMinSigma) * z;
			const F32 fBias			= expf(-1.0f * (1.0f - fOffset) * (1.0f - fOffset) / (2.0f * fSigma * fSigma));
			const F32 fMaxAmplitude	= expf(-1.0f * fOffset * fOffset / (2.0f * fSigma * fSigma)) - fBias;
			const F32 fScale		= 1.0f / fMaxAmplitude;
			for (U32 v = 0; v < iVolumeHeight; ++v)
			{
				for (U32 u = 0; u < iVolumeWidth; ++u)
				{
					const F32 x	= 2.0f * u / (iVolumeWidth - 1) - 1.0f;
					const F32 vp = powf((F32) v, g_fHeadlightVScalePow);
					const F32 y	= 2.0f * vp / (iVolumeHeight - 1) - 1.0f;

					const F32 r	= sqrtf(x*x + y*y);
					
					F32 fIntensity	= fIScale * fScale * (expf(-1.0f * (r - fOffset) * (r - fOffset) / (2.0f * fSigma * fSigma)) - fBias);

					fIntensity += g_fHeadlightTextureOffset;

					if (fIntensity > 1.0f)
						fIntensity = 1.0f;
					if (fIntensity < 0.0f)
						fIntensity = 0.0f;
					if (w == 0)
						fIntensity *= 0.00f;
					if (w == 1)
						fIntensity *= 0.50f;
					if (w == 2)
						fIntensity *= 0.75f;
					if (w == iVolumeDepth - 1)
						fIntensity *= 0.0f;

					fIntensity = powf(fIntensity, g_fHeadlightTexturePow);

					pBits[u + v * lockedBox.rowPitch] = (U8) (255.0f * fIntensity);
				}
			}
			pBits += lockedBox.slicePitch;
		}
	}

	m_HeadlightTexture.UnlockBox3D(0);

	return true;
}

void C_GameWorld::DestroyTextureHeadlight()
{
	Display::Device()->Destroy(&m_HeadlightTexture);
}

Debug::Bool g_RimDebug("Amax.RimDebug", false);

Debug::Float g_debugIntGlassVOffset("Amax.DashView.GlassVOffset", 0.2f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugIntGlassVPinch("Amax.DashView.GlassVPinch", 1.2f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));

Debug::Float g_debugIntGlassPow("Amax.DashView.GlassPow", 2.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugIntGlassScale("Amax.DashView.GlassScale", 30.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));
Debug::Float g_debugIntGlassMaxInt("Amax.DashView.GlassMaxInt", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugIntGlassMinAlpha("Amax.DashView.GlassMinAlpha", 0.1f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

extern Debug::Bool g_CubeMapEnable;
bool g_bFrigCubemapWinner = false;

void C_GameWorld::OncePerFrameFXUpdate()
{

	// set parameters
	const void* pEnvTexAddress;
	if ((g_CubeMapEnable && m_pViewManager->GetCubeMap() && C_Game::GetInstance()->GetLevelState().mNumViewports == 1) || g_bFrigCubemapWinner)
		pEnvTexAddress = const_cast<Display::C_TextureCube*>(m_pViewManager->GetCubeMap()->GetRenderTexture())->GetGPUTexture();
	else
		pEnvTexAddress = m_pSceneEnvMapGPUTexture;

	FX::SetParameter(g_FxParams.vehicleEnvTex, &pEnvTexAddress);
	FX::SetSamplerState(g_FxParams.vehicleEnvTex, g_FxParams.m_ClampLinear);
	Vehicle::C_VehicleModelManager::Instance().SetCubeMapTexture(pEnvTexAddress);

	{
		const MathsStorage::Float4 vCutSceneLightColour0 = CutScene::GetLightColour(0);
		const MathsStorage::Float4 vCutSceneLightColour1 = CutScene::GetLightColour(1);
		const MathsStorage::Float4 vCutSceneLightColour2 = CutScene::GetLightColour(2);
		const MathsStorage::Float4 vCutSceneLightColour3 = CutScene::GetLightColour(3);
		const MathsStorage::Float4 vCutSceneLightDirection0 = CutScene::GetLightDirection(0);
		const MathsStorage::Float4 vCutSceneLightDirection1 = CutScene::GetLightDirection(1);
		const MathsStorage::Float4 vCutSceneLightDirection2 = CutScene::GetLightDirection(2);
		const MathsStorage::Float4 vCutSceneLightDirection3 = CutScene::GetLightDirection(3);
		FX::SetParameter(g_FxParams.cutSceneLightColour0, &vCutSceneLightColour0);
		FX::SetParameter(g_FxParams.cutSceneLightColour1, &vCutSceneLightColour1);
		FX::SetParameter(g_FxParams.cutSceneLightColour2, &vCutSceneLightColour2);
		FX::SetParameter(g_FxParams.cutSceneLightColour3, &vCutSceneLightColour3);
		FX::SetParameter(g_FxParams.cutSceneLightDir0, &vCutSceneLightDirection0);
		FX::SetParameter(g_FxParams.cutSceneLightDir1, &vCutSceneLightDirection1);
		FX::SetParameter(g_FxParams.cutSceneLightDir2, &vCutSceneLightDirection2);
		FX::SetParameter(g_FxParams.cutSceneLightDir3, &vCutSceneLightDirection3);
	}
	
	const F32 fTime = C_GameApp::GetInstance()->GetElapsedRaceTime();
	FX::SetParameter(g_FxParams.time, Display::C_PSConstant(fTime));

	const Display::C_Float4 vSunDirection = Display::C_Float4(m_SunDirection.X(), m_SunDirection.Y(), m_SunDirection.Z(), 0.00f);
	FX::SetParameter(g_FxParams.sunDirection, &vSunDirection);

	Display::C_Float4 vDynamicScale(1.0f / g_DynamicLightScale, 1.0f / g_DynamicLightScale, g_DynamicLightScale, 1.0f / g_DynamicLightScale);
	if (!g_DynamicLightEnable)
		vDynamicScale = Display::C_Float4(0.f, 0.f, 0.f, 0.f);
	
	FX::SetParameter(g_FxParams.dynamicScale, &vDynamicScale);

	const void* pShockTexture = m_ShockTexture.GetGPUTexture();
	FX::SetParameter(g_FxParams.shockTexture, &pShockTexture);
	FX::SetSamplerState(g_FxParams.shockTexture, g_FxParams.m_WrapLinear);

	// set wind
	FX::SetParameter(g_FxParams.windMatrices, m_pWind->GetWindMatrices());
	FX::SetParameter(g_FxParams.leafMatrices, m_pWind->GetLeafMatrices());

	//Set interior glass stuff for cars
	const Display::C_Float4 vScreenSettingsVS = Display::C_Float4((float)g_debugIntGlassVOffset, (float)g_debugIntGlassVPinch, 0.0f, 0.0f);
	FX::SetParameter(g_FxParams.screenSettingsVS, &vScreenSettingsVS);
	const Display::C_Float4 vScreenSettingsPS = Display::C_Float4((float)g_debugIntGlassPow, (float)g_debugIntGlassScale, (float)g_debugIntGlassMaxInt, (float)g_debugIntGlassMinAlpha);
	FX::SetParameter(g_FxParams.screenSettingsPS, &vScreenSettingsPS);

	//Ninja'd the following from BeginFrame()->ViewDependantFXUpdate(0);

	// set hdr parameters
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		const Display::C_Float4	pVector		= Render::C_RenderState::Instance().HDR()->GetShaderParameters(0);
		const void*				pTexture;

		if (const Display::C_Texture2D* pLuminanceTexture = Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture(0))
			pTexture	= pLuminanceTexture->GetGPUTexture();
		else
			pTexture	= NULL;
		
		FX::SetParameter(g_FxParams.luminanceParameter, &pVector);
		FX::SetParameter(g_FxParams.luminanceTexture, &pTexture);
		FX::SetSamplerState(g_FxParams.luminanceTexture, g_FxParams.m_ClampPoint);

		if (m_pCrowdFlagRenderer)
			m_pCrowdFlagRenderer->SetLuminance(pVector);
	}

	if (m_pViewManager->GetView(0) && m_pViewManager->GetView(0)->GetEnvironmentSettings())
		m_pViewManager->GetView(0)->GetEnvironmentSettings()->Set();
}

void C_GameWorld::SetReflectionBlur(F32 blurX, F32 blurY, F32 deviation)
{
	m_pViewManager->SetReflectionBlur(blurX, blurY, deviation);
}

void C_GameWorld::BeginFrame()
{	
	PERF_EVENT("C_GameWorld.BeginFrame");

#if PLATFORM_PC
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x00420590) gives each display window its depth texture: the
	// view's prepass depth, or a 1x1 texture holding 0 while the prepass is off
	{
		Framework::C_ApplicationBase* pApp = Framework::C_ApplicationBase::GetInstance();
		const S32 viewCount = pApp->GetSplitscreenViewCount();
		for(S32 viewIndex = 0; viewIndex < viewCount; ++viewIndex)
		{
			const Display::C_Texture2D* pPrepassDepth = m_pViewManager->m_pLightingPrepass->GetDepthTexture(pApp->GetSplitscreenView(viewIndex).m_ViewIndex);
			Display::Device()->GetWindow(viewIndex)->m_pDepthTexture = g_MainPrepassEnable ? pPrepassDepth : &m_WindowDepthTexture;
			static int dx11DepthLogCount = 0;
			if (dx11DepthLogCount++ < 12)
				Blur2PCAutomation::LogF("DX11 BeginFrame depth view=%d prepass=%d pPrepass=%p selected=%p", viewIndex, (int)g_MainPrepassEnable, pPrepassDepth, Display::Device()->GetWindow(viewIndex)->m_pDepthTexture);
		}
	}
#endif

#if PLATFORM_PC
	// [Blur2PC] Retail PC has no garage hack: the prepass follows Amax.Views.Main.PrepassEnable (0x0111e020,
	// off at graphical detail 1) in the front end too, and mInUI only drops its lights (the light buffer
	// binding in C_MainView::Schedule 0x0042a170, RenderPrepass1 0x00426390, RenderPrepass2 0x00554c10). The
	// prepass depth is what the blob shadows read, so the hack left the showroom car without one (T115).
	Game::g_PrepassEnable = g_MainPrepassEnable;
#else
	// [Stove] HACK ALERT!!!!!!! [12022010]
	// Stop the garage framing out, this is done in a really sucky way.
	static U32 notFirstFrame = 0;
	
	if (C_Game::GetInstance()->GetLevelState().mInUI)
	{
		if(notFirstFrame)
			Game::g_PrepassEnable = false;
		else
			Game::g_PrepassEnable = true;
	}
	else
		Game::g_PrepassEnable = true;

	if(!notFirstFrame)
		notFirstFrame++;
#endif

	{
		PERF_EVENT("Render.AnimationEvalFlush");
		Animation::TaskManager().Flush();
		Animation::TaskManager().ClearStats();
	}
	
	{
		PERF_EVENT("Render.AnimatedModelFlush");
		AnimatedModel::TaskManager().Flush();
	}

	{
		EndCarEffectTasks();
	}

	{
		PERF_EVENT("Render.RenderingData");
		RenderingData::Target::BeginFrame();
	}

	{
		PERF_EVENT("Render.FxUpdate");
		OncePerFrameFXUpdate();
		//[CWright] Removed from here and tacked relevant stuff onto the end of OncePerFrameFXUpdate()
		//ViewDependentFXUpdate(0);
	}

	Vehicle::C_VehicleModelManager::Instance().PreRender();

	{
		CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.RenderBlobShadows");
		GPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "GPU.RenderBlobShadows");

		DEVICE_MARKER("BeginFrame.BlobShadowsPreRender");
		m_pViewManager->m_pBlobShadows->PreRender();
	}

	{
		PERF_EVENT("Render.SceneGraph");
		// rendering of non main view dependant views eg cubemap, perlin noise, etc
		m_pVisibilityDetector->Update( m_pSceneGraph );
		m_pSceneGraph->BeginFrame();

		if(SceneGraph::C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Punch))
		{
			if(g_PunchInstancesFirst)
				pRenderPass->SetDrawOrder(SceneGraph::E_DrawOrder_InstancesPrimitives);
			else
				pRenderPass->SetDrawOrder(SceneGraph::E_DrawOrder_PrimitivesInstances);

			if(g_PunchPrimitivesFrontBack || g_PunchInstancesFrontBack || g_PunchModelInstancesFrontBack)
				pRenderPass->SetSorter(m_pPunchDepthSorter);
			else
				pRenderPass->SetSorter(NULL);

			m_pPunchDepthSorter->SetPrimitiveSorting(g_PunchPrimitivesFrontBack);
			m_pPunchDepthSorter->SetInstanceSorting(g_PunchInstancesFrontBack);
			m_pPunchDepthSorter->SetModelInstanceSorting(g_PunchModelInstancesFrontBack);
		}
	}

	{
		PERF_EVENT("Render.CrowdSystem");
		if (m_pCrowdSystem)
			m_pCrowdSystem->BeginFrame(C_GameApp::GetInstance()->IsPaused());
	}

	m_pViewManager->BeginFrame();
}

void C_GameWorld::ScheduleOnce()
{
	FX::SetParameter(g_FxParams.luminanceParameter, Display::C_PSConstant(1.0f, 1.0f, 1.0f, 1.0f));
	if (m_pViewManager->GetCubeMap())
		m_pViewManager->GetCubeMap()->Schedule();
	if (m_pCrowdSystem)
		m_pCrowdSystem->Schedule();
}

void C_GameWorld::RenderOnce()
{
	const S32 viewCount = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	if(viewCount > 1)
	{
		if(m_pViewManager->GetCubeMap())
			m_pViewManager->GetCubeMap()->Render();
	}
}

void C_GameWorld::ViewDependentFXUpdate(S32 viewIndex)
{
	// set hdr parameters
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		const Display::C_Float4	pVector		= Render::C_RenderState::Instance().HDR()->GetShaderParameters(viewIndex);
		const void*				pTexture;

		if (const Display::C_Texture2D* pLuminanceTexture = Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture(viewIndex))
			pTexture	= pLuminanceTexture->GetGPUTexture();
		else
			pTexture	= NULL;
		
		FX::SetParameter(g_FxParams.luminanceParameter, &pVector);
		FX::SetParameter(g_FxParams.luminanceTexture, &pTexture);
		FX::SetSamplerState(g_FxParams.luminanceTexture, g_FxParams.m_ClampPoint);

		if (m_pCrowdFlagRenderer)
			m_pCrowdFlagRenderer->SetLuminance(pVector);
	}

	if (m_pViewManager->GetView(viewIndex) && m_pViewManager->GetView(viewIndex)->GetEnvironmentSettings())
		m_pViewManager->GetView(viewIndex)->GetEnvironmentSettings()->Set();
	
	//Set Colour and Depth buffers for refraction
	// [Blur2PC] the null checks are ours: the globals are the display window's targets (T089), unset
	// before the first splitscreen view is scheduled
	{
		const Display::C_Texture2D* pColour = Render::GetGlobalColourBuffer(true);
		const void* pTexture = pColour ? pColour->GetGPUTexture() : NULL;
		FX::SetParameter(g_FxParams.colourBuffer, &pTexture);
		FX::SetSamplerState(g_FxParams.colourBuffer, g_FxParams.m_MirrorPoint);
	}
	{
		const Display::C_Texture2D* pDepth = Render::GetGlobalDepthStencilBuffer();
		const void* pTexture = pDepth ? pDepth->GetGPUTexture() : NULL;
		FX::SetParameter(g_FxParams.depthBuffer, &pTexture);
		FX::SetSamplerState(g_FxParams.depthBuffer, g_FxParams.m_MirrorPoint);
	}
	{
		const MathsStorage::Float4& tempDSSP = Render::C_RenderState::Instance().GetDepthScaleShaderParameter(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewIndex).m_View.GetViewSettings3D().GetNearClip(), Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewIndex).m_View.GetViewSettings3D().GetFarClip());
		FX::SetParameter(g_FxParams.depthScale, &tempDSSP);
	}

#if PLATFORM_PC
	// [Blur2PC] T107: retail PC (Blur.exe 1.2, 0x00420740) refracts through the motion-blur target only at
	// graphical detail 2-4, where the prepass (and so that pass) runs, and clears it otherwise; the splash
	// types that refract fully are then skipped and the rest fade by 1 - RefractionIntensity
	if ((m_pViewManager->GetNumViews() == 1) && m_GraphicalDetail > 1)
		m_pSplashCamera->SetRefractionTexture(GetMotionBlur()->GetTexture());
	else
		m_pSplashCamera->SetRefractionTexture(NULL);
#else
	if ((m_pViewManager->GetNumViews() == 1))
	{
		m_pSplashCamera->SetRefractionTexture(GetMotionBlur()->GetTexture());
	}
#endif

}

void C_GameWorld::BeginSplitscreen()
{
	const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();

	m_pViewManager->GetView(viewIndex)->Begin();
}

void C_GameWorld::ScheduleSplitscreen()
{
	const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
	
	// from post process layer
	m_pPostProcessMgr->SetView(viewIndex);

	ViewDependentFXUpdate(viewIndex);

	// change the plane reflection to disable the reflection in the mirror and reflection (temp hack)
	Display::C_Float4 preRenderReflectionPlane(0,0,0,1000);
	FX::SetParameter(g_FxParams.planeReflection, &preRenderReflectionPlane);

	m_pViewManager->GetView(viewIndex)->Schedule();
}

void C_GameWorld::PreRenderSplitscreen()
{
	const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
	m_pViewManager->GetView(viewIndex)->PreRenderSplitscreen();
}

void C_GameWorld::RenderSplitscreenWithAA()
{
	const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
	
	// from post process layer
	m_pPostProcessMgr->SetView(Display::Device()->GetView()->GetIndex());
	
	GameDebug::SetShaderDebugToggles();

	m_pViewManager->GetView(viewIndex)->RenderWithAA();

	m_pWind->DebugRender();
}

void C_GameWorld::RenderSplitscreenWithoutAA()
{
	const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();

	ViewDependentFXUpdate(viewIndex);

	// render the light flares - must be done before they're rendered in the scene graph but after with aa
	DEVICE_MARKER("RenderSplitscreenWithoutAA.LightFlares");
	m_pLightEffectManager->Render(m_pViewManager->GetView(viewIndex)->GetMainView().GetTypeFlag());

	m_pViewManager->GetView(viewIndex)->RenderWithoutAA();

	if (RacingPhysics::g_displayDebugVehicle)
		GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_VehicleMessage_DebugRender );
}

void C_GameWorld::EndRenderSplitscreenWithoutAA()
{
#if PLATFORM_PC
	// [Blur2PC] Retail 0x0067a3c0 refreshes view-dependent FX before post-processing (T232).
	ViewDependentFXUpdate(Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView());
#endif

	C_GameWorldPostProcess::EndRenderSplitscreenWithoutAA();

#if PLATFORM_PC
	// [Blur2PC] T232: recovered retail EndRenderSplitscreenWithoutAA, 0x00420a70 ->
	// 0x0067a3c0. After post-processing, single-view detail 2-4 restores the prepass
	// depth/stencil for view index 1. The fullscreen UI uses this scene stencil.
	if ((m_pViewManager->GetNumViews() == 1) && m_GraphicalDetail > 1)
		Display::Device()->SetDepthStencilSurface(m_pViewManager->m_pLightingPrepass->GetDepthStencilSurface(1));
#endif

}

void C_GameWorld::RenderFullscreen()
{

}

void DeferredImpactProcess(Scheduler::Task* pTask)
{
	PERF_EVENT("DeferredImpactProcess");
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->ProcessImpactMessages();
}

JQ_DECLARE_PPU_MODULE(DeferredImpactProcessTask, Game::DeferredImpactProcess);

Debug::Bool g_bDeferredImpactProcess("Amax.DeferredImpactProcess", true);

#ifndef CFG_GOLD
Debug::Bool g_debugSpewLightManagerInfo("Graphics.SceneGraph.SpewLightManagerInfo", false);
#endif

void C_GameWorld::Update()
{
	PERF_EVENT("C_GameWorld::Update");

	CalcNumberOfSteps();

#if PLATFORM_PS3
	SpuDamage::C_ContextManager::ZeroRunJobsCount();
#endif

	// update game frame logic
	m_GameFrame++;

	{
		Time::C_SimpleTimer timer;

		Scheduler::Flush(m_RaytraceGroup);

		float tDuration = timer.Stop();
		AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kGeneric, tDuration);
	}


	if (m_RaceLoaded)
		//Cars are updated
		BeginCarEffectTasks();

	{
		PERF_EVENT("Update.ResetEnvSettings");
		ResetEnvironmentSettings();
	}

	float fTimeStep = C_GameApp::GetInstance()->GetTimeStep();
	
	if(!C_GameApp::GetInstance()->IsPaused())
	{
		// If more things need an early preupdate for perf, then maybe make this a message.
		Game::C_PickupsManager::Instance().PreUpdateOne();

		// HACK: Passing a cube environment map into driver system.
		{
			// set parameters
			const void* pEnvTexAddress = m_pSceneEnvMapGPUTexture;
			C_DriverManager::Instance()->SetVehicleCubeMapTexture(pEnvTexAddress);
		}

		UpdateVehicleParticleScale();
		{
			PERF_EVENT("Update.ActorListUpdateSystem");

			// Hopefully this is in the right place and passing the right stuff - We seem to be missing a lot of stuff thats in NightWing
			m_pActorList->UpdateSystem( fTimeStep, m_GameFrame );			// Hopefully this is in the right place and passing the right stuff - We seem to be missing a lot of stuff thats in NightWing
		}

		if (g_DebugDynamicObjectsRender)						// Display dynamic object debug info
			::Actors::C_ActorUtils::Instance().RenderDynamicObjectDebugInfo();

#if !defined(CFG_GOLD) && !defined(CFG_PROFILE)
		{
			DynamicEffect::Light::XtInterface::Update();
			DynamicEffect::Particle::XtInterface::Update();
			DynamicEffect::Track::XtInterface::Update();
		}
#endif


//		if (g_SorceryRecordsModified)							// Notify dynamic actors etc that sorcery data has been changed
//		{
//			GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_DynamicObjMsg_SorceryUpdate );
//		}

		if (C_GameApp::GetInstance()->GetRaceBookLayer() && C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->ClearImpactMessages();

		UpdatePhysics();

		::Actors::C_ActorUtils::Instance().RemoveDestroyedTriggers();				// Should this be processed here?

		if (C_GameApp::GetInstance()->GetRaceBookLayer() && C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
		{
			if (g_bDeferredImpactProcess)
				Scheduler::AddTask(Game::DeferredImpactProcessTask, 0, 0, m_ImpactProcessGroup);
			else
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->ProcessImpactMessages();	

			C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->ProcessCachedEvents();
		}	

		Components::C_VehicleComponent_PlayerRubberBanding::SendGroupDataToComponents();
		m_pParticleManager->UpdateParticles(fTimeStep, 1.0f / fTimeStep);

		// If more things need an early preupdate for perf, then maybe make this a message.
		Game::C_PickupsManager::Instance().PreUpdateTwo();
	}

	GlobalUpdate();
	
	if(!C_GameApp::GetInstance()->IsPaused())
	{
		F32 baseRate = 1.0f / 60.0f;
		if (g_TimeStep50Hz && Display::Device()->GetRefreshRate() == Display::E_RefreshRate_50HZ)
			baseRate = 1.0f / 50.0f;

		F32 gamePeriod = m_GameStep * baseRate;

		if (g_MakeSlowmoCaptureWork)
			gamePeriod = C_GameApp::GetInstance()->GetTimeStepRaw();

		AmaxCam::CameraSystem().CameraChangeUpdate(gamePeriod);

		{	
			{
				Time::C_SimpleTimer timer;

				Scheduler::Flush(m_ImpactProcessGroup);

				{
					PERF_EVENT("Animation.FirstFlush");
					// Flush pending animation tasks if BeginFrame() wasn't called e.g. during loading
					Animation::TaskManager().Flush();
					Animation::TaskManager().ClearStats();
					AnimatedModel::TaskManager().Flush();
				}

				float tDuration = timer.Stop();
				AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kGeneric, tDuration);
			}

			PERF_EVENT("Update.BroadcastActorUpdate");
			
			Time::C_SimpleTimer broadcastActorUpdateTimer;

			m_pActorList->Update(fTimeStep);

			GetActorList()->GetSubscriberSystem().BroadcastMessage(Game::E_GameMsg_UpdateLifeTime);

			C_ActivePhysicsBodyUpdateMsg activePhysicsBodyUpdateMsg( fTimeStep );
			GetActorList()->GetSubscriberSystem().BroadcastMessage( &activePhysicsBodyUpdateMsg );

			GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_DynamicObjMsg_AnimationUpdate );


// CLEAR
			GameMessaging::C_PostUpdateMsg postUpdateMsg( fTimeStep );
			GetActorList()->GetSubscriberSystem().BroadcastMessage( &postUpdateMsg );

			float broadcastActorUpdateTimerDuration = broadcastActorUpdateTimer.Stop();
			AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kActorUpdate, broadcastActorUpdateTimerDuration);
		}

		SpecialFX::C_AnimatedMeshSpecialFX::PostUpdate();

		{
			PERF_EVENT("Update.DriverManagerUpdate");
			m_pDriverManager->UpdateDrivers(fTimeStep);
		}

		if (m_pCrowdSystem && m_RaceLoaded)
		{
			PERF_EVENT("Update.CrowdSystemUpdate");

			m_pCrowdSystem->Update(Maths::HVec3(0.f), fTimeStep);
		}

		{
			Time::C_SimpleTimer timer;

			Game::Actors::RaceLogicCore* pRLC = NULL;
			if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() && 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore())
			{
				pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				for (S32 iCar = 0; iCar < pRLC->GetNumVehicles(); iCar++)
				{
					Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iCar);
					if (pActor && pActor->GetSchedulerGroup() != Scheduler::JQ_GROUP_NONE)
						Scheduler::Flush(pActor->GetSchedulerGroup());
				}
			}			
			
			float tDuration = timer.Stop();
			AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kGeneric, tDuration);
		}

		if(!C_GameApp::GetInstance()->IsPaused())
		{
			GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_VehicleMessage_PostUpdate );
		}

		{
			PERF_EVENT("Flush.VehicleAudioComponent");
			Scheduler::Flush(Game::Actors::C_VehicleActor::GetAudioSchedulerGroup());
		}

		m_pParticleManager->UpdateEmitters();
		m_pParticleManager->UpdateSuperEmitters();
		
		{
			PERF_EVENT("PickUpSubsystem.Flush");
			Scheduler::Flush(PickUpSubsystem::GetSchedulerGroup());
		}

		{
			Time::C_SimpleTimer dynEffectTimer;

			m_pDynamicEffectRenderer->Update(C_GameApp::GetInstance()->GetElapsedRaceTime(), fTimeStep, Framework::C_Application::GetInstance()->GetTimer().IsPaused());

			float dynEffectDuration = dynEffectTimer.Stop();
			AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kDynEffectRenderUpdate, dynEffectDuration);
		}

		m_pParticleManager->UpdateSpawn();

		m_pWind->Update(C_GameApp::GetInstance()->GetElapsedRaceTime(), fTimeStep);
		
		{
			PERF_EVENT("Update.CameraWorldUpdate");
			AmaxCam::CameraSystem().WorldUpdate(gamePeriod);
		}

		{
			PERF_EVENT("Update.CameraWorldUpdate");
			AmaxCam::CameraSystem().WorldUpdate2(gamePeriod);
		}

		ActorMsg::C_CameraUpdateMsg cameraUpdateMsg( Maths::Min(m_GameStep, g_maxPhysicsUpdates) / 60.f );
		GetActorList()->GetSubscriberSystem().BroadcastMessage( &cameraUpdateMsg );

		{
			PERF_EVENT("Update.SetEnvSettings");
			if (m_pSorceryEnvironmentSettings)
				m_pSorceryEnvironmentSettings->Set();  //For cubemap
			if (m_pCrowdFlagRenderer)
			{
				m_pCrowdFlagRenderer->SetFogColour(::EnvironmentSettings::FogEnvSettings::m_FogSettings.m_vFogColour);
				m_pCrowdFlagRenderer->SetDepthFogSettings(::EnvironmentSettings::FogEnvSettings::m_FogSettings.m_vDepthFogSettings);
			}
		}

		m_pParticleManager->UpdateFinal();

#ifndef CFG_GOLD
		if (g_debugSpewLightManagerInfo && GetSceneGraph() && GetSceneGraph()->GetLightManager())
			GetSceneGraph()->GetLightManager()->LogDebug();
#endif

		m_pLightTrails->Update();
	}
	else
	{
		GameMessaging::C_PausedUpdateMsg pausedUpdateMsg( fTimeStep );
		GetActorList()->GetSubscriberSystem().BroadcastMessage( &pausedUpdateMsg );
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&pausedUpdateMsg);
	}

	// Only SFX Manager needs this at present - and it needs it outside the paused loop for photo mode
	GameMessaging::C_FinalUpdateMsg finalUpdateMsg( fTimeStep );
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &finalUpdateMsg );

	//m_pAuxFlares->Build();
}

void C_GameWorld::CalcNumberOfSteps()
{
	// When a fractional amount of time passes (for example more than 1/30) we accumulate the
	// fractional amount, and then add an extra step when it reached a threshold:
	
	F32 period = C_GameApp::GetInstance()->GetTimeStepRaw();
	
	m_GameFraction += period;

	F32 baseRate = 60.0f;
	Display::E_RefreshRate screenRate = Display::Device()->GetRefreshRate();

	if (g_TimeStep50Hz && screenRate == Display::E_RefreshRate_50HZ)
		baseRate = 50.0f;

	// Find integer number of timesteps:
	m_GameStep = (S32)((m_GameFraction * baseRate) + 0.5f);
	m_GameFraction -= (F32)m_GameStep / baseRate;

#if CFG_PROFILE
	// For 50hz it's important to have more than 2 physics updates per frame in order to run at the right speed.
	// This code will allow Jan to still profile the game (if he runs in 60hz), while automatically
	// allowing a Profile build to run at the correct speed if we need 50hz:
	static bool firstTime = true;
	if (firstTime && screenRate == Display::E_RefreshRate_50HZ && g_maxPhysicsUpdates < 3)
		g_maxPhysicsUpdates = 3;
	firstTime = false;
#endif
	
	// Don't do this here as network timers will go out of synch.
	//if (m_GameStep > g_maxPhysicsUpdates)
	//	m_GameStep = g_maxPhysicsUpdates;
}

Debug::Logger& log = Debug::GetLogger("amax.timer");

void C_GameWorld::UpdateGroundPlane()
{
	Gravity::C_Body* pPlaneBody = m_pPhysicsScene->GetGroundPlane();

	if (!pPlaneBody && g_debugPhysicsPlane)
	{
		Gravity::C_PlaneDesc planeDesc;
		float y = g_debugPhysicsPlaneY;
		float x = sqrt(1.f - y * y);
		planeDesc.m_Normal = Maths::C_Normal(x, y, 0.f);
		planeDesc.m_Distance = 0.f;
		planeDesc.m_PhysicsMaterialIndex = Game::E_PhysicsMaterial_Default;
		planeDesc.m_EffectMaterialIndex = Game::E_EffectMaterial_Tarmac;

		Gravity::C_Body* pPlaneBody = m_pPhysicsScene->CreateBody();
		Gravity::C_BodyHelper helper(&planeDesc);
		Maths::HMat33 identity;
		identity.SetIdentity();

		helper.CreateBody(0.f, Maths::HVec3(0.f, 0.f, 0.f), identity, pPlaneBody, &Game::PhysicsConstants::kGroundPlaneFilter, 0, 0);
		SharedPhysics::C_Deactivator deactivator;
		pPlaneBody->SetDeactivator(deactivator);
		pPlaneBody->MakeStatic();

		m_pPhysicsScene->SetGroundPlane(pPlaneBody);
	}
	else if (pPlaneBody && !g_debugPhysicsPlane)
	{
		m_pPhysicsScene->SetGroundPlane(0);
	}
}

void C_GameWorld::UpdatePhysics()
{
	CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.Update.Physics");

	Gravity::C_Scene* pPhysicsScene = GetPhysicsScene();

	if( !pPhysicsScene || !m_RaceLoaded || 
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetCutSceneState() == Game::E_CutSceneState_OutroPrepping)
		return;

	PERF_EVENT("Update.UpdatePhysics");

	UpdateGroundPlane();
	pPhysicsScene->SetMinTimeStep(Maths::HF32(-1.f));
	if (g_MakeSlowmoCaptureWork)
	{
		F32 period = C_GameApp::GetInstance()->GetTimeStepRaw();
		AIAmax::AmaxAIR::Instance().SetTimeStep(period);
		AIAmax::AmaxAIR::Instance().Update(true);
		RaceIntroOutro::Instance().OverideControls();
#ifndef CFG_GOLD
		C_ReplayDirector::Instance().Update(period);
#endif
		RacingPhysics::PhysicsTimer::Instance().UpdatePhysicsTimer();
		{
			Time::C_SimpleTimer physicsSimulateTimer;

			PERF_EVENT("Update.PhysicsSimulate");
			pPhysicsScene->Simulate(period);

			float physicsSimulateDuration = physicsSimulateTimer.Stop();
			AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kPhysicsUpdate, physicsSimulateDuration);
		}
		RacingPhysics::g_dynamicsChanged = false;
		return;
	}

	F32 baseRate = 1.0f / 60.0f;
	if (g_TimeStep50Hz && Display::Device()->GetRefreshRate() == Display::E_RefreshRate_50HZ)
		baseRate = 1.0f / 50.0f;

	float step = baseRate * g_DebugPlaybackRate * C_GameApp::GetInstance()->GetTimeScale();
	
	AIAmax::AmaxAIR::Instance().SetTimeStep(step);
	RacingPhysics::PhysicsTimer::Instance().SetPhysicsUpdateTime(step);
	for (S32 i = 0; i < m_GameStep; i++)
	{
		if (i < g_maxPhysicsUpdates)
		{
			{
				Time::C_SimpleTimer aiTimer;

				PERF_EVENT("Update.AIUpdate");
				AIAmax::AmaxAIR::Instance().Update( (i&1) == 0 );

				float aiDuration = aiTimer.Stop();
				AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kAiUpdate, aiDuration);
			}

			RaceIntroOutro::Instance().OverideControls();

	#ifndef CFG_GOLD
			C_ReplayDirector::Instance().Update(step);
	#endif
			RacingPhysics::NetworkTests();
			RacingPhysics::PhysicsTimer::Instance().UpdatePhysicsTimer();

			{
				Time::C_SimpleTimer physicsSimulateTimer;

				PERF_EVENT("Update.PhysicsSimulate");
				pPhysicsScene->Simulate( step );

				float physicsSimulateDuration = physicsSimulateTimer.Stop();
				AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kPhysicsUpdate, physicsSimulateDuration);
			}
		}
		else
		{
			RacingPhysics::PhysicsTimer::Instance().UpdatePhysicsTimer();
		}
	}

	RacingPhysics::g_dynamicsChanged = false;
}

bool C_GameWorld::LevelSceneManagerUpdate()
{
	PERF_EVENT("Update.LevelSceneManagerUpdate");

	Time::C_SimpleTimer levelSceneManagerUpdateTimer;

	if (m_cutsceneVideoPlaying
		|| (m_waitForStreamer && !m_startStreamer)
		|| (!m_waitForStreamer && !m_allowedToStream))
		return true;

	m_pLevelSceneManager->Update();

	S32 numViews = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();

	Btl::Vector<RenderingData::Target::RequestInfo> requests;
	requests.Resize(numViews * 3);
	S32 j = 0;

	for (int i = 0; i < numViews; i++)
	{
		const SceneGraph::C_View& view(m_pViewManager->GetView(i)->GetMainView());
		const Framework::C_View& frameworkView = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i);

		m_pLevelSceneManager->UpdateView(view);
		m_pViewManager->GetView(i)->UpdateView();

		// current
		requests[j].cameraToWorld = view.GetViewSettings3D().GetCameraToWorld();
		requests[j].drawDistance = view.GetViewSettings3D().GetFarClip();
		requests[j].pvsCell = view.GetPvsCell();
		requests[j].fovY = view.GetViewSettings3D().GetFieldOfView();
		requests[j].aspect = view.GetViewSettings3D().GetAspectRatio();
		requests[j].resolution = frameworkView.m_View.GetViewport().GetW() / 1280.f;
		requests[j].directional = true;
		requests[j].mainView = (i == 0);
		j++;

		// future
		if (g_useFutureCamera)
		{
			requests[j].cameraToWorld =  Maths::HMat44(view.GetFutureCamera());
			requests[j].drawDistance = view.GetViewSettings3D().GetFarClip();
			requests[j].pvsCell = (view.GetFuturePvsCell() > -1) ? view.GetFuturePvsCell() : view.GetPvsCell();
			requests[j].fovY = view.GetViewSettings3D().GetFieldOfView();
			requests[j].aspect = view.GetViewSettings3D().GetAspectRatio();
			requests[j].resolution = frameworkView.m_View.GetViewport().GetW() / 1280.f;
			requests[j].directional = true;
			requests[j].mainView = false;
			j++;
		}

		// behind
		if (g_useStreamBehind)
		{
			requests[j].cameraPosition = Maths::HVec3(view.GetViewSettings3D().GetCameraToWorld().AW());
			requests[j].drawDistance = view.GetViewSettings3D().GetFarClip();
			requests[j].pvsCell = view.GetPvsCell();
			requests[j].resolution = 1 / 1024.f;
			requests[j].directional = false;
			requests[j].mainView = false;
			j++;
		}
	}

	bool result = GetLevelSceneManager()->UpdateStreaming(requests.First(), j);

	float levelSceneManagerUpdateTimerDuration = levelSceneManagerUpdateTimer.Stop();
	AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kLevelSceneUpdate, levelSceneManagerUpdateTimerDuration);


	return result;
}

//Handle case of going back into the UI from a race.
void C_GameWorld::HandleUISceneReady()
{
	m_HandleUISceneReady = false;

	Resource::C_Manager::GetInstance()->WaitForAllResourceData();

	//Syncronous - ensure we don't thrash.
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->LoadUILutRamPak();

	//Sync load of audio data when we're hidden by loading screen to avoid thrashing.
	Audio::InterfaceController::Get().BroadcastMessage(Audio::ePreLoadFrontEndUI, 0xffffffff);

	Resource::C_Manager::GetInstance()->SetBackgroundLoadingMode(Resource::E_BackgroundLoading_Enabled);
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StartGameRendering );
	C_GameApp::GetInstance()->DisableLoadingScreen();
	LOGINFO(g_Log, "C_GameWorld::GlobalUpdate() E_GameflowMessage_StartFrontendAudio.");
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StartFrontendAudio );
}

// this update doesn't get paused
void C_GameWorld::GlobalUpdate()
{		
	if (g_SorceryRecordsModified)							// Notify dynamic actors etc that sorcery data has been changed
		if (GetActorList())
			GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_DynamicObjMsg_SorceryUpdate );

	Lod::C_LodManager::Instance().Update(C_GameApp::GetInstance()->GetTimeStep(), !C_GameApp::GetInstance()->IsPaused());
	
	//If going back into the UI from a race.
	if (m_HandleUISceneReady)
		HandleUISceneReady();

	Game::E_LoadState loadState = C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState();

	// Allows us to disable the streamer making requests while we load
	if (m_allowedToStream && !m_waitForStreamer && !m_cutsceneVideoPlaying)
	{
		RenderingData::Target::StreamManager::Instance().SetPreLoad(false);

		FX::LockSerialiseMutex();
		LevelSceneManagerUpdate();
		FX::ReleaseSerialiseMutex();
	}
	else if (m_waitForStreamer && m_startStreamer && !m_cutsceneVideoPlaying)
	{
		if (!m_streamerStartedLoading)
		{
			//Pause resource loading to allow streamer to get exclusive disc access. Don't pause on PS3 as level data
			//and models are on different drives.
#ifndef PLATFORM_PS3
			Resource::C_Manager::GetInstance()->PauseLoadingThread();
#endif


			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_StreamerStarted );
			m_streamerStartedLoading = true;
			LOGINFO(g_Log, "C_GameWorld::GlobalUpdate() Streamer started!");
		}

		FX::LockSerialiseMutex();

		//Lolz, this is an interesting way of doing things...
		bool streamerSuccess = LevelSceneManagerUpdate();
		bool streamerBusy = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetLevelResource()->GetRenderLevel()->GetScene().GetStreamData().Busy();

		if ((!streamerSuccess || !streamerBusy) && !m_streamerFinishedLoading)
		{
			LOGINFO(g_Log, "C_GameWorld::GlobalUpdate() Streamer finished!");
			m_streamerFinishedLoading = true;
			m_waitForStreamer = false;

#ifndef PLATFORM_PS3
			//Streamer's done, resume resource loading.
			Resource::C_Manager::GetInstance()->ResumeLoadingThread();
#endif
		}

		FX::ReleaseSerialiseMutex();
	}

	//Yet another bool. This one ensures we only drop into the below code once. As in, we only try to start the race/ui scene once.
	if (!m_letsGoRacing)
	{
#ifndef PLATFORM_PS3
		bool resourceManagerFinishedLoading = Resource::C_Manager::GetInstance()->GetLoadQueueSize() > 0 ? false : true;
#else
		//Proceed, models and level streaming can co-exist happily as on difference drives. 
		bool resourceManagerFinishedLoading = true;
#endif
		//If we're done loading our resources and the streamer is ready, let's go racing!
		if (resourceManagerFinishedLoading && m_streamerFinishedLoading)
		{
			LOGINFO(g_Log, "C_GameWorld::GlobalUpdate() Let's go racing!");
			m_letsGoRacing = true;

			//Start syncronous in-game audio preload if we're going into a race as it's okay to use the
			//main thread at this point.
			if (loadState == Game::E_LoadState_IntoRace) {
				Audio::InterfaceController::Get().BroadcastMessage(Audio::ePreLoadInGameUI, 0xffffffff);
			}

			Resource::C_Manager::GetInstance()->WaitForAllResourceData();

			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StreamerFull );
	
			if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetFrontEndLoading())
				C_GameApp::GetInstance()->DisableLoadingScreen();
			else
			{
				if (loadState != Game::E_LoadState_IntoRace &&
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetResourcesLoaded())
				{		
#ifdef PLATFORM_PS3
					if (!C_PS3InstallData::Instance().IsCopyInProgress())
#endif
					{
						_UI_MESSAGE( UI::E_UiGameMsg_UISceneReady );
					}
				}

				C_GameApp::GetInstance()->GetRaceBookLayer()->SetFrontEndLoading(false);
			}

			if (loadState == Game::E_LoadState_IntoUI)
			{
				Resource::C_Manager::GetInstance()->SetBackgroundLoadingMode(Resource::E_BackgroundLoading_Enabled);
				LOGINFO(g_Log, "C_GameWorld::GlobalUpdate() E_GameflowMessage_StartFrontendAudio.");
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StartFrontendAudio );
			}
		}
	}
	
#ifndef CFG_SPEED
	if (g_debugRecreateHeadlightTexture)
	{
		#if PLATFORM_PS3
		Display::Device()->BlockUntilIdle();
		#endif
		DestroyTextureHeadlight();
		CreateTextureHeadlight();
		g_debugRecreateHeadlightTexture = false;
	}
#endif

	// PR - update photomode filters
	UI::C_UiPhotoMode::Instance().UpdateFilters();
#ifndef CFG_GOLD
	UpdateCubemapOverride();
#endif

	
}

void C_GameWorld::PostRender()
{
	m_pSceneGraph->EndFrame();
	//RenderingData::Target::StreamManager::Instance().FlushTasks();
	RenderingData::Target::EndFrame();
}

void C_GameWorld::Resize()
{
	C_GameWorldPostProcess::Resize();


	if(m_pLightingPrepass)
		m_pLightingPrepass->Resize();

	// make sure debug feeds get reattached and FOV gets updated
	S32 numViews = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	AmaxCam::CameraSystem().SetViewConfig(numViews);
	AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();
	for(S32 i = 0; i < numViews; ++i)
		cameraSystem.DebugFeedShow( i, AmaxCam::E_DebugCam_GameFeed, i);
}

struct C_LightSort
{
	Maths::HVec3 m_Position;
	F32			 m_Intensity;
	F32			 m_Distant;
	Maths::HVec3 m_Colour;
};

Debug::Bool g_OnlyDistant("Graphics.Render.Deferred.OnlyDistant", false);

int LightSortFunc(const void *A, const void *B)
{
	C_LightSort* pA = (C_LightSort*)A;
	C_LightSort* pB = (C_LightSort*)B;

	if (g_OnlyDistant)
	{
		if ((pA->m_Distant)  > (pB->m_Distant))
			return 1;
		else
			return -1;
	}
	else
	{
		if ((pA->m_Intensity / pA->m_Distant)  < (pB->m_Intensity / pB->m_Distant))
			return 1;
		else
			return -1;
	}
	
}

bool C_GameWorld::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	switch (pMessage->Type())
	{
	case Game::E_VehicleMessage_VehicleFocusPosition:
		{
			PERF_EVENT("Update.VehicleFocusPositionMessage");

			const Game::Actors::C_VehicleFocusPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFocusPositionMsg>(pMessage);

			if(m_pViewManager->GetCubeMap())
				m_pViewManager->GetCubeMap()->SetPosition(pMsg->mWorldMatrix.Translation());
			
			// FIX ME
			if(m_pViewManager->GetView(0)->GetMirror())
			{
				Maths::HVec3 cameraPosition = Maths::HVec3(C_GameVehicleData::Instance().mVehicleDefaults[0]->mirror.offset.vx, C_GameVehicleData::Instance().mVehicleDefaults[0]->mirror.offset.vy, C_GameVehicleData::Instance().mVehicleDefaults[0]->mirror.offset.vz);
				m_pViewManager->GetView(0)->GetMirror()->SetupMirror(cameraPosition * pMsg->mWorldMatrix, -pMsg->mWorldMatrix.AZ(), pMsg->mWorldMatrix.AY(), C_GameVehicleData::Instance().mVehicleDefaults[0]->mirror.fieldOfView);
			}
			
			return true;
		}
	case Game::E_GameflowMessage_RaceLoaded:
		{
			if (!m_pAuxFlares->IsCreated())
				m_pAuxFlares->Create();
			m_RaceLoaded = true;
	
			//Get scene env-map texture
			{
				RenderingData::Target::SharedTextureData *pTex = GetLevelSceneManager()->GetWorldRenderScene()->GetLightSamplePointScene().GetClosestCubemap( Maths::Constants<Maths::HVec3>::Zero() );
				if (pTex != m_pSceneEnvMapTexture)
				{
					if (m_pSceneEnvMapTexture)
						WorldRender::C_LocalisedCubemapData::ReleaseGPUTexture(m_pSceneEnvMapTexture);
					if (pTex)
						m_pSceneEnvMapGPUTexture = WorldRender::C_LocalisedCubemapData::GetGPUTexture(pTex);
					else
						m_pSceneEnvMapGPUTexture = FX::GetDefaultGPUTexture();
					m_pSceneEnvMapTexture = pTex;
				}
			}
			return true;
		}
	case Game::E_GameflowMessage_RestartLevel:
		{
			m_RaceLoaded = false;
		}
		break;
	case E_GameflowMessage_CutsceneVideoFinished:
		m_cutsceneVideoPlaying = false;
		LOGINFO(g_Log, "C_GameWorld::MessageHandler() Video finished, streaming enabled.");
		break;
	case Game::E_GameflowMessage_StartGameRendering:
		m_allowedToStream = true;
		break;
	case E_GameflowMessage_CutsceneVideoStarted:
		m_cutsceneVideoPlaying = true;
		LOGINFO(g_Log, "C_GameWorld::MessageHandler() Video started, streaming disabled.");
		break;
	case Game::E_GameflowMessage_StopGameRendering:
		m_allowedToStream = false;
		break;
	case Game::E_GameflowMessage_FillStreamer:
		m_startStreamer = true;
		RenderingData::Target::StreamManager::Instance().SetPreLoad(true);
		break;
	}
	return true;
}


const Display::C_RenderTexture2D* C_GameWorld::GetMirrorTexture(S32 view) const
{
	if(m_pViewManager->GetView(view)->GetMirror())
		return m_pViewManager->GetView(view)->GetMirror()->GetRenderTexture();
	else
		return NULL;
}

const Display::C_RenderTexture2D* C_GameWorld::GetReflectionTexture(S32 view) const
{
	if(m_pViewManager->GetView(view)->GetReflection())
		return m_pViewManager->GetView(view)->GetReflection()->GetRenderTexture();
	else
		return NULL;
}

void C_GameWorld::SetMaxBlobShadows( U32 maxBlobShadows )
{
	//Depends of the num views, reduce it
	if (maxBlobShadows > (U32(m_pViewManager->GetNumViews()) - 1))
		m_MaxBlobShadowCasters = maxBlobShadows - U32(m_pViewManager->GetNumViews()) + 1;
	else
		m_MaxBlobShadowCasters = 0;
}

void C_GameWorld::SetLevelLightFlareParams(const C_LightFlareSettings& lightFlareSettings)
{
	m_LevelLightFlareSettings = lightFlareSettings;
}

void C_GameWorld::SetCarLightFlareParams(const C_CarLightFlareSettings& lightFlareSettings)
{
	m_CarLightFlareSettings = lightFlareSettings;
}


void C_GameWorld::SetCarLightSettings( const C_CarLightSettings& carLightSettings )
{
	m_CarLightSettings = carLightSettings;
}
//////////////////////////////////////////////////////////////////////////
// Car Effect Tasks
//////////////////////////////////////////////////////////////////////////
void C_GameWorld::BeginCarEffectTasks()
{
	PERF_EVENT("BeginCarEffectTasks");

	if (m_bRunningCarEffectTasks || m_pLevelSceneManager->GetWorldRenderScene()->GetRenderLevelInstanceList().Size() == 0)
		return;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	
	if (!pRLC)
		return;

	m_bRunningCarEffectTasks = true;

	S32 oldViewIndex[20];
	//Calculate shadow casters
	{
		m_CarDynamicEffectsTaskInit.m_pGameWorld = this;
		m_CarDynamicEffectsTaskInit.m_pViewManager = m_pViewManager;
		for (S32 i = 0; i < m_pViewManager->GetNumViews(); i++)
		{
			m_CarDynamicEffectsTaskInit.m_pView[i] = m_pViewManager->GetView(i);
			m_CarDynamicEffectsTaskInit.m_pView[i]->m_MaxShadowCasters = m_MaxBlobShadowCasters;
		}
		m_CarDynamicEffectsTaskInit.m_NumViews = m_pViewManager->GetNumViews();

		m_CarDynamicEffectsTaskInit.m_pSceneGraph = m_pSceneGraph;
		m_CarDynamicEffectsTaskInit.m_pLightList = &m_pLevelSceneManager->GetWorldRenderScene()->GetRenderLevelInstanceList().Begin()->GetRenderLevel()->GetLightList();
		
		//Dynamic SH light data, set positions of the car
		//Headlights fade system
		m_CarDynamicEffectsTaskInit.m_NumVehicles = pRLC->GetNumVehicles();
		for (U32 iVehicle = 0; iVehicle < m_CarDynamicEffectsTaskInit.m_NumVehicles; iVehicle++)
		{
			Game::Actors::C_VehicleActor* pVehicleActor = const_cast<Game::Actors::C_VehicleActor*>(pRLC->GetVehicleActor(iVehicle));
		
			if (!pVehicleActor || !pVehicleActor->Created())
				continue;

			//Get position
			m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_Position = pVehicleActor->GetMatrix().Translation();
			m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_NeedUpdateDynamicSH = pVehicleActor->IsVisible();
			F32 radius = pVehicleActor->GetVehicleModel()->GetHeadlight()->m_Length / 2.f;
			m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_HeadLightRadius = radius;
			m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_HeadLightPosition = pVehicleActor->GetMatrix().Translation() + pVehicleActor->GetMatrix().AZ() * radius;
			m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_LightFlags = 1<<pVehicleActor->GetVehicleIndex();

			//Set view index if focus vehicle
			oldViewIndex[iVehicle] = m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_ViewIndex;
			m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_ViewIndex = (pVehicleActor->IsFocusVehicle()) ? pVehicleActor->GetViewportID() : -1; //-1 not focus vehicle
		}

		//Copy dynamic lights visibility from last frame
		for (S32 i = 0; i < m_pViewManager->GetNumViews(); i++)
		{
			m_DynamicPointLightVector[i].Clear();
			m_DynamicSpotLightVector[i].Clear();

			const SceneGraph::C_VisiblePointLightList& visiblePointLights = m_pViewManager->GetView(i)->GetMainView().GetVisiblePointLightList();
			for (S32 j = 0; j < visiblePointLights.Size(); j++)
			{
				const SceneGraph::C_VisiblePointLight& visibleLight = visiblePointLights[j];

				m_DynamicPointLightVector[i].PushBack();
				m_DynamicPointLightVector[i].Back().m_Position = Maths::HVec3(visibleLight.m_pObject->m_Position.x, visibleLight.m_pObject->m_Position.y, visibleLight.m_pObject->m_Position.z);
				m_DynamicPointLightVector[i].Back().m_Colour = Maths::HVec3(visibleLight.m_pObject->m_Colour.x, visibleLight.m_pObject->m_Colour.y, visibleLight.m_pObject->m_Colour.z) * visibleLight.m_Intensity;
				m_DynamicPointLightVector[i].Back().m_Radius = visibleLight.m_pObject->m_Radius;
				m_DynamicPointLightVector[i].Back().m_Flags = visibleLight.m_pObject->m_Flags;
			}

			const SceneGraph::C_VisibleSpotLightList& visibleSpotLights = m_pViewManager->GetView(i)->GetMainView().GetVisibleSpotLightList();
			for (S32 j = 0; j < visibleSpotLights.Size(); j++)
			{
				const SceneGraph::C_VisibleSpotLight& visibleLight = visibleSpotLights[j];

				m_DynamicSpotLightVector[i].PushBack();
				m_DynamicSpotLightVector[i].Back().m_Position = Maths::HVec3(visibleLight.m_pObject->m_Position.x, visibleLight.m_pObject->m_Position.y, visibleLight.m_pObject->m_Position.z);
				const Maths::Mat44 mLightWorld = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&visibleLight.m_pObject->m_Matrix);
				m_DynamicSpotLightVector[i].Back().m_Direction	= Maths::HVec3(Maths::X(Maths::RowZ(mLightWorld)), Maths::Y(Maths::RowZ(mLightWorld)), Maths::Z(Maths::RowZ(mLightWorld)));
				m_DynamicSpotLightVector[i].Back().m_Colour = Maths::HVec3(visibleLight.m_pObject->m_Colour.x, visibleLight.m_pObject->m_Colour.y, visibleLight.m_pObject->m_Colour.z) * visibleLight.m_Intensity;
				m_DynamicSpotLightVector[i].Back().m_WorldLight = Maths::Inverse(mLightWorld);
				m_DynamicSpotLightVector[i].Back().m_Radius = visibleLight.m_pObject->m_Radius;
				m_DynamicSpotLightVector[i].Back().m_RenderFlags = visibleLight.m_pObject->m_RenderFlags;
				m_DynamicSpotLightVector[i].Back().m_HorizontalFoV = visibleLight.m_pObject->m_HorizontalFoV;
				m_DynamicSpotLightVector[i].Back().m_VerticalFoV = visibleLight.m_pObject->m_VerticalFoV;
				m_DynamicSpotLightVector[i].Back().m_Flags = visibleLight.m_pObject->m_Flags;
			}

			//Update task
			m_CarDynamicEffectsTaskInit.m_DynamicPointLightCount[i] = m_DynamicPointLightVector[i].Size();
			if (m_CarDynamicEffectsTaskInit.m_DynamicPointLightCount[i] > 0)
				m_CarDynamicEffectsTaskInit.m_DynamicPointLightVector[i] = &m_DynamicPointLightVector[i].At(0);
			m_CarDynamicEffectsTaskInit.m_DynamicSpotLightCount[i] = m_DynamicSpotLightVector[i].Size();
			if (m_CarDynamicEffectsTaskInit.m_DynamicSpotLightCount[i] > 0)
				m_CarDynamicEffectsTaskInit.m_DynamicSpotLightVector[i] = &m_DynamicSpotLightVector[i].At(0);
		}

		//Calculate fake light attenuation
		if ((m_pViewManager->GetNumViews() == 1))
		{
			bool enabled = m_CarLightSettings.m_SunFakeLightSize > 0.01f;
			Maths::C_RGBA_F32 colour = m_CarLightSettings.m_SunFakeLightColour;
			Maths::HVec3 position = m_CarLightSettings.m_SunFakeLightPosition;
			F32 size = m_CarLightSettings.m_SunFakeLightSize;
			
			if (enabled)
			{
				//Use direction to attenuate the light base of SH light
				if ((pRLC && (pRLC->GetNumVehicles() > 0)) || C_Game::GetInstance()->GetLevelState().mInUI)
				{

					S8 focusVehicleIndex = pRLC->GetViewportFocusVehicleIndex(0); //Only support single view
					if (focusVehicleIndex >= 0)
					{
						F32 light;
						if (C_Game::GetInstance()->GetLevelState().mInUI)
							light = 1.f;
						else
						{
							Game::Actors::C_VehicleActor* pVehicleActor = pRLC->GetVehicleActor(focusVehicleIndex);
							const WorldRender::C_LightSampleData& lightSampleData = pVehicleActor->GetVehicleModel()->GetLightSampler()->GetLightSampleData();
							
							Maths::HVec3 shadorCasterCentre = m_pViewManager->GetView(0)->GetReferencePosition(); //Only support single view

							Maths::HVec3 dirVector = position - shadorCasterCentre;
							dirVector.Normalise();

							//Calculate intensity from direction
							Maths::HVec4 redLightSH(lightSampleData.m_SampleData[0], lightSampleData.m_SampleData[1], lightSampleData.m_SampleData[2], lightSampleData.m_SampleData[3]);
							Maths::HVec4 greenLightSH(lightSampleData.m_SampleData[4], lightSampleData.m_SampleData[5], lightSampleData.m_SampleData[6], lightSampleData.m_SampleData[7]);
							Maths::HVec4 blueLightSH(lightSampleData.m_SampleData[8], lightSampleData.m_SampleData[9], lightSampleData.m_SampleData[10], lightSampleData.m_SampleData[11]);
							Maths::HVec4 normal = dirVector || Maths::HVecW(1.f);
							light = (normal | redLightSH) + (normal | greenLightSH) + (normal | blueLightSH);
						}	
						m_CarDynamicEffectsTaskInit.m_FakeLightColour = Maths::HVec4(colour.R() * colour.A(), colour.G() * colour.A(), colour.B() * colour.A(), light);
						m_CarDynamicEffectsTaskInit.m_FakeLightPosition = position;
					}
				}
				else
					m_CarDynamicEffectsTaskInit.m_FakeLightColour.W(-1.f);
			}
			else
				m_CarDynamicEffectsTaskInit.m_FakeLightColour.W(-1.f);
		}
		else
			m_CarDynamicEffectsTaskInit.m_FakeLightColour.W(-1.f);

		//Add task
		Scheduler::AddTask(CarDynamicEffectsTaskModule,&m_CarDynamicEffectsTaskInit, NULL, m_CarDynamicEffectsTaskGroup, C_SchedulerConfig::GetInstance()->GetCarDynamicEffectsPolicy());				
	}
	//Calculate light cubemaps
	#if !CFG_GOLD
	if(!g_bDisableVehicleCubemaps)
	#endif
	{
		U32 lightCubeMapIndexTable[4];
		for (U32 i = 0; i < 4; i++) lightCubeMapIndexTable[i] = 0;

		for (U32 iVehicle = 0; iVehicle < pRLC->GetNumVehicles(); iVehicle++)
		{
			Game::Actors::C_VehicleActor* pVehicleActor = const_cast<Game::Actors::C_VehicleActor*>(pRLC->GetVehicleActor(iVehicle));

			if (!pVehicleActor || !pVehicleActor->Created())
				continue;

			//Get view, one frame delay
			S32 viewIndex = oldViewIndex[iVehicle];
			if (viewIndex >= 0)
			{
				C_View* pView = m_pViewManager->GetView(viewIndex);

				pVehicleActor->UpdateLightCubeMap(AmaxCam::CameraSystem().DebugFeed(viewIndex).Live());

				if (pVehicleActor->NeedUpdateLightCubeMap())
				{
					//Need to update the cubemap
					Display::C_TextureCube* pLightCubeMap = pVehicleActor->GetCurrentLightCubeMap();

					//Get view, one frame delay
					C_CarUpdateLightCubeMapTaskInit& carUpdateLightCubeMapTaskInit = m_CarUpdateLightCubeMapTaskInit[viewIndex];
					U32& lightCubeMapIndex = lightCubeMapIndexTable[viewIndex];

					// [Blur2PC] Retail PC locks the six faces and fills the light cube maps too (Blur.exe 1.2
					// BeginCarEffectTasks 0x00420ac0 schedules CarUpdateLightCubeMapTaskModule 0x0041bb90); the snapshot
					// compiled this out on PC, so the cars' light environment (fake sun, nearby lights) stayed black.
					Display2::C_LockedRect lockedRect;

					lockedRect = Display2::LockTextureRectCube(pLightCubeMap, Display2::E_CubeMapFace_PosX, 0);
					BIZ_ASSERT(lockedRect.GetBits());
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_MemoryLocked[0] = lockedRect.GetBits();
					lockedRect = Display2::LockTextureRectCube(pLightCubeMap, Display2::E_CubeMapFace_PosY, 0);
					BIZ_ASSERT(lockedRect.GetBits());
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_MemoryLocked[1] = lockedRect.GetBits();
					lockedRect = Display2::LockTextureRectCube(pLightCubeMap, Display2::E_CubeMapFace_PosZ, 0);
					BIZ_ASSERT(lockedRect.GetBits());
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_MemoryLocked[2] = lockedRect.GetBits();
					lockedRect = Display2::LockTextureRectCube(pLightCubeMap, Display2::E_CubeMapFace_NegX, 0);
					BIZ_ASSERT(lockedRect.GetBits());
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_MemoryLocked[3] = lockedRect.GetBits();
					lockedRect = Display2::LockTextureRectCube(pLightCubeMap, Display2::E_CubeMapFace_NegY, 0);
					BIZ_ASSERT(lockedRect.GetBits());
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_MemoryLocked[4] = lockedRect.GetBits();
					lockedRect = Display2::LockTextureRectCube(pLightCubeMap, Display2::E_CubeMapFace_NegZ, 0);
					BIZ_ASSERT(lockedRect.GetBits());
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_MemoryLocked[5] = lockedRect.GetBits();
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_Pitch = lockedRect.GetPitch();
					// [Blur2PC][HDR] Carry the actual texture format into the worker's row packing.
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_FloatRGBM = pLightCubeMap->GetFormat() == Display2::E_TextureFormat_A16B16G16R16F;
					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_pTextureCube = pLightCubeMap;

					carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[lightCubeMapIndex].m_Position = pVehicleActor->GetMatrix().AW();
					lightCubeMapIndex++;
				}
			}
		}

		//Prepare task
		// [Blur2PC] scheduled on PC as well (retail 0x00420ac0, see above)
		
		//For each view
		for (S32 iView = 0; iView < m_pViewManager->GetNumViews(); iView++)
		{
			m_CarUpdateLightCubeMapTaskInit[iView].m_NumLightCubeMaps = lightCubeMapIndexTable[iView];
			U32 lightCubeMapIndex = lightCubeMapIndexTable[iView];
			m_CarUpdateLightCubeMapTaskInit[iView].m_RimGeneratorVector = m_pViewManager->GetView(iView)->m_RimGeneratorVector;

			const U32 numLightCubeMapsUpdatePerTask = 6;

			U32 nTasks = (lightCubeMapIndex / numLightCubeMapsUpdatePerTask) + 1;

			for (U32 i = 0; i < nTasks; i++)
			{
				U32 init = i * numLightCubeMapsUpdatePerTask;
				U32 end = ((i+1) * numLightCubeMapsUpdatePerTask);
				end = (end > lightCubeMapIndex)?lightCubeMapIndex:end;
				U32 lightCubeMapRange[2] = {init, end}; //Set range for this task
				Scheduler::AddTask(CarUpdateLightCubeMapTaskModule,&m_CarUpdateLightCubeMapTaskInit[iView], NULL, m_CarUpdateLightCubeMapTaskGroup, C_SchedulerConfig::GetInstance()->GetCarUpdateLightCubeMapPolicy(), lightCubeMapRange, 2 * sizeof(U32));				
			}
		}
	}
}

void C_GameWorld::EndCarEffectTasks()
{
	PERF_EVENT("EndCarEffectTasks");

	if (!m_bRunningCarEffectTasks)
		return;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	
	if (!pRLC)
		return;
	
	m_bRunningCarEffectTasks = false;

	//Wait for the task
	Scheduler::Flush(m_CarDynamicEffectsTaskGroup);
	Scheduler::Flush(m_CarUpdateLightCubeMapTaskGroup);

	if (g_RimDebug)
	{
		for (S32 iView = 0; iView < m_pViewManager->GetNumViews(); iView++)
		{
			for (U32 i = 0; i < m_pViewManager->GetView(iView)->m_NumRimGenerators; i++)
			{
				Render::Debug::Deferred()->Draw3dLine(m_pViewManager->GetView(iView)->GetReferencePosition(), m_pViewManager->GetView(iView)->m_RimGeneratorVector[i].m_Position, Maths::RGBA(0,0,255,255));
				Render::Debug::Deferred()->Draw3dStar(m_pViewManager->GetView(iView)->m_RimGeneratorVector[i].m_Position, 1.f, Maths::C_RGBA_F32(m_pViewManager->GetView(iView)->m_RimGeneratorVector[i].m_Colour.X(), m_pViewManager->GetView(iView)->m_RimGeneratorVector[i].m_Colour.Y(), m_pViewManager->GetView(iView)->m_RimGeneratorVector[i].m_Colour.Z(), 1.f));
			}
		}
	}

	{
		PERF_EVENT("EndCarEffectTasks");

		// Update shadow casters
		for (U32 iVehicle = 0; iVehicle < m_CarDynamicEffectsTaskInit.m_NumVehicles; iVehicle++)
		{
			Game::Actors::C_VehicleActor* pActor = const_cast<Game::Actors::C_VehicleActor*> (pRLC->GetVehicleActor(iVehicle));

			if (!pActor)
				continue;

			BlobShadows::C_DynamicBlobShadowInstance* pBlobInstance = pActor->GetBlobShadowInstance();

			if (pBlobInstance)
			{
				//Get view, one frame delay
				S32 viewIndex = m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_ViewIndex;
				if (viewIndex >= 0) //Is one frame delay, so first frame problem
				{
					C_View* pView = m_pViewManager->GetView(viewIndex);
					
					//Update blobshadow
					pActor->UpdateBlobShadow(AmaxCam::CameraSystem().DebugFeed(viewIndex).Live());

					pBlobInstance->ClearShadowCasters();

					//Add sun blob shadows if needed
					if (GetCarLightSettings().m_SunBlobShadowIntensity > 0.f)
					{
						Maths::HVec3 dirVector = (GetCarLightSettings().m_SunBlobShadowPosition - pActor->GetGroundPosition());
						dirVector.Normalise();
						//Calculate Sun Fade
						const WorldRender::C_LightSampleData& lightSampleData = pActor->GetVehicleModel()->GetLightSampler()->GetLightSampleData();

						//Calculate intensity from direction
						Maths::HVec4 redLightSH(lightSampleData.m_SampleData[0], lightSampleData.m_SampleData[1], lightSampleData.m_SampleData[2], lightSampleData.m_SampleData[3]);
						Maths::HVec4 greenLightSH(lightSampleData.m_SampleData[4], lightSampleData.m_SampleData[5], lightSampleData.m_SampleData[6], lightSampleData.m_SampleData[7]);
						Maths::HVec4 blueLightSH(lightSampleData.m_SampleData[8], lightSampleData.m_SampleData[9], lightSampleData.m_SampleData[10], lightSampleData.m_SampleData[11]);
						Maths::HVec4 normal = dirVector || Maths::HVecW(1.f);
						F32 light = Maths::Clamp(F32((normal | redLightSH) + (normal | greenLightSH) + (normal | blueLightSH)), 0.f, 1.f);

						BlobShadows::C_BlobShadowCaster caster;
						caster.m_Intensity = -GetCarLightSettings().m_SunBlobShadowIntensity * light;
						caster.m_Position = GetCarLightSettings().m_SunBlobShadowPosition;
						pBlobInstance->AddShadowCaster(caster);
					}


					for (U32 j = 0; j < pView->m_NumShadowCasters; j++)
					{
						BlobShadows::C_BlobShadowCaster caster;
						caster.m_Position = pView->m_ShadowCaster[j].m_Position;
						caster.m_Intensity = pView->m_ShadowCaster[j].m_Intensity;
						pBlobInstance->AddShadowCaster(caster);
					}		
				}
			}
		}

		//Update dynamic SH light data for each car
		for (U32 iVehicle = 0; iVehicle < m_CarDynamicEffectsTaskInit.m_NumVehicles; iVehicle++)
		{
			Game::Actors::C_VehicleActor* pVehicleActor = const_cast<Game::Actors::C_VehicleActor*>(pRLC->GetVehicleActor(iVehicle));

			if (!pVehicleActor || !pVehicleActor->Created())
				continue;

			if (m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_NeedUpdateDynamicSH)
			{	
				//Update Dynamic SH
				pVehicleActor->UpdateSHLightData(m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_DynamicSphericalHarmonicLight);
			}
			
			//Set headlights fade system
			pVehicleActor->GetVehicleModel()->GetHeadlight()->m_IntensityFade = m_CarDynamicEffectsTaskInit.m_VehicleInfo[iVehicle].m_HeadLightFadeIntensity;
		}
	}
	
	#if !CFG_GOLD
	if(!g_bDisableVehicleCubemaps)
	#endif
	{
		PERF_EVENT("Broken");

		//Unlock textures
		for (S32 iView = 0; iView < m_pViewManager->GetNumViews(); iView++)
		{
			C_CarUpdateLightCubeMapTaskInit& carUpdateLightCubeMapTaskInit = m_CarUpdateLightCubeMapTaskInit[iView];
			for (U32 i = 0; i < carUpdateLightCubeMapTaskInit.m_NumLightCubeMaps; i++)
			{
				Display2::UnlockTextureRectCube(carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[i].m_pTextureCube, Display2::E_CubeMapFace_PosX, 0);
				Display2::UnlockTextureRectCube(carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[i].m_pTextureCube, Display2::E_CubeMapFace_PosY, 0);
				Display2::UnlockTextureRectCube(carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[i].m_pTextureCube, Display2::E_CubeMapFace_PosZ, 0);
				Display2::UnlockTextureRectCube(carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[i].m_pTextureCube, Display2::E_CubeMapFace_NegX, 0);
				Display2::UnlockTextureRectCube(carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[i].m_pTextureCube, Display2::E_CubeMapFace_NegY, 0);
				Display2::UnlockTextureRectCube(carUpdateLightCubeMapTaskInit.m_LightCubeMapInfo[i].m_pTextureCube, Display2::E_CubeMapFace_NegZ, 0);
			}
		}
	}
	
}

Debug::Float g_debugTotalScreenSizeEmitScaleBegin("Vehicle.Particle.TotalScreenSizeScaleEmitBegin", 1.0f, Debug::Range(0.f, 10.f));
Debug::Float g_debugTotalScreenSizeEmitScaleEnd("Vehicle.Particle.TotalScreenSizeEmitScaleEnd", 2.0f, Debug::Range(0.f, 10.f));
Debug::Float g_debugTotalScreenSizeEmitScale("Vehicle.Particle.TotalScreenSizeEmitScale", 0.5f, Debug::Range(0.f, 10.f));

Debug::Float g_debugTotalScreenSizeLifeScaleBegin("Vehicle.Particle.TotalScreenSizeLifeScaleBegin", 1.0f, Debug::Range(0.f, 10.f));
Debug::Float g_debugTotalScreenSizeLifeScaleEnd("Vehicle.Particle.TotalScreenSizeLifeScaleEnd", 2.0f, Debug::Range(0.f, 10.f));
Debug::Float g_debugTotalScreenSizeLifeScale("Vehicle.Particle.TotalScreenSizeLifeScale", 0.5f, Debug::Range(0.f, 10.f));

void C_GameWorld::UpdateVehicleParticleScale()
{
	PERF_EVENT("Update.VehicleParticleScale");

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	U32 cars = C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars;

	F32 totalScreenSize = 0.0f;
	
	for (U32 c = 0; c < cars; ++c)
	{
		const Game::Actors::C_VehicleActor* pVehicleActor = pRLC->GetVehicleActor(c);
		
		if (!pVehicleActor || !pVehicleActor->Created())
			continue;

		totalScreenSize += pVehicleActor->GetScreenSize();
	}

	// scale effects detail down to g_debugTotalScreenSizeScale as totalScreenSize goes from g_debugTotalScreenSizeScaleBegin to g_debugTotalScreenSizeScaleEnd
	m_VehicleParticleEmitScale = 1.0f - Maths::Clamp((totalScreenSize - g_debugTotalScreenSizeEmitScaleBegin) / g_debugTotalScreenSizeEmitScaleEnd, 0.0f, 1.0f) * (1.0f - g_debugTotalScreenSizeEmitScale);
	m_VehicleParticleLifeScale = 1.0f - Maths::Clamp((totalScreenSize - g_debugTotalScreenSizeLifeScaleBegin) / g_debugTotalScreenSizeLifeScaleEnd, 0.0f, 1.0f) * (1.0f - g_debugTotalScreenSizeLifeScale);
}

void C_GameWorld::Exit()
{
	EndCarEffectTasks();
}

Display::C_TextureCube* C_GameWorld::GetVehicleLightCubeMap( U32 vehicleIndex, U32 doubleBufferIndex )
{
	BIZ_ASSERT(vehicleIndex < m_NumVehicleLightCubeMapTexture);
	BIZ_ASSERT(doubleBufferIndex < 2);
	BIZ_ASSERT(m_VehicleLightCubeMapTexture[vehicleIndex][doubleBufferIndex].IsCreated());
	
	return &m_VehicleLightCubeMapTexture[vehicleIndex][doubleBufferIndex];
}

void C_GameWorld::SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate)
{
#if PLATFORM_PC
	// [Blur2PC] Retail PC sets particle and post-process quality only from the graphical detail
	// preset (their setters have no other callers in Blur.exe 1.2), so dynamic detail leaves them alone.
#else
	F32 detail = Maths::Min(cpuDetail, gpuDetail);

	m_pParticleManager->SetQuality(Maths::Lerp(0.5f, 1.0f, detail));

	C_GameWorldPostProcess::SetQuality(gpuDetail);
#endif
}

#if PLATFORM_PC
// [Blur2PC] Retail PC graphical detail preset, ported from Blur.exe 1.2 0x0041b9e0 (called from
// C_GameApp's graphical detail hook 0x0042e1c0 and at the end of Create).
void C_GameWorld::SetGraphicalDetail(const U32 graphicalDetail)
{
	if(graphicalDetail == 1)
	{
		m_pParticleManager->SetQuality(0.25f);
		m_pBlobShadows->SetVisibilityFlags(0);
		m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_9);
		if(SceneGraph::C_RenderPass* pRefract = m_pSceneGraph->GetRenderPass(E_RenderPass_Refract))
			pRefract->SetSubtechniqueHandle(m_pViewManager->GetNoDepthSubtechniqueHandle());
	}
	else if(graphicalDetail >= 2 && graphicalDetail <= 4)
	{
		static const F32 s_ParticleQuality[3]	= { 0.5f, 0.75f, 1.0f };
		static const F32 s_BlobQuality[3]		= { 0.5f, 0.75f, 1.0f };
		static const F32 s_PostQuality[3]		= { 0.5f, 0.5f, 1.0f };
		const U32 i = graphicalDetail - 2;

		m_pParticleManager->SetQuality(s_ParticleQuality[i]);
		m_pBlobShadows->SetQuality(s_BlobQuality[i]);
		m_pBlobShadows->SetVisibilityFlags(Game::E_ViewTypeFlag_Main);
		C_GameWorldPostProcess::SetQuality(s_PostQuality[i]);
		if(SceneGraph::C_RenderPass* pRefract = m_pSceneGraph->GetRenderPass(E_RenderPass_Refract))
			pRefract->SetSubtechniqueHandle(0);
	}

	// [Blur2PC] retail (0x0041bb29) enables every display window's stencil mask above detail 1 (T089)
	for(S32 windowIndex = 0; windowIndex < Display::Device()->GetWindowCount(); ++windowIndex)
		Display::Device()->GetWindow(windowIndex)->m_MaskEnable = graphicalDetail > 1;

	m_GraphicalDetail = graphicalDetail;
	m_pViewManager->SetGraphicalDetail(graphicalDetail, C_Game::GetInstance()->GetLevelState().mInUI);
}
#endif
} // namespace Game
