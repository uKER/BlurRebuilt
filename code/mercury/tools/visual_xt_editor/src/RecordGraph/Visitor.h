#ifndef RECORD_GRAPH_VISITOR_H
#define RECORD_GRAPH_VISITOR_H

#pragma once

#include "../Visitor.h"

namespace RecordGraph
{

class C_GenericNode;
class C_WeakSocket;

class I_RecordGraphVisitor : public I_NodeVisitor
{
public:
	I_RecordGraphVisitor();
	~I_RecordGraphVisitor();

public:
	virtual bool Visit(I_Node* node);
	virtual bool Visit(C_GenericNode* node);
	virtual bool Visit(C_WeakSocket* node);
};

}

#endif // RECORD_GRAPH_VISITOR_H
