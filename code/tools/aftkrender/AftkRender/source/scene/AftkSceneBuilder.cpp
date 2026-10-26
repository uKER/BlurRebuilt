#include "AftkRenderPch.h"

#include "common/ShaderConstant.h"
#include "common/RenderContext.h"
#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

#include "scene/AftkMeshNode.h"
#include "scene/AftkSceneBuilder.h"
#include "scene/AftkSceneGraphNode.h"


using namespace AftkRender;

bool C_AftkSceneBuilder::BuildScene(
	Aftk::C_Scene* pScene,
	C_Scene *pRenderScene,
	Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap)
{
	assert(pScene && "Failed to acquire scene pointer");

	if (pScene->GetNumSceneNodes() == 0)
		return true;

	if (!BuildNode(pScene->GetRootSceneNodePtr(), pRenderScene, geometryNodes, nodeMap))
		return false;

	//point mesh instance nodes to their source geometry
	for (Aftk::C_SceneNodeConstIterator it(pScene); it; it++)
	{
		if (it->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			Aftk::C_SceneMeshRemappedInstanceNode *pInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode*) &(*it);
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator itMap1 = nodeMap.Find(pInstanceNode);
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator itMap2 = nodeMap.Find(pInstanceNode->GetSourceNodePtr());
			
			assert (itMap1 != nodeMap.End() && "Instance node has not been built");
			if (itMap2 != nodeMap.End())
			{
				C_AftkMeshSceneGraphNode* pSgNode = (C_AftkMeshSceneGraphNode*) itMap1->GetValue();
				C_AftkMeshSceneGraphNode* pSourceSgNode = (C_AftkMeshSceneGraphNode*) itMap2->GetValue();
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
					C_AftkMeshSceneGraphNode* pSgNode = (C_AftkMeshSceneGraphNode*) itMap1->GetValue();
					C_AftkMeshSceneGraphNode* pSourceSgNode = (C_AftkMeshSceneGraphNode*) itMap2->GetValue();
					pSgNode->SetGeometryNode(pSourceSgNode->GetGeometryNode());
				}
			}
		}
	}
	return true;
}


C_SceneGraphNode* C_AftkSceneBuilder::BuildNode(
	Aftk::C_SceneNode* pSceneNode,
	C_Scene *pRenderScene,
	Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap)
{
	
	assert(pSceneNode && "Failed to obtain scene node pointer");

	C_AftkSceneGraphNode *pSgNode = 0;
/*
	if (!AftkUt::Query::IsExportable(pAftkSceneNode))
	{
		//Create placeholder
		pSgNode = new C_AftkSceneGraphNode(pRenderScene, pSceneNode);
	}
	else
*/
	{
		switch (pSceneNode->GetType())
		{
		case Aftk::E_ObjType_SceneMeshBonedInstance:
			pSgNode = new C_AftkMeshSkinnedSceneGraphNode(pRenderScene, pSceneNode);
			break;
		case Aftk::E_ObjType_SceneMesh:
		case Aftk::E_ObjType_SceneMeshRemappedInstance:
			pSgNode = new C_AftkMeshSceneGraphNode(pRenderScene, pSceneNode);
			break;
		case Aftk::E_ObjType_SceneInstance:
			{
				Aftk::C_SceneInstanceNode* pInstance = (Aftk::C_SceneInstanceNode*) pSceneNode;
				if (pInstance->GetSourceNodePtr() && pInstance->GetSourceNodePtr()->HasType(Aftk::E_ObjType_SceneMeshRemappable))
					pSgNode = new C_AftkMeshSceneGraphNode(pRenderScene, pSceneNode);
				else
					pSgNode = new C_AftkSceneGraphNode(pRenderScene, pSceneNode);
			}
			break;
		default:
			pSgNode = new C_AftkSceneGraphNode(pRenderScene, pSceneNode);
		};
	}

	if (!pSgNode)
		return 0;

	nodeMap[pSceneNode] = pSgNode;


	if(pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		//build mesh and attach to scene graph node
		C_AftkMeshNode *pMeshNode = new C_AftkMeshNode((Aftk::C_SceneMeshNode*)pSceneNode);
		((C_AftkMeshSceneGraphNode*)pSgNode)->SetGeometryNode(pMeshNode);
		geometryNodes.PushBack(pMeshNode);
	}

	Aftk::C_SceneNode *pChildSceneNode = pSceneNode->GetChildPtr(0);
	while (pChildSceneNode)
	{
		C_AftkSceneGraphNode *pChildSgNode = (C_AftkSceneGraphNode*) BuildNode(pChildSceneNode, pRenderScene, geometryNodes, nodeMap);
		if (pChildSgNode)
			pSgNode->AddChild(pChildSgNode);

		pChildSceneNode = pChildSceneNode->GetNextPtr();
	}

	return pSgNode;
}