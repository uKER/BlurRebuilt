#include "stdafx.h"

#include <system/misc/Progress.h>
#include <EditorApp/Components/Progress.h>

using namespace Editor;

BEGIN_EVENT_TABLE(C_ProgressStatusBar, wxStatusBar)
    EVT_SIZE(C_ProgressStatusBar::OnSize)
END_EVENT_TABLE() 

C_ProgressStatusBar::C_ProgressStatusBar(wxWindow *parent, wxWindowID id, long style, const wxString& name)
: wxStatusBar(parent, id, style, name)
, Progress::I_ProgressBar(0)
{
	static const S32 fieldWidths[2] = {-1, 150};
	static const S32 fieldStyles[2] = {wxSB_FLAT, wxSB_FLAT};

	SetFieldsCount(2);
	SetStatusWidths(2, fieldWidths);
	SetStatusStyles(2, fieldStyles);

	wxRect rect;
	GetFieldRect(1, rect);

	m_pGauge = new wxGauge(this, wxID_ANY, 100, wxPoint(rect.x+2, rect.y+2), wxSize(rect.width-4, rect.height-4), wxGA_HORIZONTAL|wxGA_SMOOTH);
	m_pGauge->Hide();
}

C_ProgressStatusBar::~C_ProgressStatusBar()
{
}

void C_ProgressStatusBar::OnSize(wxSizeEvent& e)
{
	if (!m_pGauge)
		return;

	wxRect rect;
	GetFieldRect(1, rect);

	m_pGauge->SetSize(rect.x+2, rect.y+2, rect.width-4, rect.height-4);

	e.Skip();
}
bool C_ProgressStatusBar::ProgressNotify(int iHierarchyDepth, const F32 percent, const Util::C_String &description)
{
	if (m_pGauge)
	{
		if (percent < 0.f)
			m_pGauge->Pulse();
		else if (percent > 100.f)
			m_pGauge->Hide();
		else
		{
			m_pGauge->Show();
			m_pGauge->SetValue((S32)(percent*0.01f*m_pGauge->GetRange()));
		}
	}

	SetStatusText(wxString(description.Chars()));

	return true;
}

void C_ProgressStatusBar::SetStatusText(const wxString& text, int number)
{
	wxColour c = wxSystemSettings::GetColour(wxSYS_COLOUR_MENU);
	::SendMessage((HWND)GetHandle(), SB_SETBKCOLOR, 0, (LPARAM)RGB(c.Red(),c.Green(),c.Blue()));
	wxStatusBar::SetStatusText(text, number);
}