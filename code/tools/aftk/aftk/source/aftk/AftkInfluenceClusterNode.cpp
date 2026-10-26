#include <stdafx.h>

using namespace Aftk;

C_InfluenceClusterNode::C_InfluenceClusterNode(C_Scene* pScene) : C_Node(pScene)
{
	AFTK_ASSERT(pScene);
	m_pSceneNode = 0;
}

C_Node* C_InfluenceClusterNode::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_InfluenceClusterNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_InfluenceClusterNode(pScene);
		(C_InfluenceClusterNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_InfluenceClusterNode::C_InfluenceClusterNode(const C_InfluenceClusterNode& rhs) : C_Node(0)
{
	AFTK_ASSERT(rhs.m_pScene);

	*this = rhs;
}

C_InfluenceClusterNode::~C_InfluenceClusterNode()
{
}

const C_InfluenceClusterNode& C_InfluenceClusterNode::operator =(const C_InfluenceClusterNode& rhs)
{
	m_envelopes = rhs.m_envelopes;

	if (m_pScene != rhs.m_pScene)
	{
		//Remove envelope influence pointers.
		for (U32 iEnv = 0; iEnv < (U32)m_envelopes.Size(); iEnv++)
		{
			m_envelopes[iEnv].SetInfluenceNodePtr(0);
		}
	}

	//Don't copy scene node association.
	m_pSceneNode = 0;

	m_usage = rhs.m_usage;

	//Note the index of this cluster in the scene is not copied.
	return *this;
}

U32 C_InfluenceClusterNode::AddEnvelope(const C_WeightEnvelope& envelope)
{
	m_envelopes.PushBack(envelope);

	return m_envelopes.Size()-1;
}

void C_InfluenceClusterNode::SetEnvelope(U32 instance, const C_WeightEnvelope& envelope)
{
	m_envelopes[instance] = envelope;
}

void C_InfluenceClusterNode::SetEnvelopeInitialCapacity(U32 capacity)
{
	m_envelopes.Reserve(capacity);
}

void C_InfluenceClusterNode::SetUsage(const std::string& usage)
{
	m_usage = usage;
}

U32	C_InfluenceClusterNode::GetNumEnvelopes() const
{
	return (U32)m_envelopes.Size();
}

const C_WeightEnvelope& C_InfluenceClusterNode::GetEnvelope(U32 instance) const
{
	return m_envelopes[instance];
}

C_WeightEnvelope& C_InfluenceClusterNode::GetEnvelope(U32 instance)
{
	return m_envelopes[instance];
}

C_SceneNode* C_InfluenceClusterNode::GetSceneNodePtr()
{
	return m_pSceneNode;
}

const C_SceneNode* C_InfluenceClusterNode::GetSceneNodePtr() const
{
	return m_pSceneNode;
}

const std::string& C_InfluenceClusterNode::GetUsage() const
{
	return m_usage;
}

void C_InfluenceClusterNode::SetSceneNodePtr(C_SceneNode* pSceneNode)
{
	if (pSceneNode == m_pSceneNode)
	{
		return;
	}

	//If we have a valid skin cluster, let the scene node know that this cluster has been associated with it
	if (pSceneNode)
	{
		for (U32 i = 0; i < pSceneNode->GetNumInfluenceClusters(); i++)
		{
			if (pSceneNode->GetInfluenceClusterPtr(i) == this)
			{
				//we're already associated with this node
				return;
			}
		}
		//else attach this scene node.
		pSceneNode->AddInfluenceClusterPtr(this);
		m_pSceneNode = pSceneNode;
	}
	else
	{
		//Remove association with our existing scene node.
		if (m_pSceneNode)
		{
			m_pSceneNode->DeleteInfluenceClusterPtr(this);
		}
		
		if (pSceneNode)
		{
			pSceneNode->AddInfluenceClusterPtr(this);
			m_pSceneNode = pSceneNode;
		}
		else
		{
			m_pSceneNode = 0;
		}
	}
}

E_ObjType C_InfluenceClusterNode::GetType() const
{
	return E_ObjType_InfluenceCluster;
}

bool C_InfluenceClusterNode::HasType(E_ObjType type) const
{
	if (C_Node::HasType(type))
		return true;
	if (type == E_ObjType_InfluenceCluster) 
		return true;

	return false;
}

void C_InfluenceClusterNode::Clear()
{
	SetSceneNodePtr(0);
	m_envelopes.Clear();
	m_usage.clear();
}

bool C_InfluenceClusterNode::IsUsed() const
{
	if (
		C_Node::IsUsed() ||
		m_pSceneNode)
		return true;

	return false;
}
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif
