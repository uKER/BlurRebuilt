#ifndef DYNAMIC_FORM_RECORD_NODE_H
#define DYNAMIC_FORM_RECORD_NODE_H

#pragma once

#include "Node.h"

namespace DynamicForm
{

class C_Form;

class C_RecordNode : public C_Node
{
public:
	C_RecordNode(LtStruct* pType, C_Form* pForm = 0);

protected:
	bool		OnPopulate();
	bool		OnBuild();
	void		OnLayout();

	LtStruct*	m_pType;
};

} // DynamicForm

#endif // DYNAMIC_FORM_RECORD_NODE_H
