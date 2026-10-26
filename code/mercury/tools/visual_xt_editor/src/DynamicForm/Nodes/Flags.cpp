#include "stdafx.h"

#include "Flags.h"
#include "FlagBool.h"

#include "../Form.h"

using namespace DynamicForm;

C_FlagsNode::C_FlagsNode(LfFlags* pLfFlags) :
	m_pLfFlags(pLfFlags)
{
}

C_FlagsNode::~C_FlagsNode()
{
}


bool C_FlagsNode::OnPopulate()
{
	for(XtUt::List<LarvaOption>::Iterator it(&m_pLfFlags->type->options); it; ++it)
		if (!AddChild(new C_FlagBoolNode(it)))
			return false;

	return true;
}

bool C_FlagsNode::OnBuild()
{
	// for gap & body
	{
		wxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);
		
		wxString label = GetLabel(m_pLfFlags->label);

		m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		m_Windows.PushBack(m_pText);

		// body
		m_pSizer = new wxStaticBoxSizer(wxVERTICAL, m_pForm, wxEmptyString);

		pSizer->Add(m_pText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
		pSizer->Add(m_pSizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );

		GetParentSizer()->Add(pSizer, 0, wxDOWN | wxRIGHT | wxLEFT | wxALIGN_CENTRE_VERTICAL, 0 );
	}

	return true;
}

S32 C_FlagsNode::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

void C_FlagsNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);
}

void C_FlagsNode::OnBind(Xt::DomBlock* parentDom)
{
	Xt::DomBlock* pDomBlock = Xt::AcquireBlock(parentDom, m_pLfFlags->name);

	C_Node::OnBind(pDomBlock);
}

void C_FlagsNode::OnBind(Xt::DomArray* parentDom, int index)
{
	Xt::DomBlock* pDomBlock = AcquireBlock(parentDom, index);

	C_Node::OnBind(pDomBlock);
}

