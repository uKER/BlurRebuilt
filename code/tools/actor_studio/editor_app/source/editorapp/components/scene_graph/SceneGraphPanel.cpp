#include "stdafx.h"

#include <wx/regex.h>
#include <wx/clipbrd.h>
#include <wx/Dir.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/Hints/HintCollection.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/StringUtil.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Components/SplitterControl.h>
#include <EditorApp/Components/LevelSelectDialog.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphPanel.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphMenuBar.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphFilter.h>

// Temp
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderViewport.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

#include <EditorApp/App.h>
#include <EditorApp/Config.h>

#include <algorithm>

using namespace Editor;

const char* C_SceneGraphPanel::m_pTreeIconStringTable[] =
{
#define DEF(e, str) str,
	#include <EditorApp/Components/Scene_Graph/TreeIconMapping.h>
#undef DEF
};

//NOTE: this is slow but we shouldn't need to call this often...
//'recurse' triggers a depth first search, otherwise it'll just 
//iterate through items at the 'subtree'child level
TreeItem* FindNode(TreeCtrl *pTree, const char* pName, TreeItem* subtree=0, bool recurse=false)
{
	Ut::UtVector< Ut::DynamicVector<TreeItem*> > stack;

	//depth first search into tree
	TreeItem* root = subtree ? subtree : pTree->GetRootNode();
	if (!root)
		return 0;

	TreeItem* child = root->GetChild(0);
	if (child)
		stack.PushBack(child);

	while (stack.Size() > 0)
	{
		child = stack.Back();
		if (WcToMb(child->GetLabel()) == pName)
			return child;

		if (recurse)
			child = child->GetChild(0);
		else
			child = 0;

		if (child)
			stack.PushBack(child);
		else
		{
			while (stack.Size() && !child)
			{
				child = stack.Back();
				stack.PopBack();
				child = child->GetNextSibling();

				if (child)
					stack.PushBack(child);
			}
		}
	}

	return 0;
}

class SceneGraphWrapper : public wxPanel
{
public:
	SceneGraphWrapper(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
	: wxPanel(parent, winid, pos, size, style, name)
	{
	}

	bool MSWShouldPreProcessMessage(WXMSG* msg)
	{
		return __super::MSWShouldPreProcessMessage(msg);
	}

	bool MSWTranslateMessage(WXMSG* msg)
	{
		if (msg->message == WM_KEYDOWN)
		{
			if (wxWindow::MSWTranslateMessage(msg))
				return true;
			else
			{
				//send a new event to parent panel.
				//This is because ::IsDialog() will dispatch messages to the underlying ScarabForm
				//so we'll never see it again in wxWidgets land
				::SendMessage((HWND)GetHWND(), msg->message, msg->wParam, msg->lParam);
				return false;
			}
		}
		else
			return __super::MSWTranslateMessage(msg);
	}
};

IMPLEMENT_CLASS(C_SceneGraphPanel, wxPanel);

BEGIN_EVENT_TABLE(C_SceneGraphPanel,wxPanel)
END_EVENT_TABLE()

long C_SceneTreeItemDgNode::itemCount = 0;

C_SceneGraphPanel::C_SceneGraphPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
wxPanel(parent, id, pos, size, style),
m_pMenuBar(0),
m_slots(this)
{
	m_pSceneFilter = new C_SceneGraphFilter();

	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );
	SetSizer(pSizer0);

	m_pMenuBar = new C_SceneGraphMenuBar(this,m_pSceneFilter);
	m_pMenuBar->InitialiseUI();

	m_pTextCtrl = new wxTextCtrl(
		this,
		wxID_ANY,
		wxEmptyString,
		wxDefaultPosition,
		wxDefaultSize
		);
	
	pSizer0->Add( m_pTextCtrl, 0, wxALL|wxEXPAND, 0 );
	
	m_pSplitter = new wxSplitterCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxSP_SASH_BORDERS);
	
	m_pNodeTreeCtrlPanel = new SceneGraphWrapper(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	m_pPropertyTreeCtrlPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	
	m_pNodeTreeCtrl = new C_SceneTreeCtrl(this);
	m_pNodeTreeCtrl->Create((HWND)m_pNodeTreeCtrlPanel->GetHandle());
	m_pNodeTreeCtrl->GetFlags().HasIcons = true;

	m_pPropertyTreeCtrl = new C_SceneTreeCtrl(this);
	m_pPropertyTreeCtrl->Create((HWND)m_pPropertyTreeCtrlPanel->GetHandle());
	m_pPropertyTreeCtrl->GetFlags().HasIcons = true;

	m_pNodeTreeCtrl->SetSceneFilter(m_pSceneFilter);

	m_pSplitter->Split(E_SplitterLayout_2Horizontal, m_pNodeTreeCtrlPanel, m_pPropertyTreeCtrlPanel);
	m_pSplitter->SetSplitterProportion(7500);

	pSizer0->Add(m_pSplitter, 1, wxALL|wxEXPAND, 0);

	InitialiseActorRoot();
	InitialisePropertyRoot();

	Layout();

	{
		WinRect rcClient((HWND)m_pNodeTreeCtrlPanel->GetHandle());
		SetWindowPos(*m_pNodeTreeCtrl, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}

	{
		WinRect rcClient((HWND)m_pPropertyTreeCtrlPanel->GetHandle());
		SetWindowPos(*m_pPropertyTreeCtrl, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}

	for (U32 i=0; i<E_TreeViewNode_Count; ++i)
	{
		m_pNodeTreeCtrl->LoadIcon(m_pTreeIconStringTable[i], i);
		m_pPropertyTreeCtrl->LoadIcon(m_pTreeIconStringTable[i], i);
	}

	PopulateFromExistingData();

	m_pNodeTreeCtrlPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_SceneGraphPanel::OnSize), NULL, this);
	m_pPropertyTreeCtrlPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_SceneGraphPanel::OnSize), NULL, this);

	m_pTextCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxTextEventHandler(C_SceneGraphPanel::OnFilterTextUpdate), NULL, this);
	this->Connect(wxID_ANY, wxEVT_IDLE,wxIdleEventHandler(C_SceneGraphPanel::OnIdle));

	m_slots.Connect();

	m_Flags.pendingDeletion = false;
	m_Flags.pendingSync = false;
	m_Flags.filterDgDirty = false;
	m_Flags.filterRefDirty = false;
	m_Flags.filterDirty = false;
	m_Flags.dragging = false;
	m_Flags.autoSelect = false;
}


C_SceneGraphPanel::~C_SceneGraphPanel()
{
	SAFE_DELETE_PTR(m_pNodeTreeCtrlPanel);
	SAFE_DELETE_PTR(m_pPropertyTreeCtrlPanel);

	SAFE_DELETE_PTR(m_pSceneFilter);
	SAFE_DELETE_PTR(m_pMenuBar);
}

