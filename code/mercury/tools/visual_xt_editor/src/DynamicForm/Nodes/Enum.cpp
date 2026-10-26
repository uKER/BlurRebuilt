#include "stdafx.h"

#include "Enum.h"

using namespace DynamicForm;

C_EnumNode::C_EnumNode(LfEnum* pLfEnum) :
	C_Node(),
	m_pText(NULL),
	m_pLfField(pLfEnum),
	m_pComboBox(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, (C_EnumNode*)this);
}

C_EnumNode::~C_EnumNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

void C_EnumNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pLfField->name));
}

void C_EnumNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

bool C_EnumNode::OnBuild()
{
	m_pSizer = new wxBoxSizer(wxHORIZONTAL);

	wxString label = GetLabel(m_pLfField->label);

	m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	
	m_pComboBox = new C_ComboBox(this, 0, m_pForm, wxID_ANY);

    m_pSizer->Add(m_pText, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 2);
	m_pSizer->Add(m_pComboBox, 1, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);

	m_Windows.PushBack(m_pText);
	m_Windows.PushBack(m_pComboBox);
		
	GetParentSizer()->Add(m_pSizer, 0, wxALL, 2);

	// populate combo box
	{
		m_ComboItems.Clear();
		for(XtUt::List<LarvaOption>::Iterator it(&m_pLfField->type->options); it; ++it)
			m_ComboItems.PushBack((LarvaOption*)it);

		m_Default = 0;

		for(S32 i = 0; i < m_ComboItems.Size(); i++)
		{
			if (m_ComboItems[i] == m_pLfField->type->options[0])
			{
				m_Default = i;
				break;
			}
		}

		for(S32 i = 0; i < m_ComboItems.Size(); i++)
			m_pComboBox->Append(wxString((const char*)m_ComboItems[i]->label));
	}


	RefreshControlsEdit();

	return true;
}

S32 C_EnumNode::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

void C_EnumNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);
	m_pComboBox->SetMinSize(wxSize(200, wxDefaultCoord));
}

void C_EnumNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

void C_EnumNode::RefreshControlsEdit()
{
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);
	S32	found = m_Default;

	XtString optName = Xt::AsText(pField);

	for(S32 i = 0; i < m_ComboItems.Size(); ++i)
	{
		if (optName == m_ComboItems[i]->name)
			found = i;
	}

	if(m_pComboBox)
		m_pComboBox->SetSelection(found);
}

void C_EnumNode::OnAction(S32 id, S32 action)
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		//XtString value;
		//value = m_pCheckBox->GetValue() ? "1" : "0";
		//Xt::Set(pField, value);

		S32 selection = m_pComboBox->GetSelection();

		if ((selection >= 0) && (selection < m_ComboItems.Size()))
		{
			LarvaOption* option = m_ComboItems[selection];

			Xt::Set(pField, option->name);
		}
		else
			Xt::Set(pField, "");
	}
}


void C_EnumNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_EnumNode* pNode)
{
	if (*pMonitor == pEvent->from)
		pNode->RefreshControlsEdit();
}