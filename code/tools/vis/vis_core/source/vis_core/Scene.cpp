#include <stdafx.h>

#include <vis_core/vis.h>
#include <vis_core/FaceFlags.h>
#include <vis_core/Scene.h>
#include <vis_core/Mesh.h>
#include <vis_core/Sampler.h>
#include <vis_core/Util.h>
#include <vis_core/Sampler.h>
#include <vis_core/Kd-TreeBuilder.h>
#include <vis_core/TextureTagDefinitions.h>
#include <vis_core/PolyFlagDefinitions.h>
#include <vis_core/PolygonCluster.h>

#include <file_tag_db/FileTagDB.h>

#include <aftkut/AftkUtNodeFlagParser.h>
#include <aftkut/AftkUtBlindDataHelper.h>

#include <converter_shared/RenderDataConverter.h>
#include <converter_shared/RenderDataConversion/MaterialManager.h>
#include <converter_shared/RenderDataConversion/Effect.h>


IMPLEMENT_PERSISTENCE(Scene, "Scene");

bool CheckModelIsHighestLod(Aftk::C_SceneNode *pNode);

bool GetBool(const char *cp)
{
	if(isdigit(*cp))
	{
		if(!atoi(cp))
			return false;
		return true;
	}

	if(!stricmp(cp, "true") || !stricmp(cp, "yes"))
		return true;

	if(!stricmp(cp, "false") || !stricmp(cp, "no"))
		return false;

	return false;
}		


// calculates the default draw distance for a given bounding box
float CalcDefaultDrawDistance(box3f &bbox)
{
	return Length(bbox.GetSize()) * 200.0f;
}

// does this node want to be PVS'd using d3dquery?
bool NodePVSUsingD3DQuery(Aftk::C_SceneNode *pNode)
{
	Util::C_String name( pNode->GetName().c_str() );
	if (name.ISearch("pvsportal") != -1)
		return true;

	// anything flagged with "PVSUseOcclusionQuery" on the actor property gets rendered using occlusion queries
	std::string result;
	AftkUt::C_NodeFlagParser parser(*pNode);
	if (parser.GetParameterValue("PVSUseOcclusionQuery", result) && result != "" && atoi(result.c_str()) != 0)
		return true;

	return false;
}

inline const Aftk::C_BlindDataResource* GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	S16 localBdIndex = pPoly->GetBlindDataMappingIndex();
	if (localBdIndex == -1)
	{
		return 0;
	}

	return pMesh->GetPolygonBlindDataMappingPtr(localBdIndex);
}

// returns the source node ptr of an instance. returns NULL if the node is not an instance
Aftk::C_SceneNode *GetSourceNodeFromInstance(Aftk::C_SceneNode *pNode)
{
	if (pNode->GetType() == Aftk::E_ObjType_SceneInstance)
	{
		return ((Aftk::C_SceneInstanceNode*)pNode)->GetSourceNodePtr();
	}
	else if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance) )
	{
		return ((Aftk::C_SceneMeshRemappedInstanceNode*)pNode)->GetSourceNodePtr();
	}
	return NULL;
}

/// get the lod number for a given node
int GetLodNumber(const Aftk::C_SceneNode& node)
{
	const Aftk::C_SceneNode* pParent = node.GetParentPtr();

	int lod = -1;

	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		const Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*) pParent;

		for (U32 i=0; i < pLodGroup->GetNumLodLevels(); i++)
		{
			if (pLodGroup->GetLodLevelInfo(i).GetSceneNodePtr() == &node)
			{
				lod = i;
				break;
			}
		}
	}

	return lod;
}

// get the draw distance of a node from it's actor properties
float GetDrawDistance(Aftk::C_SceneNode *pNode)
{
	// check that it's an instance
	Aftk::C_SceneNode *pSourceNode = GetSourceNodeFromInstance(pNode);
	if (!pSourceNode)
		return FLT_MAX; // if it's not an instance, it has infinite draw distance

	// check that it's not a lod
	if (GetLodNumber(*pNode) != -1)
		return 0; // for lods, calculate our own draw-distance

	// get actor properties string from blind data
	std::string text;
	AftkUt::C_NodeFlagParser parser(*pSourceNode);
	// check whether it's got base instance actor properties
	if (parser.GetFlagGroupCategory() != "System" || parser.GetFlagGroupName() != "BaseInstance")
		return 0;

	// it has base instance actor properties. get the max draw distance parameter value
	std::string value;
	if (parser.GetParameterValue("MaxDrawDistance", value) && value != "")
		return (float) atof(value.c_str()) * 4.0f;

	// default is 0
	return 0;
}

