#ifndef CONVERTER_SHARED_HIERARCHY_H
#define CONVERTER_SHARED_HIERARCHY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/Vector.h>

namespace Hrc
{

template <typename T_Node>
class C_Hierarchy;

// C_Node

template <typename T_Node>
class C_Node
{
friend class C_Hierarchy<T_Node>;
public:
	C_Node();
	~C_Node();

	C_Hierarchy<T_Node>* GetHierarchy();

	const C_Hierarchy<T_Node>* GetHierarchy() const;

	const T_Node* GetParent() const;
	T_Node* GetParent();

	const T_Node* GetFirstChild() const;
	T_Node* GetFirstChild();

	const T_Node* GetNextSibling() const;
	T_Node* GetNextSibling();

	const T_Node* GetPrevSibling() const;
	T_Node* GetPrevSibling();

	void AddChildNode( T_Node* pNewChild );

	void Remove();

private:
	void SetHierarchy( C_Hierarchy<T_Node>* pHierarchy );

private:
	C_Hierarchy<T_Node>* m_pHierarchy;

	T_Node* m_pParent;
    T_Node* m_pFirstChild;
    T_Node* m_pNextSibling;
    T_Node* m_pPrevSibling;
};

// C_Hierarchy

template <typename T_Node>
class C_Hierarchy
{
friend class C_Node<T_Node>;
public:
	C_Hierarchy();
	~C_Hierarchy();

	S32 GetNodeCount() const;

	T_Node* GetRootNode();

	const T_Node* GetRootNode() const;

	void AddRootNode( T_Node* pNode );

private:
	S32 m_NodeCount;

	T_Node* m_pRoot;
};

// C_Node

template <typename T_Node>
C_Node<T_Node>::C_Node()
	: m_pHierarchy(NULL)
	, m_pParent(NULL)
	, m_pFirstChild(NULL)
	, m_pNextSibling(NULL)
	, m_pPrevSibling(NULL)
{
}

template <typename T_Node>
C_Node<T_Node>::~C_Node()
{
	SetHierarchy(NULL);
	SAFE_DELETE_PTR( m_pFirstChild );
	SAFE_DELETE_PTR( m_pNextSibling );
}

template <typename T_Node>
C_Hierarchy<T_Node>* C_Node<T_Node>::GetHierarchy()
{
	return m_pHierarchy;
}

template <typename T_Node>
const C_Hierarchy<T_Node>* C_Node<T_Node>::GetHierarchy() const
{
	return m_pHierarchy;
}

template <typename T_Node>
const T_Node* C_Node<T_Node>::GetParent() const
{
	return m_pParent;
}

template <typename T_Node>
T_Node* C_Node<T_Node>::GetParent()
{
	return m_pParent;
}

template <typename T_Node>
const T_Node* C_Node<T_Node>::GetFirstChild() const
{
	return m_pFirstChild;
}

template <typename T_Node>
T_Node* C_Node<T_Node>::GetFirstChild()
{
	return m_pFirstChild;
}

template <typename T_Node>
const T_Node* C_Node<T_Node>::GetNextSibling() const
{
	return m_pNextSibling;
}

template <typename T_Node>
T_Node* C_Node<T_Node>::GetNextSibling()
{
	return m_pNextSibling;
}

template <typename T_Node>
const T_Node* C_Node<T_Node>::GetPrevSibling() const
{
	return m_pPrevSibling;
}

template <typename T_Node>
T_Node* C_Node<T_Node>::GetPrevSibling()
{
	return m_pPrevSibling;
}

template <typename T_Node>
void C_Node<T_Node>::SetHierarchy( C_Hierarchy<T_Node>* pHierarchy )
{
	if( m_pHierarchy )
	{
		m_pHierarchy->m_NodeCount--;
	}

	m_pHierarchy = pHierarchy;
	if(m_pHierarchy)
	{
		m_pHierarchy->m_NodeCount++;
	}

	// iterate children and set their groups too
	C_Node<T_Node>* pCurrChild = GetFirstChild();
	while(pCurrChild)
	{
		pCurrChild->SetHierarchy( m_pHierarchy );
		pCurrChild = pCurrChild->GetNextSibling();
	}
}

template <typename T_Node>
void C_Node<T_Node>::AddChildNode( T_Node* pNewChild )
{
	assert( pNewChild->m_pParent == NULL );
	assert( pNewChild->m_pFirstChild == NULL );
	assert( pNewChild->m_pNextSibling == NULL );
	assert( pNewChild->m_pPrevSibling == NULL );

	pNewChild->SetHierarchy( GetHierarchy() );

	// find last existing sibling

	T_Node* pLastChild = GetFirstChild();
	if(pLastChild)
	{
		while( pLastChild->GetNextSibling() )
		{
			pLastChild = pLastChild->GetNextSibling();
		}
	}

	pNewChild->m_pParent = (T_Node*)this;
	pNewChild->m_pPrevSibling = pLastChild;
	pNewChild->m_pNextSibling = NULL;

	if(pLastChild)
	{
		pLastChild->m_pNextSibling = pNewChild;
	}
	else
	{
		m_pFirstChild = pNewChild;
	}	
}

// C_Hierarchy

template <typename T_Node>
C_Hierarchy<T_Node>::C_Hierarchy()
	: m_NodeCount(0)
	, m_pRoot( NULL )
{
}

template <typename T_Node>
C_Hierarchy<T_Node>::~C_Hierarchy()
{
	SAFE_DELETE_PTR( m_pRoot );
}

template <typename T_Node>
S32 C_Hierarchy<T_Node>::GetNodeCount() const
{
	return m_NodeCount;
}

template <typename T_Node>
T_Node* C_Hierarchy<T_Node>::GetRootNode()
{
	return m_pRoot;
}

template <typename T_Node>
const T_Node* C_Hierarchy<T_Node>::GetRootNode() const
{
	return m_pRoot;
}

template <typename T_Node>
void C_Hierarchy<T_Node>::AddRootNode( T_Node* pNode )
{
	assert( pNode->m_pParent == NULL );
	assert( pNode->m_pFirstChild == NULL );
	assert( pNode->m_pNextSibling == NULL );
	assert( pNode->m_pPrevSibling == NULL );

	pNode->SetHierarchy(this);

	// find last existing root node

	T_Node* pLastRootNode = GetRootNode();
	if(pLastRootNode)
	{
		while( pLastRootNode->GetNextSibling() )
		{
			pLastRootNode = pLastRootNode->GetNextSibling();
		}
	}

	// new node has no parent
	pNode->m_pParent = NULL;
	pNode->m_pPrevSibling = pLastRootNode;
	pNode->m_pNextSibling = NULL;

	if(pLastRootNode)
	{		
		pLastRootNode->m_pNextSibling = pNode;
	}
	else
	{
		m_pRoot = pNode;
	}
}

} // namespace Hrc

#endif

