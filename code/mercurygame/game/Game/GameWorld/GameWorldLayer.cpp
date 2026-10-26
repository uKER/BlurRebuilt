#include "stdafx.h"

#include <Game/Game.h>

#include <framework/application/ApplicationBase.h>

#include <Game/GameWorld/GameWorldLayer.h>
#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>

#include <Render/Utils/Helper.h>

#include <world_render\RenderLevel.h>
#include <SceneGraph/SceneGraph.h>

#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/PhysicsBodyUserDataTypes.h>

// temp
#include <Game/GameActors/SystemActors.h>
#include <Game/GameControls/GameControls.h>

#include <Game/GameFlow/GameflowMessages.h>
#include <GameApp.h>

#include <DebugSystems/DebugHUDMenu.h>

#include <Game/ActorDef/GameActorList.h>

#include <Game/GameActors/TestActors.h>
#include <Game/GameActors/RagdollActor.h>
#include <Game/GameActors/VehicleTestActor.h>
#include <Game/GameActors/CharacterTestActor.h>
#include <Game/GameActors/AnimationTestActor.h>
#include <Game/GameActors/VehicleActor.h>
#include <Game/GameActors/EffectTestActor.h>

#include <Game/GameDef/Polyflags.h>
#include <Game/GameDef/Materials.h>
#include <camera2.h>
#include <Game/MGCam/CameraSystem.h>
#include <Game/MGCam/CameraSwitcher.h>
#include <Vehicle/VehicleModelManager.h>

#include <render/debug/DebugRender.h>

#include <system/Memory.h>
#include <game_resource/LevelSceneManager.h>
#include <game_resource/EffectVariations.h>
#include <game_debug/GameDebug.h>

#include <crowd_system/CrowdSystem.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>

Debug::Bool g_CreateDynamicShapes("Game.Test.CreateDynamicShapes", false);
Debug::Float g_DebugPushForceMag("Game.PushForceMag", 10000.0f);

Debug::Bool g_SpawnTestModel("Game.SpawnTestModel", false);
Debug::Bool g_SpawnRagdollModel("Game.SpawnRagdollModel", false);
Debug::Bool g_SpawnVehicle("Game.SpawnVehicle", false);
Debug::Bool g_SpawnVehicles("Game.Spawn20Vehicles", false);
Debug::Bool g_SpawnCharacter("Game.SpawnCharacter", false);
Debug::Bool g_SpawnAnimationTestActor("Game.SpawnAnimationTestActor", false);
Debug::Int  g_AnimationActorType("Game.AnimationActorType", 0, Debug::Range(0,2), Debug::Persist());

Debug::Bool g_SpawnParticleEventTestActor("Game.Particle.SpawnParticleEventTestActor", false);
Debug::Int  g_ParticleEventIndex("Game.Particle.ParticleEventIndex", 0);

namespace PostProcess { extern Debug::Float g_debugDoFPlaneInFocus; }

namespace Game
{

extern Debug::String g_SimpleUiModelString;

class C_QuitLevelMenuItem : public DebugSystems::C_MenuHUDItem
{
public:
	C_QuitLevelMenuItem(): DebugSystems::C_MenuHUDItem("Quit Level")
	{
	};

	bool Forwards()
	{
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &Messaging::BuildSimpleMessage( E_GameflowMessage_QuitLevel ) );
		return false;
	};

private:
};

C_GameWorldLayer::C_GameWorldLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
	: m_pGameWorld(NULL)
	, m_pActorStudioMessaging(NULL)
	, m_pLevelInstance(NULL)
	#ifndef CFG_GOLD
	, m_pGameDebug(NULL)
	#endif
{
	SetSubscriberSystem( pSubscriberSystem );

	AddSubscriber( E_GameflowMessage_QuitLevel );

	#ifndef CFG_GOLD
	m_pGameDebug = new GameDebug::C_GameDebug;
	m_pGameDebug->SetAlbedoGamma(true);
	m_pGameDebug->SetSpecularGamma(true);
	m_pGameDebug->SetEmissiveGamma(true);
	#endif

	C_PhysicsBodyUserDataTypes::Init();
}

C_GameWorldLayer::~C_GameWorldLayer()
{
	#ifndef CFG_GOLD
	SAFE_DELETE_PTR( m_pGameDebug );
	#endif

	C_PhysicsBodyUserDataTypes::Shutdown();
}

