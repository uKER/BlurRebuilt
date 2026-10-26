#include "stdafx.h"
#include <wx/aui/aui.h>
#include <xtwin/svnxt.h>
#include <xtwin/commitwin.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameVersionControlMenu.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>

#include <EditorApp/Tasks/LevelTasks.h>

namespace Editor
{

C_FrameVersionControlMenu::C_FrameVersionControlMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameVersionControlMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pMenu = new wxMenu();
	pWxMenuBar->Append(pMenu, _T("Version Control"));

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Version Control|Update To Latest", pFrame, &C_FrameVersionControlMenu::OnUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Version Control|Commit Changes...", pFrame, &C_FrameVersionControlMenu::OnCommit);

	return true;
}

#include <iostream>



void C_FrameVersionControlMenu::OnUpdate(wxCommandEvent& e)
{
	// TODO: Remove HACK: Clear selection, to fix issues with scarab form updating in separate thread
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", "-c");
	wxWindow *pScarabWindow = wxGetApp().GetFrame()->GetAuiManager()->GetPane("Pane_Properties").window;
	if (pScarabWindow)
	{
		pScarabWindow->AddPendingEvent(wxIdleEvent());
		pScarabWindow->ProcessPendingEvents();
	}

	// Store the level IDs, for re-loading later
	Btl::Vector<XtId> loadedLevels;
	Btl::Vector<EditorLib::C_LevelPack*> packs = EditorLib::C_Core::Instance().GetLevelManager()->GetPacks();
	for (Btl::Vector<EditorLib::C_LevelPack*>::Iterator pack = packs.Begin(); pack != packs.End(); ++pack)
	{
		Btl::Vector<EditorLib::C_XtLevel*> levels = (*pack)->GetLevels();
		for (Btl::Vector<EditorLib::C_XtLevel*>::Iterator level = levels.Begin(); level != levels.End(); ++level)
		{
			if ((*level)->GetFlags().isOpen)
				loadedLevels.PushBack((*level)->GetXtRecordId());
		}
	}

	if (!Tasks::CloseAllPacks())
		return;

	const EditorLib::C_CoreConfig& coreConfig = EditorLib::C_Core::Instance().GetConfig();
	SvnXtParams svnXtParams(
		EditorLib::C_Core::Instance().GetXtManager().GetDb(),
		(Misc::RemoveTrailingSlash(coreConfig.xtDbPath)+"/").Chars());
	if (SvnXtRecover((HWND)wxGetApp().GetFrame()->GetHandle(), &svnXtParams))
	{
		// Re-load the levels that were open previously
		for (Btl::Vector<XtId>::Iterator level = loadedLevels.Begin(); level != loadedLevels.End(); ++level)
		{
			if (!EditorLib::C_Core::Instance().GetLevelManager()->Open(*level))
			{
				LOGERROR(g_Log, "Failed to reload level with id (%X).", *level);
				wxMessageBox(Btl::String::Format("ERROR: Failed to reload level with id (%X). Has it been deleted?", *level).Chars());
			}
		}

		Tasks::LoadAllReferences(true);
	}
}

void C_FrameVersionControlMenu::OnCommit(wxCommandEvent& e)
{
	// TODO: Remove HACK: Clear selection, to fix issues with scarab form updating in separate thread
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", "-c");
	wxWindow *pScarabWindow = wxGetApp().GetFrame()->GetAuiManager()->GetPane("Pane_Properties").window;
	if (pScarabWindow)
	{
		pScarabWindow->AddPendingEvent(wxIdleEvent());
		pScarabWindow->ProcessPendingEvents();
	}

	EditorLib::C_XtManager& xtManager = EditorLib::C_Core::Instance().GetXtManager();

	// Store the level IDs, for re-loading later
	Btl::Vector<XtId> loadedLevels;
	Btl::Vector<EditorLib::C_LevelPack*> packs = EditorLib::C_Core::Instance().GetLevelManager()->GetPacks();
	for (Btl::Vector<EditorLib::C_LevelPack*>::Iterator pack = packs.Begin(); pack != packs.End(); ++pack)
	{
		Btl::Vector<EditorLib::C_XtLevel*> levels = (*pack)->GetLevels();
		for (Btl::Vector<EditorLib::C_XtLevel*>::Iterator level = levels.Begin(); level != levels.End(); ++level)
		{
			if ((*level)->GetFlags().isOpen)
				loadedLevels.PushBack((*level)->GetXtRecordId());
		}
	}
	CommitParams params;

	const EditorLib::C_CoreConfig& coreConfig = EditorLib::C_Core::Instance().GetConfig();

	params.path = (Misc::RemoveTrailingSlash(coreConfig.xtDbPath)+"/").Chars();
	params.parentWindow = (HWND)EditorLib::C_Core::Instance().GetWindowHandle();

	if (SorceryCommit(&params) == kCrProceed)
	{
		if (!Tasks::SaveLevel())
			return;
		
		Tasks::CloseAllPacks(false);

		SvnXtParams xp(EditorLib::C_Core::Instance().GetXtManager().GetDb(),
			(Misc::RemoveTrailingSlash(coreConfig.xtDbPath)+"/").Chars(),
			params.comment);

		xp.files = &params.files;

		if (SvnXtCommit((HWND)wxGetApp().GetFrame()->GetHandle(), &xp))
		{
			// Re-load the levels that were open previously
			for (Btl::Vector<XtId>::Iterator level = loadedLevels.Begin(); level != loadedLevels.End(); ++level)
			{
				if (!EditorLib::C_Core::Instance().GetLevelManager()->Open(*level))
				{
					LOGERROR(g_Log, "Failed to reload level with id (%X).", *level);
					wxMessageBox(Btl::String::Format("ERROR: Failed to reload level with id (%X). Has it been deleted?", *level).Chars());
				}
			}

			Tasks::LoadAllReferences(true);
		}
	}
}

}

