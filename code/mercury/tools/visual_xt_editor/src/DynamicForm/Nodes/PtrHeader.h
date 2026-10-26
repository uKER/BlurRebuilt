#ifndef DYNAMIC_FORM_PTRHEADER_NODE_H
#define DYNAMIC_FORM_PTRHEADER_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"

namespace DynamicForm
{

class C_PtrNode;

class C_PtrHeaderNode : public C_Node, public I_Control
{
public:

	C_PtrHeaderNode() : C_Node() {};
	C_PtrHeaderNode(C_PtrNode* pPtrNode);
	virtual ~C_PtrHeaderNode();

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_PtrHeaderNode* pNode);

	friend class C_PtrNode;

protected:

	virtual void OnAction(S32 id, S32 action);
	void	OnBind(Xt::DomBlock* parentDom);
	bool	OnBuild();
	void	OnLayout();

	void	RefreshControlsEdit();
	void	PopulateCombo();
	void	SetDom(Xt::DomField* field);

	Xt::DomMonitor	m_DomMonitor;
	
	wxStaticText*	m_pText;
	C_ComboBox*		m_pComboBox;
	
	C_PtrNode*		m_pPtrNode;
	Ut::Vector<LtStruct*>	m_ComboItems;
};

} // DynamicForm

#endif // DYNAMIC_FORM_PTRHEADER_NODE_H
