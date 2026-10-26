#include "stdafx.h"

#include "Visitor.h"

I_NodeVisitor::I_NodeVisitor()
{
}

I_NodeVisitor::~I_NodeVisitor()
{
}

bool I_NodeVisitor::Visit(I_Node* node)
{
	return true;
}

