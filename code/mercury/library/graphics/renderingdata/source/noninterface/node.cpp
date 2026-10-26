#include "stdafx.h"
#include "renderingdata/node.h"
#include "renderingdata/scene.h"

namespace RenderingData
{

Node::~Node()
{
#if !PLATFORM_SPU
	if (m_pScene)
		m_pScene->DeregisterNode(this);
#endif
}

void Node::SetScenePtr(Scene* pScene)
{
	if (pScene) pScene->RegisterNode(this);
	m_pScene = pScene;
}

S32 Node::AddSubNode(const ResourceIndex& i)
{
	Node* pNode = m_pScene->GetNodePtr(i);
	assert(pNode);
	assert(pNode->GetParent().IsNull());
	pNode->SetParent(m_location);

	m_subNodes.PushBack(i);
	return m_subNodes.Size() - 1;
}

ResourceIndex Node::EraseSubNode(int i)
{
	Node* pNode = m_pScene->GetNodePtr(m_subNodes[i]);
	if (pNode)
	{
		assert(pNode->GetParent() == m_location);
		pNode->SetParent(ResourceIndex::Null());
	}
	
	ResourceIndex node = m_subNodes[i];
	m_subNodes.EraseByIndex(i);
	return node;
}

ResourceIndex Node::ClearSubNode(S32 i)
{
	Node* pNode = m_pScene->GetNodePtr(m_subNodes[i]);
	if (pNode)
	{
		assert(pNode->GetParent() == m_location);
		pNode->SetParent(ResourceIndex::Null());
	}

	ResourceIndex node = m_subNodes[i];
	m_subNodes[i] = ResourceIndex::Null();
	return node;
}

void Node::ReplaceSubNode(S32 i, const ResourceIndex& r)
{
	ClearSubNode(i);

	Node* pNode = m_pScene->GetNodePtr(r);
	
	if (pNode)
	{
		assert(m_pScene->GetNodePtr(pNode->GetParent())== 0);
		pNode->SetParent(m_location);
	}

	m_subNodes[i] = r;
}

}
