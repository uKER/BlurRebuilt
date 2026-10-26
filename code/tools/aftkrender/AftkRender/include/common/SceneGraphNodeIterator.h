#ifndef AFTK_RENDER_SCENE_GRAPH_NODE_ITERATOR_H
#define AFTK_RENDER_SCENE_GRAPH_NODE_ITERATOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_Scene;

	class C_SceneGraphNodeConstIterator
	{
	public:
		C_SceneGraphNodeConstIterator(const C_Scene *pScene)
		: m_pBaseNode(const_cast<C_SceneGraphNode*>(pScene->GetRootSceneGraphNodePtr()))
		, m_pCurrentNode(const_cast<C_SceneGraphNode*>(pScene->GetRootSceneGraphNodePtr()))
		{
		}

		C_SceneGraphNodeConstIterator(const C_SceneGraphNode *pNode)
		: m_pBaseNode(const_cast<C_SceneGraphNode*>(pNode))
		, m_pCurrentNode(const_cast<C_SceneGraphNode*>(pNode))
		{
		}

		C_SceneGraphNodeConstIterator(const C_SceneGraphNodeConstIterator& r)
		: m_pBaseNode(r.m_pBaseNode)
		, m_pCurrentNode(r.m_pCurrentNode)
		, m_ChildNodeStack(r.m_ChildNodeStack)
		{
		}

		const C_SceneGraphNodeConstIterator& operator=(const C_SceneGraphNodeConstIterator& r)
		{
			m_pBaseNode = r.m_pBaseNode;
			m_pCurrentNode = r.m_pCurrentNode;
			m_ChildNodeStack = r.m_ChildNodeStack;
		
			return *this;
		}

		operator C_SceneGraphNode*()
		{
			return m_pCurrentNode;
		}

		C_SceneGraphNode* operator->()
		{
			return m_pCurrentNode;
		}

		const C_SceneGraphNode* operator++()
		{
			if (m_pCurrentNode->GetChildPtr(0) != 0)
				m_ChildNodeStack.PushBack(m_pCurrentNode->GetChildPtr(0));

			if (m_pCurrentNode->GetNextSiblingPtr() != 0)
				m_pCurrentNode = m_pCurrentNode->GetNextSiblingPtr();
			else
			{
				S32 stackSize = m_ChildNodeStack.Size();
				if (stackSize > 0)
				{
					m_pCurrentNode = m_ChildNodeStack[stackSize-1];
					m_ChildNodeStack.PopBack();
				}
				else
					m_pCurrentNode = 0;
			}

			return m_pCurrentNode;
		}

		const C_SceneGraphNode* operator ++(S32 inc)
		{
			const C_SceneGraphNode *pTmp = ++(*this);
			return pTmp;
		}

		void Reset()
		{
			m_pCurrentNode = m_pBaseNode;
			m_ChildNodeStack.Clear();
		}

	protected:
		C_SceneGraphNode *m_pBaseNode;
		C_SceneGraphNode *m_pCurrentNode;
		Ut::UtVector<Ut::DynamicVector<C_SceneGraphNode*> > m_ChildNodeStack;
	};




	class C_SceneGraphNodeIterator : public C_SceneGraphNodeConstIterator
	{
	public:
		C_SceneGraphNodeIterator(C_Scene *pScene)
		: C_SceneGraphNodeConstIterator(pScene)
		{
		}

		C_SceneGraphNodeIterator(C_SceneGraphNode *pNode)
		: C_SceneGraphNodeConstIterator(pNode)
		{
		}

		C_SceneGraphNodeIterator(const C_SceneGraphNodeIterator& r)
		: C_SceneGraphNodeConstIterator(r)
		{
		}

		const C_SceneGraphNodeIterator& operator=(const C_SceneGraphNodeIterator& r)
		{
			m_pBaseNode = r.m_pBaseNode;
			m_pCurrentNode = r.m_pBaseNode;
			m_ChildNodeStack = r.m_ChildNodeStack;

			return *this;
		}

		operator C_SceneGraphNode*()
		{
			return m_pCurrentNode;
		}

		C_SceneGraphNode* operator->()
		{
			return m_pCurrentNode;
		}

		C_SceneGraphNode* operator++()
		{
			return const_cast<C_SceneGraphNode*>(C_SceneGraphNodeConstIterator::operator++());
		}

		C_SceneGraphNode* operator++(S32 inc)
		{
			C_SceneGraphNode *pTmp = ++(*this);
			return pTmp;
		}
	};
}

#endif