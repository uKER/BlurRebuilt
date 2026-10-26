#ifndef GAME_RESOURCE_LEVEL_RESOURCE_H
#define GAME_RESOURCE_LEVEL_RESOURCE_H

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

namespace MetaDebug
{
	class C_MetaNode;
}

namespace WorldRender
{
	class C_RenderLevel;
	class C_RenderModelData;
};

namespace AI
{
	class C_NavMesh;
}

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
};

namespace GameResource
{

class C_LevelSceneManager;
class C_LevelInstance;
class C_LevelResource;
class C_PhysicsLevel;

typedef Resource::C_Handle<C_LevelResource> C_LevelHandle;

//! C_LevelResource
class C_LevelResource : public Resource::C_Resource<C_LevelResource>
{
	friend class C_LevelInstance;
public:
	C_LevelResource();
	~C_LevelResource();

	// Inherited from C_Resource
	// |------------------------
	bool LoadResourceData();
	bool FreeResourceData();
	bool PostLoadResourceData();
	bool PreFreeResourceData();

	S32 GetEvictDelay();
	static Util::C_String StaticGetResourceTypeName();
	// |------------------------

	bool LoadLevelData( Sys::I_File* pFile );
	bool LoadPhysicsData( Sys::I_File* pFile );
	bool LoadRenderData( Sys::I_File* pFile );
	bool InitStreaming( const Util::C_String& streamFileName );

	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m );

	GameResource::C_ModelData*	GetModelData();
	const GameResource::C_ModelData*	GetModelData() const;
	WorldRender::C_RenderModelData* GetRenderModelData();
	const WorldRender::C_RenderModelData* GetRenderModelData() const;
	WorldRender::C_RenderLevel* GetRenderLevel();
	C_PhysicsLevel*				GetPhysicsLevel();
	AI::C_NavMesh*				GetNavMesh() const;

	// returns NULL in gold build
	MetaDebug::C_MetaNode*		GetRootMetaNode() const;

private:
	void						SetDebugNames();

private:
	GameResource::C_ModelData*	m_pModelData;
	WorldRender::C_RenderLevel*	m_pRenderLevel;
	C_PhysicsLevel*				m_pPhysicsLevel;
	AI::C_NavMesh*				m_pNavMesh;
};

} // namespace GameResource

#endif // GAME_RESOURCE_LEVEL_RESOURCE_H
