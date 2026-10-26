#ifndef RECORD_GRAPH_WEAKSOCKET_NODE_H
#define RECORD_GRAPH_WEAKSOCKET_NODE_H

#pragma once

#include "src/NodeInterface.h"
#include "Xt/XtTool.h"

namespace RecordGraph
{

class C_WeakPtrNode;
class I_RecordGraphVisitor;

class C_WeakSocket : public I_Node
{
public:
	C_WeakSocket(LfWeakPtr* pWeakPtr, Xt::DomField* pField);
	virtual ~C_WeakSocket();

	void	Draw(wxDC& dc) const;
	bool	SetSelected(const bool bSelected);

	virtual bool	EndDrag(I_Node* pTarget, wxPoint point);

	virtual void	Accept(I_NodeVisitor* pVisitor);

	friend class	C_WeakConnection;

private:

	LfWeakPtr*		m_pWeakPtr;
	C_WeakPtrNode*	m_pWeakPtrNode;
	Xt::DomField*	m_pField;
};

} // RecordGraph

#endif // RECORD_GRAPH_HIDE_ICON_H
