#ifndef DYNAMIC_FORM_TEXT_CONTROL_H
#define DYNAMIC_FORM_TEXT_CONTROL_H

#pragma once

namespace DynamicForm
{

enum E_ControlAction
{
	E_ControlAction_Default,
	E_ControlAction_SetFocus
};

class I_Control
{
public:
	I_Control() {}
	virtual ~I_Control() {}

	virtual void OnAction(S32 id = -1, S32 action = E_ControlAction_Default) = 0;
	virtual void RefreshControlsEdit() = 0;
	virtual void RefreshControlsSelect() {};
};

class C_TextControl : public wxTextCtrl
{
public:

	C_TextControl() : wxTextCtrl(), m_pControl(NULL) {}
	C_TextControl(I_Control* pControl,
					S32 controlId,
					wxWindow* parent,
					wxWindowID id,
					const wxString& value = "",
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxDefaultSize,
					long style = 0,
					const wxValidator& validator = wxDefaultValidator,
					const wxString& name = wxTextCtrlNameStr);
	virtual ~C_TextControl() {};


protected:

	void			OnText(wxCommandEvent& event);
	
	I_Control*		m_pControl;
	S32				m_Id;

	DECLARE_EVENT_TABLE()
};

class C_CheckBox : public wxCheckBox
{
public:

	C_CheckBox() : wxCheckBox(), m_pControl(NULL) {}
	C_CheckBox(I_Control* pControl,
				S32 controlId,
				wxWindow* parent,
				wxWindowID id,
				const wxString& label,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxValidator& val = wxDefaultValidator,
				const wxString& name = "checkBox");

	virtual ~C_CheckBox() {}
protected:

	void			OnCheck(wxCommandEvent& event);

	I_Control*		m_pControl;
	S32				m_Id;
	
	DECLARE_EVENT_TABLE()
};

class C_ComboBox : public wxComboBox
{
public:
	C_ComboBox() : wxComboBox(), m_pControl(NULL) {}
	C_ComboBox(I_Control* pControl,
				S32 controlId,
				wxWindow* parent,
				wxWindowID id,
				const wxString& value = "",
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				int n = 0,
				const wxString choices[] = NULL,
				long style = 0,
				const wxValidator& validator = wxDefaultValidator,
				const wxString& name = "comboBox");

	virtual ~C_ComboBox() {}

protected:

	void			OnComboBox(wxCommandEvent& event);
	void			OnSetFocus(wxFocusEvent& event);
	I_Control*		m_pControl;
	S32				m_Id;

	DECLARE_EVENT_TABLE()
};
				
class C_Button : public wxButton
{
public:
	C_Button() : wxButton(), m_pControl(NULL) {}
	C_Button(I_Control* pControl,
				S32 controlId,
				wxWindow* parent,
				wxWindowID id,
				const wxString& label = wxEmptyString,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxValidator& validator = wxDefaultValidator,
				const wxString& name = "button");

	virtual ~C_Button() {}

protected:

	void		OnButton(wxCommandEvent& event);
	I_Control*	m_pControl;
	S32			m_Id;
	
	DECLARE_EVENT_TABLE()
};

				

} // DynamicForm

#endif // DYNAMIC_FORM_TEXT_CONTROL_H
