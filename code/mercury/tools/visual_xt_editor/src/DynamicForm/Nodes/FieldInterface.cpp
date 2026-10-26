#include "stdafx.h"

#include "Float.h"
#include "../Form.h"

using namespace DynamicForm;

C_FloatNode::C_FloatNode(LfFloat* pLfFloat) :
	C_Node(),
	m_pLfFloat(pLfFloat),
	m_pControl(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, this);
}

void C_FloatNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pLfFloat->name));
}

void C_FloatNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

bool C_FloatNode::OnBuild()
{
	m_pSizer = new wxBoxSizer(wxHORIZONTAL);

	wxString label = GetLabel(m_pLfFloat->name);

	wxControl* pStaticText = new wxStaticText(m_pForm, wxID_ANY, label);
	m_pControl = new C_TextControl(this, m_pForm, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));

    m_pSizer->Add(pStaticText, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 5);
	m_pSizer->Add(m_pControl, 1, wxLEFT | wxALIGN_CENTRE_VERTICAL, 5);
		
	GetParentSizer()->Add(m_pSizer, 0, wxALL, 5);

	SyncControls();

	return true;
}

void C_FloatNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	SyncControls();
}

void C_FloatNode::SyncControls()
{
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);

	if(m_pControl)
		m_pControl->ChangeValue(wxString((const char*)pField->value));
}

void C_FloatNode::OnValueChanged()
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		XtString value(m_pControl->GetValue().c_str());
		Xt::Set(pField, value);
	}
}

void C_FloatNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_FloatNode* pNode)
{
	if (*pMonitor == pEvent->from)
		pNode->SyncControls();
}
