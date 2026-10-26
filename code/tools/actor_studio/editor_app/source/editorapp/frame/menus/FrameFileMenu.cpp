#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

#include <EditorApp/Misc/RecentFiles.h>

#include <EditorApp/Frame/Menus/FrameFileMenu.h>

namespace Editor
{

C_FrameMenuBar* C_FrameFileMenu::m_pMenuBar = 0;
wxMenu* C_FrameFileMenu::m_pFileMenu = 0;
Ut::Vector<C_FileMenuItem<int> > C_FrameFileMenu::m_ClosePackMenuItems;
Ut::Vector<C_FileMenuItem<Btl::String> > C_FrameFileMenu::m_RecentFileMenuItems;

void OnMenuUpdate(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsPackLoaded());
}

C_FrameFileMenu::C_FrameFileMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameFileMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pMenu = new wxMenu();
	pWxMenuBar->Append(pMenu, _T("File"));
	int fileMenuId = pWxMenuBar->FindMenu(_T("File"));

	wxEventConnection c = wxConnect<wxEVENT_MENU_OPEN>(pFrame, &C_FrameFileMenu::OnFileMenu, fileMenuId);

	m_pFileMenu = pMenu;

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|New Level\tCtrl-N", pFrame, &C_FrameFileMenu::OnNewLevel);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|Open Level\tCtrl-O", pFrame, &C_FrameFileMenu::OnOpenLevel);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|&Add Level", pFrame, &C_FrameFileMenu::OnAddLevel);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|Recent Files...|Empty", pFrame, &C_FrameFileMenu::OnRecentFiles);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|Close Pack|Empty", pFrame, &C_FrameFileMenu::OnClosePack, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|Close All\tCtrl-Q", pFrame, &C_FrameFileMenu::OnCloseAllPacks, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|&Save\tCtrl-S", pFrame, &C_FrameFileMenu::OnSaveLevel, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|&Export to Level...\tCtrl-E", pFrame, &C_FrameFileMenu::OnExportToLevel, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|&References...\tCtrl-R", pFrame, &C_FrameFileMenu::OnShowReferencesDialog, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"&File|E&xit", pFrame, &C_FrameFileMenu::OnQuit);


	int id = m_pFileMenu->FindItem("Recent Files...");
	wxMenuItem* item = m_pFileMenu->FindItem(id, 0);
	wxMenu* subMenu = item->GetSubMenu();
	wxMenuItem* emptyItem = subMenu->FindItemByPosition(0);
	subMenu->Destroy(emptyItem);

	return true;
}

void C_FrameFileMenu::OnNewLevel(wxCommandEvent& e)
{
	Tasks::NewLevel();
}

void C_FrameFileMenu::OnOpenLevel(wxCommandEvent& e)
{
	Tasks::OpenLevel(true);
}

void C_FrameFileMenu::OnAddLevel(wxCommandEvent& e)
{
	Tasks::OpenLevel(false);
}

void C_FrameFileMenu::OnCloseAllPacks(wxCommandEvent& e)
{
	Tasks::CloseAllPacks();
}

void C_FrameFileMenu::OnSaveLevel(wxCommandEvent& e)
{
	Tasks::SaveLevel();
}

void C_FrameFileMenu::OnExportToLevel(wxCommandEvent& e)
{
	Tasks::ExportToLevel();
}

void C_FrameFileMenu::OnShowReferencesDialog(wxCommandEvent& e)
{
	Tasks::ShowReferencesDialog();
}

void C_FrameFileMenu::OnClosePack(wxCommandEvent& e)
{
	for (int i=0; i<m_ClosePackMenuItems.Size(); i++) {
		if (m_ClosePackMenuItems[i].id == e.GetId()) {
			EditorLib::C_Status status;
			EditorLib::ResultsArray results;

			Tasks::ClosePack(EditorLib::C_Core::Instance().GetLevelManager()->GetPack(m_ClosePackMenuItems[i].args));

			if (!status)
			{
				wxString msg;
				msg << "Error closing pack '" << m_RecentFileMenuItems[i].name.Chars() << "'";
				LOGERROR(g_Log, msg.c_str());
				wxMessageDialog dialog(m_pMenuBar->GetFrame(), msg, "Error", wxOK|wxICON_ERROR);
				dialog.ShowModal();
			}
		}
	}
}