void Scene::LoadAftkScene(Aftk::C_Scene *pAftk)
{
	// initialize polyrefs array
	m_pPolyRefs = new Ut::Vector<PolygonRef>;

	m_MeshNodeList.Destroy();

	for (int i=0; i<pAftk->GetNumLayers(); i++)
	{
		m_iCurrentLayer = i;

		Build_Tree(pAftk);

		AddInstances(pAftk);
	}

	m_iCurrentLayer = -1;

	Build_Tree(pAftk);

	AddInstances(pAftk);

	LOGINFO( g_Log, "%d nodes in scene\n",m_MeshNodeList.Size());
}

Scene::Scene()
{
	Initialize();
}

void Scene::Initialize()
{
	m_iCurId = 0;
	m_iCurNode = 0;
	m_iCurClusterId = 0;
	m_pPolyRefs = NULL;
	m_pPreviousNode = NULL;
	m_pPreviousBdNode = NULL;
	m_iTransparentForPVSPolyFlagMask = 0;
	m_pConverter = NULL;
	m_iClusterThreshold = 1;
	m_iPolygonThreshold = 1;
}

void Scene::SetPaths(const char *pszTexCachePath, const char *pszFXBPath)
{
	LOGDEBUG( g_Log, "Texture Cache: %s\n", pszTexCachePath);
	LOGDEBUG( g_Log, "FXB Directory: %s\n", pszFXBPath);
	m_pConverter = new RenderingData::C_Converter(pszTexCachePath, pszFXBPath, false);
}

bool Scene::NodeInCurrentLayer(Aftk::C_SceneNode *pNode)
{
	if (pNode->GetNumLayers() == 0)
		return m_iCurrentLayer == -1;

	if (m_iCurrentLayer == -1)
		return pNode->GetNumLayers() == 0;

	return pNode->GetLayerPtr(0) == pNode->GetScenePtr()->GetLayerPtr(m_iCurrentLayer);
}

// put the entire mesh in one big node
void Scene::Build_Monolith(Aftk::C_Scene *pAftk)
{
	m_iCurNode = m_MeshNodeList.Size();
	m_MeshNodeList.PushBack(new MeshNode());

	m_PVSOcclusionQueryNodes.Clear();

	for (Aftk::C_SceneNodeIterator it(pAftk->GetRootSceneNodePtr()); it; it++)
	{
		Aftk::C_SceneNode *pNode = it;
		bool bHasOcclusionQuery = false;

		if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		if (pNode->GetNumInstancesOfThisNode())
			continue;

		if (!NodeInCurrentLayer(pNode))
			continue;

		// skip certain layers
		if (!DoesNodeNeedProcessing(pNode, false))
			continue;

		if (NodePVSUsingD3DQuery(pNode))
			continue;

		AddMesh((Aftk::C_SceneMeshNode*)pNode, NULL, &bHasOcclusionQuery);

		if (bHasOcclusionQuery)
			m_PVSOcclusionQueryNodes.PushBack((Aftk::C_SceneMeshNode*)pNode);
	}
}

