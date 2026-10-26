#ifndef DYNAMIC_FORM_ARRAY_HEADER_NODE_H
#define DYNAMIC_FORM_ARRAY_HEADER_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"

namespace DynamicForm
{

class C_ArrayNode;
class C_ComboBox;

enum E_ArrayHeaderAction
{
	E_ArrayHeaderAction_Combo,
	E_ArrayHeaderAction_Add,
	E_ArrayHeaderAction_Remove,
	E_ArrayHeaderAction_Previous,
	E_ArrayHeaderAction_Next
};

class C_ArrayHeaderNode : public C_Node, public I_Control
{
public:
	C_ArrayHeaderNode(C_ArrayNode* pArrayNode);
	virtual ~C_ArrayHeaderNode();

	static void		DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_ArrayHeaderNode* pNode);

	void			RefreshControlsEdit();
	void			RefreshControlsSelect();

	S32				GetIndex();

	friend class C_ArrayNode;

protected:
	void			OnAction(S32 id, S32 action);
	bool			OnBuild();
	bool			OnEnable(bool enable);

	C_ArrayNode*	m_pArrayNode;
	Xt::DomMonitor	m_DomMonitor;

	C_ComboBox*		m_pComboBox;
	wxButton*		m_pAdd;
	wxButton*		m_pRemove;
	wxButton*		m_pPrevious;
	wxButton*		m_pNext;

	S32				m_iIndex;
};


} // DynamicForm

#endif // DYNAMIC_FORM_ARRAY_HEADER_NODE_H
