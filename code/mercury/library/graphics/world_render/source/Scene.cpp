
#include "stdafx.h"

#define SCENEGRAPH_ALLOW_INCLUDE	// only for Scene.cpp

#include <renderingdata/renderingdata.h>
#include <world_render/Scene.h>
#include <world_render/RenderLevel.h>
#include <scenegraph/scenegraphImpl.h>
#include <SceneGraph/RenderingData/StaticScene.h>
#include <SceneGraph/ListNode.h>


namespace WorldRender
{

C_LevelInstance::C_LevelInstance( SceneGraph::C_SceneGraph *pSceneGraph, C_Scene* pScene, C_RenderLevel* pRenderLevel, const U32 routes )
	:	m_Routes( routes ), 
		m_pScene( pScene ), 
		m_pRenderLevel( pRenderLevel ),
		SceneGraph::C_LevelInstance(pSceneGraph, &pRenderLevel->GetScene())
{
	// Change the type of the list in the scene graph so that portal culling works for dynamic objects.
	if(pRenderLevel->GetPortalSystem().GetRoomCount() > 1)
	{
		m_pScene->m_pSceneGraph->SetGatherListType(SceneGraph::I_ListNode::LISTNODETYPE_PORTAL_LIST);
		m_pScene->m_pSceneGraph->ReserveModelInstanceFlatLists(pRenderLevel->GetPortalSystem().GetRoomCount());
		m_pScene->m_pSceneGraph->AddPortalSystem(&pRenderLevel->GetPortalSystem());
	}

	m_pScene->m_RenderLevelInstanceList.PushBack( *this );
	m_pScene->GetLightSamplePointScene().AddLightSamplePoints( &m_pRenderLevel->GetLightSamplePoints() );

	// copy global dynamic material list
	m_pMaterialListData = new RenderingData::Target::MaterialListData;
	m_pMaterialListData->CopyData(pRenderLevel->GetMaterialList());
	m_pMaterialListTypes = &pRenderLevel->GetMaterialList();
	m_pMaterialListDmaData = m_pMaterialListData->GetDmaData();
}

C_LevelInstance::~C_LevelInstance()
{
	m_pScene->GetLightSamplePointScene().RemoveLightSamplePoints( &m_pRenderLevel->GetLightSamplePoints() );
	m_pScene->m_RenderLevelInstanceList.Erase( Btl::List<C_LevelInstance>::Iterator( this ) );
	delete m_pMaterialListData;
}

void C_LevelInstance::SetRoutes( const U32 routes )
{
	m_Routes = routes;
}

void C_LevelInstance::LoadAllRequested()
{
	m_pRenderLevel->RenderingDataStreamerLoadAllRequested();
	Build();
}

void C_LevelInstance::UpdateView(const SceneGraph::C_View& view)
{
	m_pRenderLevel->UpdateView(view);
}

bool C_LevelInstance::UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests)
{
	for (int i = 0; i < nRequests; i++)
		pRequests[i].groupMask = m_Routes;

	bool bDataStillLoading = m_pRenderLevel->UpdateStreaming(pRequests, nRequests);
	
	// we only need to recache the hierarchy if the nodes have changed in the rendering data
	if(m_pRenderLevel->NodesChanged() || GetGroupMask() != m_Routes)
	{
		SetGroupMask(m_Routes);
		Build();
	}

	return bDataStillLoading;
}

void C_LevelInstance::SetParameter(const MaterialParameter& parameter, const void* pData)
{
	if (parameter.index == -1)
		return;
	RenderingData::Material& material = m_pMaterialListData->GetMaterial(parameter.index);
	material.SetData(parameter.offset, pData, parameter.size);
}

U8* C_LevelInstance::GetParameter(const MaterialParameter& parameter)
{
	if (parameter.index >= m_pMaterialListData->GetNumMaterials())
		return 0;

	RenderingData::Material& material = m_pMaterialListData->GetMaterial(parameter.index);
	return material.GetData(parameter.offset);
}

const U8* C_LevelInstance::GetParameter(const MaterialParameter& parameter) const
{
	RenderingData::ConstMaterialReference material = m_pMaterialListData->RenderingData::MaterialListData::GetMaterial(parameter.index);
	return material.GetData(parameter.offset);
}

C_Scene::C_Scene(SceneGraph::C_SceneGraph* pSceneGraph)
	: m_pSceneGraph( pSceneGraph )
	, m_LightSamplePointScene( this )
{
}

C_Scene::~C_Scene()
{
}

void C_Scene::LoadAllRequested()
{
	Btl::List<C_LevelInstance>::Iterator i = m_RenderLevelInstanceList.Begin();
	while( i != m_RenderLevelInstanceList.End())
	{
		C_LevelInstance& levelInstance = *i;
		levelInstance.LoadAllRequested();
		++i;
	}
}

void C_Scene::UpdateView(const SceneGraph::C_View& view)
{
	Btl::List<C_LevelInstance>::Iterator i = m_RenderLevelInstanceList.Begin();
	while( i != m_RenderLevelInstanceList.End())
	{
		C_LevelInstance& levelInstance = *i;
		levelInstance.UpdateView( view );
		++i;
	}
}

bool C_Scene::UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests)
{
	bool bStreamDataStillLoading = false;
	Btl::List<C_LevelInstance>::Iterator i = m_RenderLevelInstanceList.Begin();
	while( i != m_RenderLevelInstanceList.End())
	{
		C_LevelInstance& levelInstance = *i;
		bStreamDataStillLoading |= levelInstance.UpdateStreaming( pRequests, nRequests );
		++i;
	}

	return bStreamDataStillLoading;
}

void C_Scene::DebugRender( DbUtil::I_DebugRender* pDebugRender )
{
	//GetCamera
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Maths::C_Vertex vCameraPos = viewSettings3D.GetCameraPosition();

	Maths::HMat43 m;
	m.SetIdentity();

	Btl::List<C_LevelInstance>::Iterator i = m_RenderLevelInstanceList.Begin();
	while( i != m_RenderLevelInstanceList.End())
	{
		C_LevelInstance& levelInstance = *i;
		C_RenderLevel* pRenderLevel = levelInstance.m_pRenderLevel;
		//Render debug Lights
		pRenderLevel->DebugRender(pDebugRender, m, vCameraPos);
		++i;
	}

	GetLightSamplePointScene().DebugRender( pDebugRender, vCameraPos );
}

C_LightSamplePointScene& C_Scene::GetLightSamplePointScene()
{
	return m_LightSamplePointScene;
}

SceneGraph::C_SceneGraph* C_Scene::GetSceneGraph()
{
	return m_pSceneGraph;
}

} // namespace WorldRender
