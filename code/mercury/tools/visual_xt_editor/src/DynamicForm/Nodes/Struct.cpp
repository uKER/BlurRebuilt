#include "stdafx.h"

#include "Struct.h"
#include "Base.h"
#include "../Form.h"

using namespace DynamicForm;

C_StructNode::C_StructNode(LfStruct* pStruct, C_Form* pForm) :
	C_Node(),
	m_pStruct(pStruct),
	m_pText(NULL)
{
	m_pForm = pForm;
}

bool C_StructNode::OnPopulate()
{
	for(XtUt::List<LarvaBaseType>::Iterator it(&m_pStruct->type->bases); it; ++it)
		if (!AddChild(new C_BaseNode(it)))
			return false;

	for(XtUt::List<LarvaField>::Iterator it(&m_pStruct->type->fields); it; ++it)
	{
		if (it->hidden)
			continue;

		if (!BuildNode(this, it))
			return false;
	}

	return C_Node::OnPopulate();
}

void C_StructNode::OnBind(Xt::DomBlock* parentDom)
{
	Xt::DomBlock* dom = Xt::AcquireBlock(parentDom, m_pStruct->name);

	//SetEnabled(dom != 0);

	C_Node::OnBind(dom);
}

void C_StructNode::OnBind(Xt::DomArray* parentDom, int index)
{
	Xt::DomBlock* dom = Xt::AcquireBlock(parentDom, index);

	//SetEnabled(dom != 0);

	C_Node::OnBind(dom);
}

bool C_StructNode::OnBuild()
{
	wxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);
	
	{
		wxString label = GetLabel(m_pStruct->label);

		// put text in its own sizer, this allows a minimum size
		if(label.Len() > 2)
		{
			m_pText = new wxStaticText(m_pForm, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			pSizer->Add(m_pText, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL, 2 );
			m_Windows.PushBack(m_pText);
		}

		LarvaTypeLayout layout = (m_pStruct->layout == kLtlDefault) ? m_pStruct->type->layout : m_pStruct->layout;
		wxOrientation orientation;
		
		if(layout != kLtlHorizontal)
			orientation = wxVERTICAL;
		else
			orientation = wxHORIZONTAL;

		m_pSizer = new wxStaticBoxSizer(orientation, m_pForm, wxEmptyString);
		pSizer->Add(m_pSizer, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTRE_VERTICAL, 0 );
	}

	GetParentSizer()->Add(pSizer, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL, 0 );

	return true;
}

S32 C_StructNode::GetRequiredMargin() const
{
	if(m_pText)
	{
		wxSize textSize = m_pText->GetSize();
		return textSize.GetWidth();
	}
	else
		return 0;
}

void C_StructNode::OnLayout()
{
	if(m_pText)
	{
		wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
		m_pText->SetMinSize(textSize);
	}

	m_iMargin = 0;

	LarvaTypeLayout layout = (m_pStruct->layout == kLtlDefault) ? m_pStruct->type->layout : m_pStruct->layout;
	
	// were only interested in having a margin if we are a vertical layout
	if(layout != kLtlHorizontal)
	{
		for(S32 i = 0; i < m_Children.Size(); ++i)
		{
			S32 iChildMargin = m_Children[i]->GetRequiredMargin();
			m_iMargin = m_iMargin > iChildMargin ? m_iMargin : iChildMargin;
		}
	}
}