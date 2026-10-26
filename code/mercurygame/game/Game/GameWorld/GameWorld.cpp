#include "stdafx.h"

#define RENDERINGDATA_ALLOW_INCLUDE // only for GameWorld.cpp

#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/PhysicsBodyUserDataTypes.h>
#include <Game/Game.h>
#include <Game/GameDef/Materials.h>
#include <Game/ActorDef/GameActorList.h>

#include <btl/containers/List.h>
#include <btl/containers/BitVector.h>

#include <FX/FX.h>
#include <fx/RenderMutex.h>
#include <Render/ReducedRender/Manager.h>
#include <Render/State/RenderState.h>
#include <Render/Utils/Globals.h>
#include <Render/View/ViewManager.h>

#define SCENEGRAPH_ALLOW_INCLUDE	// only for GameWorld.cpp

#include <scenegraph/SceneGraphImpl.h>
#include <scenegraph/View.h>
#include <scenegraph/MultiView.h>
#include <scenegraph/Visible/BackFrontSorter.h>
#include <scenegraph/Visible/FrontBackSorter.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/RenderGroupSettings.h>
#include <SceneGraph/RenderStates.h>
#include <RenderingData/RenderEnums.h>

#include "renderingdata/renderingdata.h"

#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/UserParticleLibrary.h>

#include <Render/PostProcess/Manager.h>
#include <Game/GameWorld/PostProcessLayer.h>

#include <game_physics/CollisionEvent.h>
#include <game_physics/World.h>

#include <game_resource/LevelResource.h>
#include <game_resource/LevelSceneManager.h>

#include <GameApp.h>
#include <SchedulerConfig.h>
#include <render/debug//DebugRender.h>

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared/Messaging/MessageTypeList.h>
#include <misc_shared/Messaging/GameMessaging.h>

#include <actor_system/ActorCreationList.h>

#include <world_render/LightList.h>
#include <game/MGCam/CameraSystem.h>

#include <Game/GameDef/Polyflags.h>

#include <racingphysics/racingphysics.h>
#include <racingphysics/debug/draw.h>
#include <racingphysics/debug/colour.h>
#include <Vehicle/VehicleModelManager.h>

#include <Game/GameWorld/GameWorldContextProvider.h>
#include <Game/GameDef/PhysicsConstants.h>

#include <Fx/EnvironmentSettings.h>
#include <Game/GameDef/GameData.h>
#include <xt/xtgame.h>
#include <Game/GameWorld/EnvironmentSettings.h>
#include <effect\sky\TexturedSkyDome.h>


#include <shared_actors/SharedActorUtils.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Utility/VisibilityDetection.h>

#include <animated_model/TaskManager.h>
#include <animation/task/EvaluationState.h>
#include <animation/task/TaskManager.h>
#include <fx/Fx.h>
#include <fx/RenderMutex.h>

#include <game_debug/ShaderDebugToggles.h>

#include <crowd_system/CrowdSystem.h>

DEBUG_ENUM(Render::Reduced::E_Downsample)
	DE_OPTION(Render::Reduced::E_Downsample_None,			"None")
	DE_OPTION(Render::Reduced::E_Downsample_QuarterMSAA,	"QuarterMSAA")
	DE_OPTION(Render::Reduced::E_Downsample_Quarter,		"Quarter")
	DE_OPTION(Render::Reduced::E_Downsample_Sixteenth,		"Sixteenth")
END_DEBUG_ENUM
Debug::Enum<Render::Reduced::E_Downsample> g_debugReducedRender("Game.ReducedRender",	Render::Reduced::E_Downsample_None);

Debug::Vector3 g_Wind("Game.Wind", 1.0f, 0.0f, 0.0f);

namespace PostProcess
{
	extern Debug::Bool	g_debugPostProcessEnable;
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




static void CollisionEventHandler_Actor(const GamePhysics::C_CollisionEvent& collisionEvent)
{
	Game::C_ContactDrawer::AddContact(collisionEvent);
}

static void CollisionEventHandler_ActorVsActor(const GamePhysics::C_CollisionEvent& collisionEvent)
{
	Game::C_ContactDrawer::AddContact(collisionEvent);
}

} // namespace GameWorld_Impl




