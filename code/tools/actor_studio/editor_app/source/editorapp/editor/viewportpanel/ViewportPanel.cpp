#include "stdafx.h"

#include <EditorApp/Editor/viewportpanel/ViewportPanel.h>
#include <EditorApp/editor/viewportpanel/ViewportMenuBar.h>

#include <EditorApp/editor/RenderViewport.h>

using namespace Editor;


C_ViewportPanel::C_ViewportPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style & ~wxSIMPLE_BORDER)
, m_pMenuBar(0)
, m_pViewport(0)
{
	m_BorderColour = wxColour(159,156,150,255);

	wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(pSizer);

	m_pMenuBar = new C_ViewportMenuBar(this);
	m_pMenuBar->InitialiseUI();

	Layout();

	//wxSIMPLE_BORDER sets the WS_BORDER win32 style, resulting in a black border...
	//Disable this and manually draw the border in our OnPaint event using our own colour scheme
	if (style & wxSIMPLE_BORDER)
	{
		Connect(GetId(), wxEVT_PAINT, wxPaintEventHandler(C_ViewportPanel::OnPaint));
		Connect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_ViewportPanel::OnSize));
	}
}

C_ViewportPanel::~C_ViewportPanel()
{
	SAFE_DELETE_PTR(m_pMenuBar);
}


void C_ViewportPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	if (!dc.IsOk())
		return;

	{
		wxSize size = GetSize();

		if (wxWindow::FindFocus() == m_pViewport)
			dc.SetPen(wxPen(*wxBLACK));
		else
			dc.SetPen(wxPen(m_BorderColour));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(0, 0, size.x, size.y);
	}
}

void C_ViewportPanel::OnSize(wxSizeEvent &e)
{
	wxSize size = e.GetSize();
	if (GetSizer())
		GetSizer()->SetDimension(1, 1, size.x-2, size.y-2);
}

void C_ViewportPanel::SetViewport(C_RenderViewport *pViewport)
{
	if (m_pViewport)
		GetSizer()->Detach(m_pViewport);

	m_pViewport = pViewport;
	m_pViewport->Reparent(this);
	GetSizer()->Add(m_pViewport, 1, wxEXPAND, 0);
}

C_RenderViewport* C_ViewportPanel::GetViewport()
{
	return m_pViewport;
}