HRESULT C_GameWorldLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;

		case Framework::E_LayerMessage_Update:
			return Update();
		break;

		case Framework::E_LayerMessage_GlobalUpdate:
			return GlobalUpdate();
		break;

		case Framework::E_LayerMessage_BeginFrame:
			m_pGameWorld->BeginFrame();
			return S_OK;
		break;

		case Framework::E_LayerMessage_ScheduleOnce:
			m_pGameWorld->ScheduleOnce();
			return S_OK;
		break;

		case Framework::E_LayerMessage_RenderOnce:
			m_pGameWorld->RenderOnce();
			return S_OK;
		break;

		case Framework::E_LayerMessage_BeginSplitscreen:
			m_pGameWorld->BeginSplitscreen();
			return S_OK;
		break;

		case Framework::E_LayerMessage_ScheduleSplitscreen:
			m_pGameWorld->ScheduleSplitscreen();
			return S_OK;
		break;

		case Framework::E_LayerMessage_PreRenderSplitscreen:
			return PreRenderSplitscreen();
		break;

		case Framework::E_LayerMessage_RenderSplitscreenWithAA:
			return RenderSplitscreenWithAA();
		break;

		case Framework::E_LayerMessage_RenderSplitscreenWithoutAA:
			return RenderSplitscreenWithoutAA();
		break;

		case Framework::E_LayerMessage_RenderFullscreen:
			return RenderFullscreen();
		break;

		case Framework::E_LayerMessage_EndFrame:
			return EndFrame();
			break;
	}

	return S_OK;
}

HRESULT C_GameWorldLayer::Create()
{
	Game::Actors::RegisterCarModelCallback();

	Memory::TrackNodes::Instance().IncrementTag();

	const Game::C_LevelSettings& levelSettings = C_Game::GetInstance()->GetLevelSettings();

	m_pGameWorld = new C_GameWorld;
	m_pGameWorld->Create( Game::C_Game::GetInstance()->GetActorFactory() );
	
	#ifndef CFG_GOLD
	m_pGameDebug->Init( GetGameWorld()->GetLevelSceneManager(), &GetGameWorld()->GetPhysicsExternalConstants() );
	#endif

	Display::Device()->CreateRenderers();
	Display::Device()->CreateRenderersVolatile();

	// Waiting for resource data is not ideal
	const Util::C_String levelFilename = levelSettings.m_LevelFilename;
	m_LevelHandle.Load( levelFilename, this, NULL );
	Resource::WaitForResourceData( (GameResource::C_LevelResource*)m_LevelHandle );

	// can't load any models before this :(
	{
		GameResource::C_LevelResource* pLevelResource = m_LevelHandle;
		Util::C_String effectVariation = pLevelResource->GetRenderLevel()->GetEffectVariation();

		LOGINFO( g_Log, "Effect Variation : \"%s\"", effectVariation.Chars() );

		GameResource::EffectVariations::Instance().SetVariation("_" + effectVariation);
		//GameResource::EffectVariations::Instance().SetVariation("_LN");
	}
	
	// add quit level menu item
	m_pQuitLevelMenuItem = new C_QuitLevelMenuItem();
	DebugSystems::C_DebugHUDMenu::GetInstance()->Root()->AddChild(m_pQuitLevelMenuItem);

	//{
	//	Actor::C_ActorCreationDataFactoryHelper<Actors::C_NodeActorCreate> pNodeActorCreate;
	//	//pNodeActorCreate->m_WorldMatrix;
	//	Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->Spawn(pNodeActorCreate.Get());
	//}

	Vehicle::C_VehicleModelManager::Instance().Create();
	Vehicle::C_VehicleModelManager::Instance().Initialise();

	if (levelSettings.m_ActorsFilename.Length() > 0)
	{
		m_pGameWorld->GetActorList()->CreateActors(levelSettings.m_ActorsFilename);
	}

	// TODO: [rthomas]
	// Due to the WaitForResourceData on the level resource, can assume crowd data is also loaded...
	// Probably need to think of a better way to get the scenegraph and crowd data into the crowd system actor :|
#ifdef ENABLE_CROWDS
	m_pGameWorld->GetCrowdSystem()->Create(m_pGameWorld->GetSceneGraph(), m_pGameWorld->GetActorList(), &m_pLevelResource->GetRenderLevel()->GetCrowdData());
#endif

	m_pActorStudioMessaging = &ActorStudioMsgGame::C_ActorStudioMessaging::Instance();
	m_pActorStudioMessaging->SetActorList(m_pGameWorld->GetActorList());
	m_pActorStudioMessaging->SetActorPack(m_pGameWorld->GetActorList()->GetXtPack());
	m_pActorStudioMessaging->Init();

	return S_OK;
}

HRESULT C_GameWorldLayer::Destroy()
{
	m_pGameWorld->GetSceneGraph()->Remove(m_pLevelInstance);

	DebugSystems::C_DebugHUDMenu::GetInstance()->RemoveItem(m_pQuitLevelMenuItem);
	SAFE_DELETE_PTR( m_pQuitLevelMenuItem );

	// get rid of debug menu
	DebugSystems::C_DebugHUDMenu::GetInstance()->Active( false );
	
	#ifndef CFG_GOLD
	m_pGameDebug->Close();
	#endif
	
	SAFE_DELETE_PTR( m_pLevelInstance );
	m_LevelHandle = NULL;
	m_pLevelResource = NULL;

	m_pGameWorld->Destroy();
	SAFE_DELETE_PTR( m_pGameWorld );

	if (m_pActorStudioMessaging)
		m_pActorStudioMessaging->Exit();
	m_pActorStudioMessaging = 0;

	return S_OK;
}

