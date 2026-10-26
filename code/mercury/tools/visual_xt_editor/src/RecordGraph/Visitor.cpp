#include "stdafx.h"

#include "Visitor.h"

#include "../NodeInterface.h"
#include "Nodes/Generic.h"
#include "Nodes/WeakSocket.h"

using namespace RecordGraph;

I_RecordGraphVisitor::I_RecordGraphVisitor()
{
}

I_RecordGraphVisitor::~I_RecordGraphVisitor()
{
}

bool I_RecordGraphVisitor::Visit(I_Node* node)
{
	return true;
}

bool I_RecordGraphVisitor::Visit(C_GenericNode* node)
{
	return true;
}

bool I_RecordGraphVisitor::Visit(C_WeakSocket* node)
{
	return true;
}



