#include "stdafx.h"
#include "SceneGraph/RenderNode.h"

#include "SceneGraph/SceneGraphImpl.h"


namespace SceneGraph
{

I_RenderNode::I_RenderNode() :
	m_Type(E_RenderNodeType_Unknown)
{
}


I_RenderNode::I_RenderNode(S32 type) :
	m_Type(type)
{
}

I_RenderNode::~I_RenderNode()
{
}

void I_RenderNode::Render(C_Context& context) const
{
	OnRender(context);
}

} // namespace SceneGraph

