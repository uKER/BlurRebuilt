#ifndef RECORD_GRAPH_WEAKCONNECTION_H
#define RECORD_GRAPH_WEAKCONNECTION_H

#pragma once

#include "../Visitor.h"

namespace RecordGraph
{

class C_WeakPtrNode;

class C_WeakConnection : public I_RecordGraphVisitor
{
public:
	C_WeakConnection(C_WeakPtrNode* pWeakPtr);
	~C_WeakConnection();

public:
	virtual bool Visit(C_WeakSocket* field);

	C_WeakPtrNode*	m_pWeakPtrNode;
};

}

#endif // RECORD_GRAPH_WEAKCONNECTION_H