void C_SceneGraphPanel::Shutdown()
{
	m_pNodeTreeCtrlPanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_SceneGraphPanel::OnSize), NULL, this);
	m_pPropertyTreeCtrlPanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_SceneGraphPanel::OnSize), NULL, this);
	m_pTextCtrl->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxTextEventHandler(C_SceneGraphPanel::OnFilterTextUpdate), NULL, this);
	this->Disconnect(wxID_ANY, wxEVT_IDLE,wxIdleEventHandler(C_SceneGraphPanel::OnIdle));

	SAFE_DELETE_PTR(m_pNodeTreeCtrl);
	SAFE_DELETE_PTR(m_pPropertyTreeCtrl);

	m_slots.Disconnect();
}

void C_SceneGraphPanel::InitialiseActorRoot()
{
	TreeItem* root = new C_SceneTreeItem();
	root->SetLabel("Root");
	m_pNodeTreeCtrl->SetRootNode(root);

	TreeItem* child;
	
	child = new C_SceneTreeItem();
	child->SetLabel("Scene Nodes");
	child->SetExpanded(true);
	child->SetIcon(E_TreeViewNode_ActorRoot);
	root->AddChild(child);

	child = new C_SceneTreeItem();
	child->SetLabel("Reference Scenes");
	child->SetExpanded(false);
	child->SetIcon(E_TreeViewNode_ReferenceScenes);
	root->AddChild(child);

	child = new C_SceneTreeItem();
	child->SetLabel("Game Data");
	child->SetExpanded(true);
	child->SetIcon(E_TreeViewNode_ReferenceScenes);
	root->AddChild(child);

	child = new C_SceneTreeItem();
	child->SetLabel("Null");
	child->SetHidden(true);
	root->AddChild(child);
}

void C_SceneGraphPanel::InitialisePropertyRoot()
{
	TreeItem* root = new C_SceneTreeItem();
	root->SetLabel("Root");
	m_pPropertyTreeCtrl->SetRootNode(root);

	TreeItem* child;

	child = new C_SceneTreeItem();
	child->SetLabel("Null");
	child->SetHidden(true);
	root->AddChild(child);
}

void C_SceneGraphPanel::OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	if (evt == EditorLib::E_DatabaseEvent_Opened)
	{
		//load project specific actor icons
		wxString iconPath = Btl::String(wxGetApp().GetConfig().configPath + "\\icons").Chars();
		if (wxDir::Exists(iconPath))
		{
			Btl::Vector<const EditorLib::I_XtType*> typeList;
			EditorLib::C_Core::Instance().GetXtTypeDatabase().BuildTypeList(typeList);

			for (S32 i=0; i<typeList.Size(); ++i)
			{
				if (typeList[i]->IsA(EditorLib::E_XtType_SceneXtActorNode) ||
					typeList[i]->IsA(EditorLib::E_XtType_SceneXtNode))
				{
					wxArrayString fileMatches;

					//search for files with the same name in xt
					wxString fileSpec = typeList[i]->GetXtType()->name;
					fileSpec += ".*";

					if (wxDir::GetAllFiles(iconPath, &fileMatches, fileSpec))
					{
						if (fileMatches.size() > 1)
							LOGWARN(g_Log, "Multiple icons found for %s", typeList[i]->GetXtType()->name.Text());

						int iconId = m_pNodeTreeCtrl->LoadIcon(fileMatches[0].c_str());
						m_pPropertyTreeCtrl->LoadIcon(fileMatches[0].c_str(), iconId);
						m_nodeIconMap.Insert(typeList[i]->GetXtType()->name.Text(), iconId);
					}
				}
			}
		}
	}
	else if (evt == EditorLib::E_DatabaseEvent_Closing)
	{
		for (NodeIconMap::Iterator it = m_nodeIconMap.Begin(); it != m_nodeIconMap.End(); ++it)
		{
			m_pNodeTreeCtrl->RemoveIcon(it->GetValue());
			m_pPropertyTreeCtrl->RemoveIcon(it->GetValue());
		}
		m_nodeIconMap.Clear();
	}
}

void C_SceneGraphPanel::OnLevelNameChanged(const EditorLib::I_Level& level)
{
	SceneNodeDataMap::Iterator it = m_NodeDataMap.Find(level.GetRootSceneNode()->GetId());
	assert(it != m_NodeDataMap.End());

	it->GetValue()->SetLabel(level.GetName().Chars());
}

void C_SceneGraphPanel::OnLevelOpening(const EditorLib::I_Level& level)
{
	TreeItem* parent = FindNode(m_pNodeTreeCtrl, "Scene Nodes");
	BIZ_ASSERT(parent);

	TreeItem* levelItem = new C_SceneTreeItemDgNode(this, level.GetRootSceneNode(), &m_NodeDataMap, E_SceneGraphDataType_Level);
	levelItem->SetExpanded(true);
	levelItem->SetIcon(E_TreeViewNode_ActorList);
	parent->AddChild(levelItem);

	m_Flags.pendingSync = true;
}

void C_SceneGraphPanel::OnLevelOpened(const EditorLib::I_Level& level)
{
	SceneNodeDataMap::Iterator it = m_NodeDataMap.Find(level.GetRootSceneNode()->GetId());
	assert(it != m_NodeDataMap.End());

	it->GetValue()->SetLabel(level.GetName().Chars());
	it->GetValue()->SetLevelReadOnly(level.IsReadOnly());

	assert(m_Flags.pendingSync);
	m_Flags.pendingSync = false;
}

void C_SceneGraphPanel::OnLevelClosing(const EditorLib::I_Level& level)
{
	m_Flags.pendingSync = true;
}

void C_SceneGraphPanel::OnLevelClosed(const EditorLib::I_Level& level)
{
	assert(m_Flags.pendingSync);
	m_Flags.pendingSync = false;
}

void C_SceneGraphPanel::OnLevelAftkRefAdded(const Btl::String& path)
{
	//find static "References" node on first level
	TreeItem* parent = FindNode(m_pNodeTreeCtrl, "Reference Scenes");
	if (parent)
	{
		C_SceneTreeItem* item = new C_SceneTreeItem();
		item->SetLabel(path.Chars());
		item->SetIcon(E_TreeViewNode_ReferenceScene_Unloaded);
		parent->AddChild(item);
		
		ReferenceRootMap::InsertionReturnType insert = m_referenceRootMap.Insert(path, item);
		m_Flags.filterRefDirty = true;
	}
}

void C_SceneGraphPanel::OnLevelAftkRefRemoved(const Btl::String& path)
{
	ReferenceRootMap::Iterator it = m_referenceRootMap.Find(path);
	if (it != m_referenceRootMap.End())
	{
		delete it->GetValue();
		m_referenceRootMap.Erase(it);
		m_Flags.filterRefDirty = true;
	}
}

