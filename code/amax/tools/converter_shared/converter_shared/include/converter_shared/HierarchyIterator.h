#ifndef CONVERTER_SHARED_HIERARCHY_ITERATOR_H
#define CONVERTER_SHARED_HIERARCHY_ITERATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "converter_shared/Hierarchy.h"

namespace Hrc
{

template <typename T_Node>
class C_HierarchyConstIterator
{
public:
	///\brief Constructor.
	///
	///The iterator will traverse the hierachy from its root.
	///@param pHrc The hierarchy through which to iterate.
	C_HierarchyConstIterator(const C_Hierarchy<T_Node>* pHrc)
	{
		m_pBaseNode = const_cast<T_Node*>(pHrc->GetRootNode());
		m_pCurrentNode = const_cast<T_Node*>(pHrc->GetRootNode());
	}

	///\brief Constructor.
	///
	///The iterator will traverse the hierachy from the given node. This includes the
	///siblings of the given node.
	///@param pNode The node to begin iterating from.
	C_HierarchyConstIterator(const T_Node* pNode) 
	{
		m_pBaseNode = const_cast<T_Node*>(pNode);
		m_pCurrentNode = const_cast<T_Node*>(pNode);
	}

	///\brief Copy constructor.
	///
	///@param rhs The iterator to copy.
	C_HierarchyConstIterator(const C_HierarchyConstIterator& rhs)
	: 
	m_pBaseNode(rhs.m_pBaseNode),
	m_pCurrentNode(rhs.m_pCurrentNode),
	m_childStack(rhs.m_childStack)
	{
	}

	///\brief Asignment operator.
	///
	///@param rhs The iterator to copy.
	const C_HierarchyConstIterator& operator=(const C_HierarchyConstIterator& rhs)
	{
		m_pBaseNode		= rhs.m_pBaseNode;
		m_pCurrentNode	= rhs.m_pCurrentNode;
		m_childStack	= rhs.m_childStack;

		return *this;
	}

	///\brief Returns the current node.
	///
	///@return The current node the iterator is positioned at.
	operator const T_Node*()
	{
		return m_pCurrentNode;
	}

	operator const bool()
	{
		return (m_pCurrentNode!=NULL);
	}
	
	const T_Node* operator->() 
	{
		return m_pCurrentNode;
	} ///< @a

	//Pre-increment
	const T_Node* operator ++()
	{
		if (m_pCurrentNode->GetFirstChild() != 0) 
		{
			// Store any children we pass so we can process them when we're done with iterating
			// thru the peers of the current node
			m_childStack.PushBack(m_pCurrentNode->GetFirstChild());
		}
		if (m_pCurrentNode->GetNextSibling() != 0) 
		{
			m_pCurrentNode = m_pCurrentNode->GetNextSibling();
		} 
		else 
		{
			U32 stackSize = (U32)m_childStack.Size();
			if (stackSize > 0) 
			{
				m_pCurrentNode = m_childStack[stackSize - 1];
				m_childStack.PopBack();
			} 
			else 
			{
				// we have reached the end of the scene
				m_pCurrentNode = 0;
			}
		}

		return m_pCurrentNode;
	}

	//Post-increment iterator
	const T_Node* operator ++(int) 
	{
		const T_Node* pTmp = ++(*this);
		return pTmp;
	} ///< @a
		

	///\brief Clears the iterator to its initial state.
	///
	void Reset()
	{
		m_pCurrentNode = m_pBaseNode;
		m_childStack.Clear();
	}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
protected:
	T_Node* m_pBaseNode;
	T_Node* m_pCurrentNode;
	Btl::Vector< T_Node* >	m_childStack;
#endif
};

template <typename T_Node>
class C_HierarchyIterator : public C_HierarchyConstIterator<T_Node>
{
public:
	///\brief Constructor.
	///
	///The iterator will traverse the hierachy from its root.
	C_HierarchyIterator(C_Hierarchy<T_Node>* pHrc)
	:
	C_HierarchyConstIterator(pHrc) {}

	///\brief Constructor.
	///
	///The iterator will traverse the hierachy from the given node. This includes the
	///siblings of the given node.
	C_HierarchyIterator(T_Node* pNode)
	:
	C_HierarchyConstIterator(pNode) {}

	///\brief Copy constructor.
	///
	///@param rhs The iterator to copy.
	C_HierarchyIterator(const C_HierarchyIterator& rhs)
	: 
	C_HierarchyConstIterator(rhs) {}

	///\brief Asignment operator.
	///
	///@param rhs The iterator to copy.
	const C_HierarchyIterator& operator=(const C_HierarchyIterator& rhs)
	{
		m_pBaseNode		= rhs.m_pBaseNode;
		m_pCurrentNode	= rhs.m_pCurrentNode;
		m_childStack	= rhs.m_childStack;

		return *this;
	}

	///\brief Returns the current node.
	///
	///@return The current node the iterator is positioned at.
	operator T_Node*()
	{
		return m_pCurrentNode;
	}

	operator const bool()
	{
		return (m_pCurrentNode!=NULL);
	}

	T_Node* operator->() 
	{
		return m_pCurrentNode;
	} ///< @a

	//Pre-increment
	T_Node* operator ++()
	{
		return const_cast<T_Node*>(C_HierarchyConstIterator::operator ++());
	}

	//Post-increment iterator
	T_Node* operator ++(int) 
	{
		T_Node** pTmp = ++(*this);
		return pTmp;
	} ///< @a
};


} // namespace Hrc

#endif

