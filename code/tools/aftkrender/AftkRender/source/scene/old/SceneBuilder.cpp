#include "stdafx.h"
#include "AftkRender.h"

using namespace AftkRender;

C_RenderNodeBuilder::C_RenderNodeBuilder(
	C_SceneRenderer *pRenderer,
	Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *pAftkRenderNodeMap)
:
	I_SceneNodeVisitor<Aftk::C_SceneNode>(),
	m_pRenderer(pRenderer),
	m_pAftkRenderNodeMap(pAftkRenderNodeMap)
{
}

C_RenderNodeBuilder::~C_RenderNodeBuilder()
{
}

bool C_RenderNodeBuilder::IsInteresting(Aftk::C_SceneNode *pNode)
{
	return true;
}

Aftk::C_Status C_RenderNodeBuilder::Visit(
	Aftk::C_SceneNode *pNode,
	C_NullMemoryManager& memoryManager)
{
	C_RenderNode *pNewRenderNode = 0;

	ASSERT(m_pRenderer);
	ASSERT(m_pAftkRenderNodeMap);

	switch (pNode->GetType())
	{
	case Aftk::E_ObjType_SceneMesh:
	{
		Aftk::C_SceneMeshNode *pAftkMeshNode = (Aftk::C_SceneMeshNode*)pNode;
		if (pAftkMeshNode->GetNumPolygons(0) > 0)
		{
			pNewRenderNode = new C_RenderMeshNode(pNode, (Aftk::C_SceneMeshRemappableNode*) pNode, m_pRenderer);
			if (!pNewRenderNode)
				return Aftk::C_Status::E_Status_OutOfMemory;
		}
		else
		{
			pNewRenderNode = new C_RenderNode(pNode, m_pRenderer);
			if (!pNewRenderNode)
				return Aftk::C_Status::E_Status_OutOfMemory;

		}
		break;
	}
	case Aftk::E_ObjType_SceneMeshRemappedInstance:
	{
		Aftk::C_SceneMeshRemappedInstanceNode *pAftkInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)pNode;
		if (pAftkInstance->GetSourceNodePtr())
		{
			Aftk::C_SceneMeshNode *pAftkMesh = (Aftk::C_SceneMeshNode*)pAftkInstance->GetSourceNodePtr();
			if (pAftkMesh->GetNumPolygons(0) > 0)
			{
				pNewRenderNode = new C_RenderMeshInstanceNode(pNode, (Aftk::C_SceneMeshRemappableNode*) pNode, m_pRenderer);
				if (!pNewRenderNode)
					return Aftk::C_Status::E_Status_OutOfMemory;
			}
		}

		if (!pNewRenderNode)
		{
			pNewRenderNode = new C_RenderNode(pNode, m_pRenderer);
			if (!pNewRenderNode)
				return Aftk::C_Status::E_Status_OutOfMemory;
		}
		break;
	}
	case Aftk::E_ObjType_SceneInstance:
	{
		Aftk::C_SceneInstanceNode* pAftkInstance = (Aftk::C_SceneInstanceNode*)pNode;
		if (pAftkInstance->GetSourceNodePtr())
		{
			Aftk::C_SceneNode *pSourceNode = pAftkInstance->GetSourceNodePtr();
			if (pSourceNode->HasType(Aftk::E_ObjType_ISceneMesh) &&
				((Aftk::C_SceneMeshNode*) pSourceNode)->GetNumPolygons(0) > 0)
			{
				pNewRenderNode = new C_RenderMeshInstanceNode(pNode, (Aftk::C_SceneMeshRemappableNode*) pNode, m_pRenderer);
				if (!pNewRenderNode)
					return Aftk::C_Status::E_Status_OutOfMemory;
			}
			else
			{
				// not an instance of a mesh
				pNewRenderNode = new C_RenderInstanceNode(pNode, m_pRenderer);
			}
		}

		if (!pNewRenderNode)
		{
			pNewRenderNode = new C_RenderNode(pNode, m_pRenderer);
			if (!pNewRenderNode)
				return Aftk::C_Status::E_Status_OutOfMemory;
		}
		break;
	}
	case Aftk::E_ObjType_SceneNode:
	default:
	{
		pNewRenderNode = new C_RenderNode(pNode, m_pRenderer);
		if (!pNewRenderNode)
			return Aftk::C_Status::E_Status_OutOfMemory;

		break;
	}
	}

	(*m_pAftkRenderNodeMap)[pNode] = pNewRenderNode;

	return Aftk::C_Status::E_Status_Success;
}