namespace Game
{

#ifndef CFG_GOLD
	Debug::Bool g_DebugGameWorldDebugActorRender("ActorDebug.DebugRender", false, Debug::Persist());
#endif

Debug::Bool g_RenderGravityScene( "Gravity.Debug.RenderScene", false );
Debug::Float g_GravityDrawDistance( "Gravity.Debug.Settings.DrawDistance", 50.0f, Debug::Range(0.0f, 500.0f));

Debug::Bool g_DrawContacts("Gravity.Debug.DrawContacts", false);

Debug::Bool g_SceneGraphFreezeFrustum("Graphics.SceneGraph.FreezeFrustum", false );
Debug::Bool g_SceneGraphFreezePvs("Graphics.SceneGraph.FreezePvs", false );
Debug::Bool g_SceneGraphRenderSolid("Graphics.SceneGraph.RenderPasses.Solid", true);
Debug::Bool g_SceneGraphRenderPunch("Graphics.SceneGraph.RenderPasses.Punch", true);
Debug::Bool g_SceneGraphRenderDecal("Graphics.SceneGraph.RenderPasses.Decal", true);
Debug::Bool g_SceneGraphRenderCustomSolid("Graphics.SceneGraph.RenderPasses.CustomSolid", true);
Debug::Bool g_SceneGraphRenderSky("Graphics.SceneGraph.RenderPasses.Sky", true);
Debug::Bool g_SceneGraphRenderRefract("Graphics.SceneGraph.RenderPasses.Refract", true);
Debug::Bool g_SceneGraphRenderEffectSolid("Graphics.SceneGraph.RenderPasses.EffectSolid", true);
Debug::Bool g_SceneGraphRenderEffectDecal("Graphics.SceneGraph.RenderPasses.EffectDecal", true);
Debug::Bool g_SceneGraphRenderAlpha("Graphics.SceneGraph.RenderPasses.Alpha", true);
Debug::Float g_fInstanceDrawDistanceScale("Game.InstanceDrawDistanceScale", 1.f, Debug::Range(0.f, 2.f));

Debug::Bool g_CubeMapEnable("Game.CubeMapEnable", true );
Debug::Bool g_CaptureCubeMap("Game.CaptureOneFrameCubeMap", false );
Debug::String g_CubeMapFileName("Game.CubeMapFileName", "GAME:\\cubemap" );




struct C_ContactDrawer::C_Item
{
	MathsStorage::Float3	pos;
	MathsStorage::Float3	dir;
	F32						len;
	Maths::RGBA				col;
};

Btl::Vector<C_ContactDrawer::C_Item>	C_ContactDrawer::s_Items;
S32										C_ContactDrawer::s_Counter = 0;
Gravity::C_Body*						C_ContactDrawer::s_pLastBodyA = NULL;
Gravity::C_Body*						C_ContactDrawer::s_pLastBodyB = NULL;

void C_ContactDrawer::Clear()
{
	s_Items.Clear();
	s_Counter = 0;
	s_pLastBodyA = NULL;
	s_pLastBodyB = NULL;
}

void C_ContactDrawer::AddContact(const GamePhysics::C_CollisionEvent& collisionEvent)
{
	if (g_DrawContacts)
	{
		const S32 index = s_Items.Size();
		s_Items.Resize(index + 1);
		C_Item& dest = s_Items[index];

		if ((collisionEvent.pBodyA != s_pLastBodyA) || (collisionEvent.pBodyB != s_pLastBodyB))
		{
			s_Counter += 5;
			if (s_Counter > 255)
				s_Counter = 0;

			s_pLastBodyA = collisionEvent.pBodyA;
			s_pLastBodyB = collisionEvent.pBodyB;
		}

		Maths::Store(collisionEvent.position, &dest.pos);
		Maths::Store(collisionEvent.normal, &dest.dir);
		dest.len = collisionEvent.penetrationDepth;
		dest.col = Maths::RGBA(255, s_Counter, 255 - s_Counter);
	}
}

void C_ContactDrawer::DebugRender(DbUtil::I_DebugRender* pDebugRender)
{
	if (g_DrawContacts)
	{
		const F32 starSize = 0.25f;
		const F32 arrowSize = 0.25f;
		const Maths::RGBA arrowColour = Maths::RGBA(0, 255, 0);

		for (Btl::Vector<C_Item>::ConstIterator it = s_Items.Begin(); it != s_Items.End(); ++it)
		{
			const Maths::HVec3 pos = Maths::Load<Maths::HVec3>(&it->pos);
			const Maths::HVec3 dir = Maths::Load<Maths::HVec3>(&it->dir);

			pDebugRender->Draw3dStar(pos, starSize, it->col);
			pDebugRender->Draw3dArrow(pos, dir, it->len, arrowSize, arrowColour);
		}
	}
}





C_GameWorld::C_GameWorld()
	: m_pPhysicsScene( NULL )
	, m_pGamePhysicsWorld( NULL )
	, m_pSceneGraph( NULL )
	, m_pActorList( NULL )
	, m_pContextProvider( NULL )
	, m_pLevelSceneManager( NULL )
	, m_pEnvironmentSettings( NULL )
	, m_pCrowdSystem( NULL )
	, m_GameFrame( 0 )
{
	m_iRenderGroup = Scheduler::CreateGroup();
	m_iAlphaLuminanceTechniqueHandle = FX::RegisterSubtechnique("AlphaLuminance");
	m_iLODTechniqueHandle = FX::RegisterSubtechnique("LOD");

	FX::LockSerialiseMutex();
	m_ClampPoint = FX::GetSamplerState(
		Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
		Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
		1, 0, 0);

	m_ClampLinear = FX::GetSamplerState(
		Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
		Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
		1, 0, 0);
	FX::ReleaseSerialiseMutex();

	InitialiseEnvironmentSettingsParams();
}

C_GameWorld::~C_GameWorld()
{
	Scheduler::DestroyGroup(m_iRenderGroup);
	delete m_pSky;
}

C_GameActorList* C_GameWorld::GetActorList()
{
	return m_pActorList;
}

Gravity::C_Scene* C_GameWorld::GetPhysicsScene()
{
	return m_pPhysicsScene;
}

GameResource::C_LevelSceneManager* C_GameWorld::GetLevelSceneManager()
{
	return m_pLevelSceneManager;
}

SceneGraph::C_SceneGraph* C_GameWorld::GetSceneGraph()
{
	return m_pSceneGraph;
}

AnimatedCrowd::C_CrowdSystem* C_GameWorld::GetCrowdSystem()
{
	return m_pCrowdSystem;
}

Sys::I_ContextProvider* C_GameWorld::GetContextProvider()
{
	return (m_pContextProvider != NULL) ? m_pContextProvider : NULL;
}

void C_GameWorld::CreateEnvironmentSettingsFromSorcery()
{
	if (m_pEnvironmentSettings)
		delete m_pEnvironmentSettings;

	XtPack *pPack = C_GameData::GetInstance()->GetMainPack();

	if (pPack)
	{
		// eventually we will want to pick the EnvironmentSettings record depending on which level was loaded
		// but for the time being, we just use the first EnvironmentSettings record we come across
		// TODO: program this properly
		for (U32 i=0; i < pPack->count; ++i)
		{
			if (pPack->records[i].record->type == XtGetType<EnvironmentSettings>())
			{
				m_pEnvironmentSettings = CreateEnvironmentSettings((EnvironmentSettings*)pPack->records[i].record->object, m_pSky);
				m_pEnvironmentSettings->AssociateDebugVariables("Graphics.EnvironmentSettings");
				return;
			}
		}
	}

	m_pEnvironmentSettings = new FX::EnvironmentSettings;
}


bool C_GameWorld::CreatePhysics()
{
	m_pPhysicsScene = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateScene( C_PhysicsMaterials::GetMaterialMatrix(), C_PhysicsMaterials::GetMaterialCount() );
	m_PhysicsExternalConstants.m_Gravity = Maths::HVec3(0, -9.8f, 0);

	//m_pScene->RegisterCollisionCallback(&drawer);

	m_pPhysicsScene->SetInternalTaskPolicy( C_SchedulerConfig::GetInstance()->GetCollisionPolicy() );
	m_pPhysicsScene->SetInternalGroupHandle( C_SchedulerConfig::GetInstance()->GetGroup( E_SchedulerGroup_PhysicsInternal ) );
	m_pPhysicsScene->RegisterRaytracingTaskPolicy( C_SchedulerConfig::GetInstance()->GetGroup( E_SchedulerGroup_PhysicsRaytrace ) );

	RacingPhysics::CreateXt( C_GameApp::GetInstance()->GetXtDatabase() );
	//RacingPhysics::VehicleManager::Instance().Initialise();
	RacingPhysics::PhysicsTimer::Instance().InitialiseTimers();

	m_pGamePhysicsWorld = new GamePhysics::C_World( m_pPhysicsScene );
	
	m_pGamePhysicsWorld->SetCollisionEventHandler(::Actors::C_SimplePhysicsBodyUserData::ClassTypeUid(),	&GameWorld_Impl::CollisionEventHandler_Actor);
	m_pGamePhysicsWorld->SetCollisionEventHandler(::Actors::C_PBCUserData::ClassTypeUid(),					&GameWorld_Impl::CollisionEventHandler_Actor);
	
	m_pGamePhysicsWorld->SetCollisionEventHandler(::Actors::C_SimplePhysicsBodyUserData::ClassTypeUid(),	::Actors::C_SimplePhysicsBodyUserData::ClassTypeUid(),	&GameWorld_Impl::CollisionEventHandler_ActorVsActor);
	m_pGamePhysicsWorld->SetCollisionEventHandler(::Actors::C_SimplePhysicsBodyUserData::ClassTypeUid(),	::Actors::C_PBCUserData::ClassTypeUid(),				&GameWorld_Impl::CollisionEventHandler_ActorVsActor);
	m_pGamePhysicsWorld->SetCollisionEventHandler(::Actors::C_PBCUserData::ClassTypeUid(),					::Actors::C_PBCUserData::ClassTypeUid(),				&GameWorld_Impl::CollisionEventHandler_ActorVsActor);

	return true;
}

bool C_GameWorld::CreateAnimatedModelTaskManager()
{
	const U32 kMaxAsyncTasksPerFrame = 64;
	bool ok = AnimatedModel::CreateTaskManager(kMaxAsyncTasksPerFrame, C_SchedulerConfig::GetInstance()->GetAnimationPolicy());
	return ok;
}

bool C_GameWorld::CreateAnimationTaskManager()
{
	const U32 kMaxAsyncTasksPerFrame = 64;
	bool ok = Animation::CreateTaskManager(kMaxAsyncTasksPerFrame, C_SchedulerConfig::GetInstance()->GetAnimationPolicy());
	return ok;
}

bool C_GameWorld::CreateActorList( Actor::C_ActorFactory* pActorFactory )
{
	if (m_pSceneGraph == NULL)
		return false;

	if (m_pPhysicsScene == NULL)
		return false;

	if(m_pLevelSceneManager == NULL)
		return false;

	m_pContextProvider = new C_GameWorldContextProvier(m_pSceneGraph, m_pLevelSceneManager, m_pPhysicsScene, &m_PhysicsExternalConstants );

	m_pActorList = new C_GameActorList(pActorFactory, m_pContextProvider);
	m_pActorList->AddComponentGenericMessageTypes(GameWorld_Impl::g_ActorComponentGenericMessageTypeList);

	m_pVisibilityDetector = new ::Actors::C_VisibilityDetector( m_pActorList );

	return true;
}

bool C_GameWorld::CreateSceneGraph()
{
	m_pSceneGraph = new SceneGraph::C_SceneGraph();

	m_pSceneGraph->SetJobQueuePolicyGather( C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy() );
	m_pSceneGraph->SetJobQueuePolicyRender( C_SchedulerConfig::GetInstance()->GetCollisionPolicy() );

	m_pBackFront = new SceneGraph::C_BackFrontSorter;
	m_pFrontBack = new SceneGraph::C_FrontBackSorter;

	// set render passes and sorting strategies etc
	m_pSceneGraph->SetNumRenderPasses(9);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Solid, NULL, m_iAlphaLuminanceTechniqueHandle);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Punch);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Decal);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Refract);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Alpha, m_pBackFront);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_CustomSolid);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_EffectSolid);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_EffectDecal);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Sky);

	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_Solid);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, E_RenderPass_Punch);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OVERLAY, E_RenderPass_Decal);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_POST_RESOLVE, E_RenderPass_Refract);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, E_RenderPass_Alpha);

	// states for full colour write
	m_pColourWriteAllStates = new SceneGraph::RenderStates;
	m_pColourWriteAllStates->colourWriteEnable = Display::E_ColourWriteEnable_All;

	SceneGraph::C_RenderPass *pPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Solid);
	assert(pPass);
	pPass->SetRenderStates(m_pColourWriteAllStates);
	pPass = m_pSceneGraph->GetRenderPass(E_RenderPass_Sky);
	assert(pPass);
	pPass->SetRenderStates(m_pColourWriteAllStates);

	m_pWithAAsettings = new SceneGraph::C_RenderGroupSettings;
	m_pWithoutAAsettings = new SceneGraph::C_RenderGroupSettings;

	return true;
}

bool C_GameWorld::CreateSceneGraphView()
{
	m_pCubeMap = new C_CubeMap(m_pSceneGraph);
	m_pCubeMap->Create(m_iLODTechniqueHandle, m_pDynamicEffectRenderer->GetNodeFlag());

	for(S32 i = 0; i < 4; ++i)
	{
		m_pMultiView[i] = new SceneGraph::C_MultiView(m_pSceneGraph);
		m_pView[i] = m_pSceneGraph->AddView();
		m_pView[i]->EnableRenderPass(	SceneGraph::Flag<E_RenderPass_Solid>() |
									SceneGraph::Flag<E_RenderPass_Punch>() |
									SceneGraph::Flag<E_RenderPass_Decal>() |
									SceneGraph::Flag<E_RenderPass_Refract>() |
									SceneGraph::Flag<E_RenderPass_Alpha>() |
									SceneGraph::Flag<E_RenderPass_CustomSolid>() |
									SceneGraph::Flag<E_RenderPass_EffectSolid>() |
									SceneGraph::Flag<E_RenderPass_EffectDecal>() |
									SceneGraph::Flag<E_RenderPass_Sky>());
		m_pView[i]->Initialise();
		m_pView[i]->ReserveGroups(E_RenderGroup_Count);
		m_pView[i]->GetGroup(E_RenderGroup_WithAA)->SetName("MainWithAA");
		m_pView[i]->GetGroup(E_RenderGroup_WithoutAA)->SetName("MainWithoutAA");
	}

	return true;
}

bool C_GameWorld::CreateStreamer()
{
	RenderingData::Target::StreamManager::Instance().SetJobQueuePolicy( C_SchedulerConfig::GetInstance()->GetStreamerPolicy() );

	return true;
}
	
bool C_GameWorld::CreateLevelSceneManager()
{
	m_pLevelSceneManager = new GameResource::C_LevelSceneManager;
	m_pLevelSceneManager->Initialise( m_pSceneGraph, m_pPhysicsScene );
	return true;
}

bool C_GameWorld::CreateRenderers()
{
	m_pSky = new Effect::Sky::C_TexturedSkyDome;
	m_pSky->SetRenderPass(E_RenderPass_Sky);
	m_pSceneGraph->Insert(m_pSky);

	m_pDynamicEffectRenderer = new DynamicEffect::C_Renderer(m_pSceneGraph);
	m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Opaque, E_RenderPass_EffectSolid);
	m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Decal, E_RenderPass_EffectDecal);
	m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent, E_RenderPass_Alpha);
	m_pDynamicEffectRenderer->ReserveVertexBuffers(512 * 1024, 64);	// 32 megs
	m_pDynamicEffectRenderer->SetAssetPath("graphics\\textures\\particles\\");
	m_pDynamicEffectRenderer->SetJobQueuePolicyHandle(C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy());
		
	m_pParticleManager = new DynamicEffect::Particle::C_ParticleManager(m_pDynamicEffectRenderer);
	m_pParticleManager->ReserveEmitters(512);
	m_pParticleManager->ReserveSuperEmitters(2048);
	m_pParticleManager->ReserveEventInstances(256);
	m_pParticleManager->SetParticleColour(MathsStorage::Float4A16(2.0f, 2.0f, 2.0f, 2.0f));
	m_pParticleManager->SetEmissiveColour(MathsStorage::Float4A16(2.0f, 2.0f, 2.0f, 2.0f));
	m_pParticleManager->SetDefaultSpawnDirection(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ());
	m_pParticleManager->SetGlobalForce(Maths::HVec3(1.0f, 0.0f, 0.0f));

	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Open("graphics\\models\\particles\\Amax_Particle_Shapes.particleMesh");

	// open the effects pack and initialise xt interface
	DynamicEffect::XtInterface::PackOpen(C_GameApp::GetInstance()->GetXtDatabase(), "gamedata\\Xt\\");
	DynamicEffect::XtInterface::Initialise(m_pDynamicEffectRenderer);
	DynamicEffect::Particle::XtInterface::Initialise(m_pParticleManager);

	m_pSceneGraph->Insert(m_pDynamicEffectRenderer);

	m_pReducedRender = new Render::Reduced::C_Manager();
	{
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

	return true;
}

bool C_GameWorld::CreateCrowdSystem()
{
	if (m_pCrowdSystem)
		return false;

	m_pCrowdSystem = new AnimatedCrowd::C_CrowdSystem();
	AnimatedCrowd::CreateXt( C_GameApp::GetInstance()->GetXtDatabase() );
	
	return true;
}

bool C_GameWorld::Create( Actor::C_ActorFactory* pActorFactory )
{
	bool success = true;

	m_GameFrame = 0;

	success = CreatePhysics();
	if(!success)
		return false;

	success = CreateSceneGraph();
	if(!success)
		return false;

	success = CreateStreamer();
	if(!success)
		return false;

	success = CreateLevelSceneManager();
	if(!success)
		return false;

	success = CreateRenderers();
	if(!success)
		return false;

	success = CreateSceneGraphView();
	if(!success)
		return false;

	success = CreateActorList( pActorFactory );
	if(!success)
		return false;
		
	success = CreateAnimationTaskManager();
	if (!success)
		return false;
	
	success = CreateAnimatedModelTaskManager();
	if (!success)
		return false;

	success = CreateCrowdSystem();
	if(!success)
		return false;

	g_bEnvironmentSettingsChangedInSorcery = true;

	return true;
}

