#ifndef GAME_RESOURCE_LEVEL_SCENE_MANAGER_H
#define GAME_RESOURCE_LEVEL_SCENE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/containers/list.h>

#include <resource/Resource.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <game_resource/ModelData.h>

#include <world_render/Scene.h>
#include <gravity/importer/Importer.h>

#include <game_resource/LevelResource.h>

namespace WorldRender
{
	class C_RenderLevel;
	class C_RenderModelData;
};

namespace SceneGraph
{
	class C_SceneGraph;
	class C_LevelInstance;
	class C_View;
}

namespace Gravity
{
	class C_Scene;
	class C_ImportScene;
	class C_ImportShape;
	class C_ImportSceneStaticInstance;
}

namespace RenderingData 
{
	namespace Target 
	{
		class RequestInfo;
	}
}

namespace GameResource
{

class C_LevelSceneManager;
class C_LevelInstance;
class C_LevelResource;
class C_PhysicsLevel;

//! C_LevelInstance
class C_LevelInstance : public WorldRender::C_LevelInstance
{
public:
	//! This object is created after a C_LevelData has been loaded.
	//! When this constructor is called, it will add the static parts of the level into the scenegraph scene and physics scene.
	C_LevelInstance( SceneGraph::C_SceneGraph *pSceneGraph, C_LevelSceneManager* pLevelManager, C_LevelResource* pLevelResource, const U32 routeMask = 0xffffffff, const Collision::C_CollisionFilter collisionFilter = Collision::C_CollisionFilter() );

	//! When this object is destroyed, the static parts of level that were added in the constructor will be removed from the scenegraph scene and physics scene.
	~C_LevelInstance();

	void Update();

	void DebugRender( DbUtil::I_DebugRender* pDebugRender );

	// control the rendering and collision for individual routes
	void EnableRoute( const S32 routeNo );
	void DisableRoute( const S32 routeNo );

	// control the rendering of individual routes
	bool IsRouteRenderEnabled( const S32 routeNo ) const;
	void EnableRouteRender( const S32 routeNo );
	void DisableRouteRender( const S32 routeNo );

	// control the collision of individual routes
	bool IsRouteCollisionEnabled( const S32 routeNo ) const;
	void EnableRouteCollision( const S32 routeNo );
	void DisableRouteCollision( const S32 routeNo );

	// directly set/get the route render enable masks
	void SetRouteRenderMask( const U32 routeRenderMask );
	U32 GetRouteRenderMask() const;

	// directly set/get the route collision enable masks
	void SetRouteCollisionMask( const U32 routeCollisionMask );
	U32 GetRouteCollisionMask() const;

private:
	void UpdatePhysicsRoutes();
	void CreatePhysicsRoutes( const U32 routes );
	void DeletePhysicsRoutes();

	U32 GetRequiredCollisionRoutes() const;

public:
	U32 GetRequiredRenderRoutes() const;

private:
	U32 m_RenderRoutes;
	U32 m_CollisionRoutes;

	U32 m_ActiveCollisionRoutes;

	C_LevelSceneManager* m_pLevelManager;
	C_LevelResource* m_pLevelResource;
	Collision::C_CollisionFilter m_CollisionFilter;
	Gravity::C_ImportSceneStaticInstance m_PhysicsStaticInstance;	
	Ut::Vector<Gravity::C_ImportSceneStaticInstance*> m_pPhysicsRouteStaticInstance;
};

//! C_LevelSceneManager
class C_LevelSceneManager
{
friend class C_LevelInstance;
public:
	C_LevelSceneManager();
	~C_LevelSceneManager();

	bool Initialise( SceneGraph::C_SceneGraph* pSceneGraph, Gravity::C_Scene* pPhysicsScene );

	void Update();
	void UpdateView(const SceneGraph::C_View& view);
	bool UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests);

	void DebugRender( DbUtil::I_DebugRender* pDebugRender );

	Gravity::C_Scene* GetPhysicsScene();
	void SetPhysicsScene(Gravity::C_Scene* p_pPhysicsScene) { m_pPhysicsScene = p_pPhysicsScene; }

	WorldRender::C_Scene* GetWorldRenderScene();
	
	void LoadAllRequested();

	bool StreamDataLoading() const;

private:
	// points to active physics scene
	Gravity::C_Scene* m_pPhysicsScene;

	WorldRender::C_Scene* m_pWorldRenderScene;

	// list of active level instances
	Btl::Vector<C_LevelInstance*> m_LevelInstanceList;

	bool m_StreamDataLoading;
};

} // namespace GameResource

#endif // GAME_RESOURCE_LEVEL_SCENE_MANAGER_H
