#include "AftkRenderPch.h"
#include "mesh/MeshSceneBuilder.h"

#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

#include "mesh/MeshElements.h"
#include "mesh/Mesh.h"
#include "mesh/MeshNode.h"
#include "mesh/MeshSceneGraphNode.h"
#include "mesh/MeshBuilder.h"

namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

using namespace AftkRender;

__forceinline Maths::HMat44 ConvertToHMat44(
	const Aftk::Matrix44& rhs)
{
	return Maths::HMat44(
		Maths::HVec4(rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3]),
		Maths::HVec4(rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3]),
		Maths::HVec4(rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3]),
		Maths::HVec4(rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3]));
}

__forceinline Maths::HVec3 ConvertToHVec3( 
	const UtMaths::MPoint< UtMaths::VectorXYZ<F32> >& rhs)
{
	return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
}


bool IgnoreMesh(const Aftk::C_SceneMeshNode* pAftkSceneNode)
{
	if (pAftkSceneNode->GetPlugPtr("collisionPrimitive") != 0)
		return true;

	return false;
}


bool C_MeshSceneBuilder::BuildScene(
	Aftk::C_Scene *pAftkScene,
	C_Scene *pRenderScene,
	Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap)
{
	assert(pAftkScene && "Failed to acquire scene pointer");

	if (pAftkScene->GetNumSceneNodes() == 0)
		return true;

	if (!BuildNode(pAftkScene->GetRootSceneNodePtr(), pRenderScene, geometryNodes, nodeMap))
		return false;

	//point mesh instance nodes to their source geometry
	for (Aftk::C_SceneNodeConstIterator it(pAftkScene); it; it++)
	{
		if (it->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			Aftk::C_SceneMeshRemappedInstanceNode *pInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode*) &(*it);
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator itMap1 = nodeMap.Find(pInstanceNode);
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator itMap2 = nodeMap.Find(pInstanceNode->GetSourceNodePtr());
			
			assert (itMap1 != nodeMap.End() && "Instance node has not been built");
			if (itMap2 != nodeMap.End())
			{
				C_MeshSceneGraphNode* pSgNode = (C_MeshSceneGraphNode*) itMap1->GetValue();
				C_MeshSceneGraphNode* pSourceSgNode = (C_MeshSceneGraphNode*) itMap2->GetValue();
				pSgNode->SetMeshResource(pSourceSgNode->GetMeshResource());
				pSgNode->SetGeometryNode(pSourceSgNode->GetGeometryNode());
			}
		}
		else if (it->HasType(Aftk::E_ObjType_SceneInstance))
		{
			Aftk::C_SceneInstanceNode *pInstanceNode = (Aftk::C_SceneInstanceNode*) &(*it);
			if (pInstanceNode->GetSourceNodePtr() && pInstanceNode->GetSourceNodePtr()->HasType(Aftk::E_ObjType_SceneMeshRemappable))
			{
				Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator itMap1 = nodeMap.Find(pInstanceNode);
				Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator itMap2 = nodeMap.Find(pInstanceNode->GetSourceNodePtr());
				
				assert (itMap1 != nodeMap.End() && "Instance node has not been built");
				if (itMap2 != nodeMap.End())
				{
					C_MeshSceneGraphNode* pSgNode = (C_MeshSceneGraphNode*) itMap1->GetValue();
					C_MeshSceneGraphNode* pSourceSgNode = (C_MeshSceneGraphNode*) itMap2->GetValue();
					pSgNode->SetMeshResource(pSourceSgNode->GetMeshResource());
					pSgNode->SetGeometryNode(pSourceSgNode->GetGeometryNode());
				}
			}
		}
	}

	{
		Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator it = nodeMap.Find(pAftkScene->GetRootSceneNodePtr());
		if (it != nodeMap.End())
			it->GetValue()->UpdateCache();
	}

	return true;
}


