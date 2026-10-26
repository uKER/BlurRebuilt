#ifndef DYNAMIC_FORM_STRING_NODE_H
#define DYNAMIC_FORM_STRING_NODE_H

#pragma once

#include "FieldInterface.h"

struct LfInt;

namespace DynamicForm
{

class C_StringNode : public I_FieldNode<C_StringNode, LfString>
{
public:
	C_StringNode() : I_FieldNode<C_StringNode, LfString>() {}
	C_StringNode(LfString* pString);
};

} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
