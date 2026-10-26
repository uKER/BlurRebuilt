#include "stdafx.h"
#include "SceneGraph/GatherNode.h"

#include "SceneGraph/Visitor.h"
#include "SceneGraph/MultiView.h"

namespace SceneGraph
{

void I_GatherNode::BeginFrame() const
{
	OnBeginFrame();
}

void I_GatherNode::Gather(const C_View &view) const
{
	OnGather(view);
}

void I_GatherNode::GatherDeferred(const void* pData) const
{
	OnGatherDeferred(pData);
}

void I_GatherNode::Gather(const C_MultiView &multiView) const
{
	OnGather(multiView);
}

void I_GatherNode::OnGather(const C_MultiView &multiView) const
{
	for(S32 i = 0; i < multiView.Size(); ++i)
		OnGather(*multiView.GetView(i));
}

// C_GroupNode

C_GroupNode::C_GroupNode(const C_SceneGraph* pSceneGraph) :
	I_GatherNode(E_GatherNodeType_Unknown, pSceneGraph)
{
}

C_GroupNode::C_GroupNode(S32 type, const C_SceneGraph* pSceneGraph) :
	I_GatherNode(type, pSceneGraph)
{
}

C_GroupNode::~C_GroupNode()
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		delete m_Children[i];
	}
}

void C_GroupNode::AddChild(I_GatherNode* pChild)
{
	m_Children.PushBack(pChild);
}

void C_GroupNode::RemoveChild(I_GatherNode* pChild)
{
	assert(pChild);

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		if(m_Children[i] == pChild)
		{
			m_Children.EraseByIndex(i);
			return;
		}
	}
}

void C_GroupNode::BeginFrame() const
{
	for (S32 i = 0, count = m_Children.Size(); i < count; ++i)
		m_Children[i]->BeginFrame();
}

void C_GroupNode::Gather(const C_View& view) const
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
		m_Children[i]->Gather(view);
}

void C_GroupNode::Gather(const C_MultiView& multiView) const
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
		m_Children[i]->Gather(multiView);
}

void C_GroupNode::Accept(I_Visitor& visitor) const
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
		m_Children[i]->Accept(visitor);
}

void C_GroupNode::Accept(I_Visitor& visitor)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
		m_Children[i]->Accept(visitor);
}

} // namespace SceneGraph

