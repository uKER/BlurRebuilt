#include "stdafx.h"
#include "AftkRender.h"

using namespace AftkRender;

C_RenderInstanceNode::C_RenderInstanceNode(
	Aftk::C_SceneNode *pAftkNode,
	C_SceneRenderer *pRenderer)
:
	C_RenderNode(pAftkNode, pRenderer),
	m_pSourceNode(0)
{
}

C_RenderInstanceNode::~C_RenderInstanceNode()
{
}

E_RenderNodeType C_RenderInstanceNode::GetType() const
{
	return E_RenderNodeType_Node_Instance;
}

bool C_RenderInstanceNode::HasType(
	E_RenderNodeType type) const
{
	return (type == C_RenderInstanceNode::GetType()
			|| C_RenderNode::HasType(type));
}

void C_RenderInstanceNode::SetSourceNodePtr(C_RenderNode *pRenderNode)
{
	m_pSourceNode = pRenderNode;
}

const C_RenderNode* C_RenderInstanceNode::GetSourceNodePtr() const
{
	return m_pSourceNode;
}

C_RenderNode* C_RenderInstanceNode::GetSourceNodePtr()
{
	return m_pSourceNode;
}