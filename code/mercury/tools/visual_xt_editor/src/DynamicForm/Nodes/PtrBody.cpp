#include "stdafx.h"

#include "PtrBody.h"
#include "../Form.h"

using namespace DynamicForm;

bool C_PtrBodyNode::OnBuild()
{
	//m_pSizer = new wxStaticBoxSizer(wxVERTICAL, m_pForm, wxEmptyString);

	//GetParentSizer()->Add(m_pSizer, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL, 0 );

	return true;
}

void C_PtrBodyNode::OnLayout()
{
	m_iMargin = 0;
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		S32 iChildMargin = m_Children[i]->GetRequiredMargin();
		m_iMargin = m_iMargin > iChildMargin ? m_iMargin : iChildMargin;
	}

	//if(m_Children.Size() == 0)
	//	GetParentSizer()->Hide(m_pSizer, true);
	//else
	//	GetParentSizer()->Show(m_pSizer, true);
}