void C_GameWorldLayer::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
	case Resource::E_Notify_DataReady:
		{
			m_pLevelResource = m_LevelHandle;
			m_pLevelInstance = new GameResource::C_LevelInstance( m_pGameWorld->GetSceneGraph(), GetGameWorld()->GetLevelSceneManager(), m_LevelHandle );			
			m_pGameWorld->GetSceneGraph()->Insert(m_pLevelInstance);
		
			GetGameWorld()->SetSunDirection( m_pLevelResource->GetRenderLevel()->GetLightList().GetSunDirection() );
			
			#ifndef CFG_GOLD
			m_pGameDebug->InitPlatformDebug( m_pLevelResource->GetModelData(), m_pLevelResource->GetRenderModelData() );
			#endif
		}
		break;
	case Resource::E_Notify_DataError:
	case Resource::E_Notify_NoData:
		SAFE_DELETE_PTR( m_pLevelInstance );			
		break;
	}
	LOGINFO( Resource::g_Log, "Notify pResource=0x%08x, ['%s'] \"%s\" %d, 0x%08x", pResource, pResource->GetResourceTypeName().Chars(), pResource->GetResourceFilename().Chars(), result, pData );
}

Debug::Bool g_CollideRaytrace("Gravity.Debug.Raytrace.Enable", true);
Debug::Bool g_CollideRaytraceResults("Gravity.Debug.Raytrace.ShowResults", false);

Debug::Bool g_CollideTransparentRaytrace("Gravity.Debug.TransparentRaytrace.Enable", false);
Debug::Bool g_CollideTransparentRaytraceResults("Gravity.Debug.TransparentRaytrace.ShowResults", false);

Debug::Bool g_PhysicsDebugCollideRayVehicleSolid( "Gravity.Debug.Raytrace.Filter.VehicleSolid", true );
Debug::Bool g_PhysicsDebugCollideRayWheelSolid( "Gravity.Debug.Raytrace.Filter.WheelSolid", true );
Debug::Bool g_PhysicsDebugCollideRayCharacterSolid( "Gravity.Debug.Raytrace.Filter.CharacterSolid", true );
Debug::Bool g_PhysicsDebugCollideRayDynamicObjectSolid( "Gravity.Debug.Raytrace.Filter.DynamicObjectSolid", true );
Debug::Bool g_PhysicsDebugCollideRayRagdollSolid( "Gravity.Debug.Raytrace.Filter.RagdollSolid", true );
Debug::Bool g_PhysicsDebugCollideRayShootable( "Gravity.Debug.Raytrace.Filter.Shootable", false );
Debug::Bool g_PhysicsDebugCollideRayCameraSolid( "Gravity.Debug.Raytrace.Filter.CameraSolid", false );
Debug::Bool g_PhysicsDebugCollideRayPuddle( "Gravity.Debug.Raytrace.Filter.Puddle", false );
Debug::Bool g_PhysicsDebugCollideRayFluid( "Gravity.Debug.Raytrace.Filter.Fluid", false );

Debug::Bool g_PhysicsDebugCollideTransparentRayVehicleSolid( "Gravity.Debug.TransparentRaytrace.Filter.VehicleSolid", true );
Debug::Bool g_PhysicsDebugCollideTransparentRayWheelSolid( "Gravity.Debug.TransparentRaytrace.Filter.WheelSolid", true );
Debug::Bool g_PhysicsDebugCollideTransparentRayCharacterSolid( "Gravity.Debug.TransparentRaytrace.Filter.CharacterSolid", true );
Debug::Bool g_PhysicsDebugCollideTransparentRayDynamicObjectSolid( "Gravity.Debug.TransparentRaytrace.Filter.DynamicObjectSolid", true );
Debug::Bool g_PhysicsDebugCollideTransparentRayRagdollSolid( "Gravity.Debug.TransparentRaytrace.Filter.RagdollSolid", true );
Debug::Bool g_PhysicsDebugCollideTransparentRayShootable( "Gravity.Debug.TransparentRaytrace.Filter.Shootable", false );
Debug::Bool g_PhysicsDebugCollideTransparentRayCameraSolid( "Gravity.Debug.TransparentRaytrace.Filter.CameraSolid", false );
Debug::Bool g_PhysicsDebugCollideTransparentRayPuddle( "Gravity.Debug.TransparentRaytrace.Filter.Puddle", false );
Debug::Bool g_PhysicsDebugCollideTransparentRayFluid( "Gravity.Debug.TransparentRaytrace.Filter.Fluid", false );

