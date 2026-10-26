#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/SceneXtNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>

namespace Editor
{

C_ActorScene::C_ActorScene()
{
	Initialise();
}

C_ActorScene::~C_ActorScene()
{
	AftkRender::C_Scene::Clear();
	m_DgRenderNodeMap.Clear();
}

void C_ActorScene::Initialise()
{
	m_DgRenderNodeMapDirty = false;
	m_pRootSceneGraphNode = new AftkRender::C_SceneGraphNode(this, "PseudoRoot");
}

void C_ActorScene::Clear()
{
	AftkRender::C_Scene::Clear();
	m_DgRenderNodeMap.Clear();

	Initialise();
}

AftkRender::C_SceneGraphNode* C_ActorScene::Find(const EditorLib::I_DgNode* pDgNode)
{
	if (m_DgRenderNodeMapDirty)
		RebuildDgRenderNodeMap();

	Ut::UtMap<const EditorLib::I_DgNode*, C_DgNodeBase*>::Iterator it = m_DgRenderNodeMap.Find(pDgNode);
	if (it != m_DgRenderNodeMap.End())
		return it->GetValue();

	return 0;
}

const AftkRender::C_SceneGraphNode* C_ActorScene::Find(const EditorLib::I_DgNode* pDgNode) const
{
	if (m_DgRenderNodeMapDirty)
		RebuildDgRenderNodeMap();

	Ut::UtMap<const EditorLib::I_DgNode*, C_DgNodeBase*>::Iterator it = m_DgRenderNodeMap.Find(pDgNode);
	if (it != m_DgRenderNodeMap.End())
		return it->GetValue();

	return 0;
}

void C_ActorScene::GetVisible(
	const RECT &region, C_DgNodeSelection &result)
{
	for (AftkRender::C_SceneGraphNodeIterator it(this); (AftkRender::C_SceneGraphNode*)it; ++it)
	{
		//Assumption: All custom dg nodes derived from C_DgNodeBase.
		if (it->IsA(AftkRender::E_SceneGraphNode_Custom))
			static_cast<C_DgNodeBase*>((AftkRender::C_SceneGraphNode*)it)->GetVisible(region, &result);
	}
}

AftkRender::C_SceneGraphNode* C_ActorScene::AddActorSceneNode(const EditorLib::I_DgNode& dgNode)
{
	if (!dgNode.IsA(EditorLib::E_ObjType_SceneNode))
		//leave it up to the render nodes to instantiate property render nodes
		return 0;

	const EditorLib::C_SceneNode& actor = (const EditorLib::C_SceneNode&)dgNode;
	AftkRender::C_SceneGraphNode *pParentNode = actor.GetParent() ? Find(actor.GetParent()) : 0;
	if (!pParentNode)
		pParentNode = m_pRootSceneGraphNode;

	{
		C_DgNodeBase* pNewActor = 0;
		if (dgNode.IsA(EditorLib::E_ObjType_SceneXtNodeBase))
			pNewActor = new C_ActorSceneNode(this, (const EditorLib::C_SceneXtNodeBase*)&actor);

		if (pNewActor)
		{
			pParentNode->AddChild(pNewActor);
		}

		return pNewActor;
	}
}

void C_ActorScene::RemoveActorSceneNode(const EditorLib::I_DgNode& dgNode)
{
	AftkRender::C_SceneGraphNode *pActorNode = Find(&dgNode);
	if (pActorNode)
		DeleteSceneGraphFromNode(pActorNode);
}

S32 C_ActorScene::OnCreateSceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode, S32 preferredId)
{
	m_DgRenderNodeMapDirty = true;
	return AftkRender::C_Scene::OnCreateSceneGraphNode(pSgNode, preferredId);
}

void C_ActorScene::OnDestroySceneGraphNode(AftkRender::C_SceneGraphNode *pSgNode)
{
	//remove backend dgNode-> render node mapping
	if (pSgNode != m_pRootSceneGraphNode && 
		(pSgNode->GetCustomTypeData()==E_CustomNodeType_Actor || pSgNode->GetCustomTypeData()==E_CustomNodeType_Property))
	{
		C_DgNodeBase *pDgSgNode = (C_DgNodeBase*) pSgNode;

		Ut::UtMap<const EditorLib::I_DgNode*, C_DgNodeBase*>::Iterator mapIt = m_DgRenderNodeMap.Find(pDgSgNode->GetDgNode());
		if (mapIt != m_DgRenderNodeMap.End())
			m_DgRenderNodeMap.Erase(mapIt);
	}


	return AftkRender::C_Scene::OnDestroySceneGraphNode(pSgNode);
}

