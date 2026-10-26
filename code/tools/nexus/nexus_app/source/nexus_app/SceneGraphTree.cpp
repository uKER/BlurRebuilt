#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include <nexus_app/MainFrm.h>

#include "../../resource.h"
#include <nexus_app/NexusSystem.h>
#include <nexus_app/NexusDoc.h>
#include <nexus_app/SceneGraphTree.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Nexus;

BEGIN_MESSAGE_MAP(SceneGraphTree, CTreeCtrlEx)
	ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelectionChanged)
END_MESSAGE_MAP()


bool SceneGraphTree::Create(CWnd* pParentWnd)
{
	//A bug in MFC (2005) causes ModifyStyle to return failure when the existing style matches the desired style.
	//Therefore we cannot check for failure >:(

	CreateEx(
		0,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP |
		TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT |
		TVS_SHOWSELALWAYS | TVS_TRACKSELECT,
		CRect(0,0,0,0),
		pParentWnd,
		pParentWnd->GetDlgCtrlID());
/*
	if (!CreateEx(
		0,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP |
		TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT |
		TVS_SHOWSELALWAYS | TVS_TRACKSELECT,
		CRect(0,0,0,0),
		pParentWnd,
		pParentWnd->GetDlgCtrlID()))
		return false;
*/
	return true;
}

void SceneGraphTree::Destroy()
{
}

void SceneGraphTree::ClearSelection()
{
	CTreeCtrlEx::ClearSelection();
}

void SceneGraphTree::Clear()
{
	DeleteAllItems();
	m_treeCtrlAftkMapping.clear();
	m_aftkTreeCtrlMapping.clear();
}

void SceneGraphTree::Update()
{
	SetRedraw(FALSE);
	Clear();

	const Aftk::C_Scene* pAftkScene = Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr();
	HTREEITEM root = 0;
	if (pAftkScene->GetRootSceneNodePtr())
		root = Update(pAftkScene->GetRootSceneNodePtr(), TVI_ROOT);

	SetRedraw(TRUE);
	if (root)
		Expand(root, TVE_EXPAND);
}

// This is used to test for duplicate TVN_SELCHANGED notifications from CTreeCtrlEx
BOOL SceneGraphTree::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UINT numItems = GetSelectedCount();

	Nexus::g_pNexusSystem->GetCorePtr()->DeselectAll();
	if (numItems > 0)
	{
		HTREEITEM selectedItem = GetFirstSelectedItem();
		U32 aftkUniqueId;
		Aftk::C_SceneNode* pSceneNode;
		{
			std::map<HTREEITEM, U32>::iterator it;
			it = m_treeCtrlAftkMapping.find(selectedItem);
			if (it == m_treeCtrlAftkMapping.end()) 
			{
				assert(false);
				return CTreeCtrlEx::OnSelchanged(pNMHDR, pResult);
			}
			aftkUniqueId = it->second;
			pSceneNode = 
				Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr()->GetSceneNodePtrByUniqueID(aftkUniqueId);
			Nexus::g_pNexusSystem->GetCorePtr()->Select(
				Nexus::g_pNexusSystem->GetState().selectionMode,
				pSceneNode);
		}
		for (UINT iItem = 0; iItem < numItems-1; iItem++)
		{
			selectedItem = GetNextSelectedItem(selectedItem);
			{
				std::map<HTREEITEM, U32>::iterator it;
				it = m_treeCtrlAftkMapping.find(selectedItem);
				if (it == m_treeCtrlAftkMapping.end()) 
				{
					assert(false);
					return CTreeCtrlEx::OnSelchanged(pNMHDR, pResult);
				}
				aftkUniqueId = it->second;
				pSceneNode = 
					Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr()->GetSceneNodePtrByUniqueID(aftkUniqueId);
				Nexus::g_pNexusSystem->GetCorePtr()->Select(
					Nexus::g_pNexusSystem->GetState().selectionMode,
					pSceneNode);
			}
		}
	}

	Nexus::g_pNexusSystem->UpdateSceneNodeSelection();
	g_pDocument->UpdateAllViews(0);
	if (pResult)
	{
		*pResult = 0;
	}
	return CTreeCtrlEx::OnSelchanged(pNMHDR, pResult);
}

void SceneGraphTree::ToggleSelection(const Aftk::C_SceneNode* pAftkSceneNode)
{
	std::map<U32, HTREEITEM>::iterator it;
	it = m_aftkTreeCtrlMapping.find(pAftkSceneNode->GetUniqueId());
	if (it == m_aftkTreeCtrlMapping.end())
		return;

	HTREEITEM treeItem = it->second;

	UINT oldState = GetItemState(treeItem, TVIS_SELECTED);
	if (oldState != 0)
        SetItemState(treeItem, 0, TVIS_SELECTED);
	else
	{
		SetItemState(treeItem, TVIS_SELECTED, TVIS_SELECTED);
		HTREEITEM parent = GetParentItem(treeItem);
		while (parent)
		{
			Expand(parent, TVE_EXPAND);
			parent = GetParentItem(parent);
		}
	}
}

HTREEITEM SceneGraphTree::Update(const Aftk::C_SceneNode* pAftkSceneNode, HTREEITEM hParent)
{
	HTREEITEM hNode = InsertItem(pAftkSceneNode->GetName().c_str(),hParent, TVI_LAST);
	m_treeCtrlAftkMapping[hNode] = (U32)pAftkSceneNode->GetUniqueId();
	m_aftkTreeCtrlMapping[(U32)pAftkSceneNode->GetUniqueId()] = hNode;

	const Aftk::C_SceneNode* pChild = pAftkSceneNode->GetChildPtr();
	while(pChild)
	{
		Update(pChild, hNode);
		pChild = pChild->GetNextPtr();
	}

	return hNode;
}