bool C_GameWorld::Destroy()
{
	for(S32 i = 0; i < 4; ++i)
	{
		delete m_pMultiView[i];
		m_pSceneGraph->RemoveView(m_pView[i]);
	}

	m_pCubeMap->Destroy();
	delete m_pCubeMap;
	
	AnimatedModel::DestroyTaskManager();
	Animation::DestroyTaskManager();
	
	DynamicEffect::XtInterface::PackClose();
	DynamicEffect::Particle::XtInterface::Shutdown();
	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Reset();

	delete m_pReducedRender;

	AnimatedCrowd::DestroyXt();
	if (m_pCrowdSystem)
		m_pCrowdSystem->Destroy();

	SAFE_DELETE_PTR( m_pSky );
	SAFE_DELETE_PTR( m_pDynamicEffectRenderer );
	SAFE_DELETE_PTR( m_pActorList );
	SAFE_DELETE_PTR( m_pVisibilityDetector );
	SAFE_DELETE_PTR( m_pContextProvider );
	SAFE_DELETE_PTR( m_pLevelSceneManager );
	SAFE_DELETE_PTR( m_pCrowdSystem );
	SAFE_DELETE_PTR( m_pSceneGraph );
	SAFE_DELETE_PTR( m_pBackFront );
	SAFE_DELETE_PTR( m_pFrontBack );
	SAFE_DELETE_PTR( m_pColourWriteAllStates );
	SAFE_DELETE_PTR( m_pWithAAsettings );
	SAFE_DELETE_PTR( m_pWithoutAAsettings );
	
	SAFE_DELETE_PTR( m_pGamePhysicsWorld );
	RacingPhysics::DestroyXt();
	if(m_pPhysicsScene)
	{
		Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseScene( m_pPhysicsScene );
		m_pPhysicsScene = NULL;
	}

	if (m_pEnvironmentSettings)
		delete m_pEnvironmentSettings;

	return true;
}

Gravity::C_ExternalConstants& C_GameWorld::GetPhysicsExternalConstants()
{
	return m_PhysicsExternalConstants;
}

Debug::Bool g_PhysicsDebugRenderVehicleSolid( "Gravity.Debug.RenderFilter.VehicleSolid", true );
Debug::Bool g_PhysicsDebugRenderWheelSolid( "Gravity.Debug.RenderFilter.WheelSolid", true );
Debug::Bool g_PhysicsDebugRenderCharacterSolid( "Gravity.Debug.RenderFilter.CharacterSolid", true );
Debug::Bool g_PhysicsDebugRenderDynamicObjectSolid( "Gravity.Debug.RenderFilter.DynamicObjectSolid", true );
Debug::Bool g_PhysicsDebugRenderRagdollSolid( "Gravity.Debug.RenderFilter.RagdollSolid", true );
Debug::Bool g_PhysicsDebugRenderShootable( "Gravity.Debug.RenderFilter.Shootable", false );
Debug::Bool g_PhysicsDebugRenderCameraSolid( "Gravity.Debug.RenderFilter.CameraSolid", false );
Debug::Bool g_PhysicsDebugRenderFluidSolid( "Gravity.Debug.RenderFilter.Fluid", false );

void GetDebugRenderSceneCollisionFilter(Collision::C_CollisionFilter& filter)
{	
	U32 filterFlags = 0;

	if( g_PhysicsDebugRenderVehicleSolid )
		filterFlags |= PolyFlags::kVehicleSolid;

	if( g_PhysicsDebugRenderWheelSolid )
		filterFlags |= PolyFlags::kWheelSolid;

	if( g_PhysicsDebugRenderCharacterSolid )
		filterFlags |= PolyFlags::kCharacterSolid;

	if( g_PhysicsDebugRenderDynamicObjectSolid )
		filterFlags |= PolyFlags::kDynamicObjectSolid;

	if( g_PhysicsDebugRenderRagdollSolid )
		filterFlags |= PolyFlags::kRagdollSolid;

	if( g_PhysicsDebugRenderShootable )
		filterFlags |= PolyFlags::kShootable;

	if( g_PhysicsDebugRenderCameraSolid )
		filterFlags |= PolyFlags::kCameraSolid;
	if( g_PhysicsDebugRenderFluidSolid )
		filterFlags |= PolyFlags::kFluid;

	filter.m_CollisionMask = filterFlags;
	filter.m_CollisionFlags = 0xffffffff;
}

void C_GameWorld::DebugRender( DbUtil::I_DebugRender* pDebugRender )
{
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView();
	Maths::HMat43 mCameraToWorld = view.m_View.GetViewSettings3D().GetCameraToWorld();

	if( g_RenderGravityScene )
	{
		Gravity::C_Scene* pScene = GetPhysicsScene();
		if( pScene ) 
		{
			pDebugRender->SetLightDirection( Maths::HVec3( 0.2f, 0.7f, 0.3f ).Normalised().Normal() );
			Collision::C_CRAxisBox box;
			F32 drawDist = g_GravityDrawDistance;

			Maths::HVec3 centre = mCameraToWorld.AW() + mCameraToWorld.AZ() * drawDist * 0.5f;
			Maths::HVec3 extents = Maths::HVec3(drawDist, drawDist, drawDist);
			box.SetExtents( extents );
			box.SetCentre( centre );
			Collision::C_CollisionFilter filter;
			GetDebugRenderSceneCollisionFilter( filter );
			pScene->DebugRenderScene( pDebugRender, box, pScene->m_Stack, &filter );
		}
	}
	
	WorldRender::C_Light::SetDebugRenderCameraMatrix( mCameraToWorld );

	if( GetLevelSceneManager() )
	{
		GetLevelSceneManager()->DebugRender( pDebugRender );
	}

	#ifndef CFG_GOLD
	if (g_DebugGameWorldDebugActorRender)
	{
		PERF_EVENT("ActorsDebugRender");
		GetActorList()->DebugRender(pDebugRender);
	}
	#endif
}

