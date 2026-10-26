#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>

#include <DialogueMan/Frame/Menus/FrameFileMenu.h>

#include <DialogueMan/ImportExport/Import.h>
#include <DialogueMan/ImportExport/Export.h>

#include <DialogueMan/GlobalSignals.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>

namespace DMan
{

C_FrameMenuBar* C_FrameFileMenu::m_pMenuBar = 0;

C_FrameFileMenu::C_FrameFileMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameFileMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"File|Import", pFrame, &C_FrameFileMenu::OnImport);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"File|Export", pFrame, &C_FrameFileMenu::OnExport);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"File|Save\tCtrl-S", pFrame, &C_FrameFileMenu::OnSave);
	//m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		//"File|Generate Report", pFrame, &C_FrameFileMenu::OnReport);
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"File|E&xit", pFrame, &C_FrameFileMenu::OnExit);

	return true;
}

void C_FrameFileMenu::OnExit(wxCommandEvent& event)
{
    wxGetApp().GetGlobalSignals().quit();
}

void C_FrameFileMenu::OnImport(wxCommandEvent& event)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Import Wizard"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	Import::Do(wxGetApp().GetFrame());
}

void C_FrameFileMenu::OnExport(wxCommandEvent& event)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Export Wizard"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	Export::Do(wxGetApp().GetFrame());
}

void C_FrameFileMenu::OnSave(wxCommandEvent& event)
{
	wxGetApp().SaveDb();
}

void C_FrameFileMenu::OnReport(wxCommandEvent& event)
{
/*	wxPanel* panel = new wxPanel(m_pNotebook, wxID_ANY);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(mainSizer);

	wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	horizSizer->Add(new wxButton(panel, 
								  wxID_ANY, 
								  wxT("Print...")),
				    0,
				    wxALL,
				    2);

	horizSizer->Add(new wxButton(panel, 
								  wxID_ANY, 
								  wxT("Save As...")),
				    0,
				    wxALL,
				    2);

	mainSizer->Add(horizSizer, 0);

	wxHtmlWindow* ctrl = new wxHtmlWindow(panel, wxID_ANY,
                                   wxDefaultPosition,
                                   wxSize(400,300));

	wxString sample;
	ctrl->SetPage(sample);

	mainSizer->Add(ctrl, 0, wxEXPAND | wxALL, 2);

	m_pNotebook->AddPage(panel, wxT("Report") , true);*/
}

}