// build a fat AABB tree and use that to subdivide the scene into nodes
void Scene::Build_Tree(Aftk::C_Scene *pAftk)
{
	int nVertices, nTris;
	Vertex *pVtx;
	MeshNode *pTmpMeshNode;

	// collect mesh data into one node
	Build_Monolith(pAftk);

	// get the monolith node and remove it from the meshnodelist
	pTmpMeshNode = m_MeshNodeList.Back();
	Ut::Vector<Vertex> &vertices = pTmpMeshNode->GetVertices();
	nTris = vertices.Size()/3;
	if (nTris)
        pVtx = &vertices[0];
	m_MeshNodeList.PopBack();

	Box3fVector bboxvec;
	bboxvec.Resize(nTris);

	// create a bounding box for each triangle
	for (int i=0; i<nTris; i++)
	{
		bboxvec[i].UpdateBox(pVtx[i*3+0].vPos);
		bboxvec[i].UpdateBox(pVtx[i*3+1].vPos);
		bboxvec[i].UpdateBox(pVtx[i*3+2].vPos);
	}

	// build the tree
	KdBuilder *pBuilder = new KdBuilder( bboxvec,
		1,		// intersect cost
		1,		// traversal cost
		1500,	// min primitives per leafnode
		-1		// max depth
		);

	// collect the leafnodes and turn them into meshnodes
	for (int i=0; i<pBuilder->m_Nodes.Size(); i++)
	{
		KdNode *pNode = &pBuilder->m_Nodes[i];

		// is this a leafnode?
		if (!KDNODE_ISLEAF(pNode))
			continue;

		// check it's not empty
		if ((pNode->iFlagAndNumPrimitives & 0x7fffffff) == 0)
			continue;

		// create a new node
		int iPrimitive = pBuilder->m_Nodes[i].iPrimitive;
		m_iCurNode = m_MeshNodeList.Size();
		m_MeshNodeList.PushBack(new MeshNode());
		m_MeshNodeList.Back()->SetAftkLayer(m_iCurrentLayer);

		int iTri, iPrim;

		int iCol = ((rand()&255)) + ((rand()&255)<<8) + ((rand()&255)<<16);

		// first pass is for normal polys, second pass is for aplha polys
		for (int iPass=0; iPass<2; iPass++)
		{
			// get list of triangles for this node
			iPrim = 0;
			for (KdMailbox *pBox = &pBuilder->m_pMailboxes->At(iPrimitive); iPrim>=0; pBox++)
			{
				iPrim = pBox->iPrimitive;
				iTri = iPrim & 0x7fffffff;

				//Add polygons flagged as alpha.
				FaceFlags& faceFlags = pVtx[iTri*3+0].flags;
				bool bAlpha = faceFlags.alpha;
				if (bAlpha == (iPass == 1))
				{
					// add this triangle to the mesh
					m_MeshNodeList[m_iCurNode]->AddPolygon(
						pVtx[iTri*3+0].vPos,
						pVtx[iTri*3+1].vPos,
						pVtx[iTri*3+2].vPos,
						pVtx[iTri*3+0].flags);
				}
			}
		}
	}

	delete pTmpMeshNode;
	delete pBuilder->m_pMailboxes;
	delete pBuilder;
}

Scene::~Scene()
{
	delete m_pConverter;

	for (int i=0; i<m_MeshNodeList.Size(); i++)
		delete m_MeshNodeList[i];
	m_MeshNodeList.Destroy();

	if (m_pPolyRefs)
		delete m_pPolyRefs;
}

// return true if the polygon has a "do not export" polyflag on
bool Scene::DoesPolygonHaveNoExportPolyFlag(Aftk::C_SceneMeshNode *pAftkMesh, Aftk::C_Polygon *pPoly)
{
	const Aftk::C_SharedResource<Aftk::C_BlindDataNode>* resource = pAftkMesh->GetPolygonBlindDataMappingPtr(pPoly->GetBlindDataMappingIndex());

	if (!resource) 
		return false;

	const Aftk::C_BlindDataNode& aftkBlindDataNode = resource->GetContent();

	for (U32 j = 0; j < aftkBlindDataNode.GetNumAttributes(); j++) 
	{
		const Aftk::C_GenericAttribute&	aftkAttribute = aftkBlindDataNode.GetAttribute(j);

		if (!stricmp(aftkAttribute.GetName().c_str(), "polySet_doNotPVS"))
			return true;
	}

	return false;
}

