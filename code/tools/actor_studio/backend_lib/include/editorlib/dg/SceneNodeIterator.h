#pragma once

#ifndef EDITOR_LIB_SCENE_NODE_ITERATOR_H
#define EDITOR_LIB_SCENE_NODE_ITERATOR_H

namespace EditorLib
{
	class C_SceneNodeConstIterator
	{
	public:
		INLINE C_SceneNodeConstIterator();

		///\brief Constructor.
		///
		///The iterator will traverse the scene hierachy from the given node. This includes the
		///siblings of the given node.
		///@param pSceneNode The scene node to begin iterating from.
		INLINE C_SceneNodeConstIterator(const C_SceneNode* pSceneNode);

		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		INLINE C_SceneNodeConstIterator(const C_SceneNodeConstIterator& rhs);

		INLINE const C_SceneNodeConstIterator& operator=(const C_SceneNodeConstIterator& rhs);

		INLINE operator C_SceneNode*();

		INLINE C_SceneNode* operator->();

		//Pre-increment
		INLINE const C_SceneNode* operator ++();

		//Post-increment iterator
		INLINE const C_SceneNode* operator ++(int);
	
	protected:
		C_SceneNode* m_pBaseNode;
		C_SceneNode* m_pCurrentNode;
		Ut::UtVector< Ut::DynamicVector<C_SceneNode*> >	m_childStack;
	};

	class C_SceneNodeIterator : public C_SceneNodeConstIterator
	{
	public:
		///\brief Constructor.
		///
		///The iterator will traverse the scene hierachy from the given node. This includes the
		///siblings of the given node.
		///@param pSceneNode The scene node to begin iterating from.
		INLINE C_SceneNodeIterator(C_SceneNode* pSceneNode);

		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		INLINE C_SceneNodeIterator(const C_SceneNodeIterator& rhs);

		///\brief Asignment operator.
		///
		///@param rhs The iterator to copy.
		INLINE const C_SceneNodeIterator& operator=(const C_SceneNodeIterator& rhs);

		///\brief Returns the current node.
		///
		///@return The current node the iterator is positioned at.
		INLINE operator C_SceneNode*();

		INLINE C_SceneNode* operator->();

		//Pre-increment
		INLINE C_SceneNode* operator ++();

		//Post-increment iterator
		INLINE C_SceneNode* operator ++(int);
	};

}

#include "SceneNodeIterator.inl"

#endif