void C_SceneGraphPanel::OnAftkRefLoaded(
	const Btl::String& path, 
	const Aftk::C_Scene& aftkScene, 
	const EditorLib::C_SceneNode& sceneNode,
	const EditorLib::AftkDgMap& aftkDgMap)
{
	ReferenceRootMap::Iterator it = m_referenceRootMap.Find(path);
	if (it == m_referenceRootMap.End())
	{
		//we've not recieved a 'levelaftkrefadded' signal (must be auto-included for visualisation)
		OnLevelAftkRefAdded(path);
		it = m_referenceRootMap.Find(path);
		assert(it != m_referenceRootMap.End());
	}

	AddAftkRef(it->GetValue(), sceneNode, aftkDgMap);

	m_Flags.filterRefDirty = true;
}

void C_SceneGraphPanel::AddAftkRef(
	C_SceneTreeItem* parent,
	const EditorLib::C_SceneNode& sceneNode,
	const EditorLib::AftkDgMap& aftkDgMap)
{
	C_SceneTreeItemDgNode* treeItem = new C_SceneTreeItemDgNode(this, &sceneNode, &m_NodeDataMap);
	treeItem->SetLabel(sceneNode.GetName().Chars());

	const EditorLib::C_SceneNode* child = sceneNode.GetChild();
	
	while (child)
	{
		AddAftkRef(treeItem, *child, aftkDgMap);
		child = child->GetNextSibling();
	}

	parent->AddChild(treeItem);
}

void C_SceneGraphPanel::OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	if (!dgNode.IsA(EditorLib::E_ObjType_SceneXtNodeBase) && 
		!dgNode.IsA(EditorLib::E_ObjType_DgXtGameDataNode) &&
		!dgNode.IsA(EditorLib::E_ObjType_DgXtCompNode) &&
		!dgNode.IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
		return;

	SceneNodeDataMap::Iterator it;

	bool nameDirty = false;
	bool refreshAll = false;
	
	if (pHc)
	{
		for (Btl::Vector<EditorLib::I_UpdateHint*>::ConstIterator hint = pHc->hints.Begin(); hint != pHc->hints.End(); ++hint)
		{
			if ((*hint)->GetType() == EditorLib::E_UpdateHint_Name)
			{
				nameDirty = true;
			}
			else
			{
				// Unknown hint, could be anything, refresh all
				refreshAll = true;
			}
		}
	} else {
		refreshAll = true;
	}

	if (nameDirty || refreshAll)
	{
		UpdateDgNodeName(dgNode);
	}

	if (!refreshAll)
		return;

	if (dgNode.IsA(EditorLib::E_ObjType_SceneXtNodeBase))
	{
		C_SceneTreeItemDgNode *pNodeData;

		it = m_NodeDataMap.Find(dgNode.GetId());
		if (it == m_NodeDataMap.End())
		{
			pNodeData = new C_SceneTreeItemDgNode(this, &dgNode, &m_NodeDataMap);
			FindNode(m_pNodeTreeCtrl, "Scene Nodes")->AddChild(pNodeData);
			UpdateDgNodeName(dgNode);
		} else {
			pNodeData = it->GetValue();
		}

		UpdateDgNodeIcon(dgNode);
	
		// Parent node to correct tree item
		const EditorLib::C_SceneNode *pSceneNode = static_cast<const EditorLib::C_SceneNode*>(&dgNode);
		const EditorLib::C_SceneNode *pParent = pSceneNode->GetParent();

		TreeItem *pParentData;

		///// TODO : REMOVE : DEBUG INFO
		// Btl::String nodeName = dgNode.GetName();
		// Btl::String parentName = pParent->GetName();
		// LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Parent %s to %s", nodeName.Chars(), parentName.Chars());
		///// END DEBUG INFO

		it = m_NodeDataMap.Find(pParent->GetId());
		if (it != m_NodeDataMap.End())
			pParentData = it->GetValue();	
		else
			pParentData = FindNode(m_pNodeTreeCtrl, "Scene Nodes");
		
		if (pNodeData->GetParent() != pParentData)
		{
			pNodeData->SetParent(pParentData);
			m_Flags.filterDgDirty = true;
		}
	}
	else if (dgNode.IsA(EditorLib::E_ObjType_DgXtGameDataNode))
	{
		C_SceneTreeItemDgNode *pNodeData;

		it = m_NodeDataMap.Find(dgNode.GetId());
		if (it == m_NodeDataMap.End())
		{
			pNodeData = new C_SceneTreeItemDgNode(this, &dgNode, &m_NodeDataMap);
			FindNode(m_pNodeTreeCtrl, "Game Data")->AddChild(pNodeData);
			UpdateDgNodeName(dgNode);
		} else {
			pNodeData = it->GetValue();
		}
	}
	else if (dgNode.IsA(EditorLib::E_ObjType_DgXtCompNode))
	{
		const EditorLib::C_DgXtCompNode *pComponent = static_cast<const EditorLib::C_DgXtCompNode*>(&dgNode);
		const EditorLib::I_DgNode *pParent = pComponent->GetParent();
		SceneNodeDataMap::Iterator itProp = m_PropertyDataMap.Find(dgNode.GetId());
		
		C_SceneTreeItemDgNode *pNodeData;

		if (itProp == m_PropertyDataMap.End())
		{
			pNodeData = new C_SceneTreeItemDgNode(this, &dgNode, &m_PropertyDataMap);
			FindNode(m_pPropertyTreeCtrl, "Null")->AddChild(pNodeData);
			UpdateDgNodeName(dgNode);
		} else {
			pNodeData = itProp->GetValue();
		}

		TreeItem *pParentData;

		it = m_PropertyDataMap.Find(pParent->GetId());
		if (it != m_PropertyDataMap.End())
		{
			pParentData = it->GetValue();
			pNodeData->SetParent(pParentData);
		}
	}
	else if (dgNode.IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
	{
		const EditorLib::C_DgXtPropNodeBase *pProp = static_cast<const EditorLib::C_DgXtPropNodeBase*>(&dgNode);
		const EditorLib::C_DgXtCompNode *pParent = pProp->GetParent();
		
		C_SceneTreeItemDgNode *pNodeData;

		SceneNodeDataMap::Iterator itProp = m_PropertyDataMap.Find(dgNode.GetId());
		if (itProp == m_PropertyDataMap.End())
		{
			pNodeData = new C_SceneTreeItemDgNode(this, &dgNode, &m_PropertyDataMap);
			FindNode(m_pPropertyTreeCtrl, "Null")->AddChild(pNodeData);
			UpdateDgNodeName(dgNode);
		} else {
			pNodeData = itProp->GetValue();
		}

		if (!pParent)
			return;

		TreeItem *pParentData;

		it = m_PropertyDataMap.Find(pParent->GetId());
		if (it != m_PropertyDataMap.End())
		{
			pParentData = it->GetValue();
			pNodeData->SetParent(pParentData);
		}		
	}

	/*
	{
		//TODO: revisit this...
		wxColour colour = *wxBLACK;
		EditorLib::C_Actor* pActor = EditorLib::XtNodeHelpers::ActorCast(const_cast<EditorLib::I_DgNode*>(&dgNode));
		if (pActor)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(pActor->GetXtData());
			assert(pComponent);

			XtString visible = Xt::AsText(Xt::At(&pComponent->data, "__visible"));
			if (visible == "false")
				colour = ::wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
		}
		m_pNodeTreeCtrl->SetItemTextColour(id, colour);
	}
	*/
}

void C_SceneGraphPanel::UpdateDgNodeName(const EditorLib::I_DgNode& dgNode)
{
	SceneNodeDataMap::Iterator it;

	it = m_NodeDataMap.Find(dgNode.GetId());
	if (it != m_NodeDataMap.End())
	{
		if (strcmp(dgNode.GetName().Chars(), WcToMb(it->GetValue()->GetLabel()).Text()))
		{
			it->GetValue()->SetLabel(dgNode.GetName().Chars());
			m_Flags.filterDgDirty = true;
		}
	}

	it = m_PropertyDataMap.Find(dgNode.GetId());
	if (it != m_PropertyDataMap.End())
	{
		if (strcmp(dgNode.GetName().Chars(), WcToMb(it->GetValue()->GetLabel()).Text()))
		{
			it->GetValue()->SetLabel(dgNode.GetName().Chars());
			m_Flags.filterDgDirty = true;
		}
	}
}

void C_SceneGraphPanel::UpdateDgNodeIcon(const EditorLib::I_DgNode& dgNode)
{
	// Refresh icon in case of type changes
	int icon = E_TreeViewNode_ActorNode;
	const EditorLib::C_XtObject *pXtObj = dgNode.AsXtObject();
	if (pXtObj && pXtObj->GetXtType())
	{
		NodeIconMap::Iterator it = 
			m_nodeIconMap.Find(pXtObj->GetXtType()->name.Text());
		if (it != m_nodeIconMap.End())
			icon = it->GetValue();
	}

	SceneNodeDataMap::Iterator it;

	it = m_NodeDataMap.Find(dgNode.GetId());
	if (it != m_NodeDataMap.End())
		it->GetValue()->SetIcon(icon);

	it = m_PropertyDataMap.Find(dgNode.GetId());
	if (it != m_PropertyDataMap.End())
		it->GetValue()->SetIcon(icon);
}

void C_SceneGraphPanel::OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
	m_Flags.pendingDeletion = true;

	SceneNodeDataMap::Iterator it;
	
	it = m_NodeDataMap.Find(dgNode.GetId());
	if (it != m_NodeDataMap.End())
	{
		C_SceneTreeItemDgNode* pData = it->GetValue();

		// Reparent all children to 'null' so they aren't removed when we undo a group creation
		TreeItem* nullItem = FindNode(m_pNodeTreeCtrl, "Null");
		TreeItem* child = pData->GetFirstChild();
		while (child)
		{
			TreeItem* oldChild = child;
			child = child->GetNextSibling();
			oldChild->SetParent(nullItem);
		}

		delete pData;
	}

	it = m_PropertyDataMap.Find(dgNode.GetId());
	if (it != m_PropertyDataMap.End())
	{
		C_SceneTreeItemDgNode* pData = it->GetValue();
		delete pData;
	}

	m_Flags.pendingDeletion = false;
}

