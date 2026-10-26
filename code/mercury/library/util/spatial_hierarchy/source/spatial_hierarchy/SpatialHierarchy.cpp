#include "StdAfx.h"

#include "spatial_hierarchy/Tree-Common.h"
#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/Kd-TreeBuilder.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/AlphaTextureManager.h"
#include "spatial_hierarchy/SpatialHierarchy.h"
#include "spatial_hierarchy/kd-TreeBuilder.h"
#include "spatial_hierarchy/kd-Tree-batch.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/LpvsUtility.h"
#include "spatial_hierarchy/PolyFlags.h"
#include <system/memory/memorytracker.h>

//#define DONT_USE_SIMD

using namespace SD;

namespace SD
{
Debug::Logger &g_Log = Debug::GetLogger("mercury.spatialhierarchy");
}

Maths::HVec3 AftkPos3ToHVec3(const Aftk::Position3 &v)
{
	return Maths::HVec3(v.x, v.y, v.z);
}

SpatialHierarchy::SpatialHierarchy()
{
	m_bAllocatedBySerialization = false;
	m_pAlphaTextureList = NULL;
	m_pAlphaTextureMgr = NULL;
	m_pCallbackFunction = NULL;
	m_nRaysCast = 0;
	m_piAlphaTex = NULL;
	m_bIgnoreInstances = false;
	m_pAlphaTextureMgr = NULL;
	m_iNoCollisionPolyflag = 0;
	m_pTriCallbackFunction = NULL;

	m_pCallbackFunction = NULL;
	m_pBaseInstanceCallbackFunction = NULL;

	m_iCurLodLevel = 0;

	SetEpsilon( 0.0001f );
}

SpatialHierarchy::~SpatialHierarchy()
{
	if (m_pAlphaTextureList)
		delete m_pAlphaTextureList;
}

// returns the LOD level of the given node. returns -1 if the node is not a LOD
int GetModelLodLevel(Aftk::C_SceneNode *pNode)
{
	// check for LODs
	Aftk::C_SceneNode *pParent = pNode->GetParentPtr();
	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		Aftk::C_SceneLodGroupNode *pLG = (Aftk::C_SceneLodGroupNode*)pParent;

		// if it's part of a lod group, but it's not the highest lod, ignore it
		for (int i=0; i<(int)pLG->GetNumLodLevels(); i++)
		{
			Aftk::C_GroupLodLevelInfo &info = pLG->GetLodLevelInfo(i);
			if (info.GetSceneNodePtr() == pNode)
				return i;
		}
	}
	return -1;
}

// returns true if the node is highest lod or if it has no lods
bool CheckModelIsHighestLod(Aftk::C_SceneNode *pNode)
{
	// check for LODs
	Aftk::C_SceneNode *pParent = pNode->GetParentPtr();
	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		Aftk::C_SceneLodGroupNode *pLG = (Aftk::C_SceneLodGroupNode*)pParent;

		// if it's part of a lod group, but it's not the highest lod, ignore it
		Aftk::C_GroupLodLevelInfo &info = pLG->GetLodLevelInfo(0);
		if (info.GetSceneNodePtr() != pNode)
			return false;

		//info.SetThreshold(10000);
	}
	return true;
}

// returns true if the node is lowest lod or if it has no lods
bool CheckModelIsLowestLod(Aftk::C_SceneNode *pNode)
{
	// check for LODs
	Aftk::C_SceneNode *pParent = pNode->GetParentPtr();
	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		Aftk::C_SceneLodGroupNode *pLG = (Aftk::C_SceneLodGroupNode*)pParent;

		// if it's part of a lod group, but it's not the highest lod, ignore it
		Aftk::C_GroupLodLevelInfo &info = pLG->GetLodLevelInfo(pLG->GetNumLodLevels()-1);
		if (info.GetSceneNodePtr() != pNode)
			return false;
	}
	return true;
}

