#include "stdafx.h"

#include "Record.h"
#include "Base.h"

#include "../Form.h"

using namespace DynamicForm;

C_RecordNode::C_RecordNode(LtStruct* pType, C_Form* pForm) :
	C_Node(),
	m_pType(pType)
{
	m_pForm = pForm;
}

bool C_RecordNode::OnPopulate()
{
	for(XtUt::List<LarvaBaseType>::Iterator it(&m_pType->bases); it; ++it)
		if (!AddChild(new C_BaseNode(it)))
			return false;

	for(XtUt::List<LarvaField>::Iterator it(&m_pType->fields); it; ++it)
		if (!BuildNode(this, it))
			return false;

	return C_Node::OnPopulate();
}

bool C_RecordNode::OnBuild()
{
	wxSizer* pSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxString label((const char*)m_pType->label);
		m_pSizer = new wxStaticBoxSizer(wxVERTICAL, m_pForm, label);
		pSizerTop->Add(m_pSizer, 0, wxALL, 10);
	}
	m_pForm->SetSizer(pSizerTop);

	return true;
}

void C_RecordNode::OnLayout()
{
	m_iMargin = 0;
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		S32 iChildMargin = m_Children[i]->GetRequiredMargin();
		m_iMargin = m_iMargin > iChildMargin ? m_iMargin : iChildMargin;
	}
}