bool C_SceneGraphPanel::BuildObjIdStrFromSelection(Btl::String& objIdStr)
{
	return false;
}

void C_SceneGraphPanel::OnIdle(wxIdleEvent &e)
{
	if (m_Flags.pendingSync)
		return;

	PurgeReparentQueue();

	if (m_Flags.filterDgDirty || m_Flags.filterRefDirty || m_Flags.filterDirty)
		UpdateFilter();

	m_Flags.filterDgDirty = false;
	m_Flags.filterRefDirty = false;
	m_Flags.filterDirty = false;

	OnGraphIdle();
}

void C_SceneGraphPanel::OnSize(wxSizeEvent& e)
{
	if (!m_pNodeTreeCtrlPanel || !m_pNodeTreeCtrl)
		return;

	{
		WinRect rcClient((HWND)m_pNodeTreeCtrlPanel->GetHandle());
		SetWindowPos(*m_pNodeTreeCtrl, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
	
	{
		WinRect rcClient((HWND)m_pPropertyTreeCtrlPanel->GetHandle());
		SetWindowPos(*m_pPropertyTreeCtrl, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}

void C_SceneGraphPanel::OnFilterTextUpdate(wxCommandEvent& e)
{
	m_pSceneFilter->SetTextFilter(m_pTextCtrl->GetValue().ToAscii());
	m_Flags.filterDirty = true;
}

void C_SceneGraphPanel::Freeze()
{
	m_pNodeTreeCtrl->Freeze();
	m_pPropertyTreeCtrl->Freeze();
}

void C_SceneGraphPanel::Thaw()
{
	m_pNodeTreeCtrl->Thaw();
	m_pPropertyTreeCtrl->Thaw();
}

void C_SceneGraphPanel::FrameSelection()
{
	if (m_Flags.pendingSync)
		return;

	Freeze();
	{
		const XtUt::Array<TreeItem*>& selection = m_pNodeTreeCtrl->GetSelection();
		for (size_t i=0; i<selection.count; i++)
			m_pNodeTreeCtrl->EnsureVisible(selection[i]);
	}
	Thaw();
}

void C_SceneGraphPanel::OnFilterMenuUpdate(void)
{
	m_Flags.filterDirty = true;
}

bool C_SceneGraphPanel::FilterItem(C_SceneTreeItemDgNode* sceneGraphData)
{
	bool childFound = false;

	for (int i=0; i<sceneGraphData->GetNumChildren(); i++)
	{
		if (FilterItem(((Editor::C_SceneTreeItemDgNode*)sceneGraphData->GetChild(i))))
			childFound = true;
	}

	if (!childFound)
	{
		if (!m_pSceneFilter->MatchDgNode(sceneGraphData->GetDataType(), sceneGraphData->GetDgNode()))
		{
			sceneGraphData->SetHidden(true);
			return false;
		}

		if (!m_pSceneFilter->MatchText(sceneGraphData->GetDgNode()->GetName()))
		{
			sceneGraphData->SetHidden(true);
			return false;
		}
	}

	sceneGraphData->SetHidden(false);
	return true;
}

void C_SceneGraphPanel::UpdateFilter()
{
	Freeze();

	for (SceneNodeDataMap::Iterator it=m_NodeDataMap.Begin(); it!=m_NodeDataMap.End(); ++it)
	{
		C_SceneTreeItemDgNode* pData= it->GetValue();
		if (((C_SceneTreeItem*)pData->GetParent())->GetType() == E_SceneGraphItemType_Generic)
			FilterItem(pData);
	}

	{
		// Sort all 2nd level and deeper nodes
		TreeItem* item = m_pNodeTreeCtrl->GetRootNode();
		assert(item);
		for(int i=0; i<item->GetNumChildren(); i++)
		{
			TreeItem* child = item->GetChild(i);
			Sz8 itemLabel = WcToMb(child->GetLabel());
			const char* itemLabelChars = itemLabel.Text();
			if ((m_Flags.filterDirty || m_Flags.filterDgDirty) && (child->GetLabel() == L"Scene Nodes" || child->GetLabel() == L"Game Data"))
				child->SortChildren(true);
			if ((m_Flags.filterDirty || m_Flags.filterRefDirty) && child->GetLabel() == L"Reference Scenes")
				child->SortChildren(true);
		}
	}

	Thaw();

	m_Flags.filterChanged = true;
}

void C_SceneGraphPanel::QueueReparent(TreeItem* item, TreeItem* parent)
{
	m_ReparentQueue[parent].PushBack(item);
}

void C_SceneGraphPanel::PurgeReparentQueue()
{
	for (Btl::OpenHashMap<TreeItem*, Btl::Vector<TreeItem*> >::Iterator queue=m_ReparentQueue.Begin(); queue != m_ReparentQueue.End(); ++queue)
	{
		Btl::String nodeIdString;

		for (Btl::Vector<TreeItem*>::Iterator item = queue->GetValue().Begin(); item != queue->GetValue().End(); ++item)
		{
			const EditorLib::I_DgNode* dgNode = ((C_SceneTreeItemDgNode*)*item)->GetDgNode();

			if (!dgNode)
				continue;

			nodeIdString += dgNode->GetId().AsString() + ",";
		}

		const EditorLib::I_DgNode* parentDgNode;

		if (((C_SceneTreeItem*)queue->GetKey())->GetType() == E_SceneGraphItemType_Generic && ((C_SceneTreeItem*)queue->GetKey())->GetLabel() == L"Scene Nodes")
		{
			parentDgNode = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetRootSceneNode();
		}
		else if (((C_SceneTreeItem*)queue->GetKey())->GetType() == E_SceneGraphItemType_DgNode)
		{
			parentDgNode = ((C_SceneTreeItemDgNode*)queue->GetKey())->GetDgNode();
		}
		else
		{
			continue;
		}

		if (nodeIdString.Length() == 0)
			continue;

		if (!parentDgNode)
			continue;

		nodeIdString = nodeIdString(0, nodeIdString.Length()-1);
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("parent", nodeIdString + " -p " + parentDgNode->GetId().AsString());
	}

	m_ReparentQueue.Clear();
}

bool C_SceneGraphPanel::OnSelectionChanging(TreeItem* item, bool selected)
{
	if (((C_SceneTreeItem*)item)->GetType() != E_SceneGraphItemType_DgNode)
		return true;

	if (((C_SceneTreeItemDgNode*)item)->GetDataType() == E_SceneGraphDataType_Level)
		return false;

	return true;
}

void C_SceneGraphPanel::OnCoreEnableViews(bool enable)
{
	if (enable == false)
		Freeze();
	else if (enable == true)
		Thaw();
}

void C_SceneGraphPanel::OnCoreSelectionChanged()
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	if (selection.GetItems().Size())
	{
		const EditorLib::C_Selection::C_Item& item = selection.GetItems()[selection.GetItems().Size()-1];
		
		EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);

		if (pNode)
		{
			SetPropertyNode(*pNode);
			return;
		}
	}
	
	InitialisePropertyRoot();
}

void C_SceneGraphPanel::SetPropertyNode(const EditorLib::I_DgNode& dgNode)
{
	InitialisePropertyRoot();

	if (dgNode.IsA(EditorLib::E_ObjType_SceneXtNodeBase))
	{
		SetPropertyActor(dgNode);
	}
	else if (dgNode.IsA(EditorLib::E_ObjType_DgXtCompNode))
	{
		const EditorLib::C_DgXtCompNode *pComponent = static_cast<const EditorLib::C_DgXtCompNode*>(&dgNode);
		const EditorLib::I_DgNode *pParent = pComponent->GetParent();

		if (!pParent)
			return;

		SetPropertyActor(*pParent);
	}
	else if (dgNode.IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
	{
		const EditorLib::C_DgXtPropNodeBase *pProp = static_cast<const EditorLib::C_DgXtPropNodeBase*>(&dgNode);
		const EditorLib::C_DgXtCompNode *pComponent = pProp->GetParent();

		if (!pComponent)
			return;

		const EditorLib::I_DgNode *pParent = pComponent->GetParent();

		if (!pParent)
			return;

		SetPropertyActor(*pParent);
	}
}

void C_SceneGraphPanel::SetPropertyActor(const EditorLib::I_DgNode& actor)
{
	TreeItem* root = m_pPropertyTreeCtrl->GetRootNode();
	AddActor(root, actor);
	root->SetExpanded(true, true);
}

void C_SceneGraphPanel::AddActor(TreeItem* pTreeItem, const EditorLib::I_DgNode& actor)
{
	C_SceneTreeItemDgNode* pActorItem = new C_SceneTreeItemDgNode(this, &actor, &m_PropertyDataMap);
	pActorItem->SetLabel(actor.GetName().Chars());
	
	const EditorLib::C_SceneXtNodeBase* pActor = (EditorLib::C_SceneXtNodeBase*)&actor;
	const EditorLib::ConstDgActorComponentArray& components = pActor->GetComponents();

	for (EditorLib::ConstDgActorComponentArray::ConstIterator it=components.Begin(); it!=components.End(); ++it)
		AddComponent(pActorItem, **it);

	pTreeItem->AddChild(pActorItem);

	UpdateDgNodeIcon(actor);
}

void C_SceneGraphPanel::AddComponent(TreeItem* pTreeItem, const EditorLib::C_DgXtCompNode& component)
{
	C_SceneTreeItemDgNode* pComponentItem = new C_SceneTreeItemDgNode(this, &component, &m_PropertyDataMap);
	pComponentItem->SetLabel(component.GetName().Chars());
	
	const EditorLib::ConstDgPropArray& properties = component.GetProperties();

	for (EditorLib::ConstDgPropArray::ConstIterator it=properties.Begin(); it!=properties.End(); ++it)
	{
		if (*it)
			AddProperty(pComponentItem, **it);
	}

	pTreeItem->AddChild(pComponentItem);
}

void C_SceneGraphPanel::AddProperty(TreeItem* pTreeItem, const EditorLib::C_DgXtPropNodeBase& prop)
{
	C_SceneTreeItemDgNode* pPropertyItem = new C_SceneTreeItemDgNode(this, &prop, &m_PropertyDataMap);
	pPropertyItem->SetLabel(prop.GetName().Chars());
	pTreeItem->AddChild(pPropertyItem);
}

void C_SceneGraphPanel::PopulateFromExistingData()
{
	EditorLib::C_XtLevel* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();

	if (!pLevel)
		return;

	EditorLib::C_SceneNode* pRoot = pLevel->GetRootSceneNode();

	if (!pRoot)
		return;

	EditorLib::C_SceneNode* child = pRoot->GetChild();
	while (child)
	{
		PopulateFromExistingData(child);
		child = child->GetNextSibling();
	}
}

void C_SceneGraphPanel::PopulateFromExistingData(EditorLib::C_SceneNode* pSceneNode)
{
	if (!pSceneNode)
		return;

	OnDgNodeUpdated(*pSceneNode, 0);

	EditorLib::C_SceneNode* child = pSceneNode->GetChild();
	while (child)
	{
		PopulateFromExistingData(child);
		child = child->GetNextSibling();
	}
}

void C_SceneGraphPanel::ShowMenu(C_SceneTreeItem* item, const wxPoint& pt)
{
	enum E_MenuItems
	{
		E_MenuItem_CopyNodeName,
		E_MenuItem_RenameNode,
		E_MenuItem_ExportToLevel,
		E_MenuItem_UnparentNode,
		E_MenuItem_UnloadLevel,
		E_MenuItem_SetActiveLevel,
		E_MenuItem_SetHidden,
	};

	m_rightClickItem = item;

	if (item->GetType() != E_SceneGraphItemType_DgNode)
		return;

	C_SceneTreeItemDgNode* dgItem = (C_SceneTreeItemDgNode*)item;
	
	wxString title;
	wxMenu menu(WcToMb(item->GetLabel()).Text());
    
	if (dgItem->GetDataType() == E_SceneGraphDataType_Level)
	{
		menu.Append(E_MenuItem_UnloadLevel, "Unload Actor List");
		menu.Append(E_MenuItem_SetActiveLevel, "Set Level Active");
		menu.Append(wxID_SEPARATOR);
		wxMenuItem* hideNodesItem = menu.AppendCheckItem(E_MenuItem_SetHidden, "Hide Nodes");
		hideNodesItem->Check(EditorLib::C_Core::Instance().GetDgManager().LevelOf(dgItem->GetDgNode()->GetId())->IsHidden());
	} else {
		menu.Append(E_MenuItem_CopyNodeName, "Copy Node Name");
		menu.Append(E_MenuItem_RenameNode, "Rename");
		menu.Append(E_MenuItem_ExportToLevel, "Export to Actor List...");
		menu.Append(E_MenuItem_UnparentNode, "Unparent");
		menu.Append(wxID_SEPARATOR);
		menu.Append(E_MenuItem_SetActiveLevel, "Set Level Active");
	}

	Connect(E_MenuItem_SetActiveLevel, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnSetActiveLevel));
	Connect(E_MenuItem_UnloadLevel, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnUnloadLevel));
	Connect(E_MenuItem_CopyNodeName, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnCopyNodeName));
	Connect(E_MenuItem_RenameNode, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnRenameNode));
	Connect(E_MenuItem_ExportToLevel, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnExportToLevel));
	Connect(E_MenuItem_UnparentNode, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnUnparentNode));
	Connect(E_MenuItem_SetHidden, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_SceneGraphPanel::OnSetHidden));
	
	PopupMenu(&menu, pt);

	Disconnect(E_MenuItem_SetActiveLevel, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_UnloadLevel, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_CopyNodeName, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_RenameNode, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_ExportToLevel, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_UnparentNode, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(E_MenuItem_SetHidden, wxEVT_COMMAND_MENU_SELECTED);
}

