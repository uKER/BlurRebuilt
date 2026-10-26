#ifndef APP_SCENE_DESCRIPTION_H
#define APP_SCENE_DESCRIPTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/SceneDesc.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <converter_shared/ModelData.h>

/*
enum E_SceneDescGroup
{
	E_SceneDescGroup_StaticLevel = 0,
	E_SceneDescGroup_Elements,
	E_SceneDescGroup_NavMesh,
	E_SceneDescGroup_PortalRooms,
	E_SceneDescGroup_PvsZones,
	E_SceneDescGroup_Count
};

void GetSceneDescGroupNames( Btl::Vector< Util::C_String >& sceneDescGroupNames );
*/

typedef Btl::Vector<const Aftk::C_SceneNode*> C_AftkNodeList;

//! C_DataSceneDescription

class C_DataSceneDescription : public Tools::ProcessGraph::C_Data
{
public:

	// inherited from C_Data
	const Util::C_String GetName() const;

	ModelData::C_ModelData& GetModelData();

	const ModelData::C_ModelData& GetModelData() const;

	C_AftkNodeList& GetStaticNodes();

	const C_AftkNodeList& GetStaticNodes() const;

	const C_AftkNodeList& GetLightNodes() const;

	void AddStaticMeshNode( const Aftk::C_SceneNode* pAftkNode );

	bool ExportModelData( Sys::I_File* pFile ) const;

	void AddNavMeshNode( const Aftk::C_SceneNode* pAftkNode );
	
	void AddPortalRoomsNode( const Aftk::C_SceneNode* pAftkNode );

	void AddPvsZonesNode( const Aftk::C_SceneNode* pAftkNode );

	void AddLightNode( const Aftk::C_SceneNode* pAftkNode );

private:
	C_AftkNodeList m_StaticNodes;
	C_AftkNodeList m_NavMeshNodes;
	C_AftkNodeList m_PortalRoomsNodes;
	C_AftkNodeList m_PvsZonesNodes;
	C_AftkNodeList m_LightNodes;
	ModelData::C_ModelData	m_ModelData;
};


#endif