// add an aftk mesh to this scene
void Scene::AddMesh(Aftk::C_SceneMeshNode* pAftkMesh, MeshNode *pExternalMeshNode, bool *pbHasOcclusionQuery = NULL)
{
	int nFaces = pAftkMesh->GetNumTriangles(0);

	int iCol = ((rand()&255)) + ((rand()&255)<<8) + ((rand()&255)<<16);

	Aftk::Matrix44 mWorldMatrix = pAftkMesh->GetWorldMatrix();

	bool hasAlpha = false;
	bool hasDoubleSided = false;
	bool hasOcclusionQuery = false;

	PolygonClusterMap polyClustering, polyClusterRemapping;

	if (m_iClusterThreshold > 1)
	{
		//only cluster if we have a large enough pixel threshold
		//first weld vertex positions otherwise the aftk polygon adjacency wont make much sense
		{
			Aftk::C_MeshWeldMemoryManager weldMemoryManager(
				AFTK_MESH_WELD_MEMORY_MANAGER_LARGE_CAPACITY);

			Aftk::C_MeshWeld::WeldVertexPositions(weldMemoryManager, pAftkMesh, Aftk::kOneMM);
		}

		//build clusters		
		U32 nClusters = BuildPolygonClusterMap(pAftkMesh, polyClustering);

		if (nClusters)
		{
			//init data structure for remapping per-mesh cluster indices to our per-scene mapping
			polyClusterRemapping.Resize(nClusters);
			memset(&polyClusterRemapping[0], -1, sizeof(S32)*nClusters);
		}
	}

	// collect polygons
	for (int iFace=0; iFace<nFaces; iFace++)
	{
		Aftk::C_Polygon *pPoly = &pAftkMesh->GetPolygon(iFace);

		assert(pPoly->GetNumPolygonVertices() == 3);

		if (DoesPolygonHaveNoExportPolyFlag(pAftkMesh, pPoly))
			continue;

		S32 clusterIndex = -1;
		if (m_iClusterThreshold > 1)
		{
			clusterIndex = polyClusterRemapping[polyClustering[iFace]];
			//fix up remapping table
			if (clusterIndex == -1)
			{
				polyClusterRemapping[polyClustering[iFace]] = m_iCurClusterId;

				clusterIndex = m_iCurClusterId;
				m_iCurClusterId++;
			}
		}
		else
		{
			//cluster per polygon (should be the same as m_iCurId)
			clusterIndex = m_iCurClusterId++;
		}

		FaceFlags faceFlags;
		AddPolygon(
			*pAftkMesh, 
			*pPoly, 
			mWorldMatrix, 
			iFace,
			clusterIndex,
			pExternalMeshNode,
			faceFlags);

		if (faceFlags.alpha)
			hasAlpha = true;
		if (faceFlags.doubleSided)
			hasDoubleSided = true;
		if (faceFlags.occlusionquery)
			hasOcclusionQuery = true;
	}

	if (pbHasOcclusionQuery)
		*pbHasOcclusionQuery = hasOcclusionQuery;
}

void Scene::AddPolygon(
	const Aftk::C_SceneMeshNode& aftkMesh, 
	const Aftk::C_Polygon& poly, 
	const Aftk::Matrix44& mtx, 
	U32 iFace, U32 iCluster,
	MeshNode* pExternalMeshNode,
	FaceFlags& faceFlags)
{
	float3 v[3];
	bool bPVSOcclusionQuery;

	// create the polygon
	for (int i=0; i < 3; i++)
	{
		// get polyvert
		const Aftk::C_PolygonVertex& pPV = poly.GetPolygonVertex(i);

		// get position
		Aftk::Position3 vPos = aftkMesh.GetVertexPosition( pPV.GetPositionIndex() );
		vPos = ApplyMatrix(mtx, vPos );
        v[i] = *(float3*)&vPos;
	}

	faceFlags.id = m_iCurId;

	// get the converted material for this polygon using the shared renderdata converter code
	RenderDataConversion::Material mtl;
	m_pConverter->GetMaterial(poly, &mtl);
	if (mtl.pEffect->GetAnnotation("PVSUsingOcclusionQuery", &bPVSOcclusionQuery) && bPVSOcclusionQuery)
		faceFlags.occlusionquery = bPVSOcclusionQuery;

	// infer alpha and doublesided flags from the blendstate on the polygon
	if ( mtl.blendState.GetAlphaTestEnable() || mtl.blendState.GetAlphaBlendEnable())
		faceFlags.alpha = true;
	if ( mtl.blendState.GetCullMode() == RenderingData::CULL_MODE_NONE )
		faceFlags.doubleSided = true;

	// potentially override flags using explicit annotations on the effect
	bool bValue;
	if (mtl.pEffect->GetAnnotation("TranslucentForPVS", &bValue))
		faceFlags.alpha = bValue;
	if (mtl.pEffect->GetAnnotation("DoubleSidedForPVS", &bValue))
		faceFlags.doubleSided = bValue;

	if (faceFlags.occlusionquery)
		return;

	// add to mesh
	if (!pExternalMeshNode)
	{
		m_MeshNodeList[m_iCurNode]->AddPolygon(v[0], v[1], v[2], faceFlags);

		// remember the polyref
		m_pPolyRefs->PushBack();
		(*m_pPolyRefs)[m_iCurId].m_iPolygon = iFace;
		(*m_pPolyRefs)[m_iCurId].m_iSceneNodeID = aftkMesh.GetUniqueId();

		// remember the cluster
		m_ClusterRefs.PushBack(iCluster);

		m_iCurId++;
	}
	else
		pExternalMeshNode->AddPolygon(v[0], v[1], v[2], faceFlags);

}

