#include "stdafx.h"

#include "Base.h"

using namespace DynamicForm;

C_BaseNode::C_BaseNode(LarvaBaseType* pBaseType) :
	m_pBaseType(pBaseType)
{
}

C_BaseNode::~C_BaseNode()
{
}

bool C_BaseNode::OnPopulate()
{
	for(XtUt::List<LarvaBaseType>::Iterator it(&m_pBaseType->type->bases); it; ++it)
		if (!AddChild(new C_BaseNode(it)))
			return false;

	for(XtUt::List<LarvaField>::Iterator it(&m_pBaseType->type->fields); it; ++it)
		if (!BuildNode(this, it))
			return false;

	return C_Node::OnPopulate();
}

void C_BaseNode::OnBind(Xt::DomBlock* parentDom)
{
	C_Node::OnBind(parentDom);
}
