#ifndef GAME_DEBUG_CUBEMAP_POINT_DEBUG_H
#define GAME_DEBUG_CUBEMAP_POINT_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/containers/list.h>
#include <game_resource/ModelResource.h>

namespace GameResource
{
	class C_LevelSceneManager;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_RenderModelData;
	struct CubemapLocation;
}

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace GameDebug
{


// C_CubemapPointInfo
class C_CubemapPointInfo
{
public:
	WorldRender::C_ModelInstance* m_pModelInstance;
	const WorldRender::CubemapLocation* m_pCubemapLocation;
};


// C_LightSamplePointDebug
class C_CubemapPointDebug
{
public:
	C_CubemapPointDebug();
	~C_CubemapPointDebug();

	void Init( GameResource::C_LevelSceneManager* pLevelSceneManager );
	void Close();

	void Update( const Maths::HVec3 PCREF pos );

private:
	void LoadModel();
	void ReleaseModel();
	void UpdateModelInstances( const Maths::HVec3 PCREF pos, bool renderInstances );


private:
	GameResource::C_LevelSceneManager*	m_pLevelSceneManager;
	GameResource::C_ModelHandle			m_ModelHandle;
	GameResource::C_ModelResource*		m_pModelResource;

	Btl::Vector<C_CubemapPointInfo>		m_CubemapPointInfo;
};

} // namespace GameDebug

#endif // GAME_DEBUG_CUBEMAP_POINT_DEBUG_H
