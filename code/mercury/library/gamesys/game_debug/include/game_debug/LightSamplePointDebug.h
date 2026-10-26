#ifndef GAME_DEBUG_LIGHT_SAMPLE_POINT_DEBUG_H
#define GAME_DEBUG_LIGHT_SAMPLE_POINT_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/containers/list.h>
#include <game_resource/ModelResource.h>
#include <world_render/LightSamplePoints.h>	// [Blur2PC] C_LightSamplePoint is held by value below

namespace GameResource
{
	class C_LevelSceneManager;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_RenderModelData;
	class C_LightSamplePoint;
}

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace GameDebug
{


// C_SamplePointInfo
class C_SamplePointInfo
{
public:
	WorldRender::C_ModelInstance* m_pModelInstance;
	WorldRender::C_LightSamplePoint m_LightSamplePoint;
};


// C_LightSamplePointDebug
class C_LightSamplePointDebug
{
public:
	C_LightSamplePointDebug();
	~C_LightSamplePointDebug();

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

	Btl::Vector<C_SamplePointInfo>		m_SamplePointInfo;
};

} // namespace GameDebug

#endif // GAME_DEBUG_LIGHT_SAMPLE_POINT_DEBUG_H
