#include <stdafx.h>

using namespace Aftk;

C_Node* C_LayerNode::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_LayerNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_LayerNode(pScene);
		*pNewNode = *this;
	}

	return pNewNode;
}

bool C_LayerNode::IsMember(const C_SceneNode* pSceneNode) const
{
	Ut::UtMap<C_SceneNode*, S32>::ConstIterator it;
	it = m_memberMap.Find(const_cast<C_SceneNode*>(pSceneNode));
	if (it != m_memberMap.End())
		return true;
	return false;
}

U32 C_LayerNode::AddMember(C_SceneNode* pSceneNode)
{
	//Remove this node from any existing layer.
	Ut::UtMap<C_SceneNode*, S32>::ConstIterator it;
	it = m_memberMap.Find(pSceneNode);

	if (it == m_memberMap.End())
	{
		//The given scene node is not already contained by this layer, so add it.
		m_memberMap[pSceneNode] = (U32)m_memberVector.Size();
		m_memberVector.PushBack(pSceneNode);

		//IMPORTANT - ACCESS NODE VIA FRIENDSHIP TO LINK IT TO THIS LAYER.
		pSceneNode->AttachToLayer(this);

		//Return the index of the new scene node in this layer.
		return (U32)m_memberVector.Size() -1;
	}
	else
	{
		//Do nothing, the given node is already a member of this layer! Return its index.
		return it->GetValue();
	}
}

C_SceneNode* C_LayerNode::GetMemberPtr(U32 instance)
{
	return m_memberVector[instance];
}

const C_SceneNode* C_LayerNode::GetMemberPtr(U32 instance) const
{
	return m_memberVector[instance];
}

S32 C_LayerNode::GetMemberIndex(C_SceneNode* pSceneNode)
{
	Ut::UtMap<C_SceneNode*, S32>::ConstIterator it;
	it = m_memberMap.Find(pSceneNode);

	if (it == m_memberMap.End())
	{
		// Not found;
		return -1;
	}
	else
	{
		//We found what we were looking for! :D
		return it->GetValue();
	}
}

void C_LayerNode::SetMember(U32 instance, C_SceneNode* pSceneNode)
{
	C_SceneNode* pOldNode = m_memberVector[instance];
	m_memberVector[instance] = pSceneNode;
	m_memberMap.Erase(pOldNode);
	m_memberMap[pSceneNode] = instance;
}

U32 C_LayerNode::GetNumMembers() const
{
	return (U32)m_memberVector.Size();
}

void C_LayerNode::DeleteMember(C_SceneNode* pSceneNode)
{
	U32 arrayIndex;
	{
		Ut::UtMap<C_SceneNode*, S32>::ConstIterator it;
		it = m_memberMap.Find(pSceneNode);

		if (it == m_memberMap.End())
		{
			// Not found;
			return;
		}

		arrayIndex = it->GetValue();
		m_memberVector.EraseByIndex(arrayIndex);
		m_memberMap.Erase(pSceneNode);

		//IMPORTANT - ACCESS NODE VIA FRIENDSHIP TO UNLINK IT FROM THIS LAYER
		pSceneNode->DetachFromLayer(this);
	}
	{
		//Now shuffle down the indices of the map members due to the removed element
		Ut::UtMap<C_SceneNode*, S32>::Iterator it;
		it = m_memberMap.Begin();
		while(it != m_memberMap.End())
		{
			U32 currentArrayIndex = it->GetValue();
			if (currentArrayIndex > arrayIndex)
			{
				//shuffle down.
				currentArrayIndex--;
				it->GetValue() = currentArrayIndex;
			}
			it++;
		};
	}	
}

bool C_LayerNode::IsUsed() const
{
	if (C_Node::IsUsed() || m_memberVector.Size() > 0)
		return true;

	return false;
}

void C_LayerNode::Clear()
{
	ClearThis();
	C_Node::Clear();
}

void C_LayerNode::ClearThis()
{
	//Disassociate layer members from the layer.
	for (U32 iMember = 0; iMember < (U32)m_memberVector.Size(); iMember++)
	{
		C_SceneNode* pMember = m_memberVector[iMember];
		if (pMember)
			pMember->DetachFromLayer(this);
	}

	m_memberMap.Clear();
	m_memberVector.Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

C_LayerNode::C_LayerNode(C_Scene* pScene) : C_Node(pScene)
{
	OneTimeInit();
}

C_LayerNode::~C_LayerNode()
{
	Clear();
	if (m_pScene)
		m_pScene->OnDestroyLayer(this);
}


C_LayerNode::C_LayerNode(const C_LayerNode& rhs) : C_Node(0)
{
	*this = rhs;
}

const C_LayerNode& C_LayerNode::operator=(const C_LayerNode& rhs)
{
	ClearThis();

	bool attachAfterCopy;
	if (!m_pScene)
		attachAfterCopy = true;
	else
		attachAfterCopy = false;

	C_Node::operator =(rhs);

	//Don't copy membership between scenes - this must be rebuilt seperately.
	if (m_pScene == rhs.m_pScene)
	{
		m_memberMap = rhs.m_memberMap;
		m_memberVector = rhs.m_memberVector;
	}

	if (attachAfterCopy)
		m_pScene->OnCreateLayer(this);

	return *this;
}

E_ObjType C_LayerNode::GetType() const
{
	return E_ObjType_Layer;
}

bool C_LayerNode::HasType(E_ObjType type) const
{
	if (C_Node::HasType(type))
		return true;
	if (type == E_ObjType_Layer) 
		return true;

	return false;
}

void C_LayerNode::OneTimeInit()
{
	if (m_pScene)
		m_pScene->OnCreateLayer(this);
}

#endif