C_RenderNodeHierarchyBuilder::C_RenderNodeHierarchyBuilder(
	Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *pAftkRenderNodeMap)
:
	I_SceneNodeVisitor<Aftk::C_SceneNode>(),
	m_pAftkRenderNodeMap(pAftkRenderNodeMap)
{
}


bool C_RenderNodeHierarchyBuilder::IsInteresting(Aftk::C_SceneNode *pNode)
{
	return true;
}

Aftk::C_Status C_RenderNodeHierarchyBuilder::Visit(
	Aftk::C_SceneNode *pNode,
	C_NullMemoryManager& memoryManager)
{
	Aftk::C_SceneNode *pParentNode = pNode->GetParentPtr();
	if (!pParentNode)
		return Aftk::C_Status::E_Status_Success;

	Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>::Iterator it;
	it = m_pAftkRenderNodeMap->Find(pNode);

	if (it == m_pAftkRenderNodeMap->End())
	{
		LOGWARN(g_Log, "Could not map aftk scene node to a render node\n");
		return Aftk::C_Status::E_Status_Failure;
	}
	C_RenderNode *renderNode = it->GetValue();

	it = m_pAftkRenderNodeMap->Find(pParentNode);

	if (it == m_pAftkRenderNodeMap->End())
	{
		LOGWARN(g_Log, "Could not map aftk scene node to a render node\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	C_RenderNode *pParentRenderNode = it->GetValue();

	pParentRenderNode->AddChild(renderNode);

	return Aftk::C_Status::E_Status_Success;
}


C_RenderNodeInstanceBuilder::C_RenderNodeInstanceBuilder(
	Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*> *pAftkRenderNodeMap)
:
	I_SceneNodeVisitor<Aftk::C_SceneNode>(),
	m_pAftkRenderNodeMap(pAftkRenderNodeMap)
{
}

bool C_RenderNodeInstanceBuilder::IsInteresting(Aftk::C_SceneNode *pNode)
{
	if (pNode->GetType() == Aftk::E_ObjType_SceneInstance ||
		pNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance ||
		pNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
	{
		return true;
	}
	else return false;
}

Aftk::C_Status C_RenderNodeInstanceBuilder::Visit(
	Aftk::C_SceneNode *pNode,
	C_NullMemoryManager& memoryManager)
{
	Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>::Iterator it;
	it = m_pAftkRenderNodeMap->Find(pNode);

	if (it == m_pAftkRenderNodeMap->End())
	{
		LOGWARN(g_Log, "Could not map aftk scene node to a render node\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	C_RenderNode *pRenderNode = it->GetValue();

	if (pRenderNode->GetType() == E_RenderNodeType_Node_Instance ||
		pRenderNode->HasType(E_RenderNodeType_Mesh_Instance))
	{
		it = m_pAftkRenderNodeMap->End();
		if (pNode->GetType() == Aftk::E_ObjType_SceneInstance)
			it = m_pAftkRenderNodeMap->Find(((Aftk::C_SceneInstanceNode*)pNode)->GetSourceNodePtr());
		else if (pNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
			it = m_pAftkRenderNodeMap->Find(((Aftk::C_SceneMeshRemappedInstanceNode*)pNode)->GetSourceNodePtr());
		else if (pNode->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
			it = m_pAftkRenderNodeMap->Find(((Aftk::C_SceneMeshBonedInstanceNode*)pNode)->GetSourceNodePtr());

		if (it == m_pAftkRenderNodeMap->End())
		{
			LOGWARN(g_Log, "Could not map aftk scene node to a render node\n");
			return Aftk::C_Status::E_Status_Failure;
		}

		C_RenderNode *pSourceRenderNode = it->GetValue();

		if (pRenderNode->GetType() == E_RenderNodeType_Node_Instance)
		{
			C_RenderInstanceNode *pInstance = (C_RenderInstanceNode*) pRenderNode;
			pInstance->SetSourceNodePtr(pSourceRenderNode);
		}
		else if (pRenderNode->HasType(E_RenderNodeType_Mesh_Instance))
		{
			C_RenderMeshInstanceNode *pMeshInstance = (C_RenderMeshInstanceNode*) pRenderNode;
			pMeshInstance->SetSourceNodePtr((C_RenderMeshNode*)pSourceRenderNode);
		}
	}
	return Aftk::C_Status::E_Status_Success;
}