#include "stdafx.h"
#include <AftkSpeedTree\AftkSpeedTreeData.h>

using namespace AftkSpeedTree;

// finds the treeinfo from a specified meshnode
S32 TreeData::FindTreeMeshInfo(Aftk::C_SceneMeshNode* pNode)
{
	for(S32 a = 0; a < m_treeMesh.Size(); a++)
	{
		for (S32 b = 0; b < m_treeMesh[a].meshNodes.Size(); b++)
		{
			if (pNode == m_treeMesh[a].meshNodes[b])
				return a;
		}
	}

	return -1;
}

// finds the treeinfo from definition, seed and size values
S32 TreeData::FindTreeMeshInfo(const Util::C_String& sFilename, U32 nSeed, F32 fSize)
{
	for(S32 a = 0; a < m_treeMesh.Size(); a++)
	{
		if(	sFilename == m_treeMesh[a].sFilename &&
			nSeed == m_treeMesh[a].nSeed &&
			fSize == m_treeMesh[a].fSize)
			return a;
	}

	return -1;
}

void TreeData::UpdateExtents(Aftk::Matrix44 matWorld)
{
	Aftk::Position3 vPosition = Aftk::Position3(matWorld[3][0], matWorld[3][1], matWorld[3][2]);

	m_treeExtents.AddVertex(vPosition);
}


void TreeData::AddTreeMeshInfo(
	const Util::C_String& sFilename,
	U32 nSeed, 
	F32 fSize,
	Aftk::C_Bounds boundingBox, 
	AftkSceneMeshNodes& meshNodes)
{
	// add this mesh to the tree info, so duplicates can be found later
	TreeMeshInfo treeInfo;

	treeInfo.sFilename	= sFilename;
	treeInfo.nSeed		= nSeed;
	treeInfo.fSize		= fSize;

	for(S32 a = 0; a < meshNodes.Size(); a++)
	{
        treeInfo.meshNodes.PushBack();
		treeInfo.meshNodes.Back() = meshNodes[a];
		m_treeMeshSet.Insert(meshNodes[a]);
	}

	//treeInfo.nNumSeeds = nNumSeeds;
	treeInfo.boundingBox = boundingBox;
	m_treeMesh.PushBack(treeInfo);
}

// return index to the mesh which this instances
S32 TreeData::AddTreeInstanceInfo(
	Aftk::Matrix44 matWorld, 
	Aftk::C_SceneInstanceNode* pInstance,
	Aftk::C_SceneMeshNode* pNode,
	Aftk::C_SceneLodGroupNode* pLodGroup)
{
	assert(pNode->GetType() == Aftk::E_ObjType_SceneMesh);

	// also add an instance location with the transform from the SPTData node
	TreeInstanceInfo	treeInstance;
	
	treeInstance.matWorld = matWorld;
	treeInstance.nIndex = FindTreeMeshInfo(pNode);
	treeInstance.pLodGroup = pLodGroup;
	
	m_treeInstances.PushBack(treeInstance);
	UpdateExtents(treeInstance.matWorld);

	if (pInstance)
	{
		m_treeInstanceSet.Insert(pInstance);
		m_toDeleteArray.PushBack(pInstance);
	}

	return treeInstance.nIndex;
}

void TreeData::RemoveFromToDeleteArray( Aftk::C_SceneNode* pToDelete )
{
	for (U32 i = 0; i < (U32)m_toDeleteArray.Size(); i++)
	{
		if (m_toDeleteArray[i] == pToDelete)
		{
			m_toDeleteArray.EraseNonOrdered(i);
			return;
		}
	}

	assert(!"Could not remove from toDeleteArray!");
}