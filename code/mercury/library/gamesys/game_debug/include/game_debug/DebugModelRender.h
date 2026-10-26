#ifndef GAMESYS_GAME_DEBUG_DEBUG_MODEL_RENDER_H
#define GAMESYS_GAME_DEBUG_DEBUG_MODEL_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/containers/list.h>
#include <game_resource/ModelResource.h>
#include <gravity/sdk/BodyHelper.h>

namespace Gravity
{
	class C_Scene;
	struct C_ExternalConstants;
}

namespace GameResource
{
	class C_ModelData;
	class C_LevelSceneManager;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_RenderModelData;
	class C_EnvironmentSampler;
}

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace GameDebug
{
// C_DebugModelDataRender
class C_DebugModelDataRender
{
public:
	C_DebugModelDataRender();
	virtual ~C_DebugModelDataRender();
	void CreateModelInstances();
	void DestroyModelInstances();
	void UpdateModelInstances( const bool gamePaused );
	void Update( const bool enable, const bool gamePaused );

	virtual void CreateModel() {}
	virtual void DestroyModel() {}

	void SetCollisionFilter( const Collision::C_CollisionFilter& filter );

	void SetEnvMapOverride(const void* pEnvMapGPUTextureOverride);

protected:	
	Collision::C_CollisionFilter				m_CollisionFilter;

	GameResource::C_LevelSceneManager*			m_pLevelSceneManager;

	GameResource::C_ModelData*					m_pModelData;
	WorldRender::C_RenderModelData*				m_pRenderModelData;

	SceneGraph::C_SceneGraph*					m_pSceneGraph;
	Btl::Vector<WorldRender::C_ModelInstance*>	m_ModelInstances;

	Gravity::C_ExternalConstants*				m_pExternalConstants;
	Gravity::C_Scene*							m_pPhysicsScene;
	Btl::Vector<Gravity::C_Body*>				m_BodyList;

	Maths::HMat43								m_Origin;
	WorldRender::C_EnvironmentSampler*			m_pEnvSampler;	

	const void*									m_pEnvMapGPUTextureOverride;
};

// C_DebugModelRender
class C_DebugModelRender : public C_DebugModelDataRender
{
public:
	C_DebugModelRender();
	~C_DebugModelRender();
	void Init( GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_ExternalConstants* pExternalConstants );
	void Close();
	void CreateModel();
	void DestroyModel();
	void Update( const Maths::HMat43& mCameraToWorld, const bool gamePaused );

public:
	GameResource::C_ModelHandle					m_ModelHandle;
	GameResource::C_ModelResource*				m_pModelResource;
	Maths::HMat43								m_WorldOrigin;
};

// C_DebugPlatformRender
class C_DebugPlatformRender : public C_DebugModelDataRender
{
public:
	C_DebugPlatformRender();
	~C_DebugPlatformRender();
	void Init( GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_ExternalConstants* pExternalConstants, GameResource::C_ModelData* pModelData, WorldRender::C_RenderModelData* pRenderModelData);
	void Close();
	void Update( const bool gamePaused );
public:
};

} // namespace GameDebug

#endif // GAMESYS_GAME_DEBUG_DEBUG_MODEL_RENDER_H
