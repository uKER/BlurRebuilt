#ifndef DYNAMIC_FORM_PTR_NODE_H
#define DYNAMIC_FORM_PTR_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"

namespace DynamicForm
{

class C_PtrNode : public C_Node
{
public:

	C_PtrNode() : C_Node() {};
	C_PtrNode(LfPtr* pPtr);
	virtual ~C_PtrNode();

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_PtrNode* pNode);

	friend class C_PtrHeaderNode;
	friend class C_PtrBodyNode;

	S32		GetRequiredMargin() const;

protected:
	
	bool	OnPopulate();
	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();
	bool	OnEnable(bool enable);
	void	OnLayout();

	void	OnTypeChanged();
	bool	PopulateBody();
	void	SetDom(Xt::DomBlock* pBlock);

	// xt
	Xt::DomMonitor		m_DomMonitor;
	LfPtr*				m_pLfPtr;

	// wx 
	wxStaticText*		m_pText;
	wxSizer*			m_pGapSizer;

	// node
	C_PtrHeaderNode*	m_pHeader;
	C_PtrBodyNode*		m_pBody;
};

} // DynamicForm

#endif // DYNAMIC_FORM_PTR_NODE_H
