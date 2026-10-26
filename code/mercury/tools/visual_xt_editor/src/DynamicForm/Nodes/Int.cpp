#include "stdafx.h"

#include "Int.h"

using namespace DynamicForm;

// filter number
U32 DynamicForm::I_FieldNode<C_IntNode, LfInt>::m_Filter = wxFILTER_NUMERIC;
wxSize DynamicForm::I_FieldNode<C_IntNode, LfInt>::m_Size = wxSize(50, 20);

C_IntNode::C_IntNode(LfInt* pLfInt) :
	I_FieldNode(pLfInt)
{
}