void GetDebugTransparentCollideRayCollisionFilter( Collision::C_CollisionFilter& filter )
{	
	U32 filterFlags = 0;

	if( g_PhysicsDebugCollideTransparentRayVehicleSolid )
		filterFlags |= PolyFlags::kVehicleSolid;

	if( g_PhysicsDebugCollideTransparentRayWheelSolid )
		filterFlags |= PolyFlags::kWheelSolid;

	if( g_PhysicsDebugCollideTransparentRayCharacterSolid )
		filterFlags |= PolyFlags::kCharacterSolid;

	if( g_PhysicsDebugCollideTransparentRayDynamicObjectSolid )
		filterFlags |= PolyFlags::kDynamicObjectSolid;

	if( g_PhysicsDebugCollideTransparentRayRagdollSolid )
		filterFlags |= PolyFlags::kRagdollSolid;

	if( g_PhysicsDebugCollideTransparentRayShootable )
		filterFlags |= PolyFlags::kShootable;

	if( g_PhysicsDebugCollideTransparentRayCameraSolid )
		filterFlags |= PolyFlags::kCameraSolid;

	if( g_PhysicsDebugCollideTransparentRayPuddle )
		filterFlags |= PolyFlags::kPuddle;

	if( g_PhysicsDebugCollideTransparentRayFluid )
		filterFlags |= PolyFlags::kFluid;

	filter.m_CollisionMask = filterFlags;
	filter.m_CollisionFlags = 0xffffffff;
}

void GetDebugCollideRayCollisionFilter( Collision::C_CollisionFilter& filter )
{	
	U32 filterFlags = 0;

	if( g_PhysicsDebugCollideRayVehicleSolid )
		filterFlags |= PolyFlags::kVehicleSolid;

	if( g_PhysicsDebugCollideRayWheelSolid )
		filterFlags |= PolyFlags::kWheelSolid;

	if( g_PhysicsDebugCollideRayCharacterSolid )
		filterFlags |= PolyFlags::kCharacterSolid;

	if( g_PhysicsDebugCollideRayDynamicObjectSolid )
		filterFlags |= PolyFlags::kDynamicObjectSolid;

	if( g_PhysicsDebugCollideRayRagdollSolid )
		filterFlags |= PolyFlags::kRagdollSolid;

	if( g_PhysicsDebugCollideRayShootable )
		filterFlags |= PolyFlags::kShootable;

	if( g_PhysicsDebugCollideRayCameraSolid )
		filterFlags |= PolyFlags::kCameraSolid;

	if( g_PhysicsDebugCollideRayPuddle )
		filterFlags |= PolyFlags::kPuddle;

	if( g_PhysicsDebugCollideRayFluid )
		filterFlags |= PolyFlags::kFluid;

	filter.m_CollisionMask = filterFlags;
	filter.m_CollisionFlags = 0xffffffff;
}

void DebugCollideRaytrace( Gravity::C_Scene* pScene )
{
	if(!g_CollideRaytrace)
		return;

	Collision::C_Ray* ray= new Collision::C_Ray;

	Collision::C_CollisionFilter rayCollisionFilter;
	GetDebugCollideRayCollisionFilter( rayCollisionFilter );

	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

	Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

	Maths::HVec3 start = Maths::HVec3( 0.2f, -0.2f, 0.0f ) * cameraMatrix;
	Maths::HVec3 end = Maths::HVec3( 0.2f, -0.2f, 30.0f ) * cameraMatrix;

	ray->InitialiseRay( start, Maths::C_Normal(end - start), (end - start).Length(), &rayCollisionFilter, Maths::HF32(0.3f) );
	ray->m_bDoubleSided = false;
							
	pScene->DoRayTrace( *ray, pScene->m_Stack, false );
	//pScene->DoDeferredRayTrace(ray, NULL, pScene->GetInternalGroupHandle(), pScene->GetInternalTaskPolicy());

	Scheduler::Flush(pScene->GetInternalGroupHandle());

	if(ray->m_result.m_pShape)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( ray->m_start, ray->m_end, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );

		if(g_CollideRaytraceResults)
		{
			Util::C_String surfaceString;
			surfaceString.Printf("Material:%s|%s\n0x%08x\n%s", GetEffectMaterialName( (E_EffectMaterial)ray->m_result.GetEffectMaterial() ).Chars(), GetPhysicsMaterialName( (E_PhysicsMaterial)ray->m_result.GetPhysicsMaterial() ).Chars(),ray->m_result.m_CollisionFlags, PolyFlags::GetCollideFlagsName(ray->m_result.m_CollisionFlags).Chars() );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( ray->m_end, surfaceString );
		}


		Gravity::C_Body* pBody = NULL;
		
		const SharedPhysics::C_GraphNode* pGraphNode = ray->m_result.GetGraphNode();
		if(pGraphNode)
		{
			pBody = pGraphNode->m_pBody;
		}

		if(pBody)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end, ray->m_result.m_normal, 0.5f, 0.2f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end + ray->m_result.m_normal * 0.01f, ray->m_result.m_normal, 0.25f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );

			F32 forceMag = Input::GetGameInput(0)->GetButtonValue( Framework::E_Button_Debug_ButtonRightTrigger )
						 - Input::GetGameInput(0)->GetButtonValue( Framework::E_Button_Debug_ButtonLeftTrigger );
			if( fabs(forceMag) > 0.2f )
			{
				forceMag = forceMag * g_DebugPushForceMag;
				Maths::HVec3 forceDir = (end - start).Normalised().Normal();
				if(!pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
				{
					pBody->Activate();
					pBody->ApplyForce( forceDir * forceMag, ray->m_end );

					F32 arrowScale = 0.0005f;
					if( forceMag > 0.0f )
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end - forceDir * forceMag * arrowScale, forceDir * forceMag * arrowScale, 1.0f, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end - forceDir * forceMag * arrowScale, forceDir, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
					}
					else
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end , forceDir * forceMag * arrowScale, 1.0f, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end, forceDir, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
					}

				}
			}
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end, ray->m_result.m_normal, 0.5f, 0.2f, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end + ray->m_result.m_normal * 0.01f, ray->m_result.m_normal, 0.25f, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
		}
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( ray->m_start, ray->m_end, Maths::RGBA(0xff, 0x00, 0x00, 0xff) );
	}

	delete ray;
}