void C_GameWorld::BeginSplitscreen()
{
}

void C_GameWorld::SetSharedFxParameters()
{
	// set frame wide parameters

	const F32 fTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
	FX::SetSharedParameterByName("g_fTime", Display::C_PSConstant(fTime));

	const Display::C_Float4 vSunDirection = Display::C_Float4( m_SunDirection.X(), m_SunDirection.Y(), m_SunDirection.Z(), 0.00f );
	FX::SetSharedParameterByName("g_vSunDirection", &vSunDirection);
	if (m_pEnvironmentSettings)
		m_pEnvironmentSettings->Set();

	GameDebug::SetShaderDebugToggles();
}

void C_GameWorld::SetSharedFxParameters(S32 viewIndex)
{
	// set hdr parameters
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		FX::ParameterHandle	pVectorParameter	= FX::GetSharedParameter("g_vLuminance");
		FX::ParameterHandle	pTextureParameter	= FX::GetSharedParameter("g_tLuminance");

		Render::C_RenderState& renderState = Render::C_RenderState::Instance();
		
		const Display::C_Float4	pVector		= renderState.HDR()->GetShaderParameters(viewIndex);
		const void*				pTexture	= renderState.HDR()->GetAdaptedLuminanceTexture(viewIndex)->GetGPUTexture();
		
		FX::SetParameter(pVectorParameter, &pVector);
		FX::SetParameter(pTextureParameter, &pTexture);
		FX::SetSamplerState(pTextureParameter, m_ClampPoint);
	}
	{
		FX::ParameterHandle p = FX::GetSharedParameter("g_tColourBuffer");
		const void* pTexture = Render::g_pColourBuffer->GetGPUTexture();
		FX::SetParameter(p, &pTexture);
		FX::SetSamplerState(p, m_ClampLinear);
	}
	{
		FX::ParameterHandle p = FX::GetSharedParameter("g_tDepthBuffer");
		const void* pTexture = Render::g_pDepthStencilBuffer->GetGPUTexture();
		FX::SetParameter(p, &pTexture);
		FX::SetSamplerState(p, m_ClampPoint);
	}
	{
		FX::ParameterHandle p = FX::GetSharedParameter("g_vDepthScale");
		const MathsStorage::Float4& tempDSSP = Render::C_RenderState::Instance().GetDepthScaleShaderParameter(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewIndex).m_View.GetViewSettings3D().GetNearClip(), Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewIndex).m_View.GetViewSettings3D().GetFarClip());
		FX::SetParameter(p, &tempDSSP);
	}
}

void C_GameWorld::RenderSplitscreenWithAA()
{
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView();
	const S32 viewIndex = view.m_View.GetUserIndex();	// render
	m_pSceneGraph->SetSharedParameters(*Display::Device()->GetView());
	
	m_pView[viewIndex]->BeginRender();
	m_pView[viewIndex]->RenderGroup(E_RenderGroup_WithAA);
	
	// set all states unknown
	Display::Device()->InvalidateState();
}

void C_GameWorld::RenderSplitscreenWithoutAA()
{
	const Display::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView().m_View;

	const S32 userIndex = view.GetUserIndex();
	const S32 viewIndex = view.GetIndex();

	m_pSceneGraph->SetSharedParameters(*Display::Device()->GetView());

	if(g_SceneGraphRenderRefract)
		m_pView[userIndex]->RenderPass(E_RenderPass_Refract);
	
	if(g_debugReducedRender != Render::Reduced::E_Downsample_None)
	{
		m_pReducedRender->SetDepth(viewIndex, g_debugReducedRender, Render::g_pDepthStencilBuffer);
		m_pReducedRender->Begin(viewIndex, g_debugReducedRender);
	}

	if(g_SceneGraphRenderAlpha)
		m_pView[userIndex]->RenderGroup(E_RenderGroup_WithoutAA);

	if(g_debugReducedRender != Render::Reduced::E_Downsample_None)
	{
		m_pReducedRender->End();
		m_pReducedRender->Render(viewIndex, g_debugReducedRender, Render::Reduced::E_BlendMode_Additive);
	}

	m_pView[userIndex]->EndRender();

	// restore depth write states etc. - check gravity scene debug render is ok if this is removed
	Display::Device()->InvalidateState();
	Display::Device()->SetViewShaderConstants3D();
}

void C_GameWorld::Update()
{
	// this code re-creates the environment settings from Sorcery when someone dynamically edits the settings
	if (g_bEnvironmentSettingsChangedInSorcery)
	{
		CreateEnvironmentSettingsFromSorcery();
		g_bEnvironmentSettingsChangedInSorcery = false;
	}

	UpdatePhysics();

	m_GameFrame++;

	m_pParticleManager->SetGlobalForce(Maths::HVec3(g_Wind.Value().vx, g_Wind.Value().vy, g_Wind.Value().vz));

	DynamicEffect::Particle::XtInterface::Update();
	m_pDynamicEffectRenderer->Update(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());

	GetActorList()->UpdateSystem( C_GameApp::GetInstance()->GetTimer().GetFrameTime(), m_GameFrame );
	GetActorList()->Update( C_GameApp::GetInstance()->GetTimer().GetFrameTime() );
	
	m_pVisibilityDetector->Update( m_pSceneGraph );
	
	MGCam::System().WorldUpdate();

	m_pCubeMap->SetPosition(MGCam::System().DebugFeed().Position());

	// TODO: [rthomas]	Temp use of camera position
	if (m_pCrowdSystem)
		m_pCrowdSystem->Update(MGCam::System().DebugFeed().Position(), C_GameApp::GetInstance()->GetTimer().GetFrameTime());
}