C_SceneGraphPanel::C_Flags& C_SceneGraphPanel::GetFlags()
{
	return m_Flags;
}

void C_SceneGraphPanel::OnSetActiveLevel(wxCommandEvent& evt)
{
	if (((C_SceneTreeItem*)m_rightClickItem)->GetType() != E_SceneGraphItemType_DgNode)
		return;

	C_SceneTreeItemDgNode* levelItem = (C_SceneTreeItemDgNode*)m_rightClickItem;

	EditorLib::I_Level* level = EditorLib::C_Core::Instance().GetDgManager().LevelOf(levelItem->GetDgNode()->GetId());
	if (level->GetType() == EditorLib::E_LevelType_XtLevel)
		Tasks::SetActiveLevel((EditorLib::C_XtLevel*)level);
}

void C_SceneGraphPanel::OnUnloadLevel(wxCommandEvent& evt)
{
	if (((C_SceneTreeItem*)m_rightClickItem)->GetType() != E_SceneGraphItemType_DgNode)
		return;

	C_SceneTreeItemDgNode* levelItem = (C_SceneTreeItemDgNode*)m_rightClickItem;

	if (levelItem->GetDataType() != E_SceneGraphDataType_Level)
		return;

	Tasks::CloseLevel(EditorLib::C_Core::Instance().GetDgManager().LevelOf(levelItem->GetDgNode()->GetId()));
}

