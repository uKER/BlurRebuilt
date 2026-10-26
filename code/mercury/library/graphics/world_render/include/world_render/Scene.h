#ifndef WORLD_RENDER_SCENE_H
#define WORLD_RENDER_SCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/archive/serialise.h>
#include "world_render/LightSamplePointScene.h"
#include <scenegraph/renderingdata/StaticScene.h>
#include "world_render/MaterialParameter.h"

namespace DbUtil
{
	class I_DebugRender;
}

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
}

namespace RenderingData 
{
	namespace Target 
	{
		class RequestInfo;
	}
}

namespace WorldRender
{

class C_Scene;
class C_RenderLevel;

//! C_LevelInstance
class C_LevelInstance : public Btl::ListElementDouble<C_LevelInstance>, public SceneGraph::C_LevelInstance
{
friend class C_Scene;
public:
	//! This object is created after a C_RenderLevel has been loaded.
	//! When this constructor is called, it will add the static parts of the level into the scenegraph scene.
	C_LevelInstance( SceneGraph::C_SceneGraph *pSceneGraph, C_Scene* pScene, C_RenderLevel* pRenderLevel, const U32 routes = 0 );

	//! When this object is destroyed, the static parts of level that were added in the constructor will be removed from the scenegraph scene.
	~C_LevelInstance();

	// set data to level parameter
	void				SetParameter(const MaterialParameter& parameter, const void* pData);

	// get current data from level parameter
	U8*					GetParameter(const MaterialParameter& parameter);
	const U8*			GetParameter(const MaterialParameter& parameter) const;

	void SetRoutes( const U32 routes );
	const C_RenderLevel* GetRenderLevel() const
	{
		return m_pRenderLevel;
	}
private:
	void UpdateView(const SceneGraph::C_View& view);
	bool UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests);

	void LoadAllRequested();

private:
	U32 m_Routes;
	C_Scene* m_pScene;
	C_RenderLevel* m_pRenderLevel;
};

//! C_Scene
class C_Scene
{
friend class C_LevelInstance;

public:
	C_Scene(SceneGraph::C_SceneGraph* pSceneGraph);
	~C_Scene();	

	void LoadAllRequested();

	void UpdateView(const SceneGraph::C_View& view);
	bool UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests);

	void DebugRender( DbUtil::I_DebugRender* pDebugRender );		

	C_LightSamplePointScene& GetLightSamplePointScene();

	SceneGraph::C_SceneGraph* GetSceneGraph();

	Btl::List<C_LevelInstance>& GetRenderLevelInstanceList()
	{
		return m_RenderLevelInstanceList;
	}
private:

	// points to active scenegraph
	SceneGraph::C_SceneGraph* m_pSceneGraph;

	// list of active level instances
	Btl::List<C_LevelInstance> m_RenderLevelInstanceList;

	C_LightSamplePointScene m_LightSamplePointScene;
};

} // namespace WorldRender

#endif // WORLD_RENDER_SCENE_H

