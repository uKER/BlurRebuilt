#include "stdafx.h"

#include "Ptr.h"
#include "PtrHeader.h"
#include "PtrBody.h"
#include "Base.h"

#include "../../Common/GenerateId.h"

using namespace DynamicForm;

C_PtrNode::C_PtrNode(LfPtr* pLfPtr) :
	C_Node(),
	m_pLfPtr(pLfPtr),
	m_pHeader(NULL),
	m_pBody(NULL)
{

}

C_PtrNode::~C_PtrNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

bool C_PtrNode::OnPopulate()
{
	assert(!m_pHeader && !m_pBody);

	if ((m_pHeader = AddChild(new C_PtrHeaderNode(this))) == 0)
		return false;

	if ((m_pBody = AddChild(new C_PtrBodyNode)) == 0)
		return false;

	return true;
}


void C_PtrNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireBlock(parentDom, m_pLfPtr->name));
}

void C_PtrNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireBlock(parentDom, index));
}

bool C_PtrNode::OnBuild()
{
	// for the text label and the controls along top
	{
		wxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);
		wxString label = GetLabel(m_pLfPtr->label);

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
		wxBoxSizer* pBodySizer = new wxStaticBoxSizer(wxVERTICAL, m_pForm, wxEmptyString);

		pSizer->Add(m_pGapSizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );
		pSizer->Add(pBodySizer, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 2 );

		m_pBody->m_pSizer = pBodySizer;
		//m_pBody->m_iMargin = 100;

		GetParentSizer()->Add(pSizer, 0, wxDOWN | wxRIGHT | wxLEFT | wxALIGN_CENTRE_VERTICAL, 0 );
	}

	return true;
}

S32 C_PtrNode::GetRequiredMargin() const
{
	wxSize textSize = m_pText->GetSize();
	return textSize.GetWidth();
}

void C_PtrNode::OnLayout()
{
	wxSize textSize = m_pParent->GetMargin() ? wxSize(m_pParent->GetMargin(), wxDefaultCoord) : wxDefaultSize;
	m_pText->SetMinSize(textSize);

	m_pGapSizer->SetMinSize(wxSize(m_pParent->GetMargin(), wxDefaultCoord));
}

bool C_PtrNode::OnEnable(bool enable)
{
	return C_Node::OnEnable(enable);
}

void C_PtrNode::SetDom(Xt::DomBlock* pBlock)
{
	Xt::Monitor(&m_DomMonitor, pBlock);

	GenerateIdForBlock(pBlock, m_pLfPtr);

	//Form()->BeginBuild();

	PopulateBody();
	C_Node::OnBind(pBlock);
}

bool C_PtrNode::PopulateBody()
{
	if(!m_pBody)
		return false;

	XtString typeName = Xt::AsText(Xt::AcquireField(AsBlock(m_DomMonitor), "__type"));

	m_pBody->ClearChildren();

	if (LarvaType* pLtType = m_pForm->GetDomDb()->larvaSchema->types[typeName])
	{
		if (pLtType->behaviour == kLtStruct)
		{
			LtStruct* pType = (LtStruct*)pLtType;

			if (IsBaseOf(m_pLfPtr->type, pType))
			{
				for(XtUt::List<LarvaBaseType>::Iterator it(&pType->bases); it; ++it)
					if (!m_pBody->AddChild(new C_BaseNode(it)))
						return false;
		
				for(XtUt::List<LarvaField>::Iterator it(&pType->fields); it; ++it)
					if (!BuildNode(m_pBody, it))
						return false;

				if (!m_pBody->Populate())
					return false;
			}
		}
	}

	return true;
}

void C_PtrNode::OnTypeChanged()
{
	m_pForm->Freeze();

	PopulateBody();

	m_pBody->Bind(Xt::AsBlock(m_DomMonitor));

	m_pBody->m_pSizer->Clear();
	m_pBody->Build();

	m_pForm->UpdateLayout();
	m_pForm->Thaw();
}