void C_SceneGraphPanel::OnCopyNodeName(wxCommandEvent& evt)
{
	if (wxTheClipboard->Open()) {	
		wxTheClipboard->SetData( new wxTextDataObject(WcToMb(m_rightClickItem->GetLabel()).Text()) );
		wxTheClipboard->Close();
	}
}

void C_SceneGraphPanel::OnRenameNode(wxCommandEvent& evt)
{
	m_pNodeTreeCtrl->StartRename(m_rightClickItem);
}

void C_SceneGraphPanel::OnExportToLevel(wxCommandEvent& evt)
{
	Tasks::ExportToLevel();
}

void C_SceneGraphPanel::OnUnparentNode(wxCommandEvent& evt)
{
	Btl::String objIdStr;
	if (BuildObjIdStrFromSelection(objIdStr))
	{
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute(
			"parent", objIdStr + " -u");
	}
}

void C_SceneGraphPanel::OnSetHidden(wxCommandEvent& evt)
{
	C_SceneTreeItemDgNode* dgItem = (C_SceneTreeItemDgNode*)m_rightClickItem;
	EditorLib::C_Core::Instance().GetDgManager().LevelOf(dgItem->GetDgNode()->GetId())->SetHidden(evt.IsChecked());
	C_RenderCore::Instance().DirtyAllViewports();
}

