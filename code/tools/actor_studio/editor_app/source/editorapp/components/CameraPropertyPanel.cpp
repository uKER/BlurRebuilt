#include "stdafx.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include <EditorApp/Components/CameraPropertyPanel.h>
#include <EditorApp/Properties/CameraProperties.h>

#include <EditorApp/Editor/RenderViewport.h>

namespace Editor
{

BEGIN_EVENT_TABLE(C_CameraPropertyPanel,wxPanel)
END_EVENT_TABLE()

C_CameraPropertyPanel::C_CameraPropertyPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
{
	SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* pBoxSizer;
	pBoxSizer = new wxBoxSizer(wxVERTICAL);

	m_pPropertyGridManager = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	m_pProperties = new C_CameraProperties(m_pPropertyGridManager);

	const wxColour lightgray(212,208,200);
	const wxColour darkgray(113,111,100);

	m_pPropertyGridManager->GetGrid()->SetWindowStyle(wxNO_BORDER);
	m_pPropertyGridManager->GetGrid()->SetMarginColour(*wxWHITE);
	m_pPropertyGridManager->GetGrid()->SetCaptionBackgroundColour(*wxWHITE);
	m_pPropertyGridManager->GetGrid()->SetCellBackgroundColour(*wxWHITE);
	m_pPropertyGridManager->GetGrid()->SetCellTextColour(darkgray);
	m_pPropertyGridManager->GetGrid()->SetCellDisabledTextColour(lightgray);
	m_pPropertyGridManager->GetGrid()->SetLineColour(lightgray);

	pBoxSizer->Add(m_pPropertyGridManager, 1, wxALL|wxEXPAND, 0 );
	m_pPropertyGridManager->GetGrid()->SetInternalFlag(wxPG_FL_SPLITTER_PRE_SET);

	wxBoxSizer *pBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);

	m_pResetButton = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0);
	m_pResetButton->SetBitmapLabel(wxBitmap(wxT("./icons/view_reset.png"), wxBITMAP_TYPE_ANY));
	m_pResetButton->SetBitmapSelected(wxBitmap( wxT("./icons/view_resetselect.png"), wxBITMAP_TYPE_ANY));
	m_pResetButton->SetBackgroundColour(*wxWHITE);
	pBoxSizer2->Add(m_pResetButton, 0, wxALL, 5);

	m_pLabel = new wxStaticText(this, wxID_ANY, "Reset View", wxDefaultPosition, wxDefaultSize, 0);
	m_pLabel->SetForegroundColour(lightgray);
	pBoxSizer2->Add(m_pLabel, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);

	pBoxSizer->Add(pBoxSizer2, 0, wxALL, 0);

	SetSizer(pBoxSizer);
	Layout();
	pBoxSizer->Fit(this);

	Connect(wxID_ANY, wxID_ANY, wxEVT_PG_CHANGED, wxObjectEventFunction(&C_CameraPropertyPanel::OnPropertyChange), 0, this);
	Connect(wxID_ANY, wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_CameraPropertyPanel::OnCameraReset));
}

C_CameraPropertyPanel::~C_CameraPropertyPanel()
{
	delete m_pProperties;
	delete m_pPropertyGridManager;
}

void C_CameraPropertyPanel::OnPropertyChange(wxPropertyGridEvent& e)
{
	m_pProperties->OnPropertyChange(e);
}

void C_CameraPropertyPanel::OnCameraReset(wxCommandEvent& e)
{
	if (e.GetId() == m_pResetButton->GetId())
	{
		C_RenderViewport *pViewport = m_pProperties->GetAttachedViewport();
		if (pViewport)
		{
			pViewport->RevertCamera();
			pViewport->SetDirty(true);
			m_pProperties->AttachViewport(pViewport);
			pViewport->SetFocus();
		}
	}
	else
		e.Skip();
}

C_CameraProperties* C_CameraPropertyPanel::GetCameraPropertiesPtr()
{
	return m_pProperties;
}

const C_CameraProperties* C_CameraPropertyPanel::GetCameraPropertiesPtr() const
{
	return m_pProperties;
}

}