void DebugCollideTransparentRaytrace( Gravity::C_Scene* pScene )
{
	if(!g_CollideTransparentRaytrace)
		return;

	Collision::C_TransparentRay* ray = new Collision::C_TransparentRay;

	Collision::C_CollisionFilter raySolidCollisionFilter;
	GetDebugCollideRayCollisionFilter( raySolidCollisionFilter );

	Collision::C_CollisionFilter rayTransparentCollisionFilter;
	GetDebugTransparentCollideRayCollisionFilter( rayTransparentCollisionFilter );

	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

	Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

	Maths::HVec3 start = Maths::HVec3( 0.2f, -0.2f, 0.0f ) * cameraMatrix;
	Maths::HVec3 end = Maths::HVec3( 0.2f, -0.2f, 30.0f ) * cameraMatrix;

	ray->InitialiseRay( start, end, &raySolidCollisionFilter, &rayTransparentCollisionFilter);
	ray->SetNumLayers(3);


	pScene->DoDeferredTransparentRayTrace( ray, pScene->GetInternalGroupHandle(), pScene->GetInternalTaskPolicy() );

	Scheduler::Flush(pScene->GetInternalGroupHandle());

	for(U32 i=0; i<ray->m_numLayers; ++i)
	{
		const Maths::HVec3 contact= ray->m_result[i].m_contact;
		const Maths::HVec3 normal = ray->m_result[i].m_normal;

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.05f, Maths::RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact, normal, 0.2f, 0.05f, Maths::RGBA(0, 255, 0));

	}

	//if(ray.m_SolidResult.m_pShape)
	//{
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_SolidResult.m_contact, 0.05f, Maths::RGBA(255,0, 0));
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_SolidResult.m_contact, ray.m_SolidResult.m_normal, 0.2f, 0.05f, Maths::RGBA(255, 0, 0));
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_end, ray.m_start, Maths::RGBA(255, 255, 0));
	//}

	if(ray->m_SolidResult.m_pShape)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( ray->m_start, ray->m_end, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );

		if(g_CollideRaytraceResults)
		{
			Util::C_String surfaceString;
			surfaceString.Printf("Material:%s|%s\n0x%08x\n%s", GetEffectMaterialName( (E_EffectMaterial)ray->m_SolidResult.GetEffectMaterial() ).Chars(), GetPhysicsMaterialName( (E_PhysicsMaterial)ray->m_SolidResult.GetPhysicsMaterial() ).Chars(),ray->m_SolidResult.m_CollisionFlags, PolyFlags::GetCollideFlagsName(ray->m_SolidResult.m_CollisionFlags).Chars() );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( ray->m_end, surfaceString );
		}


		Gravity::C_Body* pBody = NULL;
		
		const SharedPhysics::C_GraphNode* pGraphNode = ray->m_SolidResult.GetGraphNode();
		if(pGraphNode)
		{
			pBody = pGraphNode->m_pBody;
		}

		if(pBody)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end, ray->m_SolidResult.m_normal, 0.5f, 0.2f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end + ray->m_SolidResult.m_normal * 0.01f, ray->m_SolidResult.m_normal, 0.25f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );

			F32 forceMag = Input::GetGameInput(0)->GetButtonValue( Framework::E_Button_Debug_ButtonRightTrigger )
						 - Input::GetGameInput(0)->GetButtonValue( Framework::E_Button_Debug_ButtonLeftTrigger );
			if( fabs(forceMag) > 0.2f )
			{
				forceMag = forceMag * g_DebugPushForceMag;
				Maths::HVec3 forceDir = (end - start).Normalised().Normal();
				if(!pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
				{
					pBody->Activate();
					pBody->ApplyForce( forceDir * forceMag, ray->m_end );

					F32 arrowScale = 0.0005f;
					if( forceMag > 0.0f )
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end - forceDir * forceMag * arrowScale, forceDir * forceMag * arrowScale, 1.0f, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end - forceDir * forceMag * arrowScale, forceDir, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
					}
					else
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end , forceDir * forceMag * arrowScale, 1.0f, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end, forceDir, 0.2f, Maths::RGBA(0xff, 0xff, 0x00, 0xff) );
					}

				}
			}
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( ray->m_end, ray->m_SolidResult.m_normal, 0.5f, 0.2f, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlaneStar( ray->m_end + ray->m_SolidResult.m_normal * 0.01f, ray->m_SolidResult.m_normal, 0.25f, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
		}
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( ray->m_start, ray->m_end, Maths::RGBA(0xff, 0x00, 0x00, 0xff) );
	}
	delete ray;
}



