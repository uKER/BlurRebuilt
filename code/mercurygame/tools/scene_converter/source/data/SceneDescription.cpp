#include "stdafx.h"


#include <misc_shared/DataStructure/StringTable.h>

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/SceneDescription.h>

// C_DataSceneDescription

C_DataSceneDescription::C_DataSceneDescription()
{
	m_RouteNodesList.Reserve(10);
}

C_DataSceneDescription::~C_DataSceneDescription()
{
}

const Util::C_String C_DataSceneDescription::GetName() const
{
	return Util::C_String("C_DataSceneDescription");
}

ModelData::C_ModelData& C_DataSceneDescription::GetModelData()
{
	return m_ModelData;
}

const ModelData::C_ModelData& C_DataSceneDescription::GetModelData() const
{
	return m_ModelData;
}

C_AftkNodeList& C_DataSceneDescription::GetStaticNodes()
{
	return m_StaticNodes;
}

const C_AftkNodeList& C_DataSceneDescription::GetPortalRoomsNodes() const
{
	return m_PortalRoomsNodes;
}

const C_AftkNodeList& C_DataSceneDescription::GetStaticNodes() const
{
	return m_StaticNodes;
}

void C_DataSceneDescription::AddRoute( const S32 routeNo )
{
	if( routeNo >= GetRouteCount() )
	{
		m_RouteNodesList.Resize( routeNo + 1 );
	}
}

void C_DataSceneDescription::AddRouteNode( const Aftk::C_SceneNode* pAftkNode, const S32 routeNo )
{
	AddRoute(routeNo);

	m_RouteNodesList[routeNo].PushBack( pAftkNode );
}

const S32 C_DataSceneDescription::GetRouteCount() const
{
	return m_RouteNodesList.Size();
}

C_AftkNodeList& C_DataSceneDescription::GetRouteNodes( const S32 routeNo )
{
	return m_RouteNodesList[routeNo];
}

const C_AftkNodeList& C_DataSceneDescription::GetRouteNodes( const S32 routeNo ) const
{
	return m_RouteNodesList[routeNo];
}

const C_AftkNodeList& C_DataSceneDescription::GetLightNodes() const
{
	return m_LightNodes;
}

const C_AftkNodeList& C_DataSceneDescription::GetNavMeshNodes() const
{
	return m_NavMeshNodes;
}

const C_AftkNodeList& C_DataSceneDescription::GetCrowdBaseInstanceNodes() const
{
	return m_CrowdBaseInstanceNodes;
}

const S32 C_DataSceneDescription::GetNumCrowdSkeletonNodes() const
{
	return m_CrowdSkeletonNodes.Size();
}

const Aftk::C_SceneNode* C_DataSceneDescription::GetCrowdSkeletonNode(const S32 index) const
{
	BIZ_ASSERT(index >= 0 && index < GetNumCrowdSkeletonNodes());

	return m_CrowdSkeletonNodes[index];
}

const C_AftkNodeList& C_DataSceneDescription::GetCrowdInstanceNodes() const
{
	return m_CrowdInstanceNodes;
}

void C_DataSceneDescription::AddStaticMeshNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_StaticNodes.PushBack( pAftkNode );
}

void C_DataSceneDescription::AddNavMeshNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_NavMeshNodes.PushBack( pAftkNode );
}

void C_DataSceneDescription::AddPortalRoomsNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_PortalRoomsNodes.PushBack( pAftkNode );
}

void C_DataSceneDescription::AddPvsZonesNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_PvsZonesNodes.PushBack( pAftkNode );
}

void C_DataSceneDescription::AddLightNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_LightNodes.PushBack( pAftkNode );
}

bool C_DataSceneDescription::ExportModelData( Sys::I_File* pFile ) const
{
	return GetModelData().Export( pFile );
}

void C_DataSceneDescription::AddCrowdBaseInstanceNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_CrowdBaseInstanceNodes.PushBack( pAftkNode );
}

void C_DataSceneDescription::AddCrowdSkeletonNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_CrowdSkeletonNodes.PushBack( pAftkNode );
}

void C_DataSceneDescription::AddCrowdInstanceNode( const Aftk::C_SceneNode* pAftkNode )
{
	m_CrowdInstanceNodes.PushBack( pAftkNode );
}

