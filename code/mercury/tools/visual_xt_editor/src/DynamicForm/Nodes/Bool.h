#ifndef DYNAMIC_FORM_BOOL_NODE_H
#define DYNAMIC_FORM_BOOL_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"

namespace DynamicForm
{

class C_BoolNode : public C_Node, public I_Control
{
public:

	C_BoolNode() : C_Node() {};
	C_BoolNode(LfBool* pBool);
	virtual ~C_BoolNode();

	virtual void OnAction(S32 id, S32 action);

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_BoolNode* pNode);

protected:

	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();
	void	OnLayout();

	void	RefreshControlsEdit();
	void	SetDom(Xt::DomField* field);

	Xt::DomMonitor	m_DomMonitor;
	LfBool*			m_pLfField;
	wxSizer*		m_pGapSizer;
	C_CheckBox*		m_pCheckBox;
	
};



} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
