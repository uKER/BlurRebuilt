#ifndef DYNAMIC_FORM_WEAKPTR_NODE_H
#define DYNAMIC_FORM_WEAKPTR_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"
#include "../Form.h"
#include "../../Common/WeakInterface.h"

namespace DynamicForm
{

class C_WeakPtrNode : public C_Node, public I_Control, public I_WeakInterface
{
public:

	C_WeakPtrNode() : C_Node() {};
	C_WeakPtrNode(LfWeakPtr* pWeakPtr);
	virtual ~C_WeakPtrNode();

	static void DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_WeakPtrNode* pNode);

	struct Item
	{
		XtString		id;
		XtString		label;
		LarvaField*		pField;
		Xt::DomMonitor	domMonitor;

	public:
		inline Item() :
			pField(0)
		{
		}
	};

	bool AddItem(LtStruct* type, Xt::DomBlock* block, LarvaField* labelField, Xt::DomBlock* labelWatch);

protected:

	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);
	bool	OnBuild();
	void	OnLayout();
	void	OnAction(S32 id, S32 action);

	S32		GetRequiredMargin() const;

	void	PopulateCombo();

	void	RefreshControlsEdit();
	void	SetDom(Xt::DomField* field);

	Xt::DomMonitor		m_DomMonitor;
	Xt::DomMonitor		m_WatchMonitor;
	wxStaticText*		m_pText;
	C_ComboBox*			m_pComboBox;
	LfWeakPtr*			m_pLfField;
	S32					m_Default;
	Ut::Vector<Item>	m_Items;
};

} // DynamicForm

#endif // DYNAMIC_FORM_ENUM_NODE_H
