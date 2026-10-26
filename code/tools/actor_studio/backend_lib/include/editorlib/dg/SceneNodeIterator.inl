namespace EditorLib
{

C_SceneNodeConstIterator::C_SceneNodeConstIterator(const C_SceneNode* pSceneNode)
{
	m_pBaseNode = const_cast<C_SceneNode*>(pSceneNode);
	m_pCurrentNode = const_cast<C_SceneNode*>(pSceneNode);
}

C_SceneNodeConstIterator::C_SceneNodeConstIterator(const C_SceneNodeConstIterator& rhs)
: 
m_pBaseNode(rhs.m_pBaseNode),
m_pCurrentNode(rhs.m_pCurrentNode),
m_childStack(rhs.m_childStack)
{
}

///\brief Asignment operator.
///
///@param rhs The iterator to copy.
const C_SceneNodeConstIterator& C_SceneNodeConstIterator::operator=(const C_SceneNodeConstIterator& rhs)
{
	m_pBaseNode		= rhs.m_pBaseNode;
	m_pCurrentNode	= rhs.m_pCurrentNode;
	m_childStack	= rhs.m_childStack;

	return *this;
}

///\brief Returns the current node.
///
///@return The current node the iterator is positioned at.
C_SceneNodeConstIterator::operator C_SceneNode*()
{
	return m_pCurrentNode;
}

C_SceneNode* C_SceneNodeConstIterator::operator->() 
{
	return m_pCurrentNode;
} 

const C_SceneNode* C_SceneNodeConstIterator::operator ++()
{
	if (m_pCurrentNode->GetChild() != 0) 
	{
		// Store the first child as we pass so that we can process them when we're done with iterating
		// thru the peers of the current node.
		m_childStack.PushBack(m_pCurrentNode->GetChild());
	}

	if (m_pCurrentNode->GetNextSibling() != 0)
	{
		m_pCurrentNode = m_pCurrentNode->GetNextSibling();
	}
	else
	{
		//We're at the end of our sibling list.
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

const C_SceneNode* C_SceneNodeConstIterator::operator ++(int) 
{
	const C_SceneNode* pTmp = ++(*this);
	return pTmp;
} 
	



C_SceneNodeIterator::C_SceneNodeIterator(C_SceneNode* pSceneNode)
:
C_SceneNodeConstIterator(pSceneNode) 
{
}

C_SceneNodeIterator::C_SceneNodeIterator(const C_SceneNodeIterator& rhs)
: 
C_SceneNodeConstIterator(rhs)
{
}

const C_SceneNodeIterator& C_SceneNodeIterator::operator=(const C_SceneNodeIterator& rhs)
{
	C_SceneNodeConstIterator::operator =(rhs);
	return *this;
}

C_SceneNodeIterator::operator C_SceneNode*()
{
	return m_pCurrentNode;
}

C_SceneNode* C_SceneNodeIterator::operator->() 
{
	return m_pCurrentNode;
}

C_SceneNode* C_SceneNodeIterator::operator ++()
{
	return const_cast<C_SceneNode*>(C_SceneNodeConstIterator::operator ++());
}

C_SceneNode* C_SceneNodeIterator::operator ++(int) 
{
	C_SceneNode* pTmp = ++(*this);
	return pTmp;
} 

}
