#include "stdafx.h"

#include "Handle.h"

using namespace DynamicForm;

C_HandleNode::C_HandleNode(LfHandle* pLfHandle) :
	C_Node(),
	m_pLfField(pLfHandle),
	m_pComboBox(NULL),
	m_pOpenButton(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, (C_HandleNode*)this);
}

C_HandleNode::~C_HandleNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

void C_HandleNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pLfField->name));
}

void C_HandleNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

bool C_HandleNode::OnBuild()
{
	m_pSizer = new wxBoxSizer(wxHORIZONTAL);

	wxString label = GetLabel(m_pLfField->label);

	m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_pComboBox = new C_ComboBox(this, E_HandleAction_Combo, m_pForm, wxID_ANY);
	m_pOpenButton = new C_Button(this, E_HandleAction_OpenButton, m_pForm, wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize(50, 25));

    m_pSizer->Add(m_pText, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 2);
	m_pSizer->Add(m_pComboBox, 0, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);
	m_pSizer->Add(m_pOpenButton, 0, wxLEFT | wxALIGN_CENTRE_VERTICAL, 2);

	m_Windows.PushBack(m_pText);
	m_Windows.PushBack(m_pComboBox);
	m_Windows.PushBack(m_pOpenButton);
		
	GetParentSizer()->Add(m_pSizer, 0, wxALL, 2);

	RefreshControlsEdit();
	return true;
}

void C_HandleNode::PopulateCombo(bool bFast)
{
	m_ComboItems.Clear();

	XtId id = Xt::ParseId(Xt::AsText(Xt::AsField(m_DomMonitor)));
	S32 found = 0;

	Item nullItem;
	nullItem.id = 0;
	nullItem.label = "(null)";
	m_ComboItems.PushBack(nullItem);

	if (bFast)
	{
		if (Xt::DomRecord* record = m_pForm->GetDomDb()->records[id])
			AddItem(record);
	}
	else
	{
		for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&m_pForm->GetDomDb()->records); it; ++it)
			AddItem(it);
	}


	if(m_pComboBox)
	{
		m_pComboBox->Clear();

		for(S32 i = 0; i < m_ComboItems.Size(); ++i)
		{
			Item& item = m_ComboItems[i];

			if (item.id == id)
				found = i;

			m_pComboBox->Append(wxString((const char*)item.label));
		}
		
		m_pComboBox->SetSelection(found);
	}
}

void C_HandleNode::AddItem(Xt::DomRecord* pRecord)
{
	if (pRecord->state == Xt::kDrsRemoved)
		return;

	LarvaSchema*	pSchema		= m_pForm->GetDomDb()->larvaSchema;
	LarvaType*		pRecordType	= pSchema->types[pRecord->type];

	if (!pRecordType || (pRecordType->behaviour != kLtStruct))
		return;

	if (!IsBaseOf(m_pLfField->type, (LtStruct*)pRecordType))
		return;

	Item item;

	item.id = pRecord->id;

	LarvaType* pRecType = pSchema->types[pRecord->type];

	if (!pRecType)
		pRecType = m_pLfField->type;

	XtString summary = GetSummaryText(pSchema, pRecType, &pRecord->root);

	if (summary.Length() > 0)
		item.label = summary;
	else
		item.label = pRecord->name;

	m_ComboItems.PushBack(item);
}

S32 C_HandleNode::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

void C_HandleNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);
}

void C_HandleNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

void C_HandleNode::RefreshControlsEdit()
{
	GetFocus();
	PopulateCombo(m_pForm->FindFocus() != m_pComboBox);
	RefreshControlsSelect();
}

void C_HandleNode::RefreshControlsSelect()
{
	Enable(m_pParent->GetEnabled());
}



bool C_HandleNode::OnEnable(bool enable)
{
	if(m_bBuilt)
	{
		Xt::DomField* pField = Xt::AsField(m_DomMonitor);
	
		XtId id = Xt::ParseId(Xt::AsText(pField));
		bool valid = false;

		if (Xt::DomRecord* pRecord = m_pForm->GetDomDb()->records[id])
		{
			if (pRecord->state != Xt::kDrsRemoved)
			{
				LarvaSchema*	schema		= m_pForm->GetDomDb()->larvaSchema;
				LarvaType*		recordType	= schema->types[pRecord->type];

				if (recordType && (recordType->behaviour == kLtStruct))
				{
					if (IsBaseOf(m_pLfField->type, (LtStruct*)recordType))
						valid = true;
				}
			}
		}

		m_pText->Enable(enable);
		m_pComboBox->Enable(enable);
		m_pOpenButton->Enable(enable && valid);
	}

	return enable;
}

void C_HandleNode::OnAction(S32 id, S32 action)
{
	if(id == E_HandleAction_Combo)
	{
		if(action == E_ControlAction_Default)
		{
			if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
			{
				S32 selection = m_pComboBox->GetSelection();

				if ((selection >= 0) && (selection < m_ComboItems.Size()))
				{
					Item& item = m_ComboItems[selection];

					char idStr[64];
					_snprintf_s(idStr, sizeof(idStr), _TRUNCATE, "%08X", item.id);

					Xt::Set(pField, idStr);
				}
				else
					Xt::Set(pField, "");
			}

			RefreshControlsSelect();
		}
		else if(action = E_ControlAction_SetFocus)
		{
			PopulateCombo(false);
		}
	}
}


void C_HandleNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_HandleNode* pNode)
{
	if (*pMonitor == pEvent->from)
		pNode->RefreshControlsEdit();
}