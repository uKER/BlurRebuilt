#ifndef APP_SCENE_DESCRIPTION_H
#define APP_SCENE_DESCRIPTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/SceneDesc.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <converter_shared/ModelData.h>

typedef Btl::Vector<const Aftk::C_SceneNode*> C_AftkNodeList;

//! C_DataSceneDescription

class C_DataSceneDescription : public Tools::ProcessGraph::C_Data
{
public:
	C_DataSceneDescription();

	~C_DataSceneDescription();

	// inherited from C_Data
	const Util::C_String GetName() const;

	ModelData::C_ModelData& GetModelData();

	const ModelData::C_ModelData& GetModelData() const;

	C_AftkNodeList& GetStaticNodes();

	const C_AftkNodeList& GetStaticNodes() const;

	void AddRoute( const S32 routeNo );

	void AddRouteNode( const Aftk::C_SceneNode* pAftkNode, const S32 routeNo );

	const S32 GetRouteCount() const;

	C_AftkNodeList& GetRouteNodes( const S32 routeNo );

	const C_AftkNodeList& GetPortalRoomsNodes() const;

	const C_AftkNodeList& GetRouteNodes( const S32 routeNo ) const;

	const C_AftkNodeList& GetLightNodes() const;

	const C_AftkNodeList& GetNavMeshNodes() const;

	const C_AftkNodeList& GetCrowdBaseInstanceNodes() const;

	const S32 GetNumCrowdSkeletonNodes() const;

	const Aftk::C_SceneNode* GetCrowdSkeletonNode(const S32 index) const;

	const C_AftkNodeList& GetCrowdInstanceNodes() const;

	void AddStaticMeshNode( const Aftk::C_SceneNode* pAftkNode );

	bool ExportModelData( Sys::I_File* pFile ) const;

	void AddNavMeshNode( const Aftk::C_SceneNode* pAftkNode );
	
	void AddPortalRoomsNode( const Aftk::C_SceneNode* pAftkNode );

	void AddPvsZonesNode( const Aftk::C_SceneNode* pAftkNode );

	void AddLightNode( const Aftk::C_SceneNode* pAftkNode );

	void AddCrowdBaseInstanceNode( const Aftk::C_SceneNode* pAftkNode );

	void AddCrowdSkeletonNode( const Aftk::C_SceneNode* pAftkNode );

	void AddCrowdInstanceNode( const Aftk::C_SceneNode* pAftkNode );

private:
	C_AftkNodeList m_StaticNodes;
	Btl::Vector< C_AftkNodeList > m_RouteNodesList;
	C_AftkNodeList m_NavMeshNodes;
	C_AftkNodeList m_PortalRoomsNodes;
	C_AftkNodeList m_PvsZonesNodes;
	C_AftkNodeList m_LightNodes;
	C_AftkNodeList m_CrowdBaseInstanceNodes;
	C_AftkNodeList m_CrowdSkeletonNodes;
	C_AftkNodeList m_CrowdInstanceNodes;
	ModelData::C_ModelData	m_ModelData;
};


#endif