/// add a base-instance mesh. returns the index of the mesh
int Scene::AddBaseInstance(Aftk::C_SceneMeshNode *pBaseInstance)
{
	int idx = m_BaseInstanceList.Size();

	MeshNode *pMesh = new MeshNode;
	AddMesh(pBaseInstance,pMesh);
	m_BaseInstanceList.PushBack(pMesh);

	return idx;
}

// add an instance to our list
void Scene::AddInstance(Aftk::C_SceneNode *pInstance, Aftk::I_SceneMeshNode *pRefNode, int iBaseInstance)
{
	Aftk::Matrix44 mtx = pInstance->GetWorldMatrix();

	// calculate a bounding box for this instance
	box3f bbox;
	for (int iPoly=0; iPoly<pRefNode->GetNumPolygons(0); iPoly++)
	{
		// get poly
		const Aftk::C_Polygon *pPoly = &pRefNode->GetPolygon(iPoly);

		for (int i=0; i<3; i++)
		{
			// get polyvert
			U32 iPV = pPoly->GetPolygonVertexIndex(i);

			// get position
			Aftk::Position3 vPos = pRefNode->GetEvaluatedPolygonVertexPosition( iPV );
			vPos = ApplyMatrix(mtx, vPos );
			bbox.UpdateBox( float3(vPos.x,vPos.y,vPos.z) );
		}
	}

	// calculate draw distance
	float fDrawDistance = GetDrawDistance(pInstance);
	if (fDrawDistance == 0)
		fDrawDistance = CalcDefaultDrawDistance(bbox);

	// remember instance
	int i = m_InstanceList.Size();
	m_InstanceList.PushBack();
	m_InstanceList[i].m_bbox = bbox;
	m_InstanceList[i].m_fDrawDistance = fDrawDistance;
	m_InstanceList[i].m_iBaseInstance = iBaseInstance;
	m_InstanceList[i].m_iAftkLayer = m_iCurrentLayer;
	m_InstanceList[i].m_iRefIdx = m_iCurId;

	// remember the polyref
	m_pPolyRefs->PushBack();
	(*m_pPolyRefs)[m_iCurId].m_iPolygon = -1;
	(*m_pPolyRefs)[m_iCurId].m_iSceneNodeID = pInstance->GetUniqueId();

	// remember the cluster
	m_ClusterRefs.PushBack(m_iCurClusterId++);

	m_iCurId++;
}

// returns true if a node is in a route layer
bool IsThisARouteNode(Aftk::C_SceneNode *pNode)
{
	// check whether this is route mesh
	Aftk::C_LayerNode *pLayer = pNode->GetLayerPtr(0);
	if (pLayer)
	{
		if (strstr(pLayer->GetName().c_str(), "route"))
			return true;
		if (strstr(pLayer->GetName().c_str(), "Route"))
			return true;
	}

	// speed tree billboards are treated as route items
	if (strstr(pNode->GetName().c_str(), "SpeedTreeBillboard"))
		return true;

	return false;
}

// check whether an object is in the crowd pvs data layer
bool IsObjectInCrowdPvsDataLayer(Aftk::C_SceneNode *pNode)
{
	Aftk::C_LayerNode *pLayer = pNode->GetLayerPtr(0);
	if (pLayer)
	{
		if (strstr(pLayer->GetName().c_str(), "crowd_pvs_layer"))
			return true;
	}
	return false;
}

