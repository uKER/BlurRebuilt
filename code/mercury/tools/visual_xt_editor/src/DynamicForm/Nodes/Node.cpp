#include "stdafx.h"

#include "Node.h"
#include "../Form.h"

using namespace DynamicForm;

C_Node::~C_Node()
{
	ClearChildren();

	for(S32 i = 0; i < m_Windows.Size(); ++i)
		delete m_Windows[i];

	m_Windows.Clear();

	//if(m_pSizer)
	//	delete m_pSizer;
}

void C_Node::ClearChildren()
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
		delete m_Children[i];

	m_Children.Clear();
}

C_Node* C_Node::AddChild(C_Node* pNode)
{
	if(pNode->GetParent())
		return NULL;

	pNode->SetParent(this);

	pNode->m_pForm = m_pForm;

	m_Children.PushBack(pNode);
	
	return pNode;
}

C_Node* C_Node::GetChild(U32 index)
{
	return m_Children[index];
};

void C_Node::SetParent(C_Node* pNode)
{
	if(!m_pParent)
	{
		m_pParent = pNode;
	}
}

wxSizer* C_Node::GetParentSizer()
{
	if(m_pParent)
	{
		if(m_pParent->GetSizer())
			return m_pParent->GetSizer();
		else
			return m_pParent->GetParentSizer();
	}
	else if(m_pForm)
		return m_pForm->GetSizer();
	else
		return NULL;
}

void C_Node::HideWindows()
{
	for(S32 i = 0; i < m_Windows.Size(); ++i)
		m_Windows[i]->Hide();
}

void C_Node::ShowWindows()
{
	for(S32 i = 0; i < m_Windows.Size(); ++i)
		m_Windows[i]->Show();
}

// Xt stuff

// virtuals

bool C_Node::OnPopulate()
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		if(!GetChild(i)->OnPopulate())
			return false;
	}

	return true;
}

bool C_Node::OnBuild()
{
	return true;
}

bool C_Node::OnEnable(bool enable)
{
	for(S32 i = 0; i < m_Windows.Size(); ++i)
	{
		m_Windows[i]->Enable(enable);
	}

	return enable;
}

void C_Node::OnBind(Xt::DomField* parentDom)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Bind(parentDom);
	}
}

void C_Node::OnBind(Xt::DomBlock* parentDom)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Bind(parentDom);
	}
}

void C_Node::OnBind(Xt::DomArray* parentDom, int index)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Bind(parentDom, index);
	}
}

void C_Node::Layout()
{
	if (!m_bBuilt)
		return;

	OnLayout();

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Layout();
	}
}

// none virtuals

bool C_Node::Populate()
{
	return OnPopulate();
}

bool C_Node::Build()
{
	if (!m_bBuilt)
	{
		m_bBuilt = true;

		if (!OnBuild())
			return false;
	}

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		if (!GetChild(i)->Build())
			return false;
	}

	return true;
}

void C_Node::Enable(bool enable)
{
	enable = OnEnable(enable);
	m_bEnabled = enable;
	
	for(S32 i = 0; i < m_Children.Size(); ++i)
		GetChild(i)->Enable(enable);
}

void C_Node::Bind(Xt::DomField* parentDom)
{
	OnBind(parentDom);
}

void C_Node::Bind(Xt::DomBlock* parentDom)
{
	OnBind(parentDom);
}

void C_Node::Bind(Xt::DomArray* parentDom, int index)
{
	OnBind(parentDom, index);
}

void C_Node::OnLayout()
{
}

namespace DynamicForm
{

wxString GetLabel(const XtString& name)
{
	wxString label((const char*)name);
	label += ": ";
	
	return label;
}

}