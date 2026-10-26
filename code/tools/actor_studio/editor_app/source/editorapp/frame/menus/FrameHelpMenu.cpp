#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameHelpMenu.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>

#include <system/debug/buildstamp.h>

namespace Editor
{

C_FrameHelpMenu::C_FrameHelpMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameHelpMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pMenu = new wxMenu();
	pWxMenuBar->Append(pMenu, _T("Help"));

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Help|About", pFrame, &C_FrameHelpMenu::OnAbout);

	return true;
}

void C_FrameHelpMenu::OnAbout(wxCommandEvent& e)
{
	const char* buildTime = Debug::GetBuildTime();
	const char* buildDate = Debug::GetBuildDate();
	const char* buildUser = Debug::GetBuildUser();
	const char* buildPc = Debug::GetBuildPc();
	const char* buildRevision = Debug::GetBuildRevision();

	Btl::String errorMessage;
	errorMessage.Printf("ActorStudio Revision %s\n\nBuilt by %s on %s\nBuild time:  %s %s", buildRevision, buildUser, buildPc, buildDate, buildTime );

	wxMessageBox(errorMessage.Chars(), "About", wxICON_INFORMATION);
}

}

