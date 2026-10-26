#ifndef DYNAMIC_FORM_FLAGBOOL_NODE_H
#define DYNAMIC_FORM_FLAGBOOL_NODE_H

#pragma once

#include "Node.h"
#include "../Controls/Controls.h"

namespace DynamicForm
{

class C_FlagBoolNode : public C_Node, public I_Control
{
public:

	C_FlagBoolNode() : C_Node() {};
	C_FlagBoolNode(LarvaOption* pField);
	virtual ~C_FlagBoolNode();

	virtual void OnAction(S32 id, S32 action);

	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, C_FlagBoolNode* node);
	
protected:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();

	void RefreshControlsEdit();
	void SetDom(Xt::DomField* field);
	
	LarvaOption*	m_pField;
	C_CheckBox*		m_pCheckBox;
	Xt::DomMonitor	m_DomMonitor;
};

} // DynamicForm

#endif // DYNAMIC_FORM_FLAGS_NODE_H