void C_FrameFileMenu::OnRecentFiles(wxCommandEvent& e)
{
	for (int i=0; i<m_RecentFileMenuItems.Size(); i++) {
		if (m_RecentFileMenuItems[i].id == e.GetId()) {
			bool status;
			EditorLib::ResultsArray results;

			Btl::Vector<Btl::String> items;
			m_RecentFileMenuItems[i].args.Tokenise(items, ' ');

			
			if (items.Size() == 2)
			{
				XtId id;
				sscanf(items[1].Chars(), "%x", &id);

				status = Tasks::Open(id, true);
			}
			else
			{
				XtId id;
				sscanf(items[0].Chars(), "%x", &id);

				status = Tasks::Open(id, true);
			}

			if (status)
			{
				Tasks::LoadAllReferences(true);
				C_RecentFiles::MoveToTop(C_RecentFiles::GetCount()-i+C_RecentFiles::GetIndex()-1);
			}
		}
	}
}

void C_FrameFileMenu::OnFileMenu(wxMenuEvent& e)
{
	UpdateClosePacks();
	UpdateRecentFiles();
	e.Skip();
}

void C_FrameFileMenu::UpdateClosePacks()
{
	int id = m_pFileMenu->FindItem("Close Pack");
	wxMenuItem* item = m_pFileMenu->FindItem(id, 0);

	if (!item)
		return;

	wxMenu* subMenu = item->GetSubMenu();
	assert(subMenu!=0);

	for (int i=0; i<m_ClosePackMenuItems.Size(); i++)
	{
		m_ClosePackMenuItems[i].event.Disconnect();
	}

	m_ClosePackMenuItems.Clear();

	while (subMenu->GetMenuItemCount())
	{
		wxMenuItem* closePack = subMenu->FindItemByPosition(0);
		subMenu->Destroy(closePack);
	}

	Btl::Vector<EditorLib::C_LevelPack*> packs = EditorLib::C_Core::Instance().GetLevelManager()->GetPacks();
	for (int i=0; i<packs.Size(); i++) {
		wxMenuItem* closePack = new wxMenuItem(subMenu,wxID_ANY, packs[i]->GetName().Chars(), packs[i]->GetName().Chars(), wxITEM_NORMAL, 0);
		subMenu->Append(closePack);

		void (*fn)(wxCommandEvent&) = &C_FrameFileMenu::OnClosePack;

		wxEventConnection c0 = wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pMenuBar->GetFrame(), &C_FrameFileMenu::OnClosePack, closePack->GetId());

		C_FileMenuItem<int> item(closePack->GetId(), c0, packs[i]->GetId(), packs[i]->GetName());

		m_ClosePackMenuItems.PushBack(item);
	}
}

void C_FrameFileMenu::UpdateRecentFiles()
{
	int id = m_pFileMenu->FindItem("Recent Files...");
	wxMenuItem* item = m_pFileMenu->FindItem(id, 0);

	if (item != 0)
	{
		wxMenu* subMenu = item->GetSubMenu();
		assert(subMenu!=0);

		for (int i=0; i<m_RecentFileMenuItems.Size(); i++) {
			m_RecentFileMenuItems[i].event.Disconnect();
		}

		m_RecentFileMenuItems.Clear();

		while (subMenu->GetMenuItemCount() > 0) {
			wxMenuItem* recentFile = subMenu->FindItemByPosition(0);
			subMenu->Destroy(recentFile);
		}

		Btl::String registryPrefix;
		registryPrefix.Printf("RecentFiles_%x_",
			Btl::HashKeyOpBtlString::GetHash(EditorLib::C_Core::Instance().GetConfig().rootProjectPath));

		int recentFilesCount = C_RecentFiles::GetCount();
		int recentFilesStart = C_RecentFiles::GetIndex()-1;

		for (int i=0; i<recentFilesCount; i++) {
			int index = recentFilesStart-i;
			while (index<0)
				index+=recentFilesCount;

			Btl::String name = C_RecentFiles::GetName(index);
			Btl::String menuName;
			menuName.Printf("%d: %s", i+1, name.Chars());
			wxMenuItem* recentFile = new wxMenuItem(subMenu,wxID_ANY, menuName.Chars(), menuName.Chars(), wxITEM_NORMAL, 0);
			recentFile = subMenu->Append(recentFile);

			void (*fn)(wxCommandEvent&) = &C_FrameFileMenu::OnRecentFiles;

			wxEventConnection c0 = wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pMenuBar->GetFrame(), &C_FrameFileMenu::OnRecentFiles, recentFile->GetId());

			C_FileMenuItem<Btl::String> item(recentFile->GetId(), c0, C_RecentFiles::GetArgs(index), name);

			m_RecentFileMenuItems.PushBack(item);		
		}
	}
}

void C_FrameFileMenu::OnQuit(wxCommandEvent& e)
{
	Tasks::Quit();
}

}

