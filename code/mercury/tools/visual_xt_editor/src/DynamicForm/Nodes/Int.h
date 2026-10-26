#ifndef DYNAMIC_FORM_INT_NODE_H
#define DYNAMIC_FORM_INT_NODE_H

#pragma once

#include "FieldInterface.h"

struct LfInt;

namespace DynamicForm
{

class C_IntNode : public I_FieldNode<C_IntNode, LfInt>
{
public:
	C_IntNode() : I_FieldNode<C_IntNode, LfInt>() {}
	C_IntNode(LfInt* pInt);
};

} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
