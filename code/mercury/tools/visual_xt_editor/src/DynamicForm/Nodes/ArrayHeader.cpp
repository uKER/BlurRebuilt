#include "stdafx.h"

#include "ArrayHeader.h"
#include "Array.h"

#include "../Form.h"

using namespace DynamicForm;

C_ArrayHeaderNode::C_ArrayHeaderNode(C_ArrayNode* pArray) :
	C_Node(),
	m_pArrayNode(pArray),
	m_pComboBox(NULL),
	m_pAdd(NULL),
	m_pRemove(NULL),
	m_iIndex(0)
{
	m_DomMonitor.SetCallback(DomCallback, this);
}

C_ArrayHeaderNode::~C_ArrayHeaderNode()
{
}

bool C_ArrayHeaderNode::OnBuild()
{
	m_pComboBox = new C_ComboBox(this, E_ArrayHeaderAction_Combo, m_pForm, wxID_ANY);
	m_pAdd = new C_Button(this, E_ArrayHeaderAction_Add, m_pForm, wxID_ANY, wxT("Add"), wxDefaultPosition, wxSize(50, 25));
	m_pRemove = new C_Button(this, E_ArrayHeaderAction_Remove, m_pForm, wxID_ANY, wxT("Remove"), wxDefaultPosition, wxSize(50, 25));

	m_pPrevious = new C_Button(this, E_ArrayHeaderAction_Previous, m_pForm, wxID_ANY, wxT("<"), wxDefaultPosition, wxSize(25, 25));
	m_pNext = new C_Button(this, E_ArrayHeaderAction_Next, m_pForm, wxID_ANY, wxT(">"), wxDefaultPosition, wxSize(25, 25));

	m_pSizer->Add(m_pComboBox, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
	m_pSizer->Add(m_pAdd, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
	m_pSizer->Add(m_pRemove, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
	m_pSizer->Add(m_pPrevious, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
	m_pSizer->Add(m_pNext, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );

	m_Windows.PushBack(m_pComboBox);
	m_Windows.PushBack(m_pAdd);
	m_Windows.PushBack(m_pRemove);
	m_Windows.PushBack(m_pPrevious);
	m_Windows.PushBack(m_pNext);

	RefreshControlsEdit();
	
	return true;
}

bool C_ArrayHeaderNode::OnEnable(bool enable)
{
	m_iIndex = GetIndex();

	if(m_bBuilt)
	{
		Xt::DomArray* pArray = Xt::AsArray(m_pArrayNode->m_DomMonitor);
		S32 count = Xt::Count(pArray);
		bool parentEnable = m_pArrayNode->GetParent()->GetEnabled();
		
		m_pComboBox->Enable(parentEnable && count > 0);
		// add button should be enabled if array node's parent is enabled
		m_pAdd->Enable(parentEnable);
		m_pRemove->Enable(parentEnable && count > 0);
		m_pPrevious->Enable(parentEnable && count > 0 && m_iIndex != 0);
		m_pNext->Enable(parentEnable && count > 0 && m_iIndex != (count - 1));

	}

	return enable;
}

void C_ArrayHeaderNode::RefreshControlsEdit()
{
	Xt::DomArray* pArray = Xt::AsArray(m_pArrayNode->m_DomMonitor);

	if(m_pComboBox)
		m_pComboBox->Clear();
	
	if(pArray)
	{
		LarvaSchema* pSchema = m_pForm->GetDomDb()->larvaSchema;

		for(Xt::DomArray::Iterator it(pArray); it; ++it)
		{
			XtString summary = GetSummaryText(pSchema, m_pArrayNode->m_pLfArray->field, it);
			XtString text;

			XtsFormat(text, 128, "%i: %s", it.index, (const char*)summary);

			if(m_pComboBox)
				m_pComboBox->Append(wxString((const char*)text));
		}
	}

	RefreshControlsSelect();
}

void C_ArrayHeaderNode::RefreshControlsSelect()
{
	if(m_pComboBox && m_pComboBox->GetCount() > 0)
		m_pComboBox->SetSelection(GetIndex());

	m_pArrayNode->OnIndexChanged();
	m_pArrayNode->Enable(m_pArrayNode->GetParent()->GetEnabled());
}

S32 C_ArrayHeaderNode::GetIndex()
{
	Xt::DomArray* pArray = Xt::AsArray(m_pArrayNode->m_DomMonitor);

	if (!pArray)
		return -1;

	if (m_iIndex < 0)
		return 0;

	if (m_iIndex >= (S32)Count(pArray))
		return (S32)(Count(pArray) - 1);

	return m_iIndex;
}

void C_ArrayHeaderNode::OnAction(S32 id, S32 action)
{
	Xt::DomArray* pArray = Xt::AsArray(m_pArrayNode->m_DomMonitor);

	switch(id)
	{
		case E_ArrayHeaderAction_Combo:
			m_iIndex = m_pComboBox->GetSelection();
			RefreshControlsSelect();
			break;

		case E_ArrayHeaderAction_Add:
			m_iIndex = (S32)Count(pArray);
			Xt::Append(pArray, new Xt::DomBlock);
			SetDefaults(m_pArrayNode->m_pLfArray, pArray, m_iIndex);
			
			RefreshControlsEdit();
			break;

		case E_ArrayHeaderAction_Remove:
			Xt::Remove(pArray, GetIndex());
			m_iIndex = GetIndex();

			RefreshControlsEdit();
			break;

		case E_ArrayHeaderAction_Next:
			++m_iIndex;
			m_iIndex = GetIndex();
			RefreshControlsSelect();
			break;

		case E_ArrayHeaderAction_Previous:
			--m_iIndex;
			m_iIndex = GetIndex();
			RefreshControlsSelect();
			break;

	}
}


void C_ArrayHeaderNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_ArrayHeaderNode* pNode)
{
	/*if (*pMonitor == pEvent->from)
	{
		pNode->m_pHeader->SyncControls();
		pNode->SyncControls();
	}*/
}