#ifndef AFTK_SCENE_NODE_ITERATOR_H
#define AFTK_SCENE_NODE_ITERATOR_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_SceneNodeConstIterator
	{
	public:
		///\brief Constructor.
		///
		///The iterator will traverse the hierachy of the given scene from its root.
		///@param pScene The scene through which to iterate.
		C_SceneNodeConstIterator(const C_Scene* pScene)
		{
			m_pBaseNode = const_cast<C_SceneNode*>(pScene->GetRootSceneNodePtr());
			m_pCurrentNode = const_cast<C_SceneNode*>(pScene->GetRootSceneNodePtr());
		}

		///\brief Constructor.
		///
		///The iterator will traverse the scene hierachy from the given node. This includes the
		///siblings of the given node.
		///@param pSceneNode The scene node to begin iterating from.
		C_SceneNodeConstIterator(const C_SceneNode* pSceneNode) 
		{
			m_pBaseNode = const_cast<C_SceneNode*>(pSceneNode);
			m_pCurrentNode = const_cast<C_SceneNode*>(pSceneNode);
		}

		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		C_SceneNodeConstIterator(const C_SceneNodeConstIterator& rhs)
		: 
		m_pBaseNode(rhs.m_pBaseNode),
		m_pCurrentNode(rhs.m_pCurrentNode),
		m_childStack(rhs.m_childStack)
		{
		}

		///\brief Asignment operator.
		///
		///@param rhs The iterator to copy.
		const C_SceneNodeConstIterator& operator=(const C_SceneNodeConstIterator& rhs)
		{
			m_pBaseNode		= rhs.m_pBaseNode;
			m_pCurrentNode	= rhs.m_pCurrentNode;
			m_childStack	= rhs.m_childStack;

			return *this;
		}

		///\brief Returns the current node.
		///
		///@return The current node the iterator is positioned at.
		operator C_SceneNode*()
		{
			return m_pCurrentNode;
		}

		C_SceneNode* operator->() 
		{
			return m_pCurrentNode;
		} ///< @a

		//Pre-increment
		const C_SceneNode* operator ++()
		{
			if (m_pCurrentNode->GetChildPtr() != 0) 
			{
				// Store any children we pass so we can process them when we're done with iterating
				// thru the peers of the current node
				m_childStack.PushBack(m_pCurrentNode->GetChildPtr());
			}
			if (m_pBaseNode != m_pCurrentNode && m_pCurrentNode->GetNextPtr() != 0) 
			{
				m_pCurrentNode = m_pCurrentNode->GetNextPtr();
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
		const C_SceneNode* operator ++(int) 
		{
			const C_SceneNode* pTmp = ++(*this);
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
		C_SceneNode* m_pBaseNode;
		C_SceneNode* m_pCurrentNode;
		Ut::UtVector< Ut::DynamicVector<C_SceneNode*> >	m_childStack;
	#endif
	};

	class C_SceneNodeIterator : public C_SceneNodeConstIterator
	{
	public:
		///\brief Constructor.
		///
		///The iterator will traverse the hierachy of the given scene from its root.
		///@param pScene The scene through which to iterate.
		C_SceneNodeIterator(C_Scene* pScene)
		:
		C_SceneNodeConstIterator(pScene) {}

		///\brief Constructor.
		///
		///The iterator will traverse the scene hierachy from the given node. This includes the
		///siblings of the given node.
		///@param pSceneNode The scene node to begin iterating from.
		C_SceneNodeIterator(C_SceneNode* pSceneNode)
		:
		C_SceneNodeConstIterator(pSceneNode) {}

		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		C_SceneNodeIterator(const C_SceneNodeIterator& rhs)
		: 
		C_SceneNodeConstIterator(rhs) {}

		///\brief Asignment operator.
		///
		///@param rhs The iterator to copy.
		const C_SceneNodeIterator& operator=(const C_SceneNodeIterator& rhs)
		{
			m_pBaseNode		= rhs.m_pBaseNode;
			m_pCurrentNode	= rhs.m_pCurrentNode;
			m_childStack	= rhs.m_childStack;

			return *this;
		}

		///\brief Returns the current node.
		///
		///@return The current node the iterator is positioned at.
		operator C_SceneNode*()
		{
			return m_pCurrentNode;
		}

		C_SceneNode* operator->() 
		{
			return m_pCurrentNode;
		} ///< @a

		//Pre-increment
		C_SceneNode* operator ++()
		{
			return const_cast<C_SceneNode*>(C_SceneNodeConstIterator::operator ++());
		}

		//Post-increment iterator
		C_SceneNode* operator ++(int) 
		{
			C_SceneNode* pTmp = ++(*this);
			return pTmp;
		} ///< @a
	};
}
#endif

