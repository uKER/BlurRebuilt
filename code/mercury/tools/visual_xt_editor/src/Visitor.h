#ifndef NODE_GRAPH_VISITOR_H
#define NODE_GRAPH_VISITOR_H

#pragma once

class I_Node;

class I_NodeVisitor
{
public:
	I_NodeVisitor();
	~I_NodeVisitor();

public:
	virtual bool Visit(I_Node* node);
};

#endif // RECORD_GRAPH_VISITOR_H
