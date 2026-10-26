#ifndef DYNAMIC_FORM_STRUCT_NODE_H
#define DYNAMIC_FORM_STRUCT_NODE_H

#pragma once

#include "Node.h"

namespace DynamicForm
{

class C_StructNode : public C_Node
{
public:

	C_StructNode() : C_Node() {};
	C_StructNode(LfStruct* pStruct, C_Form* pForm = 0);
	virtual ~C_StructNode() {};

	S32		GetRequiredMargin() const;

protected:
	bool	OnPopulate();
	void	OnBind(Xt::DomBlock* parentDom);
	void	OnBind(Xt::DomArray* parentDom, int index);

	bool	OnBuild();
	void	OnLayout();

	LfStruct*	m_pStruct;
	wxStaticText*	m_pText;
};

} // DynamicForm

#endif // RECORD_GRAPH_STRUCT_NODE_H
