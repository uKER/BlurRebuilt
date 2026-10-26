#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>

#include <DialogueMan/Actions/AddRows.h>
#include <DialogueMan/Actions/DeleteRows.h>

#include <DialogueMan/Frame/Menus/FrameEditMenu.h>

namespace DMan
{

C_FrameMenuBar* C_FrameEditMenu::m_pMenuBar = 0;

C_FrameEditMenu::C_FrameEditMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameEditMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Add Row", pFrame, &C_FrameEditMenu::OnAdd);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Edit|Delete Rows(s)", pFrame, &C_FrameEditMenu::OnDelete);
	
	return true;
}

void C_FrameEditMenu::OnAdd(wxCommandEvent& e)
{
	AddRows::Do();
}

void C_FrameEditMenu::OnDelete(wxCommandEvent& e)
{
	DeleteRows::Do();
}

}
