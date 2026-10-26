#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <xtwin/svnxt.h>
#include <xtwin/commitwin.h>

#include <system/misc/StringUtil.h>
#include <system/misc/BtlStringUtil.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <DialogueMan/Misc/ViewDisabler.h>
#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>

#include <DialogueMan/Frame/Menus/FrameVersionControlMenu.h>
#include <DialogueMan/Components/Scarab_form/WxScarabForm.h>
#include <DialogueMan/Components/LocForm/LocForm.h>



namespace DMan
{

C_FrameMenuBar* C_FrameVersionControlMenu::m_pMenuBar = 0;

C_FrameVersionControlMenu::C_FrameVersionControlMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameVersionControlMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Version Control|Update To Latest Version", pFrame, &C_FrameVersionControlMenu::OnUpdate);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Version Control|Commit Changes", pFrame, &C_FrameVersionControlMenu::OnCommit);

	return true;
}

void C_FrameVersionControlMenu::OnUpdate(wxCommandEvent& e)
{
	if(wxGetApp().InMemorySavingMode())
	{
		wxMessageDialog msg(m_pMenuBar->GetFrame(), "This is not available in Memory Saving Mode. Please restart in Normal Mode and try again", "Dialogue Manager", wxOK | wxICON_INFORMATION);
		msg.ShowModal();
		return;
	}
	
	wxGetApp().GetFrame()->GetScarabForm().SetDom((Xt::DomElement*)0,0);
	wxGetApp().GetFrame()->GetLocForm().HideAll();

	if (!wxGetApp().SaveDb()) {
		LOGERROR(g_Log, "Failed to save database!");
		return;
	}

	SvnXtParams svnXtParams(
		wxGetApp().GetDb(),
		(Misc::RemoveTrailingSlash(wxGetApp().GetConfig().XtDbPath.Chars())+"/").Chars());
	
	SvnXtUpdate((HWND)wxGetApp().GetFrame()->GetHandle(), &svnXtParams);
}

void C_FrameVersionControlMenu::OnCommit(wxCommandEvent& e)
{
	if(wxGetApp().InMemorySavingMode())
	{
		wxMessageDialog msg(m_pMenuBar->GetFrame(), "This is not available in Memory Saving Mode. Please restart in Normal Mode and try again", "Dialogue Manager", wxOK | wxICON_INFORMATION);
		msg.ShowModal();
		return;
	}

	CommitParams params;

	params.path = (Misc::RemoveTrailingSlash(wxGetApp().GetConfig().XtDbPath.Chars())+"/").Chars();
	params.parentWindow = (HWND)wxGetApp().GetFrame()->GetHandle();

	if (SorceryCommit(&params) == kCrProceed)
	{
		if (!wxGetApp().SaveDb()) {
			wxMessageBox("Could not save database!");
			return;
		}

		SvnXtCommit((HWND)wxGetApp().GetFrame()->GetHandle(),
			&SvnXtParams(wxGetApp().GetDb(),
			(Misc::RemoveTrailingSlash(wxGetApp().GetConfig().XtDbPath.Chars())+"/").Chars(),
			params.comment));
	}
}

}
