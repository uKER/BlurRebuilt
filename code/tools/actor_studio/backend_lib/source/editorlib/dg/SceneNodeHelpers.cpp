#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/Hints/HintCollection.h>
#include <EditorLib/Dg/Hints/ImmediateReparentHint.h>
#include <EditorLib/Dg/Hints/ImmediateTransformHint.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

void C_SceneNodeHelpers::ExtractSceneNodeAndReparentChildren(
	C_SceneNode* pNode, bool updateChildMatrices)
{
	if (!pNode || !(pNode->GetSceneNodeFlags().canReparent && !pNode->GetNodeFlags().external))
	{
		assert(!"Invalid node to extract!");
		return;
	}

	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(pNode->GetId());
	BIZ_ASSERT(pLevel);

	C_SceneNode* pRoot = pLevel->GetRootSceneNode();
	if (pNode == pRoot)
	{
		assert(!"Cannot extract the root scene node");
		return;
	}

	if (!pNode->m_pParent)
	{
		assert(!"Node must have a parent!");
	}

	//Ensure that our pNode's children are transformed if need be.
	if (updateChildMatrices)
	{
		C_SceneNode *pChild = pNode->m_pChild;
		while (pChild)
		{
			pChild->SetLocalMatrix(pChild->GetLocalMatrix() * pNode->GetLocalMatrix());
			pChild = pChild->m_pNext;
		}
	}

	C_SceneNode *pChild = pNode->m_pChild;
	C_SceneNode *pParent = pNode->m_pParent;
	assert(pParent);

	//detach pNode
	if (pNode->m_pPrevious)
		pNode->m_pPrevious->m_pNext = pNode->m_pNext;

	if (pNode->m_pNext)
		pNode->m_pNext->m_pPrevious = pNode->m_pPrevious;

	if (pNode->m_pParent->m_pChild == pNode)
		pNode->m_pParent->m_pChild = pNode->m_pNext;
	
	pNode->m_pParent = 0;
	pNode->m_pPrevious = 0;
	pNode->m_pNext = 0;
	pNode->m_pChild = 0;

	//add children to parent
	Btl::Vector<C_SceneNode*> children;
	while (pChild)
	{
		children.PushBack(pChild);
		pChild = pChild->m_pNext;
	}


	for (S32 i = 0; i < children.Size(); ++i)
	{
		children[i]->m_pNext = children[i]->m_pPrevious = children[i]->m_pParent = NULL;
		pParent->AddChild(children[i]);
	}
}

bool C_SceneNodeHelpers::ReparentSceneNode(
	C_SceneNode* pNode, C_SceneNode* pNewParent, bool absoluteTransform)
{
	Matrix44 world = pNode->GetWorldMatrix();

	//do nothing if nodes are already directly related
	if (pNode->GetParent() == pNewParent)
		return true;

	//do not allow parenting to child node
	{
		C_SceneNode *pCurrent = pNewParent;
		while (pCurrent)
		{
			if (pCurrent == pNode)
			{
				LOGERROR(C_Core::Instance().GetLogger(), "Cannot parent an object to one of its children [%s - %s]", 
					pNode->GetName().Chars(), pNewParent->GetName().Chars());
				return false;
			}
			pCurrent = pCurrent->GetParent();
		}
	}

	if (pNode->VetoParentChange(pNewParent))
		return false;

	//detach pNode from its siblings & parent
	if (pNode->m_pPrevious)
		pNode->m_pPrevious->m_pNext = pNode->m_pNext;

	if (pNode->m_pNext)
		pNode->m_pNext->m_pPrevious = pNode->m_pPrevious;

	if (pNode->m_pParent && pNode->m_pParent->m_pChild == pNode)
		pNode->m_pParent->m_pChild = pNode->m_pNext;

	pNode->m_pPrevious = 0;
	pNode->m_pNext = 0;
	pNode->m_pParent = 0;

	pNewParent->AddChild(pNode);

	if (absoluteTransform)
		pNode->SetWorldMatrix(world);
	else
	{
		C_ImmediateHintCollection hc(pNode);
		hc.hints.PushBack(new C_ImmediateTransformHint(true)); 
		pNode->ImmediatePropagate(hc);
	}

	return true;
}

bool C_SceneNodeHelpers::DeleteSceneNodeAndReparentChildren(
	C_SceneNode* pNode, bool updateChildMatrices, bool deleteOverride)
{
	ExtractSceneNodeAndReparentChildren(pNode, updateChildMatrices);
	return C_Core::Instance().GetDgManager().DestroyNode(pNode, deleteOverride);
}

bool C_SceneNodeHelpers::DeleteSceneNodeAndChildren(C_SceneNode* pNode, bool deleteOverride)
{
	if (!pNode || (!pNode->IsDeletable() && !deleteOverride))
	{
		assert(!"Invalid node to delete!");
		return false;
	}

	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(pNode->GetId());
	BIZ_ASSERT(pLevel);
	C_SceneNode* pRoot = pLevel->GetRootSceneNode();

	if (pNode == pRoot)
	{
		assert(!"Cannot delete the root scene node");
		return false;
	}

	if (!pNode->m_pParent)
	{
		assert(!"Node must have a parent!");
	}

	//detach pNode from its siblings & parent
	if (pNode->m_pPrevious)
		pNode->m_pPrevious->m_pNext = pNode->m_pNext;

	if (pNode->m_pNext)
		pNode->m_pNext->m_pPrevious = pNode->m_pPrevious;

	if (pNode->m_pParent && pNode->m_pParent->m_pChild == pNode)
		pNode->m_pParent->m_pChild = pNode->m_pNext;

	while (pNode->m_pChild)
		_DeleteSceneNodeAndChildrenRecurse(pNode->m_pChild, deleteOverride);


	//Delete!
	if (!C_Core::Instance().GetDgManager().DestroyNode(pNode, deleteOverride))
		return false;
	
	return true;
}

void C_SceneNodeHelpers::_DeleteSceneNodeAndChildrenRecurse(C_SceneNode* pNode, bool deleteOverride)
{
	while (pNode->m_pChild)
		_DeleteSceneNodeAndChildrenRecurse(pNode->m_pChild, deleteOverride);

	if (pNode->m_pParent && pNode->m_pParent->m_pChild == pNode)
		pNode->m_pParent->m_pChild = pNode->m_pNext;

	if (pNode->m_pPrevious)
		pNode->m_pPrevious->m_pNext = pNode->m_pNext;

	if (pNode->m_pNext)
		pNode->m_pNext->m_pPrevious = pNode->m_pPrevious;

	C_Core::Instance().GetDgManager().DestroyNode(pNode, deleteOverride);
}

U32 C_SceneNodeHelpers::GetDepth(C_SceneNode* pNode)
{
	U32 depth = 0;
	while(pNode->GetParent())
	{
		depth++;
		pNode = pNode->GetParent();
	}

	return depth;
}

}
