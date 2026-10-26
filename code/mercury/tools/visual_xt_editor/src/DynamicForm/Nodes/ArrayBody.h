#ifndef DYNAMIC_FORM_ARRAY_BODY_NODE_H
#define DYNAMIC_FORM_ARRAY_BODY_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"

namespace DynamicForm
{

class C_ArrayNode;

class C_ArrayBodyNode : public C_Node
{
public:
	C_ArrayBodyNode(C_ArrayNode* pArrayNode) : C_Node(), m_pArrayNode(pArrayNode) {};
	~C_ArrayBodyNode() {}

	friend class C_ArrayNode;
protected:
	bool	OnEnable(bool enable);
	void	OnLayout();

	
	C_ArrayNode*	m_pArrayNode;
};

} // DynamicForm

#endif // DYNAMIC_FORM_ARRAY_BODY_NODE_H
