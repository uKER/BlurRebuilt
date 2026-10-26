#ifndef DYNAMIC_FORM_FLOAT_NODE_H
#define DYNAMIC_FORM_FLOAT_NODE_H

#pragma once

#include "FieldInterface.h"

struct LfFloat;

namespace DynamicForm
{

class C_FloatNode : public I_FieldNode<C_FloatNode, LfFloat>
{
public:
	C_FloatNode() : I_FieldNode<C_FloatNode, LfFloat>() {}
	C_FloatNode(LfFloat* pFloat);
};

} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