void C_GameWorld::UpdatePhysics()
{
	Gravity::C_Scene* pPhysicsScene = GetPhysicsScene();

	if( !pPhysicsScene )
		return;

	// clear collision contact debug
	C_ContactDrawer::Clear();

	RacingPhysics::UpdateXt();
	//RacingPhysics::VehicleManager::Instance().UpdatePhysics();
	RacingPhysics::PhysicsTimer::Instance().UpdatePhysicsTimer();

	pPhysicsScene->Simulate( C_GameApp::GetInstance()->GetTimer().GetFrameTime() );
	//pPhysicsScene->Simulate( C_GameApp::GetInstance()->GetTimer().GetFrameTime() * 0.5f );
	//GetPhysicsScene()->Simulate( 1.0f / 60.0f );


	// Process any body events: This can probably become a shared actor code function at some stage - paulm 
	const int eventCount = pPhysicsScene->GetNumEvents();
	if (eventCount > 0)
	{
		SharedPhysics::C_MemoryIterator<Gravity::C_BodyEvent> it = pPhysicsScene->GetEventIterator();
		for (int e=0 ; e<eventCount ; e++)
		{
			const Gravity::C_BodyEvent* bodyEvent = it.Get();
			if (bodyEvent->GetEventType() == Gravity::C_BodyEvent::E_RemoveConstraint)
			{
				//const Actor::C_ActorHandle actorHandle = SharedActorUtils::HandleFromUserData(bodyEvent->GetConstraintUserData());
				//GetActorList()->SendMessageToActor( actorHandle, E_GameActorMsg_ConstraintDestroyed );
			}
			else
			{
				Gravity::C_Body* body = bodyEvent->GetBody();
				if (body)
				{
					const Actor::C_Actor* pActor = C_PhysicsBodyUserDataTypes::GetActorFromUserData(body->GetUserData<void>());
					
					if (pActor != NULL)
					{
						C_PhysicsBodyEventMsg msg(*bodyEvent);
						GetActorList()->SendMessageToActor( pActor->GetHandle(), &msg );
					}
				}
			}
		}
	}
}

// this update doesn't get paused
void C_GameWorld::GlobalUpdate()
{
	// draw collision contacts
	C_ContactDrawer::DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred());

	FX::LockSerialiseMutex();

	// update the level streaming just using the main view for now
	if(!g_SceneGraphFreezePvs)
	{
		GetLevelSceneManager()->Update();

		RenderingData::Target::RequestInfo request;

		const SceneGraph::C_View& view(*m_pView[0]);
		const Framework::C_View& frameworkView = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

		GetLevelSceneManager()->UpdateView(view);

		request.cameraToWorld = view.GetViewSettings3D().GetCameraToWorld();
		request.drawDistance = view.GetViewSettings3D().GetFarClip();
		request.pvsCell = view.GetPvsCell();
		request.fovY = view.GetViewSettings3D().GetFieldOfView();
		request.aspect = view.GetViewSettings3D().GetAspectRatio();
		request.resolution = frameworkView.m_View.GetViewport().GetW() / 1280.f;
		request.directional = true;
		request.mainView = true;
		request.visibleRooms = view.GetVisibleRoomList();

		GetLevelSceneManager()->UpdateStreaming(&request, 1);
	}

	FX::ReleaseSerialiseMutex();
}

void C_GameWorld::BeginFrame()
{
	RenderingData::Target::BeginFrame();

	// Ensure that all animation and animated model tasks have finished.
	Animation::TaskManager().Flush();
	AnimatedModel::TaskManager().Flush();

	m_pSceneGraph->BeginFrame();
	Vehicle::C_VehicleModelManager::Instance().PreRender();

	SetSharedFxParameters();

	m_MultiViewCount = 0;
	for(S32 i = 0; i < 4; ++i)
		m_pMultiView[i]->ClearViews();

	for(S32 i = 0; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); ++i)
	{
		if(!m_pMultiView[m_MultiViewCount]->AddView(m_pView[i]))
		{
			++m_MultiViewCount;
			m_pMultiView[m_MultiViewCount]->ClearViews();
			m_pMultiView[m_MultiViewCount]->AddView(m_pView[i]);
		}
	}
	++m_MultiViewCount;

	// setup frustums and gather the multiviews
	for(S32 i = 0; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); ++i)
	{
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i);
		const S32 viewIndex = view.m_View.GetUserIndex();
	
		if(!g_SceneGraphFreezeFrustum)
		{
			m_pView[viewIndex]->SetViewFrustum(view.m_View.GetViewFrustum());
			m_pView[viewIndex]->SetViewData(view.m_View.GetViewSettings3D());
			m_pView[viewIndex]->SetInstanceDrawDistanceScale(g_fInstanceDrawDistanceScale);
		}
	}
	for(S32 i = 0; i < m_MultiViewCount; ++i)
		m_pMultiView[i]->BeginGather();

	SetSharedFxParameters(0);

	if(g_CubeMapEnable)
		m_pCubeMap->Gather();
}

void C_GameWorld::ScheduleOnce()
{
	if(g_CubeMapEnable)
		m_pCubeMap->Schedule();

	for(S32 i = 0; i < m_MultiViewCount; ++i)
		m_pMultiView[i]->EndGather();
}

void C_GameWorld::RenderOnce()
{
	if(g_CubeMapEnable)
		m_pCubeMap->Render();
}

