#ifndef DYNAMIC_FORM_NODE_H
#define DYNAMIC_FORM_NODE_H

#pragma once

#include "Xt/XtTool.h"

namespace DynamicForm
{

class C_Node;
class C_Form;

typedef Ut::UtVector<Ut::DynamicVector<C_Node*> > Nodes;
typedef Ut::UtVector<Ut::DynamicVector<wxWindow*> > Windows;

// similar to ScarabNode
class C_Node
{
public:

	C_Node() : m_pParent(NULL), m_pForm(NULL), m_pSizer(NULL), m_iMargin(0), m_bBuilt(false), m_bEnabled(true) { };
	virtual ~C_Node();

	template<typename Type> inline Type* AddChild(Type* pNode)
	{
		return (Type*)AddChild((C_Node*)pNode);
	}

	C_Node*			AddChild(C_Node* pNode);
	C_Node*			GetChild(U32 index);
	void			ClearChildren();

	C_Node*			GetParent() { return m_pParent; }
	virtual void	SetParent(C_Node* pNode);

	wxSizer*		GetSizer() { return m_pSizer; }
	wxSizer*		GetParentSizer();

	bool			GetEnabled() const { return m_bEnabled; }
	S32				GetMargin() const { return m_iMargin; }
	
	virtual S32		GetRequiredMargin() const { return 0; }

public:
	bool Populate();
	bool Build();
	void Bind(Xt::DomField* parentDom);
	void Bind(Xt::DomBlock* parentDom);
	void Bind(Xt::DomArray* parentDom, int index);
	void Enable(bool enable);
	void Layout();

protected:

	void	HideWindows();
	void	ShowWindows();

	virtual bool OnPopulate();
	virtual bool OnBuild();
	virtual void OnBind(Xt::DomField* parentDom);
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnEnable(bool enable);
	virtual void OnLayout();

	Windows					m_Windows;
	Nodes					m_Children;
	C_Node*					m_pParent;
	C_Form*					m_pForm;
	wxSizer*				m_pSizer;
	S32						m_iMargin;
	bool					m_bBuilt;
	bool					m_bEnabled;
};

bool BuildNode(C_Node* pParentNode, LarvaField* pField);

wxString GetLabel(const XtString& name);

} // DynamicForm

#endif // DYNAMIC_FORM_NODE_H
