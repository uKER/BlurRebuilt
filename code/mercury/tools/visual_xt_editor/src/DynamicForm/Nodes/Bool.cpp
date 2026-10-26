#include "stdafx.h"

#include "Bool.h"

#include "../Form.h"

using namespace DynamicForm;

C_BoolNode::C_BoolNode(LfBool* pLfBool) :
	C_Node(),
	m_pLfField(pLfBool),
	m_pGapSizer(NULL),
	m_pCheckBox(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, (C_BoolNode*)this);
}

C_BoolNode::~C_BoolNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

void C_BoolNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pLfField->name));
}

void C_BoolNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

bool C_BoolNode::OnBuild()
{
	m_pSizer = new wxBoxSizer(wxHORIZONTAL);

	wxString label(m_pLfField->label);

	m_pGapSizer = new wxBoxSizer(wxHORIZONTAL);
	
	m_pCheckBox = new C_CheckBox(this, 0, m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);
	m_Windows.PushBack(m_pCheckBox);

	m_pSizer->Add(m_pGapSizer, 0, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);
    m_pSizer->Add(m_pCheckBox, 0, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);
		
	GetParentSizer()->Add(m_pSizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2);

	RefreshControlsEdit();

	return true;
}

void C_BoolNode::OnLayout()
{
	m_pGapSizer->SetMinSize(wxSize(m_pParent->GetMargin(), 0));
}

void C_BoolNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

void C_BoolNode::RefreshControlsEdit()
{
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);

	if(m_pCheckBox && pField)
	{
		bool checked = atoi(Xt::AsText(pField)) != 0;
		m_pCheckBox->SetValue(checked);
	}
}

void C_BoolNode::OnAction(S32 id, S32 action)
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		XtString value;
		value = m_pCheckBox->GetValue() ? "1" : "0";
		Xt::Set(pField, value);
	}
}


void C_BoolNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_BoolNode* pNode)
{
	if (*pMonitor == pEvent->from)
		pNode->RefreshControlsEdit();
}