Debug::Bool g_debugAutoFocus("Graphics.Render.PostProcess.MotionBlurDoF.AutoFocus", false);
Debug::Float g_debugAutoFocusSpeed("Graphics.Render.PostProcess.MotionBlurDoF.AutoFocusSpeed", 0.05f);
Debug::Float g_debugAutoFocusDamping("Graphics.Render.PostProcess.MotionBlurDoF.AutoFocusDamping", 0.9f);

void DebugAutoFocus( Gravity::C_Scene* pScene )
{
	if(!g_debugAutoFocus)
	{
		return;
	}

	F32 prevDist = PostProcess::g_debugDoFPlaneInFocus;
	F32 newDist = 100000000.0f;	

	Collision::C_Ray ray;

	Collision::C_CollisionFilter rayCollisionFilter;
	GetDebugCollideRayCollisionFilter( rayCollisionFilter );

	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

	Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

	Maths::HVec3 start = Maths::HVec3( 0.0f, 0.0f, 0.0f ) * cameraMatrix;
	Maths::HVec3 end = Maths::HVec3( 0.0f, 0.0f, 300.0f ) * cameraMatrix;

	ray.InitialiseRay( start, end, &rayCollisionFilter );
							
	pScene->DoRayTrace( ray, pScene->m_Stack, false );

	if(ray.m_result.m_pShape)
	{
		newDist = (ray.m_end - ray.m_start).Length();
	}


	if( prevDist > 1000.0f )
		prevDist = 1000.0f;

	if( prevDist < 0.001f )
		prevDist = 0.001f;

	if( newDist > 1000.0f )
		newDist = 1000.0f;

	if( newDist < 0.001f )
		newDist = 0.001f;

	F32 ooPrevDist = 1.0f / prevDist;
	F32 ooNewDist = 1.0f / newDist;

	F32 diff = ooNewDist - ooPrevDist;

	static F32 speed = 0.0f;
	speed = speed * (1.0f - g_debugAutoFocusDamping) + diff * g_debugAutoFocusSpeed;

	F32 ooFinalDist = ooPrevDist + speed;

	F32 finalDist = 1.0f / ooFinalDist;
	if( finalDist > 1000.0f )
		finalDist = 1000.0f;

	if( finalDist < 0.001f )
		finalDist = 0.001f;

	PostProcess::g_debugDoFPlaneInFocus = finalDist;
}

Debug::Bool g_debugSpawnVehicleOnB("Vehicle.SpawnVehicleOnB", false, Debug::Persist());

