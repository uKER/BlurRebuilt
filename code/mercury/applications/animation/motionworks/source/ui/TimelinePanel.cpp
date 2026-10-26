#include "stdafx.h"
#include "TimelinePanel.h"
#include "GlobalSignals.h"
#include <wx/wupdlock.h>

static const S32 kTimelineMaxValue = 32768;

C_TimelinePanel::C_TimelinePanel(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size
):
	wxPanel( parent, winId, pos, size )
{
	wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* rowSizer = new wxBoxSizer( wxHORIZONTAL );
	
	topSizer->Add( rowSizer, wxSizerFlags(1).Centre().Expand());
	
	rowSizer->Add( new wxStaticText(this, wxID_ANY, _T("Timeline:"), wxDefaultPosition, wxDefaultSize), wxSizerFlags(0).Border(wxALL, 2).Centre() );
	m_slider = new wxSlider(this, wxID_ANY, 0, 0, kTimelineMaxValue, wxDefaultPosition, wxDefaultSize, wxHORIZONTAL );
	
	Connect( m_slider->GetId(), wxEVT_SCROLL_THUMBTRACK, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_TOP, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_BOTTOM, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_LINEUP, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_LINEDOWN, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_PAGEUP, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_PAGEDOWN, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	Connect( m_slider->GetId(), wxEVT_SCROLL_CHANGED, wxCommandEventHandler(C_TimelinePanel::OnScrollEvent) );
	
	rowSizer->Add(m_slider, wxSizerFlags(1).Border(wxALL,2).Expand());
	
	m_valueText = new wxTextCtrl( this, wxID_ANY, _T("0.00"), wxDefaultPosition, wxSize(70, 24), wxBORDER_SUNKEN | wxTE_CENTRE );
	m_valueText->SetToolTip( _T("Current timeline value (between 0.0 and 1.0 inclusive)" ) );
	
	rowSizer->Add(m_valueText, wxSizerFlags(0).Border(wxALL, 4).Centre());//.Shaped());
	
	Connect( m_valueText->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(C_TimelinePanel::OnValueTextChanged) );
		
	SetSizer(topSizer);
	topSizer->SetSizeHints(this);
	
}

void C_TimelinePanel::OnScrollEvent(wxCommandEvent& e)
{
	F32 t = static_cast<F32>(m_slider->GetValue()) / static_cast<F32>(kTimelineMaxValue);
	m_valueText->ChangeValue(wxString::Format("%g", t));
	C_GlobalSignals::Instance().timelineUpdated(t);
}

void C_TimelinePanel::OnValueTextChanged(wxCommandEvent& e)
{
	double value = 0.0;
	if ( e.GetString().ToDouble( &value ) )
	{
		F32 t = static_cast<F32>(value);
		
		C_GlobalSignals::Instance().timelineUpdated(t);
		m_slider->SetValue( Maths::Clamped( static_cast<S32>(t * kTimelineMaxValue), 0, kTimelineMaxValue ) );
	}
}