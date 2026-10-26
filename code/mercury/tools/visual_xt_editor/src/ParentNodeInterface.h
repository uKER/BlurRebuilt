#ifndef NODE_GRAPH_PARENTNODE_INTERFACE_H
#define NODE_GRAPH_PARENTNODE_INTERFACE_H

#pragma once

#include "NodeInterface.h"

// a node which can have embedded and external children

template<class T>
class I_ParentNode : public I_Node
{
public:
	I_ParentNode() : I_Node(), m_bHideChildren(false) {}
	I_ParentNode(S32 type) : I_Node(type), m_bHideChildren(false) {}
	virtual ~I_ParentNode();

	typedef Ut::UtVector<Ut::DynamicVector<T*> > Nodes;
	typedef Ut::UtVector<Ut::DynamicVector<I_Node*> > GenericNodes;

	virtual I_Node*		HitTest(wxPoint pt, I_Node* pExclude);
	virtual void		Draw(wxDC& dc) const = 0;

	void				BringToFront(I_Node* pChild);

	void				AddChild(T* pNode);
	T*					GetChild(U32 index) { return m_Children[index]; };
	S32					GetNumChildren() const { return m_Children.Size(); };
	S32					GetNumChildren(S32 nodeType, S32 maxIndex) const;
	void				ClearChildren();

	void				AddGenericChild(I_Node* pNode);
	T*					GetGenericChild(U32 index) { return m_GenericChildren[index]; };
	S32					GetNumGenericChildren() const { return m_GenericChildren.Size(); };
	S32					GetNumGenericChildren(S32 nodeType, S32 maxIndex) const;
	void				ClearGenericChildren();
	void				ClearGenericChildren(S32 iType);

	bool				DeleteChild(I_Node* pNode);

	I_Node*				FindNode(const Util::C_String& name);
	
	virtual void		ToggleHideChildren();
	bool				GetHideChildren() const { return m_bHideChildren; }
protected:
	bool				m_bHideChildren;

	Nodes				m_Children;
	GenericNodes		m_GenericChildren;
	Util::C_String		m_Name;		
};


// parent node
template<class T>
I_ParentNode<T>::~I_ParentNode()
{
	ClearChildren();
	ClearGenericChildren();
}

template<class T>
I_Node* I_ParentNode<T>::HitTest(wxPoint pt, I_Node* pExclude)
{
	if(pExclude == this)
		return NULL;

	for(S32 i = 0; i < m_GenericChildren.Size(); ++i)
	{
		I_Node* node = m_GenericChildren[i]->HitTest(pt - m_Position, pExclude);
		
		if(node && node != pExclude)
			return node;
	}

	// go through children backwards since last child gets drawn last
	for(S32 i = m_Children.Size() - 1; i >= 0; --i)
	{
		I_Node* node = m_Children[i]->HitTest(pt - m_Position, pExclude);
		
		if(node && node != pExclude)
			return node;
	}
	
	if(GetRect().Contains(pt))
		return this;
	else
		return NULL;
}

template<class T>
void I_ParentNode<T>::BringToFront(I_Node* pChild)
{
	S32 remove = -1;
	for(S32 i = 0; i < m_Children.Size(); ++i )
	{
		if(pChild == m_Children[i])
		{
			remove = i;
			break;
		}
	}

	if(remove != -1)
	{
		m_Children.EraseByIndex(remove);
		m_Children.PushBack((T*)pChild);
	}
}

template<class T>
bool I_ParentNode<T>::DeleteChild(I_Node* pChild)
{
	S32 remove = -1;
	for(S32 i = 0; i < m_Children.Size(); ++i )
	{
		if(pChild == m_Children[i])
		{
			remove = i;
			break;
		}
	}

	if(remove != -1)
	{
		m_Children.EraseByIndex(remove);
		delete pChild;
		return true;
	}

	for(S32 i = 0; i < m_GenericChildren.Size(); ++i )
	{
		if(pChild == m_GenericChildren[i])
		{
			remove = i;
			break;
		}
	}

	if(remove != -1)
	{
		m_GenericChildren.EraseByIndex(remove);
		delete pChild;
		return true;
	}

	return false;
}

template<class T>
void I_ParentNode<T>::AddChild(T* pNode)
{
	pNode->m_pParent = this;
	pNode->m_pGraph = m_pGraph;
	m_Children.PushBack(pNode);
}

template<class T>
S32	I_ParentNode<T>::GetNumChildren(S32 nodeType, S32 maxIndex) const
{
	S32 count = 0;
	S32 index = 0;

	while(index < m_Children.Size() && index < maxIndex)
	{
		if(m_Children[index]->GetType() == nodeType)
			++count;

		++index;
	}

	return count;
}

template<class T>
void I_ParentNode<T>::ClearChildren()
{
	for(int i = 0; i < m_Children.Size(); ++i)
		delete m_Children[i];

	m_Children.Clear();
}

template<class T>
void I_ParentNode<T>::AddGenericChild(I_Node* pNode)
{
	pNode->m_pParent = this;
	pNode->m_pGraph = m_pGraph;
	m_GenericChildren.PushBack(pNode);
}

template<class T>
void I_ParentNode<T>::ClearGenericChildren()
{
	for(int i = 0; i < m_GenericChildren.Size(); ++i)
		delete m_GenericChildren[i];

	m_GenericChildren.Clear();
}

template<class T>
void I_ParentNode<T>::ClearGenericChildren(S32 iType)
{
	for(int i = 0; i < m_GenericChildren.Size(); )
	{
		if(m_GenericChildren[i]->GetType() == iType)
			m_GenericChildren.EraseByIndex(i);
		else
			++i;
	}
}

template<class T>
S32	I_ParentNode<T>::GetNumGenericChildren(S32 nodeType, S32 maxIndex) const
{
	S32 count = 0;
	S32 index = 0;

	while(index < m_GenericChildren.Size() && index < maxIndex)
	{
		if(m_GenericChildren[index]->GetType() == nodeType)
			++count;

		++index;
	}

	return count;
}

template<class T>
void I_ParentNode<T>::ToggleHideChildren()
{
	m_bHideChildren = !m_bHideChildren;

	for(S32 i = 0; i < m_Children.Size(); ++i )
	{
		m_Children[i]->ToggleVisible();
	}
}

template<class T>
I_Node* I_ParentNode<T>::FindNode(const Util::C_String& name)
{
	if(name == m_Name)
		return this;

	for(S32 i = 0; i < m_GenericChildren.Size(); ++i)
	{
		if(I_Node* node = m_GenericChildren[i]->FindNode(name))
			return node;
	}

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		if(I_Node* node = m_Children[i]->FindNode(name))
			return node;
	}
	
	return NULL;
}

#endif // NODE_GRAPH_NODE_INTERFACE_H