HRESULT C_GameWorldLayer::Update()
{
	if(C_GameApp::GetInstance()->IsPaused())
	{
		return S_OK;
	}

	//if( Input::GetGameInput(0)->GetButtonPressedDebounced( Game::E_Button_Accelerate ) )
	//{
	//	LOGINFO( g_Log, "Thing" );
	//}

	//if( Input::GetGameInput(0)->GetButtonPressedDebounced( Framework::E_Button_MenuCancel ) )
	//{
	//	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &Messaging::BuildSimpleMessage( E_GameflowMessage_QuitLevel ) );
	//}

	// spawn model actor
	bool gravGun = MGCam::Switcher().Choice() == MGCam::E_DebugCam_GravGun;
	if( gravGun && (g_SpawnTestModel || Input::GetGameInput(0)->GetButtonPressedDebounced( Framework::E_Button_Debug_ButtonY )) )
	{
		g_SpawnTestModel = false;
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

		Actor::C_ActorCreationDataFactoryHelper<Actors::C_ModelTestActorCreateProcedural> pModelTestActorCreate;

		pModelTestActorCreate->m_WorldMatrix = Store(cameraMatrix);
		pModelTestActorCreate->m_ModelFilename = (const char*)g_SimpleUiModelString;
		pModelTestActorCreate->m_ModelFilename += "|model";

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pModelTestActorCreate.Get());
	}

	if( gravGun && (g_SpawnRagdollModel) )
	{
		g_SpawnRagdollModel = false;
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

		Actor::C_ActorCreationDataFactoryHelper<Actors::C_RagdollModelTestActorCreateProcedural> pModelTestActorCreate;

		pModelTestActorCreate->m_WorldMatrix = Store(cameraMatrix);
		pModelTestActorCreate->m_ModelFilename = (const char*)g_SimpleUiModelString;
		pModelTestActorCreate->m_ModelFilename += "|model";

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pModelTestActorCreate.Get());
	}

	// spawn vehicle test actor
	if( g_SpawnVehicle )
	{
		g_SpawnVehicle = false;
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

		Actor::C_ActorCreationDataFactoryHelper<Actors::C_VehicleTestActorCreate> pVehicleTestActorCreate;

		pVehicleTestActorCreate->m_WorldMatrix = Store(cameraMatrix);
		pVehicleTestActorCreate->m_ModelFilename = ".\\models\\vehicles\\testcar.model";

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pVehicleTestActorCreate.Get());

		// TODO: [rthomas]	Implement again
		/*if (m_pGameWorld && m_pGameWorld->GetCrowdSystem())
		{
			m_pGameWorld->GetCrowdSystem()->TriggerRaceEvent(AnimatedCrowd::E_CROWD_EVENT_RACE_STARTED, 0.f);
		}*/
	}

	if( g_SpawnVehicles )
	{
		g_SpawnVehicles = false;

		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

		Actor::C_ActorCreationDataFactoryHelper<Actors::C_VehicleTestActorCreate> pVehicleTestActorCreate;

		pVehicleTestActorCreate->m_WorldMatrix = Store(cameraMatrix);
		pVehicleTestActorCreate->m_ModelFilename = ".\\models\\vehicles\\vehicle_gt500.model|car";

		for (int x=0; x<4; x++)
		{
			for (int y=0; y<5; y++)
			{
				Maths::HMat43 mtx;
				mtx.Translation(Maths::HVec3(x, 0, y));

				pVehicleTestActorCreate->m_WorldMatrix = Store(cameraMatrix * mtx);
				Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pVehicleTestActorCreate.Get());
			}
		}
	}

	// spawn character test actor
	if( g_SpawnCharacter )
	{
		g_SpawnCharacter = false;
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();

		Actor::C_ActorCreationDataFactoryHelper<Actors::C_CharacterTestActorCreate> pCharacterTestActorCreate;

		pCharacterTestActorCreate->m_WorldMatrix = Store(cameraMatrix);
		pCharacterTestActorCreate->m_ModelFilename = ".\\models\\characters\\testcharacter.model";

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pCharacterTestActorCreate.Get());
	}

	if (g_debugSpawnVehicleOnB && Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuCancel))
	{
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();
		
		Actor::C_ActorCreationDataFactoryHelper<Actors::C_VehicleActorCreate> pVehicleActorCreate;
		
		pVehicleActorCreate->m_WorldMatrix = Store(cameraMatrix);
		pVehicleActorCreate->m_pGameWorld = GetGameWorld();
		pVehicleActorCreate->m_ModelFilename = "models/vehicles/vehicle_gt500.model";
		
		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pVehicleActorCreate.Get());
	}
	
	if ( g_SpawnAnimationTestActor )
	{
		g_SpawnAnimationTestActor = false;

		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();
		
		// Force Y to be aligned to world up vector.
		cameraMatrix = Maths::HMat43(Maths::AxisY(Maths::C_Normal::AxisPositiveY()), Maths::AxisX(Maths::C_Normal(cameraMatrix.AX())), cameraMatrix.AW());

		Actor::C_ActorCreationDataFactoryHelper<Actors::C_AnimationTestActorCreate> pCreate;

		pCreate->m_WorldMatrix = Store(cameraMatrix);
		
		static const char* kAnimationAssets[3][3] =
		{
			{
				//"animations/renwick_skeleton.skeleton", 
				"animations/Renwick.skeleton", 
				"animations/Ply_m_ar1_walk_f01_nm.cycle",
				"models/Characters/Renwick.model"
			},
			{
				"animations/capsule_skeleton.skeleton",
				"animations/capsule_animated_material.cycle",
				"models/testmodels/capsule_animated_material.model"
			},
			{
				"test/Bond.skeleton",
				"animations/crowd/stand/Crwd_Stand03_Standing_Idle01.cycle",
				"test/Bond.model"
			}
			//{
			//	"animations/Bond01Tropical.skeleton",
			//	"animations/JB_Walk.cycle",
			//	"models/characters/Bond01Tropical.model"
			//}
		};
		
		const char** assets = kAnimationAssets[g_AnimationActorType.Value()];
		pCreate->m_skeletonFilename = assets[0];
		pCreate->m_cycleFilename = assets[1];
		pCreate->m_modelFilename = assets[2];
		pCreate->m_pWorldRenderScene = GetGameWorld()->GetLevelSceneManager()->GetWorldRenderScene();

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pCreate.Get());
	}

	if ( g_SpawnParticleEventTestActor )
	{
		g_SpawnParticleEventTestActor = false;
		
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
		Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();
		Actor::C_ActorCreationDataFactoryHelper<Actors::C_EffectTestActorCreate> pCreate;
	
		pCreate->m_WorldMatrix = Store(cameraMatrix);
		pCreate->m_ParticleEventIndex = g_ParticleEventIndex;

		Maths::HVec3 vCollisionPosition(0.0f, -10000.0f, 0.0f);
		Maths::C_Normal vCollisionNormal;// = Maths::C_Normal::AxisPositiveY();
		{
			Collision::C_Ray ray;
			Collision::C_CollisionFilter rayCollisionFilter = PolyFlags::kShootable;

			U32 filterFlags = 0;
			filterFlags |= PolyFlags::kVehicleSolid;
			filterFlags |= PolyFlags::kWheelSolid;
			filterFlags |= PolyFlags::kCharacterSolid;
			filterFlags |= PolyFlags::kDynamicObjectSolid;
			filterFlags |= PolyFlags::kRagdollSolid;

			rayCollisionFilter.m_CollisionMask = filterFlags;
			rayCollisionFilter.m_CollisionFlags = 0xffffffff;

			Maths::HVec3 start = Maths::HVec3( 0.0f, 0.0f, 0.0f ) * cameraMatrix;
			Maths::HVec3 end = start - Maths::HVec3( 0.0f, 100.0f, 0.0f );
			ray.InitialiseRay( start, end, &rayCollisionFilter );
			m_pGameWorld->GetPhysicsScene()->DoRayTrace( ray, m_pGameWorld->GetPhysicsScene()->m_Stack, false );

			if(ray.GetResult().Hit())
			{
				vCollisionPosition = ray.GetResult().HitPoint();
				vCollisionNormal = ray.GetResult().Normal();
			}
		}

		pCreate->m_vCollisionPosition = vCollisionPosition;
		pCreate->m_vCollisionNormal = vCollisionNormal;

		Actor::C_ActorHandle actorHandle = m_pGameWorld->GetActorList()->CreateActor(pCreate.Get());
	}

	GetGameWorld()->Update();

	m_pActorStudioMessaging->Update();

	Vehicle::C_VehicleModelManager::Instance().Update();
	
	return S_OK;
}

