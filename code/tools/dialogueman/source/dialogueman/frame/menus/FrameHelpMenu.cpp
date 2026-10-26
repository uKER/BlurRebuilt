#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>

#include <DialogueMan/Frame/Menus/FrameHelpMenu.h>
#include <system/debug/buildstamp.h>

namespace DMan
{

C_FrameMenuBar* C_FrameHelpMenu::m_pMenuBar = 0;

C_FrameHelpMenu::C_FrameHelpMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameHelpMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

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

	Util::C_String errorMessage;
	errorMessage.Printf("Dialogue Manager Revision %s\n\nBuilt by %s on %s\nBuild time:  %s %s", buildRevision, buildUser, buildPc, buildDate, buildTime );

	wxMessageBox(errorMessage.Chars(), "About", wxICON_INFORMATION);
}

}
