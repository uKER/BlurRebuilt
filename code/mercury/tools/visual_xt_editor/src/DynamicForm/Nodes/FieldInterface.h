#ifndef DYNAMIC_FORM_FIELD_NODE_H
#define DYNAMIC_FORM_FIELD_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"

namespace DynamicForm
{

template<class Derived, class Field>
class I_FieldNode : public C_Node, public I_Control
{
public:

	I_FieldNode() : C_Node() {};
	I_FieldNode(Field* pField);
	virtual ~I_FieldNode();

	virtual void OnAction(S32 id, S32 action);

	virtual S32	GetRequiredMargin() const;

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, Derived* pNode);

protected:

	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();

	void	OnLayout();

	void	RefreshControlsEdit();
	void	SetDom(Xt::DomField* field);

	Xt::DomMonitor	m_DomMonitor;
	wxStaticText*	m_pText;
	C_TextControl*	m_pControl;
	Field*			m_pLfField;

	static U32		m_Filter;
	static wxSize	m_Size;
};

template<class Derived, class Field>
I_FieldNode<Derived, Field>::I_FieldNode(Field* pLfFloat) :
	C_Node(),
	m_pLfField(pLfFloat),
	m_pText(NULL),
	m_pControl(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, (Derived*)this);
}

template<class Derived, class Field>
I_FieldNode<Derived, Field>::~I_FieldNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pLfField->name));
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

template<class Derived, class Field>
bool I_FieldNode<Derived, Field>::OnBuild()
{
	m_pSizer = new wxBoxSizer(wxHORIZONTAL);

	wxString label = GetLabel(m_pLfField->label);

	m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_pControl = new C_TextControl(this, 0, m_pForm, wxID_ANY, wxEmptyString,
        wxDefaultPosition, m_Size, wxTE_PROCESS_ENTER, wxTextValidator(m_Filter));

    m_pSizer->Add(m_pText, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 2);
	m_pSizer->Add(m_pControl, 1, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);

	m_Windows.PushBack(m_pText);
	m_Windows.PushBack(m_pControl);

	HideWindows();
		
	GetParentSizer()->Add(m_pSizer, 0, wxALL, 2);

	RefreshControlsEdit();

	return true;
}

template<class Derived, class Field>
S32 I_FieldNode<Derived, Field>::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);

	ShowWindows();
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::RefreshControlsEdit()
{
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);

	if(m_pControl)
		m_pControl->ChangeValue(pField ? wxString((const char*)pField->value) : wxEmptyString);
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::OnAction(S32 id, S32 action)
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		XtString value(m_pControl->GetValue().c_str());
		Xt::Set(pField, value);
	}
}

template<class Derived, class Field>
void I_FieldNode<Derived, Field>::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, Derived* pNode)
{
	if (*pMonitor == pEvent->from)
		pNode->RefreshControlsEdit();
}

} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
