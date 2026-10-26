#ifndef DYNAMIC_FORM_HANDLE_NODE_H
#define DYNAMIC_FORM_HANDLE_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"

namespace DynamicForm
{

enum E_HandleAction
{
	E_HandleAction_Combo,
	E_HandleAction_OpenButton
};



class C_HandleNode : public C_Node, public I_Control
{
public:

	C_HandleNode() : C_Node() {};
	C_HandleNode(LfHandle* pHandle);
	virtual ~C_HandleNode();

	virtual void OnAction(S32 id, S32 action);
	S32	GetRequiredMargin() const;

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_HandleNode* pNode);

protected:
	struct Item
	{
			XtId		id;
			XtString	label;
	};		

	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();
	void	OnLayout();
	bool	OnEnable(bool enable);

	void	RefreshControlsEdit();
	void	RefreshControlsSelect();
	void	SetDom(Xt::DomField* field);

	void	PopulateCombo(bool bFast);
	void	AddItem(Xt::DomRecord* pRecord);

	Xt::DomMonitor	m_DomMonitor;
	wxStaticText*	m_pText;
	C_ComboBox*		m_pComboBox;
	C_Button*		m_pOpenButton;

	LfHandle*					m_pLfField;
	S32							m_Default;
	Ut::Vector<Item>			m_ComboItems;
};

} // DynamicForm

#endif // DYNAMIC_FORM_HANDLE_NODE_H
