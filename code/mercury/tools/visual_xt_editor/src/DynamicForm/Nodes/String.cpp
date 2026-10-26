#include "stdafx.h"

#include "String.h"

using namespace DynamicForm;

// filter number
U32 DynamicForm::I_FieldNode<C_StringNode, LfString>::m_Filter = wxFILTER_NONE;
wxSize DynamicForm::I_FieldNode<C_StringNode, LfString>::m_Size = wxSize(100, 20);

C_StringNode::C_StringNode(LfString* pLfString) :
	I_FieldNode(pLfString)
{
}