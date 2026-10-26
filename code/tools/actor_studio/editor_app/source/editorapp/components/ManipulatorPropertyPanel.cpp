#include "stdafx.h"

#include <wx/aui/aui.h>
#include <wx/wxflatnotebook/wxFlatNotebook.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include <EditorApp/Components/ManipulatorPropertyPanel.h>
#include <EditorApp/Properties/ManipulatorProperties.h>

using namespace Editor;


BEGIN_EVENT_TABLE(C_ManipulatorPropertyPanel,wxPanel)
END_EVENT_TABLE()

C_ManipulatorPropertyPanel::C_ManipulatorPropertyPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
, m_pCurrentManipulatorProps(0)
, m_pageChange("")
{
	wxBoxSizer* pBoxSizer;
	pBoxSizer = new wxBoxSizer(wxVERTICAL);

	m_pPropertyGridManager = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxPG_SPLITTER_AUTO_CENTER);

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
	
	SetSizer(pBoxSizer);
	Layout();
	pBoxSizer->Fit(this);

	Connect(wxID_ANY, wxID_ANY, wxEVT_PG_CHANGED, wxObjectEventFunction(&C_ManipulatorPropertyPanel::OnPropertyChange), 0, this);
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_ManipulatorPropertyPanel::OnIdle));
}

C_ManipulatorPropertyPanel::~C_ManipulatorPropertyPanel()
{
	Btl::Map<Btl::String, I_ManipulatorProperties*>::Iterator it = m_PropertiesMap.Begin();
	while (it != m_PropertiesMap.End())
	{
		delete it->GetValue();
		m_pPropertyGridManager->RemovePage(m_pPropertyGridManager->GetPageByName(it->GetKey().Chars()));
		it++;
	}
	delete m_pPropertyGridManager;
	m_PropertiesMap.Clear();
	m_pCurrentManipulatorProps = 0;
}

void C_ManipulatorPropertyPanel::OnPropertyChange(wxPropertyGridEvent& e)
{
	if (m_pCurrentManipulatorProps)
		m_pCurrentManipulatorProps->OnPropertyChange(e);
}

void C_ManipulatorPropertyPanel::SetActivePage(const char* pName)
{
	m_pageChange = pName;
}

void C_ManipulatorPropertyPanel::RefreshProperties()
{
	if (m_pCurrentManipulatorProps)
		m_pCurrentManipulatorProps->Refresh();
	m_pPropertyGridManager->Refresh();
}

void C_ManipulatorPropertyPanel::OnIdle(wxIdleEvent &e)
{
	if (m_pageChange == "")
		return;

	Btl::Map<Btl::String, I_ManipulatorProperties*>::Iterator it = m_PropertiesMap.Find(m_pageChange);
	if (it != m_PropertiesMap.End())
	{
		m_pPropertyGridManager->GetGrid()->ClearInternalFlag(wxPG_FL_SPLITTER_PRE_SET);
		m_pPropertyGridManager->SelectPage(it->GetKey().Chars());
		m_pCurrentManipulatorProps = it->GetValue();
		m_pCurrentManipulatorProps->Refresh();
	}
	else
	{
		//wxPropertyGrid bug? - set flag to not set splitter position on non-existant page...
		m_pPropertyGridManager->GetGrid()->SetInternalFlag(wxPG_FL_SPLITTER_PRE_SET);
		m_pPropertyGridManager->SelectPage(-1);
		m_pPropertyGridManager->GetGrid()->ClearInternalFlag(wxPG_FL_SPLITTER_PRE_SET);
		m_pCurrentManipulatorProps = 0;
	}

	if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook)))
	{
		wxFlatNotebook *pParent = (wxFlatNotebook*)GetParent();
		S32 pageIndex = pParent->GetPageIndex(this);
		pParent->SetPageText(pageIndex, m_pageChange.Chars());
	}
	else if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxAuiNotebook)))
	{
		wxAuiNotebook *pParent = (wxAuiNotebook*)GetParent();
		S32 pageIndex = pParent->GetPageIndex(this);
		pParent->SetPageText(pageIndex, m_pageChange.Chars());
	}

	m_pageChange = "";
}
