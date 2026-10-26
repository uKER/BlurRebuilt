#include "stdafx.h"

#include "ArrayBody.h"
#include "Array.h"

using namespace DynamicForm;

bool C_ArrayBodyNode::OnEnable(bool enable)
{
	// array body will only enable if the array has some elements in it
	Xt::DomArray* pArray = Xt::AsArray(m_pArrayNode->m_DomMonitor);
	enable = enable && (Xt::Count(pArray) > 0);
	
	return C_Node::OnEnable(enable);
}

void C_ArrayBodyNode::OnLayout()
{
	m_iMargin = 0;
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		S32 iChildMargin = m_Children[i]->GetRequiredMargin();
		m_iMargin = m_iMargin > iChildMargin ? m_iMargin : iChildMargin;
	}
}
