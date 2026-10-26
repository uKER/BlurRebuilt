#include "stdafx.h"

#include <wx/treectrl.h>
#include <wx/regex.h>
#include <wx/clipbrd.h>
#include <wx/Dir.h>

#include <xtwin/WndCore.h>
#include <xtwin/TreeCtrl.h>

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Misc/StringUtil.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Misc/Selection.h>

#include <EditorApp/editor/viewportpanel/ViewportMenuBar.h>
#include <EditorApp/Components/LevelSelectDialog.h>
#include <EditorApp/Components/scene_graph/SceneGraphPanel.h>
#include <EditorApp/Components/scene_graph/EditorSceneGraphPanel.h>
#include <EditorApp/Components/scene_graph/SceneGraphMenuBar.h>
#include <EditorApp/Components/scene_graph/SceneGraphFilter.h>

#include "EditorApp/Tasks/LevelTasks.h"
#include "EditorApp/Tasks/UITasks.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>
#include <EditorApp/Frame/Frame.h>

#include <algorithm>

using namespace Editor;

C_EditorSceneGraphPanel::C_EditorSceneGraphPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
C_SceneGraphPanel(parent, id, pos, size, style),
m_slots(this)
{
	m_slots.Connect();

	m_Flags.selectionInvalid = false;
}

C_EditorSceneGraphPanel::~C_EditorSceneGraphPanel()
{
}

void C_EditorSceneGraphPanel::Shutdown()
{
	m_slots.Disconnect();

	C_SceneGraphPanel::Shutdown();
}

void C_EditorSceneGraphPanel::OnCoreSelectionChanged()
{
	if (C_SceneGraphPanel::m_Flags.pendingSync)
		return;

	Freeze();

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	this->SetEvtHandlerEnabled(false);
	m_pNodeTreeCtrl->ClearSelection(true);
	m_pPropertyTreeCtrl->ClearSelection(true);
	SyncToExistingSelection(selection);
	this->SetEvtHandlerEnabled(true);

	m_Flags.selectionInvalid = false;
	
	Thaw();
}

void C_EditorSceneGraphPanel::SyncSelectionToTree()
{
	if (C_SceneGraphPanel::m_Flags.pendingSync || !Tasks::IsLevelLoaded())
		return;

	Freeze();

	Btl::String objIdStr;
	BuildObjIdStrFromSelection(objIdStr);

	//autoSelect happens after a sibling item was deleted
	if(C_SceneGraphPanel::m_Flags.autoSelect) 
	{
		//Make the autoselection non-undoable
		objIdStr = objIdStr + " -U";
		C_SceneGraphPanel::m_Flags.autoSelect = false;
	}

	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", objIdStr);

	m_Flags.selectionInvalid = false;

	Thaw();
}

void C_EditorSceneGraphPanel::SyncToExistingSelection(const EditorLib::C_Selection& selection)
{
	Freeze();

	m_selectionCache = selection;

	for (S32 i = 0; i < selection.GetItems().Size(); i++)
	{	
		const EditorLib::C_NodeId& nodeId = selection.GetItems()[i].id;

		SceneNodeDataMap::Iterator it;
		
		it = m_NodeDataMap.Find(nodeId);
		if (it != m_NodeDataMap.End())
		{
			it->GetValue()->SetSelected(true);
		}

		it = m_PropertyDataMap.Find(nodeId);
		if (it != m_PropertyDataMap.End())
		{
			it->GetValue()->SetSelected(true);
		}
	}

	Thaw();
}

bool C_EditorSceneGraphPanel::BuildObjIdStrFromSelection(Btl::String& objIdStr)
{
	for (S32 i = 0; i < m_selectionCache.GetItems().Size(); i++)
	{
		Btl::String tmp;
		tmp.Printf("%s,", m_selectionCache.GetItems()[i].id.AsString().Chars());
		objIdStr = objIdStr + tmp;
	}

	return m_selectionCache.GetItems().Size() > 0;
}

void C_EditorSceneGraphPanel::OnSelectionChanged(TreeCtrl* from)
{
	m_Flags.selectionInvalid = true;

	m_selectionCache.Clear();

	for (XtUt::Array<TreeItem*>::ConstIterator it(&from->GetSelection()); it; ++it)
	{
		m_selectionCache.Add(EditorLib::C_Selection::E_Mode_Object, ((C_SceneTreeItemDgNode*)*it)->GetDgNode()->GetId());
	}
}

bool C_EditorSceneGraphPanel::OnSelectionChanging(TreeItem* item, bool selected)
{
	if (!C_SceneGraphPanel::OnSelectionChanging(item, selected))
		return false;

	if (GetFlags().pendingDeletion)
		return true;

	if (selected)
	{
		if (!m_selectionCache.Find(((C_SceneTreeItemDgNode*)item)->GetDgNode()->GetId()))
			m_selectionCache.Add(EditorLib::C_Selection::E_Mode_Object, ((C_SceneTreeItemDgNode*)item)->GetDgNode()->GetId());
	}
	else
	{
		m_selectionCache.Remove(EditorLib::C_Selection::E_Mode_Object, ((C_SceneTreeItemDgNode*)item)->GetDgNode()->GetId());
	}

	m_Flags.selectionInvalid = true;

	return true;
}

void C_EditorSceneGraphPanel::OnGraphSyncSelection()
{
	OnCoreSelectionChanged();
}

void C_EditorSceneGraphPanel::OnGraphClearSelection()
{
	m_selectionCache.Clear();
}

void C_EditorSceneGraphPanel::OnGraphIdle()
{
	if (m_Flags.selectionInvalid)
		SyncSelectionToTree();
}

C_EditorSceneGraphPanel::C_Slots::C_Slots(C_EditorSceneGraphPanel* pSceneGraphPanel)
{
	m_pSceneGraphPanel = pSceneGraphPanel;
}

void C_EditorSceneGraphPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreSelectionChanged.connect(this, &Editor::C_EditorSceneGraphPanel::C_Slots::OnCoreSelectionChangedSlot);
}

void C_EditorSceneGraphPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreSelectionChanged.disconnect(this);
}

void C_EditorSceneGraphPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pSceneGraphPanel->OnCoreSelectionChanged();
}
