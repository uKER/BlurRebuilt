#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Network/NetworkManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Frame/Menus/FrameToolsMenu.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Tasks/LevelTasks.h>

#include <EditorApp/App.h>

#include "XtWin/ConnectTo.h"

namespace Editor
{

static void OnBuildLevel(wxCommandEvent& e)
{	
	Tasks::BuildLevel();
}

static void OnRefreshExternalActors(wxCommandEvent& e)
{	
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("extActorsRefresh");
}

static void OnOfflineRefreshExternalActorsRecord(wxCommandEvent& e)
{	
	EditorLib::C_XtLevel* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel) return;
	
	Btl::String args;
	args.Printf("-r 0x%x", pLevel->GetXtRecordId());
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("extActorsOfflineRefresh", args);
}

static void OnOfflineRefreshExternalActorsPack(wxCommandEvent& e)
{	
	EditorLib::C_XtLevel* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel) return;

	Btl::String args;
	args.Printf("-p 0x%x", pLevel->GetXtPackId());
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("extActorsOfflineRefresh", args);
}

static void OnOfflineRefreshExternalActorsDb(wxCommandEvent& e)
{	
	wxMessageDialog dlg(
		0,
		"Warning, this could leave the UI unresponsive for several minutes.\nOnly continue if you know what you're doing.",
		"Are you sure?",
		wxOK|wxCANCEL|wxICON_QUESTION);
	int choice = dlg.ShowModal();
	if (choice == wxID_OK)
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("extActorsOfflineRefresh", "-d");
}

static void OnClearExternalActors(wxCommandEvent& e)
{	
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("extActorsRefresh", "-c");
}

static void OnLoadAllReferences(wxCommandEvent& e)
{	
	EditorLib::AftkRefHelpers::LoadAllReferences(true, false, &AppYield);
}

static void OnUnloadAllReferences(wxCommandEvent& e)
{	
	EditorLib::AftkRefHelpers::UnloadAllReferences(false, &AppYield);
}

void NetworkStateChangedCallback()
{
	wxGetApp().UpdateTitleBar(EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel());
}

ConnectToData gConnectToData;
static void OnConnectTo(wxCommandEvent& e)
{
	EditorLib::C_Core::Instance().GetNetworkManager().SetNetworkStateChangedCallback(&NetworkStateChangedCallback);

	ConnectToDlg dlg((HWND)EditorLib::C_Core::Instance().GetWindowHandle());

	dlg.Data() = gConnectToData;

	if (dlg.Modal())
	{
		gConnectToData = dlg.Data();

		EditorLib::C_Core::Instance().GetNetworkManager().sockAddrValid = MakeSockAddr(&gConnectToData, EditorLib::C_Core::Instance().GetNetworkManager().sockAddr, EditorLib::C_Core::Instance().GetNetworkManager().targetName);
		
		EditorLib::C_Core::Instance().GetNetworkManager().Reconnect();

		SaveConnectToData(&gConnectToData);
	}
}

static void OnWorkOffline(wxCommandEvent& e)
{
	EditorLib::C_Core::Instance().GetNetworkManager().SetNetworkStateChangedCallback(&NetworkStateChangedCallback);

	if (EditorLib::C_Core::Instance().GetNetworkManager().IsWorkingOffline())
		EditorLib::C_Core::Instance().GetNetworkManager().WorkOnline();
	else
		EditorLib::C_Core::Instance().GetNetworkManager().WorkOffline();

	wxGetApp().UpdateTitleBar(EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel());
}

static void OnMenuUpdate(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsLevelLoaded());
}

static void OnMenuUpdateDb(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsDbOpen());
}

static void OnWorkOfflineUpdate(wxUpdateUIEvent& e)
{
	e.Check(EditorLib::C_Core::Instance().GetNetworkManager().IsWorkingOffline());
}

C_FrameToolsMenu::C_FrameToolsMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameToolsMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pMenu = new wxMenu();
	pWxMenuBar->Append(pMenu, _T("Tools"));

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Build Level\tF7", pFrame, &Editor::OnBuildLevel, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Refresh ExtActors for Active Record\tF9", pFrame, &Editor::OnRefreshExternalActors, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Offline ExtActors|Refresh ExtActors for Active Record", pFrame, &Editor::OnOfflineRefreshExternalActorsRecord, &Editor::OnMenuUpdateDb);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Offline ExtActors|Refresh ExtActors for Pack", pFrame, &Editor::OnOfflineRefreshExternalActorsPack, &Editor::OnMenuUpdateDb);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Offline ExtActors|Refresh ExtActors for Database", pFrame, &Editor::OnOfflineRefreshExternalActorsDb, &Editor::OnMenuUpdateDb);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Clear External Actors for Active Record", pFrame, &Editor::OnClearExternalActors, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Tools|Separator0", E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Load all Aftk Refs", pFrame, &Editor::OnLoadAllReferences, &Editor::OnMenuUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Unload all Aftk Refs", pFrame, &Editor::OnUnloadAllReferences, &Editor::OnMenuUpdate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Tools|Separator1...", E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Tools|Connect To...\tCtrl+F2", pFrame, &Editor::OnConnectTo);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Tools|Work Offline", pFrame, &Editor::OnWorkOffline, &Editor::OnWorkOfflineUpdate, E_MenuItem_Check);

	return true;
}

}
