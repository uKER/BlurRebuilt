#ifndef RECORD_GRAPH_ARRAY_ITEM_NODE_H
#define RECORD_GRAPH_ARRAY_ITEM_NODE_H

#pragma once

#include "Generic.h"

namespace RecordGraph
{

class C_ArrayNode;

class C_ArrayItemNode : public C_GenericNode
{
public:
	C_ArrayItemNode() : C_GenericNode(E_NodeType_ArrayItem) {};
	C_ArrayItemNode(C_ArrayNode* pParentArray, S32 iIndex);
	virtual ~C_ArrayItemNode();

	void	PrepareDC(wxDC& dc) const;
	void	Close();

protected:
	bool			OnPopulate();
	void			OnBind(Xt::DomBlock* parentDom);
	void			OnBind(Xt::DomArray* parentDom, S32 iIndex);
	bool			OnBuild();
	void			OnLayout(S32 index);

	bool			SetSelected(const bool bSelected);

	void			SetDom(Xt::DomArray* pArray);
	C_ArrayNode*	m_pParentArray;
	Xt::DomMonitor	m_DomMonitor;
	S32				m_iIndex;
};

} // RecordGraph

#endif // RECORD_GRAPH_ARRAY_ITEM_NODE_H