bool C_SceneTreeCtrl::OnCompareItems(TreeItem* a, TreeItem* b)
{
	C_SceneGraphFilter::E_SortFilter filter = m_pSceneFilter->GetSortFilter();

	C_SceneTreeItem* item1 = (C_SceneTreeItem*)a;
	C_SceneTreeItem* item2 = (C_SceneTreeItem*)b;

	if (item1->GetType() != E_SceneGraphItemType_DgNode || item2->GetType() != E_SceneGraphItemType_DgNode)
	{
		if (filter == C_SceneGraphFilter::E_SortFilter_TypeAsc)
			filter = C_SceneGraphFilter::E_SortFilter_AlphaAsc;
		else if (filter == C_SceneGraphFilter::E_SortFilter_TypeDesc)
			filter = C_SceneGraphFilter::E_SortFilter_AlphaDesc;
		else if (filter == C_SceneGraphFilter::E_SortFilter_Placement)
			filter = C_SceneGraphFilter::E_SortFilter_AlphaAsc;
	}

	switch (filter)
	{
	case (C_SceneGraphFilter::E_SortFilter_AlphaAsc):
		{
			wxString itemA = WcToMb(a->GetLabel()).Text();
			wxString itemB = WcToMb(b->GetLabel()).Text();

			m_AlphaSortA.Matches(itemA);
			m_AlphaSortB.Matches(itemB);

			wxString textA = m_AlphaSortA.GetMatch(itemA, 1);
			wxString textB = m_AlphaSortB.GetMatch(itemB, 1);

			if (textA == textB)
			{
				wxString numberA = m_AlphaSortA.GetMatch(itemA, 3);
				wxString numberB = m_AlphaSortB.GetMatch(itemB, 3);
				unsigned long numA, numB;
				if (numberA.ToULong(&numA) && numberB.ToULong(&numB))
					return numA < numB;
				else
					return 0;
			}
			else
				return textA.CmpNoCase(textB) < 0;
		}
	case (C_SceneGraphFilter::E_SortFilter_AlphaDesc):
		{
			wxString itemA = WcToMb(a->GetLabel()).Text();
			wxString itemB = WcToMb(b->GetLabel()).Text();

			m_AlphaSortA.Matches(itemA);
			m_AlphaSortB.Matches(itemB);

			wxString textA = m_AlphaSortA.GetMatch(itemA, 1);
			wxString textB = m_AlphaSortB.GetMatch(itemB, 1);

			if (textA == textB)
			{
				wxString numberA = m_AlphaSortA.GetMatch(itemA, 3);
				wxString numberB = m_AlphaSortB.GetMatch(itemB, 3);
				unsigned long numA, numB;
				if (numberA.ToULong(&numA) && numberB.ToULong(&numB))
					return numA > numB;
				else
					return 0;
			}
			else
				return textB.CmpNoCase(textA) < 0;
		}
	case (C_SceneGraphFilter::E_SortFilter_Placement):
		{
			return ((C_SceneTreeItemDgNode*)item1)->GetItemNumber() < ((C_SceneTreeItemDgNode*)item2)->GetItemNumber();
		}

	case (C_SceneGraphFilter::E_SortFilter_TypeAsc):
		{
			const EditorLib::I_DgNode* pDgNode1 = ((C_SceneTreeItemDgNode*)item1)->GetDgNode();
			const EditorLib::I_DgNode* pDgNode2 = ((C_SceneTreeItemDgNode*)item2)->GetDgNode();
			if (!pDgNode1 || !pDgNode2)
			{
				BIZ_ASSERT_FAIL("Sort (E_SortFilter_TypeAsc) failed!");
				return 0;
			}

			const EditorLib::C_XtObject* pXtObject1 = pDgNode1->AsXtObject();
			const EditorLib::C_XtObject* pXtObject2 = pDgNode2->AsXtObject();
			if(pXtObject1 && pXtObject2 && pXtObject1->GetXtTypeName() && pXtObject2->GetXtTypeName())
			{
				wxString composedName1 = pXtObject1->GetXtTypeName();
				composedName1 += WcToMb(a->GetLabel()).Text();

				wxString composedName2 = pXtObject2->GetXtTypeName();
				composedName2 += WcToMb(b->GetLabel()).Text();

				return composedName1.CmpNoCase(composedName2) < 0;	
			}
			
			return 0;
		}

	case (C_SceneGraphFilter::E_SortFilter_TypeDesc):
		{
			const EditorLib::I_DgNode* pDgNode1 = ((C_SceneTreeItemDgNode*)item1)->GetDgNode();
			const EditorLib::I_DgNode* pDgNode2 = ((C_SceneTreeItemDgNode*)item2)->GetDgNode();
			if (!pDgNode1 || !pDgNode2)
			{
				BIZ_ASSERT_FAIL("Sort (E_SortFilter_TypeDesc) failed!");
				return 0;
			}

			const EditorLib::C_XtObject* pXtObject1 = pDgNode1->AsXtObject();
			const EditorLib::C_XtObject* pXtObject2 = pDgNode2->AsXtObject();
			if(pXtObject1 && pXtObject2 && pXtObject1->GetXtTypeName() && pXtObject2->GetXtTypeName())
			{
				wxString composedName1 = pXtObject1->GetXtTypeName();
				composedName1 += WcToMb(a->GetLabel()).Text();

				wxString composedName2 = pXtObject2->GetXtTypeName();
				composedName2 += WcToMb(b->GetLabel()).Text();

				return composedName2.CmpNoCase(composedName1) < 0;	
			}
			
			return 0;
		}
	}

	return false;
}

bool C_SceneTreeCtrl::OnRemoveItems(const XtUt::Array<TreeItem*>& items)
{
	if (Tasks::IsLevelLoaded())
	{
		EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("delete", "");
		if (!status)
			LOGERROR(g_Log, "Deletion failed");
	}

	return false;
}

void C_SceneTreeCtrl::OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType)
{
	if (m_pSceneGraphPanel && selectionType == kTcSelectionClean)
		m_pSceneGraphPanel->OnSelectionChanged(this);
}

C_SceneTreeItemDgNode::C_SceneTreeItemDgNode(C_SceneGraphPanel *pSceneGraphPanel, const EditorLib::I_DgNode *pDgNode, SceneNodeDataMap* pDataMap, E_SceneGraphDataType dataType)
: m_pSceneGraphPanel(pSceneGraphPanel)
, m_pDgNode(pDgNode)
, m_pDataMap(pDataMap)
, m_itemNumber(itemCount)
, m_dataType(dataType)
{
	assert(m_pSceneGraphPanel);
	assert(m_pDgNode);
	assert(m_pDataMap);

	SceneNodeDataMap::InsertionReturnType insert = m_pDataMap->Insert(pDgNode->GetId(), this);
	assert (insert.inserted);

	itemCount++;
}

C_SceneTreeItemDgNode::~C_SceneTreeItemDgNode()
{
	SceneNodeDataMap::Iterator it = m_pDataMap->Find(m_pDgNode->GetId());
	assert(it != m_pDataMap->End());
	m_pDataMap->Erase(it);
}

bool C_SceneTreeItemDgNode::OnActivate()
{
	if (!m_pDgNode)
		return false;

	if (GetDataType() == E_SceneGraphDataType_Level)
	{
		EditorLib::I_Level* pLevel = EditorLib::C_Core::Instance().GetDgManager().LevelOf(m_pDgNode->GetId());

		if (pLevel->GetType() != EditorLib::E_LevelType_XtLevel)
			return false;

		Tasks::SetActiveLevel((EditorLib::C_XtLevel*)pLevel);

		return true;
	}
	
	Tasks::ShowNodePropertiesWindow();
	
	return true;
}

void C_SceneTreeItemDgNode::OnContextMenu(const WinPoint& pt)
{
	wxPoint clientpt(pt.x, pt.y);
	m_pSceneGraphPanel->ShowMenu(this, clientpt);
}

bool C_SceneTreeItemDgNode::OnItemDeleting()
{
	return false;
}

void C_SceneTreeItemDgNode::OnItemDeleted()
{
}

bool C_SceneTreeItemDgNode::OnItemSelectionChanged(bool selected)
{
	return m_pSceneGraphPanel->OnSelectionChanging(this, selected);
}

