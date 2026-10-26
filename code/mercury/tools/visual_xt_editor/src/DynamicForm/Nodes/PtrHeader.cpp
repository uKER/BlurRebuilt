#include "stdafx.h"

#include "PtrHeader.h"
#include "Ptr.h"

using namespace DynamicForm;

C_PtrHeaderNode::C_PtrHeaderNode(C_PtrNode* pPtrNode) :
	C_Node(),
	m_pPtrNode(pPtrNode),
	m_pText(NULL),
	m_pComboBox(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, (C_PtrHeaderNode*)this);
}

C_PtrHeaderNode::~C_PtrHeaderNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

void C_PtrHeaderNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, "__type"));
}

bool C_PtrHeaderNode::OnBuild()
{
	wxString label = "Type: ";

	m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	
	m_pComboBox = new C_ComboBox(this, 0, m_pForm, wxID_ANY);

    m_pSizer->Add(m_pText, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 2);
	m_pSizer->Add(m_pComboBox, 1, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);

	m_Windows.PushBack(m_pText);
	m_Windows.PushBack(m_pComboBox);
		
	PopulateCombo();

	RefreshControlsEdit();

	return true;
}

void C_PtrHeaderNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);
}

void C_PtrHeaderNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

void C_PtrHeaderNode::RefreshControlsEdit()
{
	LarvaSchema*	pSchema = m_pForm->GetDomDb()->larvaSchema;
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);
	S32	found = -1;

	if(pField)
	{
		LarvaType* pLtType = pSchema->types[Xt::AsText(pField)];
		found = 0;

		if(pLtType && (pLtType->behaviour == kLtStruct))
		{

			for(S32 i = 0; i < m_ComboItems.Size(); ++i)
			{
				if (pLtType == m_ComboItems[i])
				{
					found = i;
					break;
				}
			}
		}
	}

	if(m_pComboBox)
		m_pComboBox->SetSelection(found);
}

void C_PtrHeaderNode::PopulateCombo()
{
	LarvaSchema* pSchema = m_pForm->GetDomDb()->larvaSchema;

	m_ComboItems.Clear();
	m_ComboItems.PushBack((LtStruct*)NULL);

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&pSchema->types); it; ++it)
	{
		if (it->behaviour != kLtStruct)
			continue;

		LtStruct* pLtStruct = (LtStruct*)(LarvaType*)it;

		if (!IsBaseOf(m_pPtrNode->m_pLfPtr->type, pLtStruct))
			continue;

		m_ComboItems.PushBack(pLtStruct);
	}

	//XtUt::MergeSort(&_comboItems, ComboCompare);

	if(m_pComboBox)
	{
		m_pComboBox->Clear();	
		m_pComboBox->Append("(null)");

		for(S32 i = 0; i < m_ComboItems.Size(); ++i)
		{
			if(m_ComboItems[i])
				m_pComboBox->Append((const char*)m_ComboItems[i]->label);
		}
	}
}

void C_PtrHeaderNode::OnAction(S32 id, S32 action)
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		if(action == E_ControlAction_Default)
		{
			S32 index = m_pComboBox->GetSelection();

			if ((index >= 0) && (index < m_ComboItems.Size()))
			{
				if(LtStruct* pType = m_ComboItems[index])
				{
					SetDefaults(pType, Xt::AsBlock(m_pPtrNode->m_DomMonitor), -1);
					Xt::Set(pField, pType->name);
				}
				else
					Xt::Set(pField, "");

			}
			else
				Xt::Set(pField, "");

			m_pPtrNode->OnTypeChanged();
		}
	}
}


void C_PtrHeaderNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_PtrHeaderNode* pNode)
{
	if (*pMonitor == pEvent->from)
	{
		pNode->RefreshControlsEdit();
		pNode->m_pPtrNode->OnTypeChanged();
	}
}