// look through a material's shader nodes and see if they match any of the alpha textures
int SpatialHierarchy::CheckIfPolyAssociatedWithMaterialNode(const Aftk::C_MaterialNode *pMat, KdTree *pSH)
{
	// check each image node
	for (U32 i=0; i<pMat->GetNumShaderImageNodes(); i++)
	{
		std::string fileName;
		pMat->GetShaderImageNodePtr(i)->GetFilePlug().GetValue(fileName);
		int iATex = 
			pSH->m_pSpatialHierarchy->GetAlphaTextureIndexFromName(fileName);
		if (iATex != -1)
			return iATex;
	}

	return -1;
}

// collect mesh data from AFTK scene
int SpatialHierarchy::CollectPolygonsFromScene(Aftk::C_SceneNode *pAftkSceneNode, TriPrimVector &primitives, Box3 *psceneBBox, KdTree *pSH, bool bTransform, bool bCount)
{
	// make a list of all the polygons in the scene
	int iSceneNode = 0;
	int iSceneNodeX = 0;
	Maths::C_AxisBox bbox(psceneBBox);

	Aftk::C_Scene *pAftkScene = pAftkSceneNode->GetScenePtr();

	Aftk::Matrix44 worldMatrix = pAftkSceneNode->GetWorldMatrix();

	// convert instances into reference nodes
	if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
		return 0;

	// only interested in mesh nodes
	if (!pAftkSceneNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return 0;

	// skip things in layers called 'sky'
	if (!DoesNodeNeedProcessing(pAftkSceneNode))
		return 0;

	// check all the materials in the scene, to see if they have alpha-textures on them
	if (!m_piAlphaTex)
	{
		int nMat = pAftkScene->GetNumMaterialNodes();
		m_piAlphaTex = new int[nMat];
		for (int i=0; i<nMat; i++)
			m_piAlphaTex[i] = CheckIfPolyAssociatedWithMaterialNode(&pAftkScene->GetMaterialResourcePtr(i)->GetContent(), pSH);
	}

	Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pAftkSceneNode;

	pMesh->BuildAnimationHelpers();

	int iLayer = pMesh->GetScenePtr()->GetLayerIndex( pMesh->GetLayerPtr(0) );

	int nPrimitives = 0;

	bool bMeshHasUVs = (pMesh->GetNumTextureCoordinateSets() >= 1);

	// collect polygons
	int nFaces = pMesh->GetNumPolygons(0);
	for (int iFace=0; iFace<nFaces; iFace++)
	{
		const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(iFace);

		assert(pPoly->GetNumPolygonVertices() == 3);	// assert here if the aftk was not triangulated

		if (m_pTriCallbackFunction != NULL)
		{
			if((*m_pTriCallbackFunction)(pAftkSceneNode, m_pTriContext, iFace) == 0)
				continue;
		}
		else
		{
			// check polyflags - if it's flagged as 'don't cast shadows' then we skip it
			if (GetPolyFlags(
				pPoly, 
				0, //double-sided mask
				m_iNoCollisionPolyflag //no collide mask
				).noCollision)
				continue;
		}
		
		nPrimitives++;

		if (bCount)
			continue;

		// check whether the material contains an alpha texture
		int iAlphaTex = -1;
		const Aftk::C_MaterialResource *pMatRes = GetMaterialResourcePtr(pPoly);
		if (pMatRes)
		{
			// look up this material in the table we calculated before
			iAlphaTex = m_piAlphaTex[pMatRes->GetIndex()];
		}				

		// create new polygon
		int iNewPrim = primitives.Size();
		primitives.PushBack();
		TrianglePrim *pNewPrim = &primitives[iNewPrim];

		pNewPrim->m_iSceneNode = iSceneNodeX;
		pNewPrim->m_iPolygon   = iFace;
		for (int i=0; i<3; i++)
		{
			int iPosVertex = pPoly->GetPolygonVertexIndex(i);

			if (bTransform)
				pNewPrim->m_v[i] = AftkPos3ToHVec3(ApplyMatrix(worldMatrix, pMesh->GetEvaluatedPolygonVertexPosition( iPosVertex ) ));
			else
				pNewPrim->m_v[i] = AftkPos3ToHVec3(pMesh->GetEvaluatedPolygonVertexPosition( iPosVertex ));

			pNewPrim->m_pMeshNode = pMesh;
			if (bMeshHasUVs)
			{
				int iUVIndex = pPoly->GetPolygonVertex(i).GetTextureCoordinateIndex(0);
				if ( iUVIndex != -1)
				{
					const Aftk::TextureCoordinate4 *pUV = &pMesh->GetTextureCoordinate( 0, iUVIndex );
					pNewPrim->m_uv[i].x = pUV->x;
					pNewPrim->m_uv[i].y = pUV->y;
				}
			}

			bbox.UpdateExtents( Maths::HVec3(&pNewPrim->m_v[i]) );
			pNewPrim->m_iAlphaTex = iAlphaTex;
		}
	}

	*psceneBBox = bbox;

	return nPrimitives;
}

// get the index for a spatial hierarchy instance from an aftk unique ID
int SpatialHierarchy::GetInstanceFromAftkID(int iNodeInstance, int iHint)
{
	if (m_Objects.Size() == 0)
		return -1;

	// find this object in our list of object primitives
	if (iHint < 0 || iHint >= m_Objects.Size())
		iHint = 0;
	int i = iHint;
	while (1)
	{
		if (m_Objects[i].m_iAftkSceneNode == iNodeInstance)
		{
			return i;
		}

		i++;
		if (i >= m_Objects.Size())
			i = 0;
		if (i == iHint)
			return -1;
	}
}

// turn a particular instance on or off
void SpatialHierarchy::EnableInstance(int iID, bool bOn)
{
	if (iID>=0)
		m_Objects[iID].m_bEnabled = bOn;
}

// turn a specific layer on or off by name
bool SpatialHierarchy::SetLayer(const std::string &layerName, bool bOn)
{
	// find the layer index number for this layer name
	for (U32 i=0; i<m_pAftkScene->GetNumLayers(); i++)
	{
		if (strcmpi(layerName.c_str(), m_pAftkScene->GetLayerPtr(i)->GetName().c_str()) == 0)
		{
			if (!SetLayer(i, bOn))
			{
				// this could happen when trying to switch a nonexistent layer.
				//printf("INTERNAL ERROR: Trying to switch layer (%s) (%d) to (%s) which was not defined as 'variable'.\n",layerName.c_str(), i, bOn?"on":"off");
				return false;
			}
			return true;
		}
	}
	return false;
}

// set raytracing for a specific lod level
// set -1 or 0 to enable the highest lod. set 1 or 2 to disable the high-lod and raytrace against specified lower lods
void SpatialHierarchy::SetLodLevel(int iLodLevel)
{
	m_iCurLodLevel = iLodLevel;

	UpdateActiveKdTrees();
}

// turn a specific layer on or off by index
bool SpatialHierarchy::SetLayer(int i, bool bOn)
{
	// might already be set correctly
	if ((m_pbLayer[i]==0) == (bOn==0))
		return true;

	// update the layer switch array
	m_pbLayer[i] = bOn;

	UpdateActiveKdTrees();

	return true;
}

/// update the active layer list using m_iCurLodLevel and m_pbLayer
void SpatialHierarchy::UpdateActiveKdTrees()
{
	m_iActiveMainKdTrees.Clear();

	for (int i=0; i<m_MainKdTrees.Size(); i++)
	{
		KdTree *pTree = &m_MainKdTrees[i];

		// only add the tree if it's layer is on
		if (pTree->m_iTreeLayer != -1 && !m_pbLayer[pTree->m_iTreeLayer])
			continue;

		// only add the tree if it's lod level is -1 or equal to the current lod level
		if (pTree->m_iLodLevel != -1 &&
			pTree->m_iLodLevel != m_iCurLodLevel)
			continue;

		m_iActiveMainKdTrees.PushBack(i);
	}
}

// set layers on or off using a bitfield
void SpatialHierarchy::SetLayerBitArray(const Ut::BitArray128 &layerData)
{
	for (int i=0; i<m_nLayers; i++)
	{
		m_pbLayer[i] = layerData[i];
		SetLayer(i, layerData[i] != 0);
	}

	UpdateActiveKdTrees();
}

// find the kd-tree that matches a particular layer index.
int SpatialHierarchy::GetKdTreePtrForLayerIndex(S32 iLayer, int iLodLevel)
{
	for (int i=0; i<m_MainKdTrees.Size(); i++)
	{
		if (m_MainKdTrees[i].m_iTreeLayer == iLayer &&
			m_MainKdTrees[i].m_iLodLevel == iLodLevel)
			return i;
	}

	return -1;
}

// determine whether this node should be added as normal mesh, or as a base instance
bool SpatialHierarchy::AddToHierarchyAsInstance(Aftk::C_SceneNode *pAftkSceneNode)
{
    if (!pAftkSceneNode->GetNumInstancesOfThisNode())
		return false; // this node is not being instanced

	// find all instances of this node - if they are all normal instances then it's ok to instance it in the hierarchy
	int nInstances = 0;
	for (Aftk::C_SceneNodeIterator it(m_pAftkScene->GetRootSceneNodePtr()); it; it++)
	{
		Aftk::C_SceneNode *pNode = it;

		// we found a remappable instance
		if (pNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
		{
			Aftk::C_SceneMeshRemappedInstanceNode *pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)pNode;

			if (pInstance->GetSourceNodePtr() == pAftkSceneNode)
				return false; // it's instancing our node - return false
		}
	}

	// must only be instanced by normal instances, so it's ok to add to our hierarchy as an instance
	return true;
}

// build spatial hierarchy from an aftk scene
void SpatialHierarchy::Build(Aftk::C_Scene *pAftkScene, LayerMode *pLayerModes)
{
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server");

	m_pAftkScene = pAftkScene;
	int iCurGlobalPrim = 0;

	m_bbox = Maths::C_AxisBox();

	m_nLayers = pAftkScene->GetNumLayers();
	if (m_nLayers)
	{
		m_pbLayer.Resize(m_nLayers);
		for (int i=0; i<m_nLayers; i++)
		{
			if (pLayerModes)
				m_pbLayer[i] = (pLayerModes[i] == AlwaysOff) ? false : true;
			else
				m_pbLayer[i] = true;
		}
	}

	for (int iPass=1; iPass<=2; iPass++)
	{
		if (iPass == 2)
		{
			for (int x=0; x<m_MainKdTrees.Size(); x++)
			{
				MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.primitives");
				m_MainKdTrees[x].m_Primitives.Reserve( m_MainKdTrees[x].m_nTris );
				MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.primitives");
				m_MainKdTrees[x].m_nTris = 0;
			}

		}

		// iterate through all the scene's nodes
		for (Aftk::C_SceneNodeIterator it(pAftkScene->GetRootSceneNodePtr()); it; it++)
		{
			Aftk::C_SceneNode* pAftkSceneNode = it;

			if (!pAftkSceneNode->HasType(Aftk::E_ObjType_ISceneMesh))
				continue;

			// check that it's the highest LOD
//			if (!CheckModelIsHighestLod(pAftkSceneNode))
//				continue;

//			if (!CheckModelIsLowestLod(pAftkSceneNode))
//				continue;
			int iLodLevel = GetModelLodLevel(pAftkSceneNode);

			if (!AddToHierarchyAsInstance(pAftkSceneNode))
			{
				if (!DoesBaseInstanceNeedProcessing(pAftkSceneNode))
					continue;

				// non instanced mesh - add to the main kd-tree
				S32 iLayer = GetLayerIndexFromSceneNodePtr(pAftkSceneNode, 0);
				if (pLayerModes && iLayer!=-1)
				{
					switch ( pLayerModes[iLayer] )
					{
					case AlwaysOn:	iLayer = -1; break;	// layers that are always on get merged together
					case AlwaysOff:	continue;			// layers that are always off get skipped
					case Variable:	break;				// layers that are switchable have their own kd-tree
					default:
						assert(0);
						// INTERNAL ERROR
						break;
					}
				}
				int iTree = GetKdTreePtrForLayerIndex( iLayer, iLodLevel );
				if (iTree == -1)
				{
					// create a new tree for this layer/lod level
					iTree = m_MainKdTrees.Size();
					m_MainKdTrees.PushBack();
				}
				KdTree *pTree = &m_MainKdTrees[iTree];
				pTree->m_pSpatialHierarchy = this;
				pTree->m_iTreeLayer = iLayer;
				pTree->m_iLayer = GetLayerIndexFromSceneNodePtr(pAftkSceneNode, 0);
				pTree->m_iLodLevel = iLodLevel;

				if (iPass == 1)
				{
					pTree->m_nTris += CollectPolygonsFromScene(pAftkSceneNode, pTree->m_Primitives, &pTree->m_bbox, pTree, true, true);
				}
				else
				{
					CollectPolygonsFromScene(pAftkSceneNode, pTree->m_Primitives, &pTree->m_bbox, pTree, true, false );
				}
			}
			else
			{
				if (iPass == 1)
					continue;

				// it's an instanced mesh - create a kd-tree for the reference mesh
				MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.baseinstance");
				int i = m_KdTrees.Size();
				m_KdTrees.PushBack();
				KdTree *pTree = &m_KdTrees.Back();
				pTree->m_pSpatialHierarchy = this;
				pTree->m_iLayer = GetLayerIndexFromSceneNodePtr(pAftkSceneNode, 0);
				pTree->m_iLodLevel = iLodLevel;

				CollectPolygonsFromScene(pAftkSceneNode, pTree->m_Primitives, &pTree->m_bbox, pTree, false, false);

				int n=0;

				pTree->m_iFirstGlobalPrimIdx = iCurGlobalPrim;
				pTree->Build(pAftkSceneNode);
				iCurGlobalPrim += pTree->m_nTris;
				if (pTree->m_nTris == 0)
					m_KdTrees.PopBack();
				else
				{
					// create a matching object for the reference mesh
					if (DoesBaseInstanceNeedProcessing(pAftkSceneNode))
					{
						m_Objects.PushBack();
						if (!m_Objects.Back().Initialize(i, pAftkSceneNode, pAftkSceneNode))
							m_Objects.PopBack();
					}

					MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.instance");
					{
						// look through the scene, find the instances, create an object for each instance
						for (Aftk::C_SceneNodeIterator it2(pAftkScene->GetRootSceneNodePtr()); it2; it2++)
						{
							Aftk::C_SceneNode* pAftkSceneNode2 = it2;

							if (pAftkSceneNode2->GetType() == Aftk::E_ObjType_SceneInstance)
							{
								Aftk::C_SceneInstanceNode *pInstance = (Aftk::C_SceneInstanceNode*)pAftkSceneNode2;

								if (pInstance->GetSourceNodePtr() == pAftkSceneNode)
								{
									m_Objects.PushBack();
									if (!m_Objects.Back().Initialize(i, pInstance, pAftkSceneNode))
										m_Objects.PopBack();
									else
										n++;
								}
							}
						}
					}
					MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.instance");
				}
				MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.baseinstance");
			}
		}
	}

	// print out the tree sizes
	for (int i=0; i<m_MainKdTrees.Size(); i++)
	{
		LOGDEBUG( g_Log, "Tree layer %d lod %d - triangles %d\n",
			m_MainKdTrees[i].m_iTreeLayer, m_MainKdTrees[i].m_iLodLevel, m_MainKdTrees[i].m_Primitives.Size() );
	}

	int nPolys = 0;
	for (int i = 0; i < m_KdTrees.Size(); ++i)
		nPolys += m_KdTrees[i].m_Primitives.Size();

	LOGDEBUG( g_Log ,"And %d triangles in %d base instances\n", nPolys, m_KdTrees.Size() );

	// build the merged per-layer kd-trees
	Maths::C_AxisBox bbox = &m_bbox;
	for (int i=0; i<m_MainKdTrees.Size(); i++)
	{
		m_MainKdTrees[i].m_iFirstGlobalPrimIdx = iCurGlobalPrim;
		m_MainKdTrees[i].Build(NULL);
		iCurGlobalPrim += m_MainKdTrees[i].m_nTris;
		bbox.UpdateExtents( Maths::C_AxisBox(&m_MainKdTrees[i].m_bbox) );
	}

	if (m_Objects.Size())
		m_pTris = &m_Objects[0];

	// Compute bounds for instance kd-tree construction
	Box3Vector primBounds;
	primBounds.Reserve(m_Objects.Size());
	for (int i = 0; i < m_Objects.Size(); ++i)
	{
		primBounds.PushBack( m_Objects[i].m_worldbbox );
		bbox.UpdateExtents( Maths::C_AxisBox(&m_Objects[i].m_worldbbox) );
	}
	m_bbox = bbox;

	m_iGlobalPrimOffset = iCurGlobalPrim;
	KdBuilder *pBuilder = new KdBuilder( primBounds,
		80,		// intersect cost
		1,		// traversal cost
		10.5f,	// empty bonus
		0,		// min primitives per leafnode
		-1		// max depth
		);

	// copy Nodes vector to a cache-aligned memory block
	m_nNodes = pBuilder->m_Nodes.Size()+1;
	m_pNodes = (KdNode*)BizAlignedMalloc(sizeof(KdNode) * (m_nNodes), 128);
	memcpy(m_pNodes+1, &pBuilder->m_Nodes[0], sizeof(KdNode) * pBuilder->m_Nodes.Size());

	// take ownership of mailboxes
	m_pMailboxes = pBuilder->m_pMailboxes;
	pBuilder->m_pMailboxes = NULL;
	// resize the array to exactly fit the number of elements
	m_pMailboxes->Reserve( m_pMailboxes->Size() );

	delete pBuilder;

	if (m_piAlphaTex)
		delete [] m_piAlphaTex;

	SetEpsilon( m_xfEpsilon );

	// retrieve the alpha texture list
	if (m_pAlphaTextureMgr)
		m_pAlphaTextureList = m_pAlphaTextureMgr->TakeOwnershipOfList();

	UpdateActiveKdTrees();

	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server");
}

void SpatialHierarchy::SetAlphaTextureManager(I_AlphaTextureManager *pList)
{
    m_pAlphaTextureMgr = pList;
}

// look through all alpha texture names and see if this one is in the list
int SpatialHierarchy::GetAlphaTextureIndexFromName(const std::string &s)
{
	if (!m_pAlphaTextureMgr)
		return -1;
	return m_pAlphaTextureMgr->FindAlphaTextureInfo(s);
}

// trace a single ray
void SpatialHierarchy::RayTrace(const Ray& ray, RayTraceResult& result) const
{
	result.bHit			= false;
	result.fDistance	= ray.length;
	result.pTri			= NULL;
	result.iGlobalPrim = -1;
	result.iGlobalSubPrim = -1;

	bool bHit = false;

	int i=0;
	while (i<m_iActiveMainKdTrees.Size())
	{
		m_MainKdTrees[m_iActiveMainKdTrees[i]].RayTrace(ray, result);
		bHit = bHit | result.bHit;
		if (result.bHit && ray.bShadowRay)
			break;
		i++;
	}

	if (!m_bIgnoreInstances)
	{
		if (Trace(ray, result))
		{
			bHit = true;
			result.iGlobalPrim = -1;
		}
	}

	result.bHit = bHit;

	if (!ray.bShadowRay)
	{
		result.vHitPos = ray.origin + ray.direction * result.fDistance;
	}

	m_nRaysCast++;
}

// trace a batch of rays
int SpatialHierarchy::RayTrace(SD::RayBatch &batch) const
{
	int nRaysLeft = batch.m_nRays;
	int iRay = 0;
#ifndef DONT_USE_SIMD
	SIMDRay ray;
	SIMDResult result;
#endif
	Ray *pRay = batch.m_Ray;
	int nHit = 0;

	// check that all the sign bits match
//	assert(DirectionSignsMatch(batch));

#ifndef DONT_USE_SIMD
	// split into batches of 4
	while (nRaysLeft >= 4)
	{
		Ray *pRay = batch.m_Ray+iRay;

		// swizzle to SIMD format
		ray.bShadowRay = pRay->bShadowRay;
		for (int i=0; i<4; i++, pRay++)
		{
			for (int j=0; j<3; j++)
			{
				ray.fOrigin[j][i] = pRay->origin.v[j];
				ray.fDirection[j][i] = pRay->direction.v[j];
			}

			ray.fLength[i] = pRay->length;
		}

		// trace 4 rays in one go
		result.hit			= fMinusOne;
		result.fDistance	= _mm_load_ps(ray.fLength);

		// trace through all trees
		int iTree=0;
		while (iTree<m_iActiveMainKdTrees.Size())
		{
			m_MainKdTrees[m_iActiveMainKdTrees[iTree]].TraceSIMD(ray, result);
			if ((result.bHit[0] | result.bHit[1] | result.bHit[2] | result.bHit[3]) == 0)
				break;
			iTree++;
		}
		m_nRaysCast+=4;

		// read SIMDResult and store into m_Result[x]
		for (int i=0; i<4; i++)
		{
			batch.m_Result[iRay+i].bHit = (result.bHit[i]==0);
			if (batch.m_Result[iRay+i].bHit)
			{
				nHit++;
			}
			else
			{
				batch.m_Result[i].bHit			= false;
				batch.m_Result[i].fDistance		= batch.m_Ray[i].length;

				if (!m_bIgnoreInstances)
					Trace(batch.m_Ray[i], batch.m_Result[i]);

			}

			result.bHit[i] = 0;
		}

		iRay += 4;
		nRaysLeft -= 4;
	}
#endif

	// process any single leftover rays with the normal raytracer
	pRay = batch.m_Ray+iRay;
	for (; iRay<batch.m_nRays; iRay++, pRay++)
	{
		RayTrace(*pRay, batch.m_Result[iRay]);
		if (batch.m_Result[iRay].bHit)
		{
			nHit++;
		}
		else
		{
			RayTrace(batch.m_Ray[iRay], batch.m_Result[iRay]);
		}
	}

	return nHit;
}

int SpatialHierarchy::GetNumRaysCast() const
{
	return m_nRaysCast;
}

void SpatialHierarchy::SetIgnoreInstances(bool bIgnore) const
{
	m_bIgnoreInstances = bIgnore;
}

void SpatialHierarchy::SetEpsilon(float fEpsilon) const
{
	KdTreeTemplate<ObjectPrim>::SetEpsilon( fEpsilon );

	for (int i=0; i<m_MainKdTrees.Size(); i++)
		m_MainKdTrees[i].SetEpsilon( fEpsilon );

	m_fEpsilon = fEpsilon;
}

float SpatialHierarchy::GetEpsilon() const
{
	return m_fEpsilon;
}

bool SpatialHierarchy::DoesNodeNeedProcessing(Aftk::C_SceneNode *pNode)
{
	if (!m_pCallbackFunction)
		return true;
	else
		return (*m_pCallbackFunction)(pNode);
}

bool SpatialHierarchy::DoesBaseInstanceNeedProcessing(Aftk::C_SceneNode *pNode)
{
	if (!m_pBaseInstanceCallbackFunction)
		return true;
	else
		return (*m_pBaseInstanceCallbackFunction)(pNode);
}

void SpatialHierarchy::SetCallbackFunction(SpatialHierarchyCallbackFunctionBool *pCallbackFunction)
{
	m_pCallbackFunction = pCallbackFunction;
}

void SpatialHierarchy::SetBaseInstanceCallbackFunction(SpatialHierarchyCallbackFunctionBool *pCallbackFunction)
{
	m_pBaseInstanceCallbackFunction = pCallbackFunction;
}

void SpatialHierarchy::SetTriangleCallbackFunction(SpatialHierarchyCallbackFunctionInt *pCallbackFunction, void *pContext)
{
	m_pTriCallbackFunction = pCallbackFunction;
	m_pTriContext = pContext;
}

void SpatialHierarchy::SetNoCollisionPolyflagMask(int iPolyflag)
{
	m_iNoCollisionPolyflag = iPolyflag;	// this is obsolete. currently looks for named "polySet_dontCastShadows" polyflag
}

int GetLayerIndexFromSceneNodePtr(Aftk::C_SceneNode *pNode, int iLayer)
{
	Aftk::C_LayerNode *pLayer = pNode->GetLayerPtr(iLayer);

	if (!pLayer)
		return -1;

	return pNode->GetScenePtr()->GetLayerIndex( pLayer );
}

/// The callback function is called for each triangle in the hierarchy. The returned int is stored in the hierarchy.
void SpatialHierarchy::SetPrimitiveUserData(SpatialHierarchyCallbackFunctionInt *pCallbackFunction, void *pContext)
{
	// for each base-instance kd-tree store a user value for each triangle
	for (int iTree=0; iTree<m_KdTrees.Size(); iTree++)
	{
		// for each primitive in the tree
		int n = m_KdTrees[iTree].m_Primitives.Size();
		for (int i=0; i<n; i++)
		{
			TrianglePrim *pPrim = &m_KdTrees[iTree].m_Primitives[i];

			// retrieve the value using the callback
			int iValue = (*pCallbackFunction)(pPrim->m_pMeshNode, pContext, pPrim->m_iPolygon);

			// set the value into the upper 30 bits ofc: m_k
			m_KdTrees[iTree].m_pTris[i].m_k += iValue *4;			
		}
	}

	// for each main tree
	for (int iTree=0; iTree<m_MainKdTrees.Size(); iTree++)
	{
		// for each primitive in the tree
		int n = m_MainKdTrees[iTree].m_Primitives.Size();
		for (int i=0; i<n; i++)
		{
			TrianglePrim *pPrim = &m_MainKdTrees[iTree].m_Primitives[i];

			// base instances store the index of the polygon in the user data
			int iValue = pPrim->m_iPolygon;

			// set the value into the upper 30 bits of m_k
			m_MainKdTrees[iTree].m_pTris[i].m_k += iValue *4;			
		}
	}

	// for each instance store a user value
	for (int i=0; i<m_Objects.Size(); i++)
	{
		// retrieve the value using the callback
		int iValue = (*pCallbackFunction)(
			m_pAftkScene->GetSceneNodePtrByUniqueID(KdTreeTemplate<ObjectPrim>::m_pTris[i].m_iAftkSceneNode),
			pContext,
			0);

		// set the value into the upper 30 bits of m_k
		m_Objects[i].m_iUser = iValue;
	}
}
