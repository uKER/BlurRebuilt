#include "stdafx.h"

#include "WeakPtr.h"
#include "../Form.h"

#include "../../Common/Weak.h"

using namespace DynamicForm;

C_WeakPtrNode::C_WeakPtrNode(LfWeakPtr* pField) :
	m_pText(NULL),
	m_pComboBox(NULL),
	m_pLfField(pField),
	m_DomMonitor(DomCallback, this),
	m_WatchMonitor(DomCallback, this)
{
	m_Items.Reserve(32);
}

C_WeakPtrNode::~C_WeakPtrNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);

	for(S32 i = 0; i < m_Items.Size(); ++i)
	{
		Xt::Monitor(&m_Items[i].domMonitor, NULL);
	}
}

void C_WeakPtrNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pLfField->name));
}

void C_WeakPtrNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

bool C_WeakPtrNode::OnBuild()
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

	RefreshControlsEdit();

	return true;
}

S32 C_WeakPtrNode::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

void C_WeakPtrNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);
	m_pComboBox->SetMinSize(wxSize(200, wxDefaultCoord));
}

void C_WeakPtrNode::OnAction(S32 id, S32 action)
{
	if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
	{
		if(action == E_ControlAction_SetFocus)
		{
			PopulateCombo();
		}
		else
		{
			S32 selection = m_pComboBox->GetSelection();

			if ((selection >= 0) && (selection < m_Items.Size()))
			{
				Item& option = m_Items[selection];

				Xt::Set(pField, option.id);
			}
			else
				Xt::Set(pField, "");
		}
	}
}

bool C_WeakPtrNode::AddItem(LtStruct* type, Xt::DomBlock* block, LarvaField* labelField, Xt::DomBlock* labelWatch)
{
	if (!block || !type)
		return false;

	if (!IsBaseOf(m_pLfField->type, type))
		return false;

	XtString id = Xt::AsText(At(block, "__id"));

	if (!*id)
		return false;

	// HACK - resizing array with XtStrings in it causes memory stomping??
	if(m_Items.Size() < 32)
	{
		m_Items.PushBack();
		Item& item = m_Items.Back();

		XtString label = GetSummaryText(m_pForm->GetDomDb()->larvaSchema, labelField, labelWatch);

		item.id	= id;
		item.label	= *label ? label : id;
		item.pField	= labelField;

		Xt::Monitor(&item.domMonitor, labelWatch);
	}

	return true;
}

void C_WeakPtrNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControlsEdit();
}

void C_WeakPtrNode::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, C_WeakPtrNode* node)
{
	if (*monitor == event->from)
		node->RefreshControlsEdit();
	else if (monitor == &node->m_WatchMonitor)
	{
		int index = node->m_pComboBox->GetSelection();

		if ((index >= 0) && (index < node->m_Items.Size()))
		{
			Item* item = &node->m_Items[index];
		
			if (item->pField)
			{
				XtString label = GetSummaryText(node->m_pForm->GetDomDb()->larvaSchema, item->pField, item->domMonitor);

				item->label	= *label ? label : item->id;
				node->m_pComboBox->SetString(index, wxString((const char*)item->label));
			}
		}
	}
}

void C_WeakPtrNode::RefreshControlsEdit()
{
	Xt::DomField* pField = Xt::AsField(m_DomMonitor);

	PopulateCombo();
}

void C_WeakPtrNode::PopulateCombo()
{
	for(S32 i = 0; i < m_Items.Size(); ++i)
	{
		Xt::Monitor(&m_Items[i].domMonitor, NULL);
	}
	m_Items.Clear();

	Xt::DomField* pField = Xt::AsField(m_DomMonitor);
	XtString id = Xt::AsText(pField ? pField : 0);

	m_Items.PushBack();

	Item& nullItem = m_Items.Back();
	nullItem.id = "";
	nullItem.label = "(null)";

	if (Xt::DomRecord* record = Xt::RecordOf(m_DomMonitor))
	{
		WeakContext wc(this, m_pForm->GetDomDb()->larvaSchema);

		if (LarvaType* type = XtUt::At(&wc.schema->types, record->type))
			wc.VisitType(type, &record->root, 0, 0);
	}

	int found = -1;

	if(m_pComboBox)
	{
		m_pComboBox->Clear();

		for(S32 i = 0; i < m_Items.Size(); ++i)
		{
			Item& item = m_Items[i];

			if (item.id == id)
				found = i;

			if(m_pComboBox)
				m_pComboBox->Append(wxString((const char*)item.label));
		}

		m_pComboBox->SetSelection(found);
	}

	if(found > 0 && found <= m_Items.Size())
		Xt::Monitor(&m_WatchMonitor, m_Items[found].domMonitor);
	else
		Xt::Monitor(&m_WatchMonitor, 0);
}
