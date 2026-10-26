#ifndef RECORD_GRAPH_RECORD_NODE_H
#define RECORD_GRAPH_RECORD_NODE_H

#pragma once

#include "Generic.h"

class C_NodeGraph;

namespace RecordGraph
{

class C_RecordNode : public C_GenericNode
{
public:
	C_RecordNode() : C_GenericNode(E_NodeType_Record) {}
	C_RecordNode(LtStruct* pType, C_NodeGraph* pGraph = 0);

	virtual bool SetSelected(const bool bSelected);

protected:
	bool		OnPopulate();

	LtStruct*	m_pType;
};

} // RecordGraph

#endif // RECORD_GRAPH_RECORD_NODE_H
