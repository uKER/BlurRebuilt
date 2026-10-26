#include "GameResourcePch.h"

#include <system/serialise/C_FileBlock.h>
#include <system/Memory.h>
#include <gravity/importer/Importer.h>

#include <world_render/RenderLevel.h>

#include <game_resource/GameResource.h>
#include <game_resource/LevelResource.h>
#include <game_resource/PhysicsLevel.h>
#include <game_resource/LevelSceneManager.h>

#include <NavMesh/NavMesh.h>

namespace GameResource
{

Debug::Int g_RouteOverride( "Gamesys.Level.RouteOverride", -1 );

// C_LevelInstance
C_LevelInstance::C_LevelInstance( SceneGraph::C_SceneGraph *pSceneGraph, C_LevelSceneManager* pLevelManager, C_LevelResource* pLevelResource, const U32 routes, const Collision::C_CollisionFilter collisionFilter )
	: m_pLevelManager( pLevelManager )
	, m_pLevelResource( pLevelResource )
	, m_RenderRoutes( routes )
	, m_CollisionRoutes( routes )
	, m_CollisionFilter(collisionFilter)
	, m_PhysicsStaticInstance( m_pLevelManager->m_pPhysicsScene, m_pLevelResource->GetPhysicsLevel()->GetMainImportScene(), Maths::HVec3(5.f, 20.f, 5.f), Maths::HVec3(50.f, 50.f, 50.f), &m_CollisionFilter, &m_CollisionFilter )
	, WorldRender::C_LevelInstance( pSceneGraph, pLevelManager->m_pWorldRenderScene, pLevelResource->GetRenderLevel() )
	, m_ActiveCollisionRoutes( 0 )
{
	assert( pLevelResource->IsDataReady() );

	m_pLevelManager->m_LevelInstanceList.PushBack( this );

	UpdatePhysicsRoutes();
}

C_LevelInstance::~C_LevelInstance()
{
	DeletePhysicsRoutes();

	for (int i=0; i<m_pLevelManager->m_LevelInstanceList.Size(); i++)
	{
		if (m_pLevelManager->m_LevelInstanceList[i] == this)
		{
			m_pLevelManager->m_LevelInstanceList.EraseByIndex(i);
			break;
		}
	}

	m_pLevelManager = NULL;
	m_pLevelResource = NULL;
}

void C_LevelInstance::EnableRoute( const S32 routeNo )
{
	EnableRouteRender(routeNo);
	EnableRouteCollision(routeNo);
}

void C_LevelInstance::DisableRoute( const S32 routeNo )
{
	DisableRouteRender(routeNo);
	DisableRouteCollision(routeNo);
}

void C_LevelInstance::EnableRouteRender( const S32 routeNo )
{
	m_RenderRoutes |= 1<<routeNo;
}

void C_LevelInstance::DisableRouteRender( const S32 routeNo )
{
	m_RenderRoutes &= ~(1<<routeNo);
}

bool C_LevelInstance::IsRouteRenderEnabled( const S32 routeNo ) const
{
	return (m_RenderRoutes & (1<<routeNo)) != 0;
}

void C_LevelInstance::EnableRouteCollision( const S32 routeNo )
{
	m_CollisionRoutes |= 1<<routeNo;
}

void C_LevelInstance::DisableRouteCollision( const S32 routeNo )
{
	m_CollisionRoutes &= ~(1<<routeNo);
}

bool C_LevelInstance::IsRouteCollisionEnabled( const S32 routeNo ) const
{
	return (m_CollisionRoutes & (1<<routeNo)) != 0;
}

void C_LevelInstance::SetRouteRenderMask( const U32 routeRenderMask )
{
	m_RenderRoutes = routeRenderMask;
}

U32 C_LevelInstance::GetRouteRenderMask() const
{
	return m_RenderRoutes;
}

void C_LevelInstance::SetRouteCollisionMask( const U32 routeCollisionMask )
{
	m_CollisionRoutes = routeCollisionMask;
}

U32 C_LevelInstance::GetRouteCollisionMask() const
{
	return m_CollisionRoutes;
}

U32 C_LevelInstance::GetRequiredCollisionRoutes() const
{
	U32 collisionRoutes = m_CollisionRoutes;
	#ifndef CFG_GOLD
	if( g_RouteOverride != -1 )
	{	
		collisionRoutes = 1 << g_RouteOverride;
	}
	#endif

	return collisionRoutes;
}

U32 C_LevelInstance::GetRequiredRenderRoutes() const
{
	U32 renderRoutes = m_RenderRoutes;
#ifndef CFG_GOLD
	if( g_RouteOverride != -1 )
	{	
		renderRoutes = 1 << g_RouteOverride;
	}
#endif

	return renderRoutes;
}

void C_LevelInstance::UpdatePhysicsRoutes()
{
	const U32 collisionRoutes = GetRequiredCollisionRoutes();

	if(collisionRoutes != m_ActiveCollisionRoutes)
	{
		DeletePhysicsRoutes();
		CreatePhysicsRoutes(collisionRoutes);
	}
}

void C_LevelInstance::DeletePhysicsRoutes()
{
	m_ActiveCollisionRoutes = 0;
	for(S32 i=0; i<m_pPhysicsRouteStaticInstance.Size(); i++)
	{
		delete m_pPhysicsRouteStaticInstance[i];
	}
	m_pPhysicsRouteStaticInstance.Clear();
}

void C_LevelInstance::CreatePhysicsRoutes( const U32 routes )
{
	assert( m_pPhysicsRouteStaticInstance.Size() == 0 );

	m_ActiveCollisionRoutes = routes;
	
	if( m_pLevelManager->m_pPhysicsScene )
	{
		S32 levelRouteCount = m_pLevelResource->GetPhysicsLevel()->GetRouteCount();

		U32 noActiveRoutes = 0;
		for(S32 routeNo=0; routeNo<levelRouteCount; routeNo++)
		{
			const bool includeRoute = ((1 << routeNo) & routes) != 0;
			if(includeRoute)
				noActiveRoutes++;
		}

		m_pPhysicsRouteStaticInstance.Reserve( noActiveRoutes );

		for(S32 routeNo=0; routeNo<levelRouteCount; routeNo++)
		{
			const bool includeRoute = ((1 << routeNo) & routes) != 0;

			Gravity::C_ImportScene* pImportScene = m_pLevelResource->GetPhysicsLevel()->GetRouteImportScene(routeNo);
			if( includeRoute && pImportScene )
			{
				MEMORY_TRACKER_PUSH("gravity.m_pPhysicsRouteInstance");
				Gravity::C_ImportSceneStaticInstance* pNewInstance = new Gravity::C_ImportSceneStaticInstance( m_pLevelManager->m_pPhysicsScene, pImportScene, 
					Maths::HVec3(5.f, 20.f, 5.f), Maths::HVec3(50.f, 50.f, 50.f), &m_CollisionFilter, &m_CollisionFilter );
				m_pPhysicsRouteStaticInstance.PushBack( pNewInstance );
				MEMORY_TRACKER_POP("gravity.m_pPhysicsRouteInstance");
			}
		}
	}
}

void C_LevelInstance::DebugRender( DbUtil::I_DebugRender* pDebugRender )
{
	if(m_pLevelResource)
	{
		Maths::HMat43 idMat;
		idMat.SetIdentity();
		m_pLevelResource->DebugRender( pDebugRender, idMat );
	}
}

void C_LevelInstance::Update()
{	
	UpdatePhysicsRoutes();
	SetRoutes( GetRequiredRenderRoutes() );
}

// C_LevelSceneManager
C_LevelSceneManager::C_LevelSceneManager()
	: m_pWorldRenderScene( NULL )
	, m_StreamDataLoading( true )
{
}

C_LevelSceneManager::~C_LevelSceneManager()
{
	SAFE_DELETE_PTR( m_pWorldRenderScene );
}

bool C_LevelSceneManager::Initialise( SceneGraph::C_SceneGraph* pSceneGraph, Gravity::C_Scene* pPhysicsScene )
{
	m_pWorldRenderScene = new WorldRender::C_Scene( pSceneGraph );
	//m_pSceneGraph = pSceneGraph;
	m_pPhysicsScene = pPhysicsScene;

	return true;
}

void C_LevelSceneManager::LoadAllRequested()
{
	m_pWorldRenderScene->LoadAllRequested();
}

bool C_LevelSceneManager::StreamDataLoading() const
{
	return m_StreamDataLoading;
}

void C_LevelSceneManager::Update()
{
	Btl::Vector<C_LevelInstance*>::Iterator i = m_LevelInstanceList.Begin();
	while( i != m_LevelInstanceList.End())
	{
		C_LevelInstance *levelInstance = *i;
		levelInstance->Update();
		++i;
	}
}

void C_LevelSceneManager::UpdateView(const SceneGraph::C_View& view)
{
	m_pWorldRenderScene->UpdateView(view);
}

bool C_LevelSceneManager::UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests)
{
	return m_pWorldRenderScene->UpdateStreaming(pRequests, nRequests);
}

Gravity::C_Scene* C_LevelSceneManager::GetPhysicsScene()
{
	return m_pPhysicsScene;
}

WorldRender::C_Scene* C_LevelSceneManager::GetWorldRenderScene()
{
	 return m_pWorldRenderScene; 
}

void C_LevelSceneManager::DebugRender( DbUtil::I_DebugRender* pDebugRender )
{
	PERF_EVENT( "C_LevelSceneManager::DebugRender" );

	Btl::Vector<C_LevelInstance*>::Iterator i = m_LevelInstanceList.Begin();
	while( i != m_LevelInstanceList.End())
	{
		C_LevelInstance *levelInstance = *i;
		levelInstance->DebugRender( pDebugRender );
		++i;
	}

	GetWorldRenderScene()->DebugRender( pDebugRender );
}

} // namespace GameResource
