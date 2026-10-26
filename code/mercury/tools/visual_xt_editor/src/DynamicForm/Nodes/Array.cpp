#include "stdafx.h"

#include "Array.h"
#include "ArrayHeader.h"
#include "ArrayBody.h"

#include "../Form.h"
#include "../../Common/GenerateId.h"

using namespace DynamicForm;

C_ArrayNode::C_ArrayNode(LfArray* pLfArray) :
	C_Node(),
	m_pLfArray(pLfArray),
	m_pHeader(NULL),
	m_pBody(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, (C_ArrayNode*)this);
}

C_ArrayNode::~C_ArrayNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

bool C_ArrayNode::OnPopulate()
{
	assert(!m_pHeader && !m_pBody);

	if ((m_pHeader = AddChild(new C_ArrayHeaderNode(this))) == 0)
		return false;

	if ((m_pBody = AddChild(new C_ArrayBodyNode(this))) == 0)
		return false;

	if (!BuildNode(m_pBody, m_pLfArray->field))
		return false;

	return C_Node::OnPopulate();
}


void C_ArrayNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireArray(parentDom, m_pLfArray->name));
}

void C_ArrayNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireArray(parentDom, index));
}

bool C_ArrayNode::OnBuild()
{
	// for the text label and the controls along top
	{
		wxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);
		wxString label = GetLabel(m_pLfArray->label);

		m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		m_Windows.PushBack(m_pText);

		wxSizer* pControlSizer = new wxBoxSizer(wxHORIZONTAL);

		pSizer->Add(m_pText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
		pSizer->Add(pControlSizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );

		m_pHeader->m_pSizer = pControlSizer;

		GetParentSizer()->Add(pSizer, 0, wxUP | wxRIGHT | wxLEFT | wxALIGN_CENTRE_VERTICAL, 0 );
	}

	// for gap & body
	{
		wxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);

		// gap
		m_pGapSizer = new wxBoxSizer(wxHORIZONTAL);
		
		// body
		wxBoxSizer* pBodySizer = new wxBoxSizer(wxVERTICAL);

		pSizer->Add(m_pGapSizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
		pSizer->Add(pBodySizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );

		m_pBody->m_pSizer = pBodySizer;
		//m_pBody->m_iMargin = 100;

		GetParentSizer()->Add(pSizer, 0, wxDOWN | wxRIGHT | wxLEFT | wxALIGN_CENTRE_VERTICAL, 0 );
	}

	return true;
}

S32 C_ArrayNode::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

void C_ArrayNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);

	m_pGapSizer->SetMinSize(wxSize(m_pParent->GetMargin(), wxDefaultCoord));
}

bool C_ArrayNode::OnEnable(bool enable)
{
	return C_Node::OnEnable(enable);
}

void C_ArrayNode::SetDom(Xt::DomArray* pArray)
{
	Xt::Monitor(&m_DomMonitor, pArray);
	m_pHeader->RefreshControlsEdit();
}

void C_ArrayNode::OnIndexChanged()
{
	if(!m_pBody)
		return;

	m_pBody->Bind(AsArray(m_DomMonitor), m_pHeader->GetIndex());

	if (Xt::DomArray* pArray = Xt::AsArray(m_DomMonitor))
		GenerateIdForBlock(Xt::AsBlock(At(pArray, m_pHeader->GetIndex())), m_pLfArray);
}

void C_ArrayNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_ArrayNode* pNode)
{
	if (*pMonitor == pEvent->from)
	{
		pNode->m_pHeader->RefreshControlsEdit();
	}
}