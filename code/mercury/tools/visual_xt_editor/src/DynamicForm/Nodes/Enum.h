#ifndef DYNAMIC_FORM_ENUM_NODE_H
#define DYNAMIC_FORM_ENUM_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"

namespace DynamicForm
{

class C_EnumNode : public C_Node, public I_Control
{
public:

	C_EnumNode() : C_Node() {};
	C_EnumNode(LfEnum* pEnum);
	virtual ~C_EnumNode();

	virtual void OnAction(S32 id, S32 action);
	S32	GetRequiredMargin() const;

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_EnumNode* pNode);

protected:

	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();
	void	OnLayout();

	void	RefreshControlsEdit();
	void	SetDom(Xt::DomField* field);

	Xt::DomMonitor	m_DomMonitor;
	wxStaticText*	m_pText;
	C_ComboBox*		m_pComboBox;
	LfEnum*			m_pLfField;
	S32				m_Default;
	Ut::Vector<LarvaOption*>	m_ComboItems;
};

} // DynamicForm

#endif // DYNAMIC_FORM_ENUM_NODE_H