// this update doesn't get paused
HRESULT C_GameWorldLayer::GlobalUpdate()
{
	GetGameWorld()->GlobalUpdate();

	bool gravGun = MGCam::Switcher().Choice() == MGCam::E_DebugCam_GravGun;
	if (gravGun)
	{
		DebugCollideRaytrace( m_pGameWorld->GetPhysicsScene() );
		DebugCollideTransparentRaytrace(m_pGameWorld->GetPhysicsScene());
	}

	DebugAutoFocus( m_pGameWorld->GetPhysicsScene() );

	#ifndef CFG_GOLD
	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	Maths::HMat43 mCameraToWorld = view.m_View.GetViewSettings3D().GetCameraToWorld();

	m_pGameDebug->Update( mCameraToWorld, C_GameApp::GetInstance()->IsPaused() );
	#endif

	return S_OK;
}

bool C_GameWorldLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case E_GameflowMessage_QuitLevel:
			C_GameApp::GetInstance()->DestroyPostProcessLayer();
			C_GameApp::GetInstance()->DestroyGameWorldLayer();
		break;

		default:
			handledMessage = false;
		break;
	}

	return handledMessage;
}

HRESULT C_GameWorldLayer::BeginFrame()
{
	m_pGameWorld->BeginFrame();
	return S_OK;
}

HRESULT C_GameWorldLayer::PreRenderSplitscreen()
{
	m_pGameWorld->PreRenderSplitscreen();

	return S_OK;
}

HRESULT C_GameWorldLayer::RenderSplitscreenWithAA()
{
	Render::BeginPostEffectStates();
	
	m_pGameWorld->RenderSplitscreenWithAA();

	Render::EndPostEffectStates();

	return S_OK;
}

HRESULT C_GameWorldLayer::RenderSplitscreenWithoutAA()
{
	m_pGameWorld->RenderSplitscreenWithoutAA();

	m_pGameWorld->DebugRender( DbUtil::DefaultDebugRenderSys()->Immediate() );

	return S_OK;
}

HRESULT C_GameWorldLayer::RenderFullscreen()
{
#ifndef CFG_GOLD
	m_pGameDebug->Render2d();
#endif

	return S_OK;
}

HRESULT C_GameWorldLayer::EndFrame()
{
	m_pGameWorld->EndFrame();
	return S_OK;
}

C_GameWorld* C_GameWorldLayer::GetGameWorld()
{
	return m_pGameWorld;
}
} // namespace Game
