#ifndef DYNAMIC_FORM_FLAGS_NODE_H
#define DYNAMIC_FORM_FLAGS_NODE_H

#pragma once

#include "Node.h"

namespace DynamicForm
{

class C_FlagsNode : public C_Node
{
public:

	C_FlagsNode() : C_Node() {};
	C_FlagsNode(LfFlags* pFlags);
	virtual ~C_FlagsNode();

	
protected:

	virtual bool OnPopulate();
	virtual bool OnBuild();
	virtual void OnLayout();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);

	S32				GetRequiredMargin() const;
	
	wxStaticText*	m_pText;
	wxSizer*		m_pGapSizer;
	LfFlags*		m_pLfFlags;
};

} // DynamicForm

#endif // DYNAMIC_FORM_FLAGS_NODE_H
