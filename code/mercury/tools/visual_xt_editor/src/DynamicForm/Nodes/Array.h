#ifndef DYNAMIC_FORM_ARRAY_NODE_H
#define DYNAMIC_FORM_ARRAY_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"

namespace DynamicForm
{

class C_ArrayHeaderNode;
class C_ArrayNode;

class C_ArrayNode : public C_Node
{
public:

	C_ArrayNode() : C_Node() {};
	C_ArrayNode(LfArray* pArray);
	virtual ~C_ArrayNode();

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_ArrayNode* pNode);

	friend class C_ArrayHeaderNode;
	friend class C_ArrayBodyNode;

	S32		GetRequiredMargin() const;

protected:
	
	bool	OnPopulate();
	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();
	bool	OnEnable(bool enable);
	void	OnLayout();

	void	OnIndexChanged();
	void	SetDom(Xt::DomArray* pArray);

	// xt
	Xt::DomMonitor		m_DomMonitor;
	LfArray*			m_pLfArray;

	// wx 
	wxStaticText*		m_pText;
	wxSizer*			m_pGapSizer;

	// node
	C_ArrayHeaderNode*	m_pHeader;
	C_ArrayBodyNode*	m_pBody;
};

} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
