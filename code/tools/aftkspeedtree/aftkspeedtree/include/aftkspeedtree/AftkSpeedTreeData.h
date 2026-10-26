#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_DATA_H
#define AFTK_SPEEDTREE_DATA_H

#include <AftkSpeedTree\AftkSpeedTreeUtility.h>
#include <AftkSpeedTree\AftkSpeedTreeSetup.h>

namespace AftkSpeedTree
{
extern Debug::Logger &g_Log;

const int CGFX_BASE = 0x486a53f1;
const int CGFX_SPEEDTREE_BRANCH = CGFX_BASE + 10;
const int CGFX_SPEEDTREE_FROND = CGFX_BASE + 11;
const int CGFX_SPEEDTREE_LEAF = CGFX_BASE + 12;
const int CGFX_SPEEDTREE_BILLBOARD = CGFX_BASE + 13;
const int CGFX_SPEEDTREE_BRANCH_ALPHA = CGFX_BASE + 14;
const int CGFX_GOTHAM_FLARE = CGFX_BASE + 15;

typedef Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneMeshNode*> >	AftkSceneMeshNodes;
	
// definition pathname, vertex data seed, vertex data size, and resulting aftk mesh node
struct TreeMeshInfo
{
	Util::C_String				sFilename;
	U32							nSeed;
	F32							fSize;
	Aftk::C_Bounds				boundingBox;
	AftkSceneMeshNodes			meshNodes;
};

// world matrix and tree info
struct TreeInstanceInfo
{
	Aftk::Matrix44				matWorld;
	S32							nIndex;		// index to TreeMeshInfo
	Aftk::C_SceneLodGroupNode* 	pLodGroup;
};

typedef Ut::UtVector<Ut::DynamicVector<TreeInstanceInfo> >		TreeInstanceInfoData;
typedef Ut::UtVector<Ut::DynamicVector<TreeMeshInfo> >			TreeMeshInfoData;
typedef Ut::UtMap<S32, Aftk::C_SceneMeshNode*>					TreeBillboardMeshMap;
typedef Ut::UtMap<Util::C_String, U16>							TreeBillboardTextureMap;

typedef Ut::UtSet<Aftk::C_SceneMeshNode*>						TreeMeshSet;
typedef Ut::UtSet<Aftk::C_SceneInstanceNode*>					TreeInstanceSet;
typedef Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> >	ToDeleteArray;

class TreeData
{
public:
	TreeData()
	{
		m_nDuplicates = 0;
	}
	~TreeData()
	{
		LOGDEBUG( g_Log, "Process: Meshes: %d(%d) Instances: %d\n", m_treeMesh.Size(), m_nDuplicates, m_treeInstances.Size());
		m_treeInstances.Clear();
		m_treeMesh.Clear();
	}
	void Setup(const SpeedTreeSetup& setup) { m_Setup = setup; }

	const SpeedTreeSetup& GetSetup() { return m_Setup; }

	const TreeMeshSet& GetTreeMeshSet() { return m_treeMeshSet; }

	const TreeInstanceSet& GetTreeInstanceSet() { return m_treeInstanceSet; }

	const ToDeleteArray& GetToDeleteArray() { return m_toDeleteArray; }

	
	
	// make 
	friend class MeshGenerator;
	friend class InstanceProcessor;
	friend class LodProcessor;

private:
	// add meshinfo/instanceinfo
	void AddTreeMeshInfo(
		const Util::C_String& sFilename, 
		U32 nSeed, 
		F32 fSize,
		Aftk::C_Bounds boundingBox,
		AftkSceneMeshNodes& meshNodes);

	S32 AddTreeInstanceInfo(
		Aftk::Matrix44 matWorld, 
		Aftk::C_SceneInstanceNode* pInstance,
		Aftk::C_SceneMeshNode* pNode,
		Aftk::C_SceneLodGroupNode* pLodGroup);

	void RemoveFromToDeleteArray( Aftk::C_SceneNode* pToDelete);

	// find tree info data from either mesh node or the definition/creation data
	S32	FindTreeMeshInfo(Aftk::C_SceneMeshNode* pNode);
	S32	FindTreeMeshInfo(const Util::C_String& sFilename, U32 nSeed, F32 fSize);

	// update the total extents of where trees exist in the world
	void UpdateExtents(Aftk::Matrix44 matWorld);

    // extents of trees in the world
	Aftk::C_Bounds				m_treeExtents;

	// static stores of instance data and individual tree info, so we can batch them up
	TreeInstanceInfoData		m_treeInstances;
	TreeMeshInfoData			m_treeMesh;

	// keep track of how many trees were duplicated in the scene;
	U32							m_nDuplicates;
	SpeedTreeSetup				m_Setup;

	// These are used by our lod generation code.
	TreeMeshSet					m_treeMeshSet;
	TreeInstanceSet				m_treeInstanceSet;

	ToDeleteArray				m_toDeleteArray;
};

}

#endif
