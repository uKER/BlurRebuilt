#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_MESH_H
#define AFTK_SPEEDTREE_MESH_H

#include <SpeedTreeRT.h>
#include <AftkSpeedTree\AftkSpeedTreeUtility.h>
#include <AftkSpeedTree\AftkSpeedTreeSetup.h>

namespace AftkSpeedTree
{

// TreeData forward declarations
class TreeData;
struct TreeMeshInfo;

typedef Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneMeshNode*> >	AftkSceneMeshNodes;
typedef	Ut::UtVector<Ut::DynamicVector<U32> >						Seeds;

// export speedtree mesh class
class MeshGenerator
{
public:
	MeshGenerator(TreeData& treeData) : m_pTreeData(&treeData) {}
	~MeshGenerator() { }

	
	void Process(Aftk::C_Scene& aftk);

private:
	
	TreeData*						m_pTreeData;
	CSpeedTreeRT*					m_pSpeedTree;
	CSpeedTreeRT::SGeometry*		m_pGeometryCache;
	Aftk::C_SceneNode*				m_pSceneNode;
	
	Util::C_String					m_sFilename;

	// all the meshes and seeds generated for this tree
	AftkSceneMeshNodes				m_AftkSceneMeshNodes;
	Seeds							m_Seeds;

	// the current mesh and seed
	Aftk::C_SceneMeshNode*			m_pAftkSceneMeshNode;
	
	F32								m_fSize;
	F32								m_fSizeVariance;
	Aftk::C_Bounds					m_bounds;


	U32								m_nBranchVertexCount;
	U32								m_nFrondVertexCount;
	U32								m_nLeafVertexCount;

	S16								m_nBranchMaterialIndex;
	S16								m_nFrondMaterialIndex;
	S16								m_nLeafMaterialIndex;
	S16								m_nLeafMeshMaterialIndex;

	S16								m_nBranchPolyflagsIndex;
	S16								m_nFrondPolyflagsIndex;
	S16								m_nLeafPolyflagsIndex;

	bool ReadSeedsXML(const Util::C_String& filename);
	void ParseSeedsXML(DOMNode *pNode);

	void ProcessNode(Aftk::C_SceneNode& node);
	void SetupMesh(U32 nNum = 0);
	void SetupMeshFromTreeMeshInfo(const TreeMeshInfo* pTreeMeshInfo);
	bool SetupMaterials();
	void GenerateBranches(U32& nVertexIndexOffset);
	void GenerateFronds(U32& nVertexIndexOffset);
	void GenerateLeaves(U32& nVertexIndexOffset);
};

}

#endif


