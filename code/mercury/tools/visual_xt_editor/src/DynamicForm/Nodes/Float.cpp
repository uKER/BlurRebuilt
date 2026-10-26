#include "stdafx.h"

#include "Float.h"

using namespace DynamicForm;

// filter number
U32 DynamicForm::I_FieldNode<C_FloatNode, LfFloat>::m_Filter = wxFILTER_NUMERIC;
wxSize DynamicForm::I_FieldNode<C_FloatNode, LfFloat>::m_Size = wxSize(50, 20);

C_FloatNode::C_FloatNode(LfFloat* pLfFloat) :
	I_FieldNode(pLfFloat)
{
}