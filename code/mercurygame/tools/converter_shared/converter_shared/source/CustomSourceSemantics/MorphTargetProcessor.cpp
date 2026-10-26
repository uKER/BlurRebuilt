#include "stdafx.h"

#include <converter_shared/CustomSourceSemantics/MorphTargetProcessor.h>

namespace MeshDeform
{

namespace MorphTarget
{

/* 
===============================================================
					class C_MorphTargetProcessor

NOTES: 
===============================================================
*/

C_MorphTargetProcessor C_MorphTargetProcessor::s_Instance;

/*
===============
NAME: Process

NOTE: Pre-process the Aftk scene and prepare morph targets
===============
*/
void C_MorphTargetProcessor::Process(Aftk::C_Scene* p_pAftkScene, Util::C_String p_Tag)
{	
	m_pAftkScene		= p_pAftkScene;
	m_Tag				= p_Tag.Lower();
	m_pCurrentMeshNode	= NULL;

	if (!m_pAftkScene)
		return;

	//1) Find all mesh deformers in the scene and build a list
	C_GetMorphTargets getMorphTargets(this, m_Tag);
	Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, getMorphTargets);

	LOGINFO( Tools::g_log, "MorphTargets - numMorphTargets(%d)", m_pMorphTargets.Size());

	//2) For each morph target found, find corresponding source mesh
	for (S32 iMesh = 0; iMesh < m_pMorphTargets.Size(); iMesh++)
	{
		//2.1) Add vertex data channel to source mesh for position
		//2.2) Add vertex data channel to source mesh for normal
		//2.3) For each vertex in the source mesh
			//2.3.1) Get corresponding position and normal on target mesh and
					//add to source mesh data channel

		Aftk::C_SceneMeshNode* pMorphTarget = m_pMorphTargets[iMesh];

		m_pCurrentMeshNode = NULL;
		Util::C_String meshName = pMorphTarget->GetName().c_str();
		int iPos = meshName.ISearch(m_Tag);
		char tempName[MAX_PATH];
		char searchName[MAX_PATH];
		if (iPos+1 < meshName.Length())
		{
			_snprintf_s(tempName, lengthof(tempName), _TRUNCATE, meshName.NonConstChars());
			_snprintf_s(searchName, iPos+1, _TRUNCATE, tempName);
		}
		else
			_snprintf_s(searchName, lengthof(searchName), _TRUNCATE, meshName.NonConstChars());
		
		meshName = Util::C_String(searchName).Lower();
		C_FindMeshNode findNode(this, meshName);
		Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, findNode);

		if (!m_pCurrentMeshNode)
			continue;

		int vertexDataChannelStart = m_pCurrentMeshNode->GetNumVertexDataChannels();

		//Set up the vertex data channel
		Aftk::C_VertexDataChannelInfo info;
		
		//Position offset
		info.SetName("positionOffset");
		info.SetDataType(Aftk::E_GenericAttributeType_Vector4F32);
		
		for (U32 i = 0; i < m_pCurrentMeshNode->GetNumVertexDataChannels(); i++)
		{
			if (m_pCurrentMeshNode->GetVertexDataChannelInfo(i).GetName() == info.GetName())
				bool iShouldntGetHere = true;
		}

		m_pCurrentMeshNode->AddVertexDataChannelInfo(info);

		//Normal offset
		info.SetName("normalOffset");
		info.SetDataType(Aftk::E_GenericAttributeType_Vector4F32);
		
		m_pCurrentMeshNode->AddVertexDataChannelInfo(info);
		
		int numVertices = m_pCurrentMeshNode->GetNumPolygonVertices();
		int numDeformedVertices = pMorphTarget->GetNumPolygonVertices();

		if (numVertices != numDeformedVertices)
			continue;
			
		//For each vertex of the mesh node
		for (int v = 0; v < numVertices; v++)
		{
			Aftk::Vector4F32 position;
			Aftk::Vector4F32 normal;
			
			int pvi = pMorphTarget->GetPolygonVertex(v).GetPositionIndex();
			int nvi = pMorphTarget->GetPolygonVertex(v).GetNormalIndex();
						
			Aftk::Position3 morphPos = pMorphTarget->GetVertexPosition(pvi);
			Aftk::Position3 origPos = m_pCurrentMeshNode->GetVertexPosition(pvi);

			position.x = morphPos.x - origPos.x;
			position.y = morphPos.y - origPos.y;
			position.z = morphPos.z - origPos.z;
			position.w = 1.0f;

			Aftk::Normal3 morphNorm = pMorphTarget->GetVertexNormal(nvi);
			Aftk::Normal3 origNorm = m_pCurrentMeshNode->GetVertexNormal(nvi);

			normal.x = morphNorm.x - origNorm.x;
			normal.y = morphNorm.y - origNorm.y;
			normal.z = morphNorm.z - origNorm.z;
			normal.w = 1.0f;

			Aftk::C_PolygonVertex& pv = m_pCurrentMeshNode->GetPolygonVertex(v);

			pv.SetData(vertexDataChannelStart, position);
			pv.SetData(vertexDataChannelStart + 1, normal);
		}
	}

	//3) Delete all morph target nodes from the Aftk scene	
	for (int iMesh = 0; iMesh < m_pMorphTargets.Size(); iMesh++)
	{
		C_DeleteMeshNode deleteMeshNode(m_pMorphTargets[iMesh]->GetName().c_str());
		Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, deleteMeshNode);
	}

	m_pMorphTargets.Clear();
}

/* 
===============================================================
					class C_GetMorphTargets

NOTES: Class to visit Aftk nodes and identify morph targets
===============================================================
*/
C_GetMorphTargets::C_GetMorphTargets(C_MorphTargetProcessor* p_pProcessor, Util::C_String p_Tag)
{
	m_pProcessor	= p_pProcessor;
	m_Tag			= p_Tag.Lower();
}

bool C_GetMorphTargets::IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
{
	if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		return true;

	return false;
}

Aftk::C_Status C_GetMorphTargets::Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
{
	if (p_pNode->GetType() != Aftk::E_ObjType_SceneMesh)
	{
		//Ignore this node.
		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_SceneMeshNode* pMeshNode = (Aftk::C_SceneMeshNode*)p_pNode;
	Util::C_String name = pMeshNode->GetName().c_str();

	if (name.ISearch(m_Tag) != -1)
		m_pProcessor->AddMorphTarget(pMeshNode);

	return Aftk::C_Status::E_Status_Success;
}

} // namespace MorphTarget

} // namespace MeshDeform
