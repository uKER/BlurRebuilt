#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameEditMenu.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Components/PropertyDialog.h>
#include <EditorApp/Components/FindDialog.h>
#include <EditorApp/Components/DbPropertyDialog.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>

#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>


namespace Editor
{

static void OnMenuUpdate(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsLevelLoaded());
}

C_FrameEditMenu::C_FrameEditMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameEditMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pMenu = new wxMenu();
	pWxMenuBar->Append(pMenu, _T("Edit"));

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Undo\tCtrl-Z", pFrame, &Tasks::Undo, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Redo\tCtrl-Y", pFrame, &Tasks::Redo, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Copy\tCtrl-C", pFrame, &Tasks::CopyToClipboard, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Paste\tCtrl-V", pFrame, &Tasks::PasteFromClipboard, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Duplicate\tCtrl-D", pFrame, &Tasks::DuplicateSelection, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Edit|Separator0", E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Select All\tCtrl-A", pFrame, &Tasks::SelectAll, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Edit|Separator0", E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Preferences", pFrame, &C_PropertyDialog::OnShowDialog);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Database Properties", pFrame, &C_DbPropertyDialog::OnShowDialog);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Edit|Separator0", E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Find Actors", pFrame, &C_FinderDialog::OnShowDialog, &Editor::OnMenuUpdate);


	return true;
}


}