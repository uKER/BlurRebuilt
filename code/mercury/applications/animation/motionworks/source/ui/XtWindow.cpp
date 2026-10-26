#include "stdafx.h"
#include "XtWindow.h"
#include <wx/wxFlatNotebook/wxFlatNotebook.h>

C_XtWindow::C_XtWindow(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size,
	U32 styleFlags
):
	wxWindow( parent, winId, pos, size, styleFlags ),
	m_scarabForm(NULL),
	m_record(NULL)
{

	RegisterXtWinClasses();
	
	Connect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_XtWindow::OnSize));
	//Connect(GetId(), wxEVT_PAINT, wxPaintEventHandler(C_XtWindow::OnPaint));
}

C_XtWindow::~C_XtWindow()
{
	delete m_scarabForm;
}

void C_XtWindow::SetRecord( Xt::DomRecord* record )
{
	ScarabForm* sf = CreateScarabForm();
	
	ResizeScarabFormToFit();
	
	if ( record )
	{
		sf->SetRecord( record );
	}
	m_record = record;
	UpdateCaption();
}

Xt::DomRecord* C_XtWindow::GetRecord() const
{
	return m_record;
}

void C_XtWindow::UpdateCaption()
{
	wxString caption(m_defaultCaption);
	if (m_record)
	{
		caption = m_record->name.Text();
	}
	
	const wxClassInfo* parentClassInfo = GetParent()->GetClassInfo();
	if (!stricmp(parentClassInfo->GetClassName(), "wxFlatNotebook"))
	{
		wxFlatNotebook* parentNotebook = static_cast<wxFlatNotebook*>(GetParent());
		
		int pageIndex = parentNotebook->GetPageIndex(this);
		if (pageIndex >= 0)
		{
			parentNotebook->SetPageText(pageIndex, caption);
		}
	}	
}

void C_XtWindow::OnSize(wxSizeEvent& e)
{
	ResizeScarabFormToFit();
}

void C_XtWindow::OnPaint(wxPaintEvent& e)
{
	
}

ScarabForm* C_XtWindow::CreateScarabForm()
{
	delete m_scarabForm;
	m_scarabForm = new ScarabForm( new C_XtScarabFormContext( this ) );
	
	m_scarabForm->Create( (HWND) GetHandle() );
	
	return m_scarabForm;
}

void C_XtWindow::ResizeScarabFormToFit()
{
	if ( m_scarabForm != NULL )
	{
		WinRect rcClient( (HWND) GetHandle() );
		SetWindowPos( *m_scarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW );
	}
}
