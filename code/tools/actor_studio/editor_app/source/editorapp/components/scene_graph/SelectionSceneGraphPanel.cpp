#include "stdafx.h"

#include <wx/treectrl.h>
#include <wx/regex.h>
#include <wx/clipbrd.h>
#include <wx/Dir.h>

#include <xtwin/TreeCtrl.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneNodeIterator.h>
#include <EditorLib/Misc/StringUtil.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/Selection.h>

#include <EditorApp/editor/viewportpanel/ViewportMenuBar.h>
#include <EditorApp/Components/LevelSelectDialog.h>
#include <EditorApp/Components/scene_graph/SceneGraphPanel.h>
#include <EditorApp/Components/scene_graph/SelectionSceneGraphPanel.h>
#include <EditorApp/Components/scene_graph/SceneGraphMenuBar.h>
#include <EditorApp/Components/scene_graph/SceneGraphFilter.h>
#include <EditorApp/Components/SplitterControl.h>

#include "EditorApp/Tasks/LevelTasks.h"
#include "EditorApp/Tasks/UITasks.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>
#include <EditorApp/Frame/Frame.h>

#include <algorithm>

using namespace Editor;

C_SelectionSceneGraphPanel::C_SelectionSceneGraphPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
C_SceneGraphPanel(parent, id, pos, size, style)
{
	m_Flags.selectionInvalid = false;
	m_pSplitter->SetSplitterProportion(10000);
}


C_SelectionSceneGraphPanel::~C_SelectionSceneGraphPanel()
{
}

void C_SelectionSceneGraphPanel::Shutdown()
{
	C_SceneGraphPanel::Shutdown();
}

bool C_SelectionSceneGraphPanel::BuildObjIdStrFromSelection(Btl::String& objIdStr)
{
	if (!GetSelection().GetItems().Size())
		return false;

	Btl::String idString;

	for (EditorLib::C_Selection::ItemArray::ConstIterator it = GetSelection().GetItems().Begin(); it != GetSelection().GetItems().End(); ++it)
	{
		idString += it->id.AsString() + ",";
	}

	idString = idString(0, idString.Length()-1);

	objIdStr = idString;

	return true;
}

bool C_SelectionSceneGraphPanel::OnSelectionChanging(TreeItem* item, bool selected)
{
	if (!C_SceneGraphPanel::OnSelectionChanging(item, selected))
		return false;

	if (selected)
		m_selectionCache.Add(EditorLib::C_Selection::E_Mode_Object, ((C_SceneTreeItemDgNode*)item)->GetDgNode()->GetId());
	else
		m_selectionCache.Remove(EditorLib::C_Selection::E_Mode_Object, ((C_SceneTreeItemDgNode*)item)->GetDgNode()->GetId());

	m_Flags.selectionInvalid = true;

	return true;
}

void C_SelectionSceneGraphPanel::OnGraphClearSelection()
{
	m_selectionCache.Clear();
}

void C_SelectionSceneGraphPanel::OnGraphIdle()
{
}

const EditorLib::C_Selection& C_SelectionSceneGraphPanel::GetSelection()
{
	return m_selectionCache;
}