void C_GameWorld::ScheduleSplitscreen()
{
#ifndef PLATFORM_PS3
	//Set Dynamic cubemap if override
	if (g_CubeMapEnable)
	{
		FX::ParameterHandle p = FX::GetSharedParameter("g_tEnvironment");
		const void *pAddress = const_cast<Display::C_TextureCube*>(m_pCubeMap->GetRenderTexture())->Get();
		FX::SetParameter(p, &pAddress);
		FX::SetSamplerState(p, m_ClampLinear);
	}
#endif

	// set solid blend mode to force alpha writes for luminance
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	// set fx parameters relevant for this view
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView();
	const S32 viewIndex = view.m_View.GetUserIndex();
	SetSharedFxParameters(viewIndex);

	// TODO: this might cause weirdness in split screen rendering
	m_pWithAAsettings->SetWindow(viewIndex, true);
	m_pWithAAsettings->SetColourDepthStencilTexture(Render::g_pColourBuffer, Render::g_pDepthStencilBuffer);

	// schedule AA rendering
	m_pView[viewIndex]->BeginSchedule(E_RenderGroup_WithAA, view.m_View, m_pWithAAsettings);
	if(g_SceneGraphRenderSolid)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_Solid);
	if(g_SceneGraphRenderPunch)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_Punch);
	if(g_SceneGraphRenderDecal)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_Decal);
	if(g_SceneGraphRenderCustomSolid)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_CustomSolid);
	if(g_SceneGraphRenderSky)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_Sky);
	if(g_SceneGraphRenderEffectSolid)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_EffectSolid);
	if(g_SceneGraphRenderEffectDecal)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_EffectDecal);
	m_pView[viewIndex]->EndSchedule();

	// if we are post processing tell the aa pass we are going into the post processing buffer
	if(PostProcess::g_debugPostProcessEnable)
	{
		const PostProcess::C_RenderTarget& renderTarget = C_GameApp::GetInstance()->PostProcessLayer()->GetManager()->GetRenderTargetMapped(view.m_View.GetIndex());
		m_pWithoutAAsettings->SetColourDepthStencilSurface(&renderTarget.m_RenderTextures[0]->GetSurface(), renderTarget.m_pDepthStencilSurface);
	}
	else
		m_pWithoutAAsettings->SetWindow(viewIndex, false);

	m_pWithoutAAsettings->SetColourDepthStencilTexture(Render::g_pColourBuffer, Render::g_pDepthStencilBuffer);

	// schedule non AA rendering
	m_pView[viewIndex]->BeginSchedule(E_RenderGroup_WithoutAA, view.m_View, m_pWithoutAAsettings);
	if(g_SceneGraphRenderAlpha)
		m_pView[viewIndex]->SchedulePass(E_RenderPass_Alpha);
	m_pView[viewIndex]->EndSchedule();

}

void C_GameWorld::PreRenderSplitscreen()
{
}

void C_GameWorld::EndFrame()
{
	m_pSceneGraph->EndFrame();
	RenderingData::Target::EndFrame();
}


void C_CubeMap::Create(U32 subtechnique, U32 disableFlags)
{
	m_pMultiView = new SceneGraph::C_MultiView(m_pSceneGraph);
	m_pSettings = new SceneGraph::C_RenderGroupSettings;
	m_pSettings->SetNumWorkers(1);
	m_pSettings->SetMultisample(m_Multisample);

	for (U32 i = 0; i < 6; i++)
	{
		m_pView[i] = m_pSceneGraph->AddView();
		m_pView[i]->EnableRenderPass(	SceneGraph::Flag<E_RenderPass_Solid>() |
										SceneGraph::Flag<E_RenderPass_Punch>() |
										SceneGraph::Flag<E_RenderPass_Decal>() |
										SceneGraph::Flag<E_RenderPass_Refract>() |
										SceneGraph::Flag<E_RenderPass_Alpha>() |
										SceneGraph::Flag<E_RenderPass_CustomSolid>() |
										SceneGraph::Flag<E_RenderPass_Sky>());
		m_pView[i]->SetSubtechniqueHandle(subtechnique);
		m_pView[i]->SetType(1);
		m_pView[i]->DisableRenderNode(disableFlags);
		m_pView[i]->Initialise();
		m_pView[i]->ReserveGroups(1);
		m_pView[i]->GetGroup(0)->SetName("CubeMapFace");
		m_pMultiView->AddView(m_pView[i]);
	}

	GameView::C_CubeMapView::Create();
}

void C_CubeMap::Destroy()
{
	delete m_pMultiView;
	delete m_pSettings;

	for (U32 i = 0; i < 6; i++)
		m_pSceneGraph->RemoveView(m_pView[i]);
}

void C_CubeMap::Gather()
{
	GameView::C_CubeMapView::Gather();
	m_pMultiView->BeginGather();
}

void C_CubeMap::Render()
{
	if (g_CaptureCubeMap)
	{
		GameView::C_CubeMapView::Render(true, (const char*)g_CubeMapFileName);
		g_CaptureCubeMap = false;
	}
	else
		GameView::C_CubeMapView::Render(false, Util::C_String());
}

void C_CubeMap::Schedule()
{
	m_pMultiView->EndGather();
	GameView::C_CubeMapView::Schedule();
}

void C_CubeMap::GatherFace(const U32 face)
{
	m_pView[face]->SetViewFrustum(m_View[face].GetViewFrustum());
	m_pView[face]->SetViewData(m_View[face].GetViewSettings3D());
}

void C_CubeMap::ScheduleFace(const U32 face)
{
	m_pView[face]->BeginSchedule(0, m_View[face], m_pSettings);
	m_pView[face]->SchedulePass(E_RenderPass_Solid);
	m_pView[face]->SchedulePass(E_RenderPass_Punch);
	m_pView[face]->SchedulePass(E_RenderPass_Decal);
	m_pView[face]->SchedulePass(E_RenderPass_CustomSolid);
	m_pView[face]->SchedulePass(E_RenderPass_Sky);
	m_pView[face]->EndSchedule();
}

void C_CubeMap::RenderFace(const U32 face )
{
	m_pView[face]->BeginRender();
	m_pView[face]->RenderGroup(0);
	m_pView[face]->EndRender();
}

} // namespace Game
 