void C_ActorScene::OnNodeDeserialised(const EditorLib::I_DgNode *pNode)
{
	const C_ActorScene::DgRenderMap& dgRenderMap = GetDgRenderMap();
	C_ActorScene::DgRenderMap::ConstIterator currentNode = dgRenderMap.End();

	if (pNode)
	{
		// notify rendernode of deserialisation first
		currentNode = dgRenderMap.Find(pNode);
		if (currentNode != dgRenderMap.End())
			currentNode->GetValue()->OnNodeDeserialised(pNode);
	}

	//notify all other rendernodes
	for (C_ActorScene::DgRenderMap::ConstIterator it(dgRenderMap.Begin()); 
		it != dgRenderMap.End(); ++it)
	{
		if (it != currentNode)
			it->GetValue()->OnNodeDeserialised(pNode);
	}
}

void C_ActorScene::OnNodeResolved(const EditorLib::I_DgNode *pNode)
{
	if(pNode && pNode->IsA(EditorLib::E_ObjType_SceneNode) && !((EditorLib::C_SceneNode*)pNode)->GetParent())
	{
		return;
	}

	const C_ActorScene::DgRenderMap& dgRenderMap = GetDgRenderMap();
	C_ActorScene::DgRenderMap::ConstIterator currentNode = dgRenderMap.End();

	if (pNode)
	{
		// notify rendernode of resolution first
		currentNode = dgRenderMap.Find(pNode);
		if (currentNode != dgRenderMap.End())
		{
			//fix up hierarchy
			if(pNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				const EditorLib::C_SceneNode *pParent = static_cast<const EditorLib::C_SceneNode*>(pNode)->GetParent();
				C_ActorScene::DgRenderMap::ConstIterator parentNode = dgRenderMap.Find(pParent);

				AftkRender::C_SceneGraphNode *pCurrentRenderParent = currentNode->GetValue()->GetParentPtr();
				AftkRender::C_SceneGraphNode *pNewRenderParent = (parentNode != dgRenderMap.End()) ? parentNode->GetValue() : m_pRootSceneGraphNode;

				assert(pNewRenderParent);
				if (pCurrentRenderParent != pNewRenderParent)
				{
					currentNode->GetValue()->Detach();
					pNewRenderParent->AddChild(currentNode->GetValue());
				}
			}

			currentNode->GetValue()->OnNodeResolved(pNode);
		}
	}
	else
	{
		//fix up entire hierarchy
		for (C_ActorScene::DgRenderMap::ConstIterator it(dgRenderMap.Begin()); 
			it != dgRenderMap.End(); ++it)
		{
			if (it->GetKey()->IsA(EditorLib::E_ObjType_SceneNode))
			{
				//fix up hierarchy
				const EditorLib::C_SceneNode *pParent = static_cast<const EditorLib::C_SceneNode*>(it->GetKey())->GetParent();
				C_ActorScene::DgRenderMap::ConstIterator parentNode = dgRenderMap.Find(pParent);

				AftkRender::C_SceneGraphNode *pCurrentRenderParent = it->GetValue()->GetParentPtr();
				AftkRender::C_SceneGraphNode *pNewRenderParent = (parentNode != dgRenderMap.End()) ? parentNode->GetValue() : m_pRootSceneGraphNode;

				assert(pNewRenderParent);
				if (pCurrentRenderParent != pNewRenderParent)
				{
					pCurrentRenderParent->DetachChild(it->GetValue());
					pNewRenderParent->AddChild(it->GetValue());
				}				
			}
		}
	}

	//notify all other rendernodes
	for (C_ActorScene::DgRenderMap::ConstIterator it(dgRenderMap.Begin()); 
		it != dgRenderMap.End(); ++it)
	{
		if (it != currentNode)
			it->GetValue()->OnNodeResolved(pNode);
	}
}

const C_ActorScene::DgRenderMap& C_ActorScene::GetDgRenderMap()
{
	if (m_DgRenderNodeMapDirty)
		RebuildDgRenderNodeMap();
	return m_DgRenderNodeMap;
}

void C_ActorScene::RebuildDgRenderNodeMap() const
{
	Ut::UtMap<S32, AftkRender::C_SceneGraphNode*>::ConstIterator sgNodeIt = m_SceneGraphNodeIdMap.Begin();
	while (sgNodeIt != m_SceneGraphNodeIdMap.End())
	{
		const AftkRender::C_SceneGraphNode *pNode = sgNodeIt->GetValue();
		if (pNode != m_pRootSceneGraphNode && 
			(pNode->GetCustomTypeData()==E_CustomNodeType_Actor || pNode->GetCustomTypeData()==E_CustomNodeType_Property))
		{
			C_DgNodeBase *pDgSgNode = (C_DgNodeBase*) pNode;

			Ut::UtMap<const EditorLib::I_DgNode*, C_DgNodeBase*>::ConstIterator mapIt = m_DgRenderNodeMap.Find(pDgSgNode->GetDgNode());
			if (mapIt == m_DgRenderNodeMap.End())
				m_DgRenderNodeMap.Insert(Ut::UtMap<const EditorLib::I_DgNode*, C_DgNodeBase*>::DataType(pDgSgNode->GetDgNode(), pDgSgNode));
		}
		++sgNodeIt;
	}

	m_DgRenderNodeMapDirty = false;
}

}