bool C_SceneTreeItemDgNode::OnReparent(TreeItem* parent)
{
	m_pSceneGraphPanel->QueueReparent(this, parent);

	return false;
}

bool C_SceneTreeItemDgNode::OnReorder(int newIndex)
{
	return false;
}

bool C_SceneTreeItemDgNode::OnStartRename(Sz16& newName)
{
	if (m_pDgNode->IsReadOnly())
		return false;

	if (m_pDgNode->IsA(EditorLib::E_ObjType_DgNodeBase) && !((EditorLib::C_DgNodeBase*)m_pDgNode)->GetNodeFlags().canRename)
		return false;
	
	return true;
}

bool C_SceneTreeItemDgNode::OnEndRename(Sz16& newName)
{
	Sz8 newName8 = WcToMb(newName);

	if (newName8.Length() == 0)
		return false;

	//Spaces used to tokenise args, so not allowed in names.
	newName8 = Replace(newName8, ' ', '_');

	Btl::String args;
	args.Printf("%s %s", m_pDgNode->GetId().AsString().Chars(), newName8.Text());
	
	EditorLib::C_Status status;
	status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("rename", args);
	if (!status)
		return false;

	newName = MbToWc(newName8);

	m_pSceneGraphPanel->m_Flags.filterDgDirty = true;

	return true;
}

const EditorLib::I_DgNode* C_SceneTreeItemDgNode::GetDgNode() const
{
	return m_pDgNode;
}

E_SceneGraphDataType C_SceneTreeItemDgNode::GetDataType() const
{
	return m_dataType;
}

long C_SceneTreeItemDgNode::GetItemNumber()
{
	return m_itemNumber;
}

void C_SceneTreeItemDgNode::Reset()
{
	itemCount = 0;
}

void C_SceneTreeItemDgNode::SetLevelReadOnly(bool readOnly)
{
	if (m_dataType == E_SceneGraphDataType_Level)
	{
		if (readOnly)
			SetColour(120, 0, 0);
		else
			SetColour(0, 120, 0);
	} else
	{
		if (readOnly)
			SetColour(128, 128, 128);
		else
			SetColour(0,0,0);
	}

	C_SceneTreeItemDgNode* child = (C_SceneTreeItemDgNode*)GetFirstChild();
	while (child)
	{
		child->SetLevelReadOnly(readOnly);
		child = (C_SceneTreeItemDgNode*)child->GetNextSibling();
	}
}

////// SLOTS //////

C_SceneGraphPanel::C_Slots::C_Slots(C_SceneGraphPanel* pSceneGraphPanel)
{
	m_pSceneGraphPanel = pSceneGraphPanel;
}

void C_SceneGraphPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnDatabaseEventSlot);

	s.levelEvent.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelEventSlot);

	s.levelNameChanged.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelNameChangedSlot);
	s.levelOpening.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelOpeningSlot);
	s.levelOpened.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelClosingSlot);
	s.levelClosed.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelClosedSlot);
	s.levelAftkRefAdded.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelAftkRefAddedSlot);
	s.levelAftkRefRemoved.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnLevelAftkRefRemovedSlot);
	s.aftkRefLoaded.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnAftkRefLoadedSlot);

	s.dgNodeUpdated.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnDgNodeUpdatedSlot);
	s.dgNodeDestroy.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnDgNodeDestroySlot);

	s.coreEnableViews.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnCoreEnableViews);
	s.coreSelectionChanged.connect(this, &Editor::C_SceneGraphPanel::C_Slots::OnCoreSelectionChangedSlot);
}

void C_SceneGraphPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);

	s.levelEvent.disconnect(this);

	s.levelNameChanged.disconnect(this);
	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);
	s.levelAftkRefAdded.disconnect(this);
	s.levelAftkRefRemoved.disconnect(this);
	s.aftkRefLoaded.disconnect(this);

	s.dgNodeCreated.disconnect(this);
	s.dgNodeUpdated.disconnect(this);
	s.dgNodeDestroy.disconnect(this);

	s.coreEnableViews.disconnect(this);
	s.coreSelectionChanged.disconnect(this);
}

void C_SceneGraphPanel::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	m_pSceneGraphPanel->OnDatabaseEvent(db, evt);
}

void C_SceneGraphPanel::C_Slots::OnLevelEventSlot(const EditorLib::I_Level* level, EditorLib::E_LevelEvent evt)
{
	if (evt == EditorLib::E_LevelEvent_ReadOnlyFlagChanged)
	{
		if (level)
		{
			SceneNodeDataMap::Iterator it = m_pSceneGraphPanel->m_NodeDataMap.Find(level->GetRootSceneNode()->GetId());

			if (it == m_pSceneGraphPanel->m_NodeDataMap.End())
				return;

			it->GetValue()->SetLevelReadOnly(level->IsReadOnly());
		}
	}
}

void C_SceneGraphPanel::C_Slots::OnLevelNameChangedSlot(const EditorLib::I_Level& level)
{
	m_pSceneGraphPanel->OnLevelNameChanged(level);
}

void C_SceneGraphPanel::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	m_pSceneGraphPanel->OnLevelOpening(level);
}
			
void C_SceneGraphPanel::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	m_pSceneGraphPanel->OnLevelOpened(level);
}

void C_SceneGraphPanel::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pSceneGraphPanel->OnLevelClosing(level);
}

void C_SceneGraphPanel::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	m_pSceneGraphPanel->OnLevelClosed(level);
}

void C_SceneGraphPanel::C_Slots::OnLevelAftkRefAddedSlot(const Btl::String& originalFilePath)
{
	m_pSceneGraphPanel->OnLevelAftkRefAdded(originalFilePath);
}

void C_SceneGraphPanel::C_Slots::OnLevelAftkRefRemovedSlot(const Btl::String& originalFilePath)
{
	m_pSceneGraphPanel->OnLevelAftkRefRemoved(originalFilePath);
}

void C_SceneGraphPanel::C_Slots::OnAftkRefLoadedSlot(
	const Btl::String& originalFilePath, 
	const Aftk::C_Scene& aftkScene, 
	const EditorLib::C_SceneNode& sceneNode, 
	const EditorLib::AftkDgMap& aftkDgMap)
{
	m_pSceneGraphPanel->OnAftkRefLoaded(originalFilePath, aftkScene, sceneNode, aftkDgMap);
}

void C_SceneGraphPanel::C_Slots::OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& node, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pSceneGraphPanel->OnDgNodeUpdated(node, pHc);
}

void C_SceneGraphPanel::C_Slots::OnDgNodeDestroySlot(const EditorLib::I_DgNode& node)
{
	m_pSceneGraphPanel->OnDgNodeDestroy(node);
}

void C_SceneGraphPanel::C_Slots::OnCoreEnableViews(bool enable)
{
	m_pSceneGraphPanel->OnCoreEnableViews(enable);
}

void C_SceneGraphPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pSceneGraphPanel->OnCoreSelectionChanged();
}