// check whether this node has "pvsBBox" properties and if so add it to the instance list with the given bbox dimensions
bool Scene::NodeHasPVSBlindData(Aftk::C_SceneNode *pNode)
{
    // get a node's blind data
	if (!pNode->GetBlindDataResourcePtr())
		return false;

	const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();
	if (!pBlindData)
		return false;

	Aftk::Vector3F32 vBBoxDimensions;
	for (int i=0; i<pBlindData->GetNumAttributes(); i++)
	{
		if (pBlindData->GetAttribute(i).GetName() == "pvsBBox")
		{
			// found bounding box blind data attr
			U32 data;
			const Aftk::C_GenericAttribute *pAttr = &pBlindData->GetAttribute(i);
			if (pAttr)
			{
				// get bounding box dimensions
				pAttr->GetValue(vBBoxDimensions);

				// add the bounding box into the instance list
				box3f bbox;
				Aftk::Matrix44 mtx = pNode->GetWorldMatrix();
				bbox.vmin.x = mtx[3][0] - vBBoxDimensions.x;
				bbox.vmin.y = mtx[3][1] - vBBoxDimensions.y;
				bbox.vmin.z = mtx[3][2] - vBBoxDimensions.z;
				bbox.vmax.x = mtx[3][0] + vBBoxDimensions.x;
				bbox.vmax.y = mtx[3][1] + vBBoxDimensions.y;
				bbox.vmax.z = mtx[3][2] + vBBoxDimensions.z;

				// calculate draw distance
				float fDrawDistance = CalcDefaultDrawDistance(bbox);

				// remember instance
				int i = m_InstanceList.Size();
				m_InstanceList.PushBack();
				m_InstanceList[i].m_bbox = bbox;
				m_InstanceList[i].m_fDrawDistance = fDrawDistance;
				m_InstanceList[i].m_iBaseInstance = -1;	// no base instance mesh, simply render the bbox

				// remember the polyref
				m_pPolyRefs->PushBack();
				(*m_pPolyRefs)[m_iCurId].m_iPolygon = -1;
				(*m_pPolyRefs)[m_iCurId].m_iSceneNodeID = pNode->GetUniqueId();
				
				// remember the cluster
				m_ClusterRefs.PushBack(m_iCurClusterId++);

				m_iCurId++;

				return true;
			}
		}
	}
	return false;
}

// add all instances to the scene's instance list
void Scene::AddInstances(Aftk::C_Scene *pAftk)
{
	for (Aftk::C_SceneNodeIterator it(pAftk->GetRootSceneNodePtr()); it; it++)
	{
		Aftk::C_SceneNode *pNode = it;
		int iBaseInstance = -1;

		if (!NodeInCurrentLayer(pNode))
			continue;

		// skip certain layers - route layers are treated as instances
		if (!DoesNodeNeedProcessing(pNode, true))
			continue;

		Aftk::C_SceneNode *pRefNode;
		if (pNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			pRefNode = ((Aftk::C_SceneInstanceNode*)pNode)->GetSourceNodePtr();
		}
		else if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance) )
		{
			pRefNode = pNode;
		}
		else if (pNode->GetNumInstancesOfThisNode() > 0 || IsThisARouteNode(pNode) || !CheckModelIsHighestLod(pNode))
			pRefNode = pNode;
		else if (NodePVSUsingD3DQuery(pNode) && pNode->GetType() == Aftk::E_ObjType_SceneMesh )
		{
			//iBaseInstance = AddBaseInstance((Aftk::C_SceneMeshNode*)pNode);
			pRefNode = pNode;
		}
		else if (NodeHasPVSBlindData(pNode))
		{
			continue;
		}
		else
			continue;

		//Skip null instances.
		if (!pRefNode)
			continue;
			
		// only do instances of meshnodes
		if (!pRefNode->HasType( Aftk::E_ObjType_ISceneMesh ))
			continue;

		// if the crowd layer only flag is on, only render crowd instances and nothing else
		if (gbCrowdPvsLayerOnly != IsObjectInCrowdPvsDataLayer(pNode))
			continue;

		AddInstance(pNode, (Aftk::I_SceneMeshNode*)pRefNode, iBaseInstance);
	}

	// add mesh nodes that had polygons with PVS occlusion query set on their effect
	for (int i=0; i<m_PVSOcclusionQueryNodes.Size(); i++)
	{
		AddInstance(m_PVSOcclusionQueryNodes[i], m_PVSOcclusionQueryNodes[i], -1);
	}
}

// remove ownership of polyref array
void Scene::TakeOwnerShipOfPolyRefs()
{
	m_pPolyRefs = NULL;
}

// add PVS passes info
void Scene::AddPVSPass(PVSPass &pass)
{
	m_Passes.PushBack(pass);
}

// serialization code
void Scene::write( ost::Engine &ar )
{
	assert(m_iCurId == m_ClusterRefs.Size());
	ar << m_MeshNodeList << m_iCurId << m_InstanceList << m_Passes << m_iClusterThreshold << m_iPolygonThreshold << m_iCurClusterId << m_ClusterRefs;
}

void Scene::read ( ost::Engine &ar )
{
	ar >> m_MeshNodeList >> m_iCurId >> m_InstanceList >> m_Passes >> m_iClusterThreshold >> m_iPolygonThreshold >> m_iCurClusterId >> m_ClusterRefs;
	assert(m_iCurId == m_ClusterRefs.Size());
}

