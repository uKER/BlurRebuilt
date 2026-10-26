#ifndef EDITOR_LIB_DG_NODE_ITERATOR_H
#define EDITOR_LIB_DG_NODE_ITERATOR_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_DgNodeConstIterator
	{
	public:
		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		INLINE C_DgNodeConstIterator(const C_DgNodeConstIterator& rhs);

		///\brief Assignment operator.
		///
		///@param rhs The iterator to copy.
		INLINE const C_DgNodeConstIterator& operator=(const C_DgNodeConstIterator& rhs);

		///\brief Returns the current node.
		///
		///@return The current node the iterator is positioned at.
		INLINE operator const I_DgNode*() const;

		INLINE const I_DgNode* operator->() const;

		//Pre-increment
		INLINE const I_DgNode* operator ++();

		//Post-increment iterator
		INLINE const I_DgNode* operator ++(int);

	protected:
		INLINE C_DgNodeConstIterator();
		INLINE C_DgNodeConstIterator(const DgIdPtrMap* pDgNodes);

		const DgIdPtrMap*			m_pDgNodes;
		DgIdPtrMap::ConstIterator	m_it;

		FRIEND_CLASS C_DgManager;
	};

	class C_DgNodeIterator : public C_DgNodeConstIterator
	{
	public:
		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		INLINE C_DgNodeIterator(const C_DgNodeIterator& rhs);

		///\brief Assignment operator.
		///
		///@param rhs The iterator to copy.
		INLINE const C_DgNodeIterator& operator=(const C_DgNodeIterator& rhs);

		///\brief Returns the current node.
		///
		///@return The current node the iterator is positioned at.
		INLINE operator I_DgNode*();

		INLINE I_DgNode* operator->();

		//Pre-increment
		INLINE I_DgNode* operator ++();

		//Post-increment iterator
		INLINE I_DgNode* operator ++(int);

	private:
		INLINE C_DgNodeIterator(const DgIdPtrMap* pDgNodes);

		FRIEND_CLASS C_DgManager;
	};
}

#include "DgNodeIterator.inl"

#endif
