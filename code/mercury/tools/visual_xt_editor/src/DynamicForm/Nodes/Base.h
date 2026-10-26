#ifndef DYNAMIC_FORM_BASE_NODE_H
#define DYNAMIC_FORM_BASE_NODE_H

#pragma once

#include "Node.h"

namespace DynamicForm
{

class C_BaseNode : public C_Node
{
public:

	C_BaseNode() : C_Node() {};
	C_BaseNode(LarvaBaseType* pBase);
	virtual ~C_BaseNode();

	
protected:

	virtual bool	OnPopulate();
	virtual void	OnBind(Xt::DomBlock* parentDom);
	
	LarvaBaseType*	m_pBaseType;
};

} // DynamicForm

#endif // DYNAMIC_FORM_BASE_NODE_H
