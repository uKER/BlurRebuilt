#include "stdafx.h"

#include "Flags.h"
#include "FlagBool.h"

#include "../Form.h"

using namespace DynamicForm;

C_FlagBoolNode::C_FlagBoolNode(LarvaOption* pField) :
	m_pField(pField),
	m_pCheckBox(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, this);
}

C_FlagBoolNode::~C_FlagBoolNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}


bool C_FlagBoolNode::OnBuild()
{

	wxString label(m_pField->label);

	m_pCheckBox = new C_CheckBox(this, 0, m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);
	m_Windows.PushBack(m_pCheckBox);

	GetParentSizer()->Add(m_pCheckBox, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2);

	RefreshControlsEdit();

	return true;
}

void C_FlagBoolNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pField->name));
}

void C_FlagBoolNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

void C_FlagBoolNode::RefreshControlsEdit()
{
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);

	if(m_pCheckBox && pField)
	{
		bool checked = atoi(Xt::AsText(pField)) != 0;
		m_pCheckBox->SetValue(checked);
	}
}

void C_FlagBoolNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

void C_FlagBoolNode::OnAction(S32 id, S32 action)
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		XtString value;
		value = m_pCheckBox->GetValue() ? "1" : "0";
		Xt::Set(pField, value);
	}
}

void C_FlagBoolNode::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, C_FlagBoolNode* node)
{
	if (*monitor == event->from)
		node->RefreshControlsEdit();
}