C_SceneGraphNode* C_MeshSceneBuilder::BuildNode(
	Aftk::C_SceneNode *pAftkSceneNode,
	C_Scene *pRenderScene,
	Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap)
{
	assert(pAftkSceneNode && "Failed to obtain scene node pointer");

	C_MeshSceneGraphNode *pSgNode = 0;
	switch (pAftkSceneNode->GetType())
	{
	case Aftk::E_ObjType_SceneMeshBonedInstance:
		pSgNode = new C_MeshSkinnedSceneGraphNode(pRenderScene, pAftkSceneNode->GetName().c_str(), pAftkSceneNode->GetUniqueId());
		break;
	default:
		pSgNode = new C_MeshSceneGraphNode(pRenderScene, pAftkSceneNode->GetName().c_str(), pAftkSceneNode->GetUniqueId());
	};

	if (!pSgNode)
		return 0;

	nodeMap[pAftkSceneNode] = pSgNode;

	pSgNode->SetLocalMatrix(ConvertToHMat44(pAftkSceneNode->GetLocalMatrix()));

	if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		if (!IgnoreMesh((Aftk::C_SceneMeshNode*)pAftkSceneNode))
		{
			//build mesh and attach to scene graph node
			C_MeshResource *pMeshResource = new C_MeshResource();

			if (!C_MeshBuilder::Build(pMeshResource, (Aftk::C_SceneMeshNode*)pAftkSceneNode))
				LOGERROR(g_Log, "Failed to build mesh data for %s", pSgNode->GetName().Chars());

			C_MeshNode *pMeshNode = new C_MeshNode(pMeshResource);

			pSgNode->SetMeshResource(pMeshResource);
			pSgNode->SetGeometryNode(pMeshNode);
			geometryNodes.PushBack(pMeshNode);
		}
	}


	if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		Aftk::C_SceneMeshRemappableNode *pRemappable = (Aftk::C_SceneMeshRemappableNode*)pAftkSceneNode;
		
		Ut::UtVector< Ut::DynamicVector< Aftk::C_SharedResourceMapping<Aftk::C_MaterialNode>* > > mapping;
		pRemappable->GetPolygonMaterialMappingArray(mapping);
		for (S32 i=0; i<mapping.Size(); ++i)
		{
			U16 index = pRemappable->GetPolygonMaterialMappingPtr(i)->GetIndex();
			pSgNode->SetMaterialMapping(i, index);
		}
	}

	if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
	{
		Aftk::C_SceneInstanceNode *pAftkInstance = (Aftk::C_SceneInstanceNode*)pAftkSceneNode;

		if (pAftkInstance->GetSourceNodePtr() && pAftkInstance->GetSourceNodePtr()->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		{
			Aftk::C_SceneMeshRemappableNode *pRemappable = (Aftk::C_SceneMeshRemappableNode*)pAftkInstance->GetSourceNodePtr();
			
			Ut::UtVector< Ut::DynamicVector< Aftk::C_SharedResourceMapping<Aftk::C_MaterialNode>* > > mapping;
			pRemappable->GetPolygonMaterialMappingArray(mapping);
			for (S32 i=0; i<mapping.Size(); ++i)
			{
				U16 index = pRemappable->GetPolygonMaterialMappingPtr(i)->GetIndex();
				pSgNode->SetMaterialMapping(i, index);
			}
		}
	}

	if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
	{
		C_MeshSkinnedSceneGraphNode *pSkinnedSgNode = (C_MeshSkinnedSceneGraphNode*) pSgNode;
		Aftk::C_SceneMeshBonedInstanceNode *pAftkInstance = (Aftk::C_SceneMeshBonedInstanceNode*) pAftkSceneNode;

		Aftk::C_SceneNode* pBone0 = pAftkInstance->GetBonePtr(0);
		Aftk::C_SceneNode* pBone1 = pAftkInstance->GetBonePtr(1);

		Aftk::Matrix44 m0 = pAftkInstance->GetBasePose()*pBone0->GetBasePoseInverse()*pBone0->GetWorldMatrix();
		Aftk::Matrix44 m1 = pAftkInstance->GetBasePose()*pBone1->GetBasePoseInverse()*pBone1->GetWorldMatrix();

		pSkinnedSgNode->SetMatrixPaletteEntry(0, ConvertToHMat44(m0));
		pSkinnedSgNode->SetMatrixPaletteEntry(1, ConvertToHMat44(m1));
	}

	Aftk::C_SceneNode *pChildSceneNode = pAftkSceneNode->GetChildPtr(0);
	while (pChildSceneNode)
	{
		C_MeshSceneGraphNode *pChildSgNode = (C_MeshSceneGraphNode*) BuildNode(pChildSceneNode, pRenderScene, geometryNodes, nodeMap);
		if (pChildSgNode)
			pSgNode->AddChild(pChildSgNode);

		pChildSceneNode = pChildSceneNode->GetNextPtr();
	}

	return pSgNode;
}
