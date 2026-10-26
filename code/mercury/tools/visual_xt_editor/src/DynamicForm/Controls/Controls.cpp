#include "stdafx.h"

#include "Controls.h"

using namespace DynamicForm;

// text control

BEGIN_EVENT_TABLE(C_TextControl, wxTextCtrl)
    EVT_TEXT(wxID_ANY,  C_TextControl::OnText)
END_EVENT_TABLE()

C_TextControl::C_TextControl(I_Control* pControl,
							S32 controlId,
							wxWindow* parent,
							wxWindowID id,
							const wxString& value,
							const wxPoint& pos,
							const wxSize& size,
							long style,
							const wxValidator& validator,
							const wxString& name) :
	wxTextCtrl(parent, id, value, pos, size, style, validator, name),
	m_pControl(pControl),
	m_Id(controlId)
{
}

void C_TextControl::OnText(wxCommandEvent& event)
{
	if(m_pControl)
		m_pControl->OnAction(m_Id);
}

// check box

BEGIN_EVENT_TABLE(C_CheckBox, wxCheckBox)
    EVT_CHECKBOX(wxID_ANY,  C_CheckBox::OnCheck)
END_EVENT_TABLE()

C_CheckBox::C_CheckBox(I_Control* pControl,
						S32 controlId,
						wxWindow* parent,
						wxWindowID id,
						const wxString& label,
						const wxPoint& pos,
						const wxSize& size,
						long style,
						const wxValidator& val,
						const wxString& name) :
	wxCheckBox(parent, id, label, pos, size, style, val, name),
	m_pControl(pControl),
	m_Id(controlId)
{
}

void C_CheckBox::OnCheck(wxCommandEvent& event)
{
	if(m_pControl)
		m_pControl->OnAction(m_Id);
}

// combo box

BEGIN_EVENT_TABLE(C_ComboBox, wxComboBox)
    EVT_COMBOBOX(wxID_ANY,  C_ComboBox::OnComboBox)
	EVT_SET_FOCUS(C_ComboBox::OnSetFocus)
END_EVENT_TABLE()

C_ComboBox::C_ComboBox(I_Control* pControl,
				S32 controlId,
				wxWindow* parent,
				wxWindowID id,
				const wxString& value,
				const wxPoint& pos,
				const wxSize& size,
				int n,
				const wxString choices[],
				long style,
				const wxValidator& validator,
				const wxString& name) :
	wxComboBox(parent, id, value, pos, size, n, choices, style, validator, name),
	m_pControl(pControl),
	m_Id(controlId)
{
}

void C_ComboBox::OnComboBox(wxCommandEvent& event)
{
	if(m_pControl)
		m_pControl->OnAction(m_Id);
}

void C_ComboBox::OnSetFocus(wxFocusEvent& event)
{
	if(m_pControl)
		m_pControl->OnAction(m_Id, E_ControlAction_SetFocus);

	event.Skip();
}

// button

BEGIN_EVENT_TABLE(C_Button, wxButton)
    EVT_BUTTON(wxID_ANY,  C_Button::OnButton)
END_EVENT_TABLE()

C_Button::C_Button(I_Control* pControl,
				S32 controlId,
				wxWindow* parent,
				wxWindowID id,
				const wxString& label,
				const wxPoint& pos,
				const wxSize& size,
				long style,
				const wxValidator& validator,
				const wxString& name) :
	wxButton(parent, id, label, pos, size, style, validator, name),
	m_pControl(pControl),
	m_Id(controlId)
{

}

void C_Button::OnButton(wxCommandEvent& event)
{
	if(m_pControl)
		m_pControl->OnAction(m_Id);
}