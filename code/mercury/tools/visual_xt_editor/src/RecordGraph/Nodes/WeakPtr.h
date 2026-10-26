#ifndef RECORD_GRAPH_WEAKPTR_NODE_H
#define RECORD_GRAPH_WEAKPTR_NODE_H

#pragma once

#include "Generic.h"

class C_NodeGraph;

namespace RecordGraph
{

class C_WeakSocket;

class C_WeakPtrNode : public C_GenericNode
{
public:

	C_WeakPtrNode() : C_GenericNode(E_NodeType_WeakPtr) {};
	C_WeakPtrNode(LfWeakPtr* pWeakPtr, C_NodeGraph* pGraph = 0);
	virtual ~C_WeakPtrNode();

	void			Draw(wxDC& dc) const;

	static void		DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_WeakPtrNode* pNode);

	friend class	C_WeakConnection;

protected:
	void			OnBind(Xt::DomBlock* parentDom);
	void			OnBind(Xt::DomArray* parentDom, int index);
	void			OnLayout(S32 index);
	void			OnLayout2(S32 index);
	void			RefreshControls();

	void			SetDom(Xt::DomField* pField);

	virtual bool	EndDrag(I_Node* pTarget, wxPoint point);

	Xt::DomMonitor		m_DomMonitor;
	LfWeakPtr*			m_pWeakPtr;
	C_WeakSocket*		m_pWeakSocket;
};

} // RecordGraph

#endif // RECORD_GRAPH_WEAKPTR_NODE_H
