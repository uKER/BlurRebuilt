#ifndef DYNAMIC_FORM_PTRBODY_NODE_H
#define DYNAMIC_FORM_PTRBODY_NODE_H

#pragma once

#include "Node.h"

namespace DynamicForm
{

class C_PtrHeaderNode;

class C_PtrBodyNode : public C_Node
{
public:
	friend class C_PtrNode;

protected:
	virtual bool OnBuild();
	virtual void OnLayout();	
	
};

} // DynamicForm

#endif // DYNAMIC_FORM_PTRBODY